module procGen;

import std;
import util;

//============================================================
// 도로망 폴리라인 생성 — 게임 시작 1회 절차생성의 3단계.
//   입력: seed + PixelCostGrid + 도시 좌표 약 3000개
//   출력: 실타일 좌표 폴리라인 약 7000~10000개
//
//   파이프라인:
//     1) tile→pixel 역변환 + spatial hash
//     2) 후보 엣지 생성 — 3-layer Gabriel 그래프 (전체 / T1∪T2 / T1)
//     3) 중복 제거 + Union-Find로 컴포넌트 봉합
//     4) Coarse 그리드 1회 빌드 (8× 다운샘플, top-4 mean)
//     5) ThreadPool 병렬 hierarchical bidirectional A*:
//          - 짧은 엣지(<400px): bidirectional A* 직접
//          - 긴 엣지: coarse → corridor mask → bidirectional A* with corridor pruning
//          - corridor 실패 시 짧은 엣지는 무코리도 retry, 긴 엣지는 fail 처리
//     6) RDP 단순화 + 실타일 변환 + ±20타일 결정론적 jitter
//
//   순수 블랙박스 함수 — 외부 상태 무관.
//   픽셀좌표(1px=48타일)는 알고리즘 내부 전용, 반환값은 실타일 좌표.
//
//   핵심 설계 결정:
//   - 위상은 Gabriel 그래프 — 두 도시 잇는 disc(지름=두 도시 거리)에 다른 도시가 없는
//     엣지만 존재. 사이에 도시가 끼면 자동으로 a→c→b 두 엣지로 분할 → 클립스루/그라징
//     원천 차단. 사막처럼 도시 밀도 낮으면 disc가 자연히 비어 장거리 엣지 생성.
//   - 3-layer 계층: 전체(지역도로) / T1∪T2(광역) / T1(트렁크). 상위 layer는 하위 티어를
//     blocker로 무시 → 트렁크가 작은 도시 건너뜀 (인터스테이트가 마을 우회하듯).
//   - Hierarchical corridor: coarse A*로 corridor 추출, fine A*는 corridor 셀만 expand
//   - Bidirectional: forward(start→) + backward(end→) 동시 expand → ~2× 가속
//   - Coarse 집계는 top-4 mean. 순수 min은 sea + CityCenter false positive로 바다 위
//     "징검다리" corridor를 만들어 fine A*가 못 따라감.
//============================================================
namespace procGen
{
    namespace
    {
        //좌표 변환 — placeCities와 동일 베이스.
        constexpr int PATCH_X_MIN_LOCAL     = -54;
        constexpr int PATCH_Y_MIN_LOCAL     = -27;
        constexpr int PIXEL_PER_PATCH_LOCAL = 400;
        constexpr int TILE_BASE_X = PATCH_X_MIN_LOCAL * PIXEL_PER_PATCH_LOCAL * TILES_PER_PIXEL;
        constexpr int TILE_BASE_Y = PATCH_Y_MIN_LOCAL * PIXEL_PER_PATCH_LOCAL * TILES_PER_PIXEL;

        //비용 LUT — Terrain enum 값을 인덱스로 사용.
        constexpr float kMinCost = 0.5f;
        constexpr float kSqrt2   = 1.41421356f;

        //경로비용 / 직선거리(px) 상한 — A* 성공해도 이 비율 넘으면 폐기.
        //  land(1.0)만: ratio ≈ 1, detour 50% 늘어도 ≈ 1.5
        //  mountain(8.0) 30% 섞여도 ≈ 3.1
        //  sea(5000) 2~3픽셀 짧은 페리가 500px 경로에 끼면 ratio ≈ 20~30 (허용)
        //  sea 10픽셀 이상은 ratio가 100+ 로 폭증 → 호주↔인도네시아 같은 장거리 횡단 컷.
        constexpr float kMaxCostPerPixel = 30.0f;

        consteval std::array<float, 16> makeCostLUT()
        {
            std::array<float, 16> a{};
            a[static_cast<std::size_t>(Terrain::Land      )] =   1.0f;
            a[static_cast<std::size_t>(Terrain::Sea       )] = 5000.0f;
            a[static_cast<std::size_t>(Terrain::River     )] =  20.0f;   //강(폭 1~2px) — 옛 FreshWater 150에서 대폭 인하. 다리/도로가 자연스럽게 가로지르도록.
            a[static_cast<std::size_t>(Terrain::Lake      )] = 800.0f;   //호수 — 가로지르기 거의 불가. Sea보다 싸 비상시 횡단 가능 정도.
            a[static_cast<std::size_t>(Terrain::CityZone  )] =   0.6f;
            a[static_cast<std::size_t>(Terrain::CityCenter)] =   0.5f;
            a[static_cast<std::size_t>(Terrain::CityRiver )] =   2.0f;   //도시 내 강 — 사후 도시 분할이 다리 위치를 자유 결정하도록 저렴하게.
            a[static_cast<std::size_t>(Terrain::CitySea   )] =   3.0f;   //도시 내 바다(이스탄불·홍콩 해협) — 강보다 약간 비싸지만 도시 횡단 다리 자유 형성.
            a[static_cast<std::size_t>(Terrain::Mountain  )] =   8.0f;
            a[static_cast<std::size_t>(Terrain::Polar     )] =   6.0f;
            a[static_cast<std::size_t>(Terrain::Tundra    )] =   2.0f;
            a[static_cast<std::size_t>(Terrain::Subarctic )] =   0.7f;   //캐나다식 장거리 직선도로 — Land보다 싸게.
            a[static_cast<std::size_t>(Terrain::Monsoon              )] =   1.2f;
            a[static_cast<std::size_t>(Terrain::InsularRainforest    )] =   1.3f;   //동남아 군도 — 도시·도로 연결 가능. 옛 Sabanna 코스트 계승.
            a[static_cast<std::size_t>(Terrain::Desert               )] =   0.6f;   //라스베가스식 장거리 직선도로 — Land보다 싸게.
            a[static_cast<std::size_t>(Terrain::ContinentalRainforest)] =  11.0f;   //우거짐·습지·홍수 — Mountain(8)보다 비싸게. A*가 안데스/연안 우회 선호.
            return a;
        }
        constexpr std::array<float, 16> kCostLUT = makeCostLUT();

