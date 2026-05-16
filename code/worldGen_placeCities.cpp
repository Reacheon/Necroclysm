module;
#include <SDL3/SDL.h>

module worldGen;

import std;
import util;

using namespace worldGrid;  // Terrain, PixelCostGrid, TILES_PER_PIXEL 등 unqualified 접근

// ════════════════════════════════════════════════════════════════════════
// placeCities — 도시 좌표 배열 절차생성 (월드 1회 부트의 1단계).
//
//   책임: 사전배치 도시(PNG 클러스터링) + 절차생성 도시(rejection sampling)를 합쳐
//        실타일 좌표 CityNode 약 4400개 반환. *grid를 mutate*: 절차생성 도시의
//        폴리곤(1~2 직사각형)을 CityZone 픽셀로 그려 넣어 Map.ixx/buildRoadNetwork/
//        Sector_procGenerate가 사전배치 도시와 동일하게 인식하도록 한다.
//
//   픽셀 좌표(1px=48타일)는 알고리즘 내부 전용, 반환값은 반드시 실타일 좌표로 변환.
//
//   통계 기반 도시 입지 모델 (로컬 검사 + rejection sampling):
//     픽셀 1개 무작위 선택 → 주변을 즉석 스캔해 가장 가까운 물까지 거리 산출
//     → 점수 계산 후 비례 확률로 accept/reject. 거리장 사전계산 없음.
//
//     점수 = baseTerrain × waterBonus(d, tier) × latitudeBand(py)
//
//     waterBonus: 해안/대하천 가까울수록 지수감쇠 보너스. 티어별 비대칭으로
//       T1(대도시) 거의 항상 물가, T3(소도시) 내륙 평지도 OK.
//
//     latitudeBand: 위도 40°에서 피크, σ=25°. py=10800이 적도(0°),
//       py=0이 북극(+90°), py=21600이 남극(-90°). 적도/극지방 자연 감산.
//
//     거리장 사전계산을 안 쓰는 이유: 도시 ~3K개 vs 픽셀 933M개로 도시 수가
//     압도적으로 적음 → 사전계산 비용(O(N))이 다트 비용(O(R²)×다트수)을 크게 초과.
//     로컬 검사가 ~10× 빠름. 통계적으론 등가.
//
//   헬퍼 분리 안 함 (CLAUDE.md): 모든 로직이 본 함수 안에 인라인. 티어별 다트는
//   placeTier 람다로 3번 호출. 향후 *2곳 이상*에서 필요해지거나 *교체 가능성*이
//   명확해지면 그때 추출.
// ════════════════════════════════════════════════════════════════════════

