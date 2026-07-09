module;
#include <SDL3/SDL.h>

export module Map;

import std;
import util;
import constVar;
import GUI;
import Sprite;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import globalTime;
import connectGroupExtraIndex;
import autotile47Index;
import checkCursor;
import Player;
import Entity;
import World;
import TileData;
import worldGrid;
import worldGen;
import worldWrap;
import city;
import CityPlan;
import worldSession;
import mapDiscovery;
import MapPin;

// ════════════════════════════════════════════════════════════════════════
// Map — 풀스크린 월드맵 (고전 JRPG 월드맵 스타일).
//
//   구글지도식 무제한 줌을 폐기하고 "1청크 = 1심볼" 타일 기반으로 재작성.
//   레이어:
//     ① 베이스 지형 — tileset.png. 청크 1개당 타일 1개(잔디/해수/담수). 배치 렌더. 도시 영역도
//                     잔디 — 도시는 ③-b 도로·다리 + ④ 건물 심볼로만 드러난다.
//     ② 산 심볼     — worldGrid::Terrain::Mountain. 47-piece 블롭 오토타일
//                     (autotile47Index)로 auto47Mountain.png(16px=1청크) 중 선택.
//     ③ 도로 심볼   — ③-a 외부 도로(activePolyLines 래스터)는 autotile(직선/코너/T/십자) mapset1by1로.
//                     도시↔도시 연결 도로는 어느 CityPlan에도 안 들어가므로 폴리라인을 직접 청크
//                     셀로 래스터화. ③-b 도시 내부 도로·다리(CityPlan.roadCells). 둘 다 *미발견
//                     청크여도 항상 표시*(전세계 골격/정찰지도), 단 미발견은 어둡게(fog).
//     ④ 건물 심볼   — CityPlan.symbols → mapset1by1(1x1) / mapset2by2(2x1·1x2·2x2) / mapset3by3(3x3). 발견=실제 종류,
//                     미발견=footprint별 "?건물" placeholder(resolveUnknownSymbol) + 어둡게(fog).
//                     ④-b 교외 사이트(worldGen::activeSites)도 같은 규약으로 drawSites가 그림.
//
//   좌표계는 "픽셀=청크"(worldPixel 인덱스, 0-base). 1픽셀=1청크=24타일.
//     pixelX = (tileX - TILE_BASE_X) / TILE_PER_PIXEL,  X는 원기둥 wrap.
//   카메라 centerPX/PY(실수 픽셀) + 이산 줌(chunkPx = 청크 1개의 화면 픽셀).
//
//   심볼 데이터는 "이미 생성된(캐시된) 도시"에서만(peek). 지형(산/물)은 전 세계.
//
//   §1 설정/팔레트  §2 카메라  §3 심볼 매핑  §4 렌더링  §5 UI  §6 Map 클래스
// ════════════════════════════════════════════════════════════════════════


// ════════════════════════════════════════════════════════════════════════
// §1  설정 / 팔레트
// ════════════════════════════════════════════════════════════════════════

namespace mapcfg
{
    //이산 줌 "단계"(px/청크). 인접 단계 사이는 애니메이션으로 부드럽게 보간(매끈)하되 단계
    //  자체는 분명히 존재(rest 지점). 작을수록 광역. LOD_SWITCH_PX 미만 단계는 위성 텍스처로
    //  그려진다. 0번은 자리표시자 — zoomLevel 0 = "전세계 한 화면"(런타임 fit, levelScale가 계산).
    inline constexpr double ZOOM_STOPS[] = { 0.0, 0.20, 0.55, 1.4, 3.0, 6, 10, 16, 24, 36, 48 };
    inline constexpr int    ZOOM_COUNT   = (int)(sizeof(ZOOM_STOPS) / sizeof(ZOOM_STOPS[0]));
    inline constexpr int    DEFAULT_ZOOM = 7;   // 16px/청크 (네이티브 픽셀아트)

    //per-청크 타일 렌더 ↔ 위성 텍스처 전환 스케일. 미만=위성 텍스처(광역), 이상=살아있는 타일맵.
    //  rest 단계는 이 밴드를 피해 배치(3.0=위성 / 6=타일)해 정지 시 어정쩡한 블러 회피.
    inline constexpr double LOD_SWITCH_PX = 4.5;

    //줌 애니메이션 이징(프레임당 현재→목표 보간 비율). 클수록 빠름.
    inline constexpr double ZOOM_EASE = 0.28;

    //이 값 미만 줌에서는 바다 파도를 안 그림(저배율 클러터/draw 폭증 방지).
    inline constexpr double SYMBOL_MIN_PX = 8.0;
}

namespace mappal
{
    inline SDL_Color background()   { return {  10,  10,  14, 255 }; }
    inline SDL_Color playerMarker() { return { 220,  80,  80, 255 }; }
    inline SDL_Color uiPanel()      { return {  20,  20,  28, 220 }; }
    inline SDL_Color uiBorder()     { return { 110, 110, 115, 255 }; }
    inline SDL_Color uiText()       { return { 235, 235, 230, 255 }; }
}


// ════════════════════════════════════════════════════════════════════════
// §2  카메라 (MapView)
//
//   픽셀(=청크) 좌표계. centerPX/PY는 화면 중앙이 가리키는 픽셀(실수).
//   X는 원기둥 wrap — relX가 최단 분기로 정규화.
// ════════════════════════════════════════════════════════════════════════

struct MapView
{
    double centerPX  = 0.0;
    double centerPY  = 0.0;
    int    zoomLevel = mapcfg::DEFAULT_ZOOM;   // 목표 줌 단계(이산)
    int    z         = 0;
    int    viewW     = 0;
    int    viewH     = 0;

    //연속 줌 — curScale(현재 화면 px/청크)이 targetScale로 매 프레임 이징. 이게 "매끈함".
    //  카메라(centerPX/PY)와 스케일을 위성뷰·타일맵이 공유 → 경계 튐이 원천적으로 없음.
    double curScale    = 16.0;
    double targetScale = 16.0;
    bool   animating   = false;

    //줌 앵커 — 줌 내내 이 월드점(anchorWX/WY)을 이 화면점(anchorSX/SY)에 고정(구글지도식).
    double anchorWX = 0.0, anchorWY = 0.0;
    int    anchorSX = 0,   anchorSY = 0;

    //줌 단계 → px/청크. 0 = 전세계 한 화면(런타임 fit), 그 외 ZOOM_STOPS.
    double levelScale(int lvl) const
    {
        if (lvl <= 0) return static_cast<double>(viewH) / WORLD_PIXEL_H;
        return mapcfg::ZOOM_STOPS[lvl];
    }

    int    chunkPx()   const { return std::max(1, (int)std::lround(curScale)); }   // 컬링/span용 근사 정수
    double zoomScale() const { return curScale / 16.0; }                            // tileset 16px 기준 배율
    bool   symbolsVisible() const { return curScale >= mapcfg::SYMBOL_MIN_PX; }
    bool   worldLOD()  const { return curScale < mapcfg::LOD_SWITCH_PX; }            // 위성 텍스처 LOD?

    //목표 단계로 줌 — 화면(sx,sy) 밑 월드점을 앵커로 잡아 줌 내내 그 자리에 고정.
    void zoomAt(int sx, int sy, int delta)
    {
        anchorWX = centerPX + (sx - viewW * 0.5) / curScale;
        anchorWY = centerPY + (sy - viewH * 0.5) / curScale;
        anchorSX = sx; anchorSY = sy;
        zoomLevel = std::clamp(zoomLevel + delta, 0, mapcfg::ZOOM_COUNT - 1);
        targetScale = levelScale(zoomLevel);
        animating = true;
    }

    //매 프레임 — curScale을 targetScale로 이징하고 앵커 월드점을 앵커 화면점에 재고정.
    void tickAnim()
    {
        if (!animating) return;
        curScale += (targetScale - curScale) * mapcfg::ZOOM_EASE;
        if (std::abs(targetScale - curScale) <= targetScale * 0.003) { curScale = targetScale; animating = false; }
        centerPX = anchorWX - (anchorSX - viewW * 0.5) / curScale;
        centerPY = anchorWY - (anchorSY - viewH * 0.5) / curScale;
    }

    //카메라 중앙 기준 X 최단 거리(픽셀) — ±WORLD_CHUNK_W/2로 wrap.
    double relX(double px) const
    {
        double rel = px - centerPX;
        const double W = static_cast<double>(WORLD_CHUNK_W);
        rel -= std::round(rel / W) * W;
        return rel;
    }

    double sX(double px) const { return relX(px) * curScale + viewW * 0.5; }
    double sY(double py) const { return (py - centerPY) * curScale + viewH * 0.5; }

    //카메라 Y를 월드(극지) 안으로 클램프. X는 wrap이라 클램프 안 함. 전세계 fit(level0)에서는
    //  halfH*2 >= 월드높이 → 자동으로 centerPY=월드중앙(전체가 세로로 꽉 참).
    void clampCenterY()
    {
        const double halfH = viewH * 0.5 / curScale;
        if (halfH * 2.0 >= static_cast<double>(WORLD_PIXEL_H))
            centerPY = WORLD_PIXEL_H * 0.5;
        else
            centerPY = std::clamp(centerPY, halfH, WORLD_PIXEL_H - halfH);
    }
};

//타일 좌표 → 픽셀(청크) 좌표 (실수).
static double tileToPixelX(int tx) { return static_cast<double>(tx - TILE_BASE_X) / TILE_PER_PIXEL; }
static double tileToPixelY(int ty) { return static_cast<double>(ty - TILE_BASE_Y) / TILE_PER_PIXEL; }
//타일 좌표 → 픽셀(청크) 정수 인덱스 (청크 정렬 좌표 전용 — pos는 항상 청크 좌상단).
static int    tilePixelIX(int tx)  { return (tx - TILE_BASE_X) / TILE_PER_PIXEL; }
static int    tilePixelIY(int ty)  { return (ty - TILE_BASE_Y) / TILE_PER_PIXEL; }


// ════════════════════════════════════════════════════════════════════════
// §3  심볼 매핑 (terrain·도로·건물 → mapset 스프라이트)
// ════════════════════════════════════════════════════════════════════════

//베이스 지형 타일 id — 육지=잔디(기본), 바다=해수, 강/호수=담수.
//  도시 영역(CityZone/Center)도 잔디 — 도시는 그 위의 도로·다리(③-b)와 건물 심볼(④)로만 드러난다.
static int baseFloorId(worldGrid::Terrain t)
{
    using T = worldGrid::Terrain;
    switch (t)
    {
    case T::Sea: case T::CitySea:                   return itemID::deepSeaWater;
    case T::River: case T::Lake: case T::CityRiver: return itemID::deepFreshWater;
    default:                                        return itemID::grass;
    }
}

//도로 openBits(N=1,E=2,S=4,W=8) → mapset1by1 인덱스. degree<2는 stage7이 제거하므로
//  방어적으로 직선 fallback. (3=N+E corner, 7=N+E+S T, 15=십자 등)
static int roadSpriteIndex(std::uint8_t b)
{
    switch (b)
    {
    case 15:            return 40;   // NESW 십자
    case (1 | 4):       return 42;   // N+S 수직
    case (2 | 8):       return 41;   // E+W 수평
    case (1 | 2):       return 43;   // N+E 코너
    case (1 | 8):       return 44;   // N+W 코너
    case (4 | 8):       return 45;   // S+W 코너
    case (2 | 4):       return 46;   // E+S 코너
    case (1 | 2 | 4):   return 50;   // N+E+S (W없음) T
    case (1 | 2 | 8):   return 51;   // N+E+W (S없음) T
    case (1 | 4 | 8):   return 52;   // N+S+W (E없음) T
    case (2 | 4 | 8):   return 53;   // E+S+W (N없음) T
    case 1: case 4:     return 42;   // degree1 fallback(수직)
    case 2: case 8:     return 41;   // degree1 fallback(수평)
    default:            return -1;   // 0 = 도로 아님
    }
}

//건물 심볼 해석 결과. atlas/idx + footprint 좌상단 청크 오프셋 + 셀 청크폭.
struct ResolvedSym
{
    Sprite* atlas      = nullptr;
    int     idx        = 0;
    int     offX       = 0;   // 앵커(좌상단 청크) 기준 스프라이트 좌상단 청크 오프셋
    int     offY       = 0;
    int     cellChunks = 0;   // 스프라이트가 덮는 청크 변 길이(컬링용)
};

//(symbol, footprint w×h, hash) → 스프라이트. footprint 컨벤션:
//  1x1: mapset1by1 48px=3청크, 중앙(1,1) 정렬 → off(-1,-1).
//  2x2: mapset2by2 64px=4청크, 중앙2x2(1..2) 정렬 → off(-1,-1).
//  2x1(wide): 4x4 중 row2·col1~2 채움 → off(-1,-2).
//  1x2(tall): 4x4 중 col2·row1~2 채움 → off(-2,-1).
//  3x3: mapset3by3 80px=5청크, 중앙3x3(1..3) 정렬 → off(-1,-1).
static ResolvedSym resolveSymbol(MapSymbol s, int w, int h, std::uint64_t hash)
{
    auto one   = [&](int idx) { return ResolvedSym{ spr::mapset1by1, idx, -1, -1, 3 }; };
    auto two2  = [&](int idx) { return ResolvedSym{ spr::mapset2by2, idx, -1, -1, 4 }; };
    auto three = [&](int idx) { return ResolvedSym{ spr::mapset3by3, idx, -1, -1, 5 }; };
    //2x1/1x2 — footprint 방향으로 wide/tall 스프라이트 + 오프셋 분기.
    auto rect = [&](int wideIdx, int tallIdx) -> ResolvedSym {
        if (w == 2 && h == 1) return ResolvedSym{ spr::mapset2by2, wideIdx, -1, -2, 4 };
        return ResolvedSym{ spr::mapset2by2, tallIdx, -2, -1, 4 };   // 1x2 tall
    };

    switch (s)
    {
    case MapSymbol::apartment:        return one(1);
    case MapSymbol::bank:             return one(2);
    case MapSymbol::house:            return one(3);
    case MapSymbol::warehouse:        //창고는 다목적(경공업 공장 등) — footprint별 스프라이트 분기
        if (w == 2 && h == 2) return two2(26);
        if (w == 2 || h == 2) return rect(24, 25);
        return one(4);
    case MapSymbol::cafe:             return one(5);
    case MapSymbol::cinema:           return one(6);
    case MapSymbol::junkShop:         return one(7);
    case MapSymbol::animalHospital:   return one(8);
    case MapSymbol::pharmacy:         return one(9);
    case MapSymbol::restaurant:       return one(10);
    case MapSymbol::stationeryStore:  return one(11);
    case MapSymbol::hardwareStore:    return one(12);
    case MapSymbol::bookstore:        return one(13);
    case MapSymbol::patrolStation:    return one(15);
    case MapSymbol::convenienceStore: return one((hash & 1) ? 17 : 16);
    case MapSymbol::bicycleShop:      return one(18);
    case MapSymbol::temple:           return one(19);
    case MapSymbol::church:           return one(20);
    case MapSymbol::cathedral:        return one(21);
    case MapSymbol::skyscraper:       return one(22);
    case MapSymbol::gasStation:       return one((hash & 1) ? 25 : 24);
    case MapSymbol::shoppingArcade:   return one(32 + static_cast<int>(hash % 3));
    case MapSymbol::postOffice:       return one(55);
    case MapSymbol::autoShop:         return one(56);
    case MapSymbol::clothingStore:    return one(57);
    case MapSymbol::jewelryStore:     return one(58);
    case MapSymbol::laundromat:       return one(59);
    case MapSymbol::gardenShop:       return one(60);
    case MapSymbol::policeStation:    return rect(2, 3);
    case MapSymbol::fireStation:      return rect(4, 5);
    case MapSymbol::hotel:            return rect(11, 12);
    case MapSymbol::hospital:         return rect(13, 14);
    case MapSymbol::library:          return rect(16, 17);
    case MapSymbol::park:             return two2(1);
    case MapSymbol::hypermarket:      return two2(6);
    case MapSymbol::school:           return two2(7);
    case MapSymbol::parkingLot:       return two2(9);
    case MapSymbol::mine:             return one(112);
    case MapSymbol::lookoutTower:     return one(113);
    case MapSymbol::energyBank:       return one(114);
    case MapSymbol::warpGate:         return one(115);
    case MapSymbol::shop:             return one(116);
    case MapSymbol::nuclearPlant:     return two2(21);
    case MapSymbol::solarPlant:       return two2(22);
    case MapSymbol::researchLab:      return two2(23);
    case MapSymbol::airport:          return three(0);
    case MapSymbol::prison:           return three(1);
    case MapSymbol::militaryBase:     return three(2);
    //항만 — footprint로 1x1/2x2 분기, RULD 연속 인덱스(1x1=26~29, 2x2=27~30).
    case MapSymbol::harborR:          return (w == 1) ? one(26) : two2(27);
    case MapSymbol::harborU:          return (w == 1) ? one(27) : two2(28);
    case MapSymbol::harborL:          return (w == 1) ? one(28) : two2(29);
    case MapSymbol::harborD:          return (w == 1) ? one(29) : two2(30);
    default:                          return ResolvedSym{};   // none / mountain(별도 처리)
    }
}

