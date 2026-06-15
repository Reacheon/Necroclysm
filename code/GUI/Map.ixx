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

// ════════════════════════════════════════════════════════════════════════
// Map — 풀스크린 월드맵 (고전 JRPG 월드맵 스타일).
//
//   구글지도식 무제한 줌을 폐기하고 "1청크 = 1심볼" 타일 기반으로 재작성.
//   레이어:
//     ① 베이스 지형 — tileset.png. 청크 1개당 타일 1개(잔디/해수/담수). 배치 렌더.
//     ② 산 심볼     — worldGrid::Terrain::Mountain. 4개 사각형 뭉치면 2x2로 머지(JRPG).
//     ③ 도로 심볼   — CityPlan.roadCells(openBits) → autotile(직선/코너/T/십자) mapset1by1.
//     ④ 건물 심볼   — CityPlan.symbols → mapset1by1(1x1) / mapset2by2(2x1·1x2·2x2).
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
    //이산 줌 — 청크 1개가 차지하는 화면 픽셀. 심볼/타일은 chunkPx/16 배율로 그려짐.
    //  (베이스 타일 16px, mapset1by1 48px=3청크, mapset2by2 64px=4청크 → 모두 같은 배율.)
    //  무제한 광역 조망(대륙 단위)은 의도적으로 제외 — 지역 단위 항법 지도.
    inline constexpr int ZOOM_PX[] = { 6, 8, 12, 16, 24, 32, 48 };
    inline constexpr int ZOOM_COUNT   = (int)(sizeof(ZOOM_PX) / sizeof(ZOOM_PX[0]));
    inline constexpr int DEFAULT_ZOOM = 3;   // 16px/청크 (네이티브 픽셀아트)

    //이 값 미만 줌에서는 바다 파도를 안 그림(저배율 클러터/draw 폭증 방지).
    //  도로·건물·산 심볼은 모든 줌에서 유지 — 지형만 남으면 어색해서.
    inline constexpr int SYMBOL_MIN_PX = 8;
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
    int    zoomLevel = mapcfg::DEFAULT_ZOOM;
    int    z         = 0;
    int    viewW     = 0;
    int    viewH     = 0;

    int    chunkPx()   const { return mapcfg::ZOOM_PX[zoomLevel]; }
    double zoomScale() const { return chunkPx() / 16.0; }   // tileset 16px 기준 배율
    bool   symbolsVisible() const { return chunkPx() >= mapcfg::SYMBOL_MIN_PX; }

    //카메라 중앙 기준 X 최단 거리(픽셀) — ±WORLD_CHUNK_W/2로 wrap.
    double relX(double px) const
    {
        double rel = px - centerPX;
        const double W = static_cast<double>(WORLD_CHUNK_W);
        rel -= std::round(rel / W) * W;
        return rel;
    }

    double sX(double px) const { return relX(px) * chunkPx() + viewW * 0.5; }
    double sY(double py) const { return (py - centerPY) * chunkPx() + viewH * 0.5; }

    double pixelXFromScreen(double sx) const { return centerPX + (sx - viewW * 0.5) / chunkPx(); }
    double pixelYFromScreen(double sy) const { return centerPY + (sy - viewH * 0.5) / chunkPx(); }

    //이산 줌 — delta 레벨 이동, anchor 화면 위치 고정.
    void zoomAround(int anchorScreenX, int anchorScreenY, int delta)
    {
        const double apX = pixelXFromScreen(anchorScreenX);
        const double apY = pixelYFromScreen(anchorScreenY);
        zoomLevel = std::clamp(zoomLevel + delta, 0, mapcfg::ZOOM_COUNT - 1);
        centerPX = apX - (anchorScreenX - viewW * 0.5) / chunkPx();
        centerPY = apY - (anchorScreenY - viewH * 0.5) / chunkPx();
    }

    //카메라 Y를 월드(극지) 안으로 클램프. X는 wrap이라 클램프 안 함.
    void clampCenterY()
    {
        const double halfH = viewH * 0.5 / chunkPx();
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
    case MapSymbol::policeStation:    return rect(2, 3);
    case MapSymbol::fireStation:      return rect(4, 5);
    case MapSymbol::park:             return two2(1);
    case MapSymbol::hypermarket:      return two2(6);
    case MapSymbol::school:           return two2(7);
    default:                          return ResolvedSym{};   // none / mountain(별도 처리)
    }
}

