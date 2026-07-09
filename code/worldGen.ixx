export module worldGen;

import std;
import util;
import constVar;
import worldGrid;
import city;
import worldWrap;

//============================================================
// worldGen — 월드 1회 부트스트랩 (도시 좌표 + 도로망 폴리라인 + 인카운터 사이트).
//   책임:
//     - placeCities: 약 4400개 도시 좌표 절차생성 (사전배치 + rejection sampling)
//     - buildRoadNetwork: 도시간 도로 폴리라인 생성 (절차)
//     - placeSites: 2티어 국지 도로망(피더/링크) 성장 + 교외 인카운터 사이트 도로변 배치
//     - generateWorld: 위 단계 + worldGrid PNG 로드 + mmap 진입 순차 실행
//   사용처:
//     - WorldGenScreen: 워커 스레드에서 generateWorld 실행
//     - worldSession: 결과 (WorldGenResult) 보관
//   의존: worldGrid (Terrain, PixelCostGrid, loadWorldGrid, transitionToMmap 등)
//
//   참고: 도시 내부 layout(블록 분할/내부 도로/진입점/다리)은 worldGen 단계에서
//   결정하지 않음. CityPlan 모듈이 lazy 생성 (현재 도로 세그먼트 랜덤 제거까지 구현).
//============================================================

export namespace worldGen
{
    enum class CityTier : std::uint8_t { T1, T2, T3 };

    struct CityNode
    {
        Point3 center;            //실타일 좌표
        CityTier tier;
        worldGrid::Terrain climate = worldGrid::Terrain::Land;
        //  도시 입지 기후. 절차생성 도시는 placeCities가 주변 픽셀 다수결로 결정.
        //  사전배치 도시는 PRESET_CITIES에서 매칭된 값 사용 (미매칭이면 Land).

        city::CityName codename = city::CityName::none;
        //  사전배치 도시 식별자. 매칭된 preset의 codename, 또는 절차생성/미매칭이면 none.
        //  displayName/landmark 등 도시별 메타데이터는 city::PRESET_CITIES에서 룩업.

        int bboxPx = 0, bboxPy = 0;   // footprint bbox 좌상단 픽셀 (raw, 1px=1청크)
        int bboxW = 0,  bboxH = 0;    // footprint 픽셀 크기
        //  도시 footprint 경계상자. CityPlan_build가 이 bbox로 terrain 박스를 잡고,
        //  중심에서 4-연결 flood fill로 실제 클러스터(land+물)를 도출한다. 도시는 이미
        //  terrain 픽셀(프리셋=PNG, 절차=doPaint)로 존재하므로 직사각형 분해는 불필요.
    };

    struct RoadPolyLine
    {
        std::vector<Point3> verts;  //실타일 좌표
        bool minor = false;         //2티어 국지 도로·사이트 진입 스퍼 — Sector 페인터가 좁은 폭으로 페인트
    };

    //교외 인카운터 사이트 — 도시 밖, 2티어 도로변(대형은 1티어 전용 가지 끝)에 배치되는 구조물.
    //  CitySymbol(CityPlan.ixx)과 동일 형태 — Map.ixx 심볼 파이프라인(resolveSymbol/
    //  y정렬/컬링/fog)을 그대로 재사용. 건물 Lot 연결은 후속(지금은 좌표+심볼만).
    struct SiteNode
    {
        Point3    pos;                       //footprint 좌상단 청크의 실타일 좌표 (중심 아님)
        int       w = 1, h = 1;              //footprint 청크(=픽셀) 크기
        MapSymbol symbol = MapSymbol::none;
    };

    //활성 폴리라인 글로벌 view 포인터.
    //  worldSession이 generateWorld 완료 후 worldGenResult.roads 주소로 세팅.
    //  Sector_procGenerate가 도시간 도로 페인트(15타일 asphalt, 사이드워크 없음)에
    //  사용. nullptr면 페인트 스킵 (월드젠 전 startArea 시점).
    //
    //  쓰기는 메인 스레드 1회, 읽기는 worker 스레드 read-only — 동기화 불필요.
    inline const std::vector<RoadPolyLine>* activePolyLines = nullptr;