        //============================================================
        // Coarse 그리드 — 8× 다운샘플, top-K=4 mean 집계.
        //   5400×2700 = 14.58M float = 58MB. 일회 빌드.
        //   8×8 = 64 fine 셀 중 가장 cheap한 4개의 평균.
        //   순수 min 집계는 sea 65% 월드에서 "징검다리" 환각을 만든다:
        //     CityCenter 픽셀 1개 있는 sea 셀 → min=0.5 → coarse A*가 바다 위 routing →
        //     corridor가 sea 셀 가로지름 → fine A*는 sea 비용 500에 막혀 corridor 못 따라감.
        //   top-4 mean은 "최소 4픽셀 폭 통로" 요구 → 단일 마커에 안 속음.
        //============================================================
        struct CoarseGrid
        {
            static constexpr int F = 8;
            static constexpr int W = (PixelCostGrid::W + F - 1) / F;   // 5400
            static constexpr int H = (PixelCostGrid::H + F - 1) / F;   // 2700
            std::unique_ptr<float[]> cost;

            float at(int cx, int cy) const noexcept
            {
                return cost[static_cast<std::size_t>(cy) * W + cx];
            }
        };

        constexpr int kCoarseAggK = 4;

        //============================================================
        // Corridor mask — coarse 셀 단위 비트셋, O(marked) 클리어.
        //   thread_local 워크스페이스에 1.82MB (14.58M bits) 보유.
        //   prepare()는 marked 리스트 순회로 비트만 끔 — corridor 셀 ~수천뿐이라 fast.
        //============================================================
        struct CorridorMask
        {
            std::vector<std::uint64_t> bits;
            std::vector<std::uint32_t> marked;