namespace worldGen
{
    std::vector<CityNode> placeCities(std::uint64_t seed, PixelCostGrid& grid, CitySink onPlaced)
    {
        const std::int64_t tStart = getNanoTimer();

        prt(L"[worldGen] placeCities start (seed=%llu)\n",
            static_cast<std::uint64_t>(seed));

        //══════════════════════════════════════════════════════════════════
        // 파라미터 — 모든 튜닝 포인트 집중
        //══════════════════════════════════════════════════════════════════

        //티어별 최소 간격(픽셀). 충돌 검사는 min(R_A, R_B)이므로 작은 도시는 자기 R만큼만
        //떨어지면 됨(큰 도시 옆에 위성 마을 허용). 1px ≈ 1km.
        constexpr int R_T1 = 300;   //  9,600 타일 (= 200px × 48). 대도시 간격
        constexpr int R_T2 = 100;   //  4,800 타일. 중도시 간격
        constexpr int R_T3 = 100;    //  2,400 타일. 마을 간 / 위성도시 간격
                                    //   한 도(道) 안에 마을 5~10개 정도 분포되는 값.

        //절차생성 + 사전배치 합산 목표 도시 수 (≈ 4400, 근사치).
        constexpr int TARGET_T1 = 50;
        constexpr int TARGET_T2 = 350;
        constexpr int TARGET_T3 = 4000;

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
        //  static 키워드 필요: 로컬 struct(WaterBonusLut)의 멤버 함수가 enclosing scope의
        //  배열에 runtime 인덱스로 접근하므로, static storage 보장으로 접근 합법성 확보.
        static constexpr double WATER_PEAK[3] = { 1.5, 1.0, 0.4 };   // T1, T2, T3
        static constexpr double WATER_TAU [3] = { 8.0, 14.0, 25.0 }; // 실픽셀 단위

        //══════════════════════════════════════════════════════════════════
        // 내부 타입 — placeCities 안에서만 사용
        //══════════════════════════════════════════════════════════════════

        //내부 도시 표현 — 픽셀 좌표 + 충돌 반경 + 티어 + 기후 + codename. 최종 단계에서 CityNode로 변환.
        struct CityRec
        {
            int px;
            int py;
            int radius;     //자신의 충돌 영역(픽셀)
            CityTier tier;
            Terrain climate;
            city::CityName codename;   // 사전배치 매칭된 codename, 절차생성/미매칭은 none.

            std::vector<city::CityRect> rectangles;
            //  사전배치: Phase 0의 decomposeClusterToRects 결과. 분해 실패 시 비어 있음.
            //  절차생성: Phase 4가 페인트하면서 채움. 페인트 실패 시 비어 있음.
        };

        //해안/강 보너스 LUT — squared Euclidean distance(실픽셀²)로 인덱싱.
        //  인덱스 = dSq ∈ [0, LOCAL_SCAN_R²] = [0, 2500]. 초과는 호출자가 1.0으로 처리.
        //  티어별 (peak, tau) 비대칭으로 입지 패턴 차별화:
        //    T1: peak=1.5, tau=8 px  → d=0에서 2.5x, d=15에서 ~1.2x
        //    T2: peak=1.0, tau=14 px → d=0에서 2.0x, 완만하게 1로 수렴
        //    T3: peak=0.4, tau=25 px → d=0에서 1.4x, 거의 평탄(내륙도 OK)
        //
        //  MAX_DSQ는 함수 스코프에 — MSVC가 로컬 class의 static 데이터 멤버를 금지하므로
        //  struct 안에 둘 수 없다(C2246). enclosing constexpr은 array size 등 constant
        //  expression 위치에서 로컬 class도 자유롭게 참조 가능.
        constexpr int MAX_DSQ = LOCAL_SCAN_R * LOCAL_SCAN_R;
        struct WaterBonusLut
        {
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

        //공간 해시는 util::SpatialHash 사용 (cellSize = R_T3). 충돌검사는 호출부 인라인.

        //티어별 다트 던지기 결과 누적.
        struct DartResult
        {
            int placed{};
            int attempts{};
            int rejectTerrain{};
            int rejectScore{};
            int rejectConflict{};
        };

        //══════════════════════════════════════════════════════════════════
        // 공용 람다 — 픽셀 좌표 → 실타일 좌표 변환
        //══════════════════════════════════════════════════════════════════

        //픽셀(0,0) = 패치(PATCH_X_MIN, PATCH_Y_MIN) 좌상단.
        auto pixelToTileCenter = [](int px, int py) noexcept -> Point3
        {
            return Point3{
                px * TILES_PER_PIXEL + TILE_BASE_X + TILES_PER_PIXEL / 2,
                py * TILES_PER_PIXEL + TILE_BASE_Y + TILES_PER_PIXEL / 2,
                0
            };
        };

        //══════════════════════════════════════════════════════════════════
        // Phase 0 : 사전배치 도시 추출
        //   8-connected 클러스터링(CityZone ∪ CityCenter ∪ CityRiver ∪ CitySea).
        //   1 클러스터 = 1 도시. centroid는 클러스터 안의 CityCenter 픽셀 평균(없으면 전체 평균).
        //   CityRiver/CitySea(도시 내 강·해협)도 클러스터에 포함 — 강·바다가 도시를 가로지르는
        //   경우(이스탄불·홍콩·런던 등)에도 하나의 도시로 묶이고 bbox/면적이 수역까지 정확히 반영됨.
        //══════════════════════════════════════════════════════════════════
        std::vector<CityRec> cities;
        cities.reserve(TARGET_T1 + TARGET_T2 + TARGET_T3 + 100);
        int preT1Count = 0, preT2Count = 0, preT3Count = 0;
        {
            constexpr int W = PixelCostGrid::W;
            constexpr int H = PixelCostGrid::H;
            constexpr std::size_t total = static_cast<std::size_t>(W) * H;

            //1단계: 단일 시퀀셜 스캔으로 City 픽셀 인덱스만 수집.
            //  933M 비트 visited 배열 대신 sparse set 사용(도시 픽셀은 0.001%).
            const std::int64_t t0 = getNanoTimer();

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

            const std::int64_t t1 = getNanoTimer();
            prt(L"  [extract] city pixels = %zu  (scan %.1f ms)\n",
                cityPixels.size(), (t1 - t0) / 1.0e6);

            //2단계: 클러스터링 — set의 모든 원소를 BFS로 묶음.
            std::vector<std::size_t> frontier;
            frontier.reserve(2048);

            //PRESET_CITIES 매칭 트래커 — 클러스터링 끝난 뒤 미매칭 항목을 경고로 보고.
            std::vector<bool> presetMatched(city::PRESET_CITIES.size(), false);

            //Terrain → 표시명 (디버그 로그용).
            auto climateName = [](Terrain t) noexcept -> const wchar_t* {
                switch (t) {
                    case Terrain::Land:                  return L"Land";
                    case Terrain::Tundra:                return L"Tundra";
                    case Terrain::Subarctic:             return L"Subarctic";
                    case Terrain::Monsoon:               return L"Monsoon";
                    case Terrain::InsularRainforest:     return L"InsularRainforest";
                    case Terrain::Desert:                return L"Desert";
                    case Terrain::ContinentalRainforest: return L"ContinentalRainforest";
                    case Terrain::Polar:                 return L"Polar";
                    default:                             return L"?";
                }
            };

            std::vector<std::pair<int,int>> clusterPixels;  // 4×4 분해용 클러스터 픽셀 좌표 누적
            clusterPixels.reserve(2048);

            while (!cityPixels.empty())
            {
                const std::size_t startIdx = *cityPixels.begin();
                cityPixels.erase(cityPixels.begin());

                std::int64_t sumCenterX = 0, sumCenterY = 0;
                int centerCount = 0;
                int areaCount = 0;
                std::int64_t sumX = 0, sumY = 0;
                const int sx0 = static_cast<int>(startIdx % W);
                const int sy0 = static_cast<int>(startIdx / W);
                int minX = sx0, maxX = sx0, minY = sy0, maxY = sy0;

                clusterPixels.clear();
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
                    clusterPixels.emplace_back(cx, cy);
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

                //사전배치 도시 → PRESET_CITIES 매칭. centroid 5px 이내 항목이 있으면
                //그 codename + climate 사용. 미매칭이면 codename=none, climate=Land 폴백 + 콘솔 경고.
                city::CityName matchedCodename = city::CityName::none;
                Terrain matchedClimate = Terrain::Land;
                constexpr int MATCH_THRESHOLD_PX = 5;
                for (std::size_t pi = 0; pi < city::PRESET_CITIES.size(); ++pi)
                {
                    const auto& p = city::PRESET_CITIES[pi];
                    const int dx = p.pixelX - centroidX;
                    const int dy = p.pixelY - centroidY;
                    if (dx * dx + dy * dy <= MATCH_THRESHOLD_PX * MATCH_THRESHOLD_PX)
                    {
                        matchedCodename = p.codename;
                        matchedClimate  = p.climate;
                        presetMatched[pi] = true;
                        const std::string name(p.displayName);
                        prt(L"  [match] '%hs' at pixel (%d, %d), climate=%ls\n",
                            name.c_str(), centroidX, centroidY, climateName(p.climate));
                        break;   // 첫 매치 사용 — PRESET_CITIES에 중복 좌표 없다고 가정.
                    }
                }
                if (matchedCodename == city::CityName::none)
                {
                    const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
                    prt(warn, L"  [WARN] pre-marked cluster at pixel (%d, %d) has no PRESET_CITIES entry - add it to city.ixx\n",
                        centroidX, centroidY);
                }

                // 클러스터 → 4×4+ 직사각형 분해 (사전배치 도시 layout 입력).
                //   히스토그램 max-rect 그리디. minSize=4. mask는 CityZone/CityCenter만 (계획서 룰:
                //   "강이나 바다 픽셀은 사각형 분리에 안 써도 됨"). CityRiver/CitySea는 BFS 클러스터링
                //   에서 두 직사각형을 묶는 역할이지만 decomposition에서는 제외 — 강이 가로지르는
                //   Seoul/NY/Hongkong 같은 도시도 north/south 직사각형으로 깔끔히 분리됨.
                std::vector<city::CityRect> rects;
                const int bboxW = maxX - minX + 1;
                const int bboxH = maxY - minY + 1;
                if (bboxW >= 4 && bboxH >= 4)
                {
                    // 진단용 — 클러스터 픽셀 지형 종류별 카운트. PNG 색이 의도대로
                    // CityZone/Center/River/Sea로 읽혔는지 + bbox 외 다른 잡티 없는지 확인용.
                    int countZone = 0, countCenter = 0, countRiver = 0, countSea = 0, countOther = 0;

                    std::vector<std::uint8_t> mask(static_cast<std::size_t>(bboxW) * bboxH, 0);
                    for (const auto& [cx, cy] : clusterPixels)
                    {
                        const Terrain t = grid.data[static_cast<std::size_t>(cy) * W + cx];
                        switch (t)
                        {
                        case Terrain::CityZone:   ++countZone;   break;
                        case Terrain::CityCenter: ++countCenter; break;
                        case Terrain::CityRiver:  ++countRiver;  break;
                        case Terrain::CitySea:    ++countSea;    break;
                        default:                  ++countOther;  break;
                        }
                        if (t == Terrain::CityZone || t == Terrain::CityCenter)
                        {
                            mask[static_cast<std::size_t>(cy - minY) * bboxW + (cx - minX)] = 1;
                        }
                    }
                    rects = city::decomposeClusterToRects(mask.data(), minX, minY, bboxW, bboxH, 4);
                    if (rects.empty())
                    {
                        const SDL_Color warn{ 0xff, 0xa0, 0x60, 0xff };
                        prt(warn, L"  [WARN] preset cluster at (%d, %d) bbox %dx%d failed 4x4 decomposition - layout skipped\n",
                            centroidX, centroidY, bboxW, bboxH);
                        prt(L"          cluster pixels: Zone=%d Center=%d River=%d Sea=%d Other=%d (total=%zu)\n",
                            countZone, countCenter, countRiver, countSea, countOther, clusterPixels.size());
                    }
                }

                CityRec rec{ centroidX, centroidY, R, tier, matchedClimate, matchedCodename, {} };
                rec.rectangles = std::move(rects);
                cities.push_back(std::move(rec));
            }

            //매칭되지 않은 PRESET_CITIES 항목 보고 — 좌표 오타나 PNG 마킹 누락 추정.
            for (std::size_t pi = 0; pi < city::PRESET_CITIES.size(); ++pi)
            {
                if (presetMatched[pi]) continue;
                const auto& p = city::PRESET_CITIES[pi];
                const std::string name(p.displayName);
                const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
                prt(warn, L"  [WARN] PRESET_CITIES['%hs'] at (%d, %d) has no matching PNG cluster - check coords or PNG marking\n",
                    name.c_str(), p.pixelX, p.pixelY);
            }

            const std::int64_t t2 = getNanoTimer();
            prt(L"  [extract] clusters = %zu  (BFS %.1f ms)\n",
                cities.size(), (t2 - t1) / 1.0e6);
        }
        const std::int64_t tPre = getNanoTimer();

        //Phase 0 끝난 시점의 cities.size()를 기억 — Phase 4(폴리곤 페인트)는 인덱스 이후 절차생성분만 처리.
        //  사전배치 도시는 PNG에 이미 칠해져 있어 페인트할 필요 없음.
        const std::size_t preMarkedCount = cities.size();

        //══════════════════════════════════════════════════════════════════
        // Phase 1 : 공간 해시 + 사전배치 등록
        //══════════════════════════════════════════════════════════════════
        SpatialHash hash(PixelCostGrid::W, PixelCostGrid::H, R_T3);

        for (int i = 0; i < static_cast<int>(cities.size()); ++i)
        {
            hash.insert(i, cities[i].px, cities[i].py);

            //사전배치 도시도 진행 통지 — 화면에 같이 등장하도록.
            if (onPlaced)
            {
                onPlaced(CityNode{
                    pixelToTileCenter(cities[i].px, cities[i].py),
                    cities[i].tier,
                    cities[i].climate,
                    cities[i].codename
                });
            }
        }
        const std::int64_t tHash = getNanoTimer();

        //══════════════════════════════════════════════════════════════════
        // Phase 2 : LUT 빌드 (waterBonus + latitudeBand)
        //══════════════════════════════════════════════════════════════════
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
        const std::int64_t tLut = getNanoTimer();

        //══════════════════════════════════════════════════════════════════
        // Phase 3 : 티어별 rejection sampling
        //   1. 균등 무작위 (px, py) 선택
        //   2. terrainWeight ≤ 0이면 즉시 reject (가장 빠른 컷)
        //   3. scanForWaterSq로 가장 가까운 물까지 거리² 산출
        //   4. score = base × waterBonus(dSq, tier) × latLut[py]
        //   5. accept 확률 = score / maxScore[tier]  (uniform [0,1) 비교)
        //   6. SpatialHash 충돌 검사
        //   7. 통과 시 등록 + 콜백 발사
        //
        //   maxScore[tier] = max(base) × max(waterBonus) × max(lat)
        //                  = 1.0 × (1 + WATER_PEAK[tier]) × 1.0
        //══════════════════════════════════════════════════════════════════
        std::mt19937_64 rng(seed);

        //티어별 다트 람다 — T1/T2/T3에 동일 알고리즘 적용, 3번 호출.
        auto placeTier = [&](CityTier tier, int targetCount) -> DartResult
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
                constexpr int SENTINEL = MAX_DSQ + 1;
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

            //(cx, cy) 주변 8픽셀(Moore neighborhood)에서 가장 흔한 *기후* terrain을 결정.
            //  물/Mountain/CityZone/CityCenter/CityRiver/CitySea는 투표 제외.
            //  자연 biome 후보 8종 중 최빈값. 모두 제외되면 Land 폴백.
            //  결정론: 같은 (cx, cy) → 같은 결과. 동률 시 후보 배열 앞쪽 우선.
            auto sampleClimate8 = [td](int cx, int cy) noexcept -> Terrain
            {
                int count[16]{};
                for (int dy = -1; dy <= 1; ++dy)
                {
                    const int rawY = cy + dy;
                    if (rawY < 0 || rawY >= H) continue;
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0) continue;  // 중심 제외 — 주변 8픽셀만
                        const int rawX = cx + dx;
                        const int x = ((rawX % W) + W) % W;   // X 축 wrap
                        ++count[static_cast<int>(td[static_cast<std::size_t>(rawY) * W + x])];
                    }
                }

                static constexpr Terrain CLIMATES[] = {
                    Terrain::Land,
                    Terrain::Tundra,
                    Terrain::Subarctic,
                    Terrain::Monsoon,
                    Terrain::InsularRainforest,
                    Terrain::Desert,
                    Terrain::ContinentalRainforest,
                    Terrain::Polar,
                };

                int bestCount = 0;
                Terrain best = Terrain::Land;
                for (Terrain c : CLIMATES)
                {
                    const int n = count[static_cast<int>(c)];
                    if (n > bestCount) { bestCount = n; best = c; }
                }
                return best;
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
                case Terrain::Subarctic:             base = 0.60; break;
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

                //4. 충돌 검사 — dist(A,B) < min(R_A, R_B)이면 충돌.
                //  min 룰: 큰 도시(T1)는 자기 R만큼 같은 티어와 떨어져야 하지만, 작은 도시
                //  (T3)가 T1 옆에 위성으로 붙는 건 허용. min이면 각 도시가 "자기 영역만"
                //  주장 → 큰-작은 페어는 작은 쪽 R로 결정. 검색 반경 R로 충분 (멀리 있는
                //  T1은 min(R_cand, R_T1) 안에 못 들어옴 → T3 다트가 T1의 큰 영역을
                //  훑는 낭비 제거, 10× 이상 빠름).
                bool conflictHit = false;
                hash.forEachInRadius(px, py, R, [&](int idx)
                {
                    if (conflictHit) return;
                    const CityRec& c = cities[idx];
                    const std::int64_t dx = static_cast<std::int64_t>(c.px) - px;
                    const std::int64_t dy = static_cast<std::int64_t>(c.py) - py;
                    const std::int64_t d2 = dx * dx + dy * dy;
                    const std::int64_t minD = std::min(R, c.radius);
                    if (d2 < minD * minD) conflictHit = true;
                });
                if (conflictHit)
                {
                    ++dr.rejectConflict;
                    continue;
                }

                //5. 등록 — 기후는 도시 주변 8픽셀 다수결로 결정. 절차생성 도시는 codename=none.
                const Terrain climate = sampleClimate8(px, py);
                const int idx = static_cast<int>(cities.size());
                cities.push_back(CityRec{px, py, R, tier, climate, city::CityName::none});
                hash.insert(idx, px, py);
                ++dr.placed;

                if (onPlaced)
                {
                    onPlaced(CityNode{ pixelToTileCenter(px, py), tier, climate, city::CityName::none });
                }
            }