static std::uint64_t symHash(int px, int py)
{
    std::uint64_t h = static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) * 0x9E3779B97F4A7C15ULL;
    h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(py)) * 0xBF58476D1CE4E5B9ULL;
    h ^= h >> 31;
    return h;
}


// ════════════════════════════════════════════════════════════════════════
// §4  렌더링
// ════════════════════════════════════════════════════════════════════════

//y로 정렬해 그리는 심볼(산·건물) — 남쪽이 위에 겹치는 JRPG 페인터 순서.
struct SymDraw { float sortY; Sprite* atlas; int idx; int sx; int sy; };

namespace
{
    struct BaseQuad { float l, t, r, b; int sprIdx; Uint8 a; };

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
            const SDL_FColor col = { 1.0f, 1.0f, 1.0f, q[i].a / 255.0f };

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

//① 베이스 지형(오토타일) + 파도 + ② 산 심볼 (단일 스윕, 로컬 terrain 버퍼).
//   본체 renderTile의 floor 오토타일(tileConnectGroup+connectGroupExtraIndex)과
//   파도(스프라이트 1504~1526)을 청크 스케일로 이식 — 1청크가 1타일 역할.
//   산 심볼은 항상 수집(모든 줌 유지). 파도만 drawFoam(=symbolsVisible)일 때 (저배율 클러터/성능 회피).
static void drawTerrainLayer(const MapView& v, bool drawFoam, std::vector<SymDraw>& symOut)
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
    auto isMtn = [&](int x, int y) { return terrAt(x, y) == T::Mountain; };
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
    auto pushQuad = [&](int ix, int iy, int sprIdx, Uint8 a) {
        if ((int)batch.size() >= MAX_BATCH) flush();
        batch.push_back(BaseQuad{
            (float)v.sX((double)ix),       (float)v.sY((double)iy),
            (float)v.sX((double)(ix + 1)), (float)v.sY((double)(iy + 1)), sprIdx, a });
    };

    //파도 — 본체 renderTile addWave 매핑(1504~1526). isW: 그 방향 이웃이 물인가.
    //  자기 자신이 그 물타입이 아닐 때만(경계 셀) 그림. baseOff: 해수=애니프레임, 담수=496.
    //  맵에서는 파도가 움직이면 정신사나워서 2번째 프레임(인덱스1, 오프셋32)으로 고정.
    const int seaAnim = 32;
    auto emitFoam = [&](int ix, int iy, auto&& isW, int baseOff, Uint8 alpha) {
        const bool tC = isW(ix, iy - 1), bC = isW(ix, iy + 1), lC = isW(ix - 1, iy), rC = isW(ix + 1, iy);
        const bool trC = isW(ix + 1, iy - 1), tlC = isW(ix - 1, iy - 1),
                   blC = isW(ix - 1, iy + 1), brC = isW(ix + 1, iy + 1);
        if (!(tC || bC || lC || rC || trC || tlC || blC || brC)) return;
        auto push = [&](int idx) { pushQuad(ix, iy, idx + baseOff, alpha); };
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

            //① 베이스 타일 (오토타일)
            pushQuad(ix, iy, floorSprIdx(ix, iy), 255);

            //파도 — 해수 경계만. 자기 자신이 바다가 아닌 셀에만(본체와 동일).
            if (drawFoam && !isSea(ix, iy))
                emitFoam(ix, iy, isSea, seaAnim, 200);

            //② 산 심볼 (2x2 머지) — 짝수그리드 4-뭉치는 mapset2by2 #9, 아니면 mapset1by1 #31. 항상.
            if (t == T::Mountain)
            {
                const int ddx = ix & 1, ddy = iy & 1;   // WORLD_PIXEL_W 짝수 → ix 패리티 = wrap 패리티
                const bool full2x2 =
                    isMtn(ix - ddx,     iy - ddy)     && isMtn(ix - ddx + 1, iy - ddy) &&
                    isMtn(ix - ddx,     iy - ddy + 1) && isMtn(ix - ddx + 1, iy - ddy + 1);
                if (full2x2)
                {
                    if (ddx == 0 && ddy == 0)   // 블록 앵커만 2x2 그림 (나머지 3개는 이 스프라이트가 덮음)
                        symOut.push_back(SymDraw{ (float)iy, spr::mapset2by2, 9,
                            (int)std::lround(v.sX((double)(ix - 1))),
                            (int)std::lround(v.sY((double)(iy - 1))) });
                }
                else
                {
                    symOut.push_back(SymDraw{ (float)iy, spr::mapset1by1, 31,
                        (int)std::lround(v.sX((double)(ix - 1))),
                        (int)std::lround(v.sY((double)(iy - 1))) });
                }
            }
        }
    flush();
}

