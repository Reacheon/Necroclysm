module cityLayout;

import std;
import util;
import worldGrid;

using namespace worldGrid;

// ════════════════════════════════════════════════════════════════════════
// cityLayout_build.cpp — buildCityLayout + decomposeClusterToRects 구현.
//
//   파일 구조: stepdown rule.
//     1. 최상위 API:    buildCityLayout, decomposeClusterToRects.
//     2. mid-level:     decomposeBacktrack (재귀 폴백).
//     3. low-level:     좌표/Terrain/grid/edge/RNG 헬퍼.
//   상위에서 호출되는 하위는 모두 전방선언, 정의는 파일 하단.
//
//   프로토타입 수준:
//     - BCP: 2×2 그리드 분할 + 중앙 십자 도로 (블록 4개). 본격 BSP는 후속.
//     - 외곽 변 분석: pixel 단위 walk → water/공유/오픈 분류 → 변 전체에 도로 emit.
//     - Entry point: cardinal 방향당 가장 긴 오픈 변의 중간점. 방향당 최대 1개.
//     - 다리: 두 직사각형이 이웃하지 않고 사이에 water 직선이 있으면 중간점 1개.
//     - 좌표: 입력은 픽셀(48타일=1px), 출력은 실타일.
//
//   상위 모듈 의존:
//     - worldGrid: PixelCostGrid, Terrain, TILES_PER_PIXEL, TILE_BASE_*
//     - util: Point3, prt
// ════════════════════════════════════════════════════════════════════════

namespace cityLayout
{
    // ════════════════════════════════════════════════════════════════════
    // 전방선언 — 정의는 파일 하단.
    // ════════════════════════════════════════════════════════════════════
    namespace
    {
        struct ShareInfo   { Dir4 dir; int lo, hi; };   // hi exclusive
        struct WaterScan   { bool hasWater; bool isCoast; };
        struct EdgeTilePts { Point3 a, b; };

        constexpr Point3 pixelTopLeftToTile(int px, int py) noexcept;
        constexpr Point3 pixelCenterToTile (int px, int py) noexcept;
        constexpr bool   isOpenWater (Terrain t) noexcept;
        constexpr bool   isFreshWater(Terrain t) noexcept;
        constexpr bool   isAnyWater  (Terrain t) noexcept;
        Terrain          readGrid       (const PixelCostGrid& g, int rawPx, int rawPy) noexcept;
        bool             tryGetShare    (const CityRect& i, const CityRect& j, ShareInfo& out) noexcept;
        WaterScan        scanOutside    (const PixelCostGrid& g, const CityRect& r, Dir4 dir) noexcept;
        EdgeTilePts      edgeToTilePts  (const CityRect& r, Dir4 dir) noexcept;
        Point3           edgeMidpointTile(const CityRect& r, Dir4 dir) noexcept;
        int              edgePixelLen   (const CityRect& r, Dir4 dir) noexcept;
        std::uint32_t    roll(std::uint64_t& s) noexcept;

        bool decomposeBacktrack(std::uint8_t* mask, int W, int H, int minSize, int bboxPxX, int bboxPxY, std::vector<CityRect>& out, int& iterBudget) noexcept;
    }

