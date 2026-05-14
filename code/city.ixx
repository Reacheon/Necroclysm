export module city;

import std;
import worldGrid;

// ════════════════════════════════════════════════════════════════════════
// city — 도시 정체성 및 사전배치 카탈로그.
//
//   책임 (현재):
//     - CityName: 사전배치 도시 codename enum (소스코드 식별자)
//     - PresetCity: 사전배치 도시 메타데이터 (좌표, 이름, 기후)
//     - PRESET_CITIES: PNG에 마킹한 ~40개 도시 하드코딩 테이블
//     - CityId: 런타임 도시 식별자 강타입 (worldGenResult.cities 인덱스 래퍼)
//     - CityRect + decomposeClusterToRects: 도시 픽셀 footprint 직사각형 분해
//
//   책임 (향후):
//     - LandmarkType + 도시별 랜드마크 매핑 (콜로세움, 빅벤 등)
//     - CityIndex 공간 인덱스 (sector 쿼리)
//     - 건물 배치 함수들 (도시 내부 BCP/도로/블록은 CityPlan 모듈이 담당)
//
//   의존성: worldGrid (Terrain enum). worldGen 아님 — city는 worldGen *옆에서*
//   도시 의미를 책임. worldGen이 city를 import (CityNode가 codename 필드 가짐).
//   사이클 없음.
//
//   매칭 룰: placeCities Phase 0의 BFS 클러스터링이 끝난 직후, 클러스터 centroid가
//   PRESET_CITIES의 어떤 항목과 5px 이내면 매칭으로 인정 → CityNode.codename = 그것.
//   기후도 preset에서 받아옴 (사용자가 수동 지정한 값). 미매칭 클러스터는 codename=none,
//   기후=Land (디폴트 placeholder).
// ════════════════════════════════════════════════════════════════════════

export namespace city
{
    // 사전배치 도시 codename — 소스코드에서 특정 도시를 지칭 (랜드마크/퀘스트/UI 분기).
    // 절차생성 도시는 모두 none.
    //
    // 추가 룰: PRESET_CITIES 테이블에 항목 추가할 때마다 여기에도 항목 추가.
    // enum 값은 자동 증가 (none=0부터). 중간 삽입은 가능하지만 ABI에 영향 없음.
    enum class CityName : std::uint16_t
    {
        none = 0,

        incheon,
        seoul,
        daejeon,
        daegu,
        busan,
        gwangju,
        jeju,
        pyongyang,
        tokyo,
        sapporo,
        osaka,
        kyoto,
        fukuoka,
        beijing,
        shanghai,
        hongkong,
        taibei,
        hanoi,
        bangkok,
        jakarta,
        mumbai,
        istanbul,
        cairo,
        johannesburg,
        moscow,
        berlin,
        paris,
        london,
        rome,
        oslo,
        stockholm,
        helsinki,
        madrid,
        barcelona,
        toronto,
        chicago,
        newYork,
        washington,
        houston,
        denver,
        lasVegas,
        losAngeles,
        mexicoCity,
        saoPaulo,
        rovaniemi,
        oulu,
    };

    struct PresetCity
    {
        CityName codename;
        std::string_view displayName;     // UI 표시명 ("Seoul"). 다국어 표기는 향후 sysStr 매핑.
        int pixelX, pixelY;               // PNG의 CityCenter(적색) 픽셀 좌표. 좌상단 패치 (0,0).
        worldGrid::Terrain climate;       // 도시 기후 (수동 지정). placeCities가 그대로 사용.
    };

