module CityPlan;

import std;
import util;
import constVar;
import worldGen;
import worldGrid;

// ════════════════════════════════════════════════════════════════════════
// CityPlan_build.cpp — buildCityPlan 구현.
//
//   알고리즘: '도로 세그먼트 랜덤 제거' — 도시 직사각형들에 균일 격자 분할선
//   생성 → 48타일 단위 세그먼트 분해 → 물 인접 세그먼트 제거 → 그래프 변환 →
//   다트던지기로 랜덤 제거 (연결성 보존). 살아남은 세그먼트는 plan.segments에 저장.
//
//   향후 단계: 살아남은 세그먼트가 둘러싼 블록 분할 + 건물 prefab 배치 + 다리
//   처리 — 본 함수에 누적 (procGenerate 패턴).
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
    // 2. 도시의 진입점과 절단방향 설정 ▶ entryPoints
    //══════════════════════════════════════════════════════════════════

    enum class cutDir
    {
        horizontal,
        vertical,
    };

    std::vector<std::pair<Point3, cutDir>> entryPoints;

    if (worldGen::activePolyLines != nullptr)
    {
        for (const worldGen::RoadPolyLine& line : *worldGen::activePolyLines)
        {
            if (line.verts.size() < 2) continue;

            for (int endIdx = 0; endIdx < 2; ++endIdx)
            {
                const Point3 endpoint = (endIdx == 0) ? line.verts.front() : line.verts.back();
                const Point3 adjacent = (endIdx == 0) ? line.verts[1] : line.verts[line.verts.size() - 2];

                const int epx = (endpoint.x - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
                const int epy = (endpoint.y - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;

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
                const cutDir cd = (std::abs(dx) >= std::abs(dy)) ? cutDir::horizontal : cutDir::vertical;

                entryPoints.push_back({ endpoint, cd });
            }
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 3. 가능한 모든 도로들 긋기
    //══════════════════════════════════════════════════════════════════

    std::vector<worldGen::RoadPolyLine> divLines; //분할선들

    for (const city::CityRect& rect : node.rectangles)
    {
        for (int dy = 0; dy < rect.h; dy++)
        {
            for (int dx = 0; dx < rect.w; dx++)
            {
                if (dx == 0) //수평 분할선
                {
                    const int tileX0 = rect.px * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X;
                    const int tileX1 = (rect.px + rect.w) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X;
                    const int y = (rect.py + dy) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y + worldGrid::TILES_PER_PIXEL / 2;
                    const int z = node.center.z;

                    divLines.push_back(worldGen::RoadPolyLine{ .verts = { {tileX0, y, z}, {tileX1 - 1, y, z} } });
                }

                if (dy == 0) // 수직 분할선
                {
                    const int tileY0 = rect.py * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y;
                    const int tileY1 = (rect.py + rect.h) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y;
                    const int x = (rect.px + dx) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X + worldGrid::TILES_PER_PIXEL / 2;
                    const int z = node.center.z;

                    divLines.push_back(worldGen::RoadPolyLine{ .verts = { {x, tileY0, z}, {x, tileY1 - 1, z} } });
                }
            }
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 4. 세그먼트 단위(48타일)로 분해 시작
    //══════════════════════════════════════════════════════════════════

    std::vector<worldGen::RoadPolyLine> segments; //세그먼트들

    auto isHorizontal = [](const worldGen::RoadPolyLine& l) {
        return l.verts.front().y == l.verts.back().y;
        };

    //수평선 분해
    for (const auto& h : divLines)
    {
        if (!isHorizontal(h)) continue;

        const int hY = h.verts.front().y;
        const int hX0 = h.verts.front().x;
        const int hX1 = h.verts.back().x;
        const int z = h.verts.front().z;

        //이 수평선과 교차하는 수직선들의 x좌표 수집
        std::vector<int> crossX;
        for (const auto& v : divLines)
        {
            if (isHorizontal(v)) continue;
            const int vX = v.verts.front().x;
            const int vY0 = v.verts.front().y;
            const int vY1 = v.verts.back().y;
            if (vX > hX0 && vX < hX1 && hY >= vY0 && hY <= vY1)
                crossX.push_back(vX);
        }
        std::sort(crossX.begin(), crossX.end());

        //hX0 → crossX[0] → crossX[1] → ... → hX1 로 끊기
        int prev = hX0;
        for (int cx : crossX)
        {
            segments.push_back(worldGen::RoadPolyLine{ .verts = { {prev, hY, z}, {cx, hY, z} } });
            prev = cx;
        }
        segments.push_back(worldGen::RoadPolyLine{ .verts = { {prev, hY, z}, {hX1, hY, z} } });
    }

    //수직선 분해
    for (const auto& v : divLines)
    {
        if (isHorizontal(v)) continue;

        const int vX = v.verts.front().x;
        const int vY0 = v.verts.front().y;
        const int vY1 = v.verts.back().y;
        const int z = v.verts.front().z;

        std::vector<int> crossY;
        for (const auto& h : divLines)
        {
            if (!isHorizontal(h)) continue;
            const int hY = h.verts.front().y;
            const int hX0 = h.verts.front().x;
            const int hX1 = h.verts.back().x;
            if (hY > vY0 && hY < vY1 && vX >= hX0 && vX <= hX1)
                crossY.push_back(hY);
        }
        std::sort(crossY.begin(), crossY.end());

        int prev = vY0;
        for (int cy : crossY)
        {
            segments.push_back(worldGen::RoadPolyLine{ .verts = { {vX, prev, z}, {vX, cy, z} } });
            prev = cy;
        }
        segments.push_back(worldGen::RoadPolyLine{ .verts = { {vX, prev, z}, {vX, vY1, z} } });
    }



    //══════════════════════════════════════════════════════════════════
    // 5. 물타일과 인접한 세그먼트들 제거 (진입점은 보존)
    //══════════════════════════════════════════════════════════════════

    auto terrainAtTile = [&](Point3 tile) -> worldGrid::Terrain {
        const int px = (tile.x - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
        const int py = (tile.y - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;
        return cityPixelAt({ px, py, tile.z });
        };

    std::unordered_set<Point3, Point3::Hash> entrySet;
    for (const auto& [ep, cd] : entryPoints) entrySet.insert(ep);

    for (int i = 0; i <= 1; i++)
    {
        for (int del = -1; del <= 1; del += 2)
        {

            std::erase_if(segments, [&](const worldGen::RoadPolyLine& seg) {
                if (entrySet.contains(seg.verts[0]) || entrySet.contains(seg.verts[1])) return false;
                const Point3 outside{ seg.verts[i].x + del, seg.verts[i].y, seg.verts[i].z };
                const auto t = terrainAtTile(outside);
                return t == worldGrid::Terrain::CityRiver
                    || t == worldGrid::Terrain::CitySea
                    || t == worldGrid::Terrain::Sea
                    || t == worldGrid::Terrain::River
                    || t == worldGrid::Terrain::Lake;
                });


            std::erase_if(segments, [&](const worldGen::RoadPolyLine& seg) {
                if (entrySet.contains(seg.verts[0]) || entrySet.contains(seg.verts[1])) return false;
                const Point3 outside{ seg.verts[i].x, seg.verts[i].y + del, seg.verts[i].z };
                const auto t = terrainAtTile(outside);
                return t == worldGrid::Terrain::CityRiver
                    || t == worldGrid::Terrain::CitySea
                    || t == worldGrid::Terrain::Sea
                    || t == worldGrid::Terrain::River
                    || t == worldGrid::Terrain::Lake;
                });
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 6. 그래프 생성 및 플래그 할당
    //══════════════════════════════════════════════════════════════════

    struct Graph 
    {
        std::vector<Point3> nodeCoord;                       //노드점 위치들
        std::vector<std::array<int, 2>> edgeToNodes;         //입력한 엣지의 양끝 노드점
        std::vector<std::vector<int>> adjacency;             //특정 점에서 뻗어나가는(연결된) 엣지들
        std::vector<bool> edgeAlive;                         // 임시 제거용 플래그
        std::vector<bool> edgePreserved;                     // 보존대상 마킹
    };

    Graph graph;
    
    // 좌표 → 노드ID 매핑 (중복 좌표 통합)
    std::unordered_map<Point3, int, Point3::Hash> coordToNode;
    auto getOrCreateNode = [&](Point3 p) {
        auto [it, inserted] = coordToNode.try_emplace(p, static_cast<int>(graph.nodeCoord.size()));
        if (inserted) {
            graph.nodeCoord.push_back(p);
            graph.adjacency.emplace_back();
        }
        return it->second;
        };

    // segments → 그래프 변환
    for (size_t i = 0; i < segments.size(); ++i)
    {
        const int a = getOrCreateNode(segments[i].verts[0]);
        const int b = getOrCreateNode(segments[i].verts[1]);
        const int e = static_cast<int>(graph.edgeToNodes.size());
        graph.edgeToNodes.push_back({ a, b });
        graph.adjacency[a].push_back(e);
        graph.adjacency[b].push_back(e);
        graph.edgeAlive.push_back(true);
        graph.edgePreserved.push_back(false);
    }

    //── 6.2 강변/해안 도로 보존 플래그 설정 ──────────────────────────────────────

    auto isWater = [](worldGrid::Terrain t) {
        return t == worldGrid::Terrain::River
            || t == worldGrid::Terrain::Sea
            || t == worldGrid::Terrain::Lake
            || t == worldGrid::Terrain::CityRiver
            || t == worldGrid::Terrain::CitySea;
        };

    for (size_t e = 0; e < segments.size(); ++e)
    {
        const auto& seg = segments[e];
        const Point3& a = seg.verts[0];
        const Point3& b = seg.verts[1];
        const bool horizontal = (a.y == b.y);

        //세그먼트가 지나는 픽셀 범위 산출
        const int x0 = std::min(a.x, b.x);
        const int x1 = std::max(a.x, b.x);
        const int y0 = std::min(a.y, b.y);
        const int y1 = std::max(a.y, b.y);
        const int px0 = (x0 - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
        const int px1 = (x1 - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
        const int py0 = (y0 - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;
        const int py1 = (y1 - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;

        bool preserve = false;
        if (horizontal)
        {
            //수평선: 픽셀 행 py0의 상하 픽셀(py-1, py+1) 검사
            for (int px = px0; px <= px1 && !preserve; ++px)
            {
                if (isWater(cityPixelAt({ px, py0 - 1, a.z }))) preserve = true;
                if (isWater(cityPixelAt({ px, py0 + 1, a.z }))) preserve = true;
            }
        }
        else
        {
            //수직선: 픽셀 컬럼 px0의 좌우 픽셀(px-1, px+1) 검사
            for (int py = py0; py <= py1 && !preserve; ++py)
            {
                if (isWater(cityPixelAt({ px0 - 1, py, a.z }))) preserve = true;
                if (isWater(cityPixelAt({ px0 + 1, py, a.z }))) preserve = true;
            }
        }

        if (preserve) graph.edgePreserved[e] = true;
    }

    //── 6.3 진입점 연결 도로 보존 플래그 설정 ────────────────────────────────────
    // 진입점 분할선은 지터링이 발생하지 않으므로 완전 일치만 파악하면 OK

    for (const auto& [ep, cd] : entryPoints)
    {
        auto it = coordToNode.find(ep);
        if (it == coordToNode.end()) continue;
        for (int e : graph.adjacency[it->second])
        {
            graph.edgePreserved[e] = true;
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 7. 다트던지기로 세그먼트 제거 시작
    //══════════════════════════════════════════════════════════════════

    //── 7.1 주요 상수 설정 ────────────────────────────────────

    constexpr double CITY_OUTSKIRTS_REMOVE_P = 0.05; //도시 외곽의 세그먼트 제거확률
    constexpr double CITY_CENTER_REMOVE_P = 0.40; //도시 중심의 세그먼트 제거확률
    constexpr double CITY_REMOVE_FALLOFF_COEFF = 2.0; //도시 외곽-중심 사이의 감쇠상수
    constexpr double CITY_RECT_MIN_SEG_RATIO = 0.50; //사각형 하나의 최소 세그먼트 비율

    std::vector<size_t> order(segments.size());
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng);
    
    const int maxDist = std::max(cityWidth, cityHeight) * worldGrid::TILES_PER_PIXEL;



    //── 7.2 사각형별 카운트 파라미터 설정 ────────────────────────────────────

    std::vector<int> rectInitSegCount(node.rectangles.size(), 0);    // 초기 세그먼트 수
    std::vector<int> rectRemovedSegCount(node.rectangles.size(), 0); // 제거 누적 카운트

    //@brief 세그먼트를 입력하면 어느 직사각형인지를 반환, 없으면 -1
    auto segToRectIndex = [&](const worldGen::RoadPolyLine& seg) -> int {
        const int midX = (seg.verts[0].x + seg.verts[1].x) / 2;
        const int midY = (seg.verts[0].y + seg.verts[1].y) / 2;
        const int px = (midX - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
        const int py = (midY - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;
        for (size_t i = 0; i < node.rectangles.size(); ++i)
        {
            const auto& r = node.rectangles[i];
            if (px >= r.px && px < r.x1() && py >= r.py && py < r.y1())
                return static_cast<int>(i);
        }
        return -1;
        };

    for (const auto& seg : segments)
    {
        const int ri = segToRectIndex(seg);
        if (ri >= 0) ++rectInitSegCount[ri];
    }


    //── 7.3 엣지 제거 시 끝점 간 연결성 검사 람다 ────────────────────────────────────

    auto removalKeepsEndpointsConnected = [&](int e) -> bool {
        const int u = graph.edgeToNodes[e][0];
        const int v = graph.edgeToNodes[e][1];

        std::vector<bool> visited(graph.nodeCoord.size(), false);
        std::queue<int> q;
        q.push(u);
        visited[u] = true;

        while (!q.empty())
        {
            const int cur = q.front(); q.pop();
            if (cur == v) return true;

            for (int ei : graph.adjacency[cur])
            {
                if (ei == e) continue;             //제거 후보 엣지 무시
                if (!graph.edgeAlive[ei]) continue; //이미 제거된 엣지 무시
                const int next = (graph.edgeToNodes[ei][0] == cur) ? graph.edgeToNodes[ei][1] : graph.edgeToNodes[ei][0];
                if (!visited[next]) { visited[next] = true; q.push(next); }
            }
        }
        return false;
        };

    //── 7.4 세그먼트 랜덤 제거 플래그 설정 ────────────────────────────────────
    for (size_t idx : order)
    {
        const size_t e = idx;  // edgeId == segments 인덱스

        //── 게이트 ① 보존대상 ──
        if (graph.edgePreserved[e]) continue;

        //── 게이트 ② rect 인덱스 + 최소 비율 ──
        const int ri = segToRectIndex(segments[idx]);
        if (ri < 0) continue;
        const int alive = rectInitSegCount[ri] - rectRemovedSegCount[ri];
        const int minAlive = static_cast<int>(rectInitSegCount[ri] * CITY_RECT_MIN_SEG_RATIO);
        if (alive <= minAlive) continue;

        //── 게이트 ③ 거리 기반 확률 ──
        const double midX = (segments[idx].verts[0].x + segments[idx].verts[1].x) * 0.5;
        const double midY = (segments[idx].verts[0].y + segments[idx].verts[1].y) * 0.5;
        const double dx = midX - node.center.x;
        const double dy = midY - node.center.y;
        const double d = std::sqrt(dx * dx + dy * dy);
        const double t = std::clamp(d / maxDist, 0.0, 1.0);
        const double p = CITY_OUTSKIRTS_REMOVE_P + (CITY_CENTER_REMOVE_P - CITY_OUTSKIRTS_REMOVE_P) * std::pow(1.0 - t, CITY_REMOVE_FALLOFF_COEFF);
        if (std::uniform_real_distribution<double>{0.0, 1.0}(rng) >= p)  continue;

        //── 게이트 ④ 연결성 검사 ──
        //  엣지 e 제거 시 끝점 두 노드가 여전히 다른 경로로 연결되어 있으면 제거 OK.
        if (removalKeepsEndpointsConnected(static_cast<int>(e)))
        {
            graph.edgeAlive[e] = false;  // 제거 확정
            ++rectRemovedSegCount[ri];
        }
    }


    //══════════════════════════════════════════════════════════════════
    // 8. 세그먼트 지터링 — 블록 크기/모양 다양화 (현재는 일단 비활성화)
    //══════════════════════════════════════════════════════════════════
    //   살아있는 세그먼트를 직교 방향으로 평행이동 (수평→Y, 수직→X, 대각선 X).
    //   - 한 세그먼트 흔들면 양 끝점의 *직교* 살아있는 세그먼트만 잠금
    //     → 같은 분할선 인접 세그먼트는 독립으로 다른 양만큼 흔들기 가능
    //   - 흔든 끝점이 잠긴 직교 도로 위에 정확히 닿아야 함 (갭 방지)
    //     → 흔들기 전 직교 도로 길이 검사 → 양/단방향/스킵 결정
    //   - 진입점/강변 보존 세그먼트(edgePreserved)는 위치 고정 (잠금만 적용)
    //══════════════════════════════════════════════════════════════════

    constexpr int JITTER_MIN = 24;
    constexpr int JITTER_MAX = 24;
    constexpr double CITY_SEGMENT_JITTER_P = 0.05; //세그먼트 지터링 발동 확률 (조건 충족 시)

    //@brief 노드에서 직교 방향으로 signedJ 타일 이상 뻗은 살아있는 직교 도로 존재 여부
    auto canExtendOrtho = [&](int nodeId, bool segH, int signedJ) -> bool {
        const Point3 nodePos = graph.nodeCoord[nodeId];
        for (int e : graph.adjacency[nodeId])
        {
            if (!graph.edgeAlive[e]) continue;
            if (isHorizontal(segments[e]) == segH) continue; // 직교 아닌 것 제외

            const int otherNode = (graph.edgeToNodes[e][0] == nodeId)
                ? graph.edgeToNodes[e][1] : graph.edgeToNodes[e][0];
            const Point3 otherPos = graph.nodeCoord[otherNode];

            //직교 도로가 nodePos에서 signedJ와 같은 부호로 |signedJ| 이상 뻗어 있어야 함
            const int span = segH ? (otherPos.y - nodePos.y) : (otherPos.x - nodePos.x);
            if (signedJ > 0 && span >= signedJ) return true;
            if (signedJ < 0 && span <= signedJ) return true;
        }
        return false;
        };

    std::vector<bool> jitterLocked(segments.size(), false);
    for (size_t e = 0; e < segments.size(); ++e)
        if (graph.edgePreserved[e]) jitterLocked[e] = true;

    std::vector<size_t> jitterOrder;
    jitterOrder.reserve(segments.size());
    for (size_t e = 0; e < segments.size(); ++e)
        if (graph.edgeAlive[e] && !graph.edgePreserved[e]) jitterOrder.push_back(e);
    std::shuffle(jitterOrder.begin(), jitterOrder.end(), rng);

    for (size_t e : jitterOrder)
    {
        if (jitterLocked[e]) continue;

        //── 확률 게이트 (스킵돼도 잠그지 않음 — 다른 세그먼트의 직교 도로 역할 유지) ──
        if (std::uniform_real_distribution<double>{0.0, 1.0}(rng) >= CITY_SEGMENT_JITTER_P) continue;

        const bool h = isHorizontal(segments[e]);
        const int n0 = graph.edgeToNodes[e][0];
        const int n1 = graph.edgeToNodes[e][1];

        const int mag = localRandom(JITTER_MIN, JITTER_MAX);
        const bool canPos = canExtendOrtho(n0, h, +mag) && canExtendOrtho(n1, h, +mag);
        const bool canNeg = canExtendOrtho(n0, h, -mag) && canExtendOrtho(n1, h, -mag);

        int j;
        if (canPos && canNeg) j = (localRandom(0, 1) == 0) ? +mag : -mag;
        else if (canPos)      j = +mag;
        else if (canNeg)      j = -mag;
        else                  continue; // 양 방향 모두 불가 — 스킵

        //평행이동 (수평선은 Y축, 수직선은 X축)
        if (h) { segments[e].verts[0].y += j; segments[e].verts[1].y += j; }
        else   { segments[e].verts[0].x += j; segments[e].verts[1].x += j; }

        //양 끝점에 연결된 *직교* 살아있는 세그먼트만 잠금 (같은 방향은 자유)
        for (int n : { n0, n1 })
            for (int adjE : graph.adjacency[n])
            {
                if (adjE == static_cast<int>(e)) continue;
                if (!graph.edgeAlive[adjE]) continue;
                if (isHorizontal(segments[adjE]) != h) jitterLocked[adjE] = true;
            }
    }


    //══════════════════════════════════════════════════════════════════
    // 9. 살아남은 세그먼트(지터링 반영) plan.segments에 저장
    //══════════════════════════════════════════════════════════════════
    {
        std::vector<worldGen::RoadPolyLine> alive;
        alive.reserve(segments.size());
        for (size_t i = 0; i < segments.size(); ++i)
        {
            if (graph.edgeAlive[i]) alive.push_back(std::move(segments[i]));
        }
        plan.segments = std::move(alive);
    }


    //══════════════════════════════════════════════════════════════════
    // 10. 다리 후보 수집 — 강(CityRiver/CitySea) 가로지르는 픽셀 좌표 + 방향
    //══════════════════════════════════════════════════════════════════
    //   도시 픽셀을 셔플 순회하며 강 픽셀의 위아래/좌우 도시 인접을 검사.
    //   - 강 두께 1픽셀만 지원 (위↔아래 또는 좌↔우가 바로 도시인 경우)
    //     TODO: 강 두께 ≥ 2픽셀 지원 — 연속 강 픽셀 추적 후 양 끝 도시까지 잇기
    //   - BRIDGE_MIN_GAP_PX 이내에 이미 다리 있으면 스킵 → 간격 균등
    //   - 확률 게이트 (첫 다리는 면제 → 강 전체 0개 다리 케이스 회피)
    //
    //   plan.segments에 push하지 *않음* — 다리는 z+1 deck + ramp 시스템(16단계)으로
    //   페인트하므로 11-15단계의 평면 도로 페인트(z=0 아스팔트 일자) 대상이 아님.
    //   대신 plan.bridges에 폴리라인으로 수집 (Map.ixx 시각화도 본 채널 소비).
    //   NS 다리: 수직 폴리라인 (verts[0].x == verts[1].x)
    //   WE 다리: 수평 폴리라인 (verts[0].y == verts[1].y)
    //══════════════════════════════════════════════════════════════════

    constexpr double BRIDGE_P = 0.5;
    constexpr int BRIDGE_MIN_GAP_PX = 4;

    auto isCityWater = [&](worldGrid::PixelCoord p) -> bool {
        const auto t = cityPixelAt(p);
        return t == worldGrid::Terrain::CityRiver
            || t == worldGrid::Terrain::CitySea;
        };

    //도시 사각형 내부이고 물이 아닌 픽셀 (다리 끝점 자격)
    auto isCityLand = [&](worldGrid::PixelCoord p) -> bool {
        bool inRect = false;
        for (const city::CityRect& r : node.rectangles)
            if (p.x >= r.px && p.x < r.x1() && p.y >= r.py && p.y < r.y1())
            { inRect = true; break; }
        if (!inRect) return false;
        const auto t = cityPixelAt(p);
        return t != worldGrid::Terrain::CityRiver
            && t != worldGrid::Terrain::CitySea
            && t != worldGrid::Terrain::Sea
            && t != worldGrid::Terrain::River
            && t != worldGrid::Terrain::Lake;
        };

    //도시 박스 내부(마진 1픽셀 제외) 픽셀 셔플 — 인접 ±1 검사가 마진 안에 머묾
    std::vector<worldGrid::PixelCoord> bridgePixels;
    for (int dy = 1; dy < patchH - 1; ++dy)
        for (int dx = 1; dx < patchW - 1; ++dx)
            bridgePixels.push_back({ patchPxX + dx, patchPxY + dy, node.center.z });
    std::shuffle(bridgePixels.begin(), bridgePixels.end(), rng);

    std::vector<worldGrid::PixelCoord> placedBridges;
    bool firstBridge = true;

    for (const auto& p : bridgePixels)
    {
        if (!isCityWater(p)) continue;

        const bool ns = isCityLand({ p.x, p.y - 1, p.z }) && isCityLand({ p.x, p.y + 1, p.z });
        const bool we = isCityLand({ p.x - 1, p.y, p.z }) && isCityLand({ p.x + 1, p.y, p.z });
        if (!ns && !we) continue;

        //같은 z에서 기존 다리와 최소 간격 검사
        bool tooClose = false;
        for (const auto& b : placedBridges)
        {
            if (b.z != p.z) continue;
            if (std::abs(b.x - p.x) <= BRIDGE_MIN_GAP_PX &&
                std::abs(b.y - p.y) <= BRIDGE_MIN_GAP_PX)
            {
                tooClose = true; break;
            }
        }
        if (tooClose) continue;

        //확률 게이트 (첫 다리는 면제)
        if (!firstBridge && std::uniform_real_distribution<double>{0.0, 1.0}(rng) >= BRIDGE_P) continue;
        firstBridge = false;

        //plan.bridges 폴리라인 — 양 옆 픽셀 중심까지 96타일 직선
        const int z = p.z;
        if (ns)
        {
            const int x = p.x * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X + worldGrid::TILES_PER_PIXEL / 2;
            const int y0 = (p.y - 1) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y + worldGrid::TILES_PER_PIXEL / 2;
            const int y1 = (p.y + 1) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y + worldGrid::TILES_PER_PIXEL / 2;
            plan.bridges.push_back(worldGen::RoadPolyLine{ .verts = { {x, y0, z}, {x, y1, z} } });
        }
        else
        {
            const int y = p.y * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y + worldGrid::TILES_PER_PIXEL / 2;
            const int x0 = (p.x - 1) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X + worldGrid::TILES_PER_PIXEL / 2;
            const int x1 = (p.x + 1) * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X + worldGrid::TILES_PER_PIXEL / 2;
            plan.bridges.push_back(worldGen::RoadPolyLine{ .verts = { {x0, y, z}, {x1, y, z} } });
        }

        placedBridges.push_back(p);
    }


    //══════════════════════════════════════════════════════════════════
    // 11. 그래프 재구축 — T-junction 분할 + adjacency 빌드
    //══════════════════════════════════════════════════════════════════
    //   8단계 지터링은 segments[].verts만 옮기고 graph 노드/엣지는 그대로 둬서
    //   inconsistent해진 상태. 또 지터링된 끝점이 잠긴 직교 도로 본체 중간에
    //   닿아 새 T-junction이 생기고, 10단계 다리 끝점도 강변 도로 본체에 닿을
    //   수 있음. 최종 도로망(plan.segments) 기준으로 그래프를 처음부터 다시 빌드.
    //
    //   1) 모든 세그먼트 끝점 수집
    //   2) 각 세그먼트 본체(strictly between)에 닿는 다른 끝점을 분할점으로 모음
    //   3) 분할점에서 세그먼트를 자름 → plan.segments 갱신
    //   4) 갈라진 세그먼트들로 노드 + adjacency 빌드
    //
    //   결과: 교차점은 노드, 도로 구간은 엣지. 노드 차수로 교차 형태 분류
    //   (1=끝, 2직선=직진, 2꺾임=L, 3=T, 4=십자). 횡단보도/블록검출 등에 사용.
    //══════════════════════════════════════════════════════════════════

    {
        std::vector<Point3> allEndpoints;
        allEndpoints.reserve(plan.segments.size() * 2);
        for (const auto& s : plan.segments)
        {
            allEndpoints.push_back(s.verts[0]);
            allEndpoints.push_back(s.verts[1]);
        }

        std::vector<worldGen::RoadPolyLine> splitSegments;
        splitSegments.reserve(plan.segments.size() * 2);

        for (const auto& s : plan.segments)
        {
            const Point3& a = s.verts[0];
            const Point3& b = s.verts[1];
            const bool horizontal = (a.y == b.y);
            const int z = a.z;

            if (horizontal)
            {
                const int x0 = std::min(a.x, b.x);
                const int x1 = std::max(a.x, b.x);
                const int y = a.y;

                std::vector<int> splitsX;
                for (const auto& p : allEndpoints)
                {
                    if (p.z != z || p.y != y) continue;
                    if (p.x > x0 && p.x < x1) splitsX.push_back(p.x);
                }
                std::sort(splitsX.begin(), splitsX.end());
                splitsX.erase(std::unique(splitsX.begin(), splitsX.end()), splitsX.end());

                int prev = x0;
                for (int cx : splitsX)
                {
                    splitSegments.push_back(worldGen::RoadPolyLine{ .verts = { {prev, y, z}, {cx, y, z} } });
                    prev = cx;
                }
                splitSegments.push_back(worldGen::RoadPolyLine{ .verts = { {prev, y, z}, {x1, y, z} } });
            }
            else //vertical
            {
                const int y0 = std::min(a.y, b.y);
                const int y1 = std::max(a.y, b.y);
                const int x = a.x;

                std::vector<int> splitsY;
                for (const auto& p : allEndpoints)
                {
                    if (p.z != z || p.x != x) continue;
                    if (p.y > y0 && p.y < y1) splitsY.push_back(p.y);
                }
                std::sort(splitsY.begin(), splitsY.end());
                splitsY.erase(std::unique(splitsY.begin(), splitsY.end()), splitsY.end());

                int prev = y0;
                for (int cy : splitsY)
                {
                    splitSegments.push_back(worldGen::RoadPolyLine{ .verts = { {x, prev, z}, {x, cy, z} } });
                    prev = cy;
                }
                splitSegments.push_back(worldGen::RoadPolyLine{ .verts = { {x, prev, z}, {x, y1, z} } });
            }
        }
        plan.segments = std::move(splitSegments);
    }

    //그래프 빌드 — 6단계와 동일 패턴, 새 plan.segments 기준
    graph = Graph{};
    coordToNode.clear();
    for (size_t i = 0; i < plan.segments.size(); ++i)
    {
        const int a = getOrCreateNode(plan.segments[i].verts[0]);
        const int b = getOrCreateNode(plan.segments[i].verts[1]);
        const int e = static_cast<int>(graph.edgeToNodes.size());
        graph.edgeToNodes.push_back({ a, b });
        graph.adjacency[a].push_back(e);
        graph.adjacency[b].push_back(e);
        graph.edgeAlive.push_back(true);
        graph.edgePreserved.push_back(false);
    }


    //══════════════════════════════════════════════════════════════════
    // 12. 도로망 세그먼트에 따라 실제 블랙아스팔트 타일 배치 시작
    //══════════════════════════════════════════════════════════════════

    for (const worldGen::RoadPolyLine& seg : plan.segments)
    {
        const Point3& a = seg.verts[0];
        const Point3& b = seg.verts[1];
        const bool horizontal = (a.y == b.y);

        const int x0 = std::min(a.x, b.x);
        const int x1 = std::max(a.x, b.x);
        const int y0 = std::min(a.y, b.y);
        const int y1 = std::max(a.y, b.y);
        const int z = a.z;

        constexpr int ROAD_HALF_WIDTH = 7; //중심 1타일 + 좌우 7타일 = 두께 15

        //끝점에서 진행 방향으로도 ROAD_HALF_WIDTH만큼 연장 → 코너 사각공백 방지 (square cap)
        if (horizontal)
        {
            for (int x = x0 - ROAD_HALF_WIDTH; x <= x1 + ROAD_HALF_WIDTH; ++x)
                for (int dy = -ROAD_HALF_WIDTH; dy <= ROAD_HALF_WIDTH; ++dy)
                    plan.tiles.push_back(CityTile{
                        .pos = Point3{ x, y0 + dy, z },
                        .floor = static_cast<std::uint16_t>(itemID::blackAsphalt),
                        });
        }
        else
        {
            for (int y = y0 - ROAD_HALF_WIDTH; y <= y1 + ROAD_HALF_WIDTH; ++y)
                for (int dx = -ROAD_HALF_WIDTH; dx <= ROAD_HALF_WIDTH; ++dx)
                    plan.tiles.push_back(CityTile{
                        .pos = Point3{ x0 + dx, y, z },
                        .floor = static_cast<std::uint16_t>(itemID::blackAsphalt),
                        });
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 13. 횡단보도 — 교차점(T자/십자)의 진입 도로마다 1개씩
    //══════════════════════════════════════════════════════════════════
    //   degree ≥ 3 노드의 각 인접 엣지에 횡단보도 1개. degree ≤ 2는 불필요.
    //
    //   모양 (도로방향 6 × 도로폭 15, 두 개의 3-깊이 띠):
    //     안쪽 띠 walkDir = +perp, 바깥 띠 = -perp (우측보행 관습).
    //     띠 끝 컬럼(walkDir 반대): alongIdx 0/2에 화살표, 1은 검은 아스팔트.
    //     나머지 14×3: half 스프라이트를 cross 축에서 교대 → 인접 타일 쌍의
    //                  흰 반쪽이 마주봐 1타일 폭 흰 스트라이프 7개 형성.
    //
    //   짧은 세그먼트(L < 27)는 양쪽 횡단보도가 겹치므로 작은 인덱스 노드 쪽만 그림.
    //══════════════════════════════════════════════════════════════════

    constexpr int CW_ROAD_HALF = 7;       //도로폭 절반 (= ROAD_HALF_WIDTH)
    constexpr int CW_INT_GAP = 1;         //교차점 본체와 횡단보도 사이 간격
    constexpr int CW_BAND_DEPTH = 3;      //한 띠의 along 깊이
    constexpr int CW_REACH = CW_ROAD_HALF + CW_INT_GAP + 2 * CW_BAND_DEPTH - 1;  // = 13, 노드 중심에서 횡단보도 끝까지 along 거리
    constexpr int CW_MIN_BOTH_END_LEN = 2 * CW_REACH + 1;                        // = 27, 양 끝 모두 그리려면 필요한 최소 세그먼트 길이

    for (size_t n = 0; n < graph.nodeCoord.size(); ++n)
    {
        if (graph.adjacency[n].size() < 3) continue;
        const Point3 c = graph.nodeCoord[n];

        for (int e : graph.adjacency[n])
        {
            const int otherN = (graph.edgeToNodes[e][0] == static_cast<int>(n))
                ? graph.edgeToNodes[e][1] : graph.edgeToNodes[e][0];
            const Point3 o = graph.nodeCoord[otherN];

            //지터링으로 좁아진 블록(예: 9타일 = L=24)에서 양쪽 횡단보도가 겹치는 것 방지.
            //양 끝 모두 교차점이고 L < 27이면 인덱스 작은 노드만 그림 — 한쪽 끝은 정상 횡단보도 보존.
            const int segLen = std::abs(o.x - c.x) + std::abs(o.y - c.y);
            if (segLen < CW_MIN_BOTH_END_LEN && graph.adjacency[otherN].size() >= 3
                && static_cast<int>(n) > otherN) continue;

            //S = (sx, sy): 노드에서 이 엣지의 다른 끝으로 향하는 단위벡터 (한 축만 ±1)
            const int sx = (o.x > c.x) - (o.x < c.x);
            const int sy = (o.y > c.y) - (o.y < c.y);

            //perp = (px, py): S를 우측보행 90° 회전 (y-down CW)
            const int px = -sy;
            const int py = sx;

            //+cross 방향에 따른 half 스프라이트 (흰 반쪽이 +cross 쪽인 타일)
            std::uint16_t highSide, lowSide;
            if      (px > 0) { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltRightHalf);  lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltLeftHalf); }
            else if (px < 0) { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltLeftHalf);   lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltRightHalf); }
            else if (py > 0) { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltBottomHalf); lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltTopHalf); }
            else             { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltTopHalf);    lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltBottomHalf); }

            for (int band = 0; band < 2; ++band)
            {
                const int alongBase = CW_ROAD_HALF + CW_INT_GAP + band * CW_BAND_DEPTH;  // 8, 11
                const int walkSign = (band == 0) ? +1 : -1;
                const int wx = walkSign * px;
                const int wy = walkSign * py;
                const int arrowCrossOff = -walkSign * CW_ROAD_HALF;

                std::uint16_t arrowSprite;
                if      (wx > 0) arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowR);
                else if (wx < 0) arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowL);
                else if (wy > 0) arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowD);
                else             arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowU);

                for (int alongIdx = 0; alongIdx < CW_BAND_DEPTH; ++alongIdx)
                {
                    for (int crossOff = -CW_ROAD_HALF; crossOff <= CW_ROAD_HALF; ++crossOff)
                    {
                        std::uint16_t sprite;
                        if (crossOff == arrowCrossOff)
                        {
                            if (alongIdx == 1) continue;  //화살표 컬럼 가운데행은 비워둠
                            sprite = arrowSprite;
                        }
                        else
                        {
                            const bool isHigh = (walkSign > 0) == ((crossOff & 1) == 0);
                            sprite = isHigh ? highSide : lowSide;
                        }

                        const int tx = c.x + (alongBase + alongIdx) * sx + crossOff * px;
                        const int ty = c.y + (alongBase + alongIdx) * sy + crossOff * py;
                        plan.tiles.push_back(CityTile{ .pos = { tx, ty, c.z }, .floor = sprite });
                    }
                }
            }
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 14. 도로 중앙 노란 점선 — 13단계 횡단보도 뒤에 push (안전망)
    //══════════════════════════════════════════════════════════════════
    //   도로 중심선에 3타일 dash + 3타일 gap, 월드좌표 6배수 정렬 → 세그먼트 간 연속.
    //   dash 전체가 [axisMin, axisMax] 안에 들어올 때만 배치 → 잘림 방지.
    //
    //   각 끝점 axis 오프셋 (그래프 degree 기반, 13단계 횡단보도 룰과 일관):
    //     degree ≥ 3 + 횡단보도 그려진 끝: 14  (횡단보도 너머)
    //     degree ≥ 3 + 횡단보도 스킵된 끝:  8  (교차점만 너머)
    //     degree = 2, 직교 (L자 코너):     2  (1 dash 더 가깝게 — 빈 공간 축소)
    //     degree = 2, 같은 축 (+→| 케이스): 0  (끊김 없이 통과)
    //     degree ≤ 1 (막다른 길/진입점):    0
    //══════════════════════════════════════════════════════════════════

    constexpr int DASH_PERIOD = 6;
    constexpr int DASH_YELLOW_LEN = 3;
    constexpr int LANE_INT_OFFSET = CW_ROAD_HALF + CW_INT_GAP;                    // = 8
    constexpr int LANE_CW_OFFSET = CW_ROAD_HALF + CW_INT_GAP + 2 * CW_BAND_DEPTH; // = 14
    constexpr int LANE_CORNER_OFFSET = LANE_INT_OFFSET - DASH_PERIOD;             // = 2, L자 코너는 1 dash 더 가깝게

    //@brief edge의 한 endpoint에서 axis 오프셋 (횡단보도 유무를 13단계와 동일 룰로 판정)
    auto laneOffsetAt = [&](size_t edgeIdx, int nodeIdx) -> int {
        const int deg = static_cast<int>(graph.adjacency[nodeIdx].size());
        if (deg < 2) return 0;
        if (deg == 2)
        {
            //같은 축(직진 통과) vs 직교(L자 코너) 구분
            const int e0 = graph.adjacency[nodeIdx][0];
            const int e1 = graph.adjacency[nodeIdx][1];
            const bool h0 = (plan.segments[e0].verts[0].y == plan.segments[e0].verts[1].y);
            const bool h1 = (plan.segments[e1].verts[0].y == plan.segments[e1].verts[1].y);
            return (h0 == h1) ? 0 : LANE_CORNER_OFFSET;
        }
        const int otherN = (graph.edgeToNodes[edgeIdx][0] == nodeIdx)
            ? graph.edgeToNodes[edgeIdx][1] : graph.edgeToNodes[edgeIdx][0];
        const Point3 cn = graph.nodeCoord[nodeIdx];
        const Point3 on = graph.nodeCoord[otherN];
        const int segLen = std::abs(on.x - cn.x) + std::abs(on.y - cn.y);
        const int otherDeg = static_cast<int>(graph.adjacency[otherN].size());
        const bool cwSkipped = (segLen < CW_MIN_BOTH_END_LEN && otherDeg >= 3 && nodeIdx > otherN);
        return cwSkipped ? LANE_INT_OFFSET : LANE_CW_OFFSET;
        };

    for (size_t ei = 0; ei < plan.segments.size(); ++ei)
    {
        const auto& seg = plan.segments[ei];
        const Point3& a = seg.verts[0];
        const Point3& b = seg.verts[1];
        const bool horizontal = (a.y == b.y);
        const int z = a.z;

        const int axisStart = horizontal ? std::min(a.x, b.x) : std::min(a.y, b.y);
        const int axisEnd   = horizontal ? std::max(a.x, b.x) : std::max(a.y, b.y);
        const int perpCoord = horizontal ? a.y : a.x;  //도로 중심선의 cross 좌표

        //axisStart에 있는 그래프 node 식별
        const int n0 = graph.edgeToNodes[ei][0];
        const int n1 = graph.edgeToNodes[ei][1];
        const int n0Axis = horizontal ? graph.nodeCoord[n0].x : graph.nodeCoord[n0].y;
        const int startNode = (n0Axis == axisStart) ? n0 : n1;
        const int endNode   = (n0Axis == axisStart) ? n1 : n0;

        const int axisMin = axisStart + laneOffsetAt(ei, startNode);
        const int axisMax = axisEnd - laneOffsetAt(ei, endNode);

        //axisMin 이상의 첫 6의 배수
        const int firstDash = static_cast<int>(std::ceil(static_cast<double>(axisMin) / DASH_PERIOD)) * DASH_PERIOD;

        for (int ds = firstDash; ds + DASH_YELLOW_LEN - 1 <= axisMax; ds += DASH_PERIOD)
        {
            for (int k = 0; k < DASH_YELLOW_LEN; ++k)
            {
                const Point3 pos = horizontal ? Point3{ ds + k, perpCoord, z } : Point3{ perpCoord, ds + k, z };
                plan.tiles.push_back(CityTile{
                    .pos = pos,
                    .floor = static_cast<std::uint16_t>(itemID::yellowAsphalt),
                    });
            }
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 15. 도로 주변 3타일 paver 보도 — 인도/연석 영역
    //══════════════════════════════════════════════════════════════════
    //   12단계 도로 본체(15폭 + 양끝 square cap) 외곽 3타일을 paver로 채움.
    //   T-junction/+자에서 다른 도로 본체에 paver가 닿는 부분은 스킵 → asphalt 보존
    //   (지금까지 push된 도로 타일 위치 집합으로 판정).
    //
    //   stage 15이므로 stage 12-14 이후 실행 — 도로 타일 위로 paver가 덮어쓰지
    //   않도록 명시적 필터링 필요. 12-14가 push한 모든 타일(아스팔트/횡단보도/노란선)
    //   위치를 set으로 모아 룩업.
    //══════════════════════════════════════════════════════════════════

    constexpr int PAVER_BORDER = 3;
    constexpr int PAVER_EXPAND = 7 + PAVER_BORDER;  // = 10, 도로 반폭 + 보도

    std::unordered_set<Point3, Point3::Hash> roadTiles;
    roadTiles.reserve(plan.tiles.size());
    for (const auto& t : plan.tiles) roadTiles.insert(t.pos);

    for (const worldGen::RoadPolyLine& seg : plan.segments)
    {
        const Point3& a = seg.verts[0];
        const Point3& b = seg.verts[1];
        const bool horizontal = (a.y == b.y);

        const int x0 = std::min(a.x, b.x);
        const int x1 = std::max(a.x, b.x);
        const int y0 = std::min(a.y, b.y);
        const int y1 = std::max(a.y, b.y);
        const int z = a.z;

        if (horizontal)
        {
            for (int x = x0 - PAVER_EXPAND; x <= x1 + PAVER_EXPAND; ++x)
                for (int dy = -PAVER_EXPAND; dy <= PAVER_EXPAND; ++dy)
                {
                    const Point3 pos{ x, y0 + dy, z };
                    if (roadTiles.contains(pos)) continue;
                    plan.tiles.push_back(CityTile{
                        .pos = pos,
                        .floor = static_cast<std::uint16_t>(itemID::paver),
                        });
                }
        }
        else
        {
            for (int y = y0 - PAVER_EXPAND; y <= y1 + PAVER_EXPAND; ++y)
                for (int dx = -PAVER_EXPAND; dx <= PAVER_EXPAND; ++dx)
                {
                    const Point3 pos{ x0 + dx, y, z };
                    if (roadTiles.contains(pos)) continue;
                    plan.tiles.push_back(CityTile{
                        .pos = pos,
                        .floor = static_cast<std::uint16_t>(itemID::paver),
                        });
                }
        }
    }

    //══════════════════════════════════════════════════════════════════
    // 16. 다리 페인트 — z+1 deck + ramp + guardrail + pillar wall + z=0 진입
    //══════════════════════════════════════════════════════════════════
    //   startArea의 ramp 다리 시스템을 도시 자동생성에 통합 (10단계가 수집한
    //   plan.bridges 소비). 폴리라인 endpoint(yA/yB 또는 xA/xB)는 도시 도로 그리드의
    //   T-junction node(픽셀 중심) — 거기서 ramp까지 15타일 떨어진 위치에 배치한다.
    //
    //     T-junction node (yA)            ← 동서 도로 본체 중심 (12단계 cap이 ±7 연장)
    //         │ cap end (yA+7)             ← 동서 도로 body의 남쪽 끝
    //         │     ... (15단계 paver 보도 cap, 3행)
    //         │ 진입로 시작 (yA+11)        ← z=0 5타일 진입 zone (paver 사이드 + 아스팔트 센터)
    //         │     ...
    //         │ ramp (yA+15)               ← rampUp 프롭 (21폭), z=0/z+1 전환
    //         │ pillar wall (yA+16)        ← 21폭 wall, 다리 밑 진입 차단
    //         │ deck interior (z+1)        ← 길이 yB-yA-30
    //         │ pillar wall (yB-16)
    //         │ ramp (yB-15)
    //         │ 진입로 끝 (yB-11)
    //         │ ...
    //         │ T-junction node (yB)
    //
    //   왜 endpoint에서 15 떨어뜨리는가:
    //     ramp/pillar wall을 endpoint(yA)에 두면 양쪽 동서 도로 body(yA-7 ~ yA+7)에
    //     겹쳐 차량 통행 차단. cap end(7) + 진입 5 + 추가 3(다리 본체 6타일 축소) = 15.
    //
    //   가드레일 위치:
    //     outermost paver 타일(xc±10)에 배치 — startArea 패턴. 3타일 축소로 진입로가
    //     stage 15의 perpendicular paver cap(yA+8~yA+10)을 침범하지 않으므로 직교
    //     인도 통행 방해 없음.
    //
    //   진입로와 동서 도로 사이 갭 처리:
    //     stage 15 paver cap이 yA+8~yA+10에 3행 paver를 깔아 도로가 끊긴 듯 보임.
    //     → 다리 endpoint 방향으로 black asphalt 센터(15폭) 연장 + 13단계와 동일한
    //       횡단보도 1세트 그려서 자연스러운 T-junction 형성.
    //
    //   T-junction 그래프 연결:
    //     bridges는 plan.segments에 없으므로 11단계 그래프 노드로 안 잡힘. 즉 yA는
    //     E+W+북쪽 연결 도로 = degree 3 T-junction (또는 +가 되면 degree 4). 다리는
    //     ramp prop이 z 전환을 담당하므로 그래프 노드 등록 불필요.
    //
    //   주의: CityPlan 4단계가 z=groundZ tile은 dense PaintCell, z=deckZ tile은
    //   sparse SectorSkyTile로 라우팅 (Sector.ixx 참조).
    //══════════════════════════════════════════════════════════════════

    constexpr int BRIDGE_HALF_WIDTH = 7;    //도로폭 절반 (12단계와 동일)
    constexpr int BRIDGE_PAVER = 3;          //paver 보도 (15단계와 동일)
    constexpr int BRIDGE_TOTAL_HALF = BRIDGE_HALF_WIDTH + BRIDGE_PAVER;  // = 10, 가드레일 위치 = outermost paver 타일
    constexpr int BRIDGE_APPROACH_LEN = 5;  //z=0 진입 zone 5타일 (startArea와 동일)
    constexpr int BRIDGE_RAMP_OFFSET = BRIDGE_HALF_WIDTH + BRIDGE_APPROACH_LEN + 3;  // = 15, 본체 6타일 축소

    for (const worldGen::RoadPolyLine& bridge : plan.bridges)
    {
        if (bridge.verts.size() < 2) continue;
        const Point3& va = bridge.verts[0];
        const Point3& vb = bridge.verts[1];
        const bool bridgeNS = (va.x == vb.x);  // 수직 = NS
        const int groundZ = va.z;
        const int deckZ   = va.z + 1;

        //@brief deck floor — 중심 asphalt + 사이드 paver
        auto pushDeckFloor = [&](int x, int y, int crossOff) {
            const std::uint16_t floor = (std::abs(crossOff) > BRIDGE_HALF_WIDTH)
                ? static_cast<std::uint16_t>(itemID::paver)
                : static_cast<std::uint16_t>(itemID::blackAsphalt);
            plan.tiles.push_back(CityTile{ .pos = {x, y, deckZ}, .floor = floor });
            };

        //@brief 다리 T-junction 횡단보도 — 13단계와 동일 로직, 다리 방향용
        //   bridge polyline endpoint(yA/yB)에서 다리 방향(sx,sy)으로 횡단보도 1세트.
        //   bridges는 plan.segments에 없어서 13단계가 다리 방향 crosswalk를 안 그리므로
        //   여기서 직접 페인트. 위치/sprite 선택 룰은 13단계와 동일.
        auto paintBridgeCrosswalk = [&](Point3 c, int sx, int sy) {
            constexpr int CW_ROAD_HALF = 7;
            constexpr int CW_INT_GAP = 1;
            constexpr int CW_BAND_DEPTH = 3;

            const int px = -sy;
            const int py = sx;

            std::uint16_t highSide, lowSide;
            if      (px > 0) { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltRightHalf);  lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltLeftHalf); }
            else if (px < 0) { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltLeftHalf);   lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltRightHalf); }
            else if (py > 0) { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltBottomHalf); lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltTopHalf); }
            else             { highSide = static_cast<std::uint16_t>(itemID::whiteAsphaltTopHalf);    lowSide = static_cast<std::uint16_t>(itemID::whiteAsphaltBottomHalf); }

            for (int band = 0; band < 2; ++band)
            {
                const int alongBase = CW_ROAD_HALF + CW_INT_GAP + band * CW_BAND_DEPTH;
                const int walkSign = (band == 0) ? +1 : -1;
                const int wx = walkSign * px;
                const int wy = walkSign * py;
                const int arrowCrossOff = -walkSign * CW_ROAD_HALF;

                std::uint16_t arrowSprite;
                if      (wx > 0) arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowR);
                else if (wx < 0) arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowL);
                else if (wy > 0) arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowD);
                else             arrowSprite = static_cast<std::uint16_t>(itemID::whiteAsphaltArrowU);

                for (int alongIdx = 0; alongIdx < CW_BAND_DEPTH; ++alongIdx)
                {
                    for (int crossOff = -CW_ROAD_HALF; crossOff <= CW_ROAD_HALF; ++crossOff)
                    {
                        std::uint16_t sprite;
                        if (crossOff == arrowCrossOff)
                        {
                            if (alongIdx == 1) continue;
                            sprite = arrowSprite;
                        }
                        else
                        {
                            const bool isHigh = (walkSign > 0) == ((crossOff & 1) == 0);
                            sprite = isHigh ? highSide : lowSide;
                        }

                        const int tx = c.x + (alongBase + alongIdx) * sx + crossOff * px;
                        const int ty = c.y + (alongBase + alongIdx) * sy + crossOff * py;
                        plan.tiles.push_back(CityTile{ .pos = { tx, ty, c.z }, .floor = sprite });
                    }
                }
            }
            };

        //@brief z=0 진입로 한 줄 — paver 사이드 + asphalt 센터 + outermost paver 위 가드레일
        //   진입로가 3타일 안쪽으로 당겨졌으므로 perpendicular 인도(stage 15 paver cap)에는
        //   영향 없음 → 가드레일은 다시 outermost paver 타일(xc±10)에 배치 가능.
        auto pushApproachRowNS = [&](int xc, int y) {
            for (int dx = -BRIDGE_TOTAL_HALF; dx <= BRIDGE_TOTAL_HALF; ++dx)
            {
                const std::uint16_t floor = (std::abs(dx) > BRIDGE_HALF_WIDTH)
                    ? static_cast<std::uint16_t>(itemID::paver)
                    : static_cast<std::uint16_t>(itemID::blackAsphalt);
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, y, groundZ}, .floor = floor });
            }
            plan.tiles.push_back(CityTile{ .pos = {xc - BRIDGE_TOTAL_HALF, y, groundZ},
                                           .wall = static_cast<std::uint16_t>(itemID::guardrail) });
            plan.tiles.push_back(CityTile{ .pos = {xc + BRIDGE_TOTAL_HALF, y, groundZ},
                                           .wall = static_cast<std::uint16_t>(itemID::guardrail) });
            };
        auto pushApproachRowWE = [&](int yc, int x) {
            for (int dy = -BRIDGE_TOTAL_HALF; dy <= BRIDGE_TOTAL_HALF; ++dy)
            {
                const std::uint16_t floor = (std::abs(dy) > BRIDGE_HALF_WIDTH)
                    ? static_cast<std::uint16_t>(itemID::paver)
                    : static_cast<std::uint16_t>(itemID::blackAsphalt);
                plan.tiles.push_back(CityTile{ .pos = {x, yc + dy, groundZ}, .floor = floor });
            }
            plan.tiles.push_back(CityTile{ .pos = {x, yc - BRIDGE_TOTAL_HALF, groundZ},
                                           .wall = static_cast<std::uint16_t>(itemID::guardrail) });
            plan.tiles.push_back(CityTile{ .pos = {x, yc + BRIDGE_TOTAL_HALF, groundZ},
                                           .wall = static_cast<std::uint16_t>(itemID::guardrail) });
            };

        if (bridgeNS)  //North-South 수직 다리
        {
            const int xc = va.x;
            const int yA = std::min(va.y, vb.y);
            const int yB = std::max(va.y, vb.y);
            const int rampN_y = yA + BRIDGE_RAMP_OFFSET;
            const int rampS_y = yB - BRIDGE_RAMP_OFFSET;

            //── Deck floor (z+1) — rampN_y ~ rampS_y ──
            for (int y = rampN_y; y <= rampS_y; ++y)
                for (int dx = -BRIDGE_TOTAL_HALF; dx <= BRIDGE_TOTAL_HALF; ++dx)
                    pushDeckFloor(xc + dx, y, dx);

            //── Guardrail walls (z+1) — deck 전길이 양 사이드 (outermost paver 타일) ──
            for (int y = rampN_y; y <= rampS_y; ++y)
            {
                plan.tiles.push_back(CityTile{ .pos = {xc - BRIDGE_TOTAL_HALF, y, deckZ},
                                               .wall = static_cast<std::uint16_t>(itemID::guardrail) });
                plan.tiles.push_back(CityTile{ .pos = {xc + BRIDGE_TOTAL_HALF, y, deckZ},
                                               .wall = static_cast<std::uint16_t>(itemID::guardrail) });
            }

            //── Ramps (양 끝, 21폭) — z=0 rampUp + z+1 rampDown ──
            for (int dx = -BRIDGE_TOTAL_HALF; dx <= BRIDGE_TOTAL_HALF; ++dx)
            {
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, rampN_y, groundZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampUp) });
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, rampN_y, deckZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampDown) });
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, rampS_y, groundZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampUp) });
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, rampS_y, deckZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampDown) });
            }

            //── Pillar walls (z=0) — ramp 안쪽 1타일, 21폭 ──
            for (int dx = -BRIDGE_TOTAL_HALF; dx <= BRIDGE_TOTAL_HALF; ++dx)
            {
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, rampN_y + 1, groundZ},
                                               .wall = static_cast<std::uint16_t>(itemID::pillarWall) });
                plan.tiles.push_back(CityTile{ .pos = {xc + dx, rampS_y - 1, groundZ},
                                               .wall = static_cast<std::uint16_t>(itemID::pillarWall) });
            }

            //── z=0 진입로 (5타일, ramp 행 포함) — paver 사이드 + asphalt 센터 + 가드레일 ──
            //   북쪽: yA+11 ~ rampN_y(=yA+15). 진입로가 3타일 안쪽으로 당겨짐.
            //   남쪽: rampS_y(=yB-15) ~ yB-11. 대칭.
            for (int y = rampN_y - BRIDGE_APPROACH_LEN + 1; y <= rampN_y; ++y)
                pushApproachRowNS(xc, y);
            for (int y = rampS_y; y <= rampS_y + BRIDGE_APPROACH_LEN - 1; ++y)
                pushApproachRowNS(xc, y);

            //── Asphalt 연장 (yA+8~yA+10, yB-10~yB-8) — endpoint 방향 도로 연속성 ──
            //   stage 15 paver cap이 깔린 3행을 black asphalt 센터(15폭)로 덮어 도로
            //   연속성 확보. paver 사이드(xc±10..xc±8)는 그대로 (stage 15가 유지).
            for (int y = yA + BRIDGE_HALF_WIDTH + 1; y <= yA + BRIDGE_HALF_WIDTH + 3; ++y)
                for (int dx = -BRIDGE_HALF_WIDTH; dx <= BRIDGE_HALF_WIDTH; ++dx)
                    plan.tiles.push_back(CityTile{ .pos = {xc + dx, y, groundZ},
                                                    .floor = static_cast<std::uint16_t>(itemID::blackAsphalt) });
            for (int y = yB - BRIDGE_HALF_WIDTH - 3; y <= yB - BRIDGE_HALF_WIDTH - 1; ++y)
                for (int dx = -BRIDGE_HALF_WIDTH; dx <= BRIDGE_HALF_WIDTH; ++dx)
                    plan.tiles.push_back(CityTile{ .pos = {xc + dx, y, groundZ},
                                                    .floor = static_cast<std::uint16_t>(itemID::blackAsphalt) });

            //── T-junction 횡단보도 — 다리 방향 1세트씩 ──
            paintBridgeCrosswalk(Point3{ xc, yA, groundZ }, 0, +1);   // 북 endpoint → 남쪽 방향
            paintBridgeCrosswalk(Point3{ xc, yB, groundZ }, 0, -1);   // 남 endpoint → 북쪽 방향

            //── Deck 노란 중앙 분리선 (z+1) — 14단계와 동일 6타일 주기(3 dash + 3 gap) ──
            //   x=xc 한 줄, y는 rampN_y ~ rampS_y. 월드좌표 6의 배수 정렬해 도로 grid와 호환.
            {
                constexpr int DASH_PERIOD = 6;
                constexpr int DASH_YELLOW_LEN = 3;
                const int firstDash = static_cast<int>(std::ceil(static_cast<double>(rampN_y) / DASH_PERIOD)) * DASH_PERIOD;
                for (int ds = firstDash; ds + DASH_YELLOW_LEN - 1 <= rampS_y; ds += DASH_PERIOD)
                {
                    for (int k = 0; k < DASH_YELLOW_LEN; ++k)
                    {
                        plan.tiles.push_back(CityTile{ .pos = { xc, ds + k, deckZ },
                                                       .floor = static_cast<std::uint16_t>(itemID::yellowAsphalt) });
                    }
                }
            }
        }
        else  //West-East 수평 다리
        {
            const int yc = va.y;
            const int xA = std::min(va.x, vb.x);
            const int xB = std::max(va.x, vb.x);
            const int rampW_x = xA + BRIDGE_RAMP_OFFSET;
            const int rampE_x = xB - BRIDGE_RAMP_OFFSET;

            //── Deck floor (z+1) ──
            for (int x = rampW_x; x <= rampE_x; ++x)
                for (int dy = -BRIDGE_TOTAL_HALF; dy <= BRIDGE_TOTAL_HALF; ++dy)
                    pushDeckFloor(x, yc + dy, dy);

            //── Guardrail walls (z+1) — deck 전길이 양 사이드 (outermost paver 타일) ──
            for (int x = rampW_x; x <= rampE_x; ++x)
            {
                plan.tiles.push_back(CityTile{ .pos = {x, yc - BRIDGE_TOTAL_HALF, deckZ},
                                               .wall = static_cast<std::uint16_t>(itemID::guardrail) });
                plan.tiles.push_back(CityTile{ .pos = {x, yc + BRIDGE_TOTAL_HALF, deckZ},
                                               .wall = static_cast<std::uint16_t>(itemID::guardrail) });
            }

            //── Ramps (양 끝, 21폭) ──
            for (int dy = -BRIDGE_TOTAL_HALF; dy <= BRIDGE_TOTAL_HALF; ++dy)
            {
                plan.tiles.push_back(CityTile{ .pos = {rampW_x, yc + dy, groundZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampUp) });
                plan.tiles.push_back(CityTile{ .pos = {rampW_x, yc + dy, deckZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampDown) });
                plan.tiles.push_back(CityTile{ .pos = {rampE_x, yc + dy, groundZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampUp) });
                plan.tiles.push_back(CityTile{ .pos = {rampE_x, yc + dy, deckZ},
                                               .prop = static_cast<std::uint16_t>(itemID::rampDown) });
            }

            //── Pillar walls (z=0) — ramp 안쪽 1타일 ──
            for (int dy = -BRIDGE_TOTAL_HALF; dy <= BRIDGE_TOTAL_HALF; ++dy)
            {
                plan.tiles.push_back(CityTile{ .pos = {rampW_x + 1, yc + dy, groundZ},
                                               .wall = static_cast<std::uint16_t>(itemID::pillarWall) });
                plan.tiles.push_back(CityTile{ .pos = {rampE_x - 1, yc + dy, groundZ},
                                               .wall = static_cast<std::uint16_t>(itemID::pillarWall) });
            }

            //── z=0 진입로 (5타일, ramp 열 포함) ──
            for (int x = rampW_x - BRIDGE_APPROACH_LEN + 1; x <= rampW_x; ++x)
                pushApproachRowWE(yc, x);
            for (int x = rampE_x; x <= rampE_x + BRIDGE_APPROACH_LEN - 1; ++x)
                pushApproachRowWE(yc, x);

            //── Asphalt 연장 (xA+8~xA+10, xB-10~xB-8) — endpoint 방향 도로 연속성 ──
            for (int x = xA + BRIDGE_HALF_WIDTH + 1; x <= xA + BRIDGE_HALF_WIDTH + 3; ++x)
                for (int dy = -BRIDGE_HALF_WIDTH; dy <= BRIDGE_HALF_WIDTH; ++dy)
                    plan.tiles.push_back(CityTile{ .pos = {x, yc + dy, groundZ},
                                                    .floor = static_cast<std::uint16_t>(itemID::blackAsphalt) });
            for (int x = xB - BRIDGE_HALF_WIDTH - 3; x <= xB - BRIDGE_HALF_WIDTH - 1; ++x)
                for (int dy = -BRIDGE_HALF_WIDTH; dy <= BRIDGE_HALF_WIDTH; ++dy)
                    plan.tiles.push_back(CityTile{ .pos = {x, yc + dy, groundZ},
                                                    .floor = static_cast<std::uint16_t>(itemID::blackAsphalt) });

            //── T-junction 횡단보도 — 다리 방향 1세트씩 ──
            paintBridgeCrosswalk(Point3{ xA, yc, groundZ }, +1, 0);   // 서 endpoint → 동쪽 방향
            paintBridgeCrosswalk(Point3{ xB, yc, groundZ }, -1, 0);   // 동 endpoint → 서쪽 방향

            //── Deck 노란 중앙 분리선 (z+1) — y=yc 한 줄, x는 rampW_x ~ rampE_x ──
            {
                constexpr int DASH_PERIOD = 6;
                constexpr int DASH_YELLOW_LEN = 3;
                const int firstDash = static_cast<int>(std::ceil(static_cast<double>(rampW_x) / DASH_PERIOD)) * DASH_PERIOD;
                for (int ds = firstDash; ds + DASH_YELLOW_LEN - 1 <= rampE_x; ds += DASH_PERIOD)
                {
                    for (int k = 0; k < DASH_YELLOW_LEN; ++k)
                    {
                        plan.tiles.push_back(CityTile{ .pos = { ds + k, yc, deckZ },
                                                       .floor = static_cast<std::uint16_t>(itemID::yellowAsphalt) });
                    }
                }
            }
        }
    }


    //══════════════════════════════════════════════════════════════════
    // 17. 가로수 — 도로변 인도(paver) 안쪽 타일에 식수, dirt 바닥
    //══════════════════════════════════════════════════════════════════
    //   사이드워크 3타일(perp ±8..±10) 중 도로에 가장 가까운 ±8 paver 위에 가로수.
    //   floor 교체: paver → dirt, prop: 가로수 종 RNG 추첨 (은행/벚/단풍/목련/참나무/향나무).
    //
    //   배치 의도: 가로수가 인도 바깥 경계(=도로 쪽)에 줄지어 차량이 인도로 돌진하는
    //   것을 물리적으로 차단. 교차로/횡단보도/다리 진입은 통행 살리도록 비워둠.
    //
    //   배제 처리 — 별도 분기 없이 점유 검사로 통합:
    //     - degree ≥ 3 노드(교차로) 근처: 노드에서 TREE_NODE_CLEARANCE 만큼 잘라냄
    //       → 교차로 코너/13단계 횡단보도 영역 비움
    //     - 다리 진입로·asphalt 연장·16단계 횡단보도: 16단계에서 paver를 덮어썼으므로
    //       finalFloor가 paver가 아님 → 자동 배제
    //     - 가드레일/필러월/램프 등 prop/wall 점유 타일도 occupiedPropWall로 자동 배제
    //
    //   다리 본체(z+1 deck)에는 어차피 paver가 ±8 위치에 없음(가드레일 자리) → 자동 배제.
    //══════════════════════════════════════════════════════════════════

    constexpr int TREE_PERIOD = 6;                          // 식수 주기 (도로 dash와 같은 6타일)
    constexpr int TREE_PERP_OFFSET = CW_ROAD_HALF + 1;      // = 8, 도로에 가장 가까운 paver 타일
    constexpr int TREE_NODE_CLEARANCE = CW_REACH + 2;       // = 15, 교차점에서 거리 (횡단보도 + 가드 버퍼)

    constexpr std::array<std::uint16_t, 6> STREET_TREE_KINDS = {
        static_cast<std::uint16_t>(itemID::ginkgoTree),
        static_cast<std::uint16_t>(itemID::cherryTree),
        static_cast<std::uint16_t>(itemID::mapleTree),
        static_cast<std::uint16_t>(itemID::magnoliaTree),
        static_cast<std::uint16_t>(itemID::oakTree),
        static_cast<std::uint16_t>(itemID::juniperTree),
    };

    //최종 floor + prop/wall 점유 맵 — 다리·교차·횡단보도가 paver 위에 덮은 자리 식별용.
    //  plan.tiles는 stage 순서대로 push되므로 마지막 floor 값이 그 위치의 최종 floor.
    std::unordered_map<Point3, std::uint16_t, Point3::Hash> finalFloor;
    std::unordered_set<Point3, Point3::Hash> occupiedPropWall;
    finalFloor.reserve(plan.tiles.size());
    for (const auto& t : plan.tiles)
    {
        if (t.floor != 0) finalFloor[t.pos] = t.floor;
        if (t.prop != 0 || t.wall != 0) occupiedPropWall.insert(t.pos);
    }

    for (size_t ei = 0; ei < plan.segments.size(); ++ei)
    {
        const auto& seg = plan.segments[ei];
        const Point3& a = seg.verts[0];
        const Point3& b = seg.verts[1];
        const bool horizontal = (a.y == b.y);
        const int z = a.z;

        const int axisStart = horizontal ? std::min(a.x, b.x) : std::min(a.y, b.y);
        const int axisEnd   = horizontal ? std::max(a.x, b.x) : std::max(a.y, b.y);
        const int perpCoord = horizontal ? a.y : a.x;

        //양 끝 노드 식별 — 14단계 laneOffsetAt와 동일 패턴
        const int n0 = graph.edgeToNodes[ei][0];
        const int n1 = graph.edgeToNodes[ei][1];
        const int n0Axis = horizontal ? graph.nodeCoord[n0].x : graph.nodeCoord[n0].y;
        const int startNode = (n0Axis == axisStart) ? n0 : n1;
        const int endNode   = (n0Axis == axisStart) ? n1 : n0;

        //교차점(degree ≥ 3)이면 clearance, 아니면 0 (직진/L자/막다른 길/진입점은 통과)
        const int clearStart = (graph.adjacency[startNode].size() >= 3) ? TREE_NODE_CLEARANCE : 0;
        const int clearEnd   = (graph.adjacency[endNode].size()   >= 3) ? TREE_NODE_CLEARANCE : 0;
        const int axisMin = axisStart + clearStart;
        const int axisMax = axisEnd   - clearEnd;
        if (axisMin > axisMax) continue;  // 짧은 세그먼트 — 클리어런스로 다 잘림

        //월드좌표 TREE_PERIOD 정렬 — 인접 세그먼트와 가로수 위치가 자연스럽게 연속
        const int firstAt = static_cast<int>(std::ceil(static_cast<double>(axisMin) / TREE_PERIOD)) * TREE_PERIOD;
        for (int axis = firstAt; axis <= axisMax; axis += TREE_PERIOD)
        {
            for (int side : { -1, +1 })
            {
                const int perp = perpCoord + side * TREE_PERP_OFFSET;
                const Point3 pos = horizontal ? Point3{ axis, perp, z } : Point3{ perp, axis, z };

                //paver 위에서만 식수 (다리·교차로 중심·횡단보도가 덮어쓴 자리는 paver 아님 → 자동 배제)
                auto it = finalFloor.find(pos);
                if (it == finalFloor.end() || it->second != static_cast<std::uint16_t>(itemID::paver)) continue;
                if (occupiedPropWall.contains(pos)) continue;

                const std::uint16_t kind = STREET_TREE_KINDS[
                    std::uniform_int_distribution<size_t>{0, STREET_TREE_KINDS.size() - 1}(rng)];

                plan.tiles.push_back(CityTile{
                    .pos = pos,
                    .floor = static_cast<std::uint16_t>(itemID::dirt),
                    .prop = kind,
                    });

                //같은 단계 내 중복 방지 — 두 colinear 세그먼트가 공유하는 노드 좌표에서
                //양쪽 모두 firstAt 글로벌 정렬 때문에 같은 (axis, side)를 시도할 수 있음.
                //createProp은 동일 좌표 두 번 호출 시 errorBox로 죽으므로 즉시 점유 등록.
                occupiedPropWall.insert(pos);
            }
        }
    }

    return plan;
}
