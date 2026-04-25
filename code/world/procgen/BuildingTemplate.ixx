export module BuildingTemplate;

import std;
import util;

// ──────────────────────────────────────────────────────────
// 건물 템플릿 — "픽셀 종속" 제거
//   - lot(필지) 크기에 맞춰 실시간으로 크기 결정
//   - facade(도로 방향)에 문 배치, 회전 데이터 복제 없음
//   - 타입별로 크기 범위와 선호도가 다름 → 도심/외곽 믹스 자연스럽게 발생
// ──────────────────────────────────────────────────────────

export enum class BuildingType
{
    house,       // 소형 주택
    shop,        // 상가 / 편의점
    apartment,   // 아파트
    warehouse,   // 창고 / 산업
    office,      // 사무빌딩
    mall,        // 대형 쇼핑몰 (큰 lot에서만)
};

// 문이 향하는 방향 = 도로가 있는 방향
export enum class Facade : std::uint8_t { north, east, south, west };

// 도시 구역 — 포털로부터의 거리로 결정
//   core    : 도심. office/mall/shop 위주, house/warehouse 억제
//   midtown : 혼합. apartment/shop 위주
//   suburb  : 외곽. house 위주, mall 금지
export enum class Zone : std::uint8_t { core, midtown, suburb };

// 타입별 크기 범위: facade(도로변) × depth(안쪽) 기준
// facade 방향이 N/S면 width=facade, height=depth / E/W면 뒤집힘
export struct BuildingTypeSpec
{
    BuildingType type;
    int minFacade, maxFacade;
    int minDepth, maxDepth;
    int weight;           // lot에 맞는 후보들 중 선택 가중치
};

export inline const std::array<BuildingTypeSpec, 6>& getBuildingTypeSpecs()
{
    static const std::array<BuildingTypeSpec, 6> specs = { {
        { BuildingType::house,     10, 18,  9, 16, 14 },
        { BuildingType::shop,      12, 24, 12, 22,  9 },
        { BuildingType::apartment, 18, 30, 20, 34,  5 },
        { BuildingType::warehouse, 22, 40, 22, 38,  4 },
        { BuildingType::office,    22, 36, 22, 36,  4 },
        { BuildingType::mall,      38, 70, 32, 68,  2 },
    } };
    return specs;
}

// Zone별 타입 가중치 배수 (×100 정수, 0 = 완전 배제)
//   가중치는 base × zoneMultiplier / 100
//   도심/외곽 대비가 체감되도록 3배 이상의 격차를 둠
static int zoneMultiplierX100(BuildingType t, Zone z)
{
    switch (z)
    {
    case Zone::core:
        switch (t)
        {
        case BuildingType::house:     return 20;
        case BuildingType::shop:      return 200;
        case BuildingType::apartment: return 120;
        case BuildingType::warehouse: return 30;
        case BuildingType::office:    return 300;
        case BuildingType::mall:      return 400;
        }
        break;
    case Zone::midtown:
        switch (t)
        {
        case BuildingType::house:     return 80;
        case BuildingType::shop:      return 180;
        case BuildingType::apartment: return 200;
        case BuildingType::warehouse: return 80;
        case BuildingType::office:    return 100;
        case BuildingType::mall:      return 60;
        }
        break;
    case Zone::suburb:
        switch (t)
        {
        case BuildingType::house:     return 300;
        case BuildingType::shop:      return 80;
        case BuildingType::apartment: return 40;
        case BuildingType::warehouse: return 120;
        case BuildingType::office:    return 20;
        case BuildingType::mall:      return 0;  // 교외엔 초대형 쇼핑몰 금지
        }
        break;
    }
    return 100;
}

// lot 크기에 적합한 타입을 가중치 랜덤으로 선택 (zone 고려)
// 후보 없으면 house 반환 (최소 fallback)
export BuildingType pickBuildingType(int facadeLen, int depthLen, std::mt19937_64& rng,
    Zone zone = Zone::midtown, bool allowMall = true)
{
    const auto& specs = getBuildingTypeSpecs();
    struct Cand { BuildingType t; int w; };
    std::vector<Cand> cands;
    for (const auto& s : specs)
    {
        if (!allowMall && s.type == BuildingType::mall) continue;
        if (facadeLen < s.minFacade || depthLen < s.minDepth) continue;
        int mult = zoneMultiplierX100(s.type, zone);
        if (mult <= 0) continue;
        int w = (s.weight * mult + 50) / 100;
        if (w <= 0) continue;
        cands.push_back({ s.type, w });
    }
    if (cands.empty()) return BuildingType::house;
    int total = 0;
    for (auto& c : cands) total += c.w;
    int r = std::uniform_int_distribution<int>(0, total - 1)(rng);
    for (auto& c : cands)
    {
        if (r < c.w) return c.t;
        r -= c.w;
    }
    return cands.back().t;
}