            void prepare()
            {
                if (bits.empty())
                {
                    const std::size_t total = static_cast<std::size_t>(CoarseGrid::W) * CoarseGrid::H;
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

        //도시 표현 + 공간 해시 (placeCities 패턴 재사용).
        //  Gabriel 위상은 도시 분포(밀도/희소) 자체에서 자동 적응하므로 biome 필요 없음.
        struct CityPixel
        {
            int      px;
            int      py;
            int      z;
            CityTier tier;
        };

        struct SpatialHash
        {
            int cellSize;
            int gridW;
            int gridH;
            std::vector<std::vector<int>> cells;

            SpatialHash(int worldW, int worldH, int cellSize_)
                : cellSize(cellSize_)
                , gridW((worldW + cellSize_ - 1) / cellSize_)
                , gridH((worldH + cellSize_ - 1) / cellSize_)
                , cells(static_cast<std::size_t>(gridW) * gridH)
            {}

            std::size_t cellIdx(int cx, int cy) const noexcept
            {
                return static_cast<std::size_t>(cy) * gridW + cx;
            }

            void insert(int idx, int px, int py)
            {
                const int cx = std::clamp(px / cellSize, 0, gridW - 1);
                const int cy = std::clamp(py / cellSize, 0, gridH - 1);
                cells[cellIdx(cx, cy)].push_back(idx);
            }

            template <class F>
            void forEachInRadius(int px, int py, int searchR, F&& fn) const
            {
                const int searchCells = (searchR + cellSize - 1) / cellSize;
                const int cxC = std::clamp(px / cellSize, 0, gridW - 1);
                const int cyC = std::clamp(py / cellSize, 0, gridH - 1);
                const int x0  = std::max(0,         cxC - searchCells);
                const int x1  = std::min(gridW - 1, cxC + searchCells);
                const int y0  = std::max(0,         cyC - searchCells);
                const int y1  = std::min(gridH - 1, cyC + searchCells);
                for (int cy = y0; cy <= y1; ++cy)
                    for (int cx = x0; cx <= x1; ++cx)
                        for (int i : cells[cellIdx(cx, cy)])
                            fn(i);
            }
        };

        //============================================================
        // 후보 엣지 생성 — 3-layer Gabriel 그래프.
        //   엣지 (a,b) 존재 ⇔ a·b 잇는 지름의 disc 안에 (필터된) 다른 도시 없음.
        //
        //   레이어:
        //     - Gabriel(전체)        : 기본 메쉬 — 모든 티어가 blocker
        //     - Gabriel(T1∪T2)       : 광역도로 — T3는 blocker에서 제외 (광역 도시들이
        //                              사이의 작은 마을에 막히지 않게)
        //     - Gabriel(T1)          : 트렁크 — T2/T3 모두 blocker에서 제외 (대도시간 직결)
        //
        //   사막/Subarctic처럼 도시 밀도 낮은 지역은 disc가 자연히 비어 장거리 엣지 자동 형성.
        //   biome-aware 튜닝 불필요 — 위상이 분포에서 직접 적응.
        //
        //   복잡도: 후보 N=3000, 평균 디스크 안 도시 수 ~10 → ~3000 × 200 후보 × 10 테스트
        //   = O(NM_avg · D) ≈ 6M 연산. SpatialHash 기반이라 캐시 친화적.
        //============================================================
        struct EdgeCand
        {
            int    a;
            int    b;
            double dist;
        };

        using TierMask = std::uint8_t;
        constexpr TierMask kTierAll = 0b111;
        constexpr TierMask kTierTop = 1u << static_cast<int>(CityTier::T1);
        constexpr TierMask kTierMid = (1u << static_cast<int>(CityTier::T1))
                                    | (1u << static_cast<int>(CityTier::T2));

        constexpr bool tierIn(CityTier t, TierMask m) noexcept
        {
            return (m & (1u << static_cast<int>(t))) != 0;
        }

        //레이어별 후보 거리 상한 — 디스크 안 도시 수가 적어질수록 엣지가 길어질 수 있음.
        //  hard cap이 없으면 호주↔동남아처럼 sea만 끼고 직선거리 가까운 페어가
        //  무의미한 장거리 엣지를 만들 수 있어 차단. (A* 단계에서 sea 비용으로도 걸리지만
        //  후보 단계에서 잘라내는 게 빠름.)
        constexpr double kGabrielMaxDistAll = 3000.0;   //전체 — 도시 밀도 높아 짧은 엣지 위주
        constexpr double kGabrielMaxDistMid = 5000.0;   //T1∪T2 — 광역
        constexpr double kGabrielMaxDistTop = 8000.0;   //T1 — 트렁크 (대륙간)

        std::vector<EdgeCand> buildGabrielEdges(const std::vector<CityPixel>& cities,
                                                 const SpatialHash& hash,
                                                 TierMask mask,
                                                 double maxDist)
        {
            std::vector<EdgeCand> out;
            out.reserve(cities.size() * 4);

            const double maxD2 = maxDist * maxDist;
            const int    maxR  = static_cast<int>(maxDist);

            for (int i = 0; i < static_cast<int>(cities.size()); ++i)
            {
                const auto& a = cities[i];
                if (!tierIn(a.tier, mask)) continue;

                hash.forEachInRadius(a.px, a.py, maxR, [&](int j)
                {
                    if (j <= i) return;   //중복 방지: i<j만 처리
                    const auto& b = cities[j];
                    if (!tierIn(b.tier, mask)) return;

                    const double dx = static_cast<double>(a.px - b.px);
                    const double dy = static_cast<double>(a.py - b.py);
                    const double d2 = dx * dx + dy * dy;
                    if (d2 > maxD2 || d2 < 1.0) return;

                    //Gabriel test: AB의 disc(중심=중점, 반지름=|AB|/2) 안에
                    //마스크된 다른 도시가 strictly inside 인가?
                    const double mx = (a.px + b.px) * 0.5;
                    const double my = (a.py + b.py) * 0.5;
                    const double r2 = d2 * 0.25;
                    const int    sR = static_cast<int>(std::sqrt(r2)) + 1;

                    bool blocked = false;
                    hash.forEachInRadius(static_cast<int>(mx), static_cast<int>(my), sR, [&](int k)
                    {
                        if (blocked) return;
                        if (k == i || k == j) return;
                        const auto& c = cities[k];
                        if (!tierIn(c.tier, mask)) return;
                        const double cdx = static_cast<double>(c.px) - mx;
                        const double cdy = static_cast<double>(c.py) - my;
                        const double cd2 = cdx * cdx + cdy * cdy;
                        if (cd2 < r2) blocked = true;
                    });

                    if (!blocked)
                        out.push_back(EdgeCand{ i, j, std::sqrt(d2) });
                });
            }
            return out;
        }

        std::vector<EdgeCand> dedupEdges(std::vector<EdgeCand> in)
        {
            struct Key { int a, b; bool operator==(const Key& o) const noexcept { return a == o.a && b == o.b; } };
            struct KeyHash {
                std::size_t operator()(const Key& k) const noexcept {
                    return (static_cast<std::size_t>(static_cast<std::uint32_t>(k.a)) << 32)
                         ^  static_cast<std::size_t>(static_cast<std::uint32_t>(k.b));
                }
            };
            std::unordered_set<Key, KeyHash> seen;
            seen.reserve(in.size() * 2);
            std::vector<EdgeCand> out;
            out.reserve(in.size());
            for (const auto& e : in)
                if (seen.insert({ e.a, e.b }).second)
                    out.push_back(e);
            return out;
        }

        //연결성 봉합 — 컴포넌트 분리 시 가장 가까운 페어로 메인 컴포넌트에 묶음.
        struct DSU
        {
            std::vector<int> p, r;
            DSU(int n) : p(n), r(n, 0) { std::iota(p.begin(), p.end(), 0); }
            int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
            bool unite(int a, int b)
            {
                a = find(a); b = find(b);
                if (a == b) return false;
                if (r[a] < r[b]) std::swap(a, b);
                p[b] = a;
                if (r[a] == r[b]) ++r[a];
                return true;
            }
        };

        //============================================================
        // A* 공통 인프라
        //============================================================
        struct Step { int dx, dy; bool diag; };
        constexpr std::array<Step, 8> kSteps = {{
            { 1,  0, false }, { -1,  0, false }, { 0,  1, false }, { 0, -1, false },
            { 1,  1, true  }, { 1, -1, true   }, { -1, 1, true   }, { -1, -1, true  }
        }};

        constexpr float kHeuristicWeight       = 3.0f;   //fine — 거의 greedy
        constexpr float kCoarseHeuristicWeight = 1.5f;   //coarse — 더 tight (작아서 부담 적음)

        struct HeapNode
        {
            float       f;
            float       gAtPush;
            std::size_t idx;
            bool operator>(const HeapNode& o) const noexcept { return f > o.f; }
        };

        //Bidirectional 워크스페이스 — forward + backward 양쪽 상태 보유.
        //  coarse A*는 unidirectional이라 F쪽만 사용(gB/dirB/genB 무시).
        //  prepare()는 양쪽 generation 동시 advance.
        struct AStarWorkspace
        {
            //Forward direction (start → end)
            std::vector<float>         gF;
            std::vector<std::uint8_t>  dirF;
            std::vector<std::uint16_t> genF;
            std::vector<HeapNode>      heapF;
            std::uint16_t              curGenF = 0;

            //Backward direction (end → start) — bidirectional A*에서만 사용
            std::vector<float>         gB;
            std::vector<std::uint8_t>  dirB;
            std::vector<std::uint16_t> genB;
            std::vector<HeapNode>      heapB;
            std::uint16_t              curGenB = 0;

            //Hierarchical corridor mask
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

        struct AStarOut
        {
            std::vector<PixelCoord> path;                   //비어있으면 실패
            float                   totalCost    = 0.0f;    //경로 누적 비용(mu)
            std::uint64_t           expanded     = 0;
            std::size_t             maxHeapSize  = 0;
        };

        struct CoarseAStarOut
        {
            bool                       found = false;
            std::vector<std::uint32_t> cells;               //글로벌 coarse 셀 인덱스
            std::uint64_t              expanded = 0;
        };

        //============================================================
        // Coarse A* — CoarseGrid 위 단방향 weighted A*.
        //   반환: 경로 위 coarse 셀 인덱스 리스트(corridor mask 빌드용).
        //   workspace는 fine과 공유(gF/genF/heapF만 사용).
        //============================================================
        CoarseAStarOut astarCoarse(int sx, int sy, int ex, int ey,
                                    const CoarseGrid& coarse,
                                    int margin,
                                    AStarWorkspace& ws)
        {
            CoarseAStarOut R{};
            constexpr int W = CoarseGrid::W;
            constexpr int H = CoarseGrid::H;

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

            const std::ptrdiff_t boxW_p = boxW;
            const std::ptrdiff_t W_p    = W;
            const std::ptrdiff_t boxStep[8] = {
                 1, -1,  boxW_p, -boxW_p,
                 boxW_p + 1, -boxW_p + 1,  boxW_p - 1, -boxW_p - 1
            };
            const std::ptrdiff_t gridStep[8] = {
                 1, -1,  W_p, -W_p,
                 W_p + 1, -W_p + 1,  W_p - 1, -W_p - 1
            };
            constexpr float stepLenLUT[8] = {
                1.0f, 1.0f, 1.0f, 1.0f,
                kSqrt2, kSqrt2, kSqrt2, kSqrt2
            };

            auto idxOf = [&](int x, int y) -> std::size_t
            {
                return static_cast<std::size_t>(y - y0w) * boxW + (x - x0w);
            };

            auto octile = [&](int x, int y) -> float
            {
                const int dx = std::abs(x - ex);
                const int dy = std::abs(y - ey);
                const int dmin = std::min(dx, dy);
                const int dmax = std::max(dx, dy);
                return ((dmax - dmin) + dmin * kSqrt2) * kMinCost * kCoarseHeuristicWeight;
            };

            const std::size_t sIdx = idxOf(sx, sy);
            const std::size_t gIdx = idxOf(ex, ey);
            g_[sIdx]   = 0.0f;
            gen_[sIdx] = curGen;

            ws.heapF.push_back(HeapNode{ octile(sx, sy), 0.0f, sIdx });
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

                for (int d = 0; d < 8; ++d)
                {
                    const auto& s = kSteps[d];
                    const int nx = curX + s.dx;
                    const int ny = curY + s.dy;
                    if (nx < x0w || nx > x1w || ny < y0w || ny > y1w) continue;

                    const std::size_t nIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(top.idx) + boxStep[d]);
                    if (gen_[nIdx] == closedGen) continue;

                    const std::size_t gridNIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(gridCur) + gridStep[d]);
                    const float terrain = costData[gridNIdx];
                    const float gNew    = gCur + stepLenLUT[d] * terrain;

                    const float gPrev = (gen_[nIdx] == curGen) ? g_[nIdx]
                                                                : std::numeric_limits<float>::infinity();
                    if (gNew < gPrev)
                    {
                        g_[nIdx]   = gNew;
                        dir_[nIdx] = static_cast<std::uint8_t>(d);
                        gen_[nIdx] = curGen;
                        const float fNew = gNew + octile(nx, ny);
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
                rev.push_back(static_cast<std::uint32_t>(cy) * CoarseGrid::W + cx);
                const std::size_t cur = idxOf(cx, cy);
                if (cur == sIdx) break;
                const std::uint8_t d = dir_[cur];
                if (d >= 8) { rev.clear(); return R; }
                cx -= kSteps[d].dx;
                cy -= kSteps[d].dy;
            }
            std::reverse(rev.begin(), rev.end());
            R.cells = std::move(rev);
            R.found = true;
            return R;
        }

        //============================================================
        // Bidirectional A* (fine grid) with optional corridor mask.
        //   - Forward: start → end
        //   - Backward: end → start
        //   - 양쪽 동시 expand, 작은 top.f 쪽 먼저 → ~2× 가속
        //   - Termination: 양쪽 모두 mu(지금까지 발견된 최단 meeting 비용) 이상으로
        //                  expand하면 더 이상 개선 불가 → break
        //   - 경로 복원: meeting → start(dirF), meeting → end(dirB)
        //
        //   x0w/y0w/x1w/y1w: 명시적 박스 경계(호출자가 corridor bbox로 결정).
        //============================================================
        AStarOut astarPathBidir(PixelCoord start, PixelCoord end,
                                 const PixelCostGrid& grid,
                                 int x0w, int y0w, int x1w, int y1w,
                                 AStarWorkspace& ws,
                                 const CorridorMask* mask)
        {
            AStarOut R{};
            const int W = PixelCostGrid::W;

            const int boxW = x1w - x0w + 1;
            const int boxH = y1w - y0w + 1;
            const std::size_t total = static_cast<std::size_t>(boxW) * boxH;

            //박스 cap — corridor 활성 시 박스가 커도 실제 expand는 corridor 한정.
            //  non-corridor에선 lazy duplicate 폭증 막으려고 빠듯하게.
            //  cap × 2 directions × 7 bytes = workspace per thread.
            const std::size_t kMaxBoxCells = mask
                ? (4ull * 1024 * 1024)
                : (2ull * 1024 * 1024);
            if (total == 0 || total > kMaxBoxCells) return R;

            //start, end 모두 박스 안에 있어야 함.
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

            const std::ptrdiff_t boxW_p = boxW;
            const std::ptrdiff_t W_p    = W;
            const std::ptrdiff_t boxStep[8] = {
                 1, -1,  boxW_p, -boxW_p,
                 boxW_p + 1, -boxW_p + 1,  boxW_p - 1, -boxW_p - 1
            };
            const std::ptrdiff_t gridStep[8] = {
                 1, -1,  W_p, -W_p,
                 W_p + 1, -W_p + 1,  W_p - 1, -W_p - 1
            };
            constexpr float stepLenLUT[8] = {
                1.0f, 1.0f, 1.0f, 1.0f,
                kSqrt2, kSqrt2, kSqrt2, kSqrt2
            };

            auto idxOf = [&](int x, int y) -> std::size_t
            {
                return static_cast<std::size_t>(y - y0w) * boxW + (x - x0w);
            };

            //양쪽 휴리스틱
            const int sxFin = start.x, syFin = start.y;
            const int exFin = end.x,   eyFin = end.y;
            auto octileTo = [&](int x, int y, int tx, int ty) -> float
            {
                const int dx = std::abs(x - tx);
                const int dy = std::abs(y - ty);
                const int dmin = std::min(dx, dy);
                const int dmax = std::max(dx, dy);
                return ((dmax - dmin) + dmin * kSqrt2) * kMinCost * kHeuristicWeight;
            };

            const std::size_t sIdx = idxOf(start.x, start.y);
            const std::size_t eIdx = idxOf(end.x,   end.y);
            if (sIdx == eIdx)
            {
                R.path.push_back(PixelCoord{ start.x, start.y, start.z });
                R.totalCost = 0.0f;
                return R;
            }

            //Forward init
            gF_[sIdx]   = 0.0f;
            genF_[sIdx] = curGenF;
            ws.heapF.push_back(HeapNode{ octileTo(sxFin, syFin, exFin, eyFin), 0.0f, sIdx });
            std::push_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});

            //Backward init
            gB_[eIdx]   = 0.0f;
            genB_[eIdx] = curGenB;
            ws.heapB.push_back(HeapNode{ octileTo(exFin, eyFin, sxFin, syFin), 0.0f, eIdx });
            std::push_heap(ws.heapB.begin(), ws.heapB.end(), std::greater<HeapNode>{});

            float        mu          = std::numeric_limits<float>::infinity();
            std::size_t  meetingIdx  = static_cast<std::size_t>(-1);

            //한 쪽 방향 expand 1회 — 람다로 분리하면 헷갈리므로 분기로 처리.
            while (!ws.heapF.empty() && !ws.heapB.empty())
            {
                const float topFf = ws.heapF.front().f;
                const float topBf = ws.heapB.front().f;

                //양쪽 top이 모두 mu 이상이면 더 이상 개선 불가.
                //(W>1 weighted라 엄밀하진 않지만 실용적으로 충분.)
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

                    //Meeting check on close
                    if (genB_[top.idx] == curGenB || genB_[top.idx] == closedGenB)
                    {
                        const float cand = top.gAtPush + gB_[top.idx];
                        if (cand < mu) { mu = cand; meetingIdx = top.idx; }
                    }

                    const int cy = static_cast<int>(top.idx / boxW);
                    const int cx = static_cast<int>(top.idx) - cy * boxW;
                    const int curX = cx + x0w;
                    const int curY = cy + y0w;
                    const std::size_t gridCur = static_cast<std::size_t>(curY) * W + curX;
                    const float gCur = top.gAtPush;

                    for (int d = 0; d < 8; ++d)
                    {
                        const auto& s = kSteps[d];
                        const int nx = curX + s.dx;
                        const int ny = curY + s.dy;
                        if (nx < x0w || nx > x1w || ny < y0w || ny > y1w) continue;

                        if (mask)
                        {
                            const std::uint32_t coarseIdx =
                                (static_cast<std::uint32_t>(ny) / CoarseGrid::F) * CoarseGrid::W
                                + (static_cast<std::uint32_t>(nx) / CoarseGrid::F);
                            if (!mask->test(coarseIdx)) continue;
                        }

                        const std::size_t nIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(top.idx) + boxStep[d]);
                        if (genF_[nIdx] == closedGenF) continue;

                        const std::size_t gridNIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(gridCur) + gridStep[d]);
                        const float terrain = kCostLUT[static_cast<std::size_t>(gridData[gridNIdx])];
                        const float gNew    = gCur + stepLenLUT[d] * terrain;

                        const float gPrev = (genF_[nIdx] == curGenF) ? gF_[nIdx]
                                                                      : std::numeric_limits<float>::infinity();
                        if (gNew < gPrev)
                        {
                            gF_[nIdx]   = gNew;
                            dirF_[nIdx] = static_cast<std::uint8_t>(d);
                            genF_[nIdx] = curGenF;

                            //Meeting check on relax
                            if (genB_[nIdx] == curGenB || genB_[nIdx] == closedGenB)
                            {
                                const float cand = gNew + gB_[nIdx];
                                if (cand < mu) { mu = cand; meetingIdx = nIdx; }
                            }

                            const float fNew = gNew + octileTo(nx, ny, exFin, eyFin);
                            ws.heapF.push_back(HeapNode{ fNew, gNew, nIdx });
                            std::push_heap(ws.heapF.begin(), ws.heapF.end(), std::greater<HeapNode>{});
                            const std::size_t hs = ws.heapF.size();
                            if (hs > R.maxHeapSize) R.maxHeapSize = hs;
                        }
                    }
                }
                else
                {
                    //Backward expansion (mirror)
                    std::pop_heap(ws.heapB.begin(), ws.heapB.end(), std::greater<HeapNode>{});
                    const HeapNode top = ws.heapB.back();
                    ws.heapB.pop_back();

                    if (genB_[top.idx] == closedGenB) continue;
                    if (genB_[top.idx] != curGenB || gB_[top.idx] != top.gAtPush) continue;

                    genB_[top.idx] = closedGenB;
                    ++R.expanded;

                    if (genF_[top.idx] == curGenF || genF_[top.idx] == closedGenF)
                    {
                        const float cand = top.gAtPush + gF_[top.idx];
                        if (cand < mu) { mu = cand; meetingIdx = top.idx; }
                    }

                    const int cy = static_cast<int>(top.idx / boxW);
                    const int cx = static_cast<int>(top.idx) - cy * boxW;
                    const int curX = cx + x0w;
                    const int curY = cy + y0w;
                    const std::size_t gridCur = static_cast<std::size_t>(curY) * W + curX;
                    const float gCur = top.gAtPush;

                    for (int d = 0; d < 8; ++d)
                    {
                        const auto& s = kSteps[d];
                        const int nx = curX + s.dx;
                        const int ny = curY + s.dy;
                        if (nx < x0w || nx > x1w || ny < y0w || ny > y1w) continue;

                        if (mask)
                        {
                            const std::uint32_t coarseIdx =
                                (static_cast<std::uint32_t>(ny) / CoarseGrid::F) * CoarseGrid::W
                                + (static_cast<std::uint32_t>(nx) / CoarseGrid::F);
                            if (!mask->test(coarseIdx)) continue;
                        }

                        const std::size_t nIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(top.idx) + boxStep[d]);
                        if (genB_[nIdx] == closedGenB) continue;

                        const std::size_t gridNIdx = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(gridCur) + gridStep[d]);
                        const float terrain = kCostLUT[static_cast<std::size_t>(gridData[gridNIdx])];
                        const float gNew    = gCur + stepLenLUT[d] * terrain;