            return dr;
        };

        const int needT1 = std::max(0, TARGET_T1 - preT1Count);
        const int needT2 = std::max(0, TARGET_T2 - preT2Count);
        const int needT3 = std::max(0, TARGET_T3 - preT3Count);

        const DartResult d1 = placeTier(CityTier::T1, needT1);
        const DartResult d2 = placeTier(CityTier::T2, needT2);
        const DartResult d3 = placeTier(CityTier::T3, needT3);

        const std::int64_t tProc = getNanoTimer();

        //══════════════════════════════════════════════════════════════════
        // Phase 4 : 절차생성 도시 영역 페인트
        //   각 절차생성 도시(인덱스 ≥ preMarkedCount)에 직사각형 1~5개 폴리곤을
        //   결정해 CityZone 픽셀로 그리드에 그린다. 사전배치 도시(PNG 클러스터)는
        //   건드리지 않음.
        //
        //   목적: Map.ixx 월드맵에 절차생성 도시도 영역으로 표시 + Sector_procGenerate
        //   /buildRoadNetwork가 사전배치와 동일한 단일 인터페이스로 처리.
        //
        //   결정론: 도시별 시드 = seed XOR (i × golden_ratio). 동일 입력 → 동일 폴리곤.
        //
        //   배치 룰 (carving 방지 + 물 1px 버퍼):
        //     - 직사각형 본체 픽셀: 전부 paintable 자연 지형이거나 자기 도시의 이전 직사각형
        //       (다른 도시 영역/Sea/River/Lake/Mountain/Polar 침범 시 reject)
        //     - 직사각형 둘레 1px 버퍼: water(Sea/River/Lake/CityRiver/CitySea) 금지
        //       (해안/강가 도시도 최소 1px 간격 유지 — 시각적 분리)
        //   → 결과 폴리곤은 항상 완전한 직사각형 합집합 (carving 없음).
        //
        //   1~5 직사각형 변동:
        //     - 첫 직사각형: 도시 중심 기준 중앙 정렬, MAX_FIRST_ATTEMPTS회 재시도
        //     - 추가 직사각형: 기존 직사각형 중 하나 골라서 NSEW 한 변에 부착, 공유변 보장
        //     - 어느 직사각형이라도 못 끼우면 종료 (그 도시는 더 작은 폴리곤으로 마무리)
        //     - 첫 직사각형도 실패 시 폴리곤 0개 → 도시 페인트 스킵 (점만 남음)
        //
        //   강 픽셀 분리(이스탄불식)는 미지원, 향후 추가 가능.
        //
        //   프로토타입 — tier별 크기/직사각형 수는 게임 플레이 보고 조정.
        //══════════════════════════════════════════════════════════════════
        {
            //tier별 직사각형 한 변 길이 분포 (픽셀). R_T* 안에 들어오는 범위.
            //  넓은 범위로 폭/높이 독립 롤 → 정사각형부터 가늘고 긴 모양까지 다양.
            auto tierRange = [](CityTier t) noexcept -> std::pair<int,int> {
                switch (t) {
                    case CityTier::T1: return {20, 50};   // 중심 정렬 first rect ±25 → maxExtent 60 안에 여유
                    case CityTier::T2: return {10, 25};
                    case CityTier::T3: return { 7, 15};
                }
                return {7, 15};
            };

            //tier별 도시 *전체* 크기 한도 — CityCenter에서 폴리곤 최외곽 픽셀까지 max 거리(픽셀).
            //  현실 도시 스케일 기준: T1 bbox ≤ 120×120 (~Beijing), T2 ≤ 60×60, T3 ≤ 30×30.
            //  체인 직사각형이 이 박스를 넘지 못하도록 후보 단계에서 reject.
            auto tierMaxExtent = [](CityTier t) noexcept -> int {
                switch (t) {
                    case CityTier::T1: return 60;
                    case CityTier::T2: return 30;
                    case CityTier::T3: return 15;
                }
                return 15;
            };

            //tier별 목표 직사각형 수 분포 [min, max]. 실제 배치는 probe 실패 시 더 적을 수 있음.
            //  T1은 자주 3~4개, T3는 단일 직사각형이 흔하도록.
            auto tierRectCount = [](CityTier t) noexcept -> std::pair<int,int> {
                switch (t) {
                    case CityTier::T1: return {2, 5};
                    case CityTier::T2: return {1, 3};
                    case CityTier::T3: return {1, 2};
                }
                return {1, 1};
            };

            //페인트 가능 자연 지형 — 직사각형 본체가 이 위에 떨어져야 함.
            auto paintable = [](Terrain t) noexcept -> bool {
                switch (t) {
                    case Terrain::Land:
                    case Terrain::Tundra:
                    case Terrain::Subarctic:
                    case Terrain::Monsoon:
                    case Terrain::InsularRainforest:
                    case Terrain::Desert:
                    case Terrain::ContinentalRainforest:
                        return true;
                    default:
                        return false;
                }
            };

            //물 — 둘레 1px 버퍼에서 금지되는 지형.
            auto isWater = [](Terrain t) noexcept -> bool {
                switch (t) {
                    case Terrain::Sea:
                    case Terrain::River:
                    case Terrain::Lake:
                    case Terrain::CityRiver:
                    case Terrain::CitySea:
                        return true;
                    default:
                        return false;
                }
            };

            //결정론 LCG (Knuth MMIX 계수) — 도시별 seed 분리, 호출자 state 갱신.
            auto roll = [](std::uint64_t& s) noexcept -> std::uint32_t {
                s = s * 6364136223846793005ULL + 1442695040888963407ULL;
                return static_cast<std::uint32_t>(s >> 32);
            };

            auto rollRange = [&](std::uint64_t& s, int lo, int hi) noexcept -> int {
                return lo + static_cast<int>(roll(s) % static_cast<std::uint32_t>(hi - lo + 1));
            };

            //로컬 직사각형 표현 — wrap 처리 전 raw 좌표 사용 (시암 근처 음수 가능).
            struct Rect { int x, y, w, h; };

            //(rawX, rawY)가 직사각형 안에 있나? wrap 무시 — 같은 좌표 공간 비교.
            auto inRect = [](int rawX, int rawY, const Rect& r) noexcept -> bool {
                return rawX >= r.x && rawX < r.x + r.w && rawY >= r.y && rawY < r.y + r.h;
            };

            //직사각형이 도시 중심(cx, cy)에서 maxExt 픽셀 이내에 완전히 들어가나?
            //  4 모서리 중 가장 먼 점이 maxExt 이하면 OK. tier별 도시 크기 캡 enforce용.
            auto withinExtent = [](const Rect& r, int cx, int cy, int maxExt) noexcept -> bool {
                const int dxMax = std::max(cx - r.x, r.x + r.w - 1 - cx);
                const int dyMax = std::max(cy - r.y, r.y + r.h - 1 - cy);
                return dxMax <= maxExt && dyMax <= maxExt;
            };

            //그리드 픽셀 읽기 — X wrap 적용, Y out-of-bounds는 호출자가 처리.
            auto readGrid = [&](int rawX, int rawY) noexcept -> Terrain {
                int x = ((rawX % PixelCostGrid::W) + PixelCostGrid::W) % PixelCostGrid::W;
                return grid.data[static_cast<std::size_t>(rawY) * PixelCostGrid::W + x];
            };

            //후보 직사각형이 본체 + 버퍼 룰을 모두 통과하면 true.
            //   본체: paintable 자연 지형 OR 자기 도시의 기존 직사각형 안
            //   버퍼(둘레 1px): water 금지 (off-world는 통과)
            auto canPlace = [&](const Rect& cand, const std::vector<Rect>& mine) noexcept -> bool {
                //본체 — Y out-of-bounds는 즉시 reject
                for (int dy = 0; dy < cand.h; ++dy) {
                    const int rawY = cand.y + dy;
                    if (rawY < 0 || rawY >= PixelCostGrid::H) return false;
                    for (int dx = 0; dx < cand.w; ++dx) {
                        const int rawX = cand.x + dx;
                        const Terrain t = readGrid(rawX, rawY);
                        if (!paintable(t)) {
                            //자기 도시의 이전 직사각형 안이면 허용 (그 픽셀은 곧 페인트됨)
                            bool inOwn = false;
                            for (const Rect& r : mine) {
                                if (inRect(rawX, rawY, r)) { inOwn = true; break; }
                            }
                            if (!inOwn) return false;
                        }
                    }
                }

                //버퍼 — 본체 둘레 1px, water 금지. off-world(Y 범위 밖)는 패스.
                auto bufferOk = [&](int rawX, int rawY) noexcept -> bool {
                    if (rawY < 0 || rawY >= PixelCostGrid::H) return true;
                    return !isWater(readGrid(rawX, rawY));
                };
                //상/하 행 (코너 포함)
                for (int dx = -1; dx <= cand.w; ++dx) {
                    if (!bufferOk(cand.x + dx, cand.y - 1))         return false;
                    if (!bufferOk(cand.x + dx, cand.y + cand.h))    return false;
                }
                //좌/우 열 (코너는 위에서 처리, 여기는 안쪽만)
                for (int dy = 0; dy < cand.h; ++dy) {
                    if (!bufferOk(cand.x - 1,        cand.y + dy)) return false;
                    if (!bufferOk(cand.x + cand.w,   cand.y + dy)) return false;
                }
                return true;
            };

            //실제 페인트 — canPlace 통과한 직사각형만 호출. 본체 픽셀 전체 CityZone으로.
            auto doPaint = [&](const Rect& r) noexcept {
                for (int dy = 0; dy < r.h; ++dy) {
                    const int rawY = r.y + dy;
                    if (rawY < 0 || rawY >= PixelCostGrid::H) continue;
                    for (int dx = 0; dx < r.w; ++dx) {
                        const int rawX = r.x + dx;
                        const int x = ((rawX % PixelCostGrid::W) + PixelCostGrid::W) % PixelCostGrid::W;
                        Terrain& cell = grid.data[static_cast<std::size_t>(rawY) * PixelCostGrid::W + x];
                        //canPlace 통과한 본체는 전부 paintable이거나 자기 이전 CityZone — 무조건 페인트.
                        cell = Terrain::CityZone;
                    }
                }
            };

            //통계
            int paintedCount = 0;
            int skippedCount = 0;
            std::size_t totalRectsPainted = 0;
            std::size_t totalAttempts = 0;

            constexpr int MAX_FIRST_ATTEMPTS      = 8;    // 첫 직사각형 재시도 한도
            constexpr int MAX_ADDITIONAL_ATTEMPTS = 24;   // 추가 직사각형 시도 총량 (성공/실패 합계)

            for (std::size_t i = preMarkedCount; i < cities.size(); ++i) {
                const CityRec& c = cities[i];
                std::uint64_t state = seed ^ (static_cast<std::uint64_t>(i) * 0x9E3779B97F4A7C15ULL);
                //LCG 초기 bit 분포 개선용 한 번 돌려서 워밍업.
                roll(state);

                const auto [lo, hi] = tierRange(c.tier);
                const auto [minR, maxR] = tierRectCount(c.tier);
                const int targetRects = rollRange(state, minR, maxR);
                const int maxExt = tierMaxExtent(c.tier);

                std::vector<Rect> mine;
                mine.reserve(5);

                //첫 직사각형 — 도시 중심 기준 중앙 정렬, 여러 크기 시도
                for (int a = 0; a < MAX_FIRST_ATTEMPTS; ++a) {
                    ++totalAttempts;
                    const int w = rollRange(state, lo, hi);
                    const int h = rollRange(state, lo, hi);
                    Rect cand{ c.px - w / 2, c.py - h / 2, w, h };
                    if (!withinExtent(cand, c.px, c.py, maxExt)) continue;
                    if (canPlace(cand, mine)) {
                        mine.push_back(cand);
                        break;
                    }
                }

                //첫 직사각형 실패 → 페인트 0개. 도시는 점으로만 남음 (CityCenter도 안 찍음).
                if (mine.empty()) {
                    ++skippedCount;
                    continue;
                }

                //추가 직사각형 — 기존 중 하나 anchor로 골라 NSEW 한 변에 부착.
                int addAttempts = 0;
                while (static_cast<int>(mine.size()) < targetRects && addAttempts < MAX_ADDITIONAL_ATTEMPTS) {
                    ++addAttempts;
                    ++totalAttempts;
                    const Rect& anchor = mine[roll(state) % mine.size()];
                    const int w = rollRange(state, lo, hi);
                    const int h = rollRange(state, lo, hi);
                    const int dir = static_cast<int>(roll(state) % 4);
                    Rect cand{};
                    cand.w = w;
                    cand.h = h;
                    switch (dir) {
                        case 0: cand.x = anchor.x + (anchor.w - w) / 2; cand.y = anchor.y - h;             break;  // N
                        case 1: cand.x = anchor.x + anchor.w;            cand.y = anchor.y + (anchor.h - h) / 2; break;  // E
                        case 2: cand.x = anchor.x + (anchor.w - w) / 2; cand.y = anchor.y + anchor.h;     break;  // S
                        case 3: cand.x = anchor.x - w;                   cand.y = anchor.y + (anchor.h - h) / 2; break;  // W
                    }
                    if (!withinExtent(cand, c.px, c.py, maxExt)) continue;
                    if (canPlace(cand, mine)) {
                        mine.push_back(cand);
                    }
                }

                //모든 accepted 직사각형 페인트
                for (const Rect& r : mine) doPaint(r);
                totalRectsPainted += mine.size();

                //  CityPlan 입력용 — mine을 CityRec.rectangles에 복사 (Rect → city::CityRect).
                //  X wrap은 후속 CityPlan 단계가 raw 좌표 그대로 처리하므로 여기서는 변환 X.
                auto& dstRects = cities[i].rectangles;
                dstRects.reserve(mine.size());
                for (const Rect& r : mine)
                {
                    dstRects.push_back(city::CityRect{ r.x, r.y, r.w, r.h });
                }

                //중심 픽셀 = CityCenter (방금 칠한 CityZone 위에만)
                if (c.py >= 0 && c.py < PixelCostGrid::H) {
                    const int cxw = ((c.px % PixelCostGrid::W) + PixelCostGrid::W) % PixelCostGrid::W;
                    Terrain& cell = grid.data[static_cast<std::size_t>(c.py) * PixelCostGrid::W + cxw];
                    if (cell == Terrain::CityZone) cell = Terrain::CityCenter;
                }
                ++paintedCount;
            }

            prt(L"  [paint] procgen cities painted = %d, skipped = %d  (%zu rects, %zu attempts)\n",
                paintedCount, skippedCount, totalRectsPainted, totalAttempts);
        }
        const std::int64_t tPaint = getNanoTimer();

        //══════════════════════════════════════════════════════════════════
        // Phase 5 : 픽셀 → 실타일 변환
        //══════════════════════════════════════════════════════════════════
        std::vector<CityNode> result;
        result.reserve(cities.size());
        for (auto& c : cities)
        {
            result.push_back(CityNode{
                pixelToTileCenter(c.px, c.py),
                c.tier,
                c.climate,
                c.codename,
                std::move(c.rectangles)
            });
        }

        const std::int64_t tDone = getNanoTimer();

        //══════════════════════════════════════════════════════════════════
        // 리포트
        //══════════════════════════════════════════════════════════════════
        const double preMs   = (tPre   - tStart) / 1.0e6;
        const double hashMs  = (tHash  - tPre  ) / 1.0e6;
        const double lutMs   = (tLut   - tHash ) / 1.0e6;
        const double procMs  = (tProc  - tLut  ) / 1.0e6;
        const double paintMs = (tPaint - tProc ) / 1.0e6;
        const double convMs  = (tDone  - tPaint) / 1.0e6;
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

        prt(L"  polygon paint      : %8.2f ms\n", paintMs);
        prt(L"  finalize           : %8.2f ms\n", convMs);
        prt(L"  total              : %8.2f ms  (%.2f s)\n", totalMs, totalMs / 1000.0);
        prt(L"  total cities       : %zu  (target≈%d)\n",
            result.size(), TARGET_T1 + TARGET_T2 + TARGET_T3);

        //목표 90% 미만이면 경고 (4400은 근사치).
        const int placedTotal = static_cast<int>(result.size());
        const int targetTotal = TARGET_T1 + TARGET_T2 + TARGET_T3;
        if (placedTotal < targetTotal * 0.9)
        {
            const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
            prt(warn, L"  [WARN] city count below 90%% of target — radii too large or terrain budget tight\n");
        }

        return result;
    }
}
