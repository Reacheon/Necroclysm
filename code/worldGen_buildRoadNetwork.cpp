module worldGen;

import std;
import util;

using namespace worldGrid;  // Terrain, PixelCostGrid, TILES_PER_PIXEL 등 unqualified 접근

//============================================================
// 도로망 폴리라인 생성 — 평면 유클리드 MST + DSU 봉합망.
//   입력: seed + PixelCostGrid + 도시 좌표 약 3000개
//   출력: 실타일 좌표 폴리라인 N-1 + bridges (~3000개 내외)
//
//   파이프라인:
//     1) tile→pixel 역변환
//     2) 도시 전체 평면 MST (dense Prim's O(N²)) — 정확히 N-1 city-city 엣지.
//        트리 구조라 평행/중복 없음, 그래프 이론적 연결성 보장.
//     3) Coarse 그리드 1회 빌드 (8× 다운샘플, top-4 mean)
//     4) ThreadPool 병렬 hierarchical bidirectional A* (거리 desc 정렬, tail latency 감소):
//          - 짧은 엣지(<400px): bidirectional A* 직접
//          - 긴 엣지: coarse → corridor mask → bidirectional A* with corridor pruning
//          - 각 path 성공 시 공유 roadGrid 에 stamp → 후속 엣지 snap (kRoadDiscount)
//     5) 4-direction turn-keep 단순화 + 실타일 변환
//     6) 연결성 봉합 (safety net) — A* 실패로 끊긴 컴포넌트를 차순 짧은 city-pair 로 봉합.
//        최대 3 pass, pass 당 component 당 K=2 bridge 후보. attempted set 으로 재시도 방지.
//
//   순수 블랙박스 함수 — 외부 상태 무관, 단일 거대 함수(placeCities 패턴).
//   픽셀좌표(1px=48타일)는 알고리즘 내부 전용, 반환값은 실타일 좌표.
//
//   핵심 설계 결정:
//   - 도시 가로지름 차단: kCostLUT 의 City* = 1000 + A* 의 City* 셀 hard barrier (continue).
//     비용 모델만으로는 thin protrusion 가로지름 가능 → hard barrier 로 확정.
//   - 도시 직교 진입: boundaryEntryFor 가 cityRegion 바깥 첫 non-city 셀 + cardinal step 반환.
//     computeStrut 가 그 셀에서 kStrutLen 픽셀 forced 카디널 연장 → 폴리라인 첫/끝 segment 직각.
//     A* 시작 셀의 prevDir 도 strut 방향으로 초기화 → strut.back() 이 강 위인 경우 bend 차단.
//   - 4-방향(N/E/S/W) A* + kTurnCost — staircase/coastline-hugging 차단.
//   - 물 주변 bend 금지 — cur/prev/next 중 하나라도 River/Sea/Lake 면 turn 금지 (1셀 land
//     buffer). bidir meeting 시 dF^1 != dB 이고 meeting 셀이 물이면 후보 거부.
//   - Hierarchical corridor: coarse A*로 corridor 추출, fine A*는 corridor 셀만 expand.
//   - Bidirectional: forward + backward 동시 expand → ~2× 가속.
//   - Coarse 집계는 top-4 mean (단일 city 픽셀이 sea 위 false positive corridor 만드는 것 방지).
//   - Road snap: 이미 깔린 폴리라인 coarse cell 의 terrain × kRoadDiscount → 가까운 엣지끼리
//     공유 corridor 형성 (트리 구조 위 가벼운 보조).
//============================================================
namespace worldGen
{
    //도시 좌표들을 바탕으로 도로 폴리라인 네트워크 생성. onRoad 는 폴리라인 1개 완성될 때마다
    //  호출되는 옵션 콜백 (no-op default).
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const PixelCostGrid& grid, const std::vector<CityNode>& cities, RoadSink onRoad)
    {
        const __int64 tStart = getNanoTimer();
        prt(L"[worldGen] buildRoadNetwork start (N=%zu cities, seed=%llu)\n",
            cities.size(), static_cast<unsigned long long>(seed));

        if (cities.size() < 2) return {};

        //══════════════════════════════════════════════════════════════════
        // 상수 — 모든 튜닝 포인트 집중
        //══════════════════════════════════════════════════════════════════
        static constexpr float kMinCost = 0.5f;

        //90° 회전 시 추가되는 비용 — chunky 도로 segment 유도. 직진은 무료.
        //  staircase 차단: 50 cell staircase = 50 + 49×50 = 2500 vs 70 cell 직선 = 70 → 직선 우세.
        static constexpr float kTurnCost = 50.0f;

        //기존 도로 snap 할인 — 이미 깔린 path 위 coarse cell 의 terrain × kRoadDiscount.
        //  MST 트리 구조에선 평행 도로가 거의 없지만, 가까운 두 엣지가 비슷한 corridor 를 쓸
        //  때 공유하도록 가볍게 끌어당김. 너무 작으면 강제 우회로 부자연스러움.
        static constexpr float kRoadDiscount = 0.1f;

        //"이전 step 없음" sentinel — start/end 셀의 dirF/dirB 초깃값. 첫 step turn cost 면제.
        static constexpr std::uint8_t kNoDir = 0xFF;

        //경로비용 / 직선거리(px) 상한 — A* 성공해도 이 비율 넘으면 폐기.
        //  sea 10픽셀 이상 횡단 (호주↔인도네시아 등) 컷.
        static constexpr float  kMaxCostPerPixel        = 30.0f;
        static constexpr int    kCoarseAggK             = 4;
        static constexpr float  kHeuristicWeight        = 3.0f;   //fine — 거의 greedy
        static constexpr float  kCoarseHeuristicWeight  = 1.5f;   //coarse — 더 tight (작아서 부담 적음)

        //Coarse 그리드 차원 — 함수 스코프 상수 (MSVC 로컬 struct 정적 멤버 금지: C2246).
        static constexpr int    kCoarseF = 8;
        static constexpr int    kCoarseW = (PixelCostGrid::W + kCoarseF - 1) / kCoarseF;   // 5400
        static constexpr int    kCoarseH = (PixelCostGrid::H + kCoarseF - 1) / kCoarseF;   // 2700

        //너무 긴 엣지는 시도 자체를 skip — 박스 cap 초과로 어차피 fail.
        //  kMaxCostPerPixel 비용비율 cap이 살아있어 호주↔인도네시아 같은 sea 횡단은 후필터로 걸러짐.
        static constexpr double kMaxEdgeAttempt = 5000.0;

        //비용 LUT — Terrain enum 값을 인덱스로 사용. IIFE로 컴파일 타임 초기화.
        static constexpr std::array<float, 16> kCostLUT = []() {
            std::array<float, 16> a{};
            a[static_cast<std::size_t>(Terrain::Land      )] =   1.0f;
            a[static_cast<std::size_t>(Terrain::Sea       )] = 5000.0f;
            a[static_cast<std::size_t>(Terrain::River     )] =  20.0f;
            a[static_cast<std::size_t>(Terrain::Lake      )] = 800.0f;
            //City* 전부 1000 — 중간 도시 가로지름 차단. 엔드포인트는 cityRegion 바깥 카디널 entry.
            a[static_cast<std::size_t>(Terrain::CityZone  )] = 1000.0f;
            a[static_cast<std::size_t>(Terrain::CityCenter)] = 1000.0f;
            a[static_cast<std::size_t>(Terrain::CityRiver )] = 1000.0f;
            a[static_cast<std::size_t>(Terrain::CitySea   )] = 1000.0f;
            a[static_cast<std::size_t>(Terrain::Mountain  )] =   8.0f;
            a[static_cast<std::size_t>(Terrain::Polar     )] =   6.0f;
            a[static_cast<std::size_t>(Terrain::Tundra    )] =   2.0f;
            a[static_cast<std::size_t>(Terrain::Subarctic )] =   0.7f;
            a[static_cast<std::size_t>(Terrain::Monsoon              )] =   1.2f;
            a[static_cast<std::size_t>(Terrain::InsularRainforest    )] =   1.3f;
            a[static_cast<std::size_t>(Terrain::Desert               )] =   0.6f;
            a[static_cast<std::size_t>(Terrain::ContinentalRainforest)] =  11.0f;
            return a;
        }();

        //4-방향 카디널 전용 — 대각도로 차단.
        //  대각 step을 제거하면 폴리라인이 N/E/S/W 90° 회전만으로 구성. 도시 격자(BCP)와 정합.
        struct Step { int dx, dy; };
        static constexpr std::array<Step, 4> kSteps = {{
            { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
        }};

        //══════════════════════════════════════════════════════════════════
        // 로컬 타입 — 함수 안에서만 사용
        //══════════════════════════════════════════════════════════════════

        //도시 픽셀 좌표 (cps 배열에 cities tile 좌표 역변환 결과).
        struct CityPixel
        {
            int      px;
            int      py;
            int      z;
        };

        //MST 엣지 후보 — city-city pair + 픽셀 직선거리.
        struct EdgeCand
        {
            int    a;
            int    b;
            double dist;
        };

        //Coarse 비용 그리드 — fine 그리드 8× 다운샘플, top-4 mean 집계.
        struct CoarseGrid
        {
            std::unique_ptr<float[]> cost;
        };

        //Corridor 비트셋 — coarse 해상도. mark/test/prepare O(1), 클리어 O(marked size).
        struct CorridorMask
        {
            std::vector<std::uint64_t> bits;
            std::vector<std::uint32_t> marked;

            void prepare()
            {
                if (bits.empty())
                {
                    const std::size_t total = static_cast<std::size_t>(kCoarseW) * kCoarseH;
                    bits.assign((total + 63) / 64, 0);
                }
                for (std::uint32_t idx : marked)
                {
                    bits[idx >> 6] &= ~(1ULL << (idx & 63));
                }
                marked.clear();
            }

            void mark(std::uint32_t idx)
            {
                const std::uint32_t w = idx >> 6;
                const std::uint64_t b = 1ULL << (idx & 63);
                if (!(bits[w] & b))
                {
                    bits[w] |= b;
                    marked.push_back(idx);
                }
            }

            bool test(std::uint32_t idx) const noexcept
            {
                return ((bits[idx >> 6] >> (idx & 63)) & 1ULL) != 0;
            }
        };

        //A* 우선순위 큐 노드 — f=g+h, gAtPush 로 stale 항목 검출.
        struct HeapNode
        {
            float       f;
            float       gAtPush;
            std::size_t idx;
            bool operator>(const HeapNode& o) const noexcept { return f > o.f; }
        };

        //thread_local A* 워크스페이스 — forward + backward 양쪽 상태 + corridor mask.
        //  generation 번호로 박스 재사용 시 clear 비용 회피 (curGen 만 증가).
        //  coarse A* 는 unidirectional 이라 F 만 사용.
        struct AStarWorkspace
        {
            std::vector<float>         gF;
            std::vector<std::uint8_t>  dirF;
            std::vector<std::uint16_t> genF;
            std::vector<HeapNode>      heapF;
            std::uint16_t              curGenF = 0;

            std::vector<float>         gB;
            std::vector<std::uint8_t>  dirB;
            std::vector<std::uint16_t> genB;
            std::vector<HeapNode>      heapB;
            std::uint16_t              curGenB = 0;

            CorridorMask               mask;

            void prepare(std::size_t total, bool needBackward)
            {
                if (gF.size() < total)
                {
                    gF.resize(total); dirF.resize(total); genF.assign(total, 0);
                    curGenF = 0;
                }
                if (needBackward && gB.size() < total)
                {
                    gB.resize(total); dirB.resize(total); genB.assign(total, 0);
                    curGenB = 0;
                }

                curGenF += 2;
                if (curGenF >= 65534)
                {
                    std::fill(genF.begin(), genF.end(), 0);
                    curGenF = 2;
                }
                heapF.clear();

                if (needBackward)
                {
                    curGenB += 2;
                    if (curGenB >= 65534)
                    {
                        std::fill(genB.begin(), genB.end(), 0);
                        curGenB = 2;
                    }
                    heapB.clear();
                }
            }
        };

        //Fine A* 결과 — path 비어있으면 실패. totalCost 가 mu 값.
        struct AStarOut
        {
            std::vector<PixelCoord> path;
            float                   totalCost    = 0.0f;
            std::uint64_t           expanded     = 0;
            std::size_t             maxHeapSize  = 0;
        };

        //Coarse A* 결과 — cells 는 글로벌 coarse 셀 인덱스 (corridor mask 빌드용).
        struct CoarseAStarOut
        {
            bool                       found = false;
            std::vector<std::uint32_t> cells;
            std::uint64_t              expanded = 0;
        };

        //Fine A* 검색 박스 (inclusive bounds).
        struct FineBox { int x0, y0, x1, y1; };

        //══════════════════════════════════════════════════════════════════
        // 1) tile → pixel 역변환
        //══════════════════════════════════════════════════════════════════
        std::vector<CityPixel> cps;
        cps.reserve(cities.size());
        for (const auto& cn : cities)
        {
            cps.push_back(CityPixel{
                (cn.center.x - TILE_BASE_X) / TILES_PER_PIXEL,
                (cn.center.y - TILE_BASE_Y) / TILES_PER_PIXEL,
                cn.center.z
            });
        }

        //지형 술어 — 함수 안 여러 곳에서 사용. 단일 사용 술어는 호출처 안에 inline.
        constexpr auto isCity = [](Terrain t) noexcept {
            return t == Terrain::CityCenter || t == Terrain::CityZone
                || t == Terrain::CityRiver  || t == Terrain::CitySea;
        };
        constexpr auto isWater = [](Terrain t) noexcept {
            return t == Terrain::River || t == Terrain::Sea || t == Terrain::Lake;
        };

        //도시 카디널 경계 진입점 + step 방향 — 도시 중심에서 target 방향 주축으로 walk,
        //  첫 non-city 셀을 entry 로. step (stepX/stepY) = walk 방향 (도시 → 바깥).
        //  도시 중심 자체를 A* 끝점으로 쓰면 City* terrain cost=1000 누적 → cost cap reject.
        struct CityEntry { PixelCoord pixel; int stepX; int stepY; };
        auto boundaryEntryFor = [&grid, &cps](int cityIdx, int targetPx, int targetPy) noexcept -> CityEntry
        {
            const auto& cp = cps[cityIdx];
            const int dxRaw = targetPx - cp.px;
            const int dyRaw = targetPy - cp.py;
            int stepX = 0, stepY = 0;
            if (std::abs(dxRaw) >= std::abs(dyRaw)) stepX = (dxRaw >= 0) ? 1 : -1;
            else                                    stepY = (dyRaw >= 0) ? 1 : -1;

            int x = cp.px, y = cp.py;
            for (int step = 0; step < 200; ++step)
            {
                if (x < 0 || x >= PixelCostGrid::W || y < 0 || y >= PixelCostGrid::H) break;
                const Terrain t = grid.data[static_cast<std::size_t>(y) * PixelCostGrid::W + x];
                if (!isCity(t)) return CityEntry{ PixelCoord{ x, y, cp.z }, stepX, stepY };
                x += stepX;
                y += stepY;
            }
            return CityEntry{ PixelCoord{ cp.px, cp.py, cp.z }, stepX, stepY };
        };

        //직교 strut — boundary entry 에서 step 방향으로 최대 kStrutLen 픽셀 forced 카디널 연장.
        //  City/Sea/Lake 만나면 그 직전까지 (River 는 통과 — 다리). 반환 strut 의 첫/마지막을
        //  caller 가 폴리라인 시작/끝에 prepend/append → 폴리라인 첫·끝 segment 무조건 직각.
        static constexpr int kStrutLen = 16;
        auto computeStrut = [&grid](int entryX, int entryY, int stepX, int stepY, int z) -> std::vector<PixelCoord>
        {
            constexpr auto isHostile = [](Terrain t) noexcept {
                return t == Terrain::CityCenter || t == Terrain::CityZone
                    || t == Terrain::CityRiver  || t == Terrain::CitySea
                    || t == Terrain::Sea        || t == Terrain::Lake;
            };
            std::vector<PixelCoord> strut;
            strut.reserve(kStrutLen);
            strut.push_back(PixelCoord{ entryX, entryY, z });
            int x = entryX + stepX, y = entryY + stepY;
            for (int s = 1; s < kStrutLen; ++s)
            {
                if (x < 0 || x >= PixelCostGrid::W || y < 0 || y >= PixelCostGrid::H) break;
                const Terrain t = grid.data[static_cast<std::size_t>(y) * PixelCostGrid::W + x];
                if (isHostile(t)) break;
                strut.push_back(PixelCoord{ x, y, z });
                x += stepX;
                y += stepY;
            }
            return strut;
        };

        //══════════════════════════════════════════════════════════════════
        // 2) 평면 유클리드 MST — dense Prim's O(N²)
        //══════════════════════════════════════════════════════════════════
        //  N=3000 → 9M iter, ~50ms. 메모리 O(N). N-1 city-city 엣지, 트리 = 평행/중복 없음.
        //  거리 desc 정렬 → 긴 엣지 먼저 dispatch (threadpool tail latency 감소).
        const __int64 tMstStart = getNanoTimer();
        std::vector<EdgeCand> edges;
        {
            const int N = static_cast<int>(cps.size());
            std::vector<double> minE2(N, std::numeric_limits<double>::infinity());
            std::vector<int>    parent(N, -1);
            std::vector<bool>   inMST(N, false);
            minE2[0] = 0.0;
            for (int iter = 0; iter < N; ++iter)
            {
                int u = -1; double best2 = std::numeric_limits<double>::infinity();
                for (int v = 0; v < N; ++v)
                {
                    if (!inMST[v] && minE2[v] < best2) { best2 = minE2[v]; u = v; }
                }
                if (u < 0) break;
                inMST[u] = true;
                for (int v = 0; v < N; ++v)
                {
                    if (inMST[v]) continue;
                    const double dx = static_cast<double>(cps[u].px - cps[v].px);
                    const double dy = static_cast<double>(cps[u].py - cps[v].py);
                    const double d2 = dx * dx + dy * dy;
                    if (d2 < minE2[v]) { minE2[v] = d2; parent[v] = u; }
                }
            }
            edges.reserve(N - 1);
            for (int v = 0; v < N; ++v)
            {
                if (parent[v] >= 0)
                {
                    edges.push_back(EdgeCand{
                        std::min(parent[v], v),
                        std::max(parent[v], v),
                        std::sqrt(minE2[v])
                    });
                }
            }
            std::sort(edges.begin(), edges.end(),
                [](const EdgeCand& a, const EdgeCand& b) { return a.dist > b.dist; });
        }
        const __int64 tMstEnd = getNanoTimer();
        prt(L"  MST build          : %8.2f ms  (N=%zu cities, %zu edges)\n",
            (tMstEnd - tMstStart) / 1.0e6, cps.size(), edges.size());

        //--- 2-1) 엣지 거리 히스토그램 ---
        {
            constexpr int   BUCKETS = 8;
            const int       bounds[BUCKETS] = { 100, 200, 400, 800, 1200, 1800, 2500,
                                                 std::numeric_limits<int>::max() };
            const wchar_t*  labels[BUCKETS] = {
                L"  <100", L"100-200", L"200-400", L"400-800",
                L"800-1.2k", L"1.2-1.8k", L"1.8-2.5k", L"  >2.5k"
            };
            int hist[BUCKETS] = {};
            for (const auto& e : edges) {
                const int d = static_cast<int>(e.dist);
                for (int i = 0; i < BUCKETS; ++i)
                    if (d < bounds[i]) { ++hist[i]; break; }
            }
            prt(L"  edge distance histogram (px):\n");
            for (int i = 0; i < BUCKETS; ++i)
                prt(L"    %-9ls : %5d\n", labels[i], hist[i]);
        }

        //══════════════════════════════════════════════════════════════════
        // 3) Coarse 그리드 1회 빌드 — 8× 다운샘플, top-K=4 mean
        //══════════════════════════════════════════════════════════════════
        //  5400×2700 float = 58MB. 행 영역 nT 등분 병렬. top-4 mean → 단일 city 픽셀이 sea 위
        //  false positive corridor 만드는 것 차단 ("최소 4픽셀 폭 통로" 요구).
        const __int64 tCoarseStart = getNanoTimer();
        CoarseGrid coarse;
        {
            constexpr std::size_t total = static_cast<std::size_t>(kCoarseW) * kCoarseH;
            coarse.cost = std::make_unique<float[]>(total);

            const Terrain* fineData = grid.data.get();
            constexpr int Wf = PixelCostGrid::W;
            constexpr int Hf = PixelCostGrid::H;
            constexpr int F  = kCoarseF;

            const int nT = std::max<int>(2, static_cast<int>(std::thread::hardware_concurrency()));
            std::vector<std::thread> threads;
            threads.reserve(nT);
            const int rowsPerT = (kCoarseH + nT - 1) / nT;

            for (int t = 0; t < nT; ++t)
            {
                const int r0 = t * rowsPerT;
                const int r1 = std::min(r0 + rowsPerT, kCoarseH);
                if (r0 >= r1) break;
                threads.emplace_back([&, r0, r1]()
                {
                    float buf[F * F];
                    for (int cy = r0; cy < r1; ++cy)
                    {
                        const int y0 = cy * F;
                        const int y1 = std::min(y0 + F, Hf);
                        float* dstRow = &coarse.cost[static_cast<std::size_t>(cy) * kCoarseW];
                        for (int cx = 0; cx < kCoarseW; ++cx)
                        {
                            const int x0 = cx * F;
                            const int x1 = std::min(x0 + F, Wf);
                            int n = 0;
                            for (int y = y0; y < y1; ++y)
                            {
                                const Terrain* row = fineData + static_cast<std::size_t>(y) * Wf;
                                for (int x = x0; x < x1; ++x)
                                {
                                    buf[n++] = kCostLUT[static_cast<std::size_t>(row[x])];
                                }
                            }
                            const int kk = std::min(kCoarseAggK, n);
                            std::nth_element(buf, buf + kk - 1, buf + n);
                            float sum = 0.0f;
                            for (int i = 0; i < kk; ++i) sum += buf[i];
                            dstRow[cx] = sum / kk;
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();
        }
        const __int64 tCoarseEnd = getNanoTimer();
        prt(L"  coarse grid build  : %8.2f ms  (5400x2700 = 58MB float, top-%d mean)\n",
            (tCoarseEnd - tCoarseStart) / 1.0e6, kCoarseAggK);

        //══════════════════════════════════════════════════════════════════
        // 4) 공유 도로 그리드 — coarse 해상도 (5400×2700 uint8 = 14.58MB)
        //══════════════════════════════════════════════════════════════════
        //  성공한 폴리라인의 coarse cell 들을 1로 stamp. 후속 A* 가 terrain × kRoadDiscount 적용.
        //  동시 read/write 는 std::atomic_ref relaxed (byte 단위, fence 불필요).
        std::vector<std::uint8_t> roadGrid(static_cast<std::size_t>(kCoarseW) * kCoarseH, 0);

        //══════════════════════════════════════════════════════════════════
        // 5) A* 람다 — boxOf, astarBidir, astarCoarse, pathOneEdge
        //══════════════════════════════════════════════════════════════════

        //fine 박스 — A/B 양 끝 + marginPx 로 inclusive bounds.
        auto boxOf = [](PixelCoord A, PixelCoord B, int marginPx) noexcept -> FineBox
        {
            return FineBox{
                std::max(0,                    std::min(A.x, B.x) - marginPx),
                std::max(0,                    std::min(A.y, B.y) - marginPx),
                std::min(PixelCostGrid::W - 1, std::max(A.x, B.x) + marginPx),
                std::min(PixelCostGrid::H - 1, std::max(A.y, B.y) + marginPx)
            };
        };

        //Bidirectional fine A* — 양쪽 동시 expand, 작은 top.f 쪽 먼저 → ~2× 가속.
        //  x0w/y0w/x1w/y1w: 명시적 박스 경계. mask: corridor pruning (nullptr 면 박스 전체).
        //  initDirF/initDirB: 시작/끝 셀의 prevDir 초기값 (strut 방향 또는 kNoDir).
        //    strut 방향을 넘기면 첫 step 이 strut 와 정합 (turn cost, no-turn-on-water,
        //    meeting bend check 모두 일관 처리). strut.back() 이 강 위인 경우의 bend 차단.
        auto astarBidir = [&grid, &roadGrid](PixelCoord start, PixelCoord end, int x0w, int y0w, int x1w, int y1w, AStarWorkspace& ws, const CorridorMask* mask, std::uint8_t initDirF, std::uint8_t initDirB) -> AStarOut
        {
            AStarOut R{};
            const int W = PixelCostGrid::W;

            const int boxW = x1w - x0w + 1;
            const int boxH = y1w - y0w + 1;
            const std::size_t total = static_cast<std::size_t>(boxW) * boxH;

            const std::size_t kMaxBoxCells = mask
                ? (4ull * 1024 * 1024)
                : (2ull * 1024 * 1024);
            if (total == 0 || total > kMaxBoxCells) return R;

            if (start.x < x0w || start.x > x1w || start.y < y0w || start.y > y1w) return R;
            if (end.x   < x0w || end.x   > x1w || end.y   < y0w || end.y   > y1w) return R;

            ws.prepare(total, /*needBackward=*/true);
            const std::uint16_t curGenF    = ws.curGenF;
            const std::uint16_t closedGenF = static_cast<std::uint16_t>(curGenF + 1);
            const std::uint16_t curGenB    = ws.curGenB;
            const std::uint16_t closedGenB = static_cast<std::uint16_t>(curGenB + 1);

            float* const         gF_   = ws.gF.data();
            std::uint8_t* const  dirF_ = ws.dirF.data();
            std::uint16_t* const genF_ = ws.genF.data();
            float* const         gB_   = ws.gB.data();
            std::uint8_t* const  dirB_ = ws.dirB.data();
            std::uint16_t* const genB_ = ws.genB.data();
            const Terrain* const gridData = grid.data.get();
            std::uint8_t* const roadData = roadGrid.data();

            const std::ptrdiff_t boxW_p = boxW;
            const std::ptrdiff_t W_p    = W;
            const std::ptrdiff_t boxStep[4]  = {  1, -1,  boxW_p, -boxW_p };
            const std::ptrdiff_t gridStep[4] = {  1, -1,  W_p,    -W_p    };

            auto idxOf = [&](int x, int y) -> std::size_t
            {
                return static_cast<std::size_t>(y - y0w) * boxW + (x - x0w);
            };

            const int sxFin = start.x, syFin = start.y;
            const int exFin = end.x,   eyFin = end.y;
            auto manhattanTo = [](int x, int y, int tx, int ty) -> float
            {
                return static_cast<float>(std::abs(x - tx) + std::abs(y - ty))
                    * kMinCost * kHeuristicWeight;
            };

            const std::size_t sIdx = idxOf(start.x, start.y);
            const std::size_t eIdx = idxOf(end.x,   end.y);
            if (sIdx == eIdx)
            {
                R.path.push_back(PixelCoord{ start.x, start.y, start.z });
                R.totalCost = 0.0f;
                return R;
            }

            gF_[sIdx]   = 0.0f;
            dirF_[sIdx] = initDirF;
            genF_[sIdx] = curGenF;
            ws.heapF.push_back(HeapNode{ manhattanTo(sxFin, syFin, exFin, eyFin), 0.0f, sIdx });
            std::push_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});

            gB_[eIdx]   = 0.0f;
            dirB_[eIdx] = initDirB;
            genB_[eIdx] = curGenB;
            ws.heapB.push_back(HeapNode{ manhattanTo(exFin, eyFin, sxFin, syFin), 0.0f, eIdx });
            std::push_heap(ws.heapB.begin(), ws.heapB.end(), std::greater<HeapNode>{});

            float        mu          = std::numeric_limits<float>::infinity();
            std::size_t  meetingIdx  = static_cast<std::size_t>(-1);

            while (!ws.heapF.empty() && !ws.heapB.empty())
            {
                const float topFf = ws.heapF.front().f;
                const float topBf = ws.heapB.front().f;

                if (topFf >= mu && topBf >= mu) break;

                const bool expandFwd = (topFf <= topBf);

                if (expandFwd)
                {
                    std::pop_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});
                    const HeapNode top = ws.heapF.back();
                    ws.heapF.pop_back();

                    if (genF_[top.idx] == closedGenF) continue;
                    if (genF_[top.idx] != curGenF || gF_[top.idx] != top.gAtPush) continue;

                    genF_[top.idx] = closedGenF;
                    ++R.expanded;

                    const int cy = static_cast<int>(top.idx / boxW);
                    const int cx = static_cast<int>(top.idx) - cy * boxW;
                    const int curX = cx + x0w;
                    const int curY = cy + y0w;
                    const std::size_t gridCur = static_cast<std::size_t>(curY) * W + curX;
                    const float gCur = top.gAtPush;

                    const std::uint8_t prevDir = dirF_[top.idx];

                    //meeting bend 차단 — meeting 셀이 물이고 dF^1 != dB 면 폴리라인이 meeting
                    //  셀에서 꺾임 (dF^1 = 반대 카디널: 0↔1=E↔W, 2↔3=S↔N). bend on water 면 거부.
                    if (genB_[top.idx] == curGenB || genB_[top.idx] == closedGenB)
                    {
                        const bool meetWater = isWater(gridData[gridCur]);
                        const std::uint8_t dF = prevDir;
                        const std::uint8_t dB = dirB_[top.idx];
                        const bool waterBend = meetWater && dF < 4 && dB < 4
                                            && static_cast<std::uint8_t>(dF ^ 1) != dB;
                        if (!waterBend)
                        {
                            const float cand = top.gAtPush + gB_[top.idx];
                            if (cand < mu) { mu = cand; meetingIdx = top.idx; }
                        }
                    }

                    //cur/prev 셀 물 여부 — turn 금지 판정에 사용 (next 는 d 별이라 in-loop).
                    const bool curIsWater = isWater(gridData[gridCur]);
                    bool prevIsWater = false;
                    if (prevDir < 4)
                    {
                        const int prevX = curX - kSteps[prevDir].dx;
                        const int prevY = curY - kSteps[prevDir].dy;
                        prevIsWater = isWater(gridData[static_cast<std::size_t>(prevY) * W + prevX]);
                    }

                    for (int d = 0; d < 4; ++d)
                    {
                        const auto& s = kSteps[d];
                        const int nx = curX + s.dx;
                        const int ny = curY + s.dy;
                        if (nx < x0w || nx > x1w || ny < y0w || ny > y1w) continue;

                        if (mask)
                        {
                            const std::uint32_t coarseIdx =
                                (static_cast<std::uint32_t>(ny) / kCoarseF) * kCoarseW
                                + (static_cast<std::uint32_t>(nx) / kCoarseF);
                            if (!mask->test(coarseIdx)) continue;
                        }

                        const std::size_t nIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(top.idx) + boxStep[d]);
                        if (genF_[nIdx] == closedGenF) continue;

                        const std::size_t gridNIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(gridCur) + gridStep[d]);
                        //City* 셀 hard barrier — 도시 가로지름 차단. start/end 는 boundaryEntryFor
                        //  가 non-city 셀이라 영향 없음.
                        const Terrain nextT = gridData[gridNIdx];
                        if (isCity(nextT)) continue;
                        //물 주변 turn 금지 (1셀 land buffer) — cur/prev/next 중 하나라도 물이면
                        //  방향 전환 skip → 다리 ─bridge─ 직선 + 양 끝 1셀 land 보장.
                        const bool nextIsWater = isWater(nextT);
                        if ((curIsWater || prevIsWater || nextIsWater)
                            && prevDir < 4 && static_cast<std::uint8_t>(d) != prevDir) continue;
                        const float terrain = kCostLUT[static_cast<std::size_t>(nextT)];
                        const std::uint32_t roadCIdx =
                            (static_cast<std::uint32_t>(ny) >> 3) * kCoarseW
                            + (static_cast<std::uint32_t>(nx) >> 3);
                        const bool onRoad = std::atomic_ref<std::uint8_t>(roadData[roadCIdx])
                                                .load(std::memory_order_relaxed) != 0;
                        const float effTerrain = onRoad ? terrain * kRoadDiscount : terrain;
                        const float turn    = (prevDir < 4 && static_cast<std::uint8_t>(d) != prevDir) ? kTurnCost : 0.0f;
                        const float gNew    = gCur + effTerrain + turn;

                        const float gPrev = (genF_[nIdx] == curGenF) ? gF_[nIdx]
                                                                      : std::numeric_limits<float>::infinity();
                        if (gNew < gPrev)
                        {
                            gF_[nIdx]   = gNew;
                            dirF_[nIdx] = static_cast<std::uint8_t>(d);
                            genF_[nIdx] = curGenF;

                            //meeting bend 차단 (on relax) — 동일 규칙.
                            if (genB_[nIdx] == curGenB || genB_[nIdx] == closedGenB)
                            {
                                const bool meetWater = nextIsWater;
                                const std::uint8_t dF = static_cast<std::uint8_t>(d);
                                const std::uint8_t dB = dirB_[nIdx];
                                const bool waterBend = meetWater && dB < 4
                                                    && static_cast<std::uint8_t>(dF ^ 1) != dB;
                                if (!waterBend)
                                {
                                    const float cand = gNew + gB_[nIdx];
                                    if (cand < mu) { mu = cand; meetingIdx = nIdx; }
                                }
                            }

                            const float fNew = gNew + manhattanTo(nx, ny, exFin, eyFin);
                            ws.heapF.push_back(HeapNode{ fNew, gNew, nIdx });
                            std::push_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});
                            const std::size_t hs = ws.heapF.size();
                            if (hs > R.maxHeapSize) R.maxHeapSize = hs;
                        }
                    }
                }
                else
                {
                    std::pop_heap(ws.heapB.begin(), ws.heapB.end(), std::greater<HeapNode>{});
                    const HeapNode top = ws.heapB.back();
                    ws.heapB.pop_back();

                    if (genB_[top.idx] == closedGenB) continue;
                    if (genB_[top.idx] != curGenB || gB_[top.idx] != top.gAtPush) continue;

                    genB_[top.idx] = closedGenB;
                    ++R.expanded;

                    const int cy = static_cast<int>(top.idx / boxW);
                    const int cx = static_cast<int>(top.idx) - cy * boxW;
                    const int curX = cx + x0w;
                    const int curY = cy + y0w;
                    const std::size_t gridCur = static_cast<std::size_t>(curY) * W + curX;
                    const float gCur = top.gAtPush;

                    const std::uint8_t prevDir = dirB_[top.idx];

                    //meeting bend 차단 (on close) — 동일 규칙.
                    if (genF_[top.idx] == curGenF || genF_[top.idx] == closedGenF)
                    {
                        const bool meetWater = isWater(gridData[gridCur]);
                        const std::uint8_t dF = dirF_[top.idx];
                        const std::uint8_t dB = prevDir;
                        const bool waterBend = meetWater && dF < 4 && dB < 4
                                            && static_cast<std::uint8_t>(dF ^ 1) != dB;
                        if (!waterBend)
                        {
                            const float cand = top.gAtPush + gF_[top.idx];
                            if (cand < mu) { mu = cand; meetingIdx = top.idx; }
                        }
                    }

                    //cur/prev 셀 물 여부 — turn 금지 판정에 사용 (next 는 d 별이라 in-loop).
                    const bool curIsWater = isWater(gridData[gridCur]);
                    bool prevIsWater = false;
                    if (prevDir < 4)
                    {
                        const int prevX = curX - kSteps[prevDir].dx;
                        const int prevY = curY - kSteps[prevDir].dy;
                        prevIsWater = isWater(gridData[static_cast<std::size_t>(prevY) * W + prevX]);
                    }

                    for (int d = 0; d < 4; ++d)
                    {
                        const auto& s = kSteps[d];
                        const int nx = curX + s.dx;
                        const int ny = curY + s.dy;
                        if (nx < x0w || nx > x1w || ny < y0w || ny > y1w) continue;

                        if (mask)
                        {
                            const std::uint32_t coarseIdx =
                                (static_cast<std::uint32_t>(ny) / kCoarseF) * kCoarseW
                                + (static_cast<std::uint32_t>(nx) / kCoarseF);
                            if (!mask->test(coarseIdx)) continue;
                        }

                        const std::size_t nIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(top.idx) + boxStep[d]);
                        if (genB_[nIdx] == closedGenB) continue;

                        const std::size_t gridNIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(gridCur) + gridStep[d]);
                        //City* 셀 hard barrier — 도시 가로지름 차단. start/end 는 boundaryEntryFor
                        //  가 non-city 셀이라 영향 없음.
                        const Terrain nextT = gridData[gridNIdx];
                        if (isCity(nextT)) continue;
                        //물 주변 turn 금지 (1셀 land buffer) — cur/prev/next 중 하나라도 물이면
                        //  방향 전환 skip → 다리 ─bridge─ 직선 + 양 끝 1셀 land 보장.
                        const bool nextIsWater = isWater(nextT);
                        if ((curIsWater || prevIsWater || nextIsWater)
                            && prevDir < 4 && static_cast<std::uint8_t>(d) != prevDir) continue;
                        const float terrain = kCostLUT[static_cast<std::size_t>(nextT)];
                        const std::uint32_t roadCIdx =
                            (static_cast<std::uint32_t>(ny) >> 3) * kCoarseW
                            + (static_cast<std::uint32_t>(nx) >> 3);
                        const bool onRoad = std::atomic_ref<std::uint8_t>(roadData[roadCIdx])
                                                .load(std::memory_order_relaxed) != 0;
                        const float effTerrain = onRoad ? terrain * kRoadDiscount : terrain;
                        const float turn    = (prevDir < 4 && static_cast<std::uint8_t>(d) != prevDir) ? kTurnCost : 0.0f;
                        const float gNew    = gCur + effTerrain + turn;

                        const float gPrev = (genB_[nIdx] == curGenB) ? gB_[nIdx]
                                                                      : std::numeric_limits<float>::infinity();
                        if (gNew < gPrev)
                        {
                            gB_[nIdx]   = gNew;
                            dirB_[nIdx] = static_cast<std::uint8_t>(d);
                            genB_[nIdx] = curGenB;

                            //meeting bend 차단 (on relax) — 동일 규칙.
                            if (genF_[nIdx] == curGenF || genF_[nIdx] == closedGenF)
                            {
                                const bool meetWater = nextIsWater;
                                const std::uint8_t dF = dirF_[nIdx];
                                const std::uint8_t dB = static_cast<std::uint8_t>(d);
                                const bool waterBend = meetWater && dF < 4
                                                    && static_cast<std::uint8_t>(dF ^ 1) != dB;
                                if (!waterBend)
                                {
                                    const float cand = gNew + gF_[nIdx];
                                    if (cand < mu) { mu = cand; meetingIdx = nIdx; }
                                }
                            }

                            const float fNew = gNew + manhattanTo(nx, ny, sxFin, syFin);
                            ws.heapB.push_back(HeapNode{ fNew, gNew, nIdx });
                            std::push_heap(ws.heapB.begin(), ws.heapB.end(), std::greater<HeapNode>{});
                            const std::size_t hs = ws.heapB.size();
                            if (hs > R.maxHeapSize) R.maxHeapSize = hs;
                        }
                    }
                }
            }

            if (meetingIdx == static_cast<std::size_t>(-1)) return R;

            std::vector<PixelCoord> fwdPart;
            fwdPart.reserve(64);
            {
                int cx = static_cast<int>(meetingIdx) % boxW + x0w;
                int cy = static_cast<int>(meetingIdx) / boxW + y0w;
                std::size_t cur = meetingIdx;
                while (true)
                {
                    fwdPart.push_back(PixelCoord{ cx, cy, start.z });
                    if (cur == sIdx) break;
                    const std::uint8_t d = dirF_[cur];
                    if (d >= 4) { return R; }
                    cx -= kSteps[d].dx;
                    cy -= kSteps[d].dy;
                    cur = idxOf(cx, cy);
                }
                std::reverse(fwdPart.begin(), fwdPart.end());
            }

            std::vector<PixelCoord> bwdPart;
            bwdPart.reserve(64);
            {
                int cx = static_cast<int>(meetingIdx) % boxW + x0w;
                int cy = static_cast<int>(meetingIdx) / boxW + y0w;
                std::size_t cur = meetingIdx;
                while (cur != eIdx)
                {
                    const std::uint8_t d = dirB_[cur];
                    if (d >= 4) { return R; }
                    cx -= kSteps[d].dx;
                    cy -= kSteps[d].dy;
                    cur = idxOf(cx, cy);
                    bwdPart.push_back(PixelCoord{ cx, cy, start.z });
                }
            }

            R.path = std::move(fwdPart);
            R.path.insert(R.path.end(), bwdPart.begin(), bwdPart.end());
            R.totalCost = mu;
            return R;
        };

        //Coarse A* — coarse 그리드 위 단방향 weighted A*. corridor 셀 리스트 반환.
        //  workspace 는 fine 과 공유 (F 만 사용, B 미사용). margin: 박스 확장 cell 수.
        auto astarCoarse = [&roadGrid](int sx, int sy, int ex, int ey, const CoarseGrid& coarse, int margin, AStarWorkspace& ws) -> CoarseAStarOut
        {
            CoarseAStarOut R{};
            constexpr int W = kCoarseW;
            constexpr int H = kCoarseH;

            const int x0w = std::max(0,     std::min(sx, ex) - margin);
            const int y0w = std::max(0,     std::min(sy, ey) - margin);
            const int x1w = std::min(W - 1, std::max(sx, ex) + margin);
            const int y1w = std::min(H - 1, std::max(sy, ey) + margin);
            const int boxW = x1w - x0w + 1;
            const int boxH = y1w - y0w + 1;
            const std::size_t total = static_cast<std::size_t>(boxW) * boxH;
            if (total == 0) return R;

            ws.prepare(total, /*needBackward=*/false);
            const std::uint16_t curGen    = ws.curGenF;
            const std::uint16_t closedGen = static_cast<std::uint16_t>(curGen + 1);
            float* const         g_       = ws.gF.data();
            std::uint8_t* const  dir_     = ws.dirF.data();
            std::uint16_t* const gen_     = ws.genF.data();
            const float* const   costData = coarse.cost.get();
            std::uint8_t* const  roadData = roadGrid.data();

            const std::ptrdiff_t boxW_p = boxW;
            const std::ptrdiff_t W_p    = W;
            const std::ptrdiff_t boxStep[4]  = {  1, -1,  boxW_p, -boxW_p };
            const std::ptrdiff_t gridStep[4] = {  1, -1,  W_p,    -W_p    };

            auto idxOf = [&](int x, int y) -> std::size_t
            {
                return static_cast<std::size_t>(y - y0w) * boxW + (x - x0w);
            };

            auto manhattan = [&](int x, int y) -> float
            {
                return static_cast<float>(std::abs(x - ex) + std::abs(y - ey))
                    * kMinCost * kCoarseHeuristicWeight;
            };

            const std::size_t sIdx = idxOf(sx, sy);
            const std::size_t gIdx = idxOf(ex, ey);
            g_[sIdx]   = 0.0f;
            dir_[sIdx] = kNoDir;
            gen_[sIdx] = curGen;

            ws.heapF.push_back(HeapNode{ manhattan(sx, sy), 0.0f, sIdx });
            std::push_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});

            bool found = false;
            while (!ws.heapF.empty())
            {
                std::pop_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});
                const HeapNode top = ws.heapF.back();
                ws.heapF.pop_back();

                if (gen_[top.idx] == closedGen) continue;
                if (gen_[top.idx] != curGen || g_[top.idx] != top.gAtPush) continue;

                gen_[top.idx] = closedGen;
                ++R.expanded;

                if (top.idx == gIdx) { found = true; break; }

                const int cy = static_cast<int>(top.idx / boxW);
                const int cx = static_cast<int>(top.idx) - cy * boxW;
                const int curX = cx + x0w;
                const int curY = cy + y0w;
                const std::size_t gridCur = static_cast<std::size_t>(curY) * W + curX;
                const float gCur = top.gAtPush;

                const std::uint8_t prevDir = dir_[top.idx];

                for (int d = 0; d < 4; ++d)
                {
                    const auto& s = kSteps[d];
                    const int nx = curX + s.dx;
                    const int ny = curY + s.dy;
                    if (nx < x0w || nx > x1w || ny < y0w || ny > y1w) continue;

                    const std::size_t nIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(top.idx) + boxStep[d]);
                    if (gen_[nIdx] == closedGen) continue;

                    const std::size_t gridNIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(gridCur) + gridStep[d]);
                    const float terrain = costData[gridNIdx];
                    const bool onRoad = std::atomic_ref<std::uint8_t>(roadData[gridNIdx])
                                            .load(std::memory_order_relaxed) != 0;
                    const float effTerrain = onRoad ? terrain * kRoadDiscount : terrain;
                    const float turn    = (prevDir < 4 && static_cast<std::uint8_t>(d) != prevDir) ? kTurnCost : 0.0f;
                    const float gNew    = gCur + effTerrain + turn;

                    const float gPrev = (gen_[nIdx] == curGen) ? g_[nIdx]
                                                                : std::numeric_limits<float>::infinity();
                    if (gNew < gPrev)
                    {
                        g_[nIdx]   = gNew;
                        dir_[nIdx] = static_cast<std::uint8_t>(d);
                        gen_[nIdx] = curGen;
                        const float fNew = gNew + manhattan(nx, ny);
                        ws.heapF.push_back(HeapNode{ fNew, gNew, nIdx });
                        std::push_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});
                    }
                }
            }

            if (!found) return R;

            std::vector<std::uint32_t> rev;
            rev.reserve(64);
            int cx = ex, cy = ey;
            while (true)
            {
                rev.push_back(static_cast<std::uint32_t>(cy) * kCoarseW + cx);
                const std::size_t cur = idxOf(cx, cy);
                if (cur == sIdx) break;
                const std::uint8_t d = dir_[cur];
                if (d >= 4) { rev.clear(); return R; }
                cx -= kSteps[d].dx;
                cy -= kSteps[d].dy;
            }
            std::reverse(rev.begin(), rev.end());
            R.cells = std::move(rev);
            R.found = true;
            return R;
        };

        //한 엣지 처리 — Hierarchical bidirectional A*.
        //  짧은 엣지(<400px): fine A* 만, margin retry. 긴 엣지: coarse → corridor → fine.
        //  initDirA/initDirB: strut 방향 (cardinal index 0-3 또는 kNoDir). astarBidir 로 전달.
        auto pathOneEdge = [&boxOf, &astarBidir, &astarCoarse, &grid, &coarse](PixelCoord A, PixelCoord B, double directDistPx, AStarWorkspace& ws, std::uint64_t& fineExpanded, std::uint64_t& coarseExpanded, std::size_t& maxHeap, std::uint8_t initDirA, std::uint8_t initDirB) -> std::vector<PixelCoord>
        {
            if (directDistPx > kMaxEdgeAttempt) return {};

            const float costCap = kMaxCostPerPixel * static_cast<float>(directDistPx);

            if (directDistPx < 400.0)
            {
                const int margin1 = std::max(60, static_cast<int>(directDistPx * 0.10));
                const FineBox b1 = boxOf(A, B, margin1);
                AStarOut r1 = astarBidir(A, B, b1.x0, b1.y0, b1.x1, b1.y1, ws, nullptr, initDirA, initDirB);
                fineExpanded += r1.expanded;
                if (r1.maxHeapSize > maxHeap) maxHeap = r1.maxHeapSize;
                if (!r1.path.empty())
                {
                    if (r1.totalCost > costCap) return {};
                    return r1.path;
                }

                const int margin2 = std::max(180, static_cast<int>(directDistPx * 0.30));
                const FineBox b2 = boxOf(A, B, margin2);
                AStarOut r2 = astarBidir(A, B, b2.x0, b2.y0, b2.x1, b2.y1, ws, nullptr, initDirA, initDirB);
                fineExpanded += r2.expanded;
                if (r2.maxHeapSize > maxHeap) maxHeap = r2.maxHeapSize;
                if (r2.path.empty() || r2.totalCost > costCap) return {};
                return std::move(r2.path);
            }

            const int sxC = A.x / kCoarseF;
            const int syC = A.y / kCoarseF;
            const int exC = B.x / kCoarseF;
            const int eyC = B.y / kCoarseF;
            const int coarseDist = std::max(std::abs(sxC - exC), std::abs(syC - eyC));
            const int coarseMargin = std::max(40, coarseDist / 2);

            CoarseAStarOut cr = astarCoarse(sxC, syC, exC, eyC, coarse, coarseMargin, ws);
            coarseExpanded += cr.expanded;

            if (!cr.found || cr.cells.empty()) return {};

            constexpr int kHalo = 3;
            ws.mask.prepare();
            int cMinX = std::numeric_limits<int>::max();
            int cMinY = std::numeric_limits<int>::max();
            int cMaxX = std::numeric_limits<int>::min();
            int cMaxY = std::numeric_limits<int>::min();
            for (std::uint32_t idx : cr.cells)
            {
                const int cy = static_cast<int>(idx / kCoarseW);
                const int cx = static_cast<int>(idx % kCoarseW);
                if (cx < cMinX) cMinX = cx;
                if (cx > cMaxX) cMaxX = cx;
                if (cy < cMinY) cMinY = cy;
                if (cy > cMaxY) cMaxY = cy;
                const int x0 = std::max(0, cx - kHalo);
                const int x1 = std::min(kCoarseW - 1, cx + kHalo);
                const int y0 = std::max(0, cy - kHalo);
                const int y1 = std::min(kCoarseH - 1, cy + kHalo);
                for (int yy = y0; yy <= y1; ++yy)
                {
                    const std::uint32_t base = static_cast<std::uint32_t>(yy) * kCoarseW;
                    for (int xx = x0; xx <= x1; ++xx)
                    {
                        ws.mask.mark(base + static_cast<std::uint32_t>(xx));
                    }
                }
            }

            const int cbMinX = std::max(0, cMinX - kHalo);
            const int cbMinY = std::max(0, cMinY - kHalo);
            const int cbMaxX = std::min(kCoarseW - 1, cMaxX + kHalo);
            const int cbMaxY = std::min(kCoarseH - 1, cMaxY + kHalo);
            const int fbX0 = std::max(0,                    std::min({ cbMinX * kCoarseF,         A.x, B.x }));
            const int fbY0 = std::max(0,                    std::min({ cbMinY * kCoarseF,         A.y, B.y }));
            const int fbX1 = std::min(PixelCostGrid::W - 1, std::max({ (cbMaxX + 1) * kCoarseF - 1, A.x, B.x }));
            const int fbY1 = std::min(PixelCostGrid::H - 1, std::max({ (cbMaxY + 1) * kCoarseF - 1, A.y, B.y }));

            AStarOut r = astarBidir(A, B, fbX0, fbY0, fbX1, fbY1, ws, &ws.mask, initDirA, initDirB);
            fineExpanded += r.expanded;
            if (r.maxHeapSize > maxHeap) maxHeap = r.maxHeapSize;
            if (r.path.empty() || r.totalCost > costCap) return {};
            return std::move(r.path);
        };

        //══════════════════════════════════════════════════════════════════
        // 6) 병렬 A* — MST 엣지 처리 (단일 tier)
        //══════════════════════════════════════════════════════════════════
        const std::size_t numThreads =
            std::max<std::size_t>(2, std::thread::hardware_concurrency());

        std::atomic<int>           successCount  { 0 };
        std::atomic<int>           failCount     { 0 };
        std::atomic<std::uint64_t> fineExpTotal  { 0 };
        std::atomic<std::uint64_t> coarseExpTotal{ 0 };
        std::atomic<std::uint64_t> vertsTotal    { 0 };
        std::atomic<long long>     maxEdgeNs     { 0 };
        std::atomic<int>           maxEdgeIdx    { -1 };
        std::atomic<std::uint64_t> maxFineExp    { 0 };
        std::atomic<std::uint64_t> maxHeapSeen   { 0 };

        std::mutex                resultsMtx;
        std::vector<RoadPolyLine> results;
        results.reserve(edges.size());

        //safety net 용 자료구조 — succPairs: 성공한 city pair 들 (DSU 빌드 입력).
        //  attempted: 시도한 pair set (정렬된 a/b 64-bit key) → safety net 이 동일 fail pair 재시도 방지.
        auto encodePair = [](int a, int b) noexcept -> std::uint64_t {
            const std::uint32_t lo = static_cast<std::uint32_t>(std::min(a, b));
            const std::uint32_t hi = static_cast<std::uint32_t>(std::max(a, b));
            return (static_cast<std::uint64_t>(hi) << 32) | lo;
        };
        std::mutex                            succMtx;
        std::vector<std::pair<int, int>>      succPairs;
        std::mutex                            attemptedMtx;
        std::unordered_set<std::uint64_t>     attempted;
        succPairs.reserve(edges.size());
        attempted.reserve(edges.size() * 4);

        const __int64 tPathStart = getNanoTimer();

        //진행 워치 스레드 — 2초마다 ok/fail/경과시간 출력. worldgen 이 수십초 걸려서 모니터링 필요.
        std::atomic<bool> watcherStop{ false };
        std::jthread watcher([&](std::stop_token st)
        {
            while (!st.stop_requested() && !watcherStop.load(std::memory_order_acquire))
            {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                if (watcherStop.load(std::memory_order_acquire)) break;
                const int ok  = successCount.load(std::memory_order_relaxed);
                const int fn  = failCount   .load(std::memory_order_relaxed);
                const int tot = static_cast<int>(edges.size());
                const __int64 nowNs = getNanoTimer();
                const double secs = (nowNs - tPathStart) / 1.0e9;
                prt(L"  ... A* %d/%d  (ok=%d fail=%d)  %.1fs elapsed\n",
                    ok + fn, tot, ok, fn, secs);
            }
        });

        ThreadPool pool(numThreads);

        //엣지 1개 처리 — 양 도시에서 boundaryEntryFor + computeStrut, strut 끝점 간 A*,
        //  strut prepend/append, 4-방향 turn-keep 단순화, roadGrid stamp, 픽셀→타일.
        //  attempted 에 city pair 등록 (성공/실패 무관 — safety net 이 동일 pair 재시도 방지).
        auto edgeTask = [&](EdgeCand e, int eIdx)
        {
            thread_local AStarWorkspace ws;

            const __int64 tEdgeStart = getNanoTimer();

            {
                std::lock_guard<std::mutex> lk(attemptedMtx);
                attempted.insert(encodePair(e.a, e.b));
            }

            const CityEntry ceA = boundaryEntryFor(e.a, cps[e.b].px, cps[e.b].py);
            const CityEntry ceB = boundaryEntryFor(e.b, cps[e.a].px, cps[e.a].py);
            const std::vector<PixelCoord> strutA = computeStrut(ceA.pixel.x, ceA.pixel.y, ceA.stepX, ceA.stepY, ceA.pixel.z);
            const std::vector<PixelCoord> strutB = computeStrut(ceB.pixel.x, ceB.pixel.y, ceB.stepX, ceB.stepY, ceB.pixel.z);
            const PixelCoord aStart = strutA.back();
            const PixelCoord bEnd   = strutB.back();
            if (aStart == bEnd)
            {
                failCount.fetch_add(1, std::memory_order_relaxed);
                return;
            }

            //A* 거리 = strut 끝점 간 직선거리 (pathOneEdge short/long 분기 + cost cap 에 사용).
            const double adx = static_cast<double>(aStart.x - bEnd.x);
            const double ady = static_cast<double>(aStart.y - bEnd.y);
            const double astarDist = std::sqrt(adx * adx + ady * ady);

            //strut step → kSteps 인덱스 (E=0, W=1, S=2, N=3) — A* 시작/끝 prevDir 초기값.
            auto encodeDir = [](int sx, int sy) noexcept -> std::uint8_t {
                if (sx ==  1) return 0;
                if (sx == -1) return 1;
                if (sy ==  1) return 2;
                return 3;
            };
            const std::uint8_t initDirA = encodeDir(ceA.stepX, ceA.stepY);
            const std::uint8_t initDirB = encodeDir(ceB.stepX, ceB.stepY);

            std::uint64_t fExp = 0, cExp = 0;
            std::size_t   hMax = 0;
            std::vector<PixelCoord> raw =
                pathOneEdge(aStart, bEnd, astarDist, ws, fExp, cExp, hMax, initDirA, initDirB);

            fineExpTotal  .fetch_add(fExp, std::memory_order_relaxed);
            coarseExpTotal.fetch_add(cExp, std::memory_order_relaxed);

            {
                std::uint64_t prev = maxFineExp.load(std::memory_order_relaxed);
                while (fExp > prev && !maxFineExp.compare_exchange_weak(
                    prev, fExp, std::memory_order_relaxed)) {}
            }
            {
                std::uint64_t prev = maxHeapSeen.load(std::memory_order_relaxed);
                while (static_cast<std::uint64_t>(hMax) > prev && !maxHeapSeen.compare_exchange_weak(
                    prev, hMax, std::memory_order_relaxed)) {}
            }

            const __int64 tEdgeEnd = getNanoTimer();
            const long long ns = tEdgeEnd - tEdgeStart;
            long long prevNs = maxEdgeNs.load(std::memory_order_relaxed);
            while (ns > prevNs && !maxEdgeNs.compare_exchange_weak(
                prevNs, ns, std::memory_order_relaxed)) {}
            if (ns >= maxEdgeNs.load(std::memory_order_relaxed))
                maxEdgeIdx.store(eIdx, std::memory_order_relaxed);

            if (raw.empty())
            {
                failCount.fetch_add(1, std::memory_order_relaxed);
                return;
            }

            //폴리라인 합성 — strutA + A*[1..] + reverse(strutB)[1..]. 양쪽 끝 셀 중복 skip.
            std::vector<PixelCoord> full;
            full.reserve(strutA.size() + raw.size() + strutB.size());
            full.insert(full.end(), strutA.begin(), strutA.end());
            if (raw.size() > 1)
                full.insert(full.end(), raw.begin() + 1, raw.end());
            for (auto it = strutB.rbegin() + 1; it != strutB.rend(); ++it)
                full.push_back(*it);

            //4-방향 turn-keep 단순화 — 같은 방향 직진 구간 collapse, 방향 변곡점만 보존.
            std::vector<PixelCoord> simp;
            if (full.size() <= 2)
            {
                simp = std::move(full);
            }
            else
            {
                simp.reserve(full.size() / 4 + 2);
                simp.push_back(full.front());
                for (std::size_t k = 1; k + 1 < full.size(); ++k)
                {
                    const int dxIn  = full[k    ].x - full[k - 1].x;
                    const int dyIn  = full[k    ].y - full[k - 1].y;
                    const int dxOut = full[k + 1].x - full[k    ].x;
                    const int dyOut = full[k + 1].y - full[k    ].y;
                    if (dxIn != dxOut || dyIn != dyOut) simp.push_back(full[k]);
                }
                simp.push_back(full.back());
            }
            vertsTotal.fetch_add(simp.size(), std::memory_order_relaxed);

            //roadGrid stamp — 후속 A* 가 kRoadDiscount 적용. coarse 해상도 (8 fine px = 1 cell).
            //  std::atomic_ref relaxed store — 동시 read 와 race-free.
            {
                std::uint8_t* const rd = roadGrid.data();
                for (std::size_t k = 0; k + 1 < simp.size(); ++k)
                {
                    const int cx0 = simp[k    ].x >> 3;
                    const int cy0 = simp[k    ].y >> 3;
                    const int cx1 = simp[k + 1].x >> 3;
                    const int cy1 = simp[k + 1].y >> 3;
                    const int cdx = (cx1 > cx0) - (cx1 < cx0);
                    const int cdy = (cy1 > cy0) - (cy1 < cy0);
                    int cx = cx0, cy = cy0;
                    while (true)
                    {
                        const std::uint32_t cidx = static_cast<std::uint32_t>(cy) * kCoarseW + cx;
                        std::atomic_ref<std::uint8_t>(rd[cidx])
                            .store(1, std::memory_order_relaxed);
                        if (cx == cx1 && cy == cy1) break;
                        cx += cdx;
                        cy += cdy;
                    }
                }
            }

            //픽셀 좌표 → 실타일 좌표 (픽셀 중심). placeCities 의 pixelToTileCenter 역연산.
            auto pxToTile = [](int px, int py, int z) noexcept -> Point3
            {
                return Point3{
                    px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2,
                    py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2,
                    z
                };
            };

            RoadPolyLine line;
            line.verts.reserve(simp.size());
            for (std::size_t k = 0; k < simp.size(); ++k)
            {
                line.verts.push_back(pxToTile(simp[k].x, simp[k].y, simp[k].z));
            }

            {
                std::lock_guard<std::mutex> lk(resultsMtx);
                results.push_back(line);
            }
            {
                std::lock_guard<std::mutex> lk(succMtx);
                succPairs.push_back({ e.a, e.b });
            }
            if (onRoad) onRoad(line);

            successCount.fetch_add(1, std::memory_order_relaxed);
        };

        for (int i = 0; i < static_cast<int>(edges.size()); ++i)
        {
            const EdgeCand e = edges[i];
            const int eIdx = i;
            pool.addTask([&edgeTask, e, eIdx]() { edgeTask(e, eIdx); });
        }
        pool.waitForThreads();

        //══════════════════════════════════════════════════════════════════
        // 7) 연결성 봉합 — DSU 안전망
        //══════════════════════════════════════════════════════════════════
        //  MST 그래프는 항상 연결이지만 개별 엣지 A* fail (sea 횡단, cost cap, 좁은 corridor 등)
        //  시 실제 도로망 단절. 성공한 polyline 들로 DSU 빌드 → isolated component 별로 main
        //  component 와의 차순 짧은 city-pair 후보 K=2 개 → A* 재시도. attempted set 으로 중복
        //  fail pair skip. 최대 3 pass (대부분 1 pass 에서 종료).
        const __int64 tBridgeStart = getNanoTimer();
        std::size_t bridgesAttempted = 0;
        std::size_t bridgesOK        = 0;
        const int N = static_cast<int>(cps.size());
        for (int pass = 0; pass < 3; ++pass)
        {
            //DSU 빌드 — 현재까지 성공한 polyline 들로 도시 연결성 추적.
            DSU dsu(N);
            {
                std::lock_guard<std::mutex> lk(succMtx);
                for (const auto& p : succPairs) dsu.unite(p.first, p.second);
            }
            std::unordered_map<int, std::vector<int>> compMap;
            for (int i = 0; i < N; ++i) compMap[dsu.find(i)].push_back(i);
            if (compMap.size() <= 1) break;

            std::vector<std::vector<int>> comps;
            comps.reserve(compMap.size());
            for (auto& [_, m] : compMap) comps.push_back(std::move(m));
            std::sort(comps.begin(), comps.end(),
                [](const auto& a, const auto& b) { return a.size() > b.size(); });

            //isolated component 별로 main 과의 city-pair 차순 짧은 K=2 후보 추출.
            constexpr int kBridgesPerComp = 2;
            std::vector<EdgeCand> bridges;
            bridges.reserve((comps.size() - 1) * kBridgesPerComp);
            const auto& main = comps[0];
            for (std::size_t k = 1; k < comps.size(); ++k)
            {
                const auto& sub = comps[k];
                struct Cand { int s; int m; double d; };
                std::vector<Cand> all;
                all.reserve(sub.size() * main.size());
                for (int s : sub)
                    for (int m : main)
                    {
                        const double dx = static_cast<double>(cps[s].px - cps[m].px);
                        const double dy = static_cast<double>(cps[s].py - cps[m].py);
                        all.push_back(Cand{ s, m, std::sqrt(dx * dx + dy * dy) });
                    }
                std::sort(all.begin(), all.end(),
                    [](const Cand& a, const Cand& b) { return a.d < b.d; });
                int taken = 0;
                std::lock_guard<std::mutex> lk(attemptedMtx);
                for (const auto& c : all)
                {
                    if (attempted.contains(encodePair(c.s, c.m))) continue;
                    bridges.push_back(EdgeCand{ std::min(c.s, c.m), std::max(c.s, c.m), c.d });
                    if (++taken >= kBridgesPerComp) break;
                }
            }
            if (bridges.empty()) break;

            std::sort(bridges.begin(), bridges.end(),
                [](const EdgeCand& a, const EdgeCand& b) { return a.dist > b.dist; });
            const int prevOk = successCount.load(std::memory_order_relaxed);
            for (int i = 0; i < static_cast<int>(bridges.size()); ++i)
            {
                const EdgeCand e = bridges[i];
                const int eIdx = static_cast<int>(edges.size()) + i;
                pool.addTask([&edgeTask, e, eIdx]() { edgeTask(e, eIdx); });
            }
            pool.waitForThreads();
            const int passOk = successCount.load(std::memory_order_relaxed) - prevOk;
            bridgesAttempted += bridges.size();
            bridgesOK        += passOk;
            prt(L"  bridge pass %d     : %zu attempts, %d ok (%zu components remaining)\n",
                pass + 1, bridges.size(), passOk, compMap.size());
        }
        const __int64 tBridgeEnd = getNanoTimer();
        prt(L"  bridges total      : %8.2f ms  (attempted=%zu, ok=%zu)\n",
            (tBridgeEnd - tBridgeStart) / 1.0e6, bridgesAttempted, bridgesOK);

        watcherStop.store(true, std::memory_order_release);
        watcher.request_stop();
        if (watcher.joinable()) watcher.join();

        const __int64 tDone = getNanoTimer();

        //--- 리포트 ---
        const double mstMs    = (tMstEnd     - tMstStart   ) / 1.0e6;
        const double coarseMs = (tCoarseEnd  - tCoarseStart) / 1.0e6;
        const double pathMs   = (tDone       - tPathStart  ) / 1.0e6;
        const double totalMs  = (tDone       - tStart      ) / 1.0e6;

        const int    okN = successCount.load();
        const int    fnN = failCount   .load();
        const std::uint64_t fExp = fineExpTotal  .load();
        const std::uint64_t cExp = coarseExpTotal.load();
        const std::uint64_t vts  = vertsTotal    .load();
        const long long maxNs = maxEdgeNs.load();
        const int       maxIx = maxEdgeIdx.load();

        prt(L"  MST                : %8.2f ms\n", mstMs);
        prt(L"  coarse build       : %8.2f ms\n", coarseMs);
        prt(L"  parallel A* (%2zut) : %8.2f ms  (ok=%d fail=%d)\n",
            numThreads, pathMs, okN, fnN);
        if (okN > 0)
        {
            prt(L"    avg fine exp/road  : %llu\n",  static_cast<unsigned long long>(fExp / okN));
            prt(L"    avg coarse exp/road: %llu\n",  static_cast<unsigned long long>(cExp / okN));
            prt(L"    avg verts/road     : %.1f\n",  static_cast<double>(vts) / okN);
        }
        if (maxIx >= 0 && maxIx < static_cast<int>(edges.size()))
        {
            const EdgeCand& me = edges[maxIx];
            prt(L"    slowest edge       : %.2fs  (dist=%.0fpx)\n",
                maxNs / 1.0e9, me.dist);
        }
        prt(L"    max fine expanded  : %llu cells\n",
            static_cast<unsigned long long>(maxFineExp.load()));
        prt(L"    max heap size      : %llu\n",
            static_cast<unsigned long long>(maxHeapSeen.load()));
        prt(L"  total              : %8.2f ms  (%.2f s)\n", totalMs, totalMs / 1000.0);
        prt(L"  total roads        : %zu\n", results.size());

        return results;
    }
}
