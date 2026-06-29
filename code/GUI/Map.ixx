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
import World;
import TileData;
import worldGrid;
import worldGen;
import worldWrap;
import city;
import CityPlan;
import worldSession;
import mapDiscovery;

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
//     ④ 건물 심볼   — CityPlan.symbols → mapset1by1(1x1) / mapset2by2(2x1·1x2·2x2). 발견=실제 종류,
//                     미발견=footprint별 "?건물" placeholder(resolveUnknownSymbol) + 어둡게(fog).
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
static ResolvedSym resolveSymbol(MapSymbol s, int w, int h, std::uint64_t hash)
{
    auto one  = [&](int idx) { return ResolvedSym{ spr::mapset1by1, idx, -1, -1, 3 }; };
    auto two2 = [&](int idx) { return ResolvedSym{ spr::mapset2by2, idx, -1, -1, 4 }; };
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
    case MapSymbol::warehouse:        return one(4);
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
    default:                          return ResolvedSym{};   // none / mountain(별도 처리)
    }
}

//미발견 도시의 "?건물" 심볼 — 실제 종류 대신 footprint(w×h)별 미확인 placeholder.
//  오프셋/셀 규약은 resolveSymbol과 동일(1x1=mapset1by1 3청크 off(-1,-1), 나머지=mapset2by2 4청크
//  off는 wide -1,-2 / tall -2,-1 / 2x2 -1,-1). 그 외 footprint(3x3 등)는 프로토타입 미지원 → 스킵.
static ResolvedSym resolveUnknownSymbol(int w, int h)
{
    if (w == 1 && h == 1) return ResolvedSym{ spr::mapset1by1, 63, -1, -1, 3 };
    if (w == 2 && h == 2) return ResolvedSym{ spr::mapset2by2, 20, -1, -1, 4 };
    if (w == 2 && h == 1) return ResolvedSym{ spr::mapset2by2, 19, -1, -2, 4 };   // wide
    if (w == 1 && h == 2) return ResolvedSym{ spr::mapset2by2, 18, -2, -1, 4 };   // tall
    return ResolvedSym{};
}

static std::uint64_t symHash(int px, int py)
{
    std::uint64_t h = static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) * 0x9E3779B97F4A7C15ULL;
    h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(py)) * 0xBF58476D1CE4E5B9ULL;
    h ^= h >> 31;
    return h;
}

//[프로토타입] 절차생성 도시 임시 이름 — center 타일 해시로 결정론적 의사 영어 지명.
//  매 프레임 같은 결과여야 깜빡임이 없으므로 좌표 해시 기반(난수 X). 접두·접미 각 16개라
//  & 15로 인덱싱. TODO: 향후 문화권(직사각형 구역) 기반 생성기로 교체할 자리표시자.
static std::wstring placeholderCityName(int tileX, int tileY)
{
    static const wchar_t* pre[] = {
        L"Ash", L"Black", L"North", L"Red", L"Stone", L"West", L"Green", L"Frost",
        L"Iron", L"Gray", L"Pine", L"Bay", L"Fort", L"Salt", L"Wolf", L"Crow" };
    static const wchar_t* suf[] = {
        L"ton", L"ford", L"field", L"haven", L"burg", L"vale", L"port", L"wood",
        L"ridge", L"mont", L"dale", L"reach", L"hollow", L"crest", L"moor", L"stead" };
    const std::uint64_t h = symHash(tileX, tileY);
    return std::wstring(pre[(h >> 3) & 15]) + suf[(h >> 11) & 15];
}

//사전배치 도시 표시명 — PRESET_CITIES에서 codename 룩업(ASCII displayName → wide). 미발견 시 빈 문자열.
static std::wstring presetDisplayName(city::CityName cn)
{
    for (const auto& pc : city::PRESET_CITIES)
        if (pc.codename == cn)
            return std::wstring(pc.displayName.begin(), pc.displayName.end());
    return {};
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

    const int cp = v.chunkPx();
    const double halfW = v.viewW * 0.5 / cp;
    const double halfH = v.viewH * 0.5 / cp;

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
            const bool seen = mapDiscovery::discovered(apx, apy);

            //발견=실제 종류, 미발견=?건물 placeholder. 둘 다 못 풀면(none/미지원 footprint) 스킵.
            const ResolvedSym rs = seen
                ? resolveSymbol(sym.symbol, sym.w, sym.h, symHash(apx, apy))
                : resolveUnknownSymbol(sym.w, sym.h);
            if (!rs.atlas) continue;

            const int sx = (int)std::lround(v.sX((double)(apx + rs.offX)));
            const int sy = (int)std::lround(v.sY((double)(apy + rs.offY)));
            const int span = rs.cellChunks * cp;
            if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

            symOut.push_back(SymDraw{ (float)apy, rs.atlas, rs.idx, sx, sy, fogBright(apx, apy) });
        }
    }
}

