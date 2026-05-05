export module procGen;

import std;
import util;

//============================================================
// 내부 전용 타입 — 외부로 노출되면 안 됨.
//============================================================
namespace procGen
{
    enum class Terrain : std::uint8_t
    {
        Land,
        Sea,
        River,      //강 — 폭 1~2px. 도로가 비교적 수월히 가로질러 다리 형성.
        Lake,       //호수 — 도로가 거의 가로지를 수 없는 큰 수역.
        CityZone,   //사전 마킹된 도시의 영역
        CityCenter, //사전 마킹된 도시의 중심점
        CityRiver,  //사전 마킹된 도시 내부의 강 — 도시 범위 계산에 포함, 도시 분할용 다리 자유 배치.
        CitySea,    //사전 마킹된 도시 내부의 바다(소금물) — 이스탄불·홍콩처럼 도시를 가르는 좁은 해협. 도시 범위 포함, 도로 건설 편이.
        Mountain,
        Polar,
        Tundra,
        Subarctic,
        Monsoon,
        InsularRainforest,     //해양/도서성 열대우림 — 동남아 군도. 도서 형태라 도시·도로 연결 가능
        Desert,
        ContinentalRainforest, //대륙성 열대우림 — 아마존/콩고 내륙. Monsoon보다 조밀, 절차생성 도시 거의 차단
    };

    struct PixelCostGrid
    {
        static constexpr int W = 43200;
        static constexpr int H = 21600;
        std::unique_ptr<Terrain[]> data;

        Terrain at(int px, int py) const noexcept
        {
            return data[static_cast<std::size_t>(py) * W + px];
        }
    };

    //픽셀 좌표 (1픽셀 = 50타일). 절차적 생성 알고리즘 내부 전용.
    //타일 좌표 Point3와 강타입 분리 — 함수 파라미터에서 혼용 불가.
    //(Point3는 게임 전반에서 쓰는 실타일 좌표이므로 도시/폴리라인 등 외부 데이터는 그대로 Point3 사용.)
    struct PixelCoord
    {
        int x{}, y{}, z{};

        PixelCoord() = default;
        constexpr PixelCoord(int x_, int y_, int z_) noexcept
            : x(x_), y(y_), z(z_) {}

        friend constexpr bool operator==(PixelCoord, PixelCoord) noexcept = default;
    };
}

//============================================================
// 외부 인터페이스
//============================================================
export namespace procGen
{
    inline constexpr int TILES_PER_PIXEL = 50;//1픽셀당 실타일 수
    inline constexpr int WORLD_PIXEL_W   = 43200;
    inline constexpr int WORLD_PIXEL_H   = 21600;

    //로딩 화면 미리보기 RGBA 다운샘플 해상도. 세로는 비율로 자동 계산.
    inline constexpr int PREVIEW_W = 1080;                                       // 43200 / 40
    inline constexpr int PREVIEW_H = PREVIEW_W * WORLD_PIXEL_H / WORLD_PIXEL_W;  //   540

    enum class CityTier : std::uint8_t { T1, T2, T3 };

    struct CityNode
    {
        Point3 center;     //실타일 좌표
        CityTier tier;
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
    };

    //진행 단계.
    enum class GenPhase : int
    {
        idle      = 0, //워커 시작 전
        loadPng   = 1, //위성 PNG 디코드 중
        placeCity = 2, //도시 좌표 배치 중
        buildRoad = 3, //도로망 폴리라인 생성 중
        done      = 4, //모든 단계 완료(result에 채워짐)
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

        //위성 미리보기 RGBA (PREVIEW_W * PREVIEW_H, R8 G8 B8 A8 little-endian).
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

    //onPatch(loaded, total, patchX, patchY, grid) — 패치 1장 디코드 직후 호출.
    //  grid는 그 시점까지 로드된 부분만 유효(나머지는 Sea 디폴트). 콜백 내에서
    //  방금 채워진 패치 영역 픽셀을 즉시 읽어 미리보기 점진 갱신에 사용 가능.
    using PatchLoadSink = std::function<void(int loaded, int total, int patchX, int patchY, const PixelCostGrid& grid)>;

    //공개 진입점 — procGen_worldGridCache.cpp에서 정의.
    //  캐시(map/worldPatch.cache)가 있고 PNG fingerprint가 일치하면 압축 해제로 로드,
    //  아니면 PNG 디코드 후 캐시 기록 (loadWorldGridFromPng를 내부 호출).
    PixelCostGrid loadWorldGrid(PatchLoadSink onPatch = {});
    std::vector<CityNode> placeCities(std::uint64_t seed, const PixelCostGrid& grid, CitySink onPlaced = {});
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const PixelCostGrid& grid, const std::vector<CityNode>& cities, RoadSink onRoad = {});

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

//============================================================
// 내부 백엔드 (export 안 함) — procGen_loadWorldGrid.cpp에서 정의, 캐시 모듈에서만 호출.
//============================================================
namespace procGen
{
    PixelCostGrid loadWorldGridFromPng(PatchLoadSink onPatch);
}
