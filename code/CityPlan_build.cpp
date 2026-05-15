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
    // 5. 물타일과 인접한 세그먼트들 제거
    //══════════════════════════════════════════════════════════════════

    auto terrainAtTile = [&](Point3 tile) -> worldGrid::Terrain {
        const int px = (tile.x - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
        const int py = (tile.y - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;
        return cityPixelAt({ px, py, tile.z });
        };


    for (int i = 0; i <= 1; i++)
    {
        for (int del = -1; del <= 1; del += 2)
        {

            std::erase_if(segments, [&](const worldGen::RoadPolyLine& seg) {
                const Point3 outside{ seg.verts[i].x + del, seg.verts[i].y, seg.verts[i].z };
                const auto t = terrainAtTile(outside);
                return t == worldGrid::Terrain::CityRiver
                    || t == worldGrid::Terrain::CitySea
                    || t == worldGrid::Terrain::Sea
                    || t == worldGrid::Terrain::River
                    || t == worldGrid::Terrain::Lake;
                });


            std::erase_if(segments, [&](const worldGen::RoadPolyLine& seg) {
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
    // 6. 블록 생성 시작
    //══════════════════════════════════════════════════════════════════

    return plan;
}
