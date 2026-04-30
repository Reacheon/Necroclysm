module;
#include <SDL3/SDL.h>

module procGen;

import std;
import util;

//============================================================
// 도시 좌표 배열 — 게임 시작 1회 절차생성의 1단계
//   입력: seed + PixelCostGrid (사전 PNG 디코딩 결과)
//   출력: 실타일 Point3 좌표를 가진 CityNode 약 3000개
//
//   순수 블랙박스 함수 — 외부 상태 무관.
//   픽셀 좌표(1px=50타일)는 알고리즘 내부 전용,
//   반환값은 반드시 실타일 좌표로 변환되어 나감.
//============================================================
namespace procGen
{
    namespace
    {
        //------------------------------------------------------------
        // 좌표 변환 — procGen 픽셀(0,0) = 섹터(-54,-27) 좌상단
        //   loadWorldGrid의 SECTOR_X_MIN / SECTOR_Y_MIN과 정합
        //   바뀌면 여기도 같이 바꿔야 함
        //------------------------------------------------------------
        constexpr int SECTOR_X_MIN_LOCAL = -54;
        constexpr int SECTOR_Y_MIN_LOCAL = -27;
        constexpr int PIXEL_PER_SECTOR_LOCAL = 400;
        constexpr int TILE_BASE_X = SECTOR_X_MIN_LOCAL * PIXEL_PER_SECTOR_LOCAL * TILES_PER_PIXEL; // -1,080,000
        constexpr int TILE_BASE_Y = SECTOR_Y_MIN_LOCAL * PIXEL_PER_SECTOR_LOCAL * TILES_PER_PIXEL; //   -540,000

        Point3 pixelToTileCenter(int px, int py) noexcept
        {
            return Point3{
                px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2,
                py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2,
                0
            };
        }

        //------------------------------------------------------------
        // 도시 배치 파라미터 — 모든 튜닝 포인트는 여기 모음
        //------------------------------------------------------------
        // 티어별 최소 간격(픽셀). T1이 가장 크게 떨어짐
        constexpr int R_T1 = 600;   //  30,000 타일 (≈ 1.5 섹터)
        constexpr int R_T2 = 200;   //  10,000 타일
        constexpr int R_T3 = 80;    //   4,000 타일

        // 절차생성 + 사전배치 합산 목표 도시 수 (= 3000)
        constexpr int TARGET_T1 = 50;
        constexpr int TARGET_T2 = 350;
        constexpr int TARGET_T3 = 2600;

        // 사전배치 도시의 티어 분류 (CityZone 픽셀 면적 기준)
        constexpr int PRE_MARKED_T1_AREA = 800;   // 약 28×28 픽셀 이상 → 대도시
        constexpr int PRE_MARKED_T2_AREA = 200;   // 약 14×14 픽셀 이상 → 중도시
        constexpr int PRE_MARKED_BUFFER  = 30;    // 사전배치 영역 + 버퍼 픽셀

        // 티어당 최대 다트 시도 배율 (placed * MULT)
        constexpr int MAX_ATTEMPTS_MULT = 50;

        //------------------------------------------------------------
        // 지형별 도시 입지 확률 가중치 (0.0 = 절대 배치 불가)
        //   사전배치 도시 영역(CityZone/CityCenter)도 차단 → 중복 방지
        //------------------------------------------------------------
        constexpr double terrainWeight(Terrain t) noexcept
        {
            switch (t)
            {
            case Terrain::Land:        return 1.00;
            case Terrain::Monsoon:     return 0.85;
            case Terrain::Sabanna:     return 0.75;
            case Terrain::Subarctic:   return 0.50;
            case Terrain::Desert:      return 0.20;
            case Terrain::Tundra:      return 0.15;
            // 절대 불가 영역
            case Terrain::Sea:
            case Terrain::FreshWater:
            case Terrain::Bridge:
            case Terrain::Mountain:
            case Terrain::Polar:
            case Terrain::CityZone:
            case Terrain::CityCenter:
            default:                   return 0.0;
            }
        }

        constexpr int radiusForTier(CityTier t) noexcept
        {
            switch (t)
            {
            case CityTier::T1: return R_T1;
            case CityTier::T2: return R_T2;
            case CityTier::T3: return R_T3;
            }
            return R_T3;
        }

        //------------------------------------------------------------
        // 내부 도시 표현 — 픽셀 좌표 + 충돌 반경 + 티어
        //   마지막에 CityNode로 변환되어 빠져나감
        //------------------------------------------------------------
        struct CityRec
        {
            int px;
            int py;
            int radius;     // 자신의 충돌 영역 (픽셀)
            CityTier tier;
        };