    // ════════════════════════════════════════════════════════════════════
    // buildCityLayout — 직사각형 리스트 → CityLayout. 순수함수.
    //
    //   단계:
    //     1) bbox union 계산
    //     2) BCP 프로토타입: 각 직사각형 2×2 그리드 분할 → CityBlock 4개 + 중앙 십자 도로
    //     3) 외곽 변 분석:
    //          - water 인접 → Coast/Riverside 도로 (whole edge)
    //          - 다른 직사각형과 공유 → Boundary 도로 (whole edge)
    //          - 그 외 → entry point 후보
    //     4) cardinal 방향별 entry point: 가장 긴 오픈 변의 중간점, 방향당 최대 1개
    //     5) 다리 후보: 직사각형 쌍이 마주보고 사이에 water만 있으면 중간점 1개
    // ════════════════════════════════════════════════════════════════════
    CityLayout buildCityLayout(std::uint32_t cityIndex, Point3 centerTile, int tier, const std::vector<CityRect>& rectangles, const PixelCostGrid& grid, std::uint64_t citySeed)
    {
        CityLayout out;
        if (rectangles.empty()) return out;   // invalid

        out.cityIndex  = cityIndex;
        out.rectangles = rectangles;

        // ─── 1) bbox union (픽셀 좌표 → 실타일) ──────────────────────────
        int minPx = rectangles[0].px;
        int minPy = rectangles[0].py;
        int maxPx = rectangles[0].x1();
        int maxPy = rectangles[0].y1();
        for (const auto& r : rectangles)
        {
            minPx = std::min(minPx, r.px);
            minPy = std::min(minPy, r.py);
            maxPx = std::max(maxPx, r.x1());
            maxPy = std::max(maxPy, r.y1());
        }
        out.bboxMinTile = pixelTopLeftToTile(minPx, minPy);
        out.bboxMaxTile = pixelTopLeftToTile(maxPx, maxPy);

        // ─── 2) BCP 프로토타입: 직사각형마다 2×2 그리드 분할 + 중앙 십자 도로 ─
        // 중앙 십자 도로는 water-인접 변(Coast/Riverside)에서 강쪽 paver(3타일)를
        // 침범하지 않도록 그 방향 끝을 3타일 안쪽으로 shrink. 결과:
        //   - 강변 paver: 침범 X ✓
        //   - 코스트 asphalt(15): 정확히 겹침 → +자 교차 ✓
        //   - 내측 paver(3): 십자 asphalt가 통과 → T-junction 개구부 형성 (정상)
        // Boundary 변(두 rect 공유)은 shrink 안 함 — 모두 도시 내부, 자연스러움.
        constexpr int RIVERSIDE_GUARD = 3;   // = ROAD_SIDEWALK in Sector_procGenerate

        std::uint64_t state = citySeed;
        roll(state);  // 워밍업

        for (const auto& r : rectangles)
        {
            // 픽셀 분할점: width/2, height/2 (정수 나눗셈, 최소 2px 보장 → 4×4부터 가능)
            const int midX = r.px + r.w / 2;
            const int midY = r.py + r.h / 2;

            struct Sub { int px, py, x1, y1; };
            const Sub subs[4] = {
                { r.px,  r.py,  midX,   midY  },  // NW
                { midX,  r.py,  r.x1(), midY  },  // NE
                { r.px,  midY,  midX,   r.y1()},  // SW
                { midX,  midY,  r.x1(), r.y1()},  // SE
            };

            // plaza는 도시 중심 근처 블록 1개에만 (도시 첫 직사각형 첫 블록 기준 임시)
            const bool isFirstRect = (&r == &rectangles[0]);

            for (int qi = 0; qi < 4; ++qi)
            {
                const auto& s = subs[qi];
                if (s.x1 - s.px < 2 || s.y1 - s.py < 2) continue;  // 4×4 미만이면 스킵

                CityBlock blk;
                blk.tileMin = pixelTopLeftToTile(s.px, s.py);
                blk.tileMax = pixelTopLeftToTile(s.x1, s.y1);
                blk.kind = BlockKind::Buildable;
                // 도시 중심 가까운 첫 직사각형의 NW 블록을 Plaza로 (가벼운 휴리스틱)
                if (isFirstRect && qi == 3) blk.kind = BlockKind::Plaza;
                else if ((roll(state) % 16) == 0) blk.kind = BlockKind::Park;  // 6.25% Park
                out.blocks.push_back(blk);
            }

            // 중앙 십자 도로 — water-인접 변에서 3타일 안쪽으로 shrink (riverside paver 침범 방지).
            {
                const bool waterN = scanOutside(grid, r, Dir4::N).hasWater;
                const bool waterE = scanOutside(grid, r, Dir4::E).hasWater;
                const bool waterS = scanOutside(grid, r, Dir4::S).hasWater;
                const bool waterW = scanOutside(grid, r, Dir4::W).hasWater;

                const int hLoX = r.px   * TILES_PER_PIXEL + TILE_BASE_X + (waterW ? RIVERSIDE_GUARD : 0);
                const int hHiX = r.x1() * TILES_PER_PIXEL + TILE_BASE_X - (waterE ? RIVERSIDE_GUARD : 0);
                const int vLoY = r.py   * TILES_PER_PIXEL + TILE_BASE_Y + (waterN ? RIVERSIDE_GUARD : 0);
                const int vHiY = r.y1() * TILES_PER_PIXEL + TILE_BASE_Y - (waterS ? RIVERSIDE_GUARD : 0);
                const int midX_tile = midX * TILES_PER_PIXEL + TILE_BASE_X;
                const int midY_tile = midY * TILES_PER_PIXEL + TILE_BASE_Y;

                CityRoadSegment hRoad;
                hRoad.a = Point3{ hLoX, midY_tile, 0 };
                hRoad.b = Point3{ hHiX, midY_tile, 0 };
                hRoad.kind = RoadKind::Interior;
                out.roads.push_back(hRoad);

                CityRoadSegment vRoad;
                vRoad.a = Point3{ midX_tile, vLoY, 0 };
                vRoad.b = Point3{ midX_tile, vHiY, 0 };
                vRoad.kind = RoadKind::Interior;
                out.roads.push_back(vRoad);
            }
        }

        // ─── 3) 외곽 변 분석 — 4 방향마다 water/공유/오픈 분류 ────────────
        // 각 (rect, dir) 변마다:
        //   1순위 water 인접 → Coast/Riverside (도로 emit)
        //   2순위 다른 rect와 공유 → Boundary (도로 emit)
        //   3순위 그 외 → entry 후보

        // entry 후보: (dir, edge tile a/b, len) 모아두기
        struct EntryCand { Dir4 dir; Point3 a; Point3 b; int lenTiles; };
        std::vector<EntryCand> openEdges;

        const Dir4 dirs[4] = { Dir4::N, Dir4::E, Dir4::S, Dir4::W };

        // 외곽 변의 안쪽 방향 (Coast/Riverside의 interiorSide 결정용).
        //   N 변 → 도시 내부는 S, E 변 → W, S 변 → N, W 변 → E.
        auto interiorOf = [](Dir4 edgeDir) noexcept -> Dir4
        {
            switch (edgeDir)
            {
            case Dir4::N: return Dir4::S;
            case Dir4::E: return Dir4::W;
            case Dir4::S: return Dir4::N;
            case Dir4::W: return Dir4::E;
            default:      return Dir4::None;
            }
        };

        for (std::size_t ri = 0; ri < rectangles.size(); ++ri)
        {
            const CityRect& r = rectangles[ri];
            for (Dir4 dir : dirs)
            {
                const auto edge = edgeToTilePts(r, dir);

                // water 검사
                const WaterScan ws = scanOutside(grid, r, dir);
                if (ws.hasWater)
                {
                    CityRoadSegment seg{ edge.a, edge.b,
                        ws.isCoast ? RoadKind::Coast : RoadKind::Riverside,
                        interiorOf(dir) };
                    out.roads.push_back(seg);
                    continue;
                }

                // 공유변 검사 — 다른 rect와 cardinal dir로 붙어있는지.
                //   디둡: i < j일 때만 emit. (j > i, j도 같은 변을 가지므로 한쪽만 emit해도 충분 —
                //   Boundary 변 위치는 두 직사각형 사이에서 동일하니까)
                bool shared = false;
                bool sharedWithSmaller = false;   // 디둡: 자기보다 인덱스 작은 rect와 공유면 skip
                for (std::size_t rj = 0; rj < rectangles.size(); ++rj)
                {
                    if (rj == ri) continue;
                    ShareInfo si;
                    if (tryGetShare(r, rectangles[rj], si) && si.dir == dir)
                    {
                        shared = true;
                        if (rj < ri) { sharedWithSmaller = true; break; }
                    }
                }
                if (shared)
                {
                    if (sharedWithSmaller) continue;  // 디둡 — 작은 rj 쪽에서 이미 emit
                    CityRoadSegment seg{ edge.a, edge.b, RoadKind::Boundary, Dir4::None };
                    out.roads.push_back(seg);
                    continue;
                }

                // 오픈 — entry 후보
                openEdges.push_back({ dir, edge.a, edge.b, edgePixelLen(r, dir) * TILES_PER_PIXEL });
            }
        }

        // ─── 4) Entry point 선택 — cardinal 방향당 최대 1개 ────────────────
        // 각 방향에서 가장 긴 오픈 변의 중간점.
        for (Dir4 dir : dirs)
        {
            const EntryCand* best = nullptr;
            for (const auto& c : openEdges)
            {
                if (c.dir != dir) continue;
                if (!best || c.lenTiles > best->lenTiles) best = &c;
            }
            if (best)
            {
                CityEntryPoint ep;
                ep.tile = Point3{ (best->a.x + best->b.x) / 2, (best->a.y + best->b.y) / 2, 0 };
                ep.outward = dir;
                out.entries.push_back(ep);
            }
        }

        // ─── 5) 다리 후보 — 두 직사각형이 마주보고 사이에 water만 있으면 ───
        // 마주보기 정의: cardinal 방향 align(공유 X 또는 Y 구간) + 1픽셀 이상 갭.
        // 갭 픽셀 전부 water면 다리 가능. 중간점 1개 emit (확률적 추가는 본격 단계에).
        //
        // 다리 endpoint = 양 직사각형의 *코스트 도로 아스팔트 안쪽 끝*. 코스트 도로 21타일은
        // [3 riverside paver][15 asphalt][3 interior paver] 구성인데, 다리는 강변 paver 통과 후
        // asphalt까지만 들어가고 *내측 paver는 건드리지 않음*. 즉 다리 깊이 = 3 + 15 = 18타일.
        // 21로 잡으면 asphalt 스트립(폭 15)이 내측 paver를 3타일 더 뚫고 지나가는 시각 버그 발생.
        constexpr int BRIDGE_REACH = 18;

        for (std::size_t ri = 0; ri < rectangles.size(); ++ri)
        {
            for (std::size_t rj = ri + 1; rj < rectangles.size(); ++rj)
            {
                const CityRect& a = rectangles[ri];
                const CityRect& b = rectangles[rj];

                // 가로 방향 (a가 위, b가 아래) — y gap = b.py - a.y1, 가 양수
                int yGap = b.py - a.y1();
                int yGapDir = 1;  // 1 = a above b
                if (yGap < 0) { yGap = a.py - b.y1(); yGapDir = -1; }
                const int xLo = std::max(a.px, b.px);
                const int xHi = std::min(a.x1(), b.x1());

                if (yGap > 0 && xHi > xLo)
                {
                    // 갭 행들에서 [xLo, xHi) 범위가 전부 water인지 검사
                    const int yStart = (yGapDir > 0) ? a.y1() : b.y1();
                    bool allWater = true;
                    for (int gy = 0; gy < yGap && allWater; ++gy)
                    {
                        for (int x = xLo; x < xHi; ++x)
                        {
                            if (!isAnyWater(readGrid(grid, x, yStart + gy)))
                            {
                                allWater = false; break;
                            }
                        }
                    }
                    if (allWater)
                    {
                        // 위쪽 rect의 gap-facing 변 = first water tile y. 아래쪽 rect의 first tile y.
                        const int midX        = (xLo + xHi) / 2;
                        const int bridgeX     = midX * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2;
                        const int edgeAboveY  = yStart        * TILES_PER_PIXEL + TILE_BASE_Y;
                        const int edgeBelowY  = (yStart + yGap) * TILES_PER_PIXEL + TILE_BASE_Y;
                        const Point3 pa{ bridgeX, edgeAboveY - BRIDGE_REACH,     0 };       // 위쪽 rect 내부 18 깊이 (asphalt 끝)
                        const Point3 pb{ bridgeX, edgeBelowY + BRIDGE_REACH - 1, 0 };       // 아래쪽 rect 내부 18 깊이
                        out.bridges.push_back(CityBridge{ pa, pb });
                    }
                }

                // 세로 방향 (a가 왼쪽, b가 오른쪽) — x gap
                int xGap = b.px - a.x1();
                int xGapDir = 1;
                if (xGap < 0) { xGap = a.px - b.x1(); xGapDir = -1; }
                const int yLo = std::max(a.py, b.py);
                const int yHi = std::min(a.y1(), b.y1());

                if (xGap > 0 && yHi > yLo)
                {
                    const int xStart = (xGapDir > 0) ? a.x1() : b.x1();
                    bool allWater = true;
                    for (int gx = 0; gx < xGap && allWater; ++gx)
                    {
                        for (int y = yLo; y < yHi; ++y)
                        {
                            if (!isAnyWater(readGrid(grid, xStart + gx, y)))
                            {
                                allWater = false; break;
                            }
                        }
                    }
                    if (allWater)
                    {
                        const int midY        = (yLo + yHi) / 2;
                        const int bridgeY     = midY * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2;
                        const int edgeLeftX   = xStart          * TILES_PER_PIXEL + TILE_BASE_X;
                        const int edgeRightX  = (xStart + xGap) * TILES_PER_PIXEL + TILE_BASE_X;
                        const Point3 pa{ edgeLeftX  - BRIDGE_REACH,     bridgeY, 0 };       // 왼쪽 rect 내부 18 (asphalt 끝)
                        const Point3 pb{ edgeRightX + BRIDGE_REACH - 1, bridgeY, 0 };       // 오른쪽 rect 내부 18
                        out.bridges.push_back(CityBridge{ pa, pb });
                    }
                }
            }
        }

        // tier/center 인자는 향후 BCP 본격 단계에서 블록 크기/Plaza 위치 결정에 사용 예정.
        (void)centerTile; (void)tier;

        return out;
    }