//미발견 도시의 "?건물" 심볼 — 실제 종류 대신 footprint(w×h)별 미확인 placeholder.
//  오프셋/셀 규약은 resolveSymbol과 동일(1x1=mapset1by1 3청크 off(-1,-1), 3x3=mapset3by3 5청크
//  off(-1,-1), 나머지=mapset2by2 4청크 — off는 wide -1,-2 / tall -2,-1 / 2x2 -1,-1).
//  항만만 예외로 종류·방향을 드러내는 전용 ?스프라이트(물 위 구조물이라 일반 ?건물이 어색):
//  RULD 연속 인덱스, 1x1=117~120 / 2x2=32~35.
static ResolvedSym resolveUnknownSymbol(MapSymbol s, int w, int h)
{
    if (s >= MapSymbol::harborR && s <= MapSymbol::harborD)
    {
        const int d = static_cast<int>(s) - static_cast<int>(MapSymbol::harborR);
        if (w == 1 && h == 1) return ResolvedSym{ spr::mapset1by1, 117 + d, -1, -1, 3 };
        return ResolvedSym{ spr::mapset2by2, 32 + d, -1, -1, 4 };
    }
    if (w == 1 && h == 1) return ResolvedSym{ spr::mapset1by1, 63, -1, -1, 3 };
    if (w == 2 && h == 2) return ResolvedSym{ spr::mapset2by2, 20, -1, -1, 4 };
    if (w == 2 && h == 1) return ResolvedSym{ spr::mapset2by2, 19, -1, -2, 4 };   // wide
    if (w == 1 && h == 2) return ResolvedSym{ spr::mapset2by2, 18, -2, -1, 4 };   // tall
    if (w == 3 && h == 3) return ResolvedSym{ spr::mapset3by3, 3, -1, -1, 5 };
    return ResolvedSym{};
}

static std::uint64_t symHash(int px, int py)
{
    std::uint64_t h = static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) * 0x9E3779B97F4A7C15ULL;
    h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(py)) * 0xBF58476D1CE4E5B9ULL;
    h ^= h >> 31;
    return h;
}

//사전배치 도시 표시명 — codename(enum 값)으로 cityName.tsv 적재 벡터(globalVar::cityName) 인덱싱.
//  슬롯0(none)·범위 밖이면 빈 문자열(호출부가 빈 라벨 스킵). 콘솔 로그용 영어명은 PRESET_CITIES.displayName 유지.
static std::wstring presetDisplayName(city::CityName cn)
{
    const std::size_t i = static_cast<std::size_t>(cn);
    return (i != 0 && i < cityName.size()) ? cityName[i] : std::wstring{};
}

//MapSymbol → 화면 표시명(영어) — 청크맵 hover 툴팁용. resolveSymbol과 같은 종류 집합을 커버.
//  (프로토타입 — 다국어는 향후 sysStr 매핑. city displayName과 동일 정책: 표시는 영어, i18n은 후속.)
static std::wstring mapSymbolName(MapSymbol s)
{
    switch (s)
    {
    case MapSymbol::apartment:        return L"Apartment";
    case MapSymbol::bank:             return L"Bank";
    case MapSymbol::house:            return L"House";
    case MapSymbol::warehouse:        return L"Warehouse";
    case MapSymbol::cafe:             return L"Cafe";
    case MapSymbol::cinema:           return L"Cinema";
    case MapSymbol::junkShop:         return L"Junk Shop";
    case MapSymbol::animalHospital:   return L"Animal Hospital";
    case MapSymbol::pharmacy:         return L"Pharmacy";
    case MapSymbol::restaurant:       return L"Restaurant";
    case MapSymbol::stationeryStore:  return L"Stationery Store";
    case MapSymbol::hardwareStore:    return L"Hardware Store";
    case MapSymbol::bookstore:        return L"Bookstore";
    case MapSymbol::patrolStation:    return L"Patrol Station";
    case MapSymbol::convenienceStore: return L"Convenience Store";
    case MapSymbol::bicycleShop:      return L"Bicycle Shop";
    case MapSymbol::temple:           return L"Temple";
    case MapSymbol::church:           return L"Church";
    case MapSymbol::cathedral:        return L"Cathedral";
    case MapSymbol::skyscraper:       return L"Skyscraper";
    case MapSymbol::gasStation:       return L"Gas Station";
    case MapSymbol::shoppingArcade:   return L"Shopping Arcade";
    case MapSymbol::postOffice:       return L"Post Office";
    case MapSymbol::autoShop:         return L"Auto Shop";
    case MapSymbol::clothingStore:    return L"Clothing Store";
    case MapSymbol::jewelryStore:     return L"Jewelry Store";
    case MapSymbol::laundromat:       return L"Laundromat";
    case MapSymbol::gardenShop:       return L"Garden Shop";
    case MapSymbol::policeStation:    return L"Police Station";
    case MapSymbol::fireStation:      return L"Fire Station";
    case MapSymbol::hotel:            return L"Hotel";
    case MapSymbol::hospital:         return L"Hospital";
    case MapSymbol::library:          return L"Library";
    case MapSymbol::park:             return L"Park";
    case MapSymbol::hypermarket:      return L"Hypermarket";
    case MapSymbol::school:           return L"School";
    case MapSymbol::parkingLot:       return L"Parking Lot";
    case MapSymbol::mine:             return L"Mine";
    case MapSymbol::lookoutTower:     return L"Lookout Tower";
    case MapSymbol::energyBank:       return L"Energy Bank";
    case MapSymbol::warpGate:         return L"Warp Gate";
    case MapSymbol::shop:             return L"Shop";
    case MapSymbol::nuclearPlant:     return L"Nuclear Power Plant";
    case MapSymbol::solarPlant:       return L"Solar Power Plant";
    case MapSymbol::researchLab:      return L"Research Lab";
    case MapSymbol::airport:          return L"Airport";
    case MapSymbol::prison:           return L"Prison";
    case MapSymbol::militaryBase:     return L"Military Base";
    case MapSymbol::harborR: case MapSymbol::harborU:
    case MapSymbol::harborL: case MapSymbol::harborD:
        return L"Harbor";
    default:                          return std::wstring{};   // none / mountain
    }
}


// ════════════════════════════════════════════════════════════════════════
// §4  렌더링
// ════════════════════════════════════════════════════════════════════════

//y로 정렬해 그리는 심볼(산·건물) — 남쪽이 위에 겹치는 JRPG 페인터 순서.
//  br = 전장의 구름 밝기. 자연물(산/숲)은 미발견에서도 그리되 어둡게, 건물은 발견된 곳만(=밝게).
struct SymDraw { float sortY; Sprite* atlas; int idx; int sx; int sy; Uint8 br = 255; };

namespace
{
    struct BaseQuad { float l, t, r, b; int sprIdx; Uint8 a; Uint8 br; };   // br = 전장의 구름 밝기(0~255)

    //tileset 베이스 타일/파도 배치 flush — 인접 quad가 비트 동일 float 경계 공유 → 갭 없음.
    //  베이스 타일(alpha 255)과 파도(alpha<255)을 같은 배치로 — 청크 셀 내부에만
    //  그려지므로 셀 간 겹침 없음 → 같은 청크에서 push 순서(타일→파도)만 지키면 파도가 위.
    void flushBaseBatch(BaseQuad* q, int count)
    {
        if (count <= 0) return;

        SDL_Texture* tex = spr::tileset->getTexture();
        float texW, texH;
        SDL_GetTextureSize(tex, &texW, &texH);

        const int   srcSize = spr::tileset->getW();   // 16
        const float uW = srcSize / texW, vH = srcSize / texH;
        const int   atlasW = (int)texW;
        const float insetU = 0.5f / texW, insetV = 0.5f / texH;

        static SDL_Vertex vertices[MAX_BATCH * 4];
        static int        indices [MAX_BATCH * 6];

        for (int i = 0; i < count; i++)
        {
            const int sprIdx = q[i].sprIdx;
            const float u  = (float)((srcSize * sprIdx) % atlasW) / texW;
            const float vY = (float)(srcSize * ((srcSize * sprIdx) / atlasW)) / texH;
            const float u0 = u + insetU,  u1 = u + uW - insetU;
            const float v0 = vY + insetV, v1 = vY + vH - insetV;
            const float bf = q[i].br / 255.0f;   // 밝기 곱 — 미발견 청크는 어둡게(윤곽만 보임)
            const SDL_FColor col = { bf, bf, bf, q[i].a / 255.0f };

            const int vBase = i * 4;
            vertices[vBase    ] = { { q[i].l, q[i].t }, col, { u0, v0 } };
            vertices[vBase + 1] = { { q[i].r, q[i].t }, col, { u1, v0 } };
            vertices[vBase + 2] = { { q[i].r, q[i].b }, col, { u1, v1 } };
            vertices[vBase + 3] = { { q[i].l, q[i].b }, col, { u0, v1 } };

            const int iBase = i * 6;
            indices[iBase    ] = vBase;     indices[iBase + 1] = vBase + 1;
            indices[iBase + 2] = vBase + 2; indices[iBase + 3] = vBase;
            indices[iBase + 4] = vBase + 2; indices[iBase + 5] = vBase + 3;
        }
        SDL_RenderGeometry(renderer, tex, vertices, count * 4, indices, count * 6);
    }
}

//전장의 구름 밝기 — 발견(가봤거나 지금 보이는 곳) 청크=풀밝기, 미발견=어둡게(윤곽만).
//  전략맵엔 낡을 정보가 없어 "지금 보임 vs 기억" 구분 없이 발견/미발견 2상태로 통일.
//  지형(drawTerrainLayer)·외부 도로망(drawHighways)이 공유 — 항상 그리되 밝기로 fog를 표현.
static Uint8 fogBright(int px, int py) { return mapDiscovery::discovered(px, py) ? 255 : 100; }