//③ 도로 심볼 (캐시된 도시만) — 평면 레이어라 즉시 그림(정렬 X). setZoom은 호출자가 설정.
static void drawCityRoads(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    const int cp = v.chunkPx();
    const int span = 3 * cp;   // mapset1by1 = 3청크

    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const CityPlan* plan = CityPlanCache::ins().peek(static_cast<city::CityId>(i));
        if (!plan) continue;

        for (const auto& rc : plan->roadCells)
        {
            if (rc.pos.z != v.z) continue;
            const int idx = roadSpriteIndex(rc.openBits);
            if (idx < 0) continue;

            const int px = tilePixelIX(rc.pos.x);
            const int py = tilePixelIY(rc.pos.y);
            const int sx = (int)std::lround(v.sX((double)(px - 1)));
            const int sy = (int)std::lround(v.sY((double)(py - 1)));
            if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

            drawSprite(spr::mapset1by1, idx, sx, sy);
        }
    }
}

//④ 건물 심볼 (캐시된 도시만) — symOut에 누적(산과 함께 y정렬 후 그림).
static void drawCityBuildings(const MapView& v, std::vector<SymDraw>& symOut)
{
    const auto* cities = worldGen::activeCities;
    if (!cities) return;

    const int cp = v.chunkPx();

    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const CityPlan* plan = CityPlanCache::ins().peek(static_cast<city::CityId>(i));
        if (!plan) continue;

        for (const auto& sym : plan->symbols)
        {
            if (sym.pos.z != v.z) continue;
            const int apx = tilePixelIX(sym.pos.x);
            const int apy = tilePixelIY(sym.pos.y);

            const ResolvedSym rs = resolveSymbol(sym.symbol, sym.w, sym.h, symHash(apx, apy));
            if (!rs.atlas) continue;

            const int sx = (int)std::lround(v.sX((double)(apx + rs.offX)));
            const int sy = (int)std::lround(v.sY((double)(apy + rs.offY)));
            const int span = rs.cellChunks * cp;
            if (sx + span < 0 || sx > v.viewW || sy + span < 0 || sy > v.viewH) continue;

            symOut.push_back(SymDraw{ (float)apy, rs.atlas, rs.idx, sx, sy });
        }
    }
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