    // ════════════════════════════════════════════════════════════════════
    // decomposeClusterToRects — 분해 진입점.
    //
    //   1단계: 수평 슬랩 분해 (O(W*H), 인라인). 사용자 painted 도형 대부분 즉시 성공.
    //     각 행의 run 패턴을 계산하고, 연속된 동일 패턴 행들을 한 슬랩으로 묶음.
    //     각 슬랩의 각 run을 직사각형으로 emit. 모든 슬랩 height ≥ minSize, 모든 run
    //     width ≥ minSize면 성공. Moscow 같은 "+" cross도 슬랩 5개로 깔끔히 분해.
    //   2단계: 슬랩 실패 시 백트래킹 폴백 (complete but 지수시간, budget 10M 보호).
    //
    //   실패 시 빈 vector 반환 — 호출자가 layout 생략.
    // ════════════════════════════════════════════════════════════════════
    std::vector<CityRect> decomposeClusterToRects(const std::uint8_t* inMask, int bboxPxX, int bboxPxY, int bboxW, int bboxH, int minSize)
    {
        if (bboxW < minSize || bboxH < minSize) return {};

        std::vector<CityRect> rects;
        rects.reserve(16);

        // ─── 1단계: 수평 슬랩 분해 (인라인) ────────────────────────────────
        // 부분 emit 후 실패 가능 → 호출자가 rects.clear()로 롤백.
        const bool slabOk = [&]() -> bool
        {
            // 각 행의 run 패턴 계산: 각 run은 [start, end) 쌍.
            std::vector<std::vector<std::pair<int,int>>> rowRuns(static_cast<std::size_t>(bboxH));
            for (int y = 0; y < bboxH; ++y)
            {
                int x = 0;
                while (x < bboxW)
                {
                    if (inMask[y * bboxW + x])
                    {
                        const int start = x;
                        while (x < bboxW && inMask[y * bboxW + x]) ++x;
                        rowRuns[y].emplace_back(start, x);
                    }
                    else ++x;
                }
            }

            // 동일 패턴 연속 행을 슬랩으로 그룹화. 빈 행은 슬랩 경계.
            int y = 0;
            while (y < bboxH)
            {
                if (rowRuns[y].empty()) { ++y; continue; }
                int slabEnd = y + 1;
                while (slabEnd < bboxH && rowRuns[slabEnd] == rowRuns[y]) ++slabEnd;
                const int slabHeight = slabEnd - y;
                if (slabHeight < minSize) return false;   // 슬랩 너무 얇음
                for (const auto& [start, end] : rowRuns[y])
                {
                    const int width = end - start;
                    if (width < minSize) return false;     // run 너무 좁음
                    rects.push_back(CityRect{ bboxPxX + start, bboxPxY + y, width, slabHeight });
                }
                y = slabEnd;
            }
            return true;
        }();

        if (slabOk) return rects;

        // ─── 2단계: 슬랩 실패 → 백트래킹 폴백 ──────────────────────────────
        rects.clear();
        std::vector<std::uint8_t> mask(inMask, inMask + static_cast<std::size_t>(bboxW) * bboxH);

        constexpr int BUDGET = 10'000'000;   // worst-case 보호 — pathological pattern 시 종료
        int iterBudget = BUDGET;
        if (decomposeBacktrack(mask.data(), bboxW, bboxH, minSize, bboxPxX, bboxPxY, rects, iterBudget))
        {
            return rects;
        }

        // 진짜 분해 불가능(보장 위반) 또는 budget 초과.
        prt(L"  [decompose] FAIL: slab+backtrack exhausted at bbox (%d,%d) %dx%d (budget used=%d)\n",
            bboxPxX, bboxPxY, bboxW, bboxH, BUDGET - iterBudget);

        // ─── 진단 마스크 덤프 — PNG 마킹 오류 시각화 ──────────────────────────
        // 실패한 입력 마스크를 ASCII로 출력. '#' = 1, '.' = 0.
        // 1픽셀 삐짐/구멍/공유변 어긋남/좁은 띠를 눈으로 식별.
        // 콘솔 폭 한계 고려해 최대 160×100까지만(그 이상은 너비/높이 절단).
        constexpr int DUMP_MAX_W = 160;
        constexpr int DUMP_MAX_H = 100;
        const int dumpW = std::min(bboxW, DUMP_MAX_W);
        const int dumpH = std::min(bboxH, DUMP_MAX_H);

        // 행/열 통계 — 0 셀이 한쪽 끝에 몰려 있는지(잡티) 빠르게 파악용.
        std::size_t totalOnes = 0;
        for (int y = 0; y < bboxH; ++y)
            for (int x = 0; x < bboxW; ++x)
                if (inMask[static_cast<std::size_t>(y) * bboxW + x]) ++totalOnes;

        prt(L"  [decompose] mask dump (%dx%d, ones=%zu / %zu, shown %dx%d):\n",
            bboxW, bboxH, totalOnes, static_cast<std::size_t>(bboxW) * bboxH, dumpW, dumpH);

        std::wstring line;
        line.reserve(static_cast<std::size_t>(dumpW) + 16);
        for (int y = 0; y < dumpH; ++y)
        {
            line.assign(L"    ");
            for (int x = 0; x < dumpW; ++x)
            {
                line.push_back(inMask[static_cast<std::size_t>(y) * bboxW + x] ? L'#' : L'.');
            }
            line.push_back(L'\n');
            prt(L"%ls", line.c_str());
        }
        if (bboxW > DUMP_MAX_W || bboxH > DUMP_MAX_H)
        {
            prt(L"    [...truncated, original size %dx%d]\n", bboxW, bboxH);
        }
        return {};
    }