//⑥ 도시 이름 라벨 — 도시 중심에 표기. CityPlan 캐시 없이 activeCities의 center를 직접 사용
//   (peek로 캐시된 도시만 보는 건물/도로와 달리 전 도시 라벨 가능). 사전배치=PRESET_CITIES
//   displayName, 절차생성(codename==none)=placeholderCityName 자리표시자.
//   픽셀폰트 size 12를 zoomScale로 NEAREST 스케일(격자 보존). 배경은 반투명 검정 박스.
//   발견 여부 무관하게 화면 내 전 도시 표기 — 청크맵에선 도시 layout(?건물+도로망)이 이미
//   보이므로 이름도 같이 노출. 클러터는 화면 컬링으로만 제한(저배율 Satellite 모드는
//   drawWorldView가 사전마킹 도시만 표기). 이 함수는 비-worldLOD(청크맵) 분기에서만 호출됨.
static void drawCityLabels(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    //줌 배율로 라벨 크기 — 너무 작아 안 보이거나 너무 커 화면을 덮지 않게 클램프.
    const float scale = std::clamp((float)v.zoomScale(), 0.85f, 2.5f);

    setFont(fontType::pixel);
    setFontSize(12);

    for (const auto& node : *cities)
    {
        if (node.center.z != v.z) continue;

        const double sxd = v.sX(tileToPixelX(node.center.x));
        const double syd = v.sY(tileToPixelY(node.center.y));
        if (sxd < -256 || sxd > v.viewW + 256 || syd < -64 || syd > v.viewH + 64) continue;

        const std::wstring name = (node.codename != city::CityName::none)
            ? presetDisplayName(node.codename)
            : placeholderCityName(node.center.x, node.center.y);
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
namespace
{
    SDL_Texture*  g_satTex  = nullptr;
    int           g_satTW   = 0, g_satTH = 0;       // 텍스처 크기
    double        g_satWX0  = 0.0, g_satWY0 = 0.0;  // 덮는 월드 rect 좌상단(청크좌표, X unwrapped)
    double        g_satCPT  = -1.0;                 // chunks per texel(해상도). <0 = 무효
    std::uint64_t g_satSeed = ~0ull;                // worldSeed 바뀌면 재빌드

    SDL_Texture*  g_worldTex  = nullptr;            // 전세계 저해상도 백드롭(검정 깜빡임 방지)
    std::uint64_t g_worldSeed = ~0ull;

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
        if (g_worldTex && g_worldSeed == worldSeed) return g_worldTex;
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
                const SDL_Color c = worldTerrainColor(worldGrid::worldPixel(px, py));
                buf[static_cast<std::size_t>(ty) * TW + tx] =
                    (static_cast<std::uint32_t>(c.r) << 24) | (static_cast<std::uint32_t>(c.g) << 16)
                  | (static_cast<std::uint32_t>(c.b) <<  8) |  0xFFu;
            }
        }
        SDL_UpdateTexture(tex, nullptr, buf.data(), TW * static_cast<int>(sizeof(std::uint32_t)));
        g_worldTex  = tex;
        g_worldSeed = worldSeed;
        return tex;
    }

    //현재 뷰에 맞는 위성 윈도우 텍스처 확보(필요 시에만 재빌드). 출력: 덮는 rect 좌상단/해상도.
    //  재빌드 조건 = 시드 변경 / 해상도 어긋남 / 보이는 영역이 캐시 rect 밖. 줌 애니 중엔
    //  무조건 재사용(매프레임 빌드 회피 — rect로 블릿하므로 기하는 정확, 해상도만 잠깐 어긋).
    SDL_Texture* satelliteTexture(const MapView& v, double& oWX0, double& oWY0, double& oCPT)
    {
        if (!worldGrid::worldPixelMmapActive()) return nullptr;

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

        //재사용 판정 — 보이는 영역이 캐시 rect 안 + 해상도 ±25% 이내 + 시드 동일.
        const double visW = v.viewW / v.curScale, visH = v.viewH / v.curScale;
        const bool resOK   = g_satCPT > 0 && cpt >= g_satCPT * 0.8 && cpt <= g_satCPT * 1.25;
        const bool coverOK = g_satTex && g_satSeed == worldSeed && resOK
            && v.centerPX - visW * 0.5 >= g_satWX0 && v.centerPX + visW * 0.5 <= g_satWX0 + g_satTW * g_satCPT
            && v.centerPY - visH * 0.5 >= g_satWY0 && v.centerPY + visH * 0.5 <= g_satWY0 + g_satTH * g_satCPT;

        if (g_satTex && g_satSeed == worldSeed && (coverOK || v.animating))
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
                const worldGrid::Terrain t = yOOB ? worldGrid::Terrain::Sea
                    : worldGrid::worldPixel(worldWrap::wrapPixelX((int)std::lround(wx0 + tx * cpt)), wy);
                const SDL_Color c = worldTerrainColor(t);
                buf[static_cast<std::size_t>(ty) * TW + tx] =
                    (static_cast<std::uint32_t>(c.r) << 24) | (static_cast<std::uint32_t>(c.g) << 16)
                  | (static_cast<std::uint32_t>(c.b) <<  8) |  0xFFu;
            }
        }
        SDL_UpdateTexture(g_satTex, nullptr, buf.data(), TW * static_cast<int>(sizeof(std::uint32_t)));
        g_satWX0 = wx0; g_satWY0 = wy0; g_satCPT = cpt; g_satSeed = worldSeed;
        oWX0 = wx0; oWY0 = wy0; oCPT = cpt;
        return g_satTex;
    }
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

    //② 고해상도 윈도우 — 줌 안정(애니 X) + 광역(coverW≥월드폭/2)이 아닐 때만 위에 덧그려 선명.
    //  줌 애니 중엔 스킵(백드롭만) → 스테일 윈도우가 줌하며 어긋나 보이는 것·매프레임 재빌드 방지.
    if (!v.animating && (v.viewW / v.curScale) * 1.5 < static_cast<double>(WORLD_PIXEL_W) * 0.5)
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

    //점: 발견한 도시만(전장의 구름 핵심 — 지형색은 도시 위치를 숨기므로 점이 유일한 노출).
    //  이름: 사전마킹(preset, codename!=none) 도시만(발견 무관, 밝게) — 전세계 조망 시 방향
    //  잡는 앵커. 절차생성 자리표시자 이름은 위성지도에서 어색해 제외(이름은 청크맵에서만).
    //  → 미발견 preset은 이름만 떠 위치 힌트, 발견 도시는 점, 발견 preset은 점+이름.
    const auto* cities = worldGen::activeCities;
    if (cities)
    {
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

    //플레이어 마커(펄스).
    const int psx = (int)std::lround(v.sX(tileToPixelX(PlayerX())));
    const int psy = (int)std::lround(v.sY(tileToPixelY(PlayerY())));
    if ((SDL_GetTicks() % 900) < 600)
    {
        drawFillCircle(psx, psy, 5, SDL_Color{ 255, 255, 255, 255 }, 255);
        drawFillCircle(psx, psy, 3, mappal::playerMarker(), 255);
    }
    else
        drawFillCircle(psx, psy, 4, mappal::playerMarker(), 220);
}

