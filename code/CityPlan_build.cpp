module CityPlan;

import std;
import util;
import constVar;
import worldGen;
import worldGrid;

// ════════════════════════════════════════════════════════════════════════
// CityPlan_build.cpp — buildCityPlan 구현 (골격).
//
//   현재는 골격 — 생성 단계 진입 시 콘솔 1줄 출력 후 빈 CityPlan 반환.
//   향후 BCP·도로·블록·다리 생성 로직이 본 함수에 누적됨 (procGenerate 패턴).
//
//   주의: procGenerate(ProcGenWorker 스레드)가 CityPlanCache::getOrCompute 경유로
//   본 함수를 호출 — prt는 스레드 안전 보장 X. 골격 단계 디버그 출력 용도로만 사용.
// ════════════════════════════════════════════════════════════════════════

CityPlan buildCityPlan(city::CityId id, std::uint64_t seed)
{
    CityPlan plan{ id };



    // ── 플랜 설정 = plan.tiles에 깔고 싶은 타일을 push ────────────────────
    //   예) 도로 — x=20322, y=32012, z=1 에 아스팔트 바닥:
    //
    //     plan.tiles.push_back(CityTile{ Point3{ 20322, 32012, 1 },
    //                                    itemID::blackAsphalt,  // floor (constVar import 필요)
    //                                    0 });                  // wall: 0 = 안 건드림
    //
    //   향후 BCP가 도시 rect/도로 지오메트리로부터 이 리스트를 채우게 됨.
    //   procGenerate 4단계가 이 리스트를 읽어 PaintCell.floor/wall에 블릿.



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

    // node.rectangles: 이 도시를 이루는 직사각형들 (픽셀 좌표, 랜덤 배치 결과)
    for (const city::CityRect& r : node.rectangles)
    {
        // 픽셀 → 절대 실타일 변환
        const int tileX = r.px * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_X;
        const int tileY = r.py * worldGrid::TILES_PER_PIXEL + worldGrid::TILE_BASE_Y;
        const int tileW = r.w * worldGrid::TILES_PER_PIXEL;
        const int tileH = r.h * worldGrid::TILES_PER_PIXEL;

        // 이 직사각형 안에서 BCP/도로 로직 → 좌표는 전부 tileX/Y/W/H 에서 파생
        // 예: 직사각형 둘레를 도로로
        //   for (int x = tileX; x < tileX + tileW; ++x)
        //       plan.tiles.push_back(CityTile{ .pos = Point3{x, tileY, node.center.z},
        //                                      .floor = (uint16_t)itemID::blackAsphalt });
    }


    //══════════════════════════════════════════════════════════════════
    // 3. 가능한 모든 도로들 긋기부
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


    //── 7.5 제거 플래그 있는 도로 제거(동기화) ────────────────────────────────────
    {
        std::vector<worldGen::RoadPolyLine> alive;
        alive.reserve(segments.size());
        for (size_t i = 0; i < segments.size(); ++i)
        {
            if (graph.edgeAlive[i]) alive.push_back(std::move(segments[i]));
        }
        plan.segments = std::move(alive);
    }


    return plan;
}