    // ════════════════════════════════════════════════════════════════════
    // decomposeBacktrack — top-leftmost cell 기반 완전 백트래킹 분해.
    //
    //   완전성(complete): 분해가 존재하면 반드시 찾음.
    //
    //   핵심 invariant: 도형의 top-leftmost 셀(min y, 그 중 min x)은 *항상*
    //     어떤 직사각형의 좌상단 모서리.
    //
    //   후보: (x0, y0)에서 시작하는 모든 valid (w, h) 페어.
    //     w ∈ [minSize, maxW], h ∈ [minSize, maxH(w)].
    //     dominated 후보(예: (4, 4)는 (4, 8)에 dominated)도 *반드시 시도*. 사용자가
    //     (4, 4)로 그렸는데 인접 rect 때문에 (4, 8)도 valid한 경우, (4, 8)을 picking
    //     하면 인접 rect의 셀을 잡아먹어서 그 rect가 narrow strip이 되어 분해 실패할
    //     수 있음. 따라서 (4, 4) 같은 dominated 후보도 백트래킹에서 필요.
    //
    //   시도 순서: w descending (max부터), h descending (max부터) — 큰 직사각형 우선.
    //
    //   복잡도: O((W*H)^N) 최악. 분기 폭 가지치기 어려움(완전성 위배 위험).
    //     실제 도시 모양은 보통 첫 시도(maxW, maxH)가 적중 → 사실상 빠름.
    //     pathological 케이스는 iterBudget(10M)으로 보호.
    // ════════════════════════════════════════════════════════════════════
    namespace
    {
        bool decomposeBacktrack(std::uint8_t* mask, int W, int H, int minSize, int bboxPxX, int bboxPxY, std::vector<CityRect>& out, int& iterBudget) noexcept
        {
            if (--iterBudget < 0) return false;

            // top-leftmost 1-cell 찾기
            int x0 = -1, y0 = -1;
            for (int y = 0; y < H; ++y)
            {
                for (int x = 0; x < W; ++x)
                {
                    if (mask[y * W + x]) { x0 = x; y0 = y; break; }
                }
                if (x0 >= 0) break;
            }
            if (x0 < 0) return true;   // mask 비었음 — 분해 완료

            // (x0, y0)에서 row y0의 1-run 길이 = maxW 상한.
            int maxW = 0;
            while (x0 + maxW < W && mask[y0 * W + (x0 + maxW)]) ++maxW;
            if (maxW < minSize) return false;

            // w descending, 각 w마다 maxH 계산 후 h descending — 모든 (w, h) 시도.
            for (int w = maxW; w >= minSize; --w)
            {
                // 이 w에서 아래로 확장 가능한 maxH 계산
                int maxH = 0;
                while (y0 + maxH < H)
                {
                    bool allOnes = true;
                    for (int dx = 0; dx < w; ++dx)
                    {
                        if (!mask[(y0 + maxH) * W + (x0 + dx)]) { allOnes = false; break; }
                    }
                    if (!allOnes) break;
                    ++maxH;
                }
                if (maxH < minSize) continue;

                for (int h = maxH; h >= minSize; --h)
                {
                    // 후보 (x0, y0, w, h) 채택
                    for (int dy = 0; dy < h; ++dy)
                    {
                        std::memset(&mask[(y0 + dy) * W + x0], 0, static_cast<std::size_t>(w));
                    }
                    out.push_back(CityRect{ bboxPxX + x0, bboxPxY + y0, w, h });

                    if (decomposeBacktrack(mask, W, H, minSize, bboxPxX, bboxPxY, out, iterBudget))
                    {
                        return true;
                    }

                    // backtrack
                    out.pop_back();
                    for (int dy = 0; dy < h; ++dy)
                    {
                        std::memset(&mask[(y0 + dy) * W + x0], 1, static_cast<std::size_t>(w));
                    }
                }
            }
            return false;
        }
    }

