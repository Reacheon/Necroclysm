export module WorldLandmark;

import std;
import util;
import BuildingTemplate;

// ──────────────────────────────────────────────────────────
// WorldLandmark — 월드맵 라벨용 건물/지점 등록소
//
//   CityGen이 건물 배치 직후 registerBuilding 으로 등록.
//   Map GUI 가 forEachIn 으로 가시영역만 빠르게 훑어 라벨 그림.
//
//   이름은 (tileX, tileY, type) 해시로 결정론적 풀-인덱싱:
//     같은 좌표는 항상 같은 이름. 세션 재시작 후에도 일관됨.
//
//   현재 도로/포탈 같은 비건물 랜드마크 추가 여지를 위해 type 외에
//   "category" 단위로 확장 가능한 구조이지만, 1차 구현은 건물만.
// ──────────────────────────────────────────────────────────

export struct Landmark
{
    int tileX = 0, tileY = 0;   // 풋프린트 좌상단 (월드 타일)
    int z = 0;
    int w = 0, h = 0;           // 풋프린트 크기 (타일)
    BuildingType type = BuildingType::house;
    std::wstring name;          // 표시용 이름. 비어 있으면 라벨 생략.
};

export class LandmarkRegistry
{
public:
    static LandmarkRegistry& ins()
    {
        static LandmarkRegistry instance;
        return instance;
    }

    // 모든 랜드마크 제거 (월드 리셋용)
    void clear()
    {
        landmarks_.clear();
    }

    // CityGen 의 건물 배치 직후 호출.
    //   같은 (tileX, tileY, z) 좌표가 이미 등록돼 있으면 무시 (재실행 안전).
    //   house 는 이름이 비어 등록 후에도 라벨 안 뜸.
    void registerBuilding(int tileX, int tileY, int z, int w, int h, BuildingType type)
    {
        for (const auto& lm : landmarks_)
        {
            if (lm.tileX == tileX && lm.tileY == tileY && lm.z == z) return;
        }
        Landmark lm;
        lm.tileX = tileX;
        lm.tileY = tileY;
        lm.z = z;
        lm.w = w;
        lm.h = h;
        lm.type = type;
        lm.name = generateName(tileX, tileY, type);
        landmarks_.push_back(std::move(lm));
    }

    // z 레이어 + 타일 박스(반열림 [min, max)) 안에 footprint 가 겹치는 랜드마크 방문.
    //   Visitor 시그니처 : void(const Landmark&)
    //   선형 스캔 — 도시당 수백~수천 건물이라도 프레임당 한 번만 돌아 충분히 빠름.
    template<typename Visitor>
    void forEachIn(int minTX, int minTY, int maxTX, int maxTY, int z, Visitor v) const
    {
        for (const auto& lm : landmarks_)
        {
            if (lm.z != z) continue;
            if (lm.tileX + lm.w <= minTX) continue;
            if (lm.tileY + lm.h <= minTY) continue;
            if (lm.tileX >= maxTX) continue;
            if (lm.tileY >= maxTY) continue;
            v(lm);
        }
    }

    std::size_t size() const { return landmarks_.size(); }

private:
    std::vector<Landmark> landmarks_;

    // 결정론적 해시 (tileX, tileY 기반). std::hash 대신 직접 mix —
    // 라이브러리 의존 + 재현성 위해.
    static std::uint32_t mixHash(int x, int y, int salt)
    {
        std::uint32_t h = 0x9E3779B9u;
        h ^= (std::uint32_t)x + 0x85EBCA6Bu + (h << 6) + (h >> 2);
        h ^= (std::uint32_t)y + 0xC2B2AE35u + (h << 6) + (h >> 2);
        h ^= (std::uint32_t)salt + 0x27D4EB2Fu + (h << 6) + (h >> 2);
        h ^= h >> 16;
        h *= 0x7FEB352Du;
        h ^= h >> 15;
        return h;
    }

    static std::wstring generateName(int tileX, int tileY, BuildingType type);
};

