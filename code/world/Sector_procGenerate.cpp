module Sector;

import std;
import util;
import constVar;
import worldGrid;
import cityLayout;

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
                cell.floor = itemID::paver;   //도시 기본 보도블럭 (TODO: BCP layout이 도로/건물 페인트)
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
    // 3) 도시 layout 소비 — buildCityLayouts가 결정한 도로/사이드워크 페인트.
    //
    //   cityLayout::activeLayouts가 nullptr이면 (월드젠 전 startArea 등) 스킵.
    //   각 layout의 bbox와 섹터 tile 범위가 교차할 때만 처리.
    //
    //   페인트 룰:
    //     - asphalt 15타일 + sidewalk 3타일×2 = 총 21타일 너비 밴드
    //     - Interior/Boundary: 대칭 (sidewalk + asphalt + sidewalk)
    //     - Coast/Riverside: 단방향 (asphalt 15 + sidewalk 3, interiorSide 방향으로)
    //     - 다리: 5타일 너비 asphalt만 (사이드워크 없음)
    //     - Entry point: yellowAsphalt 마커 5×5 (시각 확인용)
    //═══════════════════════════════════════════════════════════════════════
    if (cityLayout::activeLayouts != nullptr)
    {
        const int tileMinX = sectorOriginTileX;
        const int tileMinY = sectorOriginTileY;
        const int tileMaxX = tileMinX + SectorCoord::TILES;   // exclusive
        const int tileMaxY = tileMinY + SectorCoord::TILES;

        // 도로 밴드 치수 (실타일 단위).
        //   대칭 도로(Interior/Boundary): 3 paver + 15 asphalt + 3 paver = 21타일, 중심 정렬.
        //   비대칭 도로(Coast/Riverside): 같은 21타일이지만 직사각형 안쪽으로 전부 시프트 —
        //     변 바로 안쪽 3 paver (강변/해안 인도) + 15 asphalt + 안쪽 3 paver.
        //     아스팔트는 변에서 3타일 안쪽에서 시작 → water 픽셀 침범 0.
        //   다리: 21타일 풀 폭 (사이드워크 포함). water 위를 가로지르는 게 본분.
        constexpr int ROAD_ASPHALT      = 15;
        constexpr int ROAD_SIDEWALK     =  3;
        constexpr int ROAD_HALF         = (ROAD_ASPHALT + 2 * ROAD_SIDEWALK) / 2;  // 10
        constexpr int ROAD_ASPHALT_HALF =  ROAD_ASPHALT / 2;                       //  7
        constexpr int ROAD_BAND         = ROAD_ASPHALT + 2 * ROAD_SIDEWALK;        // 21
        constexpr int BRIDGE_HALF       = ROAD_HALF;                               // 10 → 21타일 폭

        //  섹터-로컬 (dx, dy) 타일에 페인트.
        //   paintAsphalt: 무조건 덮어쓰기 — 교차점에서 아스팔트가 우선.
        //   paintPaver  : 기존이 이미 blackAsphalt면 skip — 도로 교차 시 paver가
        //                 아스팔트를 잘라먹지 않게 함. (도로 A asphalt × 도로 B paver →
        //                 paint 순서 무관하게 asphalt 살아남음)
        auto paintAsphalt = [&](int wtx, int wty) noexcept
        {
            if (wtx < tileMinX || wtx >= tileMaxX) return;
            if (wty < tileMinY || wty >= tileMaxY) return;
            const std::size_t idx = static_cast<std::size_t>(wty - tileMinY) * SectorCoord::TILES + (wtx - tileMinX);
            plan.tiles[idx].floor = itemID::blackAsphalt;
            plan.tiles[idx].flags = TILE_FLAG_WALKABLE;
        };
        auto paintPaver = [&](int wtx, int wty) noexcept
        {
            if (wtx < tileMinX || wtx >= tileMaxX) return;
            if (wty < tileMinY || wty >= tileMaxY) return;
            const std::size_t idx = static_cast<std::size_t>(wty - tileMinY) * SectorCoord::TILES + (wtx - tileMinX);
            if (plan.tiles[idx].floor == itemID::blackAsphalt) return;   // 아스팔트 위에 paver X
            plan.tiles[idx].floor = itemID::paver;
            plan.tiles[idx].flags = TILE_FLAG_WALKABLE;
        };
        auto paintMarker = [&](int wtx, int wty) noexcept   // 진입점 마커(yellow). 도로 위에 덮어쓰기 OK.
        {
            if (wtx < tileMinX || wtx >= tileMaxX) return;
            if (wty < tileMinY || wty >= tileMaxY) return;
            const std::size_t idx = static_cast<std::size_t>(wty - tileMinY) * SectorCoord::TILES + (wtx - tileMinX);
            plan.tiles[idx].floor = itemID::yellowAsphalt;
            plan.tiles[idx].flags = TILE_FLAG_WALKABLE;
        };

        //  하나의 road segment를 페인트. a→b는 cardinal(수평 또는 수직).
        //   interiorSide=None이면 대칭, 아니면 그 방향으로만 사이드워크.
        auto paintRoad = [&](const cityLayout::CityRoadSegment& s) noexcept
        {
            const bool horizontal = (s.a.y == s.b.y);
            const int  lo  = horizontal ? std::min(s.a.x, s.b.x) : std::min(s.a.y, s.b.y);
            const int  hi  = horizontal ? std::max(s.a.x, s.b.x) : std::max(s.a.y, s.b.y);
            const int  perpC = horizontal ? s.a.y : s.a.x;   // 수직 좌표(밴드 중심 기준)

            // 밴드 perpendicular 범위 결정 — paver 양쪽 + asphalt 중앙.
            //   layout: [swLo) [asphalt) [swHi)  →  총 21타일.
            int aLo, aHi, swLoLo, swLoHi, swHiLo, swHiHi;
            if (s.interiorSide == cityLayout::Dir4::None)
            {
                // 대칭: 3 paver + 15 asphalt + 3 paver, segment 중심
                aLo   = perpC - ROAD_ASPHALT_HALF;
                aHi   = perpC + ROAD_ASPHALT_HALF + 1;          // exclusive (15 = 7+1+7)
                swLoLo = aLo - ROAD_SIDEWALK; swLoHi = aLo;
                swHiLo = aHi;                 swHiHi = aHi + ROAD_SIDEWALK;
            }
            else
            {
                // Coast/Riverside: 21타일 밴드를 *직사각형 안으로 전부* 시프트.
                //   외측 paver 3 (변 바로 안쪽, 강변/해안 인도) + asphalt 15 + 내측 paver 3.
                //   asphalt는 변에서 3타일 안쪽에서 시작 → 절대 water 침범 없음.
                const bool interiorIsHi =
                    (horizontal && s.interiorSide == cityLayout::Dir4::S) ||
                    (!horizontal && s.interiorSide == cityLayout::Dir4::E);
                if (interiorIsHi)
                {
                    // 변(perpC)에서 +방향으로 21타일 전체
                    swLoLo = perpC;                                  swLoHi = perpC + ROAD_SIDEWALK;       // 강변 paver
                    aLo    = perpC + ROAD_SIDEWALK;                  aHi    = aLo + ROAD_ASPHALT;          // asphalt 15
                    swHiLo = aHi;                                    swHiHi = aHi + ROAD_SIDEWALK;         // 내측 paver
                }
                else
                {
                    // 변(perpC)에서 -방향으로 21타일 전체
                    swHiLo = perpC - ROAD_SIDEWALK;                  swHiHi = perpC;                       // 강변 paver
                    aHi    = perpC - ROAD_SIDEWALK;                  aLo    = aHi - ROAD_ASPHALT;          // asphalt 15
                    swLoLo = aLo - ROAD_SIDEWALK;                    swLoHi = aLo;                         // 내측 paver
                }
            }

            // 길이 방향으로 walking, 각 perpC 슬라이스마다 asphalt/sidewalk 페인트.
            // paver는 paintPaver(아스팔트 위 skip), asphalt는 paintAsphalt(무조건).
            for (int along = lo; along < hi; ++along)
            {
                for (int p = swLoLo; p < swLoHi; ++p)
                {
                    if (horizontal) paintPaver(along, p);
                    else            paintPaver(p, along);
                }
                for (int p = aLo; p < aHi; ++p)
                {
                    if (horizontal) paintAsphalt(along, p);
                    else            paintAsphalt(p, along);
                }
                for (int p = swHiLo; p < swHiHi; ++p)
                {
                    if (horizontal) paintPaver(along, p);
                    else            paintPaver(p, along);
                }
            }
        };

        //  다리: 21타일 풀 도로 폭 (3 paver + 15 asphalt + 3 paver) 직선.
        //   water 위를 덮음 — 다리의 본분이 강/바다 가로지르기.
        auto paintBridge = [&](const cityLayout::CityBridge& br) noexcept
        {
            const bool horizontal = (br.a.y == br.b.y);
            const int  lo  = horizontal ? std::min(br.a.x, br.b.x) : std::min(br.a.y, br.b.y);
            const int  hi  = horizontal ? std::max(br.a.x, br.b.x) : std::max(br.a.y, br.b.y);
            const int  perpC = horizontal ? br.a.y : br.a.x;
            for (int along = lo; along <= hi; ++along)
            {
                for (int dp = -BRIDGE_HALF; dp <= BRIDGE_HALF; ++dp)
                {
                    // perpC 기준 ±7는 asphalt, |dp|>7은 paver. 대칭 21타일.
                    if (std::abs(dp) <= ROAD_ASPHALT_HALF)
                    {
                        if (horizontal) paintAsphalt(along, perpC + dp);
                        else            paintAsphalt(perpC + dp, along);
                    }
                    else
                    {
                        if (horizontal) paintPaver(along, perpC + dp);
                        else            paintPaver(perpC + dp, along);
                    }
                }
            }
        };

        //  진입점 마커: 5×5 yellowAsphalt — 자동 도로 그래프 봉합 단계 이전 시각 확인용.
        auto paintEntryMarker = [&](const cityLayout::CityEntryPoint& ep) noexcept
        {
            for (int dy = -2; dy <= 2; ++dy)
                for (int dx = -2; dx <= 2; ++dx)
                    paintMarker(ep.tile.x + dx, ep.tile.y + dy);
        };

        // bbox cull 후 페인트.
        for (const auto& layout : *cityLayout::activeLayouts)
        {
            if (layout.empty()) continue;
            // 도로 밴드/마커가 bbox 바깥까지 ROAD_HALF만큼 비져나갈 수 있으니 약간 여유.
            constexpr int CULL_MARGIN = ROAD_HALF + 4;
            if (layout.bboxMaxTile.x + CULL_MARGIN <= tileMinX) continue;
            if (layout.bboxMinTile.x - CULL_MARGIN >= tileMaxX) continue;
            if (layout.bboxMaxTile.y + CULL_MARGIN <= tileMinY) continue;
            if (layout.bboxMinTile.y - CULL_MARGIN >= tileMaxY) continue;

            for (const auto& seg : layout.roads)     paintRoad(seg);
            for (const auto& br  : layout.bridges)   paintBridge(br);
            for (const auto& ep  : layout.entries)   paintEntryMarker(ep);
        }
    }

    //═══════════════════════════════════════════════════════════════════════
    // TODO 향후 단계 (모두 본 함수에 누적)
    //   4) 인카운터 사이트 좌표 (Land 픽셀 위에 결정론 배치)
    //   5) T1 도로 폴리라인이 sector 통과 시 분기 국도 생성
    //   6) 도시 layout BCP 본격화 — 블록·건물 prefab 페인트
    //═══════════════════════════════════════════════════════════════════════

    return plan;
}
