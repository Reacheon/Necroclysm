module CityPlan;

import std;
import util;
import constVar;
import worldGen;
import worldGrid;

// ════════════════════════════════════════════════════════════════════════
// CityPlan_build.cpp — buildCityPlan 구현.
//
//   알고리즘: 픽셀(=24타일) 단위 도시 절차생성. 단계 누적식 (procGenerate 패턴).
//     1. 도시 영역 terrain 박스 로드 (+1px 마진) ▶ cityPixelAt
//     2. 외부 도로 진입점 → 도시 가장자리 픽셀에 강제 도로 비트 (lock)
//     3. 강가/해안 픽셀에 평행 도로 비트 (1px 평행 충돌 회피, lock)
//     4. 도시 내부 다트던지기로 건물 픽셀 배치 (Phase 1 무작위 + Phase 2 결정적
//        anti-2x2-road 채움) — 매 가설 배치마다 연결성 BFS 검증.
//     6. 비건물 도시 셀을 도로 픽셀로 변환 (4방향 이웃과 비트 결합).
//     7. degree==1 도로를 일직선(degree==2)으로 대칭화.
//     Ex. 도로 비트를 polyline 세그먼트로 환산 (Map.ixx 디버그 오버레이).
//
//   산출물: plan.tiles, plan.segments(도로), plan.bridges, plan.buildings(debug).
//
//   주의: procGenerate(ProcGenWorker 스레드)가 CityPlanCache::getOrCompute 경유로
//   본 함수를 호출 — prt는 스레드 안전 보장 X. 디버그 출력 용도로만 사용.
// ════════════════════════════════════════════════════════════════════════

CityPlan buildCityPlan(city::CityId id, std::uint64_t seed)
{
    CityPlan plan{ id };

    plan.tiles.push_back(CityTile{
        .pos = Point3{ 20322, 32012, 1 },
        .floor = static_cast<std::uint16_t>(itemID::blackAsphalt),
        });

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
    //   Phase 1 — 무작위 좌표 + 1x1/2x2/3x3 블록 가설 배치 → checkPoints 전셀의
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

        //블록 크기 추첨 — 큰 블록일수록 희귀. 분포 튜닝 시 임계값(3, 10) 조정.
        const int boxPr = localRandom(1, 100);
        const int blockDim = (boxPr <= 3) ? 3 : (boxPr <= 10) ? 2 : 1;

        std::vector<worldGrid::PixelCoord> checkPoints;
        checkPoints.reserve(static_cast<std::size_t>(blockDim) * blockDim);
        for (int dy = 0; dy < blockDim; ++dy)
            for (int dx = 0; dx < blockDim; ++dx)
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
    // Ex. 중간 점검... 도시 내부 폴리라인 형성해서 Map.ixx에서 볼 수 있도록
    //══════════════════════════════════════════════════════════════════
    //   stage 2, 3에서 세팅한 roads[] 비트를 plan.segments 폴리라인으로 변환.
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