//① 베이스 지형(오토타일) + 파도 + ② 산 심볼 (단일 스윕, 로컬 terrain 버퍼).
//   본체 renderTile의 floor 오토타일(tileConnectGroup+connectGroupExtraIndex)과
//   파도(스프라이트 1504~1526)을 청크 스케일로 이식 — 1청크가 1타일 역할.
//   산 심볼은 항상 수집(모든 줌 유지). 파도만 drawFoam(=symbolsVisible)일 때 (저배율 클러터/성능 회피).
static void drawTerrainLayer(const MapView& v, bool drawFoam, std::vector<SymDraw>& symOut, std::vector<SymDraw>& mtnOut)
{
    if (!worldGrid::worldPixelMmapActive()) return;

    using T = worldGrid::Terrain;

    //컬링 반경은 정수 chunkPx()가 아닌 *정확한* curScale로 — 줌 애니 중 cp(=lround)가 curScale보다
    //  크면(각 반올림 구간 하단, 예 curScale∈[4.5,5)→cp=5) 범위가 화면보다 좁아져 가장자리에 베이스
    //  타일이 안 깔리고 배경(검정)이 비쳐 깜빡인다. 실제 quad는 sX/sY(curScale)로 그리므로 반경도 동일 기준.
    const double halfW = v.viewW * 0.5 / v.curScale;
    const double halfH = v.viewH * 0.5 / v.curScale;

    const int minPX = (int)std::floor(v.centerPX - halfW) - 2;
    const int maxPX = (int)std::ceil (v.centerPX + halfW) + 2;
    const int minPY = std::max(0,                  (int)std::floor(v.centerPY - halfH) - 2);
    const int maxPY = std::min(WORLD_PIXEL_H - 1,   (int)std::ceil (v.centerPY + halfH) + 2);
    if (maxPX < minPX || maxPY < minPY) return;

    //로컬 terrain 버퍼 — 그릴 범위 +1 마진(이웃 룩업). worldPixel은 y OOB면 Sea, X는 자동 wrap.
    //  매 청크 worldPixel 반복(오토타일은 이웃 4~8개 조회) 대신 1회 채워 버퍼에서 O(1) 룩업.
    const int bx0 = minPX - 1, by0 = minPY - 1;
    const int bw  = (maxPX + 1) - bx0 + 1;
    const int bh  = (maxPY + 1) - by0 + 1;
    static thread_local std::vector<T> LT;
    LT.assign(static_cast<std::size_t>(bw) * bh, T::Sea);
    for (int ly = 0; ly < bh; ++ly)
        for (int lx = 0; lx < bw; ++lx)
            LT[static_cast<std::size_t>(ly) * bw + lx] =
                worldGrid::worldPixel(worldWrap::wrapPixelX(bx0 + lx), by0 + ly);

    auto terrAt = [&](int x, int y) -> T {
        const int lx = x - bx0, ly = y - by0;
        if (lx < 0 || lx >= bw || ly < 0 || ly >= bh) return T::Sea;
        return LT[static_cast<std::size_t>(ly) * bw + lx];
    };
    auto isSea = [&](int x, int y) { T t = terrAt(x, y); return t == T::Sea || t == T::CitySea; };
    //절차적 산맥 마스크 — 위성 Mountain 외에 worldGen::isMountainChunk로 추가 생성.
    //  LT처럼 1회 채워 O(1). isMtn이 위성+절차 둘 다 보게 해 #64~79 오토타일이 자연 연결.
    static thread_local std::vector<char> LM;
    LM.assign(static_cast<std::size_t>(bw) * bh, char(0));
    for (int ly = 0; ly < bh; ++ly)
        for (int lx = 0; lx < bw; ++lx)
            LM[static_cast<std::size_t>(ly) * bw + lx] =
                worldGen::isMountainChunk(bx0 + lx, by0 + ly, worldSeed) ? char(1) : char(0);
    auto isMtn = [&](int x, int y) -> bool {
        if (terrAt(x, y) == T::Mountain) return true;          // 위성 산맥
        const int lx = x - bx0, ly = y - by0;
        if (lx < 0 || lx >= bw || ly < 0 || ly >= bh) return false;
        return LM[static_cast<std::size_t>(ly) * bw + lx] != 0;  // 절차 산맥
    };

    //숲 마스크 버퍼 — 절차적 숲(위성에 없음)이라 terrain이 아닌 공유 술어
    //  worldGen::isForestChunk로 판정. 9셀 trig라 매 이웃 재계산 대신 LT처럼 1회 채워 O(1) 룩업.
    static thread_local std::vector<char> LF;
    LF.assign(static_cast<std::size_t>(bw) * bh, char(0));
    for (int ly = 0; ly < bh; ++ly)
        for (int lx = 0; lx < bw; ++lx)
            LF[static_cast<std::size_t>(ly) * bw + lx] =
                worldGen::isForestChunk(bx0 + lx, by0 + ly, worldSeed) ? char(1) : char(0);
    auto isFor = [&](int x, int y) -> bool {
        const int lx = x - bx0, ly = y - by0;
        if (lx < 0 || lx >= bw || ly < 0 || ly >= bh) return false;
        return LF[static_cast<std::size_t>(ly) * bw + lx] != 0;
    };
    //담수 파도는 안 씀 — 본체의 담수 파도 스프라이트(+496)는 물속 깊은물↔얕은물 전용이라
    //  육지 물가에 그리면 일부 변형이 "물 들어갈 때 퍼지는 파동(Wave 2016~2021)" 스프라이트와
    //  겹쳐 잔디 위에 정지된 파동으로 보임. 담수는 floor 오토타일 가장자리로만 표현.

    const seasonFlag season = getSeason();

    //floor 오토타일 sprite index — 본체 renderTile 규칙 그대로(이웃은 청크 terrain).
    //  cg==0: 같은 floor id끼리 연결 / cg>0: 같은 connectGroup끼리 / cg==-1: 오토타일 없음.
    auto floorSprIdx = [&](int ix, int iy) -> int {
        const int id = baseFloorId(terrAt(ix, iy));
        int spr = itemDex[id].tileSprIndex + itemDex[id].extraSprIndexSingle + 16 * itemDex[id].extraSprIndex16;
        const int cg = itemDex[id].tileConnectGroup;
        if (cg != -1)
        {
            auto conn = [&](int nx, int ny) -> bool {
                const int nid = baseFloorId(terrAt(nx, ny));
                return (cg == 0) ? (id == nid) : (cg == itemDex[nid].tileConnectGroup);
            };
            spr += connectGroupExtraIndex(conn(ix, iy - 1), conn(ix, iy + 1), conn(ix - 1, iy), conn(ix + 1, iy));
        }
        if (id == itemID::grass)
        {
            if (season == seasonFlag::winter)      spr += 16;
            else if (season == seasonFlag::summer) spr += 32;
        }
        return spr;
    };

    static thread_local std::vector<BaseQuad> batch;
    batch.clear();
    auto flush    = [&]() { flushBaseBatch(batch.data(), (int)batch.size()); batch.clear(); };
    auto pushQuad = [&](int ix, int iy, int sprIdx, Uint8 a, Uint8 br) {
        if ((int)batch.size() >= MAX_BATCH) flush();
        batch.push_back(BaseQuad{
            (float)v.sX((double)ix),       (float)v.sY((double)iy),
            (float)v.sX((double)(ix + 1)), (float)v.sY((double)(iy + 1)), sprIdx, a, br });
    };

    //파도 — 본체 renderTile addWave 매핑(1504~1526). isW: 그 방향 이웃이 물인가.
    //  자기 자신이 그 물타입이 아닐 때만(경계 셀) 그림. baseOff: 해수=애니프레임, 담수=496.
    //  맵에서는 파도가 움직이면 정신사나워서 2번째 프레임(인덱스1, 오프셋32)으로 고정.
    const int seaAnim = 32;
    auto emitFoam = [&](int ix, int iy, auto&& isW, int baseOff, Uint8 alpha, Uint8 br) {
        const bool tC = isW(ix, iy - 1), bC = isW(ix, iy + 1), lC = isW(ix - 1, iy), rC = isW(ix + 1, iy);
        const bool trC = isW(ix + 1, iy - 1), tlC = isW(ix - 1, iy - 1),
                   blC = isW(ix - 1, iy + 1), brC = isW(ix + 1, iy + 1);
        if (!(tC || bC || lC || rC || trC || tlC || blC || brC)) return;
        auto push = [&](int idx) { pushQuad(ix, iy, idx + baseOff, alpha, br); };
        if      (tC && bC && lC && rC) push(1526);
        else if (tC && bC && rC)       push(1520);
        else if (lC && bC && rC)       push(1523);
        else if (bC && rC && tC)       push(1522);
        else if (rC && tC && lC)       push(1521);
        else if (tC && bC)             push(1524);
        else if (rC && lC)             push(1525);
        else if (rC && tC)             push(1505);
        else if (tC && lC)             push(1507);
        else if (lC && bC)             push(1509);
        else if (bC && rC)             push(1511);
        else if (tC)                   push(1506);
        else if (bC)                   push(1510);
        else if (lC)                   push(1508);
        else if (rC)                   push(1504);
        if (trC && !tC && !rC) push(1514);
        if (tlC && !tC && !lC) push(1515);
        if (blC && !bC && !lC) push(1512);
        if (brC && !bC && !rC) push(1513);
    };

    for (int iy = minPY; iy <= maxPY; ++iy)
        for (int ix = minPX; ix <= maxPX; ++ix)
        {
            const T t = terrAt(ix, iy);
            const Uint8 br = fogBright(ix, iy);   // 전장의 구름 밝기

            //① 베이스 타일 (오토타일)
            pushQuad(ix, iy, floorSprIdx(ix, iy), 255, br);

            //파도 — 해수 경계만. 자기 자신이 바다가 아닌 셀에만(본체와 동일).
            if (drawFoam && !isSea(ix, iy))
                emitFoam(ix, iy, isSea, seaAnim, 200, br);

            //② 산 심볼 (47-piece 블롭 오토타일) — auto47Mountain.png. 8 이웃 기반이라
            //  기존 16타일(mapset #64~79, 3청크 중앙정렬)보다 코너·변 디테일이 풍부.
            //  각 bool = "그 이웃이 非산"(대비지형) — shoreSpline과 동일 autotile47Index 컨벤션.
            //  셀=16px=1청크라 오프셋 없이 자기 청크 칸에 그대로 그림.
            //  isMtn = 위성 Mountain ∪ 절차(isMountainChunk) — 둘이 한 오토타일로 연결됨.
            if (isMtn(ix, iy))
            {
                auto bg = [&](int x, int y) { return !isMtn(x, y); };   // 대비지형(非산)
                const int idx = autotile47Index(
                    bg(ix, iy - 1), bg(ix + 1, iy), bg(ix, iy + 1), bg(ix - 1, iy),
                    bg(ix - 1, iy - 1), bg(ix + 1, iy - 1), bg(ix - 1, iy + 1), bg(ix + 1, iy + 1));
                mtnOut.push_back(SymDraw{ (float)iy, spr::auto47Mountain, idx,
                    (int)std::lround(v.sX((double)ix)),
                    (int)std::lround(v.sY((double)iy)), br });
            }

            //③ 숲 심볼 (16타일 오토타일, connectGroupExtraIndex) — 위성에 없는 절차적
            //  숲이라 terrain이 아닌 공유 술어 worldGen::isForestChunk(LF 버퍼)로 판정 →
            //  mapset1by1 #96(베이스) + 0~15. 건물과 같은 3청크 중앙 정렬(art는 중앙 16px).
            if (isFor(ix, iy))
            {
                const int idx = 96 + connectGroupExtraIndex(
                    isFor(ix, iy - 1), isFor(ix, iy + 1), isFor(ix - 1, iy), isFor(ix + 1, iy));
                symOut.push_back(SymDraw{ (float)iy, spr::mapset1by1, idx,
                    (int)std::lround(v.sX((double)(ix - 1))),
                    (int)std::lround(v.sY((double)(iy - 1))), br });
            }
        }
    flush();
}

//③-a 외부 도로 (도시간 광역 도로망). worldGen::activePolyLines를 청크 셀 openBits로
//   래스터화 — CityPlan.roadCells는 도시 footprint 안쪽만 담아서, 도시↔도시 연결 도로는
//   이 채널이 아니면 월드맵에 안 나온다. (procGenerate stage 3가 실타일에 15폭 아스팔트로
//   이미 깔지만, 그건 월드 타일이지 월드맵 심볼이 아니다.)
struct HighwayCell { int cx; int cy; int z; int sprIdx; };   // sprIdx = 빌드시 해석된 mapset1by1 인덱스

//activePolyLines를 청크 openBits 셀로 래스터화한 캐시. 폴리라인은 월드젠 후 불변이라
//  포인터 기준 1회 메모이즈 (Map 여닫기마다 재계산 회피, 새 월드면 포인터 바뀌어 재빌드).
//  4-연결(대각 없는) 라인 워크로 인접 셀끼리 양방향 비트를 OR — 교차/분기는 자연히 T·십자.
static const std::vector<HighwayCell>& highwayCells()
{
    static std::vector<HighwayCell> cache;
    static const std::vector<worldGen::RoadPolyLine>* builtFrom = nullptr;
    static bool built = false;

    const std::vector<worldGen::RoadPolyLine>* polys = worldGen::activePolyLines;
    if (built && polys == builtFrom) return cache;
    built     = true;
    builtFrom = polys;
    cache.clear();
    if (polys == nullptr) return cache;

    //(cx,cy,z) → openBits 누적. cx∈[0,WORLD_CHUNK_W) cy∈[0,WORLD_PIXEL_H) 둘 다 16비트 이내.
    std::unordered_map<std::uint64_t, std::uint8_t> bits;
    auto keyOf = [](int cx, int cy, int z) -> std::uint64_t {
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) << 32)
             | (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy)) << 16)
             |  static_cast<std::uint64_t>(static_cast<std::uint32_t>((z + 32768) & 0xFFFF));
        };
    auto stamp = [&](int cx, int cy, int z, std::uint8_t bit) { bits[keyOf(cx, cy, z)] |= bit; };

    constexpr std::uint8_t kN = 1, kE = 2, kS = 4, kW = 8;   // roadSpriteIndex와 동일 비트

    for (const worldGen::RoadPolyLine& poly : *polys)
        for (std::size_t i = 1; i < poly.verts.size(); ++i)
        {
            const Point3& a = poly.verts[i - 1];
            const Point3& b = poly.verts[i];
            const int z  = a.z;
            const int ax = tilePixelIX(a.x), ay = tilePixelIY(a.y);
            const int bx = tilePixelIX(b.x), by = tilePixelIY(b.y);

            //4-연결 라인 워크 — 매 step이 정확히 한 카디널 이동(대각 staircase). 이동마다
            //  두 셀에 서로 향하는 비트를 박아 끊김 없는 도로 토폴로지 형성.
            const int dx = std::abs(bx - ax), dy = std::abs(by - ay);
            const int sx = (ax < bx) ? 1 : -1;
            const int sy = (ay < by) ? 1 : -1;
            const int dx2 = dx * 2, dy2 = dy * 2;
            const int moves = dx + dy;
            int x = ax, y = ay, err = dx - dy;
            for (int m = 0; m < moves; ++m)
            {
                if (err > 0)
                {
                    stamp(x, y, z, (sx > 0) ? kE : kW);
                    x += sx;
                    stamp(x, y, z, (sx > 0) ? kW : kE);
                    err -= dy2;
                }
                else
                {
                    stamp(x, y, z, (sy > 0) ? kS : kN);
                    y += sy;
                    stamp(x, y, z, (sy > 0) ? kN : kS);
                    err += dx2;
                }
            }
        }

    //심볼 해석 — 셀이 수계(강/바다/호수) 위면 다리 심볼, 아니면 일반 도로 autotile. worldPixel은
    //  X 자동 wrap·범위밖 Sea·mmap 활성 필요. (procGenerate stage8이 박는 CityRoadCell.isBridge와
    //  달리 외부 도로엔 플래그가 없어 여기서 지형으로 재구성.) 물 횡단은 A*가 직진 강제라 openBits
    //  항상 한 축: E|W=수평 다리, N|S=수직 다리(38). 수평 다리는 바로 아래(cy+1)가 물이면 물에
    //  기둥 내리는 37, 물이 아니면(강폭 1칸 등) 기둥 없는 39.
    const bool mmapOn = worldGrid::worldPixelMmapActive();
    auto isWaterPixel = [](worldGrid::Terrain t) {
        using T = worldGrid::Terrain;
        return t == T::Sea || t == T::River || t == T::Lake || t == T::CityRiver || t == T::CitySea;
        };

    cache.reserve(bits.size());
    for (const auto& [k, ob] : bits)
    {
        const int cx = static_cast<int>((k >> 32) & 0xFFFF);
        const int cy = static_cast<int>((k >> 16) & 0xFFFF);
        const int z  = static_cast<int>(k & 0xFFFF) - 32768;

        int sprIdx;
        if (mmapOn && isWaterPixel(worldGrid::worldPixel(cx, cy)))
        {
            if (ob & (2 | 8))   // E|W → 수평 다리: 아래 물이면 기둥(37), 아니면 39
                sprIdx = isWaterPixel(worldGrid::worldPixel(cx, cy + 1)) ? 37 : 39;
            else                // N|S → 수직 다리
                sprIdx = 38;
        }
        else
        {
            sprIdx = roadSpriteIndex(ob);
            if (sprIdx < 0) continue;   // openBits==0 (방어적 — 정상적으로는 없음)
        }

        cache.push_back(HighwayCell{ .cx = cx, .cy = cy, .z = z, .sprIdx = sprIdx });
    }
    return cache;
}

//도로 셀이 산 위인가 — 산 위 도로는 터널(반투명)로 그린다. 생성이 산을 단일 연결 덩어리로
//  보장하므로 렌더 브리징 없이 산 판정만으로 충분(위성 Mountain ∪ 절차 isMountainChunk).
static constexpr Uint8 TUNNEL_ALPHA = 110;   // 산 위 도로 알파(터널) — 빌드 후 튜닝 가능
static bool roadCellOnMountain(int cx, int cy)
{
    return worldGrid::worldPixel(worldWrap::wrapPixelX(cx), cy) == worldGrid::Terrain::Mountain
        || worldGen::isMountainChunk(cx, cy, worldSeed);
}

//사이트 footprint 청크 판정 — 심볼 밑에 깔린 외부 도로 셀은 그리지 않는다(조망대 아래
//  중앙분리선 같은 어색함 방지). 진입 스퍼의 끝 셀이 footprint 안쪽이라 심볼과 겹치기 때문.
//  openBits(highwayCells)는 건드리지 않으므로 이웃 셀의 T/코너 연결 팔은 그대로 —
//  시각적으로 도로가 시설 안으로 들어가며 끝난다. activeSites 포인터 기준 1회 메모이즈
//  (highwayCells 패턴 — Map은 메인 스레드 전용이라 뮤텍스 불필요, isSiteChunk와 다른 점).
//  worldGen::isSiteChunk는 1링 dilation이 있어 진입로 셀까지 지워버리므로 여기선 못 쓴다.
static bool siteFootprintChunk(int cx, int cy)
{
    static std::unordered_set<std::uint64_t> mask;
    static const std::vector<worldGen::SiteNode>* builtFrom = nullptr;
    static bool built = false;

    auto keyOf = [](int x, int y) -> std::uint64_t {
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32)
             |  static_cast<std::uint64_t>(static_cast<std::uint32_t>(y));
    };

    const std::vector<worldGen::SiteNode>* sites = worldGen::activeSites;
    if (!built || builtFrom != sites)
    {
        built     = true;
        builtFrom = sites;
        mask.clear();
        if (sites != nullptr)
            for (const worldGen::SiteNode& s : *sites)
            {
                const int px = tilePixelIX(s.pos.x);
                const int py = tilePixelIY(s.pos.y);
                for (int oy = 0; oy < s.h; ++oy)
                for (int ox = 0; ox < s.w; ++ox)
                    mask.insert(keyOf(px + ox, py + oy));
            }
    }
    return mask.count(keyOf(cx, cy)) != 0;
}

