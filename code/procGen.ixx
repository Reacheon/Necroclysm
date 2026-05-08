export module procGen;

import std;
import util;

//============================================================
// 외부 노출 — Terrain은 Sector 등에서 픽셀 색 분기용으로 필요.
//============================================================
export namespace procGen
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
}

//============================================================
// 내부 전용 타입 — 외부로 노출되면 안 됨.
//============================================================
namespace procGen
{
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

    //픽셀 좌표 (1픽셀 = 48타일). 절차적 생성 알고리즘 내부 전용.
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
    inline constexpr int TILES_PER_PIXEL = 48;//1픽셀당 실타일 수 (청크 16과 정합 위해 16×3=48)
    inline constexpr int WORLD_PIXEL_W   = 43200;
    inline constexpr int WORLD_PIXEL_H   = 21600;

    //패치(=위성 PNG 1장) 격자 — 파일 포맷 사실. 5832장 = (53-(-54)+1) × (26-(-27)+1) = 108 × 54.
    //  파일명 규칙: number = PATCH_NUMBER_BIAS + patchX + 108·patchY → worldPatch-{number:03d}.png
    inline constexpr int PATCH_X_MIN       = -54;
    inline constexpr int PATCH_X_MAX       =  53;
    inline constexpr int PATCH_Y_MIN       = -27;
    inline constexpr int PATCH_Y_MAX       =  26;
    inline constexpr int PATCH_PIXEL       = 400;    //패치 1장의 픽셀 변 (400×400)
    inline constexpr int PATCH_NUMBER_BIAS = 2971;

    //픽셀 좌표 → 실타일 좌표 원점. 좌상단 패치(-54, -27)의 (0,0)이 어떤 실타일이 되는지.
    inline constexpr int TILE_BASE_X = PATCH_X_MIN * PATCH_PIXEL * TILES_PER_PIXEL; // -1,036,800
    inline constexpr int TILE_BASE_Y = PATCH_Y_MIN * PATCH_PIXEL * TILES_PER_PIXEL; //   -518,400

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
        idle         = 0, //워커 시작 전
        loadPng      = 1, //위성 PNG 디코드 중
        placeCity    = 2, //도시 좌표 배치 중
        buildRoad    = 3, //도로망 폴리라인 생성 중
        prepareSpawn = 4, //스폰 지점 주변 섹터 사전 절차생성 중 (Phase 4 — generateWorld 외부, WorldGenScreen 워커가 처리)
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

    //============================================================
    // mmap 픽셀 그리드 — Phase 1 종료 후 진입, Phase 2 게임플레이 단일 접근점.
    //   Phase 1 동안에는 heap PixelCostGrid를 직접 사용 (전역 A* 빠름).
    //   Phase 1 끝나면 transitionToMmap이:
    //     1) heap grid 933MB를 디스크 임시 파일에 기록 (압축 없음)
    //     2) heap free
    //     3) 파일을 read-only mmap → 가상 주소만 매핑, RAM 사용 0
    //   이후 worldPixel(px, py)이 mmap된 데이터에서 페이지 폴트 기반 lazy 로드로 반환.
    //   게임 종료 시 shutdownWorldPixelMmap이 mmap 해제 + 임시 파일 삭제.
    //
    //   세션 임시 파일이라 다음 실행에는 worldPatch.cache(압축, 영구)에서 다시 풀어
    //   새 mmap 파일을 만든다.
    //============================================================

    //Phase 1 직후 호출. heapGrid를 디스크에 기록 → mmap 진입 → heapGrid는 호출자가 free.
    //  실패 시 false 반환 (이 경우 호출자가 heapGrid를 그대로 유지하는 폴백 가능).
    bool transitionToMmap(const PixelCostGrid& heapGrid);

    //게임 종료 시 호출. mmap 해제 + 임시 파일 삭제.
    void shutdownWorldPixelMmap() noexcept;

    //픽셀 1개 조회. mmap 진입 후에만 유효.
    //  범위 밖이면 Sea를 반환 (방어적 — 섹터 경계 외곽에서 조회 시 안전).
    Terrain worldPixel(int px, int py) noexcept;

    //mmap 활성 여부 — 디버그/스모크 테스트용.
    bool worldPixelMmapActive() noexcept;

    //============================================================
    // 47-piece autotile prefab 마스크 — Sector_procGenerate 페이즈 2가 사용.
    //   PNG (image/spline/shoreSpline{0..N}.png) 각각이 8×6 그리드 47 셀로
    //   land(true)/water(false) bool 마스크. 게임 시작 시 textureLoader가 PNG를
    //   순회 로드해 shoreSplineMask 채움. 로드 성공한 PNG 수가 shoreSplineVariantCount.
    //
    //   procGenerate는 (seed + rawPx + rawPy) hash로 variant 선택 → 8 이웃 land
    //   마스크 → 47 인덱스 → shoreSplineMask[variant][idx] 룩업 → 타일 dirt/water 결정.
    //   variant끼리 *변/코너 경계 패턴은 동일*하게 그려져야 인접 픽셀에서 점프 X.
    //   인덱스 매핑은 GameMaker autotile47 컨벤션 (Sector_procGenerate.cpp 참조).
    //============================================================
    inline constexpr int SHORE_TILE_SIZE     = 48;
    inline constexpr int SHORE_INDEX_COUNT   = 47;
    inline constexpr int SHORE_VARIANT_MAX   = 3;   // 시도하는 PNG 최대 개수 (실제 로드 수는 shoreSplineVariantCount)

    inline std::array<std::array<std::array<bool, SHORE_TILE_SIZE * SHORE_TILE_SIZE>, SHORE_INDEX_COUNT>, SHORE_VARIANT_MAX> shoreSplineMask{};
    inline int shoreSplineVariantCount = 0;   // 로드 성공한 variant 수 — procGenerate가 modulo에 사용
}

//============================================================
// 내부 백엔드 (export 안 함) — procGen_loadWorldGrid.cpp에서 정의, 캐시 모듈에서만 호출.
//============================================================
namespace procGen
{
    PixelCostGrid loadWorldGridFromPng(PatchLoadSink onPatch);
}
