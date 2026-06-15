module CityPlan;

import std;
import util;
import constVar;
import worldGen;
import worldGrid;
import Lot;

// ════════════════════════════════════════════════════════════════════════
// CityPlan_build.cpp — buildCityPlan 구현.
//
//   알고리즘: 픽셀(=24타일) 단위 도시 절차생성. 단계 누적식 (procGenerate 패턴).
//     1. 도시 영역 terrain 박스 로드 (+1px 마진) ▶ cityPixelAt
//     2. 외부 도로 진입점 → 도시 가장자리 픽셀에 강제 도로 비트 (lock)
//     3. 강가/해안 픽셀에 평행 도로 비트 (1px 평행 충돌 회피, lock)
//     4. 도시 내부 다트던지기로 건물 픽셀 배치 (Phase 1 무작위 + Phase 2 결정적
//        anti-2x2-road 채움) — 매 가설 배치마다 연결성 BFS 검증.
//     5. 고립 건물 도로 확장 — 사방이 건물인 1x1 건물의 이웃 1x1을 빈 셀로 되돌려
//        stage 6이 도로화하게 위임 (Fixed-point, 2x2 도로 사전 차단).
//     6. 비건물 도시 셀을 도로 픽셀로 변환 (4방향 이웃과 비트 결합).
//     7. degree==1 도로를 일직선(degree==2)으로 대칭화.
//     8. CityRiver/CitySea 다리 결정 — roads[]에 NS/EW + isBridge 박기, 양 강변 출구 추가.
//     9. 도로 픽셀 → Lot 페인트 — Street/Bridge 라우팅(isBridge 분기), LotResult를 plan.tiles에 push.
//     Ex. 도로 비트를 polyline 세그먼트로 환산 (Map.ixx 디버그 오버레이 — stage 9 lot 검증용).
//
//   산출물: plan.tiles, plan.segments(도로), plan.bridges, plan.buildings(debug).
//
//   주의: procGenerate(ProcGenWorker 스레드)가 CityPlanCache::getOrCompute 경유로
//   본 함수를 호출 — prt는 스레드 안전 보장 X. 디버그 출력 용도로만 사용.
// ════════════════════════════════════════════════════════════════════════

//LotResult를 절대좌표 (originX,originY,baseZ) 기준으로 plan에 블릿.
//  stage 9(도로 픽셀)·stage 10(건물 그룹) 공용 — Lot 로컬 좌표를 절대 Point3로 옮겨
//  tiles + 4개 spawn 채널에 누적한다. r은 spawn 페이로드를 move out 하므로 소비됨.
static void blitLotResult(CityPlan& plan, LotResult& r, int originX, int originY, int baseZ)
{
    //LotResult.planes는 sparse z map (쓰여진 z만 존재). itemID::none 슬롯은 스킵.
    for (const auto& [zLayer, plane] : r.planes)
    {
        for (int y = 0; y < r.h; ++y)
            for (int x = 0; x < r.w; ++x)
            {
                const std::size_t pi = static_cast<std::size_t>(y) * r.w + x;
                const int f = plane.floor[pi];
                const int w = plane.wall[pi];
                const int p = plane.prop[pi];
                if (f == itemID::none && w == itemID::none && p == itemID::none) continue;

                plan.tiles.push_back(CityTile{
                    .pos   = Point3{ originX + x, originY + y, baseZ + zLayer },
                    .floor = f,
                    .wall  = w,
                    .prop  = p,
                });
            }
    }

    //── spawn 채널 — Lot 로컬 좌표를 절대 Point3로 옮겨 plan에 누적
    for (auto& s : r.itemStacks)
    {
        plan.itemStacks.push_back(CityItemStack{
            .pos   = Point3{ originX + s.x, originY + s.y, baseZ + s.z },
            .items = std::move(s.items),
        });
    }
    for (const auto& m : r.monsters)
    {
        plan.monsters.push_back(CityMonster{
            .pos        = Point3{ originX + m.x, originY + m.y, baseZ + m.z },
            .entityCode = m.entityCode,
        });
    }
    for (auto& v : r.vehicles)
    {
        plan.vehicles.push_back(CityVehicle{
            .pos  = Point3{ originX + v.x, originY + v.y, baseZ + v.z },
            .plan = std::move(v.plan),
        });
    }
    //prop 인스턴스화 후 후처리로 ItemPocket 채울 데이터 — XY만 절대화.
    for (auto& c : r.propContents)
    {
        plan.propContents.push_back(CityPropContents{
            .pos   = Point3{ originX + c.x, originY + c.y, baseZ + c.z },
            .items = std::move(c.items),
        });
    }
}