//외부 도로 그리기 — highwayCells()가 빌드 시 해석해둔 sprIdx(도로/다리)를 그대로 그림. setZoom은 호출자.
//  도로망은 미발견 청크여도 항상 표시(전세계 도로 골격) — 단 미발견은 지형처럼 어둡게(fogBright)
//  색조해 전장의 구름 느낌은 유지. (도시 내부 도로·다리는 drawCityRoads가 따로 그림.)
static void drawHighways(const MapView& v)
{
    const std::vector<HighwayCell>& cells = highwayCells();
    const int cp   = v.chunkPx();
    const int span = 3 * cp;   // mapset1by1 = 3청크

    for (const HighwayCell& c : cells)
    {
        if (c.z != v.z) continue;
        if (siteFootprintChunk(c.cx, c.cy)) continue;   //사이트 심볼 밑 도로 숨김 — 연결은 이웃 셀이 유지

        const int sx = (int)std::lround(v.sX((double)(c.cx - 1)));
        const int sy = (int)std::lround(v.sY((double)(c.cy - 1)));
        if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

        const Uint8 br = fogBright(c.cx, c.cy);
        SDL_SetTextureColorMod(spr::mapset1by1->getTexture(), br, br, br);
        SDL_SetTextureAlphaMod(spr::mapset1by1->getTexture(), roadCellOnMountain(c.cx, c.cy) ? TUNNEL_ALPHA : 255);
        drawSprite(spr::mapset1by1, c.sprIdx, sx, sy);
    }
    SDL_SetTextureColorMod(spr::mapset1by1->getTexture(), 255, 255, 255);   // 색조 원복(다음 패스 오염 방지)
    SDL_SetTextureAlphaMod(spr::mapset1by1->getTexture(), 255);             // 알파 원복(다음 패스/프레임 오염 방지)
}

//도시 심볼/도로망 소스 — full plan(CityPlanCache) 우선, 없으면 경량 layout(CityLayoutCache).
//  full은 symbols/roadCells의 상위집합이고 같은 seed면 layout과 동일하므로 우선해도 무손실.
//  둘 다 없으면 nullptr → 호출자가 그 도시를 스킵(아직 생성 안 됨).
static const std::vector<CityRoadCell>* roadCellsFor(city::CityId id)
{
    if (const CityPlan*   p = CityPlanCache::ins().peek(id))   return &p->roadCells;
    if (const CityLayout* l = CityLayoutCache::ins().peek(id)) return &l->roadCells;
    return nullptr;
}
static const std::vector<CitySymbol>* symbolsFor(city::CityId id)
{
    if (const CityPlan*   p = CityPlanCache::ins().peek(id))   return &p->symbols;
    if (const CityLayout* l = CityLayoutCache::ins().peek(id)) return &l->symbols;
    return nullptr;
}

//도시 footprint(bbox, 청크-픽셀 단위)가 현재 화면과 겹치는지. 중심 픽셀을
//  화면으로(relX이 X 시암 wrap 처리) + footprint 절반+여유 반경으로 근사 컬링.
static bool cityOnScreen(const MapView& v, const worldGen::CityNode& node)
{
    if (node.bboxW <= 0 || node.bboxH <= 0) return false;
    const int minPx = node.bboxPx, minPy = node.bboxPy;
    const int maxPx = node.bboxPx + node.bboxW, maxPy = node.bboxPy + node.bboxH;
    const double cx = v.sX((minPx + maxPx) * 0.5);
    const double cy = v.sY((minPy + maxPy) * 0.5);
    const double rPx = (std::max(maxPx - minPx, maxPy - minPy) * 0.5 + 2.0) * v.curScale;
    return !(cx + rPx < 0 || cx - rPx > v.viewW || cy + rPx < 0 || cy - rPx > v.viewH);
}

//화면에 들어온 미캐시 도시의 경량 layout을 백그라운드 워커에 요청 — 도시가 시야에 들어오면
//  정찰지도(?건물+도로망)가 채워진다. 두 캐시 peek로 이미 있는 도시는 스킵, requestAsync가
//  inFlight로 중복 방지, 프레임당 enqueue 상한으로 신규 영역 스크롤 시 워커 폭주 차단.
//  렌더 스레드는 요청만 하고 절대 블록되지 않음(빌드는 워커에서 락 밖 수행).
static void ensureVisibleCityLayouts(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    constexpr int MAX_ENQUEUE_PER_FRAME = 8;
    int enqueued = 0;
    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const auto& node = (*cities)[i];
        if (node.center.z != v.z) continue;
        const auto id = static_cast<city::CityId>(i);
        if (CityPlanCache::ins().peek(id))   continue;   // full plan 이미 있음
        if (CityLayoutCache::ins().peek(id)) continue;   // layout 이미 있음
        if (!cityOnScreen(v, node)) continue;
        CityLayoutCache::ins().requestAsync(id, worldSeed);
        if (++enqueued >= MAX_ENQUEUE_PER_FRAME) break;
    }
}

//③-b 도시 내부 도로·다리 심볼 (캐시 or 경량 layout) — 평면 레이어라 즉시 그림(정렬 X). setZoom은 호출자.
//   발견 여부와 무관하게 그리되 미발견 청크는 지형처럼 어둡게(fogBright) 색조. 미발견 도시는
//   잔디 위에 이 도로망/다리 + ?건물(④)이 얹혀 "정찰지도"처럼 보인다.
static void drawCityRoads(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    const int cp = v.chunkPx();
    const int span = 3 * cp;   // mapset1by1 = 3청크

    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const std::vector<CityRoadCell>* rcs = roadCellsFor(static_cast<city::CityId>(i));
        if (!rcs) continue;

        for (const auto& rc : *rcs)
        {
            if (rc.pos.z != v.z) continue;

            //다리 칸은 전용 심볼 — E|W(좌우) 37 / N|S(상하) 38. (다리 openBits는 한 축뿐)
            //  그 외는 일반 도로 autotile.
            int idx;
            if (rc.isBridge)
                idx = (rc.openBits & (2 | 8)) ? 37 : 38;   // N=1,E=2,S=4,W=8
            else
            {
                idx = roadSpriteIndex(rc.openBits);
                if (idx < 0) continue;
            }

            const int px = tilePixelIX(rc.pos.x);
            const int py = tilePixelIY(rc.pos.y);
            const int sx = (int)std::lround(v.sX((double)(px - 1)));
            const int sy = (int)std::lround(v.sY((double)(py - 1)));
            if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

            const Uint8 br = fogBright(px, py);
            SDL_SetTextureColorMod(spr::mapset1by1->getTexture(), br, br, br);
            SDL_SetTextureAlphaMod(spr::mapset1by1->getTexture(), roadCellOnMountain(px, py) ? TUNNEL_ALPHA : 255);
            drawSprite(spr::mapset1by1, idx, sx, sy);
        }
    }
    SDL_SetTextureColorMod(spr::mapset1by1->getTexture(), 255, 255, 255);   // 색조 원복(다음 패스 오염 방지)
    SDL_SetTextureAlphaMod(spr::mapset1by1->getTexture(), 255);             // 알파 원복(다음 패스/프레임 오염 방지)
}

//디버그 — 미발견 심볼 전체 식별 토글(F4, 맵 열린 상태). ?건물/Unknown이 실심볼·실명으로
//   표시된다(fog 밝기는 유지 — 발견 상태 자체는 안 건드림). 세션 전역, 세이브 무관.
static bool debugRevealSymbols = false;

//④ 건물 심볼 (캐시된 도시) — symOut에 누적(산과 함께 y정렬 후 그림). 발견된 청크는 실제 종류
//   스프라이트, 미발견 청크는 footprint별 "?건물" placeholder(resolveUnknownSymbol) + 어둡게(fog).
static void drawCityBuildings(const MapView& v, std::vector<SymDraw>& symOut)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    const int cp = v.chunkPx();

    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const std::vector<CitySymbol>* syms = symbolsFor(static_cast<city::CityId>(i));
        if (!syms) continue;

        for (const auto& sym : *syms)
        {
            if (sym.pos.z != v.z) continue;
            const int apx = tilePixelIX(sym.pos.x);
            const int apy = tilePixelIY(sym.pos.y);
            const bool seen = debugRevealSymbols || mapDiscovery::discovered(apx, apy);

            //발견=실제 종류, 미발견=?건물 placeholder. 둘 다 못 풀면(none/미지원 footprint) 스킵.
            const ResolvedSym rs = seen
                ? resolveSymbol(sym.symbol, sym.w, sym.h, symHash(apx, apy))
                : resolveUnknownSymbol(sym.symbol, sym.w, sym.h);
            if (!rs.atlas) continue;

            const int sx = (int)std::lround(v.sX((double)(apx + rs.offX)));
            const int sy = (int)std::lround(v.sY((double)(apy + rs.offY)));
            const int span = rs.cellChunks * cp;
            if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

            symOut.push_back(SymDraw{ (float)apy, rs.atlas, rs.idx, sx, sy, fogBright(apx, apy) });
        }
    }
}

//④-b 교외 인카운터 사이트 심볼 — drawCityBuildings와 동일 규약(발견=실제/미발견=?건물,
//   y정렬·컬링·fog 공용 파이프라인). 데이터 소스만 다름: worldGen::activeSites 직접 순회
//   (도시와 달리 CityPlan 캐시 불필요 — 월드젠 때 eager 확정된 세션 불변 배열).
static void drawSites(const MapView& v, std::vector<SymDraw>& symOut)
{
    const auto* sites = worldGen::activeSites;
    if (!sites) return;

    const int cp = v.chunkPx();

    for (const auto& site : *sites)
    {
        if (site.pos.z != v.z) continue;
        const int apx = tilePixelIX(site.pos.x);
        const int apy = tilePixelIY(site.pos.y);
        const bool seen = debugRevealSymbols || mapDiscovery::discovered(apx, apy);

        const ResolvedSym rs = seen
            ? resolveSymbol(site.symbol, site.w, site.h, symHash(apx, apy))
            : resolveUnknownSymbol(site.symbol, site.w, site.h);
        if (!rs.atlas) continue;

        const int sx = (int)std::lround(v.sX((double)(apx + rs.offX)));
        const int sy = (int)std::lround(v.sY((double)(apy + rs.offY)));
        const int span = rs.cellChunks * cp;
        if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

        symOut.push_back(SymDraw{ (float)apy, rs.atlas, rs.idx, sx, sy, fogBright(apx, apy) });
    }
}

//⑥ 도시 이름 라벨 — 도시 중심에 표기. CityPlan 캐시 없이 activeCities의 center를 직접 사용
//   (peek로 캐시된 도시만 보는 건물/도로와 달리 전 도시 라벨 가능). 사전배치=PRESET_CITIES
//   displayName, 절차생성(codename==none)=nameGen 영어식 지명 생성기(center 해시 seed).
//   픽셀폰트 size 12를 zoomScale로 NEAREST 스케일(격자 보존). 배경은 반투명 검정 박스.
//   발견 여부 무관하게 화면 내 전 도시 표기 — 청크맵에선 도시 layout(?건물+도로망)이 이미
//   보이므로 이름도 같이 노출. 클러터는 화면 컬링으로만 제한(저배율 Satellite 모드는
//   drawWorldView가 사전마킹 도시만 표기). 이 함수는 비-worldLOD(청크맵) 분기에서만 호출됨.
static void drawCityLabels(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    //줌 배율로 라벨 크기 — 단, zoomScale을 1:1로 따라가면 저배율(16/10px 청크)에서 폰트가
    //  10~12px까지 줄어 읽기 힘듦. 기준 배율(LABEL_BASE_SCALE, 16px/청크 기준)에서 줌 변화의
    //  LABEL_ZOOM_RATE 비율만 추종 → 줌아웃 시 천천히 작아진다. 너무 작/커지지 않게 클램프.
    constexpr float LABEL_BASE_SCALE = 1.45f;  // 16px/청크(zoomScale=1)에서의 기준 배율 — 튜닝 가능
    constexpr float LABEL_ZOOM_RATE  = 0.45f;  // 줌 변화 추종 비율(1=완전추종, 0=고정) — 튜닝 가능
    const float scale = std::clamp(
        LABEL_BASE_SCALE + ((float)v.zoomScale() - 1.0f) * LABEL_ZOOM_RATE, 0.9f, 2.5f);

    setFont(fontType::pixel);
    setFontSize(12);

    //절차도시 이름을 그릴 문자체계 — 현재 언어팩 기준. 같은 seed면 라틴/한글이 음차 관계(향후 cityName.tsv).
    const nameGen::Script nameScript = (option::language == L"Korean") ? nameGen::Script::Hangul : nameGen::Script::Latin;

    for (const auto& node : *cities)
    {
        if (node.center.z != v.z) continue;

        const double sxd = v.sX(tileToPixelX(node.center.x));
        const double syd = v.sY(tileToPixelY(node.center.y));
        if (sxd < -256 || sxd > v.viewW + 256 || syd < -64 || syd > v.viewH + 64) continue;

        const std::wstring name = (node.codename != city::CityName::none)
            ? presetDisplayName(node.codename)
            : nameGen::placeName(symHash(node.center.x, node.center.y), nameScript);
        if (name.empty()) continue;

        const int cx = (int)std::lround(sxd);
        const int cy = (int)std::lround(syd);
        const int tw = (int)std::lround(queryTextWidth(name)  * scale);
        const int th = (int)std::lround(queryTextHeight(name) * scale);

        //배경 — 반투명 검정. drawFillRect 2-인자는 알파 255 강제라 3-인자(alpha)로.
        const int padX = (int)std::lround(4.0f * scale);
        const int padY = (int)std::lround(2.0f * scale);
        drawFillRect(SDL_Rect{ cx - tw / 2 - padX, cy - th / 2 - padY,
                               tw + padX * 2, th + padY * 2 }, SDL_Color{ 0, 0, 0, 255 }, (Uint8)100);

        drawTextCenterScaled(name, cx, cy, scale, SDL_Color{ 245, 222, 120, 255 });
    }

    setFont(fontType::mainFont);   // 폰트 상태 복원(이후 UI 크롬이 자체 폰트/크기 설정)
}

// ────────── 위성 텍스처 LOD (광역 조망) ──────────
//  curScale < LOD_SWITCH_PX 일 때 per-청크 타일맵 대신 그림. 전세계 한 장 텍스처는
//  텍셀당 ~20청크라 중간 줌에서 ~90배 확대돼 뭉개진다(눈 아픔). 대신 "보이는 영역"만
//  화면 해상도로 그때그때 재샘플링 → 텍셀 ≈ 화면픽셀 ≈ 1청크라 어느 줌에서든 선명.
//  카메라(sX/sY/curScale)를 타일맵과 공유 → LOD 경계를 넘나들어도 위치·스케일 연속.
//  전장의 구름 — 위성 텍스처를 구울 때 미발견 청크 텍셀을 청크맵과 동일한 fogBright로 어둡게
//  "베이크". 별도 오버레이 없이 카메라와 정합되며, 발견 청크 수(mapDiscovery::count)가 바뀌면
//  캐시를 무효화해 새로 탐험한 영역이 밝아진다(맵 보는 중엔 발견이 안 바뀌므로 재빌드 없음).
namespace
{
    SDL_Texture*  g_satTex  = nullptr;
    int           g_satTW   = 0, g_satTH = 0;       // 텍스처 크기
    double        g_satWX0  = 0.0, g_satWY0 = 0.0;  // 덮는 월드 rect 좌상단(청크좌표, X unwrapped)
    double        g_satCPT  = -1.0;                 // chunks per texel(해상도). <0 = 무효
    std::uint64_t g_satSeed = ~0ull;                // worldSeed 바뀌면 재빌드
    std::size_t   g_satDisc = static_cast<std::size_t>(-1);   // 발견 청크 수 바뀌면 재빌드(fog 베이크)

    SDL_Texture*  g_worldTex  = nullptr;            // 전세계 저해상도 백드롭(검정 깜빡임 방지)
    std::uint64_t g_worldSeed = ~0ull;
    std::size_t   g_worldDisc = static_cast<std::size_t>(-1); // 발견 청크 수 바뀌면 재빌드(fog 베이크)

