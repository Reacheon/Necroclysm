module;
#include <SDL3/SDL.h>

module procGen;

import std;
import util;

//============================================================
// 도시 좌표 배열 — 게임 시작 1회 절차생성의 1단계.
//   입력: seed + PixelCostGrid (사전 PNG 디코딩 결과)
//   출력: 실타일 Point3 좌표를 가진 CityNode 약 3000개
//
//   순수 블랙박스 함수 — 외부 상태 무관.
//   픽셀 좌표(1px=48타일)는 알고리즘 내부 전용,
//   반환값은 반드시 실타일 좌표로 변환되어 나감.
//
// 통계 기반 도시 입지 모델 (로컬 검사 + rejection sampling):
//   픽셀 1개 무작위 선택 → 주변을 즉석 스캔해 가장 가까운 물까지 거리 산출
//   → 점수 계산 후 비례 확률로 accept/reject. 거리장 사전계산 없음.
//
//   점수 = baseTerrain × waterBonus(d, tier) × latitudeBand(py)
//
//   waterBonus: 해안/대하천 가까울수록 지수감쇠 보너스. 티어별 비대칭으로
//     T1(대도시) 거의 항상 물가, T3(소도시) 내륙 평지도 OK.
//
//   latitudeBand: 위도 40°에서 피크, σ=25°. py=10800이 적도(0°),
//     py=0이 북극(+90°), py=21600이 남극(-90°). 적도/극지방 자연 감산.
//
//   거리장 사전계산을 안 쓰는 이유: 도시 ~3K개 vs 픽셀 933M개로 도시 수가
//   압도적으로 적음 → 사전계산 비용(O(N))이 다트 비용(O(R²)×다트수)을 크게 초과.
//   로컬 검사가 ~10× 빠름. 통계적으론 등가.
//============================================================
namespace procGen
{
    namespace
    {
        //--- 도시 배치 파라미터 (모든 튜닝 포인트 집중) ---

        //티어별 최소 간격(픽셀). 충돌 검사는 min(R_A, R_B)이므로 작은 도시는 자기 R만큼만
        //떨어지면 됨(큰 도시 옆에 위성 마을 허용). 1px ≈ 1km.
        constexpr int R_T1 = 200;   //  9,600 타일 (= 200px × 48). 대도시 간격
        constexpr int R_T2 = 100;   //  4,800 타일. 중도시 간격
        constexpr int R_T3 = 50;    //  2,400 타일. 마을 간 / 위성도시 간격
                                    //   한 도(道) 안에 마을 5~10개 정도 분포되는 값.

        //절차생성 + 사전배치 합산 목표 도시 수 (≈ 3000, 근사치).
        constexpr int TARGET_T1 = 50;
        constexpr int TARGET_T2 = 350;
        constexpr int TARGET_T3 = 2600;

        //사전배치 도시의 티어 분류 (CityZone 픽셀 면적 기준).
        constexpr int PRE_MARKED_T1_AREA = 800;   // 약 28×28 픽셀 이상 → 대도시
        constexpr int PRE_MARKED_T2_AREA = 200;   // 약 14×14 픽셀 이상 → 중도시
        constexpr int PRE_MARKED_BUFFER  = 30;    // 사전배치 영역 + 버퍼 픽셀

        //다트 시도 한도(placed * MULT). rejection 다층(지형/점수/충돌) 고려한 값.
        constexpr int MAX_ATTEMPTS_MULT = 100;

        //로컬 물 스캔 반경(실픽셀). waterBonus 꼬리 도달 거리.
        //  T1 tau=8 기준 50px에서 보너스 ≈ 1.003 (사실상 1.0). 이 너머는 의미 없음.
        constexpr int LOCAL_SCAN_R = 50;

        //티어별 waterBonus 파라미터 — peak/tau는 LUT 빌드와 maxScore 계산에 공유.
        constexpr double WATER_PEAK[3] = { 1.5, 1.0, 0.4 };   // T1, T2, T3
        constexpr double WATER_TAU [3] = { 8.0, 14.0, 25.0 }; // 실픽셀 단위