    // ════════════════════════════════════════════════════════════════════
    // low-level helpers 정의
    // ════════════════════════════════════════════════════════════════════
    namespace
    {
        // ─── 좌표 변환: 픽셀 → 실타일 ───────────────────────────────────────
        // 픽셀 (px, py) 좌상단 → 실타일 좌상단. 48배 스케일 + TILE_BASE 오프셋.
        constexpr Point3 pixelTopLeftToTile(int px, int py) noexcept
        {
            return Point3{
                px * TILES_PER_PIXEL + TILE_BASE_X,
                py * TILES_PER_PIXEL + TILE_BASE_Y,
                0
            };
        }

        // 픽셀 (px, py)의 중심 실타일(타일 24,24 오프셋).
        constexpr Point3 pixelCenterToTile(int px, int py) noexcept
        {
            return Point3{
                px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2,
                py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2,
                0
            };
        }

        // ─── Terrain 분류 ──────────────────────────────────────────────────
        constexpr bool isOpenWater(Terrain t) noexcept
        {
            return t == Terrain::Sea || t == Terrain::CitySea;
        }
        constexpr bool isFreshWater(Terrain t) noexcept
        {
            return t == Terrain::River || t == Terrain::Lake || t == Terrain::CityRiver;
        }
        constexpr bool isAnyWater(Terrain t) noexcept
        {
            return isOpenWater(t) || isFreshWater(t);
        }