    //지형 타입 → 위성지도풍 색. 도시(CityZone/Center)는 회색 footprint로 표기 — 위성 줌에서
    //  도시를 지형색에 묻지 않고 명시적으로 드러낸다. 내부 물(CityRiver/CitySea)은 물색 유지.
    SDL_Color worldTerrainColor(worldGrid::Terrain t)
    {
        using T = worldGrid::Terrain;
        switch (t)
        {
        case T::Sea: case T::CitySea:                   return {  40,  72, 112, 255 };
        case T::River: case T::Lake: case T::CityRiver: return {  74, 116, 156, 255 };
        case T::Mountain:                               return { 120, 112, 102, 255 };
        case T::Desert:                                 return { 198, 178, 120, 255 };
        case T::Polar:                                  return { 232, 238, 244, 255 };
        case T::Tundra:                                 return { 188, 198, 192, 255 };
        case T::Subarctic:                              return { 132, 158, 134, 255 };
        case T::Monsoon:                                return {  92, 142,  82, 255 };
        case T::InsularRainforest:                      return {  54, 122,  74, 255 };
        case T::ContinentalRainforest:                  return {  44, 112,  64, 255 };
        case T::CityZone: case T::CityCenter:           return { 128, 128, 128, 255 };   // 도시 footprint = 회색
        default:                                        return {  86, 132,  78, 255 };   // Land
        }
    }

    //전세계 저해상도 백드롭 — 시드별 1회 빌드. 항상 화면을 덮어 윈도우 미커버 영역(로딩 전/
    //  패닝 경계/줌아웃 주변)의 검정 깜빡임을 막는다. fit 줌에서는 2160px가 화면과 1:1이라 선명.
    SDL_Texture* worldMapTexture()
    {
        if (!worldGrid::worldPixelMmapActive()) return nullptr;
        const std::size_t disc = mapDiscovery::count();   // 발견 상태 변화 감지(fog 베이크 무효화)
        if (g_worldTex && g_worldSeed == worldSeed && g_worldDisc == disc) return g_worldTex;
        if (g_worldTex) { SDL_DestroyTexture(g_worldTex); g_worldTex = nullptr; }

        constexpr int TW = 2160, TH = 1080;   // 2:1 (월드 비율)
        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, TW, TH);
        if (!tex) return nullptr;
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);

        static std::vector<std::uint32_t> buf;
        buf.assign(static_cast<std::size_t>(TW) * TH, 0);
        for (int ty = 0; ty < TH; ++ty)
        {
            const int py = static_cast<int>(static_cast<std::int64_t>(ty) * WORLD_PIXEL_H / TH);
            for (int tx = 0; tx < TW; ++tx)
            {
                const int px = static_cast<int>(static_cast<std::int64_t>(tx) * WORLD_PIXEL_W / TW);
                const Uint8 br = fogBright(px, py);   // 전장의 구름 — 미발견 청크 어둡게(청크맵과 동일 강도)
                const SDL_Color c = worldTerrainColor(worldGrid::worldPixel(px, py));
                buf[static_cast<std::size_t>(ty) * TW + tx] =
                    (static_cast<std::uint32_t>(c.r * br / 255) << 24) | (static_cast<std::uint32_t>(c.g * br / 255) << 16)
                  | (static_cast<std::uint32_t>(c.b * br / 255) <<  8) |  0xFFu;
            }
        }
        SDL_UpdateTexture(tex, nullptr, buf.data(), TW * static_cast<int>(sizeof(std::uint32_t)));
        g_worldTex  = tex;
        g_worldSeed = worldSeed;
        g_worldDisc = disc;
        return tex;
    }

    //현재 뷰에 맞는 위성 윈도우 텍스처 확보(필요 시에만 재빌드). 출력: 덮는 rect 좌상단/해상도.
    //  재빌드 조건 = 시드 변경 / 해상도 어긋남 / 보이는 영역이 캐시 rect 밖. 줌 애니 중엔
    //  무조건 재사용(매프레임 빌드 회피 — rect로 블릿하므로 기하는 정확, 해상도만 잠깐 어긋).
    SDL_Texture* satelliteTexture(const MapView& v, double& oWX0, double& oWY0, double& oCPT)
    {
        if (!worldGrid::worldPixelMmapActive()) return nullptr;
        const std::size_t disc = mapDiscovery::count();   // 발견 상태 변화 감지(fog 베이크 무효화)

        //텍스처 크기 — 화면 비율, 1텍셀≈1화면픽셀 목표. (TW가 선명도/빌드비용 튜닝 포인트.)
        constexpr int TW = 1280;
        const int TH = std::max(1, (int)std::lround(TW * (double)v.viewH / std::max(1, v.viewW)));

        //덮을 월드 폭(청크) = 보이는폭 * 마진. 한 월드 폭은 넘지 않게 캡(wrap blit 회피).
        constexpr double marginF = 1.5;
        double coverW = (v.viewW / v.curScale) * marginF;
        coverW = std::min(coverW, static_cast<double>(WORLD_PIXEL_W) * 0.95);
        const double cpt    = std::max(1e-9, coverW / TW);   // chunks per texel
        const double coverH = TH * cpt;
        const double wx0 = v.centerPX - coverW * 0.5;
        const double wy0 = v.centerPY - coverH * 0.5;

        //재사용 판정 — 커버리지(가시영역이 캐시 rect 안)와 해상도(±25%)를 분리.
        const double visW = v.viewW / v.curScale, visH = v.viewH / v.curScale;
        const bool coversView = g_satTex
            && v.centerPX - visW * 0.5 >= g_satWX0 && v.centerPX + visW * 0.5 <= g_satWX0 + g_satTW * g_satCPT
            && v.centerPY - visH * 0.5 >= g_satWY0 && v.centerPY + visH * 0.5 <= g_satWY0 + g_satTH * g_satCPT;
        const bool resOK = g_satCPT > 0 && cpt >= g_satCPT * 0.8 && cpt <= g_satCPT * 1.25;

        //정지 시엔 커버+해상도 둘 다 맞을 때만 재사용(아니면 뷰에 맞게 재빌드 → 최선명).
        //  애니 중엔 *커버될 때만* 재사용하고 해상도 드리프트는 무시 — 줌인은 커버가 유지되니
        //  그대로 재사용(미세 텍스처를 화면에 축소/확대), 줌아웃은 뷰가 커져 커버를 잃는 순간
        //  재빌드해 풀스크린 커버를 복구한다(백드롭 뿌얘짐 제거). 커버 경계마다만 재빌드라
        //  매프레임 재빌드는 아님(원작자가 피하려던 비용은 그대로 회피).
        if (g_satTex && g_satSeed == worldSeed && g_satDisc == disc && coversView && (resOK || v.animating))
        {
            oWX0 = g_satWX0; oWY0 = g_satWY0; oCPT = g_satCPT;
            return g_satTex;
        }

        if (!g_satTex || g_satTW != TW || g_satTH != TH)
        {
            if (g_satTex) SDL_DestroyTexture(g_satTex);
            g_satTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, TW, TH);
            if (!g_satTex) return nullptr;
            SDL_SetTextureScaleMode(g_satTex, SDL_SCALEMODE_LINEAR);
            g_satTW = TW; g_satTH = TH;
        }

        static std::vector<std::uint32_t> buf;
        buf.assign(static_cast<std::size_t>(TW) * TH, 0);
        for (int ty = 0; ty < TH; ++ty)
        {
            const int  wy   = (int)std::lround(wy0 + ty * cpt);
            const bool yOOB = (wy < 0 || wy >= WORLD_PIXEL_H);
            for (int tx = 0; tx < TW; ++tx)
            {
                const int wx = worldWrap::wrapPixelX((int)std::lround(wx0 + tx * cpt));
                const worldGrid::Terrain t = yOOB ? worldGrid::Terrain::Sea : worldGrid::worldPixel(wx, wy);
                const Uint8 br = fogBright(wx, wy);   // 전장의 구름 — 미발견 청크 어둡게(청크맵과 동일)
                const SDL_Color c = worldTerrainColor(t);
                buf[static_cast<std::size_t>(ty) * TW + tx] =
                    (static_cast<std::uint32_t>(c.r * br / 255) << 24) | (static_cast<std::uint32_t>(c.g * br / 255) << 16)
                  | (static_cast<std::uint32_t>(c.b * br / 255) <<  8) |  0xFFu;
            }
        }
        SDL_UpdateTexture(g_satTex, nullptr, buf.data(), TW * static_cast<int>(sizeof(std::uint32_t)));
        g_satWX0 = wx0; g_satWY0 = wy0; g_satCPT = cpt; g_satSeed = worldSeed; g_satDisc = disc;
        oWX0 = wx0; oWY0 = wy0; oCPT = cpt;
        return g_satTex;
    }
}

//⑤-b 얼굴 말풍선 마커 — 흰 테두리 원 배지 안에 플레이어 얼굴, hasTail이면 tailAng 방향으로 꼬리(말풍선).
//   화면 밖 클램프(꼬리=플레이어 방향)와 위성맵 기본 마커(꼬리=아래, π/2)가 공유한다. 얼굴은 합성
//   텍스처 frame0의 얼굴 영역(중앙 16px의 살짝 위, FACE_SRC)을 구멍 중앙에 NEAREST 스케일 — 합성
//   텍스처는 투명 배경이라 사각 크롭이어도 원 밖으로 안 삐져 보인다(원형 클립 불요). 어두운 구멍 원이
//   얼굴 투명부를 메워 "원에 뚫린 구멍 속 얼굴"처럼 보인다. (composePlayerTexture는 끝에서 렌더타겟을
//   메인 타겟으로 복원하므로 맵 패스 중 호출해도 안전.)
static void drawFaceBubble(int cx, int cy, double radius, double tailAng, bool hasTail)
{
    if (PlayerPtr == nullptr) return;

    const SDL_Color rim    = { 255, 255, 255, 255 };   // 흰 테두리/꼬리
    const SDL_Color hole   = {  28,  30,  40, 255 };   // 구멍(어두운 배경)
    const double    margin = std::max(2.0, radius * 0.16);

    //꼬리 — 배지 중심에서 tailAng 방향 삼각형(흰색). 밑변은 중심 부근, 꼭짓점은 바깥(fillTri가
    //  이 함수보다 뒤라 SDL_RenderGeometry로 직접). 배지 원이 위에 덮어 밑변 안쪽은 가려진다.
    if (hasTail)
    {
        const double tip = radius * 1.7, half = radius * 0.55;
        const double ca = std::cos(tailAng), sa = std::sin(tailAng);
        const SDL_FColor fc = { 1.f, 1.f, 1.f, 1.f };
        SDL_Vertex tv[3] = {
            { { (float)(cx + ca * tip),  (float)(cy + sa * tip)  }, fc, { 0, 0 } },
            { { (float)(cx - sa * half), (float)(cy + ca * half) }, fc, { 0, 0 } },
            { { (float)(cx + sa * half), (float)(cy - ca * half) }, fc, { 0, 0 } } };
        int ti[3] = { 0, 1, 2 };
        SDL_RenderGeometry(renderer, nullptr, tv, 3, ti, 3);
    }

    //배지 — 흰 원(테두리) → 안쪽 어두운 원(구멍).
    drawFillCircle(cx, cy, (int)std::lround(radius),          rim,  255);
    drawFillCircle(cx, cy, (int)std::lround(radius - margin), hole, 255);

    //얼굴 — frame0 얼굴 크롭(FACE_SRC)을 구멍에 *원형*으로 클립. 화면 정원(반지름 rFace)을 triangle
    //  fan(원판 메시)으로 만들고 각 둘레 정점의 UV를 크롭 중앙 기준 같은 각도·반지름에 매핑 → src의
    //  내접원만 그려져 네모 모서리(어깨·머리카락 삐침)가 잘려나간다(SDL 사각 클립 한계 우회). 얼굴
    //  투명부는 BLEND로 구멍(어두운 원)이 비친다.
    const SDL_FRect FACE_SRC   = { 16.f, 6.f, 16.f, 16.f };   // frame0 내 얼굴 크롭(중앙 16px의 살짝 위) — 튜닝 가능
    constexpr double FACE_FILL = 1.0;                         // 구멍 지름 대비 얼굴 채움 비율 — 튜닝 가능
    SDL_Texture* faceTex = PlayerPtr->composePlayerTexture(false);   // 얼굴 말풍선은 눈 깜빡임 없음
    SDL_SetTextureBlendMode(faceTex, SDL_BLENDMODE_BLEND);
    {
        float texW, texH;
        SDL_GetTextureSize(faceTex, &texW, &texH);
        const float cu = (FACE_SRC.x + FACE_SRC.w * 0.5f) / texW;   // 크롭 중심 UV
        const float cv = (FACE_SRC.y + FACE_SRC.h * 0.5f) / texH;
        const float hu = (FACE_SRC.w * 0.5f) / texW;               // 크롭 반폭 UV(=내접원 반지름)
        const float hv = (FACE_SRC.h * 0.5f) / texH;
        const double rFace = (radius - margin) * FACE_FILL;
        constexpr int SEG = 32;
        const SDL_FColor fcw = { 1.f, 1.f, 1.f, 1.f };
        SDL_Vertex fv[SEG + 2];
        int        fi[SEG * 3];
        fv[0] = { { (float)cx, (float)cy }, fcw, { cu, cv } };      // 중심
        for (int i = 0; i <= SEG; ++i)
        {
            const double th = (double)i / SEG * (std::numbers::pi * 2.0);
            const double c = std::cos(th), s = std::sin(th);
            fv[i + 1] = { { (float)(cx + c * rFace), (float)(cy + s * rFace) }, fcw,
                          { cu + (float)c * hu, cv + (float)s * hv } };
        }
        for (int i = 0; i < SEG; ++i) { fi[i * 3] = 0; fi[i * 3 + 1] = i + 1; fi[i * 3 + 2] = i + 2; }
        SDL_RenderGeometry(renderer, faceTex, fv, SEG + 2, fi, SEG * 3);
    }
    SDL_DestroyTexture(faceTex);
}

static void drawWorldView(const MapView& v)
{
    SDL_Texture* bg = worldMapTexture();   // 전세계 백드롭 — 항상 화면을 덮어 검정 방지
    if (!bg)
    {
        setFont(fontType::mainFont);
        setFontSize(28);
        drawTextOutlineCenter(L"No world data yet", v.viewW / 2, v.viewH / 2, mappal::uiText());
        return;
    }

    //① 백드롭 — 전세계 텍스처를 통합 카메라로 wrap-타일 블릿. fit에서 1:1 선명, 줌인 시
    //  저해상도 배경이 되어 윈도우 미커버/로딩중 영역을 검정 대신 채운다(깜빡임 제거).
    {
        const double worldWpx = static_cast<double>(WORLD_PIXEL_W) * v.curScale;
        const double topY     = v.sY(0.0);
        const double height   = static_cast<double>(WORLD_PIXEL_H) * v.curScale;
        double originX = v.viewW * 0.5 - v.centerPX * v.curScale;
        originX = std::fmod(originX, worldWpx);
        if (originX > 0) originX -= worldWpx;
        for (double ox = originX; ox < v.viewW; ox += worldWpx)
        {
            SDL_FRect dst = { static_cast<float>(ox), static_cast<float>(topY),
                              static_cast<float>(worldWpx), static_cast<float>(height) };
            SDL_RenderTexture(renderer, bg, nullptr, &dst);
        }
    }

    //② 고해상도 윈도우 — 광역(coverW≥월드폭/2)이 아니면 백드롭 위에 덧그려 선명. 줌 애니 중에도
    //  그린다: satelliteTexture가 애니 중 커버를 보장(커버 잃으면 재빌드)하므로 항상 화면을 덮어
    //  줌인/줌아웃 모두 백드롭 뿌얘짐이 안 보인다. (정지 시엔 뷰에 정확히 맞게 재빌드되어 최선명.)
    if ((v.viewW / v.curScale) * 1.5 < static_cast<double>(WORLD_PIXEL_W) * 0.5)
    {
        double wx0, wy0, cpt;
        if (SDL_Texture* win = satelliteTexture(v, wx0, wy0, cpt))
        {
            const double left = v.sX(wx0);
            const double top  = v.sY(wy0);
            const double w    = g_satTW * cpt * v.curScale;
            const double h    = g_satTH * cpt * v.curScale;
            SDL_FRect dst = { static_cast<float>(left), static_cast<float>(top),
                              static_cast<float>(w), static_cast<float>(h) };
            SDL_RenderTexture(renderer, win, nullptr, &dst);
        }
    }
}

