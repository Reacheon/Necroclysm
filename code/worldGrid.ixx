export module worldGrid;

import std;
import util;
import constVar;

//============================================================
// worldGrid — 월드 픽셀 그리드 데이터·접근 단일 책임 모듈.
//   책임:
//     - 위성 PNG (5832장) 디코드 → 43200×21600 Terrain 그리드 구성
//     - 그리드 압축 캐시 (디스크 영구 저장 + stale 검증)
//     - mmap 진입 후 게임 내내 픽셀 단위 lazy 페이지 폴트 접근
//     - 47-piece autotile shore prefab 마스크 (Sector_procGenerate가 룩업)
//   사용처:
//     - worldGen: 1회 부트스트랩에 grid를 받아 도시·도로 생성에 사용
//     - Sector_procGenerate: 매 sector 절차생성마다 픽셀 색 분기·해안 autotile
//     - GUI/Map: 미니맵·월드맵 픽셀 색 표시
//     - textureLoader: shoreSplineMask 채우기 (writer)
//============================================================

//============================================================
// 외부 노출 — Terrain은 Sector 등에서 픽셀 색 분기용으로 필요.
//============================================================
export namespace worldGrid
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
// 그리드 데이터 타입 — worldGen이 placeCities/buildRoadNetwork 시그니처에서 PixelCostGrid를,
// 내부 알고리즘에서 PixelCoord를 참조하므로 export 필요. Map/Sector 등 다른 소비자는
// 보통 worldPixel()만 사용하고 이 타입들을 직접 다루지 않음.
//============================================================
export namespace worldGrid
{
    struct PixelCostGrid
    {
        static constexpr int W = WORLD_PIXEL_W;   //constVar 단일 진리원천
        static constexpr int H = WORLD_PIXEL_H;
        std::unique_ptr<Terrain[]> data;

        Terrain at(int px, int py) const noexcept
        {
            return data[static_cast<std::size_t>(py) * W + px];
        }
    };

    //픽셀 좌표 (1픽셀 = 24타일 = 1청크). 절차적 생성 알고리즘 내부 전용.
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
export namespace worldGrid
{
    //패치(=위성 PNG 1장) 파일명 그리드 BIAS. 5832장 = 108 × 54.
    //  파일명 규칙: number = PATCH_NUMBER_BIAS + patchX + 108·patchY → worldPatch-{number:03d}.png
    //  ★ TILE_PER_PIXEL / PIXEL_PER_PATCH / WORLD_PIXEL_W/H / PATCH_X/Y_MIN/MAX /
    //    TILE_BASE_X/Y 는 모두 constVar에 통합 (이 모듈에서는 import constVar로 가져와 사용).
    inline constexpr int PATCH_NUMBER_BIAS = 2971;

    //로딩 화면 미리보기 RGBA 다운샘플 해상도. 세로는 비율로 자동 계산.
    inline constexpr int PREVIEW_W = 1080;                                       // 43200 / 40
    inline constexpr int PREVIEW_H = PREVIEW_W * WORLD_PIXEL_H / WORLD_PIXEL_W;  //   540

    //onPatch(loaded, total, patchX, patchY, grid) — 패치 1장 디코드 직후 호출.
    //  grid는 그 시점까지 로드된 부분만 유효(나머지는 Sea 디폴트). 콜백 내에서
    //  방금 채워진 패치 영역 픽셀을 즉시 읽어 미리보기 점진 갱신에 사용 가능.
    using PatchLoadSink = std::function<void(int loaded, int total, int patchX, int patchY, const PixelCostGrid& grid)>;

    //공개 진입점 — worldGrid_load.cpp에서 정의.
    //  5832장 패치 PNG를 디코드해 933MB Terrain 그리드를 구성한다.
    PixelCostGrid loadWorldGrid(PatchLoadSink onPatch = {});

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
    inline constexpr int SHORE_TILE_SIZE     = 24;
    inline constexpr int SHORE_INDEX_COUNT   = 47;
    inline constexpr int SHORE_VARIANT_MAX   = 3;   // 시도하는 PNG 최대 개수 (실제 로드 수는 shoreSplineVariantCount)

    inline std::array<std::array<std::array<bool, SHORE_TILE_SIZE * SHORE_TILE_SIZE>, SHORE_INDEX_COUNT>, SHORE_VARIANT_MAX> shoreSplineMask{};
    inline int shoreSplineVariantCount = 0;   // 로드 성공한 variant 수 — procGenerate가 modulo에 사용
}