    //활성 도시 목록 글로벌 view 포인터 — activePolyLines와 동일 패턴.
    //  worldSession이 generateWorld 완료 후 worldGenResult.cities 주소로 세팅.
    //  loadNearbySectors(비동기) / teleportPlayer(동기)가 CityPlan lazy 생성 트리거에 사용.
    //  CityId = 이 벡터의 인덱스. nullptr면 스킵 (월드젠 전 startArea 시점).
    inline const std::vector<CityNode>* activeCities = nullptr;

    //활성 사이트 목록 글로벌 view 포인터 — activeCities와 동일 패턴.
    //  Map.ixx(심볼 렌더·hover 툴팁)와 isSiteChunk(숲 clearing)가 소비.
    //  nullptr면 스킵 (월드젠 전 startArea 시점).
    inline const std::vector<SiteNode>* activeSites = nullptr;

    //generateWorld 결과 — WorldGenProgress::result에 채워짐.
    struct WorldGenResult
    {
        std::vector<CityNode> cities;
        std::vector<RoadPolyLine> roads;
        std::vector<SiteNode> sites;
    };

    //진행 단계.
    enum class GenPhase : int
    {
        idle         = 0, //워커 시작 전
        loadPng      = 1, //위성 PNG 디코드 중
        placeCity    = 2, //도시 좌표 배치 중
        buildRoad    = 3, //도로망 폴리라인 생성 중
        placeSite    = 4, //교외 인카운터 사이트 배치 중 (mmap 전환 포함)
        prepareSpawn = 5, //스폰 지점 주변 섹터 사전 절차생성 중 (외부, WorldGenScreen 워커가 처리)
        done         = 6, //모든 단계 완료(result에 채워짐)
    };

    //워커 스레드와 WorldGenScreen GUI가 공유하는 진행 상태.
    //  워커: 매 단계 시작/끝, 각 도시/도로 추가 시점에 갱신.
    //  메인(GUI): 매 프레임 스냅샷 락 잡고 읽기만.
    struct WorldGenProgress
    {
        std::atomic<GenPhase> phase{ GenPhase::idle };
        std::atomic<bool>     done { false };

        //PNG 진행 (5832장 기준)
        std::atomic<int> patchesLoadedTotal{ 0 };
        std::atomic<int> patchesLoadedDone { 0 };

        //사이트 배치 진행 (placeSites 콜백에서 증가 — GUI 서브카운트 표시용)
        std::atomic<int> sitesPlaced{ 0 };

        //도시 누적 스냅샷
        std::mutex             citiesMtx;
        std::vector<CityNode>  citiesSnap;

        //도로 누적 스냅샷
        std::mutex                  roadsMtx;
        std::vector<RoadPolyLine>   roadsSnap;

        //위성 미리보기 RGBA (worldGrid::PREVIEW_W * PREVIEW_H, R8 G8 B8 A8 little-endian).
        //  초기 alpha=0(전면 투명). 패치 1장 로드 끝날 때마다 해당 10×10 블록을
        //  alpha=0xff 색으로 갱신, 메인 스레드가 SDL_UpdateTexture로 부분 반영.
        //  미로드 영역은 alpha=0이라 BLEND 모드에서 자연스럽게 투명 처리됨.
        std::mutex                       previewMtx;
        std::vector<std::uint32_t>       previewRGBA;
        std::atomic<bool>                previewReady   { false };  //버퍼 alloc 완료 시 true
        std::atomic<int>                 previewVersion { 0     };  //패치 갱신 1번마다 +1

        //최종 결과 — done == true 직전에 채움. done 이후에는 read-only.
        std::optional<WorldGenResult>    result;
    };

    //콜백 타입. default no-op — 내부에서 출력에 영향 없음(순수성 유지).
    using CitySink = std::function<void(const CityNode&)>;
    using RoadSink = std::function<void(const RoadPolyLine&)>;
    using SiteSink = std::function<void(const SiteNode&)>;

    //placeCities는 grid를 mutate — 절차생성 도시의 폴리곤을 CityZone 픽셀로 그려 넣음.
    //  사전배치 도시는 PNG에 이미 있으니 건드리지 않음. buildRoadNetwork는 painted 결과를 봄.
    std::vector<CityNode> placeCities(std::uint64_t seed, worldGrid::PixelCostGrid& grid, CitySink onPlaced = {});