//⑤-b 위성 뷰 도시 점/라벨 — drawWorldView에서 분리. 야간 오버레이 *뒤*에 그려 점/라벨은 조명
//  영향을 안 받는다(밤에도 또렷, 청크맵 라벨과 동일 정책). 점: 발견한 도시만(전장의 구름 핵심 —
//  지형색은 도시 위치를 숨기므로 점이 유일한 노출). 이름: 사전마킹(preset, codename!=none) 도시만
//  (발견 무관) — 전세계 조망 시 방향 앵커. 절차생성 자리표시자 이름은 위성지도에서 어색해 제외(이름은
//  청크맵에서만). → 미발견 preset은 이름만 떠 위치 힌트, 발견 도시는 점, 발견 preset은 점+이름.
static void drawWorldCities(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    setFont(fontType::pixel);
    setFontSize(12);
    for (const auto& node : *cities)
    {
        const bool seen   = mapDiscovery::discovered(tilePixelIX(node.center.x), tilePixelIY(node.center.y));
        const bool preset = (node.codename != city::CityName::none);
        if (!seen && !preset) continue;

        const int sx = (int)std::lround(v.sX(tileToPixelX(node.center.x)));
        const int sy = (int)std::lround(v.sY(tileToPixelY(node.center.y)));
        if (sx < -8 || sx > v.viewW + 8 || sy < -8 || sy > v.viewH + 8) continue;

        if (seen)
        {
            drawFillCircle(sx, sy, 3, SDL_Color{ 245, 222, 120, 255 }, 255);
            drawFillCircle(sx, sy, 2, SDL_Color{  70,  45,  15, 255 }, 255);
        }

        if (preset)
        {
            const std::wstring name = presetDisplayName(node.codename);
            if (!name.empty())
                drawTextOutlineCenter(name, sx, sy - 13, SDL_Color{ 245, 222, 120, 255 });
        }
    }
    setFont(fontType::mainFont);
}

//⑤-c 위성 뷰 플레이어 마커 — 얼굴 말풍선. 화면 안이면 배지를 위치 위로 올려 꼬리가 아래(위치)를
//  가리키고, 화면 밖이면 플레이어 방향 꼬리 + 가장자리 클램프. (위성맵은 광역이라 항상 말풍선.)
//  drawWorldView에서 분리 — 야간 오버레이 *뒤*에 그려 말풍선은 조명 영향을 안 받는다(밤에도 또렷).
static void drawWorldPlayerMarker(const MapView& v)
{
    const double pxd = v.sX(tileToPixelX(PlayerX()));
    const double pyd = v.sY(tileToPixelY(PlayerY()));
    constexpr double R = 26.0, M = 16.0;
    if (pxd < 0 || pxd > v.viewW || pyd < 0 || pyd > v.viewH)
    {
        const double ang = std::atan2(pyd - v.viewH * 0.5, pxd - v.viewW * 0.5);
        const int ex = (int)std::clamp(pxd, M + R, v.viewW - M - R);
        const int ey = (int)std::clamp(pyd, M + R, v.viewH - M - R);
        drawFaceBubble(ex, ey, R, ang, true);
    }
    else
    {
        const int bx = (int)std::lround(pxd);
        const int by = (int)std::lround(pyd - R * 1.5);   // 배지를 위로 올려 꼬리 끝이 위치를 가리킴
        drawFaceBubble(bx, by, R, std::numbers::pi * 0.5, true);   // 꼬리 아래(π/2)
    }
}

//⑤ 플레이어 마커 — 화면 안이면 펄스, 밖이면 가장자리 클램프.
static void drawPlayerMarker(const MapView& v)
{
    //플레이어 위치를 자신이 속한 청크 칸 *중앙*으로 양자화 — 건물 심볼과 동일하게 청크 그리드에
    //  스냅(청크 내 세부 타일 위치는 무시). tilePixelIX/IY=청크 인덱스, +0.5=칸 중앙.
    const double ppX = tilePixelIX(PlayerX()) + 0.5;
    const double ppY = tilePixelIY(PlayerY()) + 0.5;
    const double sxd = v.sX(ppX);
    const double syd = v.sY(ppY);

    //화면 밖 — 빨간 사각 대신 얼굴 말풍선(꼬리=플레이어 방향)을 가장자리로 클램프.
    if (sxd < 0 || sxd > v.viewW || syd < 0 || syd > v.viewH)
    {
        constexpr double R = 28.0, M = 16.0;
        const double ang = std::atan2(syd - v.viewH * 0.5, sxd - v.viewW * 0.5);
        const int ex = (int)std::clamp(sxd, M + R, (double)v.viewW - M - R);
        const int ey = (int)std::clamp(syd, M + R, (double)v.viewH - M - R);
        drawFaceBubble(ex, ey, R, ang, true);
        return;
    }

    const int sx = (int)std::round(sxd);
    const int sy = (int)std::round(syd);

    //프로토타입 — 빨간 점/펄스는 안 그리고, 플레이어 캐릭터 스프라이트 *자체*가 깜빡이는 마커.
    //  캐릭터 본체 bbox는 프레임 중앙 ~16x16(건물 art와 동일 크기, 머리만 위로 빼꼼)이라 건물 심볼과
    //  같은 zoomScale 배율로 칸 중심에 그리면 본체가 1청크에 꽉 찬다(프레임 중심 24,24 ≈ 본체 중심).
    //  단 줌아웃에선 본체(=16*z px)가 너무 작아지지 않게 화면 본체폭 하한(MARKER_MIN_BODY_PX)으로 살짝 키움.
    //  composePlayerTexture는 매 호출 288x384 TARGET 텍스처를 새로 만들고 끝에서 렌더타겟을 nullptr
    //  (=맵이 그려지는 메인 타겟)로 복원하므로 맵 패스 중 호출해도 안전. Sprite takeOwnership=true →
    //  스코프 끝에서 SDL_DestroyTexture(매 프레임 1개, 프로토타입 OK·추후 캐시 가능).
    //  frame 0 = charSprIndex::WALK(정면 idle — 상태창 포트레이트와 동일).
    //깜빡임 — 켜짐 구간(900ms 중 600ms)만 캐릭터를 그리고 흐림 구간은 완전히 사라짐(on/off).
    //  켜짐 구간만 합성·드로우라 비용도 절약. 합성 텍스처는 BLEND 명시(투명 배경 처리, 매 프레임 새 텍스처).
    if (PlayerPtr != nullptr && (SDL_GetTicks() % 900) < 600)
    {
        constexpr double MARKER_MIN_BODY_PX = 14.0;   // 줌아웃 시 본체 최소 화면폭(px) — 튜닝 가능
        const float z = (float)std::max(v.zoomScale(), MARKER_MIN_BODY_PX / 16.0);
        SDL_Texture* charTex = PlayerPtr->composePlayerTexture(false);   // 맵 마커는 눈 깜빡임 없음
        Sprite charSpr(renderer, charTex, 48, 48, true);
        SDL_SetTextureBlendMode(charTex, SDL_BLENDMODE_BLEND);
        setZoom(z);
        drawSpriteCenter(&charSpr, charSprIndex::WALK, sx, sy);
        setZoom(1.0f);
    }
}


// ────────── ⑧ 맵 핀 (플레이어 웨이포인트 = 빛의 기둥) ──────────
//  타일점에서 위로 솟는 발광 기둥. 크기는 줌(curScale)에 비례 가변(클램프) — 당길수록 커지고 멀수록 작게.
//  애니는 차분하게(호흡 + 바닥 원형 펄스). 메뉴 색칩은 단색 위치핀 기호(원+삼각). SDL 원시함수만(신규 아트 0).

static SDL_Color mpTint(SDL_Color c, double f)
{
    return SDL_Color{ (Uint8)(c.r + (255 - c.r) * f), (Uint8)(c.g + (255 - c.g) * f), (Uint8)(c.b + (255 - c.b) * f), 255 };
}

//단색 삼각형 채움(SDL_RenderGeometry) — 위치핀 기호의 끝(아래 삼각)용.
static void fillTri(float x0, float y0, float x1, float y1, float x2, float y2, SDL_Color c)
{
    const SDL_FColor fc = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f };
    SDL_Vertex v[3] = { { { x0, y0 }, fc, { 0, 0 } }, { { x1, y1 }, fc, { 0, 0 } }, { { x2, y2 }, fc, { 0, 0 } } };
    int idx[3] = { 0, 1, 2 };
    SDL_RenderGeometry(renderer, nullptr, v, 3, idx, 3);
}

//세로 그라디언트 빔(빛의 기둥 1겹) — 바닥(baseY)에서 위(topY)로 알파가 아래→위로 옅어지는 사다리꼴.
//  SDL_RenderGeometry 쿼드(2삼각형) + 정점 알파. 알파<255라 BLEND 모드 보장.
static void gradBeam(float cx, float baseY, float topY, float halfBase, float halfTop, SDL_Color c, Uint8 aBase, Uint8 aTop)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const SDL_FColor cb = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, aBase / 255.0f };
    const SDL_FColor ct = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, aTop  / 255.0f };
    SDL_Vertex v[4] = {
        { { cx - halfBase, baseY }, cb, { 0, 0 } },
        { { cx + halfBase, baseY }, cb, { 0, 0 } },
        { { cx + halfTop,  topY  }, ct, { 0, 0 } },
        { { cx - halfTop,  topY  }, ct, { 0, 0 } },
    };
    int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(renderer, nullptr, v, 4, idx, 6);
}

//타원 외곽선(바닥 원형 펄스용) — 선분 근사. 채움이 아니라 링이라 "퍼지는 파동"으로 읽힌다.
static void ellipseRing(int cx, int cy, double rx, double ry, SDL_Color c, Uint8 a)
{
    if (rx < 1.0 || a == 0) return;
    const int seg = std::max(16, (int)std::lround(rx * 1.6));
    double px = cx + rx, py = cy;
    for (int i = 1; i <= seg; ++i)
    {
        const double ang = (double)i / seg * (std::numbers::pi * 2.0);
        const double nx = cx + std::cos(ang) * rx;
        const double ny = cy + std::sin(ang) * ry;
        drawLine((int)std::lround(px), (int)std::lround(py), (int)std::lround(nx), (int)std::lround(ny), c, a);
        px = nx; py = ny;
    }
}

//빛의 기둥 마커 — 타일점(ax,ay)에서 위로 솟는 발광 기둥 + 바닥 글로우 풀. 애니는 차분하게:
//  ① 전체가 밝아졌다 어두워지는 호흡 ② 바닥에서 천천히 퍼지는 원형(타원) 펄스 1겹. (상승 펄스는 과해서 폐기.)
//  scale=화면 px/청크(curScale). 크기를 여기 비례시키되 클램프 → 어느 줌에서도 자연스러운 크기.
static void drawBeacon(int ax, int ay, SDL_Color col, double scale)
{
    const double W = std::clamp(scale * 0.34, 3.0, 26.0);     // 기준 반폭(줌 가변)
    const double H = std::clamp(scale * 2.60, 24.0, 150.0);   // 기둥 높이(줌 가변)
    const float  cx = (float)ax, baseY = (float)ay, topY = (float)(ay - H);
    const SDL_Color hot = mpTint(col, 0.55);                  // 중심부 밝은 틴트

    const double now = (double)SDL_GetTicks();
    const double breath = 0.82 + 0.18 * std::sin(now / 1700.0 * 2.0 * std::numbers::pi);   // 전체 강도 호흡
    auto A = [&](double a) -> Uint8 { return (Uint8)std::clamp(a * breath, 0.0, 255.0); };

    //바닥 글로우 풀 — 빛이 바닥에 고인 동심 타원(중심 밝게).
    drawFillEllipse(ax, ay, (int)std::lround(W * 2.4), (int)std::lround(W * 1.0),  col, A(55));
    drawFillEllipse(ax, ay, (int)std::lround(W * 1.5), (int)std::lround(W * 0.62), col, A(110));
    drawFillEllipse(ax, ay, (int)std::lround(W * 0.8), (int)std::lround(W * 0.34), hot, A(190));

    //기둥 — 겉(넓고 옅음) → 중간 → 중심부(좁고 밝음). 위로 갈수록 알파 0.
    gradBeam(cx, baseY, topY, (float)(W * 1.8),  (float)(W * 1.3), col, A(40),  0);
    gradBeam(cx, baseY, topY, (float)(W * 1.0),  (float)(W * 0.7), col, A(115), 0);
    gradBeam(cx, baseY, topY, (float)(W * 0.42), (float)(W * 0.3), hot, A(200), 0);

    //바닥 원형 펄스 — 바닥에서 바깥으로 천천히 퍼지며 사라지는 타원 링 1겹(지면 파동). 차분.
    {
        const double pp = std::fmod(now, 2200.0) / 2200.0;       // 0..1 (약 2.2초 주기)
        const double rx = W * (1.1 + pp * 2.6);
        const Uint8  pa = (Uint8)std::lround((1.0 - pp) * 100.0); // 퍼질수록 옅게
        ellipseRing(ax, ay, rx, rx * 0.42, col, pa);
    }
}

//화면 밖 핀의 가장자리 방향 표시 — 플레이어 마커 off-screen 처리와 동일 컨셉(가장자리로 클램프).
//  미니맵 rim 화살표와 같은 결: 흰 테 없이 색 글로우 + 바깥 방향 화살표 + 밝은 중심. (앵커가 화면 밖일 때)
static void drawMapPinEdge(const MapView& v, double sxd, double syd, SDL_Color col)
{
    constexpr double margin = 22.0;
    const double ang = std::atan2(syd - v.viewH * 0.5, sxd - v.viewW * 0.5);   // 화면 중심→핀 방향
    const int ex = (int)std::clamp(sxd, margin, v.viewW - margin);
    const int ey = (int)std::clamp(syd, margin, v.viewH - margin);
    const SDL_Color hot = mpTint(col, 0.45);

    drawFillCircle(ex, ey, 10, col, 50);    // 글로우(바깥)
    drawFillCircle(ex, ey, 6,  col, 110);   // 글로우(안쪽)

    const double L = 12.0, halfW = 7.5, back = 3.0;   // 바깥을 가리키는 화살표
    const float tipX = ex + (float)(std::cos(ang) * L),    tipY = ey + (float)(std::sin(ang) * L);
    const float bX   = ex - (float)(std::cos(ang) * back), bY   = ey - (float)(std::sin(ang) * back);
    const float pX   = (float)(-std::sin(ang) * halfW),    pY   = (float)(std::cos(ang) * halfW);
    fillTri(tipX, tipY, bX + pX, bY + pY, bX - pX, bY - pY, hot);
    drawFillCircle(ex, ey, 3, hot, 255);    // 밝은 중심
}

//⑧ 맵 핀 — 색상별 마커. 현재 보는 z의 색마다 빛의 기둥으로 그림(크기는 v.curScale 비례, 가변).
//  앵커(핀 타일점)가 화면 안이면 기둥, 밖이면 플레이어 마커처럼 가장자리 방향 표시(drawMapPinEdge).
//  sX(tileToPixelX)/sY(tileToPixelY) 환산 — 핀 x가 청크 중심 타일이라 셀 한가운데에 앉는다.
static void drawMapPins(const MapView& v)
{
    for (int i = 0; i < MAP_PIN_COLOR_COUNT; ++i)
    {
        if (!mapPins[i] || mapPins[i]->z != v.z) continue;
        const double sxd = v.sX(tileToPixelX(mapPins[i]->x));
        const double syd = v.sY(tileToPixelY(mapPins[i]->y));
        const SDL_Color col = mapPinColor(i);
        if (sxd < 0 || sxd > v.viewW || syd < 0 || syd > v.viewH)
            drawMapPinEdge(v, sxd, syd, col);   // 화면 밖 → 가장자리 방향 표시
        else
            drawBeacon((int)std::lround(sxd), (int)std::lround(syd), col, v.curScale);
    }
}


