export module cityLayout;

import std;
import util;
import worldGrid;

// ════════════════════════════════════════════════════════════════════════
// cityLayout — 도시 내부 BCP 분할 + 도로/진입점/다리 결정.
//
//   책임:
//     - CityRect: 도시를 구성하는 직사각형(픽셀 좌표, 4×4 이상 보장)
//     - CityLayout: 한 도시의 모든 절차생성 결정값 (블록/도로/진입점/다리)
//     - decomposeClusterToRects: 사전배치 도시의 임의 모양 클러스터를 4×4 이상
//                                직사각형들로 분해 (계획서 보장: 항상 분해 가능)
//     - buildCityLayout: 직사각형 리스트 + 주변 grid → CityLayout. 순수함수.
//
//   현재 상태(프로토타입):
//     - BCP는 단순 균등 그리드 분할(tier별 블록 크기 분포). 본격 BSP는 후속.
//     - 직사각형 외곽 변이 water 인접하면 해당 변 전체에 해안/강가 도로 발생.
//     - 직사각형끼리 공유변이 있으면 양쪽 외곽 변에 도로 발생 (계획서 A-B-C-D 룰).
//     - 진입점: water 비인접 외곽 변의 중간점. 방향당 최소 1개 보장.
//     - 다리: 두 직사각형 사이에 water 직선 있으면 최소 1개 배치.
//
//   픽셀 좌표(1px=48타일) 입력, *결과는 실타일 좌표*. 외부 소비자(Sector_procGenerate,
//   buildRoadNetwork 진입점 라우팅)는 실타일만 다룸.
//
//   결정론: 같은 (seed, rectangles, grid) → 같은 layout. 도시별 시드는
//   buildCityLayouts가 worldSeed XOR cityIndex로 derive해서 넘김.
// ════════════════════════════════════════════════════════════════════════

export namespace cityLayout
{
    // ─── 직사각형 (픽셀 좌표) ─────────────────────────────────────────────
    // worldGrid::TILES_PER_PIXEL = 48 정합. raw 좌표 사용 — X 시암 wrap은
    // 호출자가 처리. w/h는 항상 ≥ 4 (계획서 보장).
    struct CityRect
    {
        int px = 0, py = 0;   // 좌상단 픽셀 좌표 (raw, X wrap 미적용)
        int w  = 0, h  = 0;   // 폭/높이 픽셀, ≥ 4

        constexpr int x1() const noexcept { return px + w; }  // exclusive
        constexpr int y1() const noexcept { return py + h; }
    };

    // ─── 도시 블록(BCP 잎노드) ───────────────────────────────────────────
    enum class BlockKind : std::uint8_t
    {
        Buildable,  // 건물 들어갈 자리(디폴트)
        Park,       // 공원/광장
        Plaza,      // 메인 광장(도시 중심 근처 1개 정도)
    };

    struct CityBlock
    {
        Point3    tileMin;     // 실타일 좌표, inclusive
        Point3    tileMax;     // 실타일 좌표, exclusive
        BlockKind kind = BlockKind::Buildable;
    };

    // ─── 도시 내부 도로 ──────────────────────────────────────────────────
    // cardinal only(수직 or 수평). 너비 15픽셀(=720타일) 기본이지만
    // 프로토타입은 임시로 1픽셀(48타일)부터 시작 — 본격 페인트 시점에 확장.
    enum class RoadKind : std::uint8_t
    {
        Interior,  // BCP 분할로 생긴 도시 내부 도로
        Coast,     // 직사각형 외곽 변, 바깥쪽 바다/도시바다 인접
        Riverside, // 직사각형 외곽 변, 바깥쪽 강/도시강 인접
        Boundary,  // 직사각형끼리 공유변에 의해 발생하는 외곽 도로(계획서 A-B-C-D 룰)
    };

    enum class Dir4 : std::uint8_t { N, E, S, W, None };

    struct CityRoadSegment
    {
        Point3   a;        // 실타일 좌표, 한쪽 끝
        Point3   b;        // 실타일 좌표, 반대편 끝 — a/b 중 하나는 같은 좌표축
        RoadKind kind = RoadKind::Interior;

        // 단방향 사이드워크 방향. Coast/Riverside는 도시 내부 방향으로 비대칭 페인트.
        //  Interior/Boundary는 None(대칭 21타일 = 사이드워크 3 + 아스팔트 15 + 사이드워크 3).
        //  Coast/Riverside는 N/E/S/W (도시 내부 방향): 아스팔트 15 + 사이드워크 3 (= 18타일).
        //  asphalt edge가 segment line에 정확히 일치 — water/외부 픽셀 침범 없음.
        Dir4 interiorSide = Dir4::None;
    };

    // ─── 진입점 ──────────────────────────────────────────────────────────
    // 외곽 변 중 water 비인접 부분 → 도시 밖으로 나가는 도로 시작점.
    // buildRoadNetwork가 다음 단계에서 이 좌표를 A* 타겟으로 사용.