    // 하드코딩 테이블 — PNG에 마킹한 도시들. 사용자가 PNG 보면서 좌표/이름/기후 채움.
    //   픽셀 좌표 계산: pixelX = (tileX - TILE_BASE_X) / TILES_PER_PIXEL
    //                  TILE_BASE_X = -54 * 400 * 48 = -1,036,800
    //                  TILES_PER_PIXEL = 48
    //   예: Seoul SPAWN_DEFAULT 타일 (731544, -216312) → 픽셀 (36840, 6293).
    //
    //   매칭 정확도: PNG의 CityCenter 적색 픽셀 좌표를 정확히 입력하면 BFS centroid와
    //   1픽셀 이내로 매칭. 다수 CityCenter 픽셀 클러스터면 centroid가 평균 위치라
    //   약간 어긋날 수 있음 — placeCities는 5px threshold로 매칭 허용.
    //
    //   기후 enum: worldGrid::Terrain 그대로 사용.
    //     Land (온대 기본), Tundra, Subarctic, Monsoon, InsularRainforest,
    //     ContinentalRainforest, Desert, Polar.
    inline constexpr std::array PRESET_CITIES = {
        //          codename                    displayName       pixelX   pixelY   climate
        PresetCity{ CityName::incheon,          "Incheon",         36797,   6314,   worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,            "Seoul",           36841,   6294,   worldGrid::Terrain::Land },
        PresetCity{ CityName::daejeon,          "Daejeon",         36876,   6432,   worldGrid::Terrain::Land },
        PresetCity{ CityName::daegu,            "Daegu",           37027,   6494,   worldGrid::Terrain::Land },
        PresetCity{ CityName::busan,            "Busan",           37095,   6582,   worldGrid::Terrain::Land },
        PresetCity{ CityName::gwangju,          "Gwangju",         36825,   6601,   worldGrid::Terrain::Land },
        PresetCity{ CityName::jeju,             "Jeju",            36786,   6782,   worldGrid::Terrain::Land },
        PresetCity{ CityName::pyongyang,        "Pyongyang",       36693,   6119,   worldGrid::Terrain::Land },
        PresetCity{ CityName::tokyo,            "Tokyo",           38360,   6522,   worldGrid::Terrain::Land },
        PresetCity{ CityName::sapporo,          "Sapporo",         38551,   5628,   worldGrid::Terrain::Land },
        PresetCity{ CityName::osaka,            "Osaka",           37860,   6640,   worldGrid::Terrain::Land },
        PresetCity{ CityName::kyoto,            "Kyoto",           37896,   6604,   worldGrid::Terrain::Land },
        PresetCity{ CityName::fukuoka,          "Fukuoka",         37234,   6769,   worldGrid::Terrain::Land },
        PresetCity{ CityName::beijing,          "Beijing",         35597,   6027,   worldGrid::Terrain::Land },
        PresetCity{ CityName::shanghai,         "Shanghai",        36182,   7083,   worldGrid::Terrain::Land },
        PresetCity{ CityName::hongkong,         "Hongkong",        35298,   8111,   worldGrid::Terrain::Land },
        PresetCity{ CityName::taibei,           "Taibei",          36179,   7794,   worldGrid::Terrain::Land },
        PresetCity{ CityName::hanoi,            "Hanoi",           34289,   8277,   worldGrid::Terrain::Land },
        PresetCity{ CityName::bangkok,          "Bangkok",         33673,   9150,   worldGrid::Terrain::InsularRainforest },
        PresetCity{ CityName::jakarta,          "Jakarta",         34420,  11544,   worldGrid::Terrain::InsularRainforest },
        PresetCity{ CityName::mumbai,           "Mumbai",          30346,   8519,   worldGrid::Terrain::Monsoon },
        PresetCity{ CityName::istanbul,         "Istanbul",        25081,   5869,   worldGrid::Terrain::Land },
        PresetCity{ CityName::cairo,            "Cairo",           25347,   7187,   worldGrid::Terrain::Land },
        PresetCity{ CityName::johannesburg,     "Johannesburg",    24954,  13937,   worldGrid::Terrain::Land },
        PresetCity{ CityName::moscow,           "Moscow",          25858,   3919,   worldGrid::Terrain::Land },
        PresetCity{ CityName::berlin,           "Berlin",          23185,   4497,   worldGrid::Terrain::Land },
        PresetCity{ CityName::paris,            "Paris",           21875,   4955,   worldGrid::Terrain::Land },
        PresetCity{ CityName::london,           "London",          21586,   4617,   worldGrid::Terrain::Land },
        PresetCity{ CityName::rome,             "Rome",            23105,   5780,   worldGrid::Terrain::Land },
        PresetCity{ CityName::oslo,             "Oslo",            22891,   3658,   worldGrid::Terrain::Subarctic },
        PresetCity{ CityName::stockholm,        "Stockholm",       23790,   3680,   worldGrid::Terrain::Subarctic },
        PresetCity{ CityName::helsinki,         "Helsinki",        24593,   3581,   worldGrid::Terrain::Land },
        PresetCity{ CityName::madrid,           "Madrid",          21156,   5947,   worldGrid::Terrain::Land },
        PresetCity{ CityName::barcelona,        "Barcelona",       21849,   5838,   worldGrid::Terrain::Land },
        PresetCity{ CityName::toronto,          "Toronto",         12072,   5553,   worldGrid::Terrain::Land },
        PresetCity{ CityName::chicago,          "Chicago",         11080,   5783,   worldGrid::Terrain::Land },
        PresetCity{ CityName::newYork,          "New York",        12734,   5918,   worldGrid::Terrain::Land },
        PresetCity{ CityName::washington,       "Washington",      12366,   6135,   worldGrid::Terrain::Land },
        PresetCity{ CityName::houston,          "Houston",         10158,   7214,   worldGrid::Terrain::Land },
        PresetCity{ CityName::denver,           "Denver",           9005,   6072,   worldGrid::Terrain::Land },
        PresetCity{ CityName::lasVegas,         "Las Vegas",        7755,   6457,   worldGrid::Terrain::Desert },
        PresetCity{ CityName::losAngeles,       "Los Angeles",      7387,   6705,   worldGrid::Terrain::Land },
        PresetCity{ CityName::mexicoCity,       "Mexico City",      9711,   8475,   worldGrid::Terrain::Land },
        PresetCity{ CityName::saoPaulo,         "Sao Paulo",       16005,  13623,   worldGrid::Terrain::Land },
        PresetCity{ CityName::rovaniemi,         "Rovaniemi",       24703,  2816,   worldGrid::Terrain::Land },
        PresetCity{ CityName::oulu,         "Oulu",       24662,  2971,   worldGrid::Terrain::Land },
    };