//⑨ 야간 조명 오버레이 — 본체 월드 타일이 시간대별로 받는 곱셈(MUL) 틴트(밤=남색)를 월드맵에도
//   똑같이 적용. 타일별 배치인 본체(renderTile::drawMulFogs)와 달리 맵은 화면 전체에 색이 균일하므로
//   풀스크린 사각형 1장이면 동일 결과(같은 색·같은 MUL 블렌드). 틴트 색은 constVar:colors가 단일 출처.
//   지형·심볼 위에 깔되 라벨·플레이어·핀·UI보다 아래라 정보/마커 가독성은 유지된다(빛기둥은 밤에 빛남).
static void drawNightOverlay(const MapView& v)
{
    const SDL_Color c = mulCol::ambientMulColorAt(getHour() + getMin() / 60.0f);
    if (c.a == 0) return;   //낮 — 틴트 없음(곱셈 항등)

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MUL);
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_FRect full = { 0.0f, 0.0f, (float)v.viewW, (float)v.viewH };
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);   //블렌드 모드 원복(다음 패스 오염 방지)
}


// ════════════════════════════════════════════════════════════════════════
// §5  UI 크롬
// ════════════════════════════════════════════════════════════════════════

struct ZoomButtons { SDL_Rect zoomIn, zoomOut, home; };

static ZoomButtons computeZoomButtons()
{
    constexpr int margin = 22, btnSize = 64, gap = 10;
    int yBase = cameraH - margin - btnSize;
    return {
        { margin,                       yBase, btnSize, btnSize },
        { margin + btnSize + gap,       yBase, btnSize, btnSize },
        { margin + (btnSize + gap) * 2, yBase, btnSize, btnSize }
    };
}

static void drawZoomPanel(const MapView& v, const ZoomButtons& zb)
{
    constexpr int margin = 22;
    int btnSize = zb.zoomIn.w;
    constexpr int gap = 10;
    constexpr int coordRow = 24;   // 라벨과 버튼 사이에 끼울 좌표 한 줄 높이

    //플레이어 좌표 한 줄 — 라벨(주황) + 값(흰색). 폭을 미리 측정해 패널 너비에 반영.
    const wchar_t* clabels[3] = { L"X", L"Y", L"Z" };
    const int      cvals[3]   = { PlayerX(), PlayerY(), PlayerZ() };
    setFontSize(16);
    int coordW = 0;
    for (int i = 0; i < 3; ++i)
    {
        setFont(fontType::mainFontSemiBold); coordW += queryTextWidth(std::wstring(clabels[i])) + 4;
        setFont(fontType::mainFontBold);     coordW += queryTextWidth(std::to_wstring(cvals[i]));
        if (i < 2) coordW += 14;
    }

    //버튼은 computeZoomButtons가 화면 하단 기준으로 고정 → 패널만 위로 coordRow만큼 키워 좌표 줄을 끼운다.
    int panelW = std::max((btnSize + gap) * 3 + 18, coordW + 28);
    int panelH = btnSize + 30 + 14 + coordRow;
    int panelX = margin - 9;
    int panelY = cameraH - margin - btnSize - 36 - coordRow;
    drawStadium(panelX, panelY, panelW, panelH, mappal::uiPanel(), 220, 3);

    //① 줌 라벨(또는 Satellite).
    setFont(fontType::mainFont);
    setFontSize(16);
    std::wstring label = v.worldLOD()
        ? std::wstring(L"Satellite")
        : L"Zoom  " + std::to_wstring((int)std::lround(v.curScale)) + L" px/chunk";
    drawText(label, panelX + 14, panelY + 8, mappal::uiText());

    //② 좌표 한 줄 — 라벨 아래, 버튼 위. 라벨=주황(Status #e1772e), 값=밝은 흰색.
    {
        int hx = panelX + 14;
        const int cy = panelY + 8 + coordRow;
        for (int i = 0; i < 3; ++i)
        {
            setFont(fontType::mainFontSemiBold);
            drawText(clabels[i], hx, cy, SDL_Color{ 0xe1, 0x77, 0x2e, 255 });
            hx += queryTextWidth(std::wstring(clabels[i])) + 4;
            setFont(fontType::mainFontBold);
            const std::wstring val = std::to_wstring(cvals[i]);
            drawText(val, hx, cy, SDL_Color{ 240, 240, 234, 255 });
            hx += queryTextWidth(val) + 14;
        }
    }

    //③ 줌 버튼.
    auto button = [&](const SDL_Rect& r, const std::wstring& glyph)
        {
            SDL_Color fill = { 35, 35, 45, 255 };
            if (checkCursor(&r)) fill = click ? lowCol::deepBlue : lowCol::blue;
            drawStadium(r.x, r.y, r.w, r.h, fill, 240, 3);
            drawRect(r, mappal::uiBorder());
            setFontSize(28);
            drawTextCenter(glyph, r.x + r.w / 2, r.y + r.h / 2 - 2, mappal::uiText());
        };
    button(zb.zoomIn,  L"+");
    button(zb.zoomOut, L"-");
    button(zb.home,    L"@");
}

static void drawTabButton()
{
    SDL_Color btnColor = mappal::uiPanel();
    btnColor.a = 255;
    if (checkCursor(&tab))
    {
        if (click == false) btnColor = lowCol::blue;
        else                btnColor = lowCol::deepBlue;
    }
    drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 220, 5);
    setZoom(1.5);
    drawSpriteCenter(spr::icon48, 182, tab.x + 90, tab.y + 78);
    setZoom(1.0);
    setFontSize(22);
    drawTextCenter(sysStr[31], tab.x + 90, tab.y + 150);
    drawSpriteCenter(spr::keyboardButtons,
        keyboardIndex::tab + SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_TAB],
        tab.x + 164, tab.y + 8);
}