CityPlan buildCityPlan(city::CityId id, std::uint64_t seed)
{
    CityPlan plan{ id };

    prt(L"[CityPlan] buildCityPlan id=%u seed=%llu tiles=%zu\n",
        static_cast<unsigned>(id), static_cast<std::uint64_t>(seed),
        plan.tiles.size());


    const worldGen::CityNode& node = (*worldGen::activeCities)[static_cast<std::uint32_t>(id)];
   
    if (node.rectangles.empty()) return plan;

    std::mt19937_64 rng{ seed ^ (static_cast<std::uint64_t>(id) * 0x9E3779B97F4A7C15ULL) };
    auto localRandom = [&](int a, int b) { return std::uniform_int_distribution<int>{a, b}(rng); };

    //══════════════════════════════════════════════════════════════════
    // 1. 도시의 픽셀 데이터를 로컬 변수에 저장 (주변픽셀 감지 위해 +1px 마진) ▶ cityPixelAt
    //══════════════════════════════════════════════════════════════════

    int minX = node.rectangles[0].px, minY = node.rectangles[0].py;
    int maxX = node.rectangles[0].x1(), maxY = node.rectangles[0].y1();
    for (const auto& r : node.rectangles)
    {
        minX = std::min(minX, r.px);   minY = std::min(minY, r.py);
        maxX = std::max(maxX, r.x1()); maxY = std::max(maxY, r.y1());
    }

    const int cityWidth = maxX - minX;
    const int cityHeight = maxY - minY;

    //도시 주변 픽셀도 알 수 있게 1픽셀 마진을 가진 박스
    const int patchPxX = minX - 1; 
    const int patchPxY = minY - 1;
    const int patchW = cityWidth + 2;
    const int patchH = cityHeight + 2;

    std::vector<worldGrid::Terrain> cityTerrainBox(static_cast<std::size_t>(patchW) * patchH);

    for (int dy = 0; dy < patchH; ++dy)
        for (int dx = 0; dx < patchW; ++dx)
            cityTerrainBox[static_cast<std::size_t>(dy) * patchW + dx] = worldGrid::worldPixel(patchPxX + dx, patchPxY + dy);

    auto cityPixelAt = [&](worldGrid::PixelCoord p) -> worldGrid::Terrain {
        return cityTerrainBox[static_cast<std::size_t>(p.y - patchPxY) * patchW + (p.x - patchPxX)];
        };


    //══════════════════════════════════════════════════════════════════
    // 2. 도시의 진입점 강제 도로 설정
    //══════════════════════════════════════════════════════════════════


    struct RoadPixel
    {
        enum class Dir : std::uint8_t { NORTH = 1, EAST = 2, SOUTH = 4, WEST = 8 };

        std::uint8_t openBits = 0;
        std::uint8_t lockBits = 0;
        bool         isBridge = false;   //다리 결정 stage가 강 픽셀에 박음. stage 8 lot 라우팅 분기 키.
        BridgeRole   bridgeRole = BridgeRole::Single;   //N픽셀 강 세그먼트 위치 (isBridge일 때만 유효).

        bool isOpen  (Dir d) const { return openBits & std::uint8_t(d); }
        bool isLocked(Dir d) const { return lockBits & std::uint8_t(d); }
        int  degree () const { return std::popcount(openBits); }

        void open (Dir d) { if (!isLocked(d)) openBits |=  std::uint8_t(d); }
        void close(Dir d) { if (!isLocked(d)) openBits &= ~std::uint8_t(d); }
        void lock (Dir d) { lockBits |= std::uint8_t(d); }
    };

    //patch 좌표계 (마진 +1px 포함) — cityTerrainBox와 동일 인덱싱. 가장자리 픽셀의 4방향
    //이웃 검사 시 마진 픽셀(빈 RoadPixel)이 자동으로 자리 잡아 OOB 회피.
    std::vector<RoadPixel> roads(static_cast<std::size_t>(patchW) * patchH);

    auto roadIdx = [&](int px, int py) -> std::size_t {
        return static_cast<std::size_t>(py - patchPxY) * patchW + (px - patchPxX);
        };

    if (worldGen::activePolyLines != nullptr)
    {
        for (const worldGen::RoadPolyLine& line : *worldGen::activePolyLines)
        {
            if (line.verts.size() < 2) continue;

            for (int endIdx = 0; endIdx < 2; ++endIdx)
            {
                const Point3 endpoint = (endIdx == 0) ? line.verts.front() : line.verts.back();
                const Point3 adjacent = (endIdx == 0) ? line.verts[1] : line.verts[line.verts.size() - 2];

                const int epx = (endpoint.x - TILE_BASE_X) / TILE_PER_PIXEL;
                const int epy = (endpoint.y - TILE_BASE_Y) / TILE_PER_PIXEL;

                bool inCity = false;
                for (const city::CityRect& r : node.rectangles)
                {
                    if (epx >= r.px && epx < r.x1() && epy >= r.py && epy < r.y1())
                    {
                        inCity = true;
                        break;
                    }
                }
                if (!inCity) continue;

                const int dx = endpoint.x - adjacent.x;
                const int dy = endpoint.y - adjacent.y;
                const std::size_t idx = roadIdx(epx, epy);

                //외부 도로와 연결되게 맞방향으로 도로를 설치
                RoadPixel::Dir dir;
                if (std::abs(dx) >= std::abs(dy)) dir = (dx < 0) ? RoadPixel::Dir::EAST  : RoadPixel::Dir::WEST;
                else                              dir = (dy < 0) ? RoadPixel::Dir::SOUTH : RoadPixel::Dir::NORTH;

                roads[idx].open(dir);
                roads[idx].lock(dir);   // 외부 도로 연결 면은 후속 단계가 못 끔
            }
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 3. 강가 강제 도로 형성
    //══════════════════════════════════════════════════════════════════

    auto isWater = [](worldGrid::Terrain t) {
        return t == worldGrid::Terrain::River
            || t == worldGrid::Terrain::Sea
            || t == worldGrid::Terrain::Lake
            || t == worldGrid::Terrain::CityRiver
            || t == worldGrid::Terrain::CitySea;
        };

    //도시 마킹된 지면 픽셀만 — CityRiver/CitySea는 물이라 자동 배제됨.
    //강이 도시 경계를 넘어 흐르는 케이스에서 도시 바깥 일반 Land가 강변으로
    //잡혀 도로가 깔리는 것을 막기 위해 도입.
    auto isCityLandPixel = [](worldGrid::Terrain t) {
        return t == worldGrid::Terrain::CityZone
            || t == worldGrid::Terrain::CityCenter;
        };

    //patch-local 좌표의 도시 지면 여부 — 패치 밖이면 false (마진 너머 = 도시 아님)
    auto isCityLandAtLocal = [&](int lx, int ly) -> bool {
        if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
        return isCityLandPixel(cityTerrainBox[static_cast<std::size_t>(ly) * patchW + lx]);
        };

    for (int y = 0; y < patchH; ++y)
        for (int x = 0; x < patchW; ++x)
        {
            const worldGrid::PixelCoord p{ patchPxX + x, patchPxY + y, node.center.z };
            if (isWater(cityPixelAt(p)))
            {
                for (int dir = 0; dir <4; dir++)
                {
                    int dx = 0 , dy = 0;
                    if (dir == 0) dx = +1;
                    else if (dir == 1) dy = -1;
                    else if (dir == 2) dx = -1;
                    else if (dir == 3) dy = +1;

                    //pNearbyNearby(2px out)가 패치 밖이면 OOB → 스킵 (마진은 +1px만 확보됨)
                    //이걸 통과하면 pNearby(1px out)는 자동으로 패치 안
                    const int nnx = x + dx * 2;
                    const int nny = y + dy * 2;
                    if (nnx < 0 || nnx >= patchW || nny < 0 || nny >= patchH) continue;

                    const worldGrid::PixelCoord pNearby{ patchPxX + x + dx, patchPxY + y + dy, node.center.z };
                    if (isCityLandPixel(cityPixelAt(pNearby)))
                    {
                        const std::size_t nearbyIdx = roadIdx(pNearby.x, pNearby.y);
                        const worldGrid::PixelCoord pNearbyNearby{ patchPxX + x + dx * 2, patchPxY + y + dy * 2, node.center.z };
                        const std::size_t nearbyNearbyIdx = roadIdx(pNearbyNearby.x, pNearbyNearby.y);

                        //강변에 강과 평행한 방향(강 가장자리를 따라가는 축)으로 도로 개방
                        //물→강변 벡터가 수평(dx≠0)이면 강은 수직 → N/S 도로, 그 외엔 강이 수평 → E/W 도로
                        //n1dx/n1dy는 d1 방향의 단위 오프셋, d2는 항상 반대 (-n1dx, -n1dy)
                        RoadPixel::Dir d1, d2;
                        int n1dx, n1dy;
                        if (dx != 0) { d1 = RoadPixel::Dir::NORTH; d2 = RoadPixel::Dir::SOUTH; n1dx = 0;  n1dy = -1; }
                        else         { d1 = RoadPixel::Dir::EAST;  d2 = RoadPixel::Dir::WEST;  n1dx = +1; n1dy = 0;  }

                        //개방 조건 — 두 게이트 동시 통과해야 도로 오픈:
                        //  (a) 도시 지면 체크 — d 방향 이웃이 도시 지면이어야 함. 사방이 바다인
                        //      고립 도시에서 강변 도로가 바다로 직행하는 케이스 차단.
                        //  (b) 평행 충돌 체크 — pNearbyNearby에 같은 방향 도로가 이미 있으면 차단.
                        //      24타일(1픽셀) 간격 평행 도로 사이에 0폭 블록이 생겨 건물 배치 불가.
                        //
                        //개방 시 lock도 같이 — 강변/해안 도로는 stage 5 격자 알고리즘이 못 끔.
                        //그리고 인접 1px 평행 lane 회피 신호로도 작동 (locked bit가 곧 "여기 도로 있음")
                        const int lbx = x + dx;  // 강변 픽셀 local 좌표
                        const int lby = y + dy;
                        if (isCityLandAtLocal(lbx + n1dx, lby + n1dy) && roads[nearbyNearbyIdx].isOpen(d1) == false) { roads[nearbyIdx].open(d1); roads[nearbyIdx].lock(d1); }
                        if (isCityLandAtLocal(lbx - n1dx, lby - n1dy) && roads[nearbyNearbyIdx].isOpen(d2) == false) { roads[nearbyIdx].open(d2); roads[nearbyIdx].lock(d2); }
                    }
                }
            }
        }


    //══════════════════════════════════════════════════════════════════
    // 4. 도시 내부 건물 픽셀 다트던지기
    //══════════════════════════════════════════════════════════════════
    //   Phase 1 — 무작위 좌표 + 1x1/2x1/1x2/2x2 블록 가설 배치 → checkPoints 전셀의
    //             점유/지면/도로 검사 + 연결성 BFS → 통과 시 확정, 실패면 ++failStreak.
    //             failStreak >= MAX_FAIL_STREAK 이면 Phase 2로 이행.
    //   Phase 2 — 결정적 채움. "미래 도로 2x2" (= 기존 도로 OR 빈 도시 셀의 4셀 결합)을
    //             검출, 빈 코너에 1x1 박아 깨뜨림. 4코너 모두 실패 시 다음 2x2,
    //             전체 스캔 후에도 못 박으면 종료.
    //   invariant — 최종 출력에 2x2 도로 잔존 금지 (Phase 2가 강제). 단, stage 2/3
    //             도로 4셀로만 된 2x2는 buildable 셀 없어 예외 (구조적 한계).
    //   연결성    — 매 가설 배치마다 BFS — 끊는 배치는 거부. CityRiver/CitySea는
    //             미래 다리/강변·해안도로 가정으로 traversable 처리.

    struct BuildingPixel
    {
        worldGrid::PixelCoord coord;
        int memberBuildingIndex = -1;
    };

    int buildingIndexCursor = 0;
    std::vector<BuildingPixel> buildings;
    std::vector<int> buildingIndexGrid(static_cast<std::size_t>(patchW) * patchH, -1);

    //── 셀 판정 헬퍼 (patch-local 좌표, OOB는 모두 false 반환) ──
    //빈 도시 픽셀: 도시 지면 & 건물 미점유 & 도로 미부설. Phase 1 allFree 검사용.
    auto isEmptyCityPixelLocal = [&](int lx, int ly) -> bool {
        if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
        const std::size_t idx = static_cast<std::size_t>(ly) * patchW + lx;
        const worldGrid::Terrain t = cityTerrainBox[idx];
        if (t != worldGrid::Terrain::CityZone && t != worldGrid::Terrain::CityCenter) return false;
        if (buildingIndexGrid[idx] != -1) return false;
        if (roads[idx].openBits != 0) return false;
        return true;
        };

    //미래 도로 셀: 기존 도로 비트 있음 OR 빈 도시 지면. stage 6 후 도로가 될 모든 셀.
    //Phase 2가 "stage 3 강변도로 + 빈 셀" mixed 2x2도 잡아야 하기 때문에 도입.
    auto isFutureRoadCell = [&](int lx, int ly) -> bool {
        if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
        const std::size_t idx = static_cast<std::size_t>(ly) * patchW + lx;
        if (roads[idx].openBits != 0) return true;
        const worldGrid::Terrain t = cityTerrainBox[idx];
        if (t != worldGrid::Terrain::CityZone && t != worldGrid::Terrain::CityCenter) return false;
        return buildingIndexGrid[idx] == -1;
        };

    //연결성 BFS용 traversable: 미래 도로 + CityRiver/CitySea (다리/강변·해안도로 가정).
    auto isTraversableCell = [&](int lx, int ly) -> bool {
        if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
        const std::size_t idx = static_cast<std::size_t>(ly) * patchW + lx;
        if (roads[idx].openBits != 0) return true;
        const worldGrid::Terrain t = cityTerrainBox[idx];
        if (t == worldGrid::Terrain::CityRiver || t == worldGrid::Terrain::CitySea) return true;
        if (t != worldGrid::Terrain::CityZone && t != worldGrid::Terrain::CityCenter) return false;
        return buildingIndexGrid[idx] == -1;
        };

    //모든 traversable 셀이 4-카디널로 단일 연결 컴포넌트인지 BFS — 건물 가설 배치 후 호출.
    //끊기면 false → 호출자가 롤백. O(patchW*patchH)를 매 다트마다 무는 건 부담스럽지만
    //구현 우선 — incremental connectivity (union-find with rollback 등)는 후속 최적화 여지.
    std::vector<std::uint8_t> bfsVisited(static_cast<std::size_t>(patchW) * patchH, 0);
    std::vector<std::pair<int, int>> bfsStack;
    bfsStack.reserve(64);

    auto allTraversableConnected = [&]() -> bool {
        std::fill(bfsVisited.begin(), bfsVisited.end(), std::uint8_t{ 0 });

        int seedLx = -1, seedLy = -1;
        for (int ly = 0; ly < patchH && seedLx < 0; ++ly)
            for (int lx = 0; lx < patchW && seedLx < 0; ++lx)
                if (isTraversableCell(lx, ly)) { seedLx = lx; seedLy = ly; }
        if (seedLx < 0) return true;

        bfsStack.clear();
        bfsStack.emplace_back(seedLx, seedLy);
        bfsVisited[static_cast<std::size_t>(seedLy) * patchW + seedLx] = 1;
        while (!bfsStack.empty())
        {
            auto [cx, cy] = bfsStack.back();
            bfsStack.pop_back();
            constexpr int dxs[4] = {  0,  0, +1, -1 };
            constexpr int dys[4] = { -1, +1,  0,  0 };
            for (int d = 0; d < 4; ++d)
            {
                const int nx = cx + dxs[d];
                const int ny = cy + dys[d];
                if (nx < 0 || nx >= patchW || ny < 0 || ny >= patchH) continue;
                const std::size_t nidx = static_cast<std::size_t>(ny) * patchW + nx;
                if (bfsVisited[nidx]) continue;
                if (!isTraversableCell(nx, ny)) continue;
                bfsVisited[nidx] = 1;
                bfsStack.emplace_back(nx, ny);
            }
        }
        for (int ly = 0; ly < patchH; ++ly)
            for (int lx = 0; lx < patchW; ++lx)
                if (isTraversableCell(lx, ly)
                    && !bfsVisited[static_cast<std::size_t>(ly) * patchW + lx]) return false;
        return true;
        };

    //Phase 1 무작위 다트가 N번 연속 실패하면 saturation 도달로 보고 Phase 2(결정적 채움)로 이행.
    //작을수록 도로 비율 ↑ (Phase 1 빨리 끊김 → Phase 2 최소 채움 → 채커보드 패턴 근접).
    int failStreak = 0;
    constexpr int MAX_FAIL_STREAK = 50;

    while (true)
    {
        //═════════ Phase 2: 미래 도로 2x2 검출 + 빈 코너에 1x1 ═════════
        if (failStreak >= MAX_FAIL_STREAK)
        {
            bool placed = false;
            for (int ly = 0; ly < patchH - 1 && !placed; ++ly)
                for (int lx = 0; lx < patchW - 1 && !placed; ++lx)
                {
                    if (!isFutureRoadCell(lx,     ly    )) continue;
                    if (!isFutureRoadCell(lx + 1, ly    )) continue;
                    if (!isFutureRoadCell(lx,     ly + 1)) continue;
                    if (!isFutureRoadCell(lx + 1, ly + 1)) continue;

                    //4코너 각각 시도 — 기존 도로 셀은 빌딩 못 박으므로 빈 셀만 후보.
                    const int candidates[4][2] = {
                        { lx,     ly     },
                        { lx + 1, ly     },
                        { lx,     ly + 1 },
                        { lx + 1, ly + 1 },
                    };
                    for (int c = 0; c < 4 && !placed; ++c)
                    {
                        const int cx = candidates[c][0];
                        const int cy = candidates[c][1];
                        const std::size_t cidx = static_cast<std::size_t>(cy) * patchW + cx;
                        if (roads[cidx].openBits != 0) continue;        //기존 도로 위에 건물 X
                        if (buildingIndexGrid[cidx] != -1) continue;    //이미 건물 (방어적)

                        buildingIndexGrid[cidx] = buildingIndexCursor;
                        if (allTraversableConnected())
                        {
                            const worldGrid::PixelCoord cp{ patchPxX + cx, patchPxY + cy, 0 };
                            buildings.push_back(BuildingPixel{ .coord = cp, .memberBuildingIndex = buildingIndexCursor });
                            ++buildingIndexCursor;
                            placed = true;
                        }
                        else
                        {
                            buildingIndexGrid[cidx] = -1;
                        }
                    }
                }
            if (!placed) break;   //어떤 미래도로 2x2도 못 깨면 종료 — 잔존 2x2 도로는 구조적 한계
            continue;
        }

        //═════════ Phase 1: 무작위 다트 던지기 ═════════
        const worldGrid::PixelCoord targetCoord{
            localRandom(patchPxX, patchPxX + patchW - 1),
            localRandom(patchPxY, patchPxY + patchH - 1),
            0
        };

        //블록 크기 추첨 — 큰 블록일수록 희귀. 분포 튜닝 시 임계값 조정.
        //  3x3은 매칭 건물 Lot 부재로 당분간 제외(기존 3% → 2x2로 흡수).
        //  2x2:10% / 2x1:10% / 1x2:10% / 1x1:70%
        const int boxPr = localRandom(1, 100);
        int blockW, blockH;
        if      (boxPr <= 10) { blockW = 2; blockH = 2; }
        else if (boxPr <= 20) { blockW = 2; blockH = 1; }
        else if (boxPr <= 30) { blockW = 1; blockH = 2; }
        else                  { blockW = 1; blockH = 1; }

        std::vector<worldGrid::PixelCoord> checkPoints;
        checkPoints.reserve(static_cast<std::size_t>(blockW) * blockH);
        for (int dy = 0; dy < blockH; ++dy)
            for (int dx = 0; dx < blockW; ++dx)
                checkPoints.push_back(worldGrid::PixelCoord{ targetCoord.x + dx, targetCoord.y + dy, 0 });

        //가설 배치 가능한지 검사 — patch bounds + 도시 지면 + 비점유 + 비도로 일체 확인.
        //isEmptyCityPixelLocal이 OOB도 false로 처리해서 블록이 patch 가장자리 넘어가면 자동 거부.
        bool allFree = true;
        for (const auto& cp : checkPoints)
            if (!isEmptyCityPixelLocal(cp.x - patchPxX, cp.y - patchPxY)) { allFree = false; break; }

        if (!allFree) { ++failStreak; continue; }

        //연결성 검증 (stage 5) — 가설 배치 후 BFS, 끊기면 롤백.
        //buildingIndexGrid를 미리 세팅해야 isTraversableCell이 정확히 동작 → simulate→check→commit/rollback.
        for (const auto& cp : checkPoints)
            buildingIndexGrid[static_cast<std::size_t>(cp.y - patchPxY) * patchW + (cp.x - patchPxX)] = buildingIndexCursor;

        if (!allTraversableConnected())
        {
            for (const auto& cp : checkPoints)
                buildingIndexGrid[static_cast<std::size_t>(cp.y - patchPxY) * patchW + (cp.x - patchPxX)] = -1;
            ++failStreak;
            continue;
        }

        //확정 — buildingIndexGrid는 위에서 이미 세팅됨, buildings 벡터만 갱신
        for (const auto& cp : checkPoints)
            buildings.push_back(BuildingPixel{ .coord = cp, .memberBuildingIndex = buildingIndexCursor });
        ++buildingIndexCursor;
        failStreak = 0;
    }

 
    //══════════════════════════════════════════════════════════════════
    // 5. 고립 건물 도로 확장 (Fixed-point)
    //══════════════════════════════════════════════════════════════════
    //   인접한 도로가 없는, 사방이 건물로 둘러싸인 고립 건물 후보정.
    //   stage 4 직후 위치 — 빈 도시 셀 = 미래 도로(stage 6이 채움)이므로
    //   "빈 셀 이웃" 케이스는 자동 해결, 진짜 고립은 4방향 전부 건물인 경우뿐.
    //
    //   방법: 고립 건물 B 발견 시 4방향 1x1 건물 이웃 N 중 하나를 빈 셀로 되돌림
    //         (= 건물 등록 취소). stage 6이 자동 도로 변환 + 비트 결합, stage 7이 직선화.
    //   조건: (a) N이 1x1 — 다중 픽셀 건물 부분 변환 금지
    //         (b) N의 4방향 이웃 중 B 제외 최소 1개가 traversable
    //             — 새 도로가 기존 도로망에 연결됨 (고립 1x1 도로 포켓 방지)
    //         (c) N 변환 후 2x2 미래 도로 패턴 생기지 X — Phase 2 invariant 보존
    //   모든 후보 실패 시 방치 (추후 숲/산으로 대체 예정).
    //
    //   Fixed-point: 한 건물 해제가 옆 건물도 해제(캐스케이드)할 수 있으므로
    //                한 건이라도 변환 발생 시 다시 — 0건이면 종료.

    {
        //건물별 점유 셀 수 — 1x1 판정용. 변환 시 0으로 감소시켜 동적 갱신.
        std::vector<int> memberSize(buildingIndexCursor, 0);
        for (int v : buildingIndexGrid)
            if (v >= 0) ++memberSize[v];

        constexpr int dxs[4] = {  0,  0, +1, -1 };
        constexpr int dys[4] = { -1, +1,  0,  0 };

        while (true)
        {
            bool converted = false;
            for (int ly = 0; ly < patchH; ++ly)
                for (int lx = 0; lx < patchW; ++lx)
                {
                    const std::size_t bidx = static_cast<std::size_t>(ly) * patchW + lx;
                    if (buildingIndexGrid[bidx] < 0) continue;

                    //isolation 검사 — 4방향 이웃이 전부 비-traversable인지 (건물 또는 OOB)
                    bool isolated = true;
                    for (int d = 0; d < 4; ++d)
                        if (isTraversableCell(lx + dxs[d], ly + dys[d])) { isolated = false; break; }
                    if (!isolated) continue;

                    //1x1 건물 이웃을 변환 후보로 순회
                    for (int d = 0; d < 4; ++d)
                    {
                        const int nx = lx + dxs[d];
                        const int ny = ly + dys[d];
                        if (nx < 0 || nx >= patchW || ny < 0 || ny >= patchH) continue;
                        const std::size_t nidx = static_cast<std::size_t>(ny) * patchW + nx;
                        const int memberN = buildingIndexGrid[nidx];
                        if (memberN < 0) continue;
                        if (memberSize[memberN] != 1) continue;

                        //연결성 — N의 B 제외 이웃 중 traversable이 하나라도 있어야
                        bool connects = false;
                        for (int d2 = 0; d2 < 4; ++d2)
                        {
                            const int n2x = nx + dxs[d2];
                            const int n2y = ny + dys[d2];
                            if (n2x == lx && n2y == ly) continue;
                            if (isTraversableCell(n2x, n2y)) { connects = true; break; }
                        }
                        if (!connects) continue;

                        //2x2 미래 도로 사전 차단 — 가설 변환(grid=-1) 후 N 포함 4개 2x2 윈도우 검사
                        buildingIndexGrid[nidx] = -1;
                        bool creates2x2 = false;
                        for (int oy = -1; oy <= 0 && !creates2x2; ++oy)
                            for (int ox = -1; ox <= 0 && !creates2x2; ++ox)
                            {
                                const int wx = nx + ox;
                                const int wy = ny + oy;
                                if (isFutureRoadCell(wx,     wy    )
                                 && isFutureRoadCell(wx + 1, wy    )
                                 && isFutureRoadCell(wx,     wy + 1)
                                 && isFutureRoadCell(wx + 1, wy + 1)) creates2x2 = true;
                            }
                        if (creates2x2) { buildingIndexGrid[nidx] = memberN; continue; }

                        //커밋 — grid는 이미 -1, memberSize만 갱신
                        memberSize[memberN] = 0;
                        converted = true;
                        break;
                    }
                }
            if (!converted) break;
        }

        //buildings 벡터 정리 — 변환된 픽셀(grid -1) 항목 제거
        std::erase_if(buildings, [&](const BuildingPixel& bp) {
            const std::size_t idx = static_cast<std::size_t>(bp.coord.y - patchPxY) * patchW + (bp.coord.x - patchPxX);
            return buildingIndexGrid[idx] == -1;
        });
    }


    //══════════════════════════════════════════════════════════════════
    // 6. 도시블록이 없는 칸에 도로 생성
    //══════════════════════════════════════════════════════════════════
    //   stage 4가 못 메운 빈 도시 픽셀(2x2 빈 채움 후 남은 고립 1x1, 건물 사이 갈래길 등)을
    //   도로 픽셀로 전환. 기준: CityZone/CityCenter & 건물 미점유.
    //   각 도로 픽셀에서 4방향 이웃이 (기존 도로 OR 새로 도로 전환된 픽셀)이면 그쪽 비트 개방
    //   — stage 2/3 진입점·강변 도로와 자동 결합, 신규 도로 격자끼리도 연결됨.
    //   stage 2/3의 lock은 방향별이라 신규 비트 추가에는 무영향(open이 unlocked 방향만 개방).

    {
        auto isRoadCell = [&](int lx, int ly) -> bool {
            if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
            const std::size_t idx = static_cast<std::size_t>(ly) * patchW + lx;
            if (roads[idx].openBits != 0) return true;
            const worldGrid::Terrain t = cityTerrainBox[idx];
            if (t != worldGrid::Terrain::CityZone && t != worldGrid::Terrain::CityCenter) return false;
            return buildingIndexGrid[idx] == -1;
            };

        for (int ly = 0; ly < patchH; ++ly)
            for (int lx = 0; lx < patchW; ++lx)
            {
                if (!isRoadCell(lx, ly)) continue;
                const std::size_t idx = static_cast<std::size_t>(ly) * patchW + lx;
                if (isRoadCell(lx,     ly - 1)) roads[idx].open(RoadPixel::Dir::NORTH);
                if (isRoadCell(lx,     ly + 1)) roads[idx].open(RoadPixel::Dir::SOUTH);
                if (isRoadCell(lx + 1, ly    )) roads[idx].open(RoadPixel::Dir::EAST);
                if (isRoadCell(lx - 1, ly    )) roads[idx].open(RoadPixel::Dir::WEST);
            }
    }


    //══════════════════════════════════════════════════════════════════
    // 7. 차수가 1인 도로를 차수2 일직선 도로로 변환(후처리)
    //══════════════════════════════════════════════════════════════════
    //강가 도로 등으로 인해 발생하는 불완전 도로가 6단계에서 안 이어졌으면 일직선 도로로 변환시켜서 완성함
    //  degree==1이면 켜진 방향의 반대편 비트를 추가 → 막다른 길이 일직선 통과처럼 보이게.
    //  stage 2/3 lock은 방향별이라 반대편(unlocked) 비트 추가에 무영향.
    //  단일 패스로 충분 — 비트 추가는 다른 픽셀 degree에 영향 없음 (양방향 동기화 아님).

    {
        for (int ly = 0; ly < patchH; ++ly)
            for (int lx = 0; lx < patchW; ++lx)
            {
                RoadPixel& r = roads[static_cast<std::size_t>(ly) * patchW + lx];
                if (r.degree() != 1) continue;

                if      (r.isOpen(RoadPixel::Dir::NORTH)) r.open(RoadPixel::Dir::SOUTH);
                else if (r.isOpen(RoadPixel::Dir::SOUTH)) r.open(RoadPixel::Dir::NORTH);
                else if (r.isOpen(RoadPixel::Dir::EAST )) r.open(RoadPixel::Dir::WEST );
                else if (r.isOpen(RoadPixel::Dir::WEST )) r.open(RoadPixel::Dir::EAST );
            }
    }

    //══════════════════════════════════════════════════════════════════
    // 8. 다리 결정 — roads[]에 NS/EW + isBridge 박기, 양 강변 출구 추가
    //══════════════════════════════════════════════════════════════════
    //   CityRiver/CitySea 픽셀에 다리 결정 — 2~4px 폭 직선 강 대응(N픽셀 횡단을
    //   N개 세그먼트 lot으로 분해, [[project_cityriver_internal]] 데이터 규약).
    //   강변 도로는 stage 3에서 강제로 깔리므로 다리 끝점은 항상 도로 본체에 닿음.
    //
    //   알고리즘:
    //     1) 도시 박스 내부(±1 마진 제외) 픽셀 셔플 순회 (visited로 중복 횡단 차단)
    //     2) 물 픽셀이 속한 수직 런 [yN..yS]·수평 런 [xW..xE]을 재고, 양끝 바로 밖이
    //        city land이고 런 길이 ≤ MAX_BRIDGE_SPAN인 축이 유효한 횡단.
    //        둘 다 유효면 짧은 축(진짜 횡단) 우선 — 긴 축은 강을 따라가는 방향.
    //     3) gap 검사 — 같은 z에서 BRIDGE_MIN_GAP_PX 이내 기존 횡단(중심) 있으면 스킵
    //     4) 확률 게이트 — 첫 다리는 면제 (강 전체 0개 다리 케이스 회피)
    //     5) roads[]에 런 전체 박기:
    //          런 각 픽셀: openBits = NS or EW, isBridge = true, bridgeRole 배정
    //                     (Nv==1 Single / 끝 EndLow·EndHigh / 내부 Mid).
    //          양 강변:  강 쪽 출구 비트 OR — stage 3 강변 도로(평행) 마스크와 결합되면서
    //                   강변 lot이 T자/십자로 자연 승격 (streetByOpenSides 정상 매칭).
    //                   stage 3 lock은 강과 *평행한* 방향만 — 강 쪽 비트는 unlocked.
    //     6) plan.bridges 폴리라인 push — Map.ixx 디버그 오버레이용 (강변~강변 전 구간)

    {
        constexpr double BRIDGE_P = 0.5;
        constexpr int BRIDGE_MIN_GAP_PX = 4;
        constexpr int MAX_BRIDGE_SPAN = 6;   //2~4px 강 + 여유. 초과 폭(넓은 해협)은 다리 미생성.
        constexpr int HALF_PX = TILE_PER_PIXEL / 2;

        //local 좌표 물 여부 — 패치 밖(마진 너머)이면 false. cityPixelAt은 무검사라 런 스캔에 필수.
        auto waterAtLocal = [&](int lx, int ly) -> bool {
            if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
            return isWater(cityTerrainBox[static_cast<std::size_t>(ly) * patchW + lx]);
            };

        std::vector<worldGrid::PixelCoord> waterPixels;
        for (int ly = 1; ly < patchH - 1; ++ly)
            for (int lx = 1; lx < patchW - 1; ++lx)
                waterPixels.push_back({ patchPxX + lx, patchPxY + ly, node.center.z });
        std::shuffle(waterPixels.begin(), waterPixels.end(), rng);

        std::vector<char> visited(static_cast<std::size_t>(patchW) * patchH, 0);
        std::vector<worldGrid::PixelCoord> placedCenters;
        bool firstBridge = true;

        for (const auto& p : waterPixels)
        {
            const int sx = p.x - patchPxX;
            const int sy = p.y - patchPxY;
            if (visited[static_cast<std::size_t>(sy) * patchW + sx]) continue;
            if (!isWater(cityPixelAt(p))) continue;

            //수직 런 [yN..yS] — p가 속한 세로 물줄. 양끝 바로 밖이 city land여야 NS 횡단 유효.
            int yN = sy; while (waterAtLocal(sx, yN - 1)) --yN;
            int yS = sy; while (waterAtLocal(sx, yS + 1)) ++yS;
            const int Nv = yS - yN + 1;
            const bool nsValid = Nv <= MAX_BRIDGE_SPAN
                && isCityLandAtLocal(sx, yN - 1) && isCityLandAtLocal(sx, yS + 1);

            //수평 런 [xW..xE] — 가로 물줄. 양끝 바로 밖이 city land여야 EW 횡단 유효.
            int xW = sx; while (waterAtLocal(xW - 1, sy)) --xW;
            int xE = sx; while (waterAtLocal(xE + 1, sy)) ++xE;
            const int Nh = xE - xW + 1;
            const bool weValid = Nh <= MAX_BRIDGE_SPAN
                && isCityLandAtLocal(xW - 1, sy) && isCityLandAtLocal(xE + 1, sy);

            if (!nsValid && !weValid) continue;   //양안 land인 직선 횡단 없음 — visited 안 박음
            const bool chooseNS = nsValid && (!weValid || Nv <= Nh);   //짧은 축(진짜 횡단) 우선, 동률 NS

            //같은 z에서 기존 횡단과 최소 간격 — 강 따라 다리 밀집 방지 (런 중심 기준)
            const int cxLocal = chooseNS ? sx : (xW + xE) / 2;
            const int cyLocal = chooseNS ? (yN + yS) / 2 : sy;
            const worldGrid::PixelCoord center{ patchPxX + cxLocal, patchPxY + cyLocal, p.z };

            bool tooClose = false;
            for (const auto& bp : placedCenters)
            {
                if (bp.z != center.z) continue;
                if (std::abs(bp.x - center.x) <= BRIDGE_MIN_GAP_PX &&
                    std::abs(bp.y - center.y) <= BRIDGE_MIN_GAP_PX) { tooClose = true; break; }
            }
            if (tooClose) continue;

            //확률 게이트 — 첫 다리는 면제 (강마다 최소 1개 다리 보장)
            if (!firstBridge && std::uniform_real_distribution<double>{0.0, 1.0}(rng) >= BRIDGE_P) continue;
            firstBridge = false;

            //roads[]에 런 전체 박기 — 각 픽셀 마스크 + isBridge + role, 양 강변 출구 OR
            if (chooseNS)
            {
                for (int y = yN; y <= yS; ++y)
                {
                    const std::size_t idx = static_cast<std::size_t>(y) * patchW + sx;
                    roads[idx].openBits = std::uint8_t(RoadPixel::Dir::NORTH) | std::uint8_t(RoadPixel::Dir::SOUTH);
                    roads[idx].isBridge = true;
                    roads[idx].bridgeRole = (Nv == 1) ? BridgeRole::Single
                                          : (y == yN) ? BridgeRole::EndLow
                                          : (y == yS) ? BridgeRole::EndHigh
                                                      : BridgeRole::Mid;
                    visited[idx] = 1;
                }
                roads[static_cast<std::size_t>(yN - 1) * patchW + sx].open(RoadPixel::Dir::SOUTH);   //북쪽 강변 → 다리
                roads[static_cast<std::size_t>(yS + 1) * patchW + sx].open(RoadPixel::Dir::NORTH);   //남쪽 강변 → 다리

                const int bx  =  p.x                * TILE_PER_PIXEL + TILE_BASE_X + HALF_PX;
                const int by0 = (patchPxY + yN - 1) * TILE_PER_PIXEL + TILE_BASE_Y + HALF_PX;
                const int by1 = (patchPxY + yS + 1) * TILE_PER_PIXEL + TILE_BASE_Y + HALF_PX;
                plan.bridges.push_back(worldGen::RoadPolyLine{ .verts = { {bx, by0, p.z}, {bx, by1, p.z} } });
            }
            else   //EW
            {
                for (int x = xW; x <= xE; ++x)
                {
                    const std::size_t idx = static_cast<std::size_t>(sy) * patchW + x;
                    roads[idx].openBits = std::uint8_t(RoadPixel::Dir::EAST) | std::uint8_t(RoadPixel::Dir::WEST);
                    roads[idx].isBridge = true;
                    roads[idx].bridgeRole = (Nh == 1) ? BridgeRole::Single
                                          : (x == xW) ? BridgeRole::EndLow
                                          : (x == xE) ? BridgeRole::EndHigh
                                                      : BridgeRole::Mid;
                    visited[idx] = 1;
                }
                roads[static_cast<std::size_t>(sy) * patchW + (xW - 1)].open(RoadPixel::Dir::EAST);   //서쪽 강변 → 다리
                roads[static_cast<std::size_t>(sy) * patchW + (xE + 1)].open(RoadPixel::Dir::WEST);   //동쪽 강변 → 다리

                const int by  =  p.y                * TILE_PER_PIXEL + TILE_BASE_Y + HALF_PX;
                const int bx0 = (patchPxX + xW - 1) * TILE_PER_PIXEL + TILE_BASE_X + HALF_PX;
                const int bx1 = (patchPxX + xE + 1) * TILE_PER_PIXEL + TILE_BASE_X + HALF_PX;
                plan.bridges.push_back(worldGen::RoadPolyLine{ .verts = { {bx0, by, p.z}, {bx1, by, p.z} } });
            }

            placedCenters.push_back(center);
        }
    }


    //══════════════════════════════════════════════════════════════════
    // 9. 도로 픽셀 → Lot 페인트
    //══════════════════════════════════════════════════════════════════
    //   stage 7까지 확정된 roads[].openBits를 픽셀별로 streetByOpenSides() 라우팅 →
    //   각 픽셀(24×24타일)의 lot 결과를 plan.tiles에 push. procGenerate stage 4가
    //   plan.tiles를 자기 섹터 범위로 클립해 SectorPlan에 페인트.
    //
    //   degree<2 마스크는 stage 7이 제거 보장 → streetByOpenSides가 nullptr 반환하면
    //   stage 7 invariant 위반(호출자 버그). 일단 방어적으로 스킵.
    //
    //   픽셀별 시드 — (seed, pxX, pxY) 해시. 같은 (cityId, pixel) → 같은 lot 변형 보장.
    //   가로수 종류 등이 픽셀마다 결정론적으로 다르게 나옴.

    {
        for (int ly = 0; ly < patchH; ++ly)
            for (int lx = 0; lx < patchW; ++lx)
            {
                const std::size_t idx = static_cast<std::size_t>(ly) * patchW + lx;
                const std::uint8_t mask = roads[idx].openBits;
                if (mask == 0) continue;

                const Lot* lot = roads[idx].isBridge
                    ? static_cast<const Lot*>(bridgeByOpenSides(mask, roads[idx].bridgeRole))
                    : static_cast<const Lot*>(streetByOpenSides(mask));
                if (lot == nullptr) continue;

                std::uint64_t lotSeed = seed ^ 0x9E3779B97F4A7C15ULL;
                lotSeed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(patchPxX + lx)) * 0xBF58476D1CE4E5B9ULL;
                lotSeed  = (lotSeed ^ (lotSeed >> 27)) * 0x94D049BB133111EBULL;
                lotSeed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(patchPxY + ly)) * 0x94D049BB133111EBULL;
                lotSeed ^= lotSeed >> 31;

                LotResult r = lot->generate(lotSeed);

                const int originX = (patchPxX + lx) * TILE_PER_PIXEL + TILE_BASE_X;
                const int originY = (patchPxY + ly) * TILE_PER_PIXEL + TILE_BASE_Y;
                blitLotResult(plan, r, originX, originY, node.center.z);
            }
    }


    //══════════════════════════════════════════════════════════════════
    // 10. 건물 픽셀 그룹 → 건물 Lot 페인트
    //══════════════════════════════════════════════════════════════════
    //   stage 4~5가 확정한 building 픽셀을 memberBuildingIndex별 그룹(직사각형
    //   footprint)으로 복원 → buildingByFootprint로 Lot 선택 → 인접 도로를 향하도록
    //   회전 결정 → blitLotResult로 plan.tiles에 블릿. stage 9(도로)와 동일한 픽셀→Lot
    //   패턴이며, 차이는 "그룹 단위 멀티픽셀 footprint"라는 점뿐.
    //
    //   회전↔footprint 결합: 비정사각 Lot(authored 2x1)은 none/180이 2x1, 90/270이 1x2.
    //   그룹이 직사각형이면 footprint 보존 회전만 후보(2x1 그룹→도어 N/S, 1x2 그룹→도어
    //   E/W), 정사각이면 4방향 자유. allowRotation()==false면 none만(authored 방향).
    //   도어 컨벤션은 캐논 남쪽(rotateLotResult CCW): none=S, ccw90=E, ccw180=N, ccw270=W.
    //   도어가 도로 향하는 회전을 우선, 없으면 footprint 맞는 아무 회전(균등 추첨).

    {
        //building 픽셀을 memberIndex별 bounding box로 묶음 — stage 5 후 모두 직사각형.
        struct Group { int minPx, minPy, maxPx, maxPy, count; bool init; };
        std::unordered_map<int, Group> groups;
        for (const auto& bp : buildings)
        {
            Group& g = groups[bp.memberBuildingIndex];
            if (!g.init) { g.minPx = g.maxPx = bp.coord.x; g.minPy = g.maxPy = bp.coord.y; g.init = true; }
            else
            {
                g.minPx = std::min(g.minPx, bp.coord.x);  g.maxPx = std::max(g.maxPx, bp.coord.x);
                g.minPy = std::min(g.minPy, bp.coord.y);  g.maxPy = std::max(g.maxPy, bp.coord.y);
            }
            ++g.count;
        }

        //절대 px의 roads[] openBits 조회 — 그룹 외곽 도로 인접 판정용. 패치 밖이면 도로 없음.
        auto roadOpenAtPx = [&](int px, int py) -> bool {
            const int lx = px - patchPxX, ly = py - patchPxY;
            if (lx < 0 || lx >= patchW || ly < 0 || ly >= patchH) return false;
            return roads[static_cast<std::size_t>(ly) * patchW + lx].openBits != 0;
            };

        for (const auto& kv : groups)
        {
            const Group& g = kv.second;
            const int gw = g.maxPx - g.minPx + 1;
            const int gh = g.maxPy - g.minPy + 1;
            if (g.count != gw * gh) continue;   //비직사각(이론상 없음) — 방어적 스킵

            //그룹 결정론 시드 — origin px 해시 (stage 9 픽셀 시드와 동형). 선택·회전·내용 공유.
            std::uint64_t gSeed = seed ^ 0x9E3779B97F4A7C15ULL;
            gSeed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(g.minPx)) * 0xBF58476D1CE4E5B9ULL;
            gSeed  = (gSeed ^ (gSeed >> 27)) * 0x94D049BB133111EBULL;
            gSeed ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(g.minPy)) * 0x94D049BB133111EBULL;
            gSeed ^= gSeed >> 31;
            std::mt19937_64 gRng{ gSeed };

            const Lot* lot = buildingByFootprint(gw, gh, gRng);
            if (lot == nullptr) continue;   //매칭 footprint 없음 — 빈 마당으로 남김

            //그룹 4면 도로 인접 여부 (외곽 한 줄에 도로 픽셀이 하나라도 있으면 true)
            bool roadN = false, roadS = false, roadE = false, roadW = false;
            for (int i = 0; i < gw; ++i)
            {
                if (roadOpenAtPx(g.minPx + i, g.minPy - 1)) roadN = true;
                if (roadOpenAtPx(g.minPx + i, g.maxPy + 1)) roadS = true;
            }
            for (int j = 0; j < gh; ++j)
            {
                if (roadOpenAtPx(g.minPx - 1, g.minPy + j)) roadW = true;
                if (roadOpenAtPx(g.maxPx + 1, g.minPy + j)) roadE = true;
            }

            //회전 후보 — footprint가 (gw,gh) 일치 + (회전가능 or none). reservoir 균등 추첨으로
            //  도로 향하는 회전(bestRoad)과 전체(bestAny)를 동시에 뽑아, 도로 후보 있으면 우선.
            const int aw = lot->sizeChunkW();
            const int ah = lot->sizeChunkH();
            const bool canRot = lot->allowRotation();

            lotRot bestRoad = lotRot::none, bestAny = lotRot::none;
            int nRoad = 0, nAny = 0;
            auto consider = [&](lotRot rot, int rw, int rh, bool doorRoad) {
                if (rw != gw || rh != gh) return;            //footprint 불일치
                if (rot != lotRot::none && !canRot) return;  //비회전 Lot은 none만
                ++nAny;
                if (std::uniform_int_distribution<int>{1, nAny}(gRng) == 1) bestAny = rot;
                if (doorRoad)
                {
                    ++nRoad;
                    if (std::uniform_int_distribution<int>{1, nRoad}(gRng) == 1) bestRoad = rot;
                }
                };
            consider(lotRot::none,   aw, ah, roadS);   //도어 남
            consider(lotRot::ccw90,  ah, aw, roadE);   //도어 동 (W/H 스왑)
            consider(lotRot::ccw180, aw, ah, roadN);   //도어 북
            consider(lotRot::ccw270, ah, aw, roadW);   //도어 서 (W/H 스왑)
            if (nAny == 0) continue;   //방어적 — buildingByFootprint가 후보 존재를 보장

            const lotRot chosen = (nRoad > 0) ? bestRoad : bestAny;

            LotResult r = generateRotated(*lot, gRng(), chosen);

            const int originX = g.minPx * TILE_PER_PIXEL + TILE_BASE_X;
            const int originY = g.minPy * TILE_PER_PIXEL + TILE_BASE_Y;

            //월드맵 심볼 — 건물 종류 + 그룹 footprint(회전 후 실제 점유 모양 gw×gh).
            //  Lot이 빈 스켈레톤이라 blitLotResult가 타일을 안 깔아도 심볼은 이 채널로 표시.
            plan.symbols.push_back(CitySymbol{
                .pos    = Point3{ originX, originY, node.center.z },
                .w      = gw,
                .h      = gh,
                .symbol = mapSymbolOf(lot),
            });

            blitLotResult(plan, r, originX, originY, node.center.z);
        }
    }


    //══════════════════════════════════════════════════════════════════
    // Ex. 중간 점검... 도시 내부 폴리라인 형성해서 Map.ixx에서 볼 수 있도록
    //══════════════════════════════════════════════════════════════════
    //   stage 2, 3, 8에서 세팅한 roads[] 비트를 plan.segments 폴리라인으로 변환.
    //   각 픽셀의 4방향 비트마다 픽셀 중심 → 픽셀 경계 절반 길이(=12타일) 라인.
    //   양쪽 픽셀 모두 켜져 있으면 두 절반이 합쳐져 중심-중심 라인이 되고,
    //   한쪽만 켜져 있으면 절반 라인으로 남아 비대칭(stage 3의 한쪽 강변 비트 등) 감지 가능.
    //   Map.ixx::drawCityRoadOverlay가 plan.segments를 2점 라인으로 그림 → 그대로 소비.

    {
        constexpr int HALF_PX = TILE_PER_PIXEL / 2;
        const int z = node.center.z;

        for (int y = 0; y < patchH; ++y)
            for (int x = 0; x < patchW; ++x)
            {
                const RoadPixel& r = roads[static_cast<std::size_t>(y) * patchW + x];
                if (r.openBits == 0) continue;

                //월드맵 도로 autotile 래스터 — 청크 좌상단 + openBits 그대로 기록.
                plan.roadCells.push_back(CityRoadCell{
                    .pos      = Point3{ (patchPxX + x) * TILE_PER_PIXEL + TILE_BASE_X,
                                        (patchPxY + y) * TILE_PER_PIXEL + TILE_BASE_Y, z },
                    .openBits = r.openBits,
                });

                const int cx = (patchPxX + x) * TILE_PER_PIXEL + TILE_BASE_X + HALF_PX;
                const int cy = (patchPxY + y) * TILE_PER_PIXEL + TILE_BASE_Y + HALF_PX;

                if (r.isOpen(RoadPixel::Dir::NORTH))
                    plan.segments.push_back(worldGen::RoadPolyLine{ .verts = { {cx, cy, z}, {cx, cy - HALF_PX, z} } });
                if (r.isOpen(RoadPixel::Dir::SOUTH))
                    plan.segments.push_back(worldGen::RoadPolyLine{ .verts = { {cx, cy, z}, {cx, cy + HALF_PX, z} } });
                if (r.isOpen(RoadPixel::Dir::EAST))
                    plan.segments.push_back(worldGen::RoadPolyLine{ .verts = { {cx, cy, z}, {cx + HALF_PX, cy, z} } });
                if (r.isOpen(RoadPixel::Dir::WEST))
                    plan.segments.push_back(worldGen::RoadPolyLine{ .verts = { {cx, cy, z}, {cx - HALF_PX, cy, z} } });
            }
    }

    //── debug: 건물 픽셀 분포를 CityPlan에 보관 (Map 오버레이용) ──
    //  로컬 buildings의 픽셀좌표를 픽셀 top-left 실타일좌표로 변환해서 plan.buildings에 적재.
    {
        const int z = node.center.z;
        plan.buildings.reserve(buildings.size());
        for (const auto& bp : buildings)
        {
            const int tx = bp.coord.x * TILE_PER_PIXEL + TILE_BASE_X;
            const int ty = bp.coord.y * TILE_PER_PIXEL + TILE_BASE_Y;
            plan.buildings.push_back(CityBuildingPixel{
                .pos = Point3{ tx, ty, z },
                .memberIndex = bp.memberBuildingIndex,
                });
        }
    }

    return plan;
}
