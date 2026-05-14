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
        static_cast<unsigned>(id), static_cast<unsigned long long>(seed),
        plan.tiles.size());


    const worldGen::CityNode& node = (*worldGen::activeCities)[static_cast<std::uint32_t>(id)];
   
    if (node.rectangles.empty()) return plan;

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

    auto cityPixelAt = [&](int px, int py) -> worldGrid::Terrain {
        return cityTerrainBox[static_cast<std::size_t>(py - patchPxY) * patchW + (px - patchPxX)];
        };


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

            for (const Point3& endpoint : { line.verts.front(), line.verts.back() })
            {
                const int epx = (endpoint.x - worldGrid::TILE_BASE_X) / worldGrid::TILES_PER_PIXEL;
                const int epy = (endpoint.y - worldGrid::TILE_BASE_Y) / worldGrid::TILES_PER_PIXEL;

                for (const city::CityRect& r : node.rectangles)
                {
                    const bool besideN = (epx >= r.px && epx < r.x1()) && (epy == r.py - 1);
                    const bool besideS = (epx >= r.px && epx < r.x1()) && (epy == r.y1());
                    const bool besideW = (epy >= r.py && epy < r.y1()) && (epx == r.px - 1);
                    const bool besideE = (epy >= r.py && epy < r.y1()) && (epx == r.x1());

                    if (besideN)
                    {
                        entryPoints.push_back({ Point3{ endpoint.x, endpoint.y + 1, 0},cutDir::vertical });
                        break;
                    }
                    else if (besideS)
                    {
                        entryPoints.push_back({ Point3{ endpoint.x, endpoint.y - 1, 0 }, cutDir::vertical });
                        break;
                    }
                    else if (besideE)
                    {
                        entryPoints.push_back({ Point3{ endpoint.x - 1, endpoint.y, 0 }, cutDir::horizontal });
                        break;
                    }
                    else if (besideW)
                    {
                        entryPoints.push_back({Point3{ endpoint.x + 1, endpoint.y, 0 }, cutDir::horizontal});
                        break;
                    }
                }
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

    return plan;
}