        // ─── grid에서 픽셀 안전하게 읽기 (X wrap, Y 범위 외는 Sea) ─────────
        Terrain readGrid(const PixelCostGrid& g, int rawPx, int rawPy) noexcept
        {
            if (rawPy < 0 || rawPy >= PixelCostGrid::H) return Terrain::Sea;
            const int x = ((rawPx % PixelCostGrid::W) + PixelCostGrid::W) % PixelCostGrid::W;
            return g.data[static_cast<std::size_t>(rawPy) * PixelCostGrid::W + x];
        }

        // ─── 두 직사각형이 cardinal 방향으로 공유변을 가지나? ──────────────
        // 반환: 공유변 (a, b) 픽셀 범위, X(가로 변) 또는 Y(세로 변).
        //       overlap 길이 0이면 공유 없음.
        // i가 j 방향으로 공유변을 가지면 true. dir = i 기준 j가 있는 방향.
        bool tryGetShare(const CityRect& i, const CityRect& j, ShareInfo& out) noexcept
        {
            // N: i.py == j.y1 (j가 위쪽에 붙음, i의 N 변 = j의 S 변)
            if (i.py == j.y1())
            {
                const int lo = std::max(i.px, j.px);
                const int hi = std::min(i.x1(), j.x1());
                if (hi > lo) { out = { Dir4::N, lo, hi }; return true; }
            }
            // S: i.y1 == j.py
            if (i.y1() == j.py)
            {
                const int lo = std::max(i.px, j.px);
                const int hi = std::min(i.x1(), j.x1());
                if (hi > lo) { out = { Dir4::S, lo, hi }; return true; }
            }
            // W: i.px == j.x1
            if (i.px == j.x1())
            {
                const int lo = std::max(i.py, j.py);
                const int hi = std::min(i.y1(), j.y1());
                if (hi > lo) { out = { Dir4::W, lo, hi }; return true; }
            }
            // E: i.x1 == j.px
            if (i.x1() == j.px)
            {
                const int lo = std::max(i.py, j.py);
                const int hi = std::min(i.y1(), j.y1());
                if (hi > lo) { out = { Dir4::E, lo, hi }; return true; }
            }
            return false;
        }

