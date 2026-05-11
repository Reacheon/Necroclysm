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
//
//   책임 (향후):
//     - LandmarkType + 도시별 랜드마크 매핑 (콜로세움, 빅벤 등)
//     - CityId 강타입 + 런타임 ID 부여
//     - CityIndex 공간 인덱스 (sector 쿼리)
//     - CityPolygon 추출, BCP 분할, 건물 배치 함수들
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

        seoul,
        busan,
        tokyo,
        osaka,
        beijing,
        shanghai,
        rome,
        london,
        paris,
        berlin,
        moscow,
        newYork,
        losAngeles,
        // ... PNG에 마킹한 도시들을 여기 추가
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
        //                codename                 displayName    pixelX   pixelY   climate
        PresetCity{ CityName::seoul,             "Incheon",         36797,   6314,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Seoul",         36841,   6294,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Daejeon",         36876,   6432,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Daegu",         37027,   6494,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Busan",         37095,   6582,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Gwangju",         36825,   6601,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Jeju",         36786,   6782,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Pyongyang",         36693,   6119,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Tokyo",         38360,   6522,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Sapporo",         38551,   5628,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Osaka",         37860,   6640,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Kyoto",         37896,   6604,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Fukuoka",         37234,   6777,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Beijing",         35597,   6027,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Shanghai",         36182,   7083,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Hongkong",         35298,   8111,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Taibei",         36179,   7794,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Hanoi",         34289,   8277,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Bangkok",         33673,   9150,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Jakarta",         34420,   11544,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Mumbai",         30346,   8519,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Istanbul",         25081,   5879,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Moscow",         25858,   3919,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Berlin",         23185,   4497,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Paris",         21875,   4955,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "London",         21586,   4617,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "Rome",         23105,   5780,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },
        PresetCity{ CityName::seoul,             "-",         0,   0,    worldGrid::Terrain::Land },

        // PresetCity{ CityName::busan,             "Busan",         /*?*/,   /*?*/,   worldGrid::Terrain::Land },
        // PresetCity{ CityName::tokyo,             "Tokyo",         /*?*/,   /*?*/,   worldGrid::Terrain::Monsoon },
        // PresetCity{ CityName::beijing,           "Beijing",       /*?*/,   /*?*/,   worldGrid::Terrain::Land },
        // ... 사용자가 PNG 보면서 항목 추가 후 주석 해제
    };
}