//⑤ 플레이어 마커 — 화면 안이면 펄스, 밖이면 가장자리 클램프.
static void drawPlayerMarker(const MapView& v)
{
    const double ppX = tileToPixelX(PlayerX());
    const double ppY = tileToPixelY(PlayerY());
    const double sxd = v.sX(ppX);
    const double syd = v.sY(ppY);

    if (sxd < 0 || sxd > v.viewW || syd < 0 || syd > v.viewH)
    {
        const int ex = (int)std::clamp(sxd, 16.0, (double)v.viewW - 16.0);
        const int ey = (int)std::clamp(syd, 16.0, (double)v.viewH - 16.0);
        drawFillRect(SDL_Rect{ ex - 7, ey - 7, 14, 14 }, mappal::playerMarker());
        drawRect(SDL_Rect{ ex - 7, ey - 7, 14, 14 }, SDL_Color{ 255, 255, 255, 255 });
        return;
    }

    const int sx = (int)std::round(sxd);
    const int sy = (int)std::round(syd);
    if ((SDL_GetTicks() % 900) < 600)
    {
        drawFillCircle(sx, sy, 7, SDL_Color{ 255, 255, 255, 255 }, 255);
        drawFillCircle(sx, sy, 5, mappal::playerMarker(), 255);
    }
    else
    {
        drawFillCircle(sx, sy, 5, mappal::playerMarker(), 200);
    }
}