                        const float gPrev = (genB_[nIdx] == curGenB) ? gB_[nIdx]
                                                                      : std::numeric_limits<float>::infinity();
                        if (gNew < gPrev)
                        {
                            gB_[nIdx]   = gNew;
                            dirB_[nIdx] = static_cast<std::uint8_t>(d);
                            genB_[nIdx] = curGenB;

                            if (genF_[nIdx] == curGenF || genF_[nIdx] == closedGenF)
                            {
                                const float cand = gNew + gF_[nIdx];
                                if (cand < mu) { mu = cand; meetingIdx = nIdx; }
                            }

                            const float fNew = gNew + octileTo(nx, ny, sxFin, syFin);
                            ws.heapB.push_back(HeapNode{ fNew, gNew, nIdx });
                            std::push_heap(ws.heapB.begin(), ws.heapB.end(), std::greater<HeapNode>{});
                            const std::size_t hs = ws.heapB.size();
                            if (hs > R.maxHeapSize) R.maxHeapSize = hs;
                        }
                    }
                }
            }

            if (meetingIdx == static_cast<std::size_t>(-1)) return R;

            //Reconstruction:
            //  forward part:  meeting → start via dirF, reverse → [start, ..., meeting]
            //  backward part: meeting → end   via dirB, append (meeting 자체는 skip)
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
                    if (d >= 8) { return R; }
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
                    if (d >= 8) { return R; }
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
        }

        //박스 마진으로부터 명시적 박스 계산(짧은 엣지용).
        struct FineBox { int x0, y0, x1, y1; };
        FineBox boxFromMargin(PixelCoord A, PixelCoord B, int marginPx) noexcept
        {
            return FineBox{
                std::max(0,                    std::min(A.x, B.x) - marginPx),
                std::max(0,                    std::min(A.y, B.y) - marginPx),
                std::min(PixelCostGrid::W - 1, std::max(A.x, B.x) + marginPx),
                std::min(PixelCostGrid::H - 1, std::max(A.y, B.y) + marginPx)
            };
        }
        //--- RDP 단순화 ---
        //  segment AB 위 perpendicular 거리² 최대인 점을 찾아 eps² 초과면 분할 재귀.
        //  segment 길이 0이면 endpoint 거리. dx/dy/len2는 (lo,hi) 페어당 1회 hoisting.
        void rdpRecurse(const std::vector<PixelCoord>& in, int lo, int hi,
                        double eps2, std::vector<int>& keep)
        {
            if (hi <= lo + 1) return;
            const auto& A = in[lo];
            const auto& B = in[hi];
            const double sdx  = static_cast<double>(B.x - A.x);
            const double sdy  = static_cast<double>(B.y - A.y);
            const double slen2 = sdx * sdx + sdy * sdy;

            int    maxIdx = -1;
            double maxD2  = 0.0;
            for (int i = lo + 1; i < hi; ++i)
            {
                const double px = in[i].x, py = in[i].y;
                double d2;
                if (slen2 == 0.0)
                {
                    const double ex = px - A.x, ey = py - A.y;
                    d2 = ex * ex + ey * ey;
                }
                else
                {
                    const double t  = ((px - A.x) * sdx + (py - A.y) * sdy) / slen2;
                    const double tc = std::clamp(t, 0.0, 1.0);
                    const double cx = A.x + tc * sdx;
                    const double cy = A.y + tc * sdy;
                    const double ex = px - cx, ey = py - cy;
                    d2 = ex * ex + ey * ey;
                }
                if (d2 > maxD2) { maxD2 = d2; maxIdx = i; }
            }
            if (maxD2 > eps2 && maxIdx > 0)
            {
                rdpRecurse(in, lo, maxIdx, eps2, keep);
                keep.push_back(maxIdx);
                rdpRecurse(in, maxIdx, hi, eps2, keep);
            }
        }

        //============================================================
        // 한 엣지 처리 — Hierarchical bidirectional A*.
        //   짧은 엣지(<400px): bidirectional A* 직접, 마진 retry.
        //   긴 엣지: coarse → corridor → bidirectional fine A* with corridor.
        //           corridor 실패 시 폴백 없이 fail (tail edge 시간 폭증 방지).
        //============================================================
        //너무 긴 엣지는 시도 자체를 skip — 박스 cap 초과로 어차피 fail.
        //  사막/Subarctic biome-aware KNN이 6000~7000px 엣지도 만들어내므로 cap을
        //  넉넉하게 잡아야 함. kMaxCostPerPixel(=30) 비용비율 cap이 살아있어 호주↔인도네시아
        //  같은 sea 횡단은 후필터로 걸러짐.
        constexpr double kMaxEdgeAttempt = 5000.0;

        std::vector<PixelCoord> pathOneEdge(PixelCoord A, PixelCoord B,
                                             const PixelCostGrid& grid,
                                             const CoarseGrid& coarse,
                                             double directDistPx,
                                             AStarWorkspace& ws,
                                             std::uint64_t& fineExpanded,
                                             std::uint64_t& coarseExpanded,
                                             std::size_t&   maxHeap)
        {
            if (directDistPx > kMaxEdgeAttempt) return {};

            //비용비율 체크 — A* 성공해도 sea 가로지름이면 폐기.
            //(A*는 비용 무관하게 무조건 경로를 찾으므로 mu에 누적된 값으로 후필터.)
            const float costCap = kMaxCostPerPixel * static_cast<float>(directDistPx);

            //짧은 엣지: corridor overhead 회피, bidirectional fine A* 직접.
            if (directDistPx < 400.0)
            {
                const int margin1 = std::max(60, static_cast<int>(directDistPx * 0.10));
                const FineBox b1 = boxFromMargin(A, B, margin1);
                AStarOut r1 = astarPathBidir(A, B, grid, b1.x0, b1.y0, b1.x1, b1.y1, ws, nullptr);
                fineExpanded += r1.expanded;
                if (r1.maxHeapSize > maxHeap) maxHeap = r1.maxHeapSize;
                if (!r1.path.empty())
                {
                    if (r1.totalCost > costCap) return {};
                    return r1.path;
                }

                const int margin2 = std::max(180, static_cast<int>(directDistPx * 0.30));
                const FineBox b2 = boxFromMargin(A, B, margin2);
                AStarOut r2 = astarPathBidir(A, B, grid, b2.x0, b2.y0, b2.x1, b2.y1, ws, nullptr);
                fineExpanded += r2.expanded;
                if (r2.maxHeapSize > maxHeap) maxHeap = r2.maxHeapSize;
                if (r2.path.empty() || r2.totalCost > costCap) return {};
                return std::move(r2.path);
            }

            //긴 엣지: hierarchical.
            const int sxC = A.x / CoarseGrid::F;
            const int syC = A.y / CoarseGrid::F;
            const int exC = B.x / CoarseGrid::F;
            const int eyC = B.y / CoarseGrid::F;
            const int coarseDist = std::max(std::abs(sxC - exC), std::abs(syC - eyC));
            const int coarseMargin = std::max(40, coarseDist / 2);

            CoarseAStarOut cr = astarCoarse(sxC, syC, exC, eyC, coarse, coarseMargin, ws);
            coarseExpanded += cr.expanded;

            if (!cr.found || cr.cells.empty()) return {};

            //Corridor 마킹 + bbox 동시 산출 — coarse 경로 + halo coarse cells 만큼 mask에 set.
            //  halo=3 → 7×7 stamp = 56 fine pixel 폭 corridor (≈2800타일 폭).
            //  bbox와 mask 둘 다 cr.cells 1회 순회로 같이 산출 (이전엔 별도 함수 2번 순회).
            constexpr int kHalo = 3;
            ws.mask.prepare();
            int cMinX = std::numeric_limits<int>::max();
            int cMinY = std::numeric_limits<int>::max();
            int cMaxX = std::numeric_limits<int>::min();
            int cMaxY = std::numeric_limits<int>::min();
            for (std::uint32_t idx : cr.cells)
            {
                const int cy = static_cast<int>(idx / CoarseGrid::W);
                const int cx = static_cast<int>(idx % CoarseGrid::W);
                if (cx < cMinX) cMinX = cx;
                if (cx > cMaxX) cMaxX = cx;
                if (cy < cMinY) cMinY = cy;
                if (cy > cMaxY) cMaxY = cy;
                const int x0 = std::max(0, cx - kHalo);
                const int x1 = std::min(CoarseGrid::W - 1, cx + kHalo);
                const int y0 = std::max(0, cy - kHalo);
                const int y1 = std::min(CoarseGrid::H - 1, cy + kHalo);
                for (int yy = y0; yy <= y1; ++yy)
                {
                    const std::uint32_t base = static_cast<std::uint32_t>(yy) * CoarseGrid::W;
                    for (int xx = x0; xx <= x1; ++xx)
                    {
                        ws.mask.mark(base + static_cast<std::uint32_t>(xx));
                    }
                }
            }

            //Fine 박스 = corridor bbox(halo 포함) → fine 픽셀 확장 + 시작/끝점 강제 포함.
            //  corridor 외부 셀은 mask로 다 prune되니 박스 메모리 낭비 없음.
            const int cbMinX = std::max(0, cMinX - kHalo);
            const int cbMinY = std::max(0, cMinY - kHalo);
            const int cbMaxX = std::min(CoarseGrid::W - 1, cMaxX + kHalo);
            const int cbMaxY = std::min(CoarseGrid::H - 1, cMaxY + kHalo);
            const int fbX0 = std::max(0,                    std::min({ cbMinX * CoarseGrid::F,         A.x, B.x }));
            const int fbY0 = std::max(0,                    std::min({ cbMinY * CoarseGrid::F,         A.y, B.y }));
            const int fbX1 = std::min(PixelCostGrid::W - 1, std::max({ (cbMaxX + 1) * CoarseGrid::F - 1, A.x, B.x }));
            const int fbY1 = std::min(PixelCostGrid::H - 1, std::max({ (cbMaxY + 1) * CoarseGrid::F - 1, A.y, B.y }));

            AStarOut r = astarPathBidir(A, B, grid, fbX0, fbY0, fbX1, fbY1, ws, &ws.mask);
            fineExpanded += r.expanded;
            if (r.maxHeapSize > maxHeap) maxHeap = r.maxHeapSize;
            if (r.path.empty() || r.totalCost > costCap) return {};
            return std::move(r.path);
        }
    } // anonymous namespace

    //도시 좌표들을 바탕으로 도로 폴리라인 네트워크를 생성. 순수 블랙박스 함수.
    //onRoad는 폴리라인 1개 완성될 때마다 호출되는 옵션 콜백. default no-op이면 출력 영향 없음.
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed,
                                                const PixelCostGrid& grid,
                                                const std::vector<CityNode>& cities,
                                                RoadSink onRoad)
    {
        const __int64 tStart = getNanoTimer();
        prt(L"[procGen] buildRoadNetwork start (N=%zu cities, seed=%llu)\n",
            cities.size(), static_cast<unsigned long long>(seed));

        if (cities.size() < 2) return {};

        //--- 1) tile → pixel 역변환 ---
        //  placeCities의 pixelToTileCenter 역연산. TILE_BASE는 동일 베이스(상단 상수).
        std::vector<CityPixel> cps;
        cps.reserve(cities.size());
        int t1n = 0, t2n = 0, t3n = 0;
        for (const auto& cn : cities)
        {
            cps.push_back(CityPixel{
                (cn.center.x - TILE_BASE_X) / TILES_PER_PIXEL,
                (cn.center.y - TILE_BASE_Y) / TILES_PER_PIXEL,
                cn.center.z,
                cn.tier
            });
            switch (cn.tier)
            {
            case CityTier::T1: ++t1n; break;
            case CityTier::T2: ++t2n; break;
            case CityTier::T3: ++t3n; break;
            }
        }
        prt(L"  tier counts: T1=%d T2=%d T3=%d\n", t1n, t2n, t3n);

        //--- 2) 공간 해시 ---
        SpatialHash hash(PixelCostGrid::W, PixelCostGrid::H, 80);
        for (int i = 0; i < static_cast<int>(cps.size()); ++i)
            hash.insert(i, cps[i].px, cps[i].py);

        const __int64 tHash = getNanoTimer();

        //--- 3) 후보 엣지 — 3-layer Gabriel ---
        std::vector<EdgeCand> base = buildGabrielEdges(cps, hash, kTierAll, kGabrielMaxDistAll);
        std::vector<EdgeCand> mid  = buildGabrielEdges(cps, hash, kTierMid, kGabrielMaxDistMid);
        std::vector<EdgeCand> top  = buildGabrielEdges(cps, hash, kTierTop, kGabrielMaxDistTop);
        const std::size_t baseN = base.size();
        const std::size_t midN  = mid.size();
        const std::size_t topN  = top.size();

        base.insert(base.end(), mid.begin(), mid.end());
        base.insert(base.end(), top.begin(), top.end());
        std::vector<EdgeCand> edges = dedupEdges(std::move(base));

        const __int64 tEdges = getNanoTimer();

        //--- 4) 연결성 봉합 ---
        //  Gabriel ⊇ MST 라 단일 컴포넌트가 정상이지만, 도시 클러스터가 sea로
        //  완전히 고립된 경우(예: 외딴 섬 도시) 추가 엣지가 필요할 수 있음.
        //  알고리즘: DSU로 컴포넌트 분할 → main(=가장 큰) 외 각 sub에서 main까지
        //  가장 가까운 페어로 1엣지 추가 → 최종 dedup.
        std::size_t extrasN = 0;
        {
            DSU dsu(static_cast<int>(cps.size()));
            for (const auto& e : edges) dsu.unite(e.a, e.b);

            std::unordered_map<int, std::vector<int>> comps;
            for (int i = 0; i < static_cast<int>(cps.size()); ++i)
                comps[dsu.find(i)].push_back(i);

            if (comps.size() > 1)
            {
                std::vector<std::vector<int>*> sortedComps;
                sortedComps.reserve(comps.size());
                for (auto& [_, mem] : comps) sortedComps.push_back(&mem);
                std::sort(sortedComps.begin(), sortedComps.end(),
                    [](const auto* a, const auto* b) { return a->size() > b->size(); });

                const auto& mainComp = *sortedComps[0];
                std::vector<EdgeCand> extras;
                for (std::size_t k = 1; k < sortedComps.size(); ++k)
                {
                    const auto& sub = *sortedComps[k];
                    double best = std::numeric_limits<double>::infinity();
                    int bestA = -1, bestB = -1;
                    for (int s : sub)
                    {
                        for (int m : mainComp)
                        {
                            const double dx = static_cast<double>(cps[s].px - cps[m].px);
                            const double dy = static_cast<double>(cps[s].py - cps[m].py);
                            const double d2 = dx * dx + dy * dy;
                            if (d2 < best)
                            {
                                best  = d2;
                                bestA = std::min(s, m);
                                bestB = std::max(s, m);
                            }
                        }
                    }
                    if (bestA >= 0)
                        extras.push_back(EdgeCand{ bestA, bestB, std::sqrt(best) });
                }
                extrasN = extras.size();
                if (!extras.empty())
                {
                    edges.insert(edges.end(), extras.begin(), extras.end());
                    edges = dedupEdges(std::move(edges));
                }
            }
        }

        const __int64 tConn = getNanoTimer();

        prt(L"  edges: gabriel(all)=%zu (T1+T2)=%zu (T1)=%zu connectivity=%zu => final=%zu\n",
            baseN, midN, topN, extrasN, edges.size());

        //--- 4-1) 엣지 거리 히스토그램 ---
        {
            constexpr int   BUCKETS = 8;
            const int       bounds[BUCKETS] = { 100, 200, 400, 800, 1200, 1800, 2500,
                                                 std::numeric_limits<int>::max() };
            const wchar_t*  labels[BUCKETS] = {
                L"  <100", L"100-200", L"200-400", L"400-800",
                L"800-1.2k", L"1.2-1.8k", L"1.8-2.5k", L"  >2.5k"
            };
            int hist[BUCKETS] = {};
            for (const auto& e : edges)
            {
                const int d = static_cast<int>(e.dist);
                for (int i = 0; i < BUCKETS; ++i)
                    if (d < bounds[i]) { ++hist[i]; break; }
            }
            prt(L"  edge distance histogram (px):\n");
            for (int i = 0; i < BUCKETS; ++i)
                prt(L"    %-9ls : %5d\n", labels[i], hist[i]);
        }

        //--- 4-2) Coarse 그리드 1회 빌드 ---
        //  8× 다운샘플 (5400×2700 float = 58MB), top-K=4 mean 집계.
        //  병렬: 행을 nT 등분하여 각 스레드가 자기 행 영역만 처리.
        const __int64 tCoarseStart = getNanoTimer();
        CoarseGrid coarse;
        {
            constexpr std::size_t total = static_cast<std::size_t>(CoarseGrid::W) * CoarseGrid::H;
            coarse.cost = std::make_unique<float[]>(total);

            const Terrain* fineData = grid.data.get();
            constexpr int Wf = PixelCostGrid::W;
            constexpr int Hf = PixelCostGrid::H;
            constexpr int F  = CoarseGrid::F;

            const int nT = std::max<int>(2, static_cast<int>(std::thread::hardware_concurrency()));
            std::vector<std::thread> threads;
            threads.reserve(nT);
            const int rowsPerT = (CoarseGrid::H + nT - 1) / nT;

            for (int t = 0; t < nT; ++t)
            {
                const int r0 = t * rowsPerT;
                const int r1 = std::min(r0 + rowsPerT, CoarseGrid::H);
                if (r0 >= r1) break;
                threads.emplace_back([&, r0, r1]()
                {
                    float buf[F * F];
                    for (int cy = r0; cy < r1; ++cy)
                    {
                        const int y0 = cy * F;
                        const int y1 = std::min(y0 + F, Hf);
                        float* dstRow = &coarse.cost[static_cast<std::size_t>(cy) * CoarseGrid::W];
                        for (int cx = 0; cx < CoarseGrid::W; ++cx)
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

        //--- 4-3) 엣지 거리 내림차순 정렬 — tail latency 감소 ---
        std::sort(edges.begin(), edges.end(),
            [](const EdgeCand& a, const EdgeCand& b) { return a.dist > b.dist; });

        //--- 5) 병렬 hierarchical bidirectional A* ---
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

        const __int64 tPathStart = getNanoTimer();

        //--- 5-1) 진행 워치 스레드 ---
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

        for (int eIdx = 0; eIdx < static_cast<int>(edges.size()); ++eIdx)
        {
            const EdgeCand e = edges[eIdx];
            pool.addTask([&, e, eIdx]()
            {
                thread_local AStarWorkspace ws;

                const __int64 tEdgeStart = getNanoTimer();

                const PixelCoord A{ cps[e.a].px, cps[e.a].py, cps[e.a].z };
                const PixelCoord B{ cps[e.b].px, cps[e.b].py, cps[e.b].z };
                if (A == B)
                {
                    failCount.fetch_add(1, std::memory_order_relaxed);
                    return;
                }

                std::uint64_t fExp = 0, cExp = 0;
                std::size_t   hMax = 0;
                std::vector<PixelCoord> raw =
                    pathOneEdge(A, B, grid, coarse, e.dist, ws, fExp, cExp, hMax);

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

                //RDP 단순화 — eps=1.5px (eps²=2.25). perpendicular 거리 이하 정점 제거.
                //  본체는 재귀라 rdpRecurse는 파일 스코프 유지.
                std::vector<PixelCoord> simp;
                if (raw.size() <= 2)
                {
                    simp = std::move(raw);
                }
                else
                {
                    std::vector<int> keep;
                    keep.reserve(raw.size() / 4 + 2);
                    keep.push_back(0);
                    rdpRecurse(raw, 0, static_cast<int>(raw.size()) - 1, 1.5 * 1.5, keep);
                    keep.push_back(static_cast<int>(raw.size()) - 1);
                    simp.reserve(keep.size());
                    for (int i : keep) simp.push_back(raw[i]);
                }
                vertsTotal.fetch_add(simp.size(), std::memory_order_relaxed);

                //픽셀 → 실타일 + 결정론적 jitter.
                //  endpoint(시작/끝) 정점은 도시 좌표 정렬용으로 jitter 없이 픽셀 중심.
                //  중간 정점은 (seed, px, py) splitmix64 해시로 ±20타일 jitter — 동일 seed면
                //  동일 결과(결정론), 픽셀 단위로 무관(파편화).
                auto pxToTile = [seed](int px, int py, int z, bool endpoint) noexcept -> Point3
                {
                    const int baseX = px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2;
                    const int baseY = py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2;
                    if (endpoint) return Point3{ baseX, baseY, z };

                    std::uint64_t h = seed ^ 0xD6E8FEB86659FD93ULL;
                    h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) * 0x9E3779B97F4A7C15ULL;
                    h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
                    h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(py)) * 0x94D049BB133111EBULL;
                    h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
                    h ^= h >> 31;

                    const int jx = static_cast<int>(h % 41u) - 20;        // -20 .. +20
                    const int jy = static_cast<int>((h >> 16) % 41u) - 20;
                    return Point3{ baseX + jx, baseY + jy, z };
                };

                RoadPolyLine line;
                line.verts.reserve(simp.size());
                for (std::size_t k = 0; k < simp.size(); ++k)
                {
                    const bool endpoint = (k == 0) || (k + 1 == simp.size());
                    line.verts.push_back(pxToTile(simp[k].x, simp[k].y, simp[k].z, endpoint));
                }

                {
                    std::lock_guard<std::mutex> lk(resultsMtx);
                    results.push_back(line);
                }
                if (onRoad) onRoad(line);

                successCount.fetch_add(1, std::memory_order_relaxed);
            });
        }

        pool.waitForThreads();

        watcherStop.store(true, std::memory_order_release);
        watcher.request_stop();
        if (watcher.joinable()) watcher.join();

        const __int64 tDone = getNanoTimer();

        //--- 리포트 ---
        const double hashMs   = (tHash       - tStart      ) / 1.0e6;
        const double edgeMs   = (tEdges      - tHash       ) / 1.0e6;
        const double connMs   = (tConn       - tEdges      ) / 1.0e6;
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

        prt(L"  spatial hash       : %8.2f ms\n", hashMs);
        prt(L"  knn + trunk edges  : %8.2f ms\n", edgeMs);
        prt(L"  connectivity       : %8.2f ms\n", connMs);
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
            const auto& me = edges[maxIx];
            prt(L"    slowest edge       : %.2fs  (dist=%.0fpx, tier=%d-%d)\n",
                maxNs / 1.0e9, me.dist,
                static_cast<int>(cps[me.a].tier), static_cast<int>(cps[me.b].tier));
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