        //------------------------------------------------------------
        // 균등 격자 공간 해시 — 셀 크기 = R_T3
        //   삽입은 O(1), 충돌검사는 검색반경/셀크기 만큼의 셀만 훑음
        //------------------------------------------------------------
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
                const int cx = px / cellSize;
                const int cy = py / cellSize;
                cells[cellIdx(cx, cy)].push_back(idx);
            }

            // 후보 (px,py,R)이 기존 도시들과 충돌하면 true
            //   currentMaxR : 시스템 내 가장 큰 반경 (검색 윈도우 결정)
            //   판정 : dist(A,B) < max(R_A, R_B) 이면 충돌
            bool conflicts(int px, int py, int R, int currentMaxR,
                           const std::vector<CityRec>& cities) const
            {
                const int searchR = std::max(R, currentMaxR);
                const int searchCells = (searchR + cellSize - 1) / cellSize;
                const int cxC = px / cellSize;
                const int cyC = py / cellSize;

                const int x0 = std::max(0, cxC - searchCells);
                const int x1 = std::min(gridW - 1, cxC + searchCells);
                const int y0 = std::max(0, cyC - searchCells);
                const int y1 = std::min(gridH - 1, cyC + searchCells);

                for (int cy = y0; cy <= y1; ++cy)
                {
                    for (int cx = x0; cx <= x1; ++cx)
                    {
                        for (int idx : cells[cellIdx(cx, cy)])
                        {
                            const CityRec& c = cities[idx];
                            const long long dx = static_cast<long long>(c.px) - px;
                            const long long dy = static_cast<long long>(c.py) - py;
                            const long long d2 = dx * dx + dy * dy;
                            const long long minD = std::max(R, c.radius);
                            if (d2 < minD * minD) return true;
                        }
                    }
                }
                return false;
            }
        };

        //------------------------------------------------------------
        // Phase 1 : 사전배치 도시 추출
        //   8-connected 클러스터링 (CityZone ∪ CityCenter)
        //   → 1 클러스터 = 1 도시
        //   centroid는 클러스터 안의 CityCenter 픽셀 평균 (없으면 전체 평균)
        //------------------------------------------------------------
        struct PreMarkedExtract
        {
            std::vector<CityRec> cities;
            int t1Count{};
            int t2Count{};
            int t3Count{};
        };

        PreMarkedExtract extractPreMarked(const PixelCostGrid& grid)
        {
            const int W = PixelCostGrid::W;
            const int H = PixelCostGrid::H;
            const std::size_t total = static_cast<std::size_t>(W) * H;

            //--- 1단계: 단일 시퀀셜 스캔으로 City 픽셀 인덱스만 수집 ---
            //    933M 비트 visited 배열 대신 sparse set 사용 (도시 픽셀은 0.001%)
            const __int64 t0 = getNanoTimer();

            std::unordered_set<std::size_t> cityPixels;
            cityPixels.reserve(8192);
            for (std::size_t i = 0; i < total; ++i)
            {
                const Terrain t = grid.data[i];
                if (t == Terrain::CityZone || t == Terrain::CityCenter)
                {
                    cityPixels.insert(i);
                }
            }

            const __int64 t1 = getNanoTimer();
            prt(L"  [extract] city pixels = %zu  (scan %.1f ms)\n",
                cityPixels.size(), (t1 - t0) / 1.0e6);

            PreMarkedExtract out{};
            if (cityPixels.empty()) return out;

            //--- 2단계: 클러스터링 — set의 모든 원소를 BFS로 묶음 ---
            std::vector<std::size_t> frontier;
            frontier.reserve(2048);

            while (!cityPixels.empty())
            {
                const std::size_t startIdx = *cityPixels.begin();
                cityPixels.erase(cityPixels.begin());

                long long sumCenterX = 0, sumCenterY = 0;
                int centerCount = 0;
                int areaCount = 0;
                long long sumX = 0, sumY = 0;
                const int sx0 = static_cast<int>(startIdx % W);
                const int sy0 = static_cast<int>(startIdx / W);
                int minX = sx0, maxX = sx0, minY = sy0, maxY = sy0;

                frontier.clear();
                frontier.push_back(startIdx);

                while (!frontier.empty())
                {
                    const std::size_t curIdx = frontier.back();
                    frontier.pop_back();

                    const int cx = static_cast<int>(curIdx % W);
                    const int cy = static_cast<int>(curIdx / W);

                    ++areaCount;
                    sumX += cx;
                    sumY += cy;
                    if (cx < minX) minX = cx;
                    if (cx > maxX) maxX = cx;
                    if (cy < minY) minY = cy;
                    if (cy > maxY) maxY = cy;
                    if (grid.data[curIdx] == Terrain::CityCenter)
                    {
                        sumCenterX += cx;
                        sumCenterY += cy;
                        ++centerCount;
                    }

                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        const int ny = cy + dy;
                        if (ny < 0 || ny >= H) continue;
                        for (int dx = -1; dx <= 1; ++dx)
                        {
                            if (dx == 0 && dy == 0) continue;
                            const int nx = cx + dx;
                            if (nx < 0 || nx >= W) continue;
                            const std::size_t nIdx = static_cast<std::size_t>(ny) * W + nx;
                            auto it = cityPixels.find(nIdx);
                            if (it != cityPixels.end())
                            {
                                cityPixels.erase(it);
                                frontier.push_back(nIdx);
                            }
                        }
                    }
                }

                int centroidX, centroidY;
                if (centerCount > 0)
                {
                    centroidX = static_cast<int>(sumCenterX / centerCount);
                    centroidY = static_cast<int>(sumCenterY / centerCount);
                }
                else
                {
                    // CityCenter 픽셀이 없으면 (CityZone만 칠해진 클러스터) 전체 centroid 사용
                    centroidX = static_cast<int>(sumX / areaCount);
                    centroidY = static_cast<int>(sumY / areaCount);
                }

                // 바운딩 박스 기반 외접 반경 (보수적 추정)
                const int boundR = std::max({
                    maxX - centroidX,
                    centroidX - minX,
                    maxY - centroidY,
                    centroidY - minY
                });

                CityTier tier;
                int minTierR;
                if (areaCount >= PRE_MARKED_T1_AREA)
                {
                    tier = CityTier::T1;
                    minTierR = R_T1 / 4;
                    ++out.t1Count;
                }
                else if (areaCount >= PRE_MARKED_T2_AREA)
                {
                    tier = CityTier::T2;
                    minTierR = R_T2 / 4;
                    ++out.t2Count;
                }
                else
                {
                    tier = CityTier::T3;
                    minTierR = R_T3 / 2;
                    ++out.t3Count;
                }

                const int R = std::max(boundR + PRE_MARKED_BUFFER, minTierR);

                out.cities.push_back(CityRec{centroidX, centroidY, R, tier});
            }

            const __int64 t2 = getNanoTimer();
            prt(L"  [extract] clusters = %zu  (BFS %.1f ms)\n",
                out.cities.size(), (t2 - t1) / 1.0e6);

            return out;
        }

        //------------------------------------------------------------
        // Phase 3 : 티어별 다트 던지기 (rejection sampling)
        //   targetCount만큼 통과시킬 때까지 또는 max_attempts 초과까지 반복
        //------------------------------------------------------------
        struct DartResult
        {
            int placed{};
            int attempts{};
            int rejectTerrain{};
            int rejectConflict{};
        };

        DartResult placeTier(CityTier tier, int targetCount,
                             std::vector<CityRec>& cities,
                             SpatialHash& hash,
                             int& currentMaxR,
                             std::mt19937_64& rng,
                             const PixelCostGrid& grid,
                             const CitySink& onPlaced)
        {
            DartResult dr{};
            if (targetCount <= 0) return dr;

            const int W = PixelCostGrid::W;
            const int H = PixelCostGrid::H;
            const int R = radiusForTier(tier);
            const int maxAttempts = targetCount * MAX_ATTEMPTS_MULT;

            std::uniform_int_distribution<int> distX(0, W - 1);
            std::uniform_int_distribution<int> distY(0, H - 1);
            std::uniform_real_distribution<double> distU(0.0, 1.0);

            while (dr.placed < targetCount && dr.attempts < maxAttempts)
            {
                ++dr.attempts;
                const int px = distX(rng);
                const int py = distY(rng);

                const Terrain t = grid.at(px, py);
                const double w = terrainWeight(t);
                if (w <= 0.0)
                {
                    ++dr.rejectTerrain;
                    continue;
                }
                if (w < 1.0 && distU(rng) > w)
                {
                    ++dr.rejectTerrain;
                    continue;
                }

                if (hash.conflicts(px, py, R, currentMaxR, cities))
                {
                    ++dr.rejectConflict;
                    continue;
                }

                const int idx = static_cast<int>(cities.size());
                cities.push_back(CityRec{px, py, R, tier});
                hash.insert(idx, px, py);
                if (R > currentMaxR) currentMaxR = R;
                ++dr.placed;

                //진행 콜백 — 픽셀 → 실타일 변환해서 즉시 통지(누적 단위 1)
                if (onPlaced)
                {
                    onPlaced(CityNode{ pixelToTileCenter(px, py), tier });
                }
            }

            return dr;
        }
    }

    //@brief 게임 시작 1회, 도시 좌표 약 3000개를 절차적으로 배치한다. 순수 블랙박스 함수.
    //@param seed 난수 시드 (재현 가능)
    //@param grid 월드 픽셀 지형 그리드 (loadWorldGrid 결과)
    //@param onPlaced 옵션 진행 콜백. default no-op이면 출력에 영향 X = 순수성 유지.
    //@return 실타일 좌표 + 티어를 가진 CityNode 벡터 (사전배치 + 절차생성 합쳐짐)
    std::vector<CityNode> placeCities(std::uint64_t seed, const PixelCostGrid& grid, CitySink onPlaced)
    {
        const __int64 tStart = getNanoTimer();

        prt(L"[procGen] placeCities start (seed=%llu)\n",
            static_cast<unsigned long long>(seed));

        //--- Phase 1 : 사전배치 도시 추출 ---
        PreMarkedExtract pre = extractPreMarked(grid);
        const __int64 tPre = getNanoTimer();

        //--- Phase 2 : 공간 해시 + 사전배치 등록 ---
        SpatialHash hash(PixelCostGrid::W, PixelCostGrid::H, R_T3);

        std::vector<CityRec> cities = std::move(pre.cities);
        cities.reserve(TARGET_T1 + TARGET_T2 + TARGET_T3 + 100);

        int currentMaxR = 0;
        for (int i = 0; i < static_cast<int>(cities.size()); ++i)
        {
            hash.insert(i, cities[i].px, cities[i].py);
            if (cities[i].radius > currentMaxR) currentMaxR = cities[i].radius;

            //사전배치 도시도 진행 통지 — 화면에 같이 등장하게
            if (onPlaced)
            {
                onPlaced(CityNode{
                    pixelToTileCenter(cities[i].px, cities[i].py),
                    cities[i].tier
                });
            }
        }
        const __int64 tHash = getNanoTimer();

        //--- Phase 3 : 티어별 절차 생성 ---
        std::mt19937_64 rng(seed);

        const int needT1 = std::max(0, TARGET_T1 - pre.t1Count);
        const int needT2 = std::max(0, TARGET_T2 - pre.t2Count);
        const int needT3 = std::max(0, TARGET_T3 - pre.t3Count);

        const DartResult d1 = placeTier(CityTier::T1, needT1, cities, hash, currentMaxR, rng, grid, onPlaced);
        const DartResult d2 = placeTier(CityTier::T2, needT2, cities, hash, currentMaxR, rng, grid, onPlaced);
        const DartResult d3 = placeTier(CityTier::T3, needT3, cities, hash, currentMaxR, rng, grid, onPlaced);

        const __int64 tProc = getNanoTimer();

        //--- Phase 4 : 픽셀 → 실타일 변환 ---
        std::vector<CityNode> result;
        result.reserve(cities.size());
        for (const auto& c : cities)
        {
            result.push_back(CityNode{
                pixelToTileCenter(c.px, c.py),
                c.tier
            });
        }

        const __int64 tDone = getNanoTimer();

        //--- 리포트 ---
        const double preMs   = (tPre  - tStart) / 1.0e6;
        const double hashMs  = (tHash - tPre  ) / 1.0e6;
        const double procMs  = (tProc - tHash ) / 1.0e6;
        const double convMs  = (tDone - tProc ) / 1.0e6;
        const double totalMs = (tDone - tStart) / 1.0e6;

        prt(L"  pre-marked extract : %8.2f ms  (T1=%d T2=%d T3=%d, total=%zu)\n",
            preMs, pre.t1Count, pre.t2Count, pre.t3Count,
            static_cast<std::size_t>(pre.t1Count + pre.t2Count + pre.t3Count));
        prt(L"  hash build         : %8.2f ms\n", hashMs);
        prt(L"  procedural place   : %8.2f ms\n", procMs);

        auto reportTier = [](const wchar_t* name, int target, const DartResult& d) {
            prt(L"    %ls: target=%4d placed=%4d  (attempts=%d rejTerrain=%d rejConflict=%d)\n",
                name, target, d.placed, d.attempts, d.rejectTerrain, d.rejectConflict);
        };
        reportTier(L"T1", needT1, d1);
        reportTier(L"T2", needT2, d2);
        reportTier(L"T3", needT3, d3);

        prt(L"  finalize           : %8.2f ms\n", convMs);
        prt(L"  total              : %8.2f ms  (%.2f s)\n", totalMs, totalMs / 1000.0);
        prt(L"  total cities       : %zu  (target=%d)\n",
            result.size(), TARGET_T1 + TARGET_T2 + TARGET_T3);

        // 목표 미달 시 경고
        const int placedTotal = static_cast<int>(result.size());
        const int targetTotal = TARGET_T1 + TARGET_T2 + TARGET_T3;
        if (placedTotal < targetTotal * 0.9)
        {
            const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
            prt(warn, L"  [WARN] city count below 90%% of target — terrain budget too tight or radii too large\n");
        }

        return result;
    }
}
