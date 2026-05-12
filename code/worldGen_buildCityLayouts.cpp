module worldGen;

import std;
import util;
import cityLayout;

using namespace worldGrid;

// ════════════════════════════════════════════════════════════════════════
// buildCityLayouts — 도시 ~3000개의 layout을 ThreadPool로 병렬 계산.
//
//   입력: seed, grid(read-only), cities (rectangles 필드 채워진 상태)
//   출력: layouts[i] = cityLayout::buildCityLayout(cities[i], ...)
//         cities[i].rectangles 비어 있으면 layouts[i] = invalid CityLayout
//
//   각 도시 작업은 독립 — 결과는 layouts[i] 슬롯에 직접 쓰기. atomic 카운터로
//   진행도 갱신. CityLayout 자체 데이터 크기는 도시당 ~수 KB 수준이라 메인
//   스레드에서 콜백/스냅샷 보유 비용 없음 (콜백은 인덱스만 받음).
//
//   순수성: grid는 read-only, mutate 없음. 결과는 호출자 소유.
// ════════════════════════════════════════════════════════════════════════

namespace worldGen
{
    std::vector<cityLayout::CityLayout> buildCityLayouts(std::uint64_t seed, const PixelCostGrid& grid, const std::vector<CityNode>& cities, LayoutSink onLayout)
    {
        const __int64 tStart = getNanoTimer();
        prt(L"[worldGen] buildCityLayouts start (N=%zu cities, seed=%llu)\n",
            cities.size(), static_cast<unsigned long long>(seed));

        std::vector<cityLayout::CityLayout> out(cities.size());

        //적당한 워커 수 — 하드웨어 코어 수에서 1~2개 빼서 메인/PNG IO와 경합 회피.
        unsigned hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 4;
        const unsigned numThreads = std::max(2u, hw - 1u);

        std::atomic<int> doneCount{ 0 };
        std::atomic<int> emptyCount{ 0 };   // rectangles 비어 layout 생략된 수

        {
            ThreadPool pool(numThreads);

            for (std::size_t i = 0; i < cities.size(); ++i)
            {
                pool.addTask([i, seed, &grid, &cities, &out, &doneCount, &emptyCount, &onLayout]()
                {
                    const CityNode& c = cities[i];
                    if (c.rectangles.empty())
                    {
                        // layout 미생성 — invalid CityLayout (cityIndex=INVALID) 그대로 둠.
                        emptyCount.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        const std::uint64_t citySeed =
                            seed ^ (static_cast<std::uint64_t>(i) * 0x9E3779B97F4A7C15ULL);
                        out[i] = cityLayout::buildCityLayout(
                            static_cast<std::uint32_t>(i),
                            c.center,
                            static_cast<int>(c.tier),
                            c.rectangles,
                            grid,
                            citySeed);
                    }
                    doneCount.fetch_add(1, std::memory_order_relaxed);
                    if (onLayout) onLayout(static_cast<std::uint32_t>(i));
                });
            }

            pool.waitForThreads();
        }

        // 통계 보고
        std::size_t totalBlocks = 0, totalRoads = 0, totalEntries = 0, totalBridges = 0;
        std::size_t validLayouts = 0;
        for (const auto& l : out)
        {
            if (l.empty()) continue;
            ++validLayouts;
            totalBlocks  += l.blocks.size();
            totalRoads   += l.roads.size();
            totalEntries += l.entries.size();
            totalBridges += l.bridges.size();
        }

        const double totalMs = (getNanoTimer() - tStart) / 1.0e6;
        prt(L"  cities         : %zu valid / %d empty / %zu total\n",
            validLayouts, emptyCount.load(), cities.size());
        prt(L"  blocks         : %zu (avg %.1f / city)\n",
            totalBlocks, validLayouts ? double(totalBlocks) / validLayouts : 0.0);
        prt(L"  roads          : %zu (avg %.1f / city)\n",
            totalRoads, validLayouts ? double(totalRoads) / validLayouts : 0.0);
        prt(L"  entries        : %zu (avg %.2f / city)\n",
            totalEntries, validLayouts ? double(totalEntries) / validLayouts : 0.0);
        prt(L"  bridges        : %zu\n", totalBridges);
        prt(L"  total          : %8.2f ms  (%.2f s, %u threads)\n",
            totalMs, totalMs / 1000.0, numThreads);

        return out;
    }
}