    //buildRoadNetwork — 도시간 광역 도로 폴리라인 생성.
    //  도시 진입은 cityRegion 경계로 직교 (cardinal) 진입. 도시 내부 layout 의존 없음.
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const worldGrid::PixelCostGrid& grid, const std::vector<CityNode>& cities, RoadSink onRoad = {});

    //placeSites — 2티어 국지 도로망 성장 + 교외 인카운터 사이트 배치.
    //  ① 2티어 도로 성장: 도시 경계 방사(도시 가장자리 타일 프리펜드 — CityPlan 진입
    //     등록과 정합) + 1티어 앵커 확률 발아(지역 밀도 노이즈) — 막다른 피더 또는
    //     1티어 접속 링크(루프 형성), 서브가지 깊이 2.
    //  ② 사이트는 2티어 도로변 수직 스퍼(2~4px)로 얹힘. 대형(공항/군부대/교도소/원전)만
    //     1티어 직결 직선/L/Z 가지. 지형/도시거리/사이트거리/도로침범 검사 통과 시 확정.
    //  ③ 정산: 사이트도 존치 자식도 없는 피더는 철거, 존치 피더는 마지막 수요 지점에서 트림.
    //  accept마다 폴리라인(minor=true)을 roads에 직접 append(기존 소비처가 자동 소비).
    //  worldPixel 사용 — mmap 전환 후 호출 전제.
    std::vector<SiteNode> placeSites(std::uint64_t seed, const std::vector<CityNode>& cities, std::vector<RoadPolyLine>& roads, SiteSink onSite = {}, RoadSink onRoad = {});

    //월드 골격(도시 좌표 + 도로 폴리라인 + 인카운터 사이트)을 게임 시작 1회 절차적 생성.
    //  4단계(PNG 로드 → 도시 배치 → 도로망 → 사이트 배치)를 순차 실행하면서 progress의 phase /
    //  카운터 / 누적 스냅샷 / 미리보기 RGBA를 갱신한다. 완료 시점에 progress.result에
    //  최종 결과가 들어가고 progress.done = true가 된다.
    //
    //  WorldGenScreen은 이 함수를 std::jthread로 백그라운드 실행하면서 매 프레임
    //  progress의 atomic/mutex 보호 스냅샷을 읽어 화면에 그린다. 도시 내부 도로 /
    //  타일 페인팅 / 랜덤 인카운터 등은 청크로드 시점에 지연 생성.
    //
    //  prt() 등 std::wprintf 계열은 스레드 안전이 보장되지 않으므로 워커 스레드에서는
    //  호출하지 않는다 — 각 phase 함수 내부의 prt만 사용.
    void generateWorld(std::uint64_t seed, WorldGenProgress& progress);

    //도시간 광역 도로망(activePolyLines)을 청크 셀로 래스터화한 마스크 — 숲이 도로를
    //  덮지 않도록(맵 심볼·인-월드 공통) isForestChunk가 조회. 도로 셀을 1링 dilation해
    //  3청크폭 숲 스프라이트가 도로에 겹치는 것까지 차단(도로변 clearing). 폴리라인은
    //  월드젠 후 불변 → activePolyLines 포인터 기준 1회 빌드, 이후 O(1) 조회.
    //  worker(Sector)·main(Map) 양쪽 호출 → mutex 가드. inline 지역 static은 프로그램당 1개.
    inline bool isHighwayChunk(int chunkPxX, int chunkPxY)
    {
        static std::unordered_set<std::uint64_t> mask;
        static const std::vector<RoadPolyLine>*  builtFrom = nullptr;
        static bool       built = false;
        static std::mutex mtx;

        auto key = [](int cx, int cy) noexcept -> std::uint64_t {
            return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) << 32)
                 |  static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
        };

        std::lock_guard<std::mutex> lk(mtx);
        if (!built || builtFrom != activePolyLines)
        {
            built     = true;
            builtFrom = activePolyLines;
            mask.clear();
            if (activePolyLines != nullptr)
            {
                auto floorDiv = [](int a, int b) noexcept {
                    return (a >= 0) ? (a / b) : -(((-a) + b - 1) / b);
                };
                auto stampDilated = [&](int cx, int cy) {
                    for (int oy = -1; oy <= 1; ++oy)
                    for (int ox = -1; ox <= 1; ++ox)
                        mask.insert(key(worldWrap::wrapPixelX(cx + ox), cy + oy));
                };
                //각 세그먼트를 청크 공간 4-연결 워크로 따라가며 지나는 셀 stamp (+1링).
                for (const RoadPolyLine& poly : *activePolyLines)
                    for (std::size_t i = 1; i < poly.verts.size(); ++i)
                    {
                        const Point3& a = poly.verts[i - 1];
                        const Point3& b = poly.verts[i];
                        const int ax = floorDiv(a.x - TILE_BASE_X, TILE_PER_PIXEL);
                        const int ay = floorDiv(a.y - TILE_BASE_Y, TILE_PER_PIXEL);
                        const int bx = floorDiv(b.x - TILE_BASE_X, TILE_PER_PIXEL);
                        const int by = floorDiv(b.y - TILE_BASE_Y, TILE_PER_PIXEL);
                        const int dx = std::abs(bx - ax), dy = std::abs(by - ay);
                        const int sx = (ax < bx) ? 1 : -1, sy = (ay < by) ? 1 : -1;
                        const int moves = dx + dy;
                        int x = ax, y = ay, err = dx - dy;
                        stampDilated(x, y);
                        for (int m = 0; m < moves; ++m)
                        {
                            if (err > 0) { x += sx; err -= dy * 2; }
                            else         { y += sy; err += dx * 2; }
                            stampDilated(x, y);
                        }
                    }
            }
        }
        return mask.count(key(worldWrap::wrapPixelX(chunkPxX), chunkPxY)) != 0;
    }

    //인카운터 사이트 footprint 청크 판정 — isHighwayChunk와 동일 패턴(activeSites 포인터
    //  기준 1회 빌드 마스크, mutex 가드). footprint + 1링 dilation stamp — 3청크폭 숲
    //  스프라이트가 사이트에 겹치는 것까지 차단(사이트 클리어링). isForestChunk가 양보 조회,
    //  향후 사이트 Lot이 깔릴 자리를 나무로부터 예약하는 역할 겸용.
    inline bool isSiteChunk(int chunkPxX, int chunkPxY)
    {
        static std::unordered_set<std::uint64_t> mask;
        static const std::vector<SiteNode>* builtFrom = nullptr;
        static bool       built = false;
        static std::mutex mtx;

        auto key = [](int cx, int cy) noexcept -> std::uint64_t {
            return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) << 32)
                 |  static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy));
        };

        std::lock_guard<std::mutex> lk(mtx);
        if (!built || builtFrom != activeSites)
        {
            built     = true;
            builtFrom = activeSites;
            mask.clear();
            if (activeSites != nullptr)
            {
                auto floorDiv = [](int a, int b) noexcept {
                    return (a >= 0) ? (a / b) : -(((-a) + b - 1) / b);
                };
                for (const SiteNode& s : *activeSites)
                {
                    const int px = floorDiv(s.pos.x - TILE_BASE_X, TILE_PER_PIXEL);
                    const int py = floorDiv(s.pos.y - TILE_BASE_Y, TILE_PER_PIXEL);
                    for (int oy = -1; oy <= s.h; ++oy)
                    for (int ox = -1; ox <= s.w; ++ox)
                        mask.insert(key(worldWrap::wrapPixelX(px + ox), py + oy));
                }
            }
        }
        return mask.count(key(worldWrap::wrapPixelX(chunkPxX), chunkPxY)) != 0;
    }

    //절차적 산맥 청크 판정 — 씨앗에서 사방 성장(flood-fill)해 물(바다·강)에서 막히고,
    //  씨앗과 연결된 부분만, MTN_MIN_SIZE 이상만 산으로 채택. → 산은 *구성상* 하나의
    //  4-연결 덩어리이며 물에 의해 분단되지 않는다(렌더 브리징 불필요). 위성 Mountain(Terrain)
    //  과는 isMtn/Sector에서 합집합으로 합쳐져 한 오토타일로 연결.
    //    · 후보(candidate): 기존 "지터드 시드 격자 + 각진동 반경"을 영역 후보로 재사용.
    //    · 벽: 물/도시 청크. 씨앗 청크가 물/도시면 그 씨앗은 산 없음.
    //    · 캐시: 씨앗 셀 단위 지연 계산(isHighwayChunk 패턴, mutex 가드). 시드+위성 불변이라
    //      무효화 없음. Map 렌더 스레드·Sector 워커 공유.
    //  도로 양보(isHighwayChunk)는 더 이상 안 함 — 도로는 산을 지우지 않고 터널로 관통(렌더).
    //  X 시암(±W/2)에서 영역이 끊길 수 있으나 거긴 태평양이라 실영향 작음(기존 동일).
    inline bool isMountainChunk(int chunkPxX, int chunkPxY, std::uint64_t seed)
    {
        constexpr int MTN_CELL     = 32;   // 시드 격자 한 변(청크) — 숲보다 넓게
        constexpr int MTN_R_MIN    = 5;    // 산 반경 최소(청크)
        constexpr int MTN_R_MAX    = 14;   // 산 반경 최대(청크) — +엣지워프(×1.6≈22.4) < MTN_CELL
        constexpr int MTN_FILL     = 8;   // 0..255, 셀이 산 시드 보유 확률(~9%) — 산=던전이라 희소하게
        constexpr int MTN_MIN_SIZE = 16;   // 이 미만(청크) 덩어리는 폐기 — 1px·과편 던전 방지

        const int pcx = worldWrap::wrapPixelX(chunkPxX);
        const int pcy = chunkPxY;

        //물/도시 청크인가 — 성장의 벽(산은 물을 품지도 가로지르지도 않는다).
        auto isWaterOrCity = [](int x, int y) noexcept -> bool {
            switch (worldGrid::worldPixel(worldWrap::wrapPixelX(x), y))
            {
            case worldGrid::Terrain::Sea:
            case worldGrid::Terrain::CitySea:
            case worldGrid::Terrain::River:
            case worldGrid::Terrain::Lake:
            case worldGrid::Terrain::CityRiver:
            case worldGrid::Terrain::CityZone:
            case worldGrid::Terrain::CityCenter:
                return true;
            default:
                return false;
            }
        };

        //① 빠른 컷 — 물/도시는 산 아님.
        if (isWaterOrCity(pcx, pcy)) return false;

        auto floorDiv = [](int a, int b) noexcept {
            return (a >= 0) ? (a / b) : -(((-a) + b - 1) / b);
        };
        auto hashCell = [seed](int cx, int cy) noexcept -> std::uint64_t {
            std::uint64_t h = seed ^ 0xBADA55C0FFEEULL;   // 숲과 다른 salt
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) * 0xBF58476D1CE4E5B9ULL;
            h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy)) * 0x94D049BB133111EBULL;
            h ^= h >> 31;
            return h;
        };
        auto chunkKey = [](int x, int y) noexcept -> std::uint64_t {
            return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32)
                 |  static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
        };
        //특정 셀 시드의 각진동 후보 영역 판정(기존 반경 테스트 그대로).
        auto inCandidate = [](int x, int y, int seedX, int seedY, int baseR, std::uint64_t h) noexcept -> bool {
            const double dx = static_cast<double>(x - seedX);
            const double dy = static_cast<double>(y - seedY);
            const double d2 = dx * dx + dy * dy;
            const double rMax = static_cast<double>(MTN_R_MAX) * 1.6;
            if (d2 > rMax * rMax) return false;
            const double ang = std::atan2(dy, dx);
            const double p1 = static_cast<double>((h >> 32) & 0x3ff) / 1024.0 * 6.28318530718;
            const double p2 = static_cast<double>((h >> 42) & 0x3ff) / 1024.0 * 6.28318530718;
            const double a1 = 0.18 + static_cast<double>((h >> 52) & 0x3f) / 63.0 * 0.18;
            const double a2 = 0.10 + static_cast<double>((h >> 58) & 0x3f) / 63.0 * 0.12;
            const double r  = static_cast<double>(baseR) * (1.0 + a1 * std::sin(2.0 * ang + p1) + a2 * std::sin(3.0 * ang + p2));
            return d2 < r * r;
        };

        //캐시 — 프로그램당 1개(inline 지역 static). mutex 가드(Map 렌더·Sector 워커 공유).
        static std::unordered_set<std::uint64_t> mountainSet;     // 산으로 확정된 청크
        static std::unordered_set<std::uint64_t> computedCells;   // 이미 flood-fill한 씨앗 셀
        static std::uint64_t builtSeed = 0;
        static bool          seedInit  = false;
        static std::mutex    mtx;

        std::lock_guard<std::mutex> lk(mtx);
        if (!seedInit || builtSeed != seed)   // 새 월드면 캐시 리셋
        {
            mountainSet.clear();
            computedCells.clear();
            builtSeed = seed;
            seedInit  = true;
        }

        //씨앗 셀 1개를 flood-fill해 그 산 영역을 mountainSet에 적재(미계산분만).
        auto computeCell = [&](int ccx, int ccy)
        {
            if (!computedCells.insert(chunkKey(ccx, ccy)).second) return;   // 이미 계산됨

            const std::uint64_t h = hashCell(ccx, ccy);
            if ((h & 0xff) >= static_cast<std::uint64_t>(MTN_FILL)) return;   // 시드 없는 셀

            const int seedX = ccx * MTN_CELL + static_cast<int>((h >> 8)  % MTN_CELL);
            const int seedY = ccy * MTN_CELL + static_cast<int>((h >> 16) % MTN_CELL);
            const int baseR = MTN_R_MIN + static_cast<int>((h >> 24) % (MTN_R_MAX - MTN_R_MIN + 1));
            if (isWaterOrCity(seedX, seedY)) return;   // 씨앗이 물/도시면 산 없음

            //씨앗에서 4-연결 BFS — 후보∩비물 영역의 씨앗-연결성분(키는 X 시암 정규화).
            constexpr int fdx[4] = { 0, 0, +1, -1 };
            constexpr int fdy[4] = { -1, +1, 0, 0 };
            std::unordered_set<std::uint64_t> visited;
            std::vector<std::pair<int, int>>  stack;
            visited.insert(chunkKey(worldWrap::wrapPixelX(seedX), seedY));
            stack.emplace_back(seedX, seedY);
            while (!stack.empty())
            {
                const auto [x, y] = stack.back();
                stack.pop_back();
                for (int d = 0; d < 4; ++d)
                {
                    const int nx = x + fdx[d], ny = y + fdy[d];
                    const std::uint64_t nk = chunkKey(worldWrap::wrapPixelX(nx), ny);
                    if (visited.count(nk)) continue;
                    if (!inCandidate(nx, ny, seedX, seedY, baseR, h)) continue;
                    if (isWaterOrCity(nx, ny)) continue;
                    visited.insert(nk);
                    stack.emplace_back(nx, ny);
                }
            }

            if (visited.size() < static_cast<std::size_t>(MTN_MIN_SIZE)) return;   // 과편 폐기
            for (std::uint64_t k : visited) mountainSet.insert(k);
        };

        //② 질의점 주변 3×3 씨앗 셀 계산(후보반경<MTN_CELL이라 3×3로 충분).
        const int cellX = floorDiv(pcx, MTN_CELL);
        const int cellY = floorDiv(pcy, MTN_CELL);
        for (int gy = -1; gy <= 1; ++gy)
        for (int gx = -1; gx <= 1; ++gx)
            computeCell(cellX + gx, cellY + gy);

        return mountainSet.count(chunkKey(pcx, pcy)) != 0;
    }

    //숲 청크 판정 — 절차적 (위성에 숲 없음). 전역 상태 없는 *순수 함수*:
    //  Sector_procGenerate(실타일 grass+나무)와 Map.ixx(월드맵 16-오토타일 #96~111)가
    //  같은 술어를 호출해 일관 — 산맥이 Terrain::Mountain 하나를 공유하는 것과 동일 분업.
    //
    //  모델: "지터드 시드 격자 + 해석적 반경". chunkPx = 픽셀좌표(=청크, 1:1).
    //    ① 위성 terrain 게이트 — 숲-가능 바이옴만 (도시/산/사막/물/극지 제외)
    //    ② 이웃 3×3 셀의 숲 시드를 거리 테스트. 반경은 시드별 각진동(angular harmonic)으로
    //       일그러뜨려 원이 아닌 유기적 로브 (픽셀섬 없이 매끄러운 가장자리 → 오토타일 친화).
    //    ③ 도로 청크면 양보 (isHighwayChunk) — 숲이 도시간 도로를 덮지 않게.
    //  maxReach(≈12.8) < FOREST_CELL(24) 이라 3×3 이웃이면 충분 (먼 셀 시드는 도달 불가).
    //  X 시암: 내부에서 wrapPixelX로 정규화 → 두 호출자가 같은 청크에 같은 답.
    //    (블롭은 ±W/2 경계에서 끊김 — 거긴 태평양이라 실영향 작음, 추후 보강.)
    inline bool isForestChunk(int chunkPxX, int chunkPxY, std::uint64_t seed)
    {
        constexpr int FOREST_CELL  = 24;   // 시드 격자 한 변(청크)
        constexpr int FOREST_R_MIN = 3;    // 숲 반경 최소(청크)
        constexpr int FOREST_R_MAX = 8;    // 숲 반경 최대(청크) — +엣지워프 < FOREST_CELL
        constexpr int FOREST_FILL  = 70;   // 0..255, 셀이 숲 시드 보유 확률(~27%)

        const int pcx = worldWrap::wrapPixelX(chunkPxX);
        const int pcy = chunkPxY;

        //① 숲-가능 바이옴 게이트 (위성 terrain).
        switch (worldGrid::worldPixel(pcx, pcy))
        {
        case worldGrid::Terrain::Land:
        case worldGrid::Terrain::Monsoon:
        case worldGrid::Terrain::InsularRainforest:
        case worldGrid::Terrain::ContinentalRainforest:
        case worldGrid::Terrain::Subarctic:
            break;
        default:
            return false;   // Sea/River/Lake/City*/Mountain/Desert/Tundra/Polar
        }

        auto floorDiv = [](int a, int b) noexcept {
            return (a >= 0) ? (a / b) : -(((-a) + b - 1) / b);
        };
        auto hashCell = [seed](int cx, int cy) noexcept -> std::uint64_t {
            std::uint64_t h = seed ^ 0x0F0235EEDF0E57ULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) * 0xBF58476D1CE4E5B9ULL;
            h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy)) * 0x94D049BB133111EBULL;
            h ^= h >> 31;
            return h;
        };

        //② 이웃 3×3 셀의 시드를 거리 테스트.
        const int cellX = floorDiv(pcx, FOREST_CELL);
        const int cellY = floorDiv(pcy, FOREST_CELL);
        bool found = false;
        for (int gy = -1; gy <= 1 && !found; ++gy)
        for (int gx = -1; gx <= 1 && !found; ++gx)
        {
            const int ccx = cellX + gx;
            const int ccy = cellY + gy;
            const std::uint64_t h = hashCell(ccx, ccy);
            if ((h & 0xff) >= static_cast<std::uint64_t>(FOREST_FILL)) continue;  // 이 셀엔 숲 없음

            const int seedX = ccx * FOREST_CELL + static_cast<int>((h >> 8)  % FOREST_CELL);
            const int seedY = ccy * FOREST_CELL + static_cast<int>((h >> 16) % FOREST_CELL);
            const int baseR = FOREST_R_MIN + static_cast<int>((h >> 24) % (FOREST_R_MAX - FOREST_R_MIN + 1));

            const double dx = static_cast<double>(pcx - seedX);
            const double dy = static_cast<double>(pcy - seedY);
            const double d2 = dx * dx + dy * dy;
            const double rMax = static_cast<double>(FOREST_R_MAX) * 1.6;
            if (d2 > rMax * rMax) continue;   // 빠른 컬 (각진동 최대치 밖)

            const double ang = std::atan2(dy, dx);
            const double p1 = static_cast<double>((h >> 32) & 0x3ff) / 1024.0 * 6.28318530718;
            const double p2 = static_cast<double>((h >> 42) & 0x3ff) / 1024.0 * 6.28318530718;
            const double a1 = 0.18 + static_cast<double>((h >> 52) & 0x3f) / 63.0 * 0.18;  // 0.18..0.36
            const double a2 = 0.10 + static_cast<double>((h >> 58) & 0x3f) / 63.0 * 0.12;  // 0.10..0.22
            const double r  = static_cast<double>(baseR) * (1.0 + a1 * std::sin(2.0 * ang + p1) + a2 * std::sin(3.0 * ang + p2));
            if (d2 < r * r) found = true;
        }
        if (!found) return false;

        //③ 양보 — 숲이라도 산맥/사이트/도시간 도로(1링 포함)면 비움 (우선순위 산>사이트>숲>도로).
        if (isMountainChunk(pcx, pcy, seed)) return false;
        if (isSiteChunk(pcx, pcy)) return false;
        if (isHighwayChunk(pcx, pcy)) return false;
        return true;
    }
}