static void drawCoordPanel()
{
    SDL_Rect panel{ 22, 22, 240, 134 };
    drawStadium(panel.x, panel.y, panel.w, panel.h, mappal::uiPanel(), 220, 3);

    setFontSize(16);
    const std::wstring header = L"Player Coordinates";
    setFont(fontType::mainFontBold);
    drawText(header, panel.x + 14, panel.y + 8, mappal::uiText());
    setFont(fontType::mainFont);

    int headerW   = queryTextWidth(header);
    int sepX      = panel.x + 14;
    int sepY      = panel.y + 34;
    int sepEndX   = panel.x + panel.w - 14;
    int solidEndX = std::min(sepX + headerW + 8, sepEndX);
    drawLine(sepX, sepY, solidEndX, sepY, mappal::uiBorder());
    int fadeLen = sepEndX - solidEndX;
    for (int i = 0; i < fadeLen; i++)
    {
        Uint8 a = (Uint8)(255 - (255 * i / fadeLen));
        drawPoint(solidEndX + 1 + i, sepY, mappal::uiBorder(), a);
    }

    int rowY    = panel.y + 43;
    int labelX  = panel.x + 18;
    int valueRX = panel.x + panel.w - 18;
    auto kv = [&](const wchar_t* label, int val)
        {
            setFontSize(15);
            setFont(fontType::mainFontSemiBold);
            drawText(label, labelX, rowY + 3, mappal::uiText());
            setFont(fontType::mainFont);
            setFontSize(20);
            std::wstring s = std::to_wstring(val);
            int vw = queryTextWidth(s);
            drawText(s, valueRX - vw, rowY, mappal::uiText());
            rowY += 28;
        };
    kv(L"X", PlayerX());
    kv(L"Y", PlayerY());
    kv(L"Z", PlayerZ());
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
    std::wstring label = L"Zoom  " + std::to_wstring(v.chunkPx()) + L" px/chunk";
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
        view.zoomLevel = persistedZoom;
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
        view.clampCenterY();

        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, mappal::background());

        static thread_local std::vector<SymDraw> symDraws;
        symDraws.clear();

        //심볼(도로·건물·산)은 모든 줌에서 유지 — 지형(tileset)만 남고 심볼이 사라지면 어색함.
        //  바다 파도만 줌 게이트(읽기 불가 + 광역에서 draw 폭증하는 장식 디테일).
        const bool foamOn = view.symbolsVisible();

        drawTerrainLayer(view, foamOn, symDraws);   // ① 베이스(+파도) + ② 산 수집(항상)

        setZoom((float)view.zoomScale());
        drawCityRoads    (view);             // ③ 도로 (평면, 즉시)
        drawCityBuildings(view, symDraws);   // ④ 건물 수집

        //산+건물 y정렬 페인터 순서 — 남쪽이 위에 겹침.
        std::sort(symDraws.begin(), symDraws.end(),
            [](const SymDraw& a, const SymDraw& b) { return a.sortY < b.sortY; });
        for (const auto& s : symDraws)
            drawSprite(s.atlas, s.idx, s.sx, s.sy);

        setZoom(1.0f);

        drawPlayerMarker(view);

        drawCoordPanel();
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

        dragging = true;
        dragMoved = false;
        dragAnchorPX = view.centerPX;
        dragAnchorPY = view.centerPY;
    }

    void clickMotionGUI(int dx, int dy) override
    {
        if (!dragging) return;
        if (dx * dx + dy * dy > 16) dragMoved = true;
        view.centerPX = dragAnchorPX + dx / (double)view.chunkPx();
        view.centerPY = dragAnchorPY + dy / (double)view.chunkPx();
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
        if (checkCursor(&zb.zoomIn))  { view.zoomAround(view.viewW / 2, view.viewH / 2, +1); return; }
        if (checkCursor(&zb.zoomOut)) { view.zoomAround(view.viewW / 2, view.viewH / 2, -1); return; }
        if (checkCursor(&zb.home))
        {
            view.centerPX = tileToPixelX(PlayerX());
            view.centerPY = tileToPixelY(PlayerY());
            return;
        }
    }

    void mouseWheel() override
    {
        if (getStateInput() == false) return;
        if (event.wheel.y > 0)
            view.zoomAround((int)getMouseX(), (int)getMouseY(), +1);
        else if (event.wheel.y < 0)
            view.zoomAround((int)getMouseX(), (int)getMouseY(), -1);
    }

    void keyDownGUI() override
    {
        if (getStateInput() == false) return;
        if (event.key.key == SDLK_M || event.key.key == SDLK_ESCAPE)
            close(aniFlag::winUnfoldClose);
    }

    void step() override { tabType = tabFlag::back; }
};
