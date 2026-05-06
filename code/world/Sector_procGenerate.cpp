module Sector;

import std;
import util;
import constVar;
import procGen;

// ════════════════════════════════════════════════════════════════════════
// procGenerate — Sector-level 절차생성의 단일 슈퍼함수.
//
//   책임: SectorPlan.tiles (3840×3840 PaintCell) 전부를 결정해서 채움.
//        청크는 본 산출물을 *블릿만* — 자체 결정 0.
//
//   향후 단계는 모두 본 함수에 누적됨:
//     1) raw 픽셀 기반 베이스 페인트 (현재 구현)
//     2) 곡선 강·해안 광역 알고리즘 (마칭 스퀘어 등 — TODO)
//     3) 인카운터 사이트 좌표
//     4) 도시 BCP 결과로 블록·도로·건물 페인트
//     5) T1 도로 폴리라인 아스팔트
//     6) Bridge 후처리 (도로↔수계 교차)
//
//   각 단계가 *같은 14.7M PaintCell 배열*에 *적층 페인트* (Painter's algorithm).
//   순서가 중요 — 나중 단계가 앞 단계를 덮어씀.
//
//   결정론: 같은 seed + sc → 같은 SectorPlan. 세이브/로드 후 재현 보장.
//
//   ProcGenWorker 백그라운드 스레드에서 호출됨 — World 참조 X, mmap read-only.
//
//   헬퍼 분리 안 함 (CLAUDE.md): 모든 로직이 본 함수 안에 인라인. 1곳에서만 쓰이는
//   서브로직을 추출하면 navigation 비용만 늘고 이득 없음. 향후 *2곳 이상*에서
//   필요해지거나 *교체 가능성*이 명확해지면 그때 추출.
// ════════════════════════════════════════════════════════════════════════

SectorPlan procGenerate(SectorCoord sc, std::uint64_t seed)
{
    SectorPlan plan(sc);
    plan.tiles.resize(static_cast<std::size_t>(SectorCoord::TILES) * SectorCoord::TILES);

    constexpr int TILE_BASE_X = -54 * PIXEL_PER_PATCH * TILE_PER_PIXEL;   // -1,036,800
    constexpr int TILE_BASE_Y = -27 * PIXEL_PER_PATCH * TILE_PER_PIXEL;   //   -518,400

    const int sectorOriginTileX = sc.x * SectorCoord::TILES;
    const int sectorOriginTileY = sc.y * SectorCoord::TILES;

    //--- 1) Raw 픽셀 기반 베이스 페인트 ---
    //   각 타일의 raw 픽셀(48타일 블록)을 Terrain으로 받아 PaintCell로 변환.
    //   현재는 픽셀 양자화가 그대로 보임 — 강·해안 곡선 알고리즘은 향후 본 베이스 위에 덮어쓰기.
    for (int dy = 0; dy < SectorCoord::TILES; ++dy)
    {
        const int wty = sectorOriginTileY + dy;
        const int rawPy = (wty - TILE_BASE_Y) / TILE_PER_PIXEL;

        for (int dx = 0; dx < SectorCoord::TILES; ++dx)
        {
            const int wtx = sectorOriginTileX + dx;
            const int rawPx = (wtx - TILE_BASE_X) / TILE_PER_PIXEL;

            const procGen::Terrain t = procGen::worldPixel(rawPx, rawPy);

            //per-tile 결정론 randomVal — (seed, worldTile) 해시 16비트.
            //  세이브/로드 후에도 같은 시드면 같은 스프라이트 변형 보장.
            std::uint64_t h = seed ^ 0x9E3779B97F4A7C15ULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(wtx)) * 0xBF58476D1CE4E5B9ULL;
            h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(wty)) * 0x94D049BB133111EBULL;
            h ^= h >> 31;

            //Terrain → PaintCell 매핑.
            PaintCell cell;
            cell.randomVal = static_cast<std::uint16_t>(h & 0xffffu);
            cell.floor = 220;                 //meadow grass 디폴트
            cell.flags = TILE_FLAG_WALKABLE;

            switch (t)
            {
            case procGen::Terrain::Sea:
            case procGen::Terrain::CitySea:
                cell.floor = itemID::deepSeaWater;
                cell.flags = 0;               //walkable false
                break;

            case procGen::Terrain::River:
            case procGen::Terrain::Lake:
            case procGen::Terrain::CityRiver:
                cell.floor = itemID::deepFreshWater;
                cell.flags = 0;
                break;

            case procGen::Terrain::Land:
            case procGen::Terrain::Monsoon:
            case procGen::Terrain::InsularRainforest:
            case procGen::Terrain::Subarctic:
            case procGen::Terrain::ContinentalRainforest:
                cell.floor = 220;             //grass
                break;

            case procGen::Terrain::Desert:
                cell.floor = itemID::sandFloor;
                break;

            case procGen::Terrain::Mountain:
                cell.floor = 109;             //dirt placeholder (TODO: mountain wall + 등반)
                break;

            case procGen::Terrain::Tundra:
            case procGen::Terrain::Polar:
                cell.floor = 220;
                cell.flags |= TILE_FLAG_HAS_SNOW;
                break;

            case procGen::Terrain::CityZone:
            case procGen::Terrain::CityCenter:
                cell.floor = 109;             //city dirt placeholder (TODO: BCP layout)
                break;
            }

            plan.tiles[static_cast<std::size_t>(dy) * SectorCoord::TILES + dx] = cell;
        }
    }

    //--- TODO 향후 단계 (모두 본 함수에 누적) ---
    //   2) 곡선 강·해안 광역 알고리즘 (마칭 스퀘어 등 — sector 단위 광역 처리)
    //   3) 인카운터 사이트 좌표 (Land 픽셀 위에 결정론 배치)
    //   4) T1 도로 폴리라인이 sector 통과 시 분기 국도 생성
    //   5) 도시 layout (BCP) — 블록·도로·건물 페인트
    //   6) Bridge 후처리 (도로↔수계)

    return plan;
}