    struct CityEntryPoint
    {
        Point3 tile;            // 실타일 좌표
        Dir4   outward = Dir4::N;  // 도시를 벗어나는 방향
    };

    // ─── 다리 ────────────────────────────────────────────────────────────
    // 두 직사각형 사이를 가로지르는 water 픽셀 직선 위에 놓이는 다리.
    // 좌표는 양 끝점(도로↔도로 잇는 라인). 실제 타일 페인트는 Sector_procGenerate.
    struct CityBridge
    {
        Point3 a;   // 한쪽 직사각형의 외곽 도로에서 출발
        Point3 b;   // 반대편 직사각형의 외곽 도로에 도착
    };

    // ─── 도시 1채의 전체 layout ──────────────────────────────────────────
    // cityIndex == invalid 면 layout 미생성(rectangles 비었거나 BCP 실패).
    inline constexpr std::uint32_t INVALID_CITY_INDEX = 0xffffffffu;

    struct CityLayout
    {
        std::uint32_t cityIndex = INVALID_CITY_INDEX;
        std::vector<CityRect>        rectangles;   // 입력 그대로 보존(디버그/시각화)
        std::vector<CityBlock>       blocks;
        std::vector<CityRoadSegment> roads;
        std::vector<CityEntryPoint>  entries;
        std::vector<CityBridge>      bridges;

        Point3 bboxMinTile{ 0, 0, 0 };   // 모든 직사각형의 union bbox (실타일, inclusive)
        Point3 bboxMaxTile{ 0, 0, 0 };   //                                      (exclusive)

        bool empty() const noexcept { return cityIndex == INVALID_CITY_INDEX; }
    };

    // ─── 클러스터 → 직사각형 분해 ─────────────────────────────────────────
    // PNG 클러스터링 결과(임의 모양의 City* 픽셀 집합)를 4×4 이상 직사각형들로 분해.
    //
    //   입력: bbox 안의 inMask[(py-py0)*W + (px-px0)] = (그 픽셀이 클러스터 소속이면 true)
    //         W = bboxW, H = bboxH.
    //         (px0, py0) = bbox 좌상단 raw 픽셀 좌표.
    //   출력: 클러스터를 완전히 덮는 (오버랩 없는) 4×4+ 직사각형 리스트.
    //         분해 실패(어떤 4×4 직사각형도 못 찾을 만큼 좁은 영역 잔재) 시 빈 리스트.
    //
    //   알고리즘: 그리디 max-rect — 매 라운드 남은 영역에서 가장 큰 직사각형 추출(히스토그램).
    //   계획서가 "전부 최소 4×4로 분해 가능"을 보장하므로 빈 리스트 = PNG 마킹 문제 신호.
    std::vector<CityRect> decomposeClusterToRects(const std::uint8_t* inMask, int bboxPxX, int bboxPxY, int bboxW, int bboxH, int minSize = 4);

    // ─── 활성 layout 글로벌 포인터 ─────────────────────────────────────────
    // worldGen이 완료되고 worldGenResult가 채워진 직후 worldSession이 세팅.
    // Sector_procGenerate가 city layout을 소비할 때 이걸 통해 접근 — 모듈 사이클 회피
    // (Sector_procGenerate가 worldGen/worldSession을 직접 import하면 cycle 발생).
    //
    //  쓰기는 worldSession에서 단일 스레드(메인) 한 번만. 읽기는 worker 스레드들에서
    //  자유롭게 — 게임플레이 시점에는 read-only라 동기화 불필요. 데이터 자체는
    //  WorldGenResult.layouts가 소유, 본 포인터는 view.
    inline const std::vector<CityLayout>* activeLayouts = nullptr;

    // ─── 직사각형 리스트 → CityLayout ─────────────────────────────────────
    // 순수함수. grid는 read-only(외곽 water 검사용).
    //
    //   입력:
    //     cityIndex  — WorldGenResult.cities 인덱스(layout.cityIndex로 기록)
    //     center     — 도시 중심 *실타일* 좌표(plaza 위치 결정용)
    //     tier       — T1/T2/T3 (BCP 블록 크기 분포 결정)
    //     rectangles — 1~N개 픽셀 좌표 직사각형, 4×4 이상
    //     grid       — heap PixelCostGrid (X wrap 처리 포함 안전 접근)
    //     citySeed   — 도시별 결정론 시드 (보통 worldSeed XOR cityIndex)
    //
    //   rectangles 비어 있으면 invalid CityLayout 반환.
    CityLayout buildCityLayout(std::uint32_t cityIndex, Point3 centerTile, int tier, const std::vector<CityRect>& rectangles, const worldGrid::PixelCostGrid& grid, std::uint64_t citySeed);
}