        // ─── 변 하나의 outside 픽셀 water 검사 ──────────────────────────────
        // 변 = (dir, lo, hi). 변 길이 hi-lo 픽셀의 *바깥쪽* 픽셀들을 훑어
        // sea/river 등장 여부 + 종류 결정. coast(Sea/CitySea)가 river보다 우선.
        WaterScan scanOutside(const PixelCostGrid& g, const CityRect& r, Dir4 dir) noexcept
        {
            WaterScan ws{ false, false };
            switch (dir)
            {
            case Dir4::N:
                for (int x = r.px; x < r.x1(); ++x)
                {
                    const Terrain t = readGrid(g, x, r.py - 1);
                    if (isOpenWater(t))  { ws.hasWater = true; ws.isCoast = true; return ws; }
                    if (isFreshWater(t)) { ws.hasWater = true; }
                }
                break;
            case Dir4::S:
                for (int x = r.px; x < r.x1(); ++x)
                {
                    const Terrain t = readGrid(g, x, r.y1());
                    if (isOpenWater(t))  { ws.hasWater = true; ws.isCoast = true; return ws; }
                    if (isFreshWater(t)) { ws.hasWater = true; }
                }
                break;
            case Dir4::W:
                for (int y = r.py; y < r.y1(); ++y)
                {
                    const Terrain t = readGrid(g, r.px - 1, y);
                    if (isOpenWater(t))  { ws.hasWater = true; ws.isCoast = true; return ws; }
                    if (isFreshWater(t)) { ws.hasWater = true; }
                }
                break;
            case Dir4::E:
                for (int y = r.py; y < r.y1(); ++y)
                {
                    const Terrain t = readGrid(g, r.x1(), y);
                    if (isOpenWater(t))  { ws.hasWater = true; ws.isCoast = true; return ws; }
                    if (isFreshWater(t)) { ws.hasWater = true; }
                }
                break;
            }
            return ws;
        }