    // ─── 런타임 도시 식별자 ───────────────────────────────────────────────
    // worldGenResult.cities 인덱스의 강타입 래퍼. CityPlanCache 키 / CityIndex 조회에 사용.
    // enum class라 정수·다른 인덱스와 혼용 불가. std::hash<enum>가 자동 지원 → 맵 키로 바로 사용.
    enum class CityId : std::uint32_t {};

    // ─── 직사각형 (픽셀 좌표) ─────────────────────────────────────────────
    // 도시를 구성하는 직사각형. w/h는 항상 ≥ minSize (계획서 보장).
    // 픽셀 좌표(1px=48타일), raw — X 시암 wrap은 호출자가 처리.
    struct CityRect
    {
        int px = 0, py = 0;   // 좌상단 픽셀 좌표 (raw)
        int w  = 0, h  = 0;   // 폭/높이 픽셀

        constexpr int x1() const noexcept { return px + w; }  // exclusive
        constexpr int y1() const noexcept { return py + h; }
    };

    // ─── 클러스터 → 직사각형 분해 ─────────────────────────────────────────
    // PNG 클러스터링 결과(임의 모양의 City* 픽셀 집합)를 minSize 이상 직사각형들로 분해.
    //   입력: inMask[(py-py0)*bboxW + (px-px0)] = (그 픽셀이 클러스터 소속이면 1)
    //         (bboxPxX, bboxPxY) = bbox 좌상단 raw 픽셀 좌표.
    //   출력: 클러스터를 완전히 덮는 (오버랩 없는) 직사각형 리스트. 분해 실패 시 빈 리스트.
    //   알고리즘: 수평 슬랩 분해 → 실패 시 백트래킹 폴백. 결정론적. 정의는 city_decompose.cpp.
    std::vector<CityRect> decomposeClusterToRects(const std::uint8_t* inMask, int bboxPxX, int bboxPxY, int bboxW, int bboxH, int minSize = 4);
}