        //해안/강 보너스 LUT — squared Euclidean distance(실픽셀²)로 인덱싱.
        //  인덱스 = dSq ∈ [0, LOCAL_SCAN_R²] = [0, 2500]. 초과는 호출자가 1.0으로 처리.
        //  티어별 (peak, tau) 비대칭으로 입지 패턴 차별화:
        //    T1: peak=1.5, tau=8 px  → d=0에서 2.5x, d=15에서 ~1.2x
        //    T2: peak=1.0, tau=14 px → d=0에서 2.0x, 완만하게 1로 수렴
        //    T3: peak=0.4, tau=25 px → d=0에서 1.4x, 거의 평탄(내륙도 OK)
        struct WaterBonusLut
        {
            static constexpr int MAX_DSQ = LOCAL_SCAN_R * LOCAL_SCAN_R;
            double table[3][MAX_DSQ + 1]{};

            WaterBonusLut() noexcept
            {
                for (int t = 0; t < 3; ++t)
                {
                    for (int dSq = 0; dSq <= MAX_DSQ; ++dSq)
                    {
                        const double d = std::sqrt(static_cast<double>(dSq));
                        table[t][dSq] = 1.0 + WATER_PEAK[t] * std::exp(-d / WATER_TAU[t]);
                    }
                }
            }

            double get(int dSq, CityTier tier) const noexcept
            {
                if (dSq > MAX_DSQ) return 1.0;
                return table[static_cast<int>(tier)][dSq];
            }
        };

        //내부 도시 표현 — 픽셀 좌표 + 충돌 반경 + 티어. 최종 단계에서 CityNode로 변환.
        struct CityRec
        {
            int px;
            int py;
            int radius;     //자신의 충돌 영역(픽셀)
            CityTier tier;
        };

        //균등 격자 공간 해시 — 셀 크기 = R_T3.
        //  삽입은 O(1), 충돌검사는 검색반경/셀크기 만큼의 셀만 훑음.
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

