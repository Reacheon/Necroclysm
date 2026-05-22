export module worldGen;

import std;
import util;
import constVar;
import worldGrid;
import city;

//============================================================
// worldGen — 월드 1회 부트스트랩 (도시 좌표 + 도로망 폴리라인).
//   책임:
//     - placeCities: 약 4400개 도시 좌표 절차생성 (사전배치 + rejection sampling)
//     - buildRoadNetwork: 도시간 도로 폴리라인 생성 (절차)
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

        std::vector<city::CityRect> rectangles;
        //  도시의 직사각형 분해 결과(픽셀 좌표). 절차생성 도시는 placeCities Phase 4에서
        //  쌓은 1~5개 직사각형 그대로, 사전배치 도시는 PNG 클러스터에서 역분해된 1~N개.
        //  CityPlan_build가 이 rectangles를 입력으로 받아 도시 내부 layout 생성.
    };

    struct RoadPolyLine
    {
        std::vector<Point3> verts;  //실타일 좌표
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

    //generateWorld 결과 — WorldGenProgress::result에 채워짐.
    struct WorldGenResult
    {
        std::vector<CityNode> cities;
        std::vector<RoadPolyLine> roads;
    };

    //진행 단계.
    enum class GenPhase : int
    {
        idle         = 0, //워커 시작 전
        loadPng      = 1, //위성 PNG 디코드 중
        placeCity    = 2, //도시 좌표 배치 중
        buildRoad    = 3, //도로망 폴리라인 생성 중
        prepareSpawn = 4, //스폰 지점 주변 섹터 사전 절차생성 중 (외부, WorldGenScreen 워커가 처리)
        done         = 5, //모든 단계 완료(result에 채워짐)
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

    //placeCities는 grid를 mutate — 절차생성 도시의 폴리곤을 CityZone 픽셀로 그려 넣음.
    //  사전배치 도시는 PNG에 이미 있으니 건드리지 않음. buildRoadNetwork는 painted 결과를 봄.
    std::vector<CityNode> placeCities(std::uint64_t seed, worldGrid::PixelCostGrid& grid, CitySink onPlaced = {});

    //buildRoadNetwork — 도시간 광역 도로 폴리라인 생성.
    //  도시 진입은 cityRegion 경계로 직교 (cardinal) 진입. 도시 내부 layout 의존 없음.
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const worldGrid::PixelCostGrid& grid, const std::vector<CityNode>& cities, RoadSink onRoad = {});

    //월드 골격(도시 좌표 + 도로 폴리라인)을 게임 시작 1회 절차적 생성.
    //  3단계(PNG 로드 → 도시 배치 → 도로망)를 순차 실행하면서 progress의 phase /
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
}
