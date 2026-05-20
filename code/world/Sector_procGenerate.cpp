module Sector;

import std;
import util;
import constVar;
import worldGrid;
import worldGen;
import city;
import CityPlan;

// ════════════════════════════════════════════════════════════════════════
// procGenerate — Sector-level 절차생성의 단일 슈퍼함수.
//
//   책임: SectorPlan.tiles (3840×3840 PaintCell) 전부를 결정해서 채움.
//        청크는 본 산출물을 *블릿만* — 자체 결정 0.
//
//   향후 단계는 모두 본 함수에 누적됨:
//     1) raw 픽셀 기반 베이스 페인트 (현재 구현)
//     2) 곡선 강·해안 — 47 autotile (현재 구현)
//     3) 광역 도로 폴리라인 페인트 — 15타일 asphalt, 사이드워크 X (현재 구현)
//     4) 인카운터 사이트 좌표
//     5) Bridge 후처리 (도로↔수계 교차 보강)
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

            const worldGrid::Terrain pixelTerrain = worldGrid::worldPixel(rawPx, rawPy);

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
            case worldGrid::Terrain::Sea:
            case worldGrid::Terrain::CitySea:
                cell.floor = itemID::deepSeaWater;
                cell.flags = 0;               //walkable false
                break;

            case worldGrid::Terrain::River:
            case worldGrid::Terrain::Lake:
            case worldGrid::Terrain::CityRiver:
                cell.floor = itemID::deepFreshWater;
                cell.flags = 0;
                break;

            case worldGrid::Terrain::Land:
            case worldGrid::Terrain::Monsoon:
            case worldGrid::Terrain::InsularRainforest:
            case worldGrid::Terrain::Subarctic:
            case worldGrid::Terrain::ContinentalRainforest:
                cell.floor = itemID::dirt;
                break;

            case worldGrid::Terrain::Desert:
                cell.floor = itemID::sandFloor;
                break;

            case worldGrid::Terrain::Mountain:
                cell.floor = itemID::dirt;    //(TODO: mountain wall + 등반)
                break;

            case worldGrid::Terrain::Tundra:
            case worldGrid::Terrain::Polar:
                cell.floor = itemID::dirt;
                cell.flags |= TILE_FLAG_HAS_SNOW;
                break;

            case worldGrid::Terrain::CityZone:
            case worldGrid::Terrain::CityCenter:
                //도시 픽셀은 그냥 dirt(기본값) — paver는 CityPlan stage 15가 도로 주변에만 깔음
                break;
            }

            plan.tiles[static_cast<std::size_t>(dy) * SectorCoord::TILES + dx] = cell;
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // 2) 곡선 강·해안 — 47 Autotile
    // 강이나 해안선은 픽셀(48타일)로 양자화되어 있기에 월드맵에서 보면 불연속적으로 보인다.
    // 기존에 물이 있던 자리를 땅으로 채우는 47 오토타일링을 공격적으로 해서 해안선이나 강가를 자연스럽게 만드려고 한다.
    // 오토타일링이므로 인접한 8개의 픽셀들이 어떤 픽셀인지를 참조해서 자기 위치의 픽셀(48타일) 내의 타일들을 흙으로 채워야 한다. 주변을 오염시키면 안된다.
    // 가능하면 자연스럽게 이어져야 한다.
    // 채우는 타일은 일단 dirt로 한다. 나중에 모래나 이런 걸로 바꿀 수도 있으나 지금은 이 정도로 하자.
    // 따로 헬퍼 함수를 만들지 말고 딥모듈 원칙을 따라서 이 함수 내에서 코드를 끝낼 것 (람다 함수가 중간에 필요하면 사용하여도 OK)
    //═══════════════════════════════════════════════════════════════════════

    //   접근법: 47-piece autotile prefab 룩업. 알고리즘 기반 SDF/사분면 룰을 모두
    //   포기 — 본질적 트레이드오프(경계 일관 vs 변 디테일 vs 코너 둥글기)가
    //   해결 불가. 대신 사용자가 직접 그린 PNG (image/spline/shoreSpline{0..N}.png,
    //   각 8×6 그리드의 48×48 셀 47개)를 마스크로 변환해 룩업.
    //
    //   각 water 픽셀에서:
    //     1) (seed, rawPx, rawPy) hash → variant 선택 (해안선 패턴 다양화)
    //     2) 8 이웃 land 마스크 → 47 인덱스 (GameMaker autotile47 컨벤션)
    //     3) worldGrid::shoreSplineMask[variant][idx] 룩업 → 픽셀 내부 (lx,ly) 위치의
    //        bool 값이 true면 그 타일을 dirt로 덮어씀.
    //   마스크 데이터는 textureLoader가 게임 시작 시 PNG 픽셀 색상(#5b4940=land,
    //   #3899ff=water)을 분석해 채움. variant끼리 변/코너 경계 패턴이 동일해야
    //   인접 픽셀에서 매끄럽게 연결됨.
    //
    //   인덱스 매핑 (47 = 16 + 16 + 2 + 8 + 4 + 1):
    //     0..15 — 변 0 + 외각 코너 16조합 (NW=1, NE=2, SE=4, SW=8 raw 비트 합)
    //     16..19 — L-Edge(W) + 외각 NE/SE 0/1조합
    //     20..23 — T-Edge(N) + 외각 SE/SW
    //     24..27 — R-Edge(E) + 외각 SW/NW
    //     28..31 — B-Edge(S) + 외각 NW/NE
    //     32 — L+R (수직 일자통로)
    //     33 — T+B (수평 일자통로)
    //     34..41 — 변 2 인접 (T+L, T+R, R+B, L+B) + 외각 코너 0/1
    //     42..45 — 변 3 (데드엔드 4가지)
    //     46 — 변 4 (둘러쌓인 물)
    //
    //   외각 코너는 *양변 모두 water*일 때만 raw 비트 의미. 양변 land 코너는
    //   prefab에서 자동 land 처리 (raw 비트 무관). 한쪽 변만 land인 코너는 변에
    //   흡수되어 raw 비트 무관. 이 마스킹 룰로 raw 8비트 → 47 인덱스 압축.
    //
    //   섹터 경계 연속성: 마진 1px 포함 Terrain 채집 → 8 이웃 안전 룩업.

    constexpr int MARGIN_PX = 1;
    constexpr int SECTOR_PX = SectorCoord::TILES / TILE_PER_PIXEL;
    constexpr int FIELD_SZ  = SECTOR_PX + 2 * MARGIN_PX;

    const int sectorOriginPxX = (sectorOriginTileX - TILE_BASE_X) / TILE_PER_PIXEL;
    const int sectorOriginPxY = (sectorOriginTileY - TILE_BASE_Y) / TILE_PER_PIXEL;
    const int fieldOriginPxX  = sectorOriginPxX - MARGIN_PX;
    const int fieldOriginPxY  = sectorOriginPxY - MARGIN_PX;

    std::vector<worldGrid::Terrain> terr(static_cast<std::size_t>(FIELD_SZ) * FIELD_SZ);
    for (int fy = 0; fy < FIELD_SZ; ++fy)
    {
        const int rawPy = fieldOriginPxY + fy;
        for (int fx = 0; fx < FIELD_SZ; ++fx)
        {
            const int rawPx = fieldOriginPxX + fx;
            terr[static_cast<std::size_t>(fy) * FIELD_SZ + fx] = worldGrid::worldPixel(rawPx, rawPy);
        }
    }

    auto isLandTerrain = [](worldGrid::Terrain t) -> bool {
        switch (t)
        {
        case worldGrid::Terrain::Sea:
        case worldGrid::Terrain::CitySea:
        case worldGrid::Terrain::River:
        case worldGrid::Terrain::Lake:
        case worldGrid::Terrain::CityRiver:
            return false;
        default:
            return true;
        }
    };

    //   픽셀 좌표 + seed hash → variant 선택. 결정론(같은 seed/좌표 → 항상 같은 variant)
    //   유지하면서 인접 픽셀별로 다른 prefab variant가 선택돼 해안선 패턴 다양화.
    //   shoreSplineVariantCount=0이면 graceful fallback (페이즈 2 미적용 — 1단계 결과만).
    auto pickVariant = [seed](int rawPx, int rawPy, int variantCount) -> int {
        if (variantCount <= 1) return 0;
        std::uint64_t h = seed ^ 0x9E3779B97F4A7C15ULL;
        h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(rawPx)) * 0xBF58476D1CE4E5B9ULL;
        h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
        h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(rawPy)) * 0x94D049BB133111EBULL;
        h ^= h >> 31;
        return static_cast<int>(h % static_cast<std::uint64_t>(variantCount));
    };

    //   8 이웃 land 마스크 → 47 prefab 인덱스. GameMaker autotile47 컨벤션.
    //   외각 코너는 양변 water일 때만 raw 비트 의미. 양변 land 코너는 prefab에서
    //   자동 처리되므로 raw 무관. 한쪽 변 land 코너도 raw 무관 (변에 흡수).
    auto autotile47 = [](bool n, bool e, bool s, bool w, bool nw, bool ne, bool sw, bool se) -> int {
        const bool oNW = nw && !n && !w;
        const bool oNE = ne && !n && !e;
        const bool oSW = sw && !s && !w;
        const bool oSE = se && !s && !e;
        const int edges = (n ? 1 : 0) + (e ? 1 : 0) + (s ? 1 : 0) + (w ? 1 : 0);

        if (edges == 0)
        {
            //   변 0: 외각 코너 16조합. NW=1, NE=2, SE=4, SW=8 비트 합.
            return (oNW ? 1 : 0) | (oNE ? 2 : 0) | (oSE ? 4 : 0) | (oSW ? 8 : 0);
        }
        if (edges == 4) return 46;
        if (edges == 1)
        {
            //   변 1 + 외각 코너 (변 인접 코너 2개는 한쪽 land라 흡수, 나머지 2개만 외각 가능).
            if (w) return 16 + ((oNE ? 1 : 0) | (oSE ? 2 : 0));
            if (n) return 20 + ((oSE ? 1 : 0) | (oSW ? 2 : 0));
            if (e) return 24 + ((oSW ? 1 : 0) | (oNW ? 2 : 0));
            return 28 + ((oNW ? 1 : 0) | (oNE ? 2 : 0));   // s
        }
        if (edges == 2)
        {
            if (w && e) return 32;   // L+R 수직 통로
            if (n && s) return 33;   // T+B 수평 통로
            //   변 2 인접: 양변 land 코너 1개(자동 처리), 양변 water 코너 1개(외각 가능).
            if (n && w) return oSE ? 35 : 34;   // T+L, 외각 SE
            if (n && e) return oSW ? 37 : 36;   // T+R, 외각 SW
            if (e && s) return oNW ? 39 : 38;   // R+B, 외각 NW
            return oNE ? 41 : 40;               // L+B, 외각 NE
        }
        //   edges == 3: 데드엔드. 양변 land 코너 2개 자동 처리, 외각 가능 0개.
        if (!s) return 42;   // L+T+R
        if (!e) return 43;   // L+T+B
        if (!n) return 44;   // L+B+R
        return 45;           // T+R+B
    };

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

            //   자기 픽셀이 land이면 1단계 결과 그대로 (주변 오염 X).
            if (isLandTerrain(terr[static_cast<std::size_t>(fy) * FIELD_SZ + fx])) continue;

            const bool n  = isLandTerrain(terr[static_cast<std::size_t>(fy - 1) * FIELD_SZ + fx]);
            const bool s  = isLandTerrain(terr[static_cast<std::size_t>(fy + 1) * FIELD_SZ + fx]);
            const bool e  = isLandTerrain(terr[static_cast<std::size_t>(fy)     * FIELD_SZ + fx + 1]);
            const bool w  = isLandTerrain(terr[static_cast<std::size_t>(fy)     * FIELD_SZ + fx - 1]);
            const bool ne = isLandTerrain(terr[static_cast<std::size_t>(fy - 1) * FIELD_SZ + fx + 1]);
            const bool nw = isLandTerrain(terr[static_cast<std::size_t>(fy - 1) * FIELD_SZ + fx - 1]);
            const bool se = isLandTerrain(terr[static_cast<std::size_t>(fy + 1) * FIELD_SZ + fx + 1]);
            const bool sw = isLandTerrain(terr[static_cast<std::size_t>(fy + 1) * FIELD_SZ + fx - 1]);

            const int idx = autotile47(n, e, s, w, nw, ne, sw, se);
            const int variant = pickVariant(rawPx, rawPy, worldGrid::shoreSplineVariantCount);

            //   prefab 마스크 룩업: 자기 픽셀 내부 (localX, localY) 위치가 land이면 dirt 채움.
            //   variantCount=0이면 페이즈 2 적용 X (PNG 로드 실패한 경우 graceful fallback).
            if (worldGrid::shoreSplineVariantCount > 0 &&
                worldGrid::shoreSplineMask[static_cast<std::size_t>(variant)][static_cast<std::size_t>(idx)][static_cast<std::size_t>(localY) * worldGrid::SHORE_TILE_SIZE + localX])
            {
                const std::size_t tileIdx = static_cast<std::size_t>(dy) * SectorCoord::TILES + dx;
                plan.tiles[tileIdx].floor = itemID::dirt;
                plan.tiles[tileIdx].flags = TILE_FLAG_WALKABLE;
            }
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // 3) 광역 도로 폴리라인 페인트 — buildRoadNetwork가 생성한 도시간 폴리라인.
    //
    //   worldGen::activePolyLines가 nullptr이면 (월드젠 전 startArea 등) 스킵.
    //   각 폴리라인 segment를 8방향(카디널+45°) 두꺼운 라인으로 페인트.
    //
    //   페인트 룰:
    //     - 15타일 너비 asphalt만 (사이드워크 X — 도시 외부)
    //     - 각 segment를 primary-axis로 walk하면서 매 타일마다 15×15 square stamp.
    //       (정확한 perpendicular strip 알고리즘 대신 redundant blob — 단순/안전)
    //     - water 위에도 paint — 도로가 강/바다를 가로지름 (의도된 다리 표현)
    //
    //═══════════════════════════════════════════════════════════════════════
    if (worldGen::activePolyLines != nullptr)
    {
        const int tileMinX = sectorOriginTileX;
        const int tileMinY = sectorOriginTileY;
        const int tileMaxX = tileMinX + SectorCoord::TILES;
        const int tileMaxY = tileMinY + SectorCoord::TILES;

        constexpr int POLY_HALF = 7;   // 15타일 = 2*7+1, 너비 정확히 15

        auto paintPolyAsphalt = [&](int wtx, int wty) noexcept
        {
            if (wtx < tileMinX || wtx >= tileMaxX) return;
            if (wty < tileMinY || wty >= tileMaxY) return;
            const std::size_t idx = static_cast<std::size_t>(wty - tileMinY) * SectorCoord::TILES + (wtx - tileMinX);
            plan.tiles[idx].floor = itemID::blackAsphalt;
            plan.tiles[idx].flags = TILE_FLAG_WALKABLE;
        };

        auto stampBlob = [&](int cx, int cy) noexcept
        {
            for (int by = -POLY_HALF; by <= POLY_HALF; ++by)
            for (int bx = -POLY_HALF; bx <= POLY_HALF; ++bx)
                paintPolyAsphalt(cx + bx, cy + by);
        };

        // 한 segment (실타일 좌표 a→b) 를 두꺼운 라인으로 페인트.
        //   steps = primary axis 최대 거리. 매 step에서 보간 위치에 stamp.
        //   카디널/45°/일반 각도 모두 동일 처리. 45°에서도 stamp 겹쳐 끊김 없음.
        auto paintSegment = [&](const Point3& a, const Point3& b) noexcept
        {
            const int dx = b.x - a.x;
            const int dy = b.y - a.y;
            const int adx = std::abs(dx);
            const int ady = std::abs(dy);
            const int steps = std::max(adx, ady);
            if (steps == 0) { stampBlob(a.x, a.y); return; }

            // segment bbox cull — segment 단위로도 한 번 더.
            const int sMinX = std::min(a.x, b.x) - POLY_HALF;
            const int sMaxX = std::max(a.x, b.x) + POLY_HALF;
            const int sMinY = std::min(a.y, b.y) - POLY_HALF;
            const int sMaxY = std::max(a.y, b.y) + POLY_HALF;
            if (sMaxX < tileMinX || sMinX >= tileMaxX) return;
            if (sMaxY < tileMinY || sMinY >= tileMaxY) return;

            for (int s = 0; s <= steps; ++s)
            {
                const int x = a.x + static_cast<int>(static_cast<std::int64_t>(dx) * s / steps);
                const int y = a.y + static_cast<int>(static_cast<std::int64_t>(dy) * s / steps);
                stampBlob(x, y);
            }
        };

        for (const auto& poly : *worldGen::activePolyLines)
        {
            if (poly.verts.size() < 2) continue;

            // 폴리라인 전체 bbox cull
            int pMinX = poly.verts[0].x, pMaxX = poly.verts[0].x;
            int pMinY = poly.verts[0].y, pMaxY = poly.verts[0].y;
            for (const auto& v : poly.verts)
            {
                if (v.x < pMinX) pMinX = v.x;
                if (v.x > pMaxX) pMaxX = v.x;
                if (v.y < pMinY) pMinY = v.y;
                if (v.y > pMaxY) pMaxY = v.y;
            }
            if (pMaxX + POLY_HALF < tileMinX) continue;
            if (pMinX - POLY_HALF >= tileMaxX) continue;
            if (pMaxY + POLY_HALF < tileMinY) continue;
            if (pMinY - POLY_HALF >= tileMaxY) continue;

            for (std::size_t i = 1; i < poly.verts.size(); ++i)
            {
                paintSegment(poly.verts[i - 1], poly.verts[i]);
            }
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // 4) 도시 CityPlan 소비 — CityPlan.tiles의 floor/wall/prop을 PaintCell/sparse에 페인트.
    //
    //   이 섹터 근처 도시 각각의 CityPlan을 CityPlanCache::getOrCompute로 조회 —
    //   miss면 호출 스레드(여기선 ProcGenWorker)에서 즉시 buildCityPlan 계산 후 캐시.
    //
    //   각 CityTile 라우팅:
    //     - t.pos.z == sc.z + floor/wall: 본 z층 dense PaintCell에 페인트
    //     - t.pos.z != sc.z + floor/wall: sparse skyTiles에 push (다리 deck 등 다른 z층)
    //     - t.prop != 0: 모든 z를 sparse props에 push (createChunk가 createProp 호출)
    //
    //   섹터 단일 조회 정책: createChunk는 chunkZ가 무엇이든 sc.z=0 섹터 1개만 조회.
    //   본 sector(z=0)에 모든 z의 sparse 데이터가 모이므로 z=±1 청크도 같은 SectorPlan
    //   에서 자기 z 데이터를 추출 → z층마다 따로 14.7M 평면 안 깔아도 됨.
    //
    //   클리핑: 각 CityTile을 자기 섹터 타일 범위로 클립 (도시가 섹터 가로질러도 OK).
    //═══════════════════════════════════════════════════════════════════════
    {
        const Point3 sectorCenter{
            sectorOriginTileX + SectorCoord::TILES / 2,
            sectorOriginTileY + SectorCoord::TILES / 2,
            sc.z };

        for (city::CityId id : citiesInRangeOf(sectorCenter))
        {
            const CityPlan& cp = CityPlanCache::ins().getOrCompute(id, seed);
            for (const CityTile& t : cp.tiles)
            {
                const int dx = t.pos.x - sectorOriginTileX;
                const int dy = t.pos.y - sectorOriginTileY;
                const bool inSectorXY = (dx >= 0 && dx < SectorCoord::TILES
                                      && dy >= 0 && dy < SectorCoord::TILES);
                if (!inSectorXY) continue;

                if (t.pos.z == sc.z)
                {
                    if (t.floor || t.wall)
                    {
                        PaintCell& cell = plan.tiles[static_cast<std::size_t>(dy) * SectorCoord::TILES + dx];
                        if (t.floor) cell.floor = t.floor;
                        if (t.wall)  cell.wall  = t.wall;
                    }
                }
                else
                {
                    if (t.floor || t.wall)
                    {
                        plan.skyTiles.push_back(SectorSkyTile{
                            .pos   = t.pos,
                            .floor = t.floor,
                            .wall  = t.wall,
                            .flags = static_cast<std::uint8_t>(t.floor ? TILE_FLAG_WALKABLE : 0),
                            });
                    }
                }

                if (t.prop) plan.props.push_back(SectorProp{ .pos = t.pos, .itemId = t.prop });
            }
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // TODO 향후 단계 (모두 본 함수에 누적)
    //   - 인카운터 사이트 좌표 (Land 픽셀 위에 결정론 배치)
    //   - 폴리라인 주변 국도 분기 — 1티어 도로에서 갈라지는 마이너 도로망
    //   - CityPlan에 itemStack/vehicle/prop/entity 스폰 레인 추가
    //   - Bridge 후처리 보강 — 폴리라인↔수계 교차 시 다리 텍스처
    //═══════════════════════════════════════════════════════════════════════

    return plan;
}