            //후보 (px,py,R)이 기존 도시들과 충돌하면 true.
            //  판정: dist(A,B) < min(R_A, R_B)이면 충돌.
            //
            //  min을 쓰는 이유: 큰 도시(T1)는 자기 R만큼 같은 티어와 떨어져야 하지만,
            //  작은 도시(T3)가 T1 옆에 위성으로 붙는 건 허용해야 함. min이면 각 도시가
            //  "자기 영역만" 주장하는 셈이라 큰-작은 페어는 작은 쪽 R로 결정됨.
            //
            //  사전배치(CityZone)는 terrainWeight=0이라 절차생성이 그 위로 안 떨어지므로
            //  물리적 겹침은 자동 방지. radius=boundR+buffer는 사전배치끼리의 클러스터 분리용.
            bool conflicts(int px, int py, int R, int /*unusedMaxR*/, const std::vector<CityRec>& cities) const
            {
                //min 룰이라 충돌 거리 ≤ R. 검색 범위도 R로 충분 — 멀리 있는 T1은
                //min(R_candidate, R_T1) 안에 못 들어옴, T3 다트가 T1의 600px 영역을
                //훑는 낭비를 제거(10× 이상 빨라짐).
                const int searchCells = (R + cellSize - 1) / cellSize;
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
                            //작은 R 기준 — 위성도시 패턴 허용
                            const long long minD = std::min(R, c.radius);
                            if (d2 < minD * minD) return true;
                        }
                    }
                }
                return false;
            }
        };

        //티어별 다트 던지기(rejection sampling):
        //  1. 균등 무작위 (px, py) 선택
        //  2. terrainWeight ≤ 0이면 즉시 reject (가장 빠른 컷)
        //  3. scanForWaterSq로 가장 가까운 물까지 거리² 산출
        //  4. score = base × waterBonus(dSq, tier) × latLut[py]
        //  5. accept 확률 = score / maxScore[tier]  (uniform [0,1) 비교)
        //  6. SpatialHash 충돌 검사
        //  7. 통과 시 등록 + 콜백 발사
        //
        //  maxScore[tier] = max(base) × max(waterBonus) × max(lat)
        //                 = 1.0 × (1 + WATER_PEAK[tier]) × 1.0
        struct DartResult
        {
            int placed{};
            int attempts{};
            int rejectTerrain{};
            int rejectScore{};
            int rejectConflict{};
        };

        //--- 헬퍼 함수 forward declarations (stepdown 순서) ---
        DartResult placeTier(CityTier tier, int targetCount, std::vector<CityRec>& cities, SpatialHash& hash, int& currentMaxR, std::mt19937_64& rng, const PixelCostGrid& grid, const std::vector<double>& latLut, const WaterBonusLut& waterLut, const CitySink& onPlaced);
        Point3 pixelToTileCenter(int px, int py) noexcept;
    }

    //도시 좌표 약 3000개를 절차적으로 배치(사전배치 + 절차생성). 순수 블랙박스 함수.
    //onPlaced default no-op이면 출력 영향 없음. 반환은 실타일 Point3 좌표.
    std::vector<CityNode> placeCities(std::uint64_t seed, const PixelCostGrid& grid, CitySink onPlaced)
    {
        const __int64 tStart = getNanoTimer();

        prt(L"[procGen] placeCities start (seed=%llu)\n",
            static_cast<unsigned long long>(seed));

        //--- Phase 0 : 사전배치 도시 추출 ---
        //  8-connected 클러스터링(CityZone ∪ CityCenter ∪ CityRiver ∪ CitySea).
        //  1 클러스터 = 1 도시. centroid는 클러스터 안의 CityCenter 픽셀 평균(없으면 전체 평균).
        //  CityRiver/CitySea(도시 내 강·해협)도 클러스터에 포함 — 강·바다가 도시를 가로지르는
        //  경우(이스탄불·홍콩·런던 등)에도 하나의 도시로 묶이고 bbox/면적이 수역까지 정확히 반영됨.
        std::vector<CityRec> cities;
        cities.reserve(TARGET_T1 + TARGET_T2 + TARGET_T3 + 100);
        int preT1Count = 0, preT2Count = 0, preT3Count = 0;
        {
            constexpr int W = PixelCostGrid::W;
            constexpr int H = PixelCostGrid::H;
            constexpr std::size_t total = static_cast<std::size_t>(W) * H;

            //1단계: 단일 시퀀셜 스캔으로 City 픽셀 인덱스만 수집.
            //  933M 비트 visited 배열 대신 sparse set 사용(도시 픽셀은 0.001%).
            const __int64 t0 = getNanoTimer();

            std::unordered_set<std::size_t> cityPixels;
            cityPixels.reserve(8192);
            for (std::size_t i = 0; i < total; ++i)
            {
                const Terrain t = grid.data[i];
                if (t == Terrain::CityZone || t == Terrain::CityCenter
                    || t == Terrain::CityRiver || t == Terrain::CitySea)
                {
                    cityPixels.insert(i);
                }
            }

            const __int64 t1 = getNanoTimer();
            prt(L"  [extract] city pixels = %zu  (scan %.1f ms)\n",
                cityPixels.size(), (t1 - t0) / 1.0e6);

            //2단계: 클러스터링 — set의 모든 원소를 BFS로 묶음.
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
                    //CityCenter 픽셀이 없으면(CityZone만 칠해진 클러스터) 전체 centroid 사용.
                    centroidX = static_cast<int>(sumX / areaCount);
                    centroidY = static_cast<int>(sumY / areaCount);
                }

                //바운딩 박스 기반 외접 반경(보수적 추정).
                const int boundR = std::max({
                    maxX - centroidX,
                    centroidX - minX,
                    maxY - centroidY,
                    centroidY - minY
                });

                //사전배치 티어는 클러스터 면적(픽셀 수) 기준으로 분류. PRE_MARKED_T*_AREA 참고.
                CityTier tier;
                int minTierR;
                if (areaCount >= PRE_MARKED_T1_AREA)
                {
                    tier = CityTier::T1;
                    minTierR = R_T1 / 4;
                    ++preT1Count;
                }
                else if (areaCount >= PRE_MARKED_T2_AREA)
                {
                    tier = CityTier::T2;
                    minTierR = R_T2 / 4;
                    ++preT2Count;
                }
                else
                {
                    tier = CityTier::T3;
                    minTierR = R_T3 / 2;
                    ++preT3Count;
                }

                const int R = std::max(boundR + PRE_MARKED_BUFFER, minTierR);

                cities.push_back(CityRec{centroidX, centroidY, R, tier});
            }

            const __int64 t2 = getNanoTimer();
            prt(L"  [extract] clusters = %zu  (BFS %.1f ms)\n",
                cities.size(), (t2 - t1) / 1.0e6);
        }
        const __int64 tPre = getNanoTimer();

        //--- Phase 1 : 공간 해시 + 사전배치 등록 ---
        SpatialHash hash(PixelCostGrid::W, PixelCostGrid::H, R_T3);

        int currentMaxR = 0;
        for (int i = 0; i < static_cast<int>(cities.size()); ++i)
        {
            hash.insert(i, cities[i].px, cities[i].py);
            if (cities[i].radius > currentMaxR) currentMaxR = cities[i].radius;

            //사전배치 도시도 진행 통지 — 화면에 같이 등장하도록.
            if (onPlaced)
            {
                onPlaced(CityNode{
                    pixelToTileCenter(cities[i].px, cities[i].py),
                    cities[i].tier
                });
            }
        }
        const __int64 tHash = getNanoTimer();

        //--- Phase 2 : LUT 빌드 (waterBonus + latitudeBand) ---
        const WaterBonusLut waterLut;                            // 3 × 2501 × 8B ≈ 60KB

        //위도 belt LUT — py(0..H-1) → 위도 가중치(0.5..1.0). H × 8B ≈ 168KB.
        //  py=0 → +90°(북극), py=10800 → 0°(적도), py=21600 → -90°(남극).
        //  |위도|=40°에서 피크 1.0, σ=25°. 너무 강하지 않게 0.5에서 클램프.
        //  적도(열대 우림)와 극지방을 부드럽게 감산 — 같은 Subarctic 안에서도 남쪽 끝과
        //  북쪽 끝이 차별화됨. terrain만으론 못 잡는 연속성을 보완.
        std::vector<double> latLut(static_cast<std::size_t>(PixelCostGrid::H));
        {
            constexpr double H_HALF = PixelCostGrid::H / 2.0;       // 10800.0
            constexpr double DEG_PER_PX = 90.0 / H_HALF;            // 픽셀당 위도(°)
            constexpr double PEAK_LAT = 40.0;                       // 피크 위도(°)
            constexpr double SIGMA = 25.0;                          // 가우시안 σ(°)
            constexpr double DENOM = 2.0 * SIGMA * SIGMA;
            for (int py = 0; py < PixelCostGrid::H; ++py)
            {
                const double lat = (H_HALF - py) * DEG_PER_PX;       // [+90, -90]
                const double dev = std::abs(lat) - PEAK_LAT;
                const double band = std::exp(-(dev * dev) / DENOM);
                latLut[py] = std::max(0.5, band);
            }
        }
        const __int64 tLut = getNanoTimer();

        //--- Phase 3 : 티어별 rejection sampling ---
        std::mt19937_64 rng(seed);

        const int needT1 = std::max(0, TARGET_T1 - preT1Count);
        const int needT2 = std::max(0, TARGET_T2 - preT2Count);
        const int needT3 = std::max(0, TARGET_T3 - preT3Count);

        const DartResult d1 = placeTier(CityTier::T1, needT1, cities, hash,
                                        currentMaxR, rng, grid, latLut, waterLut, onPlaced);
        const DartResult d2 = placeTier(CityTier::T2, needT2, cities, hash,
                                        currentMaxR, rng, grid, latLut, waterLut, onPlaced);
        const DartResult d3 = placeTier(CityTier::T3, needT3, cities, hash,
                                        currentMaxR, rng, grid, latLut, waterLut, onPlaced);

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
        const double preMs   = (tPre   - tStart) / 1.0e6;
        const double hashMs  = (tHash  - tPre  ) / 1.0e6;
        const double lutMs   = (tLut   - tHash ) / 1.0e6;
        const double procMs  = (tProc  - tLut  ) / 1.0e6;
        const double convMs  = (tDone  - tProc ) / 1.0e6;
        const double totalMs = (tDone  - tStart) / 1.0e6;

        prt(L"  pre-marked extract : %8.2f ms  (T1=%d T2=%d T3=%d, total=%zu)\n",
            preMs, preT1Count, preT2Count, preT3Count,
            static_cast<std::size_t>(preT1Count + preT2Count + preT3Count));
        prt(L"  hash + register    : %8.2f ms\n", hashMs);
        prt(L"  LUT build          : %8.2f ms\n", lutMs);
        prt(L"  procedural place   : %8.2f ms\n", procMs);

        auto reportTier = [](const wchar_t* name, int target, const DartResult& d) {
            prt(L"    %ls: target=%4d placed=%4d  (att=%d rTerrain=%d rScore=%d rConflict=%d)\n",
                name, target, d.placed, d.attempts,
                d.rejectTerrain, d.rejectScore, d.rejectConflict);
        };
        reportTier(L"T1", needT1, d1);
        reportTier(L"T2", needT2, d2);
        reportTier(L"T3", needT3, d3);

        prt(L"  finalize           : %8.2f ms\n", convMs);
        prt(L"  total              : %8.2f ms  (%.2f s)\n", totalMs, totalMs / 1000.0);
        prt(L"  total cities       : %zu  (target≈%d)\n",
            result.size(), TARGET_T1 + TARGET_T2 + TARGET_T3);

        //목표 90% 미만이면 경고 (3000은 근사치).
        const int placedTotal = static_cast<int>(result.size());
        const int targetTotal = TARGET_T1 + TARGET_T2 + TARGET_T3;
        if (placedTotal < targetTotal * 0.9)
        {
            const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
            prt(warn, L"  [WARN] city count below 90%% of target — radii too large or terrain budget tight\n");
        }

        return result;
    }

    namespace
    {
        DartResult placeTier(CityTier tier, int targetCount, std::vector<CityRec>& cities, SpatialHash& hash, int& currentMaxR, std::mt19937_64& rng, const PixelCostGrid& grid, const std::vector<double>& latLut, const WaterBonusLut& waterLut, const CitySink& onPlaced)
        {
            DartResult dr{};
            if (targetCount <= 0) return dr;

            constexpr int W = PixelCostGrid::W;
            constexpr int H = PixelCostGrid::H;
            //티어별 충돌 반경 — R_T1/T2/T3 (위 파라미터 블록 참고).
            const int R = (tier == CityTier::T1) ? R_T1
                        : (tier == CityTier::T2) ? R_T2
                                                 : R_T3;
            const int maxAttempts = targetCount * MAX_ATTEMPTS_MULT;
            const double maxScore = 1.0 + WATER_PEAK[static_cast<int>(tier)];

            std::uniform_int_distribution<int> distX(0, W - 1);
            std::uniform_int_distribution<int> distY(0, H - 1);
            std::uniform_real_distribution<double> distU(0.0, 1.0);

            const Terrain* td = grid.data.get();

            //로컬 물 스캔 — (px,py) 주변 chebyshev shell을 1씩 확장하며 가장 가까운
            //  Sea/River/Lake/CityRiver/CitySea 픽셀까지의 squared Euclidean distance를 반환.
            //    - 중심이 물이면 0
            //    - LOCAL_SCAN_R 안에 물이 없으면 MAX_DSQ + 1 (호출자가 보너스 1.0 처리)
            //    - early break: r×r ≥ bestSq 시 더 가까워질 수 없으므로 종료
            //  shell 크기 = 8r 픽셀. 평균적으로 해안 픽셀은 ~10픽셀 안에서 끝남.
            auto scanForWaterSq = [td](int px, int py) noexcept -> int
            {
                constexpr int SENTINEL = WaterBonusLut::MAX_DSQ + 1;
                auto isWater = [td](int x, int y) noexcept {
                    const Terrain t = td[static_cast<std::size_t>(y) * W + x];
                    return t == Terrain::Sea
                        || t == Terrain::River
                        || t == Terrain::Lake
                        || t == Terrain::CityRiver
                        || t == Terrain::CitySea;
                };

                //중심 픽셀(호출자가 보장하지만 안전상 유지).
                if (px >= 0 && px < W && py >= 0 && py < H && isWater(px, py)) return 0;

                int bestSq = SENTINEL;

                for (int r = 1; r <= LOCAL_SCAN_R; ++r)
                {
                    if (r * r >= bestSq) break;  //더 가까워질 가능성 없음

                    const int yT = py - r;
                    const int yB = py + r;
                    const int xL = px - r;
                    const int xR = px + r;

                    //위/아래 행 (코너 포함)
                    const bool yTok = (yT >= 0);
                    const bool yBok = (yB <  H);
                    if (yTok || yBok)
                    {
                        for (int dx = -r; dx <= r; ++dx)
                        {
                            const int x = px + dx;
                            if (x < 0 || x >= W) continue;
                            const int sq = dx * dx + r * r;
                            if (sq >= bestSq) continue;
                            if (yTok && isWater(x, yT)) bestSq = sq;
                            if (sq >= bestSq) continue;
                            if (yBok && isWater(x, yB)) bestSq = sq;
                        }
                    }
                    //좌/우 열 (코너 제외)
                    const bool xLok = (xL >= 0);
                    const bool xRok = (xR <  W);
                    if (xLok || xRok)
                    {
                        for (int dy = -r + 1; dy <= r - 1; ++dy)
                        {
                            const int y = py + dy;
                            if (y < 0 || y >= H) continue;
                            const int sq = r * r + dy * dy;
                            if (sq >= bestSq) continue;
                            if (xLok && isWater(xL, y)) bestSq = sq;
                            if (sq >= bestSq) continue;
                            if (xRok && isWater(xR, y)) bestSq = sq;
                        }
                    }
                }

                return bestSq;
            };

            while (dr.placed < targetCount && dr.attempts < maxAttempts)
            {
                ++dr.attempts;
                const int px = distX(rng);
                const int py = distY(rng);

                //1. 지형 컷 (가장 빠른 reject) — 지형별 도시 입지 기본 가중치.
                //  default(=0.0) = 절대 배치 불가: Sea/River/Lake/Mountain/Polar +
                //  사전배치 영역(CityZone/CityCenter/CityRiver/CitySea). 사전배치 위로
                //  절차생성이 떨어지지 않도록 여기서 차단 → 중복 방지.
                const Terrain t = td[static_cast<std::size_t>(py) * W + px];
                double base;
                switch (t)
                {
                case Terrain::Land:                  base = 1.00; break;
                case Terrain::Monsoon:               base = 0.75; break;
                case Terrain::InsularRainforest:     base = 0.70; break;  //수라바야·세부·반둥·다낭급 2차 도시. 수도(자카르타·마닐라 등)는 PNG 사전배치 별개
                case Terrain::Subarctic:             base = 0.30; break;
                case Terrain::ContinentalRainforest: base = 0.08; break;  //아마존/콩고 내륙 — 강가 사전배치(마나우스 등)는 별개
                case Terrain::Desert:                base = 0.10; break;
                case Terrain::Tundra:                base = 0.02; break;
                default:                             base = 0.0;  break;
                }
                if (base <= 0.0)
                {
                    ++dr.rejectTerrain;
                    continue;
                }

                //2. 로컬 물 스캔 + 점수
                const int dSq = scanForWaterSq(px, py);
                const double s = base * waterLut.get(dSq, tier) * latLut[py];

                //3. 점수 비례 accept
                if (distU(rng) > s / maxScore)
                {
                    ++dr.rejectScore;
                    continue;
                }

                //4. 충돌 검사
                if (hash.conflicts(px, py, R, currentMaxR, cities))
                {
                    ++dr.rejectConflict;
                    continue;
                }

                //5. 등록
                const int idx = static_cast<int>(cities.size());
                cities.push_back(CityRec{px, py, R, tier});
                hash.insert(idx, px, py);
                if (R > currentMaxR) currentMaxR = R;
                ++dr.placed;

                if (onPlaced)
                {
                    onPlaced(CityNode{ pixelToTileCenter(px, py), tier });
                }
            }

            return dr;
        }

        //좌표 변환 — procGen 픽셀(0,0) = 패치(PATCH_X_MIN, PATCH_Y_MIN) 좌상단.
        Point3 pixelToTileCenter(int px, int py) noexcept
        {
            return Point3{
                px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2,
                py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2,
                0
            };
        }
    }
}