// ════════════════════════════════════════════════════════════════════════
// §5  UI 크롬
// ════════════════════════════════════════════════════════════════════════

//플레이어 좌표 — 우측하단, 축별 3줄. 무채색만 사용: 라벨(X/Y/Z)은 흐린 회색+준굵게,
//  값은 밝은 흰색+굵게 → 색상(hue) 없이 밝기·굵기로 라벨↔값 구분(라벨 3개는 동일색).
//  값은 우측정렬, 박스 없이 외곽선으로 가독성 확보. 안내 문구는 없음.
static void drawCoordPanel(const MapView& v)
{
    constexpr int margin = 26;
    constexpr int rowH   = 26;
    const int RX = v.viewW - margin;   // 값 우측 정렬 기준선

    const wchar_t* labels[3] = { L"X", L"Y", L"Z" };
    const int      vals[3]   = { PlayerX(), PlayerY(), PlayerZ() };

    const SDL_Color labelCol = { 240, 240, 240, 255 };   // 흐린 회색(X/Y/Z 동일)
    const SDL_Color valueCol = { 240, 240, 234, 255 };   // 밝은 흰색

    setFontSize(19);

    //값 최대폭(굵은 폰트 기준)으로 라벨 컬럼 결정 — 값=우측정렬(RX), 라벨=그 왼쪽 고정 컬럼.
    setFont(fontType::mainFontBold);
    int maxValW = 0;
    for (int i = 0; i < 3; ++i) maxValW = std::max(maxValW, queryTextWidth(std::to_wstring(vals[i])));
    const int labelColRight = RX - maxValW - 18;
    const int topY = v.viewH - margin - rowH * 3;

    for (int i = 0; i < 3; ++i)
    {
        const int y = topY + i * rowH;

        setFont(fontType::mainFontBold);
        const std::wstring val = std::to_wstring(vals[i]);
        drawTextOutline(val, RX - queryTextWidth(val), y, valueCol);                 // 값(흰·굵게, 우측정렬)

        setFont(fontType::mainFontSemiBold);
        const std::wstring lab = labels[i];
        drawTextOutline(lab, labelColRight - queryTextWidth(lab), y, labelCol);      // 라벨(회색·준굵게)
    }

    setFont(fontType::mainFont);
}

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
    int panelW = (btnSize + gap) * 3 + 18;
    int panelH = btnSize + 30 + 14;
    int panelX = margin - 9;
    int panelY = cameraH - margin - btnSize - 36;
    drawStadium(panelX, panelY, panelW, panelH, mappal::uiPanel(), 220, 3);

    setFontSize(16);
    std::wstring label = v.worldLOD()
        ? std::wstring(L"Satellite")
        : L"Zoom  " + std::to_wstring((int)std::lround(v.curScale)) + L" px/chunk";
    drawText(label, panelX + 14, panelY + 8, mappal::uiText());

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
            SDL_SetTextureColorMod(spr::auto47Mountain->getTexture(), 255, 255, 255);

            setZoom(1.0f);

            drawCityLabels(view);   // ⑥ 도시 이름 라벨 (발견된 도시만)
            drawPlayerMarker(view);
        }

        drawCoordPanel(view);
        drawZoomPanel(view, computeZoomButtons());
        drawTabButton();
    }

    // ────────── 입력 ──────────
    void clickDownGUI() override
    {
        if (option::inputMethod == input::mouse && event.button.button != SDL_BUTTON_LEFT) return;

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
        //휠 = 마우스 지점 앵커로 한 단계 줌. 가장 광역(level 0)=전세계 한 화면, 가장 근접=48px/청크.
        const int delta = (event.wheel.y > 0) ? +1 : (event.wheel.y < 0 ? -1 : 0);
        if (delta != 0) view.zoomAt((int)getMouseX(), (int)getMouseY(), delta);
    }

    void keyDownGUI() override
    {
        if (getStateInput() == false) return;
        if (event.key.key == SDLK_M || event.key.key == SDLK_ESCAPE)
            close(aniFlag::winUnfoldClose);
    }

    void step() override { tabType = tabFlag::back; }
};