        // ─── 변의 두 끝점을 실타일 좌표로 ──────────────────────────────────
        // dir: 변의 방향 (N=위쪽 가로변, E=오른쪽 세로변, S=아래쪽, W=왼쪽)
        EdgeTilePts edgeToTilePts(const CityRect& r, Dir4 dir) noexcept
        {
            const Point3 tl  = pixelTopLeftToTile(r.px,    r.py);
            const Point3 tr  = pixelTopLeftToTile(r.x1(),  r.py);
            const Point3 bl  = pixelTopLeftToTile(r.px,    r.y1());
            const Point3 br  = pixelTopLeftToTile(r.x1(),  r.y1());
            switch (dir)
            {
            case Dir4::N: return { tl, tr };
            case Dir4::E: return { tr, br };
            case Dir4::S: return { bl, br };
            case Dir4::W: return { tl, bl };
            }
            return { tl, tl };
        }

        // ─── 변 중간점을 실타일 좌표로 ─────────────────────────────────────
        Point3 edgeMidpointTile(const CityRect& r, Dir4 dir) noexcept
        {
            const auto [a, b] = edgeToTilePts(r, dir);
            return Point3{ (a.x + b.x) / 2, (a.y + b.y) / 2, 0 };
        }

        // ─── 변 픽셀 길이 ──────────────────────────────────────────────────
        int edgePixelLen(const CityRect& r, Dir4 dir) noexcept
        {
            return (dir == Dir4::N || dir == Dir4::S) ? r.w : r.h;
        }

        // ─── 결정론 LCG (placeCities Phase 4 패턴 재사용) ──────────────────
        std::uint32_t roll(std::uint64_t& s) noexcept
        {
            s = s * 6364136223846793005ULL + 1442695040888963407ULL;
            return static_cast<std::uint32_t>(s >> 32);
        }
    } // anonymous namespace (low-level helpers)
}
