module cityLayout;

import std;
import util;

// ════════════════════════════════════════════════════════════════════════
// cityLayout_build.cpp — decomposeClusterToRects 구현 (스텁).
//
//   책임: 임의 모양 도시 클러스터(픽셀 마스크)를 4×4 이상 직사각형들로 분해.
//   BCP/CityLayout/buildCityLayout 등 도시 내부 절차생성 코드는 모두 제거 — 향후 sector
//   단계에서 lazy 재구현 예정.
//
//   알고리즘:
//     1단계: 수평 슬랩 분해 (O(W*H), 인라인). 사용자 painted 도형 대부분 즉시 성공.
//     2단계: 슬랩 실패 시 백트래킹 폴백 (complete but 지수시간, budget 10M 보호).
//
//   결정론: 같은 입력 마스크 → 같은 직사각형 리스트.
// ════════════════════════════════════════════════════════════════════════

namespace cityLayout
{
    namespace
    {
        // ════════════════════════════════════════════════════════════════
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
        // ════════════════════════════════════════════════════════════════
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
}
