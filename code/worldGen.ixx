export module worldGen;

import std;
import util;
import worldGrid;
import city;
import cityLayout;

//============================================================
// worldGen — 월드 1회 부트스트랩 (도시 좌표 + 도로망 폴리라인).
//   책임:
//     - placeCities: 3000개 도시 좌표 절차생성 (사전배치 + rejection sampling)
//     - buildRoadNetwork: Gabriel 그래프 + hierarchical bidirectional A*로 도로 폴리라인
//     - generateWorld: 위 단계 + worldGrid PNG 로드 + mmap 진입 순차 실행
//   사용처:
//     - WorldGenScreen: 워커 스레드에서 generateWorld 실행
//     - worldSession: 결과 (WorldGenResult) 보관
//   의존: worldGrid (Terrain, PixelCostGrid, loadWorldGrid, transitionToMmap 등)
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

        std::vector<cityLayout::CityRect> rectangles;
        //  도시의 직사각형 분해 결과(픽셀 좌표). 절차생성 도시는 placeCities Phase 4에서
        //  쌓은 1~5개 직사각형 그대로, 사전배치 도시는 PNG 클러스터에서 역분해된 1~N개.
        //  비어 있으면 buildCityLayouts가 layout 생략(점만 남는 도시).
    };

    struct RoadPolyLine
    {
        std::vector<Point3> verts;  //실타일 좌표
    };

    //generateWorld 결과 — WorldGenProgress::result에 채워짐.
    struct WorldGenResult
    {
        std::vector<CityNode> cities;
        std::vector<RoadPolyLine> roads;
        std::vector<cityLayout::CityLayout> layouts;
        //  도시별 layout. cities와 1:1 대응 (인덱스 = cityIndex).
        //  rectangles 비어 있는 도시는 layout도 비어 있음 (점만 남음).
    };

    //진행 단계.
    enum class GenPhase : int
    {
        idle         = 0, //워커 시작 전
        loadPng      = 1, //위성 PNG 디코드 중
        placeCity    = 2, //도시 좌표 배치 중
        cityLayout   = 3, //도시 내부 BCP/도로/진입점 계산 중
        buildRoad    = 4, //도로망 폴리라인 생성 중
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

        //도시 누적 스냅샷
        std::mutex             citiesMtx;
        std::vector<CityNode>  citiesSnap;

        //도로 누적 스냅샷
        std::mutex                  roadsMtx;
        std::vector<RoadPolyLine>   roadsSnap;

        //도시 layout 진행도 — 병렬 처리라 카운터만 필요. 스냅샷은 미사용(데이터 크기 큼).
        std::atomic<int> layoutsDone { 0 };
        std::atomic<int> layoutsTotal{ 0 };

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
    using CitySink   = std::function<void(const CityNode&)>;
    using RoadSink   = std::function<void(const RoadPolyLine&)>;
    using LayoutSink = std::function<void(std::uint32_t cityIndex)>;
    //  layout sink는 layout 자체를 넘기지 않음 — 데이터 크기가 크고 결과는 layouts[idx]에
    //  직접 슬롯 쓰기로 끝남. 콜백은 진행도 카운터 증가만 담당.

    //placeCities는 grid를 mutate — 절차생성 도시의 폴리곤을 CityZone 픽셀로 그려 넣음.
    //  사전배치 도시는 PNG에 이미 있으니 건드리지 않음. buildRoadNetwork는 painted 결과를 봄.
    std::vector<CityNode> placeCities(std::uint64_t seed, worldGrid::PixelCostGrid& grid, CitySink onPlaced = {});

    //도시별 layout(BCP 블록, 도로, 진입점, 다리)을 ThreadPool로 병렬 계산.
    //  cities[i].rectangles가 비어 있으면 layouts[i]도 빈 CityLayout으로 채움.
    //  cities 순서와 layouts 순서가 1:1 대응 — buildRoadNetwork가 같은 인덱스로 참조 가능.
    std::vector<cityLayout::CityLayout> buildCityLayouts(std::uint64_t seed, const worldGrid::PixelCostGrid& grid, const std::vector<CityNode>& cities, LayoutSink onLayout = {});

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