// 이름 풀 — 작지만 다양성 있는 후기 미국식 도시 명명.
//   배열 크기는 8~14 사이로 고정해 해시 인덱스 충돌이 너무 자주 같은 이름 반복하지 않도록.
//   prefix(좌표 해시1) + base(좌표 해시2) 조합으로 N×M 변형 가능.
namespace landmarkNamePool
{
    inline const std::array<const wchar_t*, 12> mallPrefix = {
        L"Greenfield", L"Central",  L"Westside",  L"Riverside",
        L"Pine",       L"Sunset",   L"Hilltop",   L"Eastgate",
        L"Northgate",  L"Lakeshore",L"Crossroads",L"Town"
    };
    inline const std::array<const wchar_t*, 4> mallSuffix = {
        L"Mall", L"Plaza", L"Center", L"Galleria"
    };

    inline const std::array<const wchar_t*, 12> officePrefix = {
        L"Hill",       L"Parker",   L"Riverside", L"Empire",
        L"Sentinel",   L"Beacon",   L"Liberty",   L"Quarry",
        L"Heritage",   L"Pinnacle", L"Meridian",  L"Vanguard"
    };
    inline const std::array<const wchar_t*, 4> officeSuffix = {
        L"Tower", L"Building", L"Office", L"Center"
    };

    inline const std::array<const wchar_t*, 12> aptPrefix = {
        L"Sunset",     L"Pine",     L"Highview",  L"Riverbend",
        L"Cedar",      L"Maple",    L"Birch",     L"Oakwood",
        L"Brookside",  L"Willow",   L"Stoneridge",L"Crescent"
    };
    inline const std::array<const wchar_t*, 4> aptSuffix = {
        L"Apartments", L"Heights", L"Court", L"Lofts"
    };

    inline const std::array<const wchar_t*, 12> warehousePrefix = {
        L"Depot 7",    L"Port",     L"Industry",  L"Sector",
        L"Iron",       L"Granger",  L"Maple",     L"Crate",
        L"Red Bay",    L"Foothill", L"District 4",L"Outpost"
    };
    inline const std::array<const wchar_t*, 4> warehouseSuffix = {
        L"Storage", L"Depot", L"Warehouse", L"Yard"
    };

    inline const std::array<const wchar_t*, 12> shopPrefix = {
        L"Quick",      L"Corner",   L"Hilltop",   L"Mike's",
        L"Sun",        L"Ridge",    L"Pine",      L"Halt",
        L"Gas & Go",   L"Dollar",   L"Jiffy",     L"Lucky"
    };
    inline const std::array<const wchar_t*, 4> shopSuffix = {
        L"Mart", L"Store", L"Shop", L"Market"
    };

    // house 는 라벨 안 띄움 (도시 라벨 과밀 방지). 이름 비워 반환.
}

inline std::wstring LandmarkRegistry::generateName(int tileX, int tileY, BuildingType type)
{
    using namespace landmarkNamePool;

    if (type == BuildingType::house) return L"";

    auto pick = [&](const auto& prefixArr, const auto& suffixArr) -> std::wstring
        {
            std::uint32_t hp = mixHash(tileX, tileY, 0xA1);
            std::uint32_t hs = mixHash(tileX, tileY, 0xB2);
            const wchar_t* p = prefixArr[hp % prefixArr.size()];
            const wchar_t* s = suffixArr[hs % suffixArr.size()];
            std::wstring out;
            out.reserve(32);
            out += p;
            out += L' ';
            out += s;
            return out;
        };

    switch (type)
    {
    case BuildingType::mall:      return pick(mallPrefix,      mallSuffix);
    case BuildingType::office:    return pick(officePrefix,    officeSuffix);
    case BuildingType::apartment: return pick(aptPrefix,       aptSuffix);
    case BuildingType::warehouse: return pick(warehousePrefix, warehouseSuffix);
    case BuildingType::shop:      return pick(shopPrefix,      shopSuffix);
    default: return L"";
    }
}
