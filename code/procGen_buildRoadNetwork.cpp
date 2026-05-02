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
//     2) 후보 엣지 생성 — 티어별 KNN + T1↔T1 트렁크
//     3) 중복 제거 + Union-Find로 컴포넌트 봉합
//     4) Coarse 그리드 1회 빌드 (8× 다운샘플, top-4 mean)
//     5) ThreadPool 병렬 hierarchical bidirectional A*:
//          - 짧은 엣지(<400px): bidirectional A* 직접
//          - 긴 엣지: coarse → corridor mask → bidirectional A* with corridor pruning
//          - corridor 실패 시 짧은 엣지는 무코리도 retry, 긴 엣지는 fail 처리
//     6) RDP 단순화 + 실타일 변환 + ±20타일 결정론적 jitter
//
//   순수 블랙박스 함수 — 외부 상태 무관.
//   픽셀좌표(1px=50타일)는 알고리즘 내부 전용, 반환값은 실타일 좌표.
//
//   핵심 설계 결정:
//   - 위상은 KNN 격자 메쉬 유지 (T1↔T1 트렁크, T2 K=4, T3 K=2 → 시각적 격자감 보존)
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
        constexpr int SECTOR_X_MIN_LOCAL     = -54;
        constexpr int SECTOR_Y_MIN_LOCAL     = -27;
        constexpr int PIXEL_PER_SECTOR_LOCAL = 400;
        constexpr int TILE_BASE_X = SECTOR_X_MIN_LOCAL * PIXEL_PER_SECTOR_LOCAL * TILES_PER_PIXEL;
        constexpr int TILE_BASE_Y = SECTOR_Y_MIN_LOCAL * PIXEL_PER_SECTOR_LOCAL * TILES_PER_PIXEL;

        PixelCoord tileToPixel(Point3 t) noexcept
        {
            return PixelCoord{
                (t.x - TILE_BASE_X) / TILES_PER_PIXEL,
                (t.y - TILE_BASE_Y) / TILES_PER_PIXEL,
                t.z
            };
        }

        Point3 pixelToTileJitter(int px, int py, int z, std::uint64_t seed, bool isEndpoint) noexcept
        {
            const int baseX = px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2;
            const int baseY = py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2;
            if (isEndpoint) return Point3{ baseX, baseY, z };

            std::uint64_t h = seed ^ 0xD6E8FEB86659FD93ULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) * 0x9E3779B97F4A7C15ULL;
            h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(py)) * 0x94D049BB133111EBULL;
            h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
            h ^= h >> 31;

            const int jx = static_cast<int>(h % 41u) - 20;        // -20 .. +20
            const int jy = static_cast<int>((h >> 16) % 41u) - 20;
            return Point3{ baseX + jx, baseY + jy, z };
        }

        //비용 LUT — Terrain enum 값을 인덱스로 사용.
        constexpr float kMinCost = 0.5f;
        constexpr float kSqrt2   = 1.41421356f;

        //경로비용 / 직선거리(px) 상한 — A* 성공해도 이 비율 넘으면 폐기.
        //  land(1.0)만: ratio ≈ 1, detour 50% 늘어도 ≈ 1.5
        //  mountain(8.0) 30% 섞여도 ≈ 3.1
        //  sea(5000) 2~3픽셀 짧은 페리가 500px 경로에 끼면 ratio ≈ 20~30 (허용)
        //  sea 10픽셀 이상은 ratio가 100+ 로 폭증 → 호주↔인도네시아 같은 장거리 횡단 컷.
        constexpr float kMaxCostPerPixel = 30.0f;

        consteval std::array<float, 14> makeCostLUT()
        {
            std::array<float, 14> a{};
            a[static_cast<std::size_t>(Terrain::Land      )] =   1.0f;
            a[static_cast<std::size_t>(Terrain::Sea       )] = 5000.0f;
            a[static_cast<std::size_t>(Terrain::FreshWater)] = 150.0f;
            a[static_cast<std::size_t>(Terrain::Bridge    )] =   0.5f;   //Bridge는 도시 내 강분단 1픽셀 한정. 강/바다 횡단 다리는 사후 자동 형성.
            a[static_cast<std::size_t>(Terrain::CityZone  )] =   0.6f;
            a[static_cast<std::size_t>(Terrain::CityCenter)] =   0.5f;
            a[static_cast<std::size_t>(Terrain::Mountain  )] =   8.0f;
            a[static_cast<std::size_t>(Terrain::Polar     )] =   6.0f;
            a[static_cast<std::size_t>(Terrain::Tundra    )] =   2.0f;
            a[static_cast<std::size_t>(Terrain::Subarctic )] =   0.7f;   //캐나다식 장거리 직선도로 — Land보다 싸게.
            a[static_cast<std::size_t>(Terrain::Monsoon   )] =   1.2f;
            a[static_cast<std::size_t>(Terrain::Sabanna   )] =   1.3f;
            a[static_cast<std::size_t>(Terrain::Desert    )] =   0.6f;   //라스베가스식 장거리 직선도로 — Land보다 싸게.
            a[static_cast<std::size_t>(Terrain::RainForest)] =  11.0f;   //우거짐·습지·홍수 — Mountain(8)보다 비싸게. A*가 안데스/연안 우회 선호.
            return a;
        }
        constexpr std::array<float, 14> kCostLUT = makeCostLUT();

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

        CoarseGrid buildCoarseGrid(const PixelCostGrid& fine)
        {
            CoarseGrid c;
            const std::size_t total = static_cast<std::size_t>(CoarseGrid::W) * CoarseGrid::H;
            c.cost = std::make_unique<float[]>(total);

            const Terrain* fineData = fine.data.get();
            const int Wf = PixelCostGrid::W;
            const int Hf = PixelCostGrid::H;
            constexpr int F = CoarseGrid::F;

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
                        float* dstRow = &c.cost[static_cast<std::size_t>(cy) * CoarseGrid::W];
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
            return c;
        }

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
        struct CityPixel
        {
            int      px;
            int      py;
            int      z;
            CityTier tier;
            Terrain  biome;   //도시 주변 우세 biome — 사막/Subarctic은 KNN 반경 확장
        };

        //도시 biome 샘플링 — 도시 중심 픽셀은 CityCenter라 정보 없음.
        //  주변 ±20px(=1000타일) 11×11 그리드에서 도시/수역 제외하고 우세 biome 결정.
        //  사막/Subarctic 판정용 — 후보 엣지 KNN 반경 확장에만 사용됨.
        Terrain sampleCityBiome(const PixelCostGrid& grid, int px, int py) noexcept
        {
            int counts[13] = {};
            constexpr int R    = 20;
            constexpr int STEP = 4;
            const Terrain* gridData = grid.data.get();
            for (int dy = -R; dy <= R; dy += STEP)
            {
                const int sy = py + dy;
                if (sy < 0 || sy >= PixelCostGrid::H) continue;
                const Terrain* row = gridData + static_cast<std::size_t>(sy) * PixelCostGrid::W;
                for (int dx = -R; dx <= R; dx += STEP)
                {
                    const int sx = px + dx;
                    if (sx < 0 || sx >= PixelCostGrid::W) continue;
                    const Terrain t = row[sx];
                    switch (t)
                    {
                    case Terrain::CityCenter:
                    case Terrain::CityZone:
                    case Terrain::Sea:
                    case Terrain::FreshWater:
                    case Terrain::Bridge:
                        continue;
                    default: break;
                    }
                    ++counts[static_cast<std::size_t>(t)];
                }
            }
            int bestIdx   = static_cast<int>(Terrain::Land);
            int bestCount = 0;
            for (int i = 0; i < 13; ++i)
            {
                if (counts[i] > bestCount) { bestCount = counts[i]; bestIdx = i; }
            }
            return static_cast<Terrain>(bestIdx);
        }

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
        // 후보 엣지 생성 — 티어별 KNN.
        //   T1=6, T2=4, T3=2 — 시각적 격자 메쉬 형성.
        //============================================================
        struct EdgeCand
        {
            int    a;
            int    b;
            double dist;
        };

        constexpr int knnK(CityTier t, Terrain biome) noexcept
        {
            int base;
            switch (t)
            {
            case CityTier::T1: base = 6; break;
            case CityTier::T2: base = 4; break;
            case CityTier::T3: base = 2; break;
            default:           base = 2; break;
            }
            //사막/Subarctic은 가까운 도시 자체가 적어서 K 작게 잡으면 단일 실패가
            //곧 컴포넌트 절단으로 이어짐 → 이웃 수 자체를 늘려 redundancy 확보.
            if (biome == Terrain::Desert || biome == Terrain::Subarctic)
                base += 2;
            return base;
        }
        constexpr int knnMaxDist(CityTier t, Terrain biome) noexcept
        {
            int base;
            switch (t)
            {
            case CityTier::T1: base = 1500; break;
            case CityTier::T2: base = 1200; break;
            case CityTier::T3: base = 500;  break;
            default:           base = 500;  break;
            }
            //사막/Subarctic은 도시 밀도 낮고 도로 건설 쉬움 → 후보 반경 6배.
            //  라스베가스/캐나다식 장거리 도로 표현. 도시 자체가 적어서 엣지 후보
            //  절대 증가량은 미미(T3 500→3000, T2 1200→7200).
            if (biome == Terrain::Desert || biome == Terrain::Subarctic)
                base *= 6;
            //우림은 정반대 — 우거짐/습지로 장거리 도로 거의 불가능. 강 따라 가까운
            //  이웃하고만 후보 형성. 마나우스가 외부와 도로로 거의 안 이어진 현실 반영.
            if (biome == Terrain::RainForest)
                base = base * 7 / 10;
            return base;
        }

        constexpr int T1_TRUNK_K        = 4;
        constexpr int T1_TRUNK_MAX_DIST = 1500;

        std::vector<EdgeCand> buildKnnEdges(const std::vector<CityPixel>& cities,
                                            const SpatialHash& hash)
        {
            std::vector<EdgeCand> out;
            out.reserve(cities.size() * 6);

            std::vector<std::pair<double, int>> nearest;
            nearest.reserve(64);

            for (int i = 0; i < static_cast<int>(cities.size()); ++i)
            {
                const auto& c = cities[i];
                const int K = knnK(c.tier, c.biome);
                const int R = knnMaxDist(c.tier, c.biome);

                nearest.clear();
                hash.forEachInRadius(c.px, c.py, R, [&](int j)
                {
                    if (j == i) return;
                    const auto& o = cities[j];
                    const double dx = static_cast<double>(c.px - o.px);
                    const double dy = static_cast<double>(c.py - o.py);
                    const double d2 = dx * dx + dy * dy;
                    if (d2 > static_cast<double>(R) * R) return;
                    nearest.emplace_back(d2, j);
                });

                if (static_cast<int>(nearest.size()) > K)
                {
                    std::partial_sort(nearest.begin(), nearest.begin() + K, nearest.end(),
                        [](const auto& a, const auto& b) { return a.first < b.first; });
                    nearest.resize(K);
                }

                for (const auto& [d2, j] : nearest)
                {
                    const int a = std::min(i, j);
                    const int b = std::max(i, j);
                    out.push_back(EdgeCand{ a, b, std::sqrt(d2) });
                }
            }
            return out;
        }

        std::vector<EdgeCand> buildT1Trunks(const std::vector<CityPixel>& cities)
        {
            std::vector<int> t1Idx;
            for (int i = 0; i < static_cast<int>(cities.size()); ++i)
                if (cities[i].tier == CityTier::T1) t1Idx.push_back(i);

            std::vector<EdgeCand> out;
            out.reserve(t1Idx.size() * T1_TRUNK_K);
            std::vector<std::pair<double, int>> nearest;

            for (int ii : t1Idx)
            {
                const auto& c = cities[ii];
                nearest.clear();
                for (int jj : t1Idx)
                {
                    if (jj == ii) continue;
                    const auto& o = cities[jj];
                    const double dx = static_cast<double>(c.px - o.px);
                    const double dy = static_cast<double>(c.py - o.py);
                    const double d2 = dx * dx + dy * dy;
                    if (d2 > static_cast<double>(T1_TRUNK_MAX_DIST) * T1_TRUNK_MAX_DIST) continue;
                    nearest.emplace_back(d2, jj);
                }
                if (static_cast<int>(nearest.size()) > T1_TRUNK_K)
                {
                    std::partial_sort(nearest.begin(), nearest.begin() + T1_TRUNK_K, nearest.end(),
                        [](const auto& a, const auto& b) { return a.first < b.first; });
                    nearest.resize(T1_TRUNK_K);
                }
                for (const auto& [d2, jj] : nearest)
                {
                    const int a = std::min(ii, jj);
                    const int b = std::max(ii, jj);
                    out.push_back(EdgeCand{ a, b, std::sqrt(d2) });
                }
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

        std::vector<EdgeCand> ensureConnectivity(const std::vector<EdgeCand>& edges,
                                                  const std::vector<CityPixel>& cities)
        {
            DSU dsu(static_cast<int>(cities.size()));
            for (const auto& e : edges) dsu.unite(e.a, e.b);

            std::unordered_map<int, std::vector<int>> comps;
            for (int i = 0; i < static_cast<int>(cities.size()); ++i)
                comps[dsu.find(i)].push_back(i);

            std::vector<EdgeCand> extras;
            if (comps.size() <= 1) return extras;

            std::vector<std::vector<int>*> sortedComps;
            sortedComps.reserve(comps.size());
            for (auto& [_, mem] : comps) sortedComps.push_back(&mem);
            std::sort(sortedComps.begin(), sortedComps.end(),
                [](const auto* a, const auto* b) { return a->size() > b->size(); });

            const auto& mainComp = *sortedComps[0];
            for (std::size_t k = 1; k < sortedComps.size(); ++k)
            {
                const auto& sub = *sortedComps[k];
                double best = std::numeric_limits<double>::infinity();
                int bestA = -1, bestB = -1;
                for (int s : sub)
                {
                    for (int m : mainComp)
                    {
                        const double dx = static_cast<double>(cities[s].px - cities[m].px);
                        const double dy = static_cast<double>(cities[s].py - cities[m].py);
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
            return extras;
        }

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
        // Corridor 마킹 — coarse 경로 + halo R coarse cells 만큼 mask에 set.
        //   halo=3 → 7×7 stamp = 56 fine pixel 폭 corridor (≈2800타일 폭).
        //============================================================
        void markCorridor(const std::vector<std::uint32_t>& path,
                          int halo,
                          CorridorMask& mask)
        {
            mask.prepare();
            for (std::uint32_t idx : path)
            {
                const int cy = static_cast<int>(idx / CoarseGrid::W);
                const int cx = static_cast<int>(idx % CoarseGrid::W);
                const int x0 = std::max(0, cx - halo);
                const int x1 = std::min(CoarseGrid::W - 1, cx + halo);
                const int y0 = std::max(0, cy - halo);
                const int y1 = std::min(CoarseGrid::H - 1, cy + halo);
                for (int yy = y0; yy <= y1; ++yy)
                {
                    const std::uint32_t base = static_cast<std::uint32_t>(yy) * CoarseGrid::W;
                    for (int xx = x0; xx <= x1; ++xx)
                    {
                        mask.mark(base + static_cast<std::uint32_t>(xx));
                    }
                }
            }
        }

        //corridor 셀 bbox 계산 (fine 박스 사이즈 결정용).
        struct CoarseBBox { int minX, minY, maxX, maxY; };
        CoarseBBox coarseBBox(const std::vector<std::uint32_t>& path, int halo)
        {
            int minX = std::numeric_limits<int>::max();
            int minY = std::numeric_limits<int>::max();
            int maxX = std::numeric_limits<int>::min();
            int maxY = std::numeric_limits<int>::min();
            for (std::uint32_t idx : path)
            {
                const int cy = static_cast<int>(idx / CoarseGrid::W);
                const int cx = static_cast<int>(idx % CoarseGrid::W);
                if (cx < minX) minX = cx;
                if (cx > maxX) maxX = cx;
                if (cy < minY) minY = cy;
                if (cy > maxY) maxY = cy;
            }
            return CoarseBBox{
                std::max(0, minX - halo),
                std::max(0, minY - halo),
                std::min(CoarseGrid::W - 1, maxX + halo),
                std::min(CoarseGrid::H - 1, maxY + halo)
            };
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
        FineBox boxFromCoarseBBox(const CoarseBBox& cb, PixelCoord A, PixelCoord B) noexcept
        {
            //coarse bbox(halo 포함) → fine 픽셀로 확장. start/end도 반드시 포함.
            const int fminX = cb.minX * CoarseGrid::F;
            const int fminY = cb.minY * CoarseGrid::F;
            const int fmaxX = (cb.maxX + 1) * CoarseGrid::F - 1;
            const int fmaxY = (cb.maxY + 1) * CoarseGrid::F - 1;
            return FineBox{
                std::max(0,                    std::min({fminX, A.x, B.x})),
                std::max(0,                    std::min({fminY, A.y, B.y})),
                std::min(PixelCostGrid::W - 1, std::max({fmaxX, A.x, B.x})),
                std::min(PixelCostGrid::H - 1, std::max({fmaxY, A.y, B.y}))
            };
        }

        //--- RDP 단순화 ---
        double perpDist2(int px, int py, int ax, int ay, int bx, int by) noexcept
        {
            const double dx = static_cast<double>(bx - ax);
            const double dy = static_cast<double>(by - ay);
            const double len2 = dx * dx + dy * dy;
            if (len2 == 0.0)
            {
                const double ex = px - ax, ey = py - ay;
                return ex * ex + ey * ey;
            }
            const double t  = ((px - ax) * dx + (py - ay) * dy) / len2;
            const double tc = std::clamp(t, 0.0, 1.0);
            const double cx = ax + tc * dx;
            const double cy = ay + tc * dy;
            const double ex = px - cx, ey = py - cy;
            return ex * ex + ey * ey;
        }

        void rdpRecurse(const std::vector<PixelCoord>& in, int lo, int hi,
                        double eps2, std::vector<int>& keep)
        {
            if (hi <= lo + 1) return;
            int    maxIdx = -1;
            double maxD2  = 0.0;
            const auto& A = in[lo];
            const auto& B = in[hi];
            for (int i = lo + 1; i < hi; ++i)
            {
                const double d2 = perpDist2(in[i].x, in[i].y, A.x, A.y, B.x, B.y);
                if (d2 > maxD2) { maxD2 = d2; maxIdx = i; }
            }
            if (maxD2 > eps2 && maxIdx > 0)
            {
                rdpRecurse(in, lo, maxIdx, eps2, keep);
                keep.push_back(maxIdx);
                rdpRecurse(in, maxIdx, hi, eps2, keep);
            }
        }

        std::vector<PixelCoord> rdpSimplify(const std::vector<PixelCoord>& in, double eps)
        {
            if (in.size() <= 2) return in;
            std::vector<int> keep;
            keep.reserve(in.size() / 4 + 2);
            keep.push_back(0);
            rdpRecurse(in, 0, static_cast<int>(in.size()) - 1, eps * eps, keep);
            keep.push_back(static_cast<int>(in.size()) - 1);

            std::vector<PixelCoord> out;
            out.reserve(keep.size());
            for (int i : keep) out.push_back(in[i]);
            return out;
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

            constexpr int kHalo = 3;
            markCorridor(cr.cells, kHalo, ws.mask);

            //Fine 박스를 corridor bbox에 딱 맞춤 — corridor 외부 셀은 다 prune되니 박스 메모리 낭비 없음.
            const CoarseBBox cb = coarseBBox(cr.cells, kHalo);
            const FineBox fb = boxFromCoarseBBox(cb, A, B);

            AStarOut r = astarPathBidir(A, B, grid, fb.x0, fb.y0, fb.x1, fb.y1, ws, &ws.mask);
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

        //--- 1) tile → pixel 역변환 + biome 샘플링 ---
        std::vector<CityPixel> cps;
        cps.reserve(cities.size());
        int biomeDesert = 0, biomeSubarctic = 0;
        for (const auto& cn : cities)
        {
            const PixelCoord p = tileToPixel(cn.center);
            const Terrain biome = sampleCityBiome(grid, p.x, p.y);
            if      (biome == Terrain::Desert   ) ++biomeDesert;
            else if (biome == Terrain::Subarctic) ++biomeSubarctic;
            cps.push_back(CityPixel{ p.x, p.y, p.z, cn.tier, biome });
        }
        prt(L"  biome (sparse-net cities): desert=%d subarctic=%d (KNN radius x4)\n",
            biomeDesert, biomeSubarctic);

        //--- 2) 공간 해시 ---
        SpatialHash hash(PixelCostGrid::W, PixelCostGrid::H, 80);
        for (int i = 0; i < static_cast<int>(cps.size()); ++i)
            hash.insert(i, cps[i].px, cps[i].py);

        const __int64 tHash = getNanoTimer();

        //--- 3) 후보 엣지 (KNN + T1 트렁크) ---
        std::vector<EdgeCand> knn   = buildKnnEdges(cps, hash);
        const std::size_t     knnN  = knn.size();
        std::vector<EdgeCand> trunk = buildT1Trunks(cps);
        knn.insert(knn.end(), trunk.begin(), trunk.end());
        std::vector<EdgeCand> edges = dedupEdges(std::move(knn));

        const __int64 tEdges = getNanoTimer();

        //--- 4) 연결성 봉합 ---
        std::vector<EdgeCand> extras = ensureConnectivity(edges, cps);
        const std::size_t     extrasN = extras.size();
        if (!extras.empty())
        {
            edges.insert(edges.end(), extras.begin(), extras.end());
            edges = dedupEdges(std::move(edges));
        }

        const __int64 tConn = getNanoTimer();

        prt(L"  edges: knn=%zu trunk=%zu connectivity=%zu => final=%zu\n",
            knnN, trunk.size(), extrasN, edges.size());

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
        const __int64 tCoarseStart = getNanoTimer();
        CoarseGrid coarse = buildCoarseGrid(grid);
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

                std::vector<PixelCoord> simp = rdpSimplify(raw, 1.5);
                vertsTotal.fetch_add(simp.size(), std::memory_order_relaxed);

                RoadPolyLine line;
                line.verts.reserve(simp.size());
                for (std::size_t k = 0; k < simp.size(); ++k)
                {
                    const bool endpoint = (k == 0) || (k + 1 == simp.size());
                    line.verts.push_back(
                        pixelToTileJitter(simp[k].x, simp[k].y, simp[k].z, seed, endpoint));
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