// 배치된 건물 (월드 타일 좌표계)
export struct PlacedBuilding
{
    int tileX, tileY, z;        // 풋프린트 좌상단
    int w, h;                   // 실제 footprint 크기 (facade 방향 반영)
    int doorLocalX, doorLocalY; // 풋프린트 내부 문 좌표 (벽 위 한 칸)
    BuildingType type;
    Facade facade;
};

// lot (lotX, lotY, lotW, lotH)와 facade 방향에서 건물 하나 생성
//   - 타입의 facade/depth 범위를 lot 가용 공간과 교집합해 실 크기 랜덤 결정
//   - facade 쪽 가장자리에 밀착, 다른 방향은 setback(여백) 후 lot 중앙 정렬
//   - 문은 facade 변의 중앙
export PlacedBuilding generateBuildingInLot(
    int lotX, int lotY, int lotW, int lotH, int z,
    Facade facade, BuildingType type, std::mt19937_64& rng, int setback = 1)
{
    const auto& specs = getBuildingTypeSpecs();
    const BuildingTypeSpec& s = specs[(int)type];

    // facade가 N/S면 lot 가로가 facade 변. E/W면 lot 세로가 facade 변
    bool facadeIsHorizontal = (facade == Facade::north || facade == Facade::south);

    int availFacade = (facadeIsHorizontal ? lotW : lotH) - 2 * setback;
    int availDepth  = (facadeIsHorizontal ? lotH : lotW) - 2 * setback;
    availFacade = std::max(4, availFacade);
    availDepth  = std::max(4, availDepth);

    int fMin = std::min(s.minFacade, availFacade);
    int fMax = std::min(s.maxFacade, availFacade);
    if (fMax < fMin) fMax = fMin;
    int dMin = std::min(s.minDepth, availDepth);
    int dMax = std::min(s.maxDepth, availDepth);
    if (dMax < dMin) dMax = dMin;

    int fLen = (fMin < fMax) ? std::uniform_int_distribution<int>(fMin, fMax)(rng) : fMin;
    int dLen = (dMin < dMax) ? std::uniform_int_distribution<int>(dMin, dMax)(rng) : dMin;

    int bw, bh;
    if (facadeIsHorizontal) { bw = fLen; bh = dLen; }
    else                    { bw = dLen; bh = fLen; }

    int innerX = lotX + setback;
    int innerY = lotY + setback;
    int innerW = lotW - 2 * setback;
    int innerH = lotH - 2 * setback;
    if (innerW < bw) innerW = bw;
    if (innerH < bh) innerH = bh;

    int bx = 0, by = 0;
    switch (facade)
    {
    case Facade::north: // 도로 북쪽 → 건물 lot 북단에 밀착
        bx = innerX + (innerW - bw) / 2;
        by = innerY;
        break;
    case Facade::south:
        bx = innerX + (innerW - bw) / 2;
        by = innerY + (innerH - bh);
        break;
    case Facade::west:
        bx = innerX;
        by = innerY + (innerH - bh) / 2;
        break;
    case Facade::east:
        bx = innerX + (innerW - bw);
        by = innerY + (innerH - bh) / 2;
        break;
    }

    int doorLocalX = 0, doorLocalY = 0;
    switch (facade)
    {
    case Facade::north: doorLocalX = bw / 2; doorLocalY = 0;      break;
    case Facade::south: doorLocalX = bw / 2; doorLocalY = bh - 1; break;
    case Facade::west:  doorLocalX = 0;      doorLocalY = bh / 2; break;
    case Facade::east:  doorLocalX = bw - 1; doorLocalY = bh / 2; break;
    }

    return { bx, by, z, bw, bh, doorLocalX, doorLocalY, type, facade };
}