//청크 그리드 마커 — gridMarker(16px)를 zoomScale로 스케일 → 정확히 1청크 셀(16*zoomScale=curScale).
//  펄스 알파는 게임 화면 마커(HUD)와 동일 sin 곡선. (px,py)=대상 청크 픽셀 인덱스. hover·메뉴 타겟 공용.
static void drawChunkMarker(const MapView& v, int px, int py)
{
    const double rat = std::fmod(static_cast<double>(SDL_GetTicks()) / 1500.0, 1.0);
    const double na  = (std::sin(rat * 2.0 * 3.14159265358979) + 1.0) * 0.5;
    const Uint8  a   = static_cast<Uint8>(100 + na * (255 - 100));

    setZoom(static_cast<float>(v.zoomScale()));
    SDL_SetTextureBlendMode(spr::gridMarker->getTexture(), SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(spr::gridMarker->getTexture(), a);
    drawSpriteCenter(spr::gridMarker, 0,
        (int)std::lround(v.sX(px + 0.5)), (int)std::lround(v.sY(py + 0.5)));
    SDL_SetTextureAlphaMod(spr::gridMarker->getTexture(), 255);
    setZoom(1.0f);
}

//⑦ 마우스 hover — 가리키는 청크에 타일 마커(게임 화면과 동일 gridMarker, 펄스 알파) + 그 청크에
//   건물이 있으면 마우스 우측하단에 이름 툴팁(반투명 검정 배경). 청크맵 분기에서만 호출 —
//   위성 LOD(광역 조망)에선 미호출(마커/툴팁 불필요). 건물 탐색은 hover 청크를 bbox로 감싸는
//   도시만 심볼 스캔(drawCityBuildings와 동일 footprint 규약: pos=좌상단, w×h 청크). 미발견 건물은
//   "Unknown"(?건물 스프라이트와 정합 — 종류는 가봐야 드러남).
static void drawHoverInfo(const MapView& v)
{
    //UI 크롬(탭/줌 버튼) 위에선 hover 비활성 — 툴팁/마커가 버튼을 덮지 않게.
    ZoomButtons zb = computeZoomButtons();
    if (checkCursor(&tab) || checkCursor(&zb.zoomIn) || checkCursor(&zb.zoomOut) || checkCursor(&zb.home))
        return;

    const int mx = (int)getMouseX();
    const int my = (int)getMouseY();

    //스크린 → 청크 픽셀 역변환(sX/sY의 역). X는 원기둥 wrap, Y는 월드 범위 밖이면 공허 → 스킵.
    const double pxF = v.centerPX + (mx - v.viewW * 0.5) / v.curScale;
    const double pyF = v.centerPY + (my - v.viewH * 0.5) / v.curScale;
    const int px = (int)std::floor(pxF);
    const int py = (int)std::floor(pyF);
    if (py < 0 || py >= WORLD_PIXEL_H) return;

    //① 타일 마커 — hover 청크에 그리드 마커.
    drawChunkMarker(v, px, py);

    //② 건물 이름 — hover 청크를 footprint로 덮는 심볼 탐색. 발견=실제 종류, 미발견=Unknown.
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    const int wpx = worldWrap::wrapPixelX(px);   // 건물 anchor(apx)는 wrap된 좌표
    std::wstring label;
    for (std::size_t i = 0; i < cities->size() && label.empty(); ++i)
    {
        const auto& node = (*cities)[i];
        if (node.center.z != v.z) continue;
        //이 도시 bbox가 hover 청크를 안 감싸면 심볼 스캔 생략(값싼 컬링).
        if (wpx < node.bboxPx || wpx >= node.bboxPx + node.bboxW ||
            py  < node.bboxPy || py  >= node.bboxPy + node.bboxH) continue;

        const std::vector<CitySymbol>* syms = symbolsFor(static_cast<city::CityId>(i));
        if (!syms) continue;

        for (const auto& sym : *syms)
        {
            if (sym.pos.z != v.z) continue;
            const int apx = tilePixelIX(sym.pos.x);
            const int apy = tilePixelIY(sym.pos.y);
            if (wpx < apx || wpx >= apx + sym.w || py < apy || py >= apy + sym.h) continue;

            label = (debugRevealSymbols || mapDiscovery::discovered(apx, apy)) ? mapSymbolName(sym.symbol) : L"Unknown";
            break;
        }
    }

    //②-b 교외 사이트 — 도시 bbox 밖이라 위 루프에 안 걸림. activeSites 선형 스캔
    //   (수천 개 footprint 포함검사, hover 시 프레임당 1회 — 무해).
    if (label.empty() && worldGen::activeSites != nullptr)
    {
        for (const auto& site : *worldGen::activeSites)
        {
            if (site.pos.z != v.z) continue;
            const int apx = tilePixelIX(site.pos.x);
            const int apy = tilePixelIY(site.pos.y);
            if (wpx < apx || wpx >= apx + site.w || py < apy || py >= apy + site.h) continue;

            label = (debugRevealSymbols || mapDiscovery::discovered(apx, apy)) ? mapSymbolName(site.symbol) : L"Unknown";
            break;
        }
    }
    if (label.empty()) return;

    //③ 툴팁 — 마우스 우측하단. 반투명 검정 배경 + 흰 글씨. 화면 밖으로 넘치면 반대쪽으로 클램프.
    setFont(fontType::mainFont);
    setFontSize(18);
    const int tw = queryTextWidth(label);
    const int th = queryTextHeight(label);
    constexpr int padX = 8, padY = 5, off = 18;

    int bx = mx + off;
    int by = my + off;
    if (bx + tw + padX * 2 > v.viewW) bx = mx - off - (tw + padX * 2);   // 우측 넘침 → 좌측
    if (by + th + padY * 2 > v.viewH) by = v.viewH - (th + padY * 2);    // 하단 넘침 → 위로 당김
    if (bx < 0) bx = 0;
    if (by < 0) by = 0;

    drawFillRect(SDL_Rect{ bx, by, tw + padX * 2, th + padY * 2 }, SDL_Color{ 0, 0, 0, 255 }, (Uint8)190);
    drawText(label, bx + padX, by + padY, mappal::uiText());
    setFont(fontType::mainFont);
}


// ════════════════════════════════════════════════════════════════════════
// §6  Map 클래스
// ════════════════════════════════════════════════════════════════════════

export class Map : public GUI
{
private:
    inline static Map* ptr = nullptr;
    MapView view;

    bool   dragging      = false;
    bool   dragMoved     = false;
    double dragAnchorPX  = 0.0;
    double dragAnchorPY  = 0.0;

    //핀 컨텍스트 메뉴 상태 — 청크맵에서 우클릭 시 열림. 타겟 타일 좌표(찍을 위치)와 레이아웃 rect 보관.
    //  Map이 항상 최상단 GUI이므로 별도 GUI 서브클래스 없이 Map 안에서 모달처럼 직접 처리(드래그/줌 억제).
    bool     pinMenuOpen  = false;
    int      pinMenuTileX = 0;
    int      pinMenuTileY = 0;
    int      pinMenuZ     = 0;
    SDL_Rect pinMenuRect{};
    std::array<SDL_Rect, MAP_PIN_COLOR_COUNT> pinSwatch{};

    //줌은 Map 인스턴스 수명 넘어 영속. 센터/Z는 매 열기마다 플레이어 기준 리셋.
    inline static int persistedZoom = mapcfg::DEFAULT_ZOOM;

public:
    Map() : GUI(false)
    {
        errorBox(ptr != nullptr, L"More than one Map instance was generated.");
        ptr = this;

        view.viewW = cameraW;
        view.viewH = cameraH;
        view.z = PlayerZ();
        view.centerPX = tileToPixelX(PlayerX());
        view.centerPY = tileToPixelY(PlayerY());
        view.zoomLevel   = persistedZoom;
        view.targetScale = view.levelScale(persistedZoom);
        view.curScale    = view.targetScale;   // 열 때는 애니 없이 그 줌부터
        view.animating   = false;
        x = 0; y = 0;

        deactInput();
        deactDraw();
        addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
    }

    ~Map() { persistedZoom = view.zoomLevel; ptr = nullptr; }

    static Map* ins() { return ptr; }

    void changeXY(int /*ix*/, int /*iy*/, bool /*center*/) override { x = 0; y = 0; }

    // ────────── 핀 컨텍스트 메뉴 ──────────

    //메뉴 박스/스와치/제거버튼 rect 배치 — 앵커(mx,my) 우하단에 펼치되 화면 밖이면 반대로 클램프.
    //  레이아웃: 좌표 한 줄 → "MAP PIN" 헤더(버튼 위, 양옆 구분선) → 색 버튼 줄(서로 떨어짐) → (마커 있으면)제거.
    //  좌표 폭을 그릴 때와 동일 폰트로 측정해 패널 너비 결정(버튼 줄과 max). 상수는 drawPinMenu와 공유.
    void layoutPinMenu(int mx, int my)
    {
        constexpr int padX = 14, padTop = 9, padBot = 12, coordH = 24, headerH = 18;
        constexpr int btn = 48, btnGap = 12, gapCoordHeader = 4, gapHeaderBtn = 8, groupGap = 16, labelGap = 4;

        setFontSize(17);
        const wchar_t* labels[3] = { L"X", L"Y", L"Z" };
        const int      vals[3]   = { pinMenuTileX, pinMenuTileY, pinMenuZ };
        int coordW = 0;
        for (int i = 0; i < 3; ++i)
        {
            setFont(fontType::mainFontSemiBold); coordW += queryTextWidth(labels[i]) + labelGap;
            setFont(fontType::mainFontBold);     coordW += queryTextWidth(std::to_wstring(vals[i]));
            if (i < 2) coordW += groupGap;
        }
        setFont(fontType::mainFont);

        const int swatchRowW = MAP_PIN_COLOR_COUNT * btn + (MAP_PIN_COLOR_COUNT - 1) * btnGap;
        const int contentW   = std::max(swatchRowW, coordW);
        const int panelW     = contentW + padX * 2;
        const int swatchTop  = padTop + coordH + gapCoordHeader + headerH + gapHeaderBtn;
        const int panelH     = swatchTop + btn + padBot;

        int bx = mx + 6, by = my + 6;
        if (bx + panelW > cameraW) bx = cameraW - panelW - 4;
        if (by + panelH > cameraH) by = cameraH - panelH - 4;
        if (bx < 4) bx = 4;
        if (by < 4) by = 4;
        pinMenuRect = { bx, by, panelW, panelH };

        const int sx0 = bx + (panelW - swatchRowW) / 2;
        for (int i = 0; i < MAP_PIN_COLOR_COUNT; ++i)
            pinSwatch[i] = { sx0 + i * (btn + btnGap), by + swatchTop, btn, btn };
    }

    //우클릭 → 메뉴 열기. 청크맵에서만, UI 크롬 위는 무시. 타겟 청크 중심 타일을 좌표로 잡는다.
    void openPinMenu(int mx, int my)
    {
        if (view.worldLOD()) return;   // 위성 LOD가 아닌 청크맵 단계에서만
        ZoomButtons zb = computeZoomButtons();
        if (checkCursor(&tab) || checkCursor(&zb.zoomIn) || checkCursor(&zb.zoomOut) || checkCursor(&zb.home)) return;

        //스크린 → 청크 픽셀 역변환(drawHoverInfo와 동일). X는 원기둥 wrap, Y 월드 밖이면 무시.
        const double pxF = view.centerPX + (mx - view.viewW * 0.5) / view.curScale;
        const double pyF = view.centerPY + (my - view.viewH * 0.5) / view.curScale;
        const int px = (int)std::floor(pxF);
        const int py = (int)std::floor(pyF);
        if (py < 0 || py >= WORLD_PIXEL_H) return;
        const int wpx = worldWrap::wrapPixelX(px);

        //청크 중심 타일 좌표 — tileToPixelX로 되돌리면 정확히 이 셀 중심으로 환산된다.
        pinMenuTileX = TILE_BASE_X + wpx * TILE_PER_PIXEL + TILE_PER_PIXEL / 2;
        pinMenuTileY = TILE_BASE_Y + py  * TILE_PER_PIXEL + TILE_PER_PIXEL / 2;
        pinMenuZ     = view.z;

        layoutPinMenu(mx, my);
        pinMenuOpen = true;
    }

    //버튼 1개 — 좌측하단 줌 패널(+/-/@)과 동일 스타일: drawStadium(채움,240,edge3) + drawRect(테두리).
    //  base {35,35,45} / 호버 lowCol::blue / 누름 lowCol::deepBlue. active=그 색 마커가 어딘가에 사용 중(배경 deepBlue).
    void pinButton(const SDL_Rect& r, bool active)
    {
        SDL_Color fill = active ? lowCol::deepBlue : SDL_Color{ 35, 35, 45, 255 };
        if (checkCursor(&r)) fill = click ? lowCol::deepBlue : lowCol::blue;
        drawStadium(r.x, r.y, r.w, r.h, fill, 240, 3);
        drawRect(r, mappal::uiBorder());
    }

    //메뉴 렌더 — 좌표 한 줄(라벨 회색+값 흰색) + "MAP PIN" 헤더(버튼 위, 양옆 구분선) + 색 버튼 줄 + (마커 있으면)제거.
    //  패널은 줌 패널과 동일하게 drawStadium(uiPanel,220,3)만 — 윈도우 테두리 없음. 버튼도 줌 버튼과 동일 스타일.
    void drawPinMenu()
    {
        if (!pinMenuOpen) return;

        constexpr int padX = 14, padTop = 9, coordH = 24, headerH = 18, gapCoordHeader = 4, groupGap = 16, labelGap = 4;

        //패널 — 줌 패널과 동일(테두리 없음).
        drawStadium(pinMenuRect.x, pinMenuRect.y, pinMenuRect.w, pinMenuRect.h, mappal::uiPanel(), 220, 3);

        //좌표 한 줄(라벨 회색 + 값 흰색).
        const wchar_t* labels[3] = { L"X", L"Y", L"Z" };
        const int      vals[3]   = { pinMenuTileX, pinMenuTileY, pinMenuZ };
        setFontSize(17);
        int       hx = pinMenuRect.x + padX;
        const int hy = pinMenuRect.y + padTop;
        for (int i = 0; i < 3; ++i)
        {
            setFont(fontType::mainFontSemiBold);
            drawText(labels[i], hx, hy, SDL_Color{ 0xe1, 0x77, 0x2e, 255 });   // 주황(Status.ixx 라벨 색 #e1772e)
            hx += queryTextWidth(labels[i]) + labelGap;
            setFont(fontType::mainFontBold);
            const std::wstring val = std::to_wstring(vals[i]);
            drawText(val, hx, hy, mappal::uiText());
            hx += queryTextWidth(val) + groupGap;
        }

        //"MAP PIN" 헤더 — 버튼 줄 바로 위, 텍스트 양옆으로 구분선(좌표와의 오인 방지).
        {
            setFont(fontType::mainFontSemiBold);
            setFontSize(12);
            const std::wstring cap = L"MAP PIN";
            const int capW = queryTextWidth(cap);
            const int midY = pinMenuRect.y + padTop + coordH + gapCoordHeader + headerH / 2;
            const int cxc  = pinMenuRect.x + pinMenuRect.w / 2;
            drawTextCenter(cap, cxc, midY, SDL_Color{ 150, 152, 160, 255 });
            const SDL_Color line = mappal::uiBorder();
            drawLine(pinMenuRect.x + padX, midY, cxc - capW / 2 - 8, midY, line, 130);
            drawLine(cxc + capW / 2 + 8, midY, pinMenuRect.x + pinMenuRect.w - padX, midY, line, 130);
        }

        //색 버튼 — 줌 버튼 스타일 + 핀 아이콘 스프라이트(icon16: 적120·녹121·청122·보라123, MAP_PIN_PALETTE 순서와 정합).
        //  그 색 마커가 (위치 무관) 존재하면 배경 deepBlue(사용 중 표시).
        //  클릭 동작은 토글: 그 색이 존재하면(어디든) 취소, 없으면 이 위치에 설치(clickUpGUI).
        for (int i = 0; i < MAP_PIN_COLOR_COUNT; ++i)
        {
            const SDL_Rect& s = pinSwatch[i];
            pinButton(s, mapPinExists(i));
            setZoom(2.0f);   // 16px 아이콘을 2배(32px)로 — 버튼 대비 너무 작아 확대
            drawSpriteCenter(spr::icon16, 120 + i, s.x + s.w / 2, s.y + s.h / 2);
            setZoom(1.0f);
        }

        setFont(fontType::mainFont);
    }

    void drawGUI() override
    {
        if (getStateDraw() == false) return;

        //펼침/닫힘 애니메이션 — 박스만 그리고 종료.
        double r = getFoldRatio();
        if (r < 1.0)
        {
            int w  = (int)(cameraW * r);
            int h  = (int)(cameraH * r);
            int dx = (cameraW - w) / 2;
            int dy = (cameraH - h) / 2;
            drawFillRect(SDL_Rect{ dx, dy, w, h }, mappal::background());
            return;
        }

        view.viewW = cameraW;
        view.viewH = cameraH;
        view.tickAnim();        // 연속 줌 애니메이션(스케일 이징 + 앵커 고정) — "매끈함"
        view.clampCenterY();

        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, mappal::background());

        if (view.worldLOD())
        {
            //광역(위성 텍스처 LOD) — per-청크 타일맵 대신 다운샘플 위성 + 발견 도시 점.
            drawWorldView(view);
            drawNightOverlay(view);      // ⑨ 야간 남색 틴트(위성 지형 위, 도시·마커·핀 아래)
            drawWorldCities(view);       // ⑤-b 도시 점/라벨 — 오버레이 뒤라 조명 영향 안 받음
            drawMapPins(view);           // ⑧ 맵 핀(빛의 기둥) — 말풍선보다 먼저(뒤에)
            drawWorldPlayerMarker(view); // ⑤-c 플레이어 말풍선 — 핀(빛의 기둥) 앞, 조명 영향 안 받음
        }
        else
        {
            static thread_local std::vector<SymDraw> symDraws, mtnDraws;
            symDraws.clear();
            mtnDraws.clear();

            //심볼(도로·건물·산)은 모든 줌에서 유지 — 지형(tileset)만 남고 심볼이 사라지면 어색함.
            //  바다 파도만 줌 게이트(읽기 불가 + 광역에서 draw 폭증하는 장식 디테일).
            const bool foamOn = view.symbolsVisible();

            drawTerrainLayer(view, foamOn, symDraws, mtnDraws);   // ① 베이스(+파도) + ② 산(mtnDraws)·숲(symDraws) 수집

            setZoom((float)view.zoomScale());

            //② 산 레이어 — 도로보다 *먼저* 그린다(도로가 산 위에 터널로 반투명되게). 1셀 타일이라 y정렬 불필요.
            for (const auto& s : mtnDraws)
            {
                SDL_SetTextureColorMod(s.atlas->getTexture(), s.br, s.br, s.br);
                drawSprite(s.atlas, s.idx, s.sx, s.sy);
            }
            SDL_SetTextureColorMod(spr::auto47Mountain->getTexture(), 255, 255, 255);

            drawHighways     (view);             // ③-a 외부 도로망 (산 위면 터널 반투명, 미발견은 어둡게)
            ensureVisibleCityLayouts(view);      // 화면 내 미캐시 도시 경량 layout 백그라운드 요청
            drawCityRoads    (view);             // ③-b 내부 도로·다리 (산 위면 터널 반투명, 미발견은 어둡게)
            drawCityBuildings(view, symDraws);   // ④ 건물 (발견=실제 / 미발견=?건물, 어둡게)
            drawSites        (view, symDraws);   // ④-b 교외 사이트 (같은 규약, activeSites 직접 순회)

            //숲+건물 y정렬 페인터 순서 — 남쪽이 위에 겹침. 그릴 때 br로 색조(전장의 구름).
            std::sort(symDraws.begin(), symDraws.end(),
                [](const SymDraw& a, const SymDraw& b) { return a.sortY < b.sortY; });
            for (const auto& s : symDraws)
            {
                SDL_SetTextureColorMod(s.atlas->getTexture(), s.br, s.br, s.br);
                drawSprite(s.atlas, s.idx, s.sx, s.sy);
            }
            //색조 모드 원복 — 텍스처 상태라 다른 패스/다음 프레임 오염 방지.
            SDL_SetTextureColorMod(spr::mapset1by1->getTexture(), 255, 255, 255);
            SDL_SetTextureColorMod(spr::mapset2by2->getTexture(), 255, 255, 255);
            SDL_SetTextureColorMod(spr::mapset3by3->getTexture(), 255, 255, 255);
            SDL_SetTextureColorMod(spr::auto47Mountain->getTexture(), 255, 255, 255);

            setZoom(1.0f);

            drawNightOverlay(view);   // ⑨ 야간 남색 틴트(지형·심볼 위, 라벨·마커·핀·UI 아래)
            drawCityLabels(view);   // ⑥ 도시 이름 라벨 (발견된 도시만)
            drawMapPins(view);                     // ⑧ 맵 핀(빛의 기둥) — 플레이어 마커보다 먼저(뒤에)
            drawPlayerMarker(view);                // ⑤ 플레이어 마커 — 핀(빛의 기둥) 앞에
            //⑦ 마커/툴팁 — 메뉴 열림 중엔 툴팁은 끄되(클러터), 그리드 마커는 메뉴가 가리키는 청크에 고정 유지.
            if (pinMenuOpen) drawChunkMarker(view, tilePixelIX(pinMenuTileX), tilePixelIY(pinMenuTileY));
            else             drawHoverInfo(view);
        }

        drawZoomPanel(view, computeZoomButtons());   // 줌 패널 — 좌표 한 줄 포함(라벨↔버튼 사이)
        drawTabButton();
        drawPinMenu();   // ⑧ 핀 컨텍스트 메뉴 — 항상 최상단(크롬 위)
    }

    // ────────── 입력 ──────────

    //우클릭 = 핀 메뉴 열기(청크맵). 마우스 우버튼 down은 clickDownGUI가 일찍 return하므로 팬 시작 안 함.
    void clickRightGUI() override
    {
        if (getStateInput() == false) return;
        openPinMenu((int)getMouseX(), (int)getMouseY());
    }

    void clickDownGUI() override
    {
        if (option::inputMethod == input::mouse && event.button.button != SDL_BUTTON_LEFT) return;

        if (pinMenuOpen) return;   // 메뉴 열림 중엔 팬 시작 금지(선택/닫기는 clickUpGUI에서)

        if (checkCursor(&tab)) return;
        ZoomButtons zb = computeZoomButtons();
        if (checkCursor(&zb.zoomIn) || checkCursor(&zb.zoomOut) || checkCursor(&zb.home)) return;

        view.animating = false;   // 드래그 시작 → 진행 중인 줌 애니 취소(앵커가 센터 덮어쓰지 않게)
        dragging = true;
        dragMoved = false;
        dragAnchorPX = view.centerPX;
        dragAnchorPY = view.centerPY;
    }

    void clickMotionGUI(int dx, int dy) override
    {
        if (!dragging) return;
        if (dx * dx + dy * dy > 16) dragMoved = true;
        view.centerPX = dragAnchorPX + dx / view.curScale;
        view.centerPY = dragAnchorPY + dy / view.curScale;   // Y는 clampCenterY가 월드 안으로 보정
    }

    void clickUpGUI() override
    {
        if (getStateInput() == false) return;

        //핀 메뉴가 열려 있으면 메뉴 상호작용이 우선 — 색 버튼=토글(그 색이 어디든 있으면 취소, 없으면 이 위치에 설치),
        //  그 외=닫기(클릭 소비).
        if (pinMenuOpen)
        {
            for (int i = 0; i < MAP_PIN_COLOR_COUNT; ++i)
                if (checkCursor(&pinSwatch[i]))
                {
                    if (mapPinExists(i)) clearMapPin(i);                                     // 어디든 있으면 취소
                    else                 setMapPin(i, pinMenuTileX, pinMenuTileY, pinMenuZ);  // 없으면 이 위치에 설치
                    pinMenuOpen = false;
                    return;
                }
            pinMenuOpen = false;
            return;
        }

        bool wasDrag = dragMoved;
        dragging = false;
        dragMoved = false;
        if (wasDrag) return;

        if (checkCursor(&tab)) { close(aniFlag::winUnfoldClose); return; }

        ZoomButtons zb = computeZoomButtons();
        //버튼 줌 — 화면 중앙 앵커. 단계 사이는 애니로 매끈히 보간(zoomAt→tickAnim).
        if (checkCursor(&zb.zoomIn))  { view.zoomAt(view.viewW / 2, view.viewH / 2, +1); return; }
        if (checkCursor(&zb.zoomOut)) { view.zoomAt(view.viewW / 2, view.viewH / 2, -1); return; }
        if (checkCursor(&zb.home))
        {
            view.animating = false;
            view.centerPX = tileToPixelX(PlayerX());
            view.centerPY = tileToPixelY(PlayerY());
            return;
        }
    }

    void mouseWheel() override
    {
        if (getStateInput() == false) return;
        pinMenuOpen = false;   // 줌하면 타겟 셀이 어긋나므로 메뉴 닫음
        //휠 = 마우스 지점 앵커로 한 단계 줌. 가장 광역(level 0)=전세계 한 화면, 가장 근접=48px/청크.
        const int delta = (event.wheel.y > 0) ? +1 : (event.wheel.y < 0 ? -1 : 0);
        if (delta != 0) view.zoomAt((int)getMouseX(), (int)getMouseY(), delta);
    }

    void keyDownGUI() override
    {
        if (getStateInput() == false) return;
        if (pinMenuOpen && event.key.key == SDLK_ESCAPE) { pinMenuOpen = false; return; }   // ESC=메뉴 먼저 닫기
        if (event.key.key == SDLK_F4) { debugRevealSymbols = !debugRevealSymbols; return; } // 디버그 — 미발견 심볼 전체 식별 토글
        if (event.key.key == SDLK_M || event.key.key == SDLK_ESCAPE)
            close(aniFlag::winUnfoldClose);
    }

    void step() override { tabType = tabFlag::back; }
};
