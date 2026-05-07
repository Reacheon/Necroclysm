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
//     2) 곡선 강·해안 — 도메인 워핑 + 부호 거리장 (현재 구현)
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

    //═══════════════════════════════════════════════════════════════════════
    // 1) Raw 픽셀 기반 베이스 페인트
    //   각 타일의 raw 픽셀(48타일 블록)을 Terrain으로 받아 PaintCell로 변환.
    //   픽셀 양자화(48-tile 계단)는 2단계가 수계 경계에 한해 곡선으로 덮어씀.
    //═══════════════════════════════════════════════════════════════════════
    for (int dy = 0; dy < SectorCoord::TILES; ++dy)
    {
        const int wty = sectorOriginTileY + dy;
        const int rawPy = (wty - TILE_BASE_Y) / TILE_PER_PIXEL;

        for (int dx = 0; dx < SectorCoord::TILES; ++dx)
        {
            const int wtx = sectorOriginTileX + dx;
            const int rawPx = (wtx - TILE_BASE_X) / TILE_PER_PIXEL;

            const procGen::Terrain pixelTerrain = procGen::worldPixel(rawPx, rawPy);

            //per-tile 결정론 randomVal — (seed, worldTile) 해시 16비트.
            //  세이브/로드 후에도 같은 시드면 같은 스프라이트 변형 보장.
            std::uint64_t tileHash = seed ^ 0x9E3779B97F4A7C15ULL;
            tileHash ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(wtx)) * 0xBF58476D1CE4E5B9ULL;
            tileHash = (tileHash ^ (tileHash >> 27)) * 0x94D049BB133111EBULL;
            tileHash ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(wty)) * 0x94D049BB133111EBULL;
            tileHash ^= tileHash >> 31;

            //Terrain → PaintCell 매핑.
            PaintCell cell;
            cell.randomVal = static_cast<std::uint16_t>(tileHash & 0xffffu);
            cell.floor = itemID::dirt;        //일반 땅 디폴트 (이전: grass)
            cell.flags = TILE_FLAG_WALKABLE;

            switch (pixelTerrain)
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
                cell.floor = itemID::dirt;
                break;

            case procGen::Terrain::Desert:
                cell.floor = itemID::sandFloor;
                break;

            case procGen::Terrain::Mountain:
                cell.floor = itemID::dirt;    //(TODO: mountain wall + 등반)
                break;

            case procGen::Terrain::Tundra:
            case procGen::Terrain::Polar:
                cell.floor = itemID::dirt;
                cell.flags |= TILE_FLAG_HAS_SNOW;
                break;

            case procGen::Terrain::CityZone:
            case procGen::Terrain::CityCenter:
                cell.floor = itemID::paver;   //도시 기본 보도블럭 (TODO: BCP layout이 도로/건물 페인트)
                break;
            }

            plan.tiles[static_cast<std::size_t>(dy) * SectorCoord::TILES + dx] = cell;
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // 2) 곡선 강·해안 — 8-이웃 픽셀 단위 land 채우기 (사분면 룰)
    //
    //   픽셀 베이스(1단계) 위에 land로만 곡선 채우기. 즉 *water 픽셀의 내부*
    //   타일 일부를 dirt로 덮어 쓴다. land 픽셀은 *절대* 손대지 않음.
    //
    //   왜 단방향: 이전 bilinear 보간은 land와 water가 *양방향* 침투 →
    //     - land가 깎이면 도시·도로 페인트와 충돌, 결과 모양 예측 불가
    //     - 룰이 암묵적이라 패턴별 의도 표현 불가
    //   한 방향(water → land)으로만 변형하면 land 영역 보존 + 룰 명료.
    //
    //   사분면 처리: 각 픽셀을 NE/NW/SE/SW 4 사분면(24×24)으로 분할.
    //     각 사분면에 영향 주는 3 이웃(예: NE → N, NE, E)의 land 마스크
    //     8가지 패턴별로 채우기 모양 결정. 다른 3 사분면은 좌표 미러로 동일 처리.
    //
    //   패턴별 모양 (A=수직변 이웃, B=대각 이웃, C=수평변 이웃):
    //     000 - 모두 water        → 채우기 없음
    //     100 - A만 land          → A변 따라 띠, 외각 코너로 갈수록 두께 0
    //     001 - C만 land          → C변 따라 띠, 외각 코너로 갈수록 두께 0
    //     010 - B만 land          → 외각 코너에 작은 점
    //     110 - A+B               → A변 따라 균일 두께 띠 (외각까지 유지)
    //     011 - B+C               → C변 따라 균일 두께 띠
    //     101 - A+C, B water      → 두 띠 합집합 (드문 패턴)
    //     111 - 모두 land         → 4분원 (안쪽 사분원 water, 그 밖 land)
    //
    //   노이즈: 띠 두께에 저주파 ±2 wave 추가. world 좌표 hash + 4타일 스케일
    //     선형 보간 → 픽셀 경계에서 자동 연속.
    //
    //   섹터 경계 연속성: 마진 1px 포함 마스크 채집 → 8 이웃 모두 안전.
    //═══════════════════════════════════════════════════════════════════════

    constexpr int MARGIN_PX = 1;
    constexpr int SECTOR_PX = SectorCoord::TILES / TILE_PER_PIXEL;
    constexpr int FIELD_SZ  = SECTOR_PX + 2 * MARGIN_PX;

    const int sectorOriginPxX = (sectorOriginTileX - TILE_BASE_X) / TILE_PER_PIXEL;
    const int sectorOriginPxY = (sectorOriginTileY - TILE_BASE_Y) / TILE_PER_PIXEL;
    const int fieldOriginPxX  = sectorOriginPxX - MARGIN_PX;
    const int fieldOriginPxY  = sectorOriginPxY - MARGIN_PX;

    //─── 2-A) 마진 포함 픽셀 Terrain 채집 ──────────────────────────────────
    std::vector<procGen::Terrain> terr(static_cast<std::size_t>(FIELD_SZ) * FIELD_SZ);
    for (int fy = 0; fy < FIELD_SZ; ++fy)
    {
        const int rawPy = fieldOriginPxY + fy;
        for (int fx = 0; fx < FIELD_SZ; ++fx)
        {
            const int rawPx = fieldOriginPxX + fx;
            terr[static_cast<std::size_t>(fy) * FIELD_SZ + fx] = procGen::worldPixel(rawPx, rawPy);
        }
    }

    auto isLandTerrain = [](procGen::Terrain t) -> bool {
        switch (t)
        {
        case procGen::Terrain::Sea:
        case procGen::Terrain::CitySea:
        case procGen::Terrain::River:
        case procGen::Terrain::Lake:
        case procGen::Terrain::CityRiver:
            return false;
        default:
            return true;
        }
    };

    //─── 2-B) 저주파 1D wave (world 좌표 기반, ±2 타일) ────────────────────
    //   인접 픽셀과 자동 연속: 같은 world 좌표 → 같은 wave 값.
    auto wave1D = [](int worldCoord) -> int {
        constexpr int SCALE = 4;
        const int g0 = (worldCoord >= 0) ? (worldCoord / SCALE) : ((worldCoord - SCALE + 1) / SCALE);
        const int g1 = g0 + 1;
        const int t  = worldCoord - g0 * SCALE;
        auto hash = [](int g) {
            std::uint32_t v = static_cast<std::uint32_t>(g) ^ 0x9E3779B9u;
            v *= 0x85EBCA6Bu; v ^= v >> 13;
            v *= 0xC2B2AE35u; v ^= v >> 16;
            return static_cast<int>(v % 5) - 2;
        };
        const int a = hash(g0);
        const int b = hash(g1);
        return (a * (SCALE - t) + b * t) / SCALE;
    };

    //─── 2-C) 사분면 채우기 룰 ─────────────────────────────────────────────
    //   NE 사분면 좌표: qx ∈ [0, Q), qy ∈ [0, Q).
    //     qx=0 → 픽셀 중앙 수직 라인,    qx=Q-1 → 픽셀 우측 변 (외각)
    //     qy=0 → 픽셀 상단 변 (외각),    qy=Q-1 → 픽셀 중앙 수평 라인
    //   dN = qy (A변까지 거리),  dE = (Q-1)-qx (C변까지 거리)
    //   외각 코너 = (qx=Q-1, qy=0),     안쪽 코너 = (qx=0, qy=Q-1)
    constexpr int Q       = TILE_PER_PIXEL / 2;   // 24
    constexpr int D_THIN  = 6;                    // 단일 변 최대 두께
    constexpr int D_BAND  = 10;                   // 변+코너 균일 띠 두께
    constexpr int R_SMALL = 5;                    // 코너만 있을 때 작은 점 반경
    constexpr int R_DISK  = Q - 1;                // 4분원 반경

    auto fillQuadrant = [&](bool A, bool B, bool C, int qx, int qy, int waveA, int waveC) -> bool {
        const int dN       = qy;
        const int dE       = (Q - 1) - qx;
        const int dInnerSq = qx * qx + (Q - 1 - qy) * (Q - 1 - qy);
        const int dOuterSq = dN * dN + dE * dE;
        const int p = (A ? 4 : 0) | (B ? 2 : 0) | (C ? 1 : 0);

        switch (p)
        {
        case 0b000: return false;
        case 0b100: { const int t = D_THIN * (Q - 1 - qx) / (Q - 1) + waveA; return dN < t; }
        case 0b001: { const int t = D_THIN * (Q - 1 - qy) / (Q - 1) + waveC; return dE < t; }
        case 0b010: return dOuterSq < R_SMALL * R_SMALL;
        case 0b110: { const int t = D_BAND + waveA; return dN < t; }
        case 0b011: { const int t = D_BAND + waveC; return dE < t; }
        case 0b101: { const int tA = (D_THIN + 2) + waveA; const int tC = (D_THIN + 2) + waveC; return dN < tA || dE < tC; }
        case 0b111: { const int r = R_DISK + waveA; return dInnerSq >= r * r; }
        }
        return false;
    };

    //─── 2-D) 타일별 처리 ──────────────────────────────────────────────────
    //   water 픽셀에 한해 사분면 룰 적용. land 픽셀은 1단계 결과 그대로 유지.
    for (int dy = 0; dy < SectorCoord::TILES; ++dy)
    {
        const int wty    = sectorOriginTileY + dy;
        const int relY   = wty - TILE_BASE_Y;
        const int rawPy  = relY / TILE_PER_PIXEL;
        const int localY = relY - rawPy * TILE_PER_PIXEL;
        const int fy     = rawPy - fieldOriginPxY;

        for (int dx = 0; dx < SectorCoord::TILES; ++dx)
        {
            const int wtx    = sectorOriginTileX + dx;
            const int relX   = wtx - TILE_BASE_X;
            const int rawPx  = relX / TILE_PER_PIXEL;
            const int localX = relX - rawPx * TILE_PER_PIXEL;
            const int fx     = rawPx - fieldOriginPxX;

            const std::size_t centerIdx = static_cast<std::size_t>(fy) * FIELD_SZ + fx;
            if (isLandTerrain(terr[centerIdx])) continue;

            const bool n  = isLandTerrain(terr[static_cast<std::size_t>(fy - 1) * FIELD_SZ + fx]);
            const bool s  = isLandTerrain(terr[static_cast<std::size_t>(fy + 1) * FIELD_SZ + fx]);
            const bool e  = isLandTerrain(terr[static_cast<std::size_t>(fy)     * FIELD_SZ + fx + 1]);
            const bool w  = isLandTerrain(terr[static_cast<std::size_t>(fy)     * FIELD_SZ + fx - 1]);
            const bool ne = isLandTerrain(terr[static_cast<std::size_t>(fy - 1) * FIELD_SZ + fx + 1]);
            const bool nw = isLandTerrain(terr[static_cast<std::size_t>(fy - 1) * FIELD_SZ + fx - 1]);
            const bool se = isLandTerrain(terr[static_cast<std::size_t>(fy + 1) * FIELD_SZ + fx + 1]);
            const bool sw = isLandTerrain(terr[static_cast<std::size_t>(fy + 1) * FIELD_SZ + fx - 1]);

            //   사분면 결정 + 좌표 미러 → 모든 사분면을 NE 형태(qx,qy ∈ [0,Q))로 통일
            int qx, qy;
            bool A, B, C;
            if      (localX >= Q && localY <  Q) { qx = localX - Q;          qy = localY;                          A = n; B = ne; C = e; }
            else if (localX <  Q && localY <  Q) { qx = (Q - 1) - localX;    qy = localY;                          A = n; B = nw; C = w; }
            else if (localX >= Q && localY >= Q) { qx = localX - Q;          qy = (TILE_PER_PIXEL - 1) - localY;   A = s; B = se; C = e; }
            else                                 { qx = (Q - 1) - localX;    qy = (TILE_PER_PIXEL - 1) - localY;   A = s; B = sw; C = w; }

            //   wave 입력: A변(수평변)은 변 따라 wtx 변화, C변(수직변)은 변 따라 wty 변화.
            const int waveA = wave1D(wtx);
            const int waveC = wave1D(wty);

            if (fillQuadrant(A, B, C, qx, qy, waveA, waveC))
            {
                const std::size_t tileIdx = static_cast<std::size_t>(dy) * SectorCoord::TILES + dx;
                plan.tiles[tileIdx].floor = itemID::dirt;
                plan.tiles[tileIdx].flags = TILE_FLAG_WALKABLE;
            }
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // TODO 향후 단계 (모두 본 함수에 누적)
    //   3) 인카운터 사이트 좌표 (Land 픽셀 위에 결정론 배치)
    //   4) T1 도로 폴리라인이 sector 통과 시 분기 국도 생성
    //   5) 도시 layout (BCP) — 블록·도로·건물 페인트
    //   6) Bridge 후처리 (도로↔수계)
    //═══════════════════════════════════════════════════════════════════════

    return plan;
}
