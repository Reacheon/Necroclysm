module;
#include <SDL3/SDL.h>

export module Map;

import std;
import util;
import constVar;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import Player;
import World;
import Patch;
import TileData;

// ════════════════════════════════════════════════════════════════════════
// Map — 풀스크린 인터랙티브 월드맵 (구글지도 스타일)
//   §1 설정/팔레트  §2 카메라  §3 텍스처 캐시  §4 데이터 로딩
//   §5 렌더링       §6 UI 크롬  §7 Map 클래스
// ════════════════════════════════════════════════════════════════════════


// ════════════════════════════════════════════════════════════════════════
// §1  설정 / 팔레트
// ════════════════════════════════════════════════════════════════════════

namespace mapcfg
{
    // 픽셀 퍼펙트 줌 레벨 — pxPerTile = 스크린 픽셀 / 월드 타일.
    //   조건1: 50*pxPerTile 가 정수 → 패치 텍스처 (1 patch px = 50 tiles) 가
    //          모든 화면 픽셀에 정수 비율로 매핑 → 띠/뭉개짐 없음.
    //   조건2: pxPerTile 가 ≥1 일 때 정수 → 16px 타일 스프라이트가 모든
    //          타일에 동일한 정수 픽셀 크기로 렌더 → 균일.
    inline constexpr double ZOOM_LEVELS[] = {
        0.04, 0.06, 0.08, 0.10, 0.12, 0.16, 0.20, 0.24, 0.32, 0.40,
        0.50, 0.60, 0.80, 1.0,  2.0,  3.0,  4.0,  5.0,  6.0
    };
    inline constexpr int    ZOOM_LEVEL_COUNT   = (int)(sizeof(ZOOM_LEVELS) / sizeof(ZOOM_LEVELS[0]));
    inline constexpr int    DEFAULT_ZOOM_LEVEL = 11;  // 0.60
    inline constexpr double DEFAULT_PX_PER_TILE = ZOOM_LEVELS[DEFAULT_ZOOM_LEVEL];

    // 프레임당 신규 텍스처 빌드 한도 (영속 캐시 — 첫 방문 영역에만 쓰임).
    inline constexpr int FRAME_BUDGET_PATCHES = 2;     // 패치 1개 ≈ 5~8ms

    // 가시 미로드 패치 자동 로드 한도
    inline constexpr int FRAME_BUDGET_PATCH_LOAD = 2;

    // 가시 영역 pxPerTile 이 이 값 이하일 때만 패치 자동 로드
    inline constexpr double AUTOLOAD_MAX_ZOOM = 1.5;
}

namespace mappal
{
    // 바이옴 색 — 구글맵 톤
    inline SDL_Color biomeColor(chunkFlag f)
    {
        switch (f)
        {
        case chunkFlag::seawater:    return {  85, 132, 173, 255 };
        case chunkFlag::freshwater:  return { 137, 180, 200, 255 };
        case chunkFlag::dirt:        return { 219, 211, 187, 255 };
        case chunkFlag::meadow:      return { 192, 215, 168, 255 };
        case chunkFlag::city:        return { 230, 226, 218, 255 };
        case chunkFlag::bridge:      return { 200, 195, 188, 255 };
        case chunkFlag::underground: return { 130, 130, 130, 255 };
        default:                     return {  18,  18,  22, 255 };
        }
    }

    inline SDL_Color background()   { return {  10,  10,  14, 255 }; }
    inline SDL_Color playerMarker() { return { 220,  80,  80, 255 }; }

    // UI 크롬
    inline SDL_Color uiPanel()      { return {  20,  20,  28, 220 }; }
    inline SDL_Color uiBorder()     { return { 110, 110, 115, 255 }; }
    inline SDL_Color uiText()       { return { 235, 235, 230, 255 }; }
}


// ════════════════════════════════════════════════════════════════════════
// §2  카메라 (MapView)
// ════════════════════════════════════════════════════════════════════════

// 화면 중앙이 가리키는 월드 타일 (실수) + 줌. 좌표 변환 함수 캡슐화.
struct MapView
{
    double centerTileX = 0.0;
    double centerTileY = 0.0;
    double pxPerTile   = mapcfg::DEFAULT_PX_PER_TILE;
    int    z           = 0;
    int    viewW       = 0;
    int    viewH       = 0;

    double screenXFromTileX(double tx) const { return (tx - centerTileX) * pxPerTile + viewW * 0.5; }
    double screenYFromTileY(double ty) const { return (ty - centerTileY) * pxPerTile + viewH * 0.5; }
    double tileXFromScreenX(double sx) const { return centerTileX + (sx - viewW * 0.5) / pxPerTile; }
    double tileYFromScreenY(double sy) const { return centerTileY + (sy - viewH * 0.5) / pxPerTile; }

    // 가시 타일 박스 (반열림 [min, max))
    void visibleTileBounds(double& minTX, double& minTY, double& maxTX, double& maxTY) const
    {
        minTX = tileXFromScreenX(0);
        minTY = tileYFromScreenY(0);
        maxTX = tileXFromScreenX(viewW);
        maxTY = tileYFromScreenY(viewH);
    }

    // 현재 pxPerTile 에 가장 가까운 ZOOM_LEVELS 인덱스 (로그 거리 — 비례 척도).
    int currentZoomLevel() const
    {
        int best = 0;
        double bestDiff = std::numeric_limits<double>::infinity();
        double cur = std::log(pxPerTile);
        for (int i = 0; i < mapcfg::ZOOM_LEVEL_COUNT; i++)
        {
            double d = std::abs(cur - std::log(mapcfg::ZOOM_LEVELS[i]));
            if (d < bestDiff) { bestDiff = d; best = i; }
        }
        return best;
    }

    // 이산 줌 — delta 만큼 레벨 이동 후 anchor 화면 위치 고정.
    void zoomAround(int anchorScreenX, int anchorScreenY, int delta)
    {
        int level = std::clamp(currentZoomLevel() + delta, 0, mapcfg::ZOOM_LEVEL_COUNT - 1);
        double anchorTX = tileXFromScreenX(anchorScreenX);
        double anchorTY = tileYFromScreenY(anchorScreenY);
        pxPerTile = mapcfg::ZOOM_LEVELS[level];
        centerTileX = anchorTX + (viewW * 0.5 - anchorScreenX) / pxPerTile;
        centerTileY = anchorTY + (viewH * 0.5 - anchorScreenY) / pxPerTile;
    }
};


// ════════════════════════════════════════════════════════════════════════
// §3  텍스처 캐시
//
//   resetFrame(budget) — 매 프레임 시작 호출. budget + pending 카운터 초기화.
//   getOrBuild(key)    — hit 즉시 반환. miss 시 budget 안에서 빌드, 0 이면
//                        nullptr + pending++. 데이터 부재 시 nullptr (pending X).
//   pendingThisFrame() — 이번 프레임 budget 부족으로 미룬 빌드 수.
//   clear()            — 모든 텍스처 파괴 (월드 리셋 등).
// ════════════════════════════════════════════════════════════════════════

// 패치 한 장 = PIXEL_PER_PATCH × PIXEL_PER_PATCH 텍스처 (1 px = 1 patch pixel).
class PatchTextureCache
{
public:
    static PatchTextureCache& ins() { static PatchTextureCache c; return c; }

    void resetFrame(int budget) { budget_ = budget; pending_ = 0; }
    int  pendingThisFrame() const { return pending_; }

    SDL_Texture* getOrBuild(int sx, int sy, int sz)
    {
        Key k{ sx, sy, sz };
        if (auto it = textures_.find(k); it != textures_.end()) return it->second;

        const Patch* biome = World::ins()->getPatch(sx, sy, sz);
        if (!biome) return nullptr;  // 데이터 부재 — pending 아님

        if (budget_ <= 0) { pending_++; return nullptr; }

        SDL_Texture* tex = build(biome);
        if (!tex) return nullptr;
        textures_[k] = tex;
        budget_--;
        return tex;
    }

    void clear()
    {
        for (auto& [k, t] : textures_) if (t) SDL_DestroyTexture(t);
        textures_.clear();
    }

private:
    static SDL_Texture* build(const Patch* biome)
    {
        SDL_Surface* surf = SDL_CreateSurface(PIXEL_PER_PATCH, PIXEL_PER_PATCH, SDL_PIXELFORMAT_RGBA32);
        if (!surf) return nullptr;

        SDL_LockSurface(surf);
        const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surf->format);
        for (int py = 0; py < PIXEL_PER_PATCH; py++)
        {
            std::uint32_t* row = (std::uint32_t*)((std::uint8_t*)surf->pixels + py * surf->pitch);
            for (int px = 0; px < PIXEL_PER_PATCH; px++)
            {
                SDL_Color c = mappal::biomeColor(biome->get(px, py));
                row[px] = SDL_MapRGBA(fmt, nullptr, c.r, c.g, c.b, c.a);
            }
        }
        SDL_UnlockSurface(surf);

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
        if (tex) SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        return tex;
    }

    struct Key { int sx, sy, sz; bool operator==(const Key&) const = default; };
    struct KeyHash
    {
        std::size_t operator()(const Key& k) const noexcept
        {
            std::size_t h = (std::size_t)(k.sx + 1024) * 65537ull;
            h ^= (std::size_t)(k.sy + 1024) * 257ull;
            h ^= (std::size_t)(k.sz + 8) * 31ull;
            return h;
        }
    };
    std::unordered_map<Key, SDL_Texture*, KeyHash> textures_;
    int budget_  = 0;
    int pending_ = 0;
};


// ════════════════════════════════════════════════════════════════════════
// §4  데이터 로딩 (PNG 바이옴 자동 로드)
// ════════════════════════════════════════════════════════════════════════

// 가시 영역의 미로드 패치를 PNG 만 로드. 프레임당 한도로 휠 스파이크 방지.
//   loadVisible() 의 반환값 = 아직 로드 안 된 패치 수 → 스피너 트리거.
class PatchAutoLoader
{
public:
    static int loadVisible(const MapView& v)
    {
        if (v.pxPerTile > mapcfg::AUTOLOAD_MAX_ZOOM) return 0;

        double minTX, minTY, maxTX, maxTY;
        v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

        int minSX = (int)std::floor(minTX / TILE_PER_PATCH);
        int minSY = (int)std::floor(minTY / TILE_PER_PATCH);
        int maxSX = (int)std::floor(maxTX / TILE_PER_PATCH);
        int maxSY = (int)std::floor(maxTY / TILE_PER_PATCH);

        int loaded = 0;
        int stillPending = 0;
        for (int sy = minSY; sy <= maxSY; sy++)
        {
            for (int sx = minSX; sx <= maxSX; sx++)
            {
                if (!World::ins()->isEmptyPatch(sx, sy, v.z)) continue;
                if (loaded < mapcfg::FRAME_BUDGET_PATCH_LOAD)
                {
                    World::ins()->createPatch(sx, sy, v.z);
                    loaded++;
                }
                else
                {
                    stillPending++;
                }
            }
        }
        return stillPending;
    }
};


// ════════════════════════════════════════════════════════════════════════
// §5  렌더링 계층
// ════════════════════════════════════════════════════════════════════════

// (1) 바이옴 베이스 — 가시 패치 텍스처를 적절히 스케일해 blit
static void drawBiomeLayer(const MapView& v)
{
    double minTX, minTY, maxTX, maxTY;
    v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

    int minSX = (int)std::floor(minTX / TILE_PER_PATCH);
    int minSY = (int)std::floor(minTY / TILE_PER_PATCH);
    int maxSX = (int)std::floor(maxTX / TILE_PER_PATCH);
    int maxSY = (int)std::floor(maxTY / TILE_PER_PATCH);

    double patchScreenSize = (double)TILE_PER_PATCH * v.pxPerTile;

    for (int sy = minSY; sy <= maxSY; sy++)
    {
        for (int sx = minSX; sx <= maxSX; sx++)
        {
            SDL_Texture* tex = PatchTextureCache::ins().getOrBuild(sx, sy, v.z);
            if (!tex) continue;  // 미빌드 → 다음 프레임에 채워짐

            double dstX = v.screenXFromTileX((double)sx * TILE_PER_PATCH);
            double dstY = v.screenYFromTileY((double)sy * TILE_PER_PATCH);
            SDL_FRect dst{
                (float)std::floor(dstX),
                (float)std::floor(dstY),
                (float)std::ceil(patchScreenSize) + 1.0f,
                (float)std::ceil(patchScreenSize) + 1.0f
            };
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
        }
    }
}

// (2) 타일 스프라이트 레이어 — 생성된 청크의 floor / wall 을 spr::tileset 으로 직접 그림.
//   SDL_RenderGeometry 직접 사용: drawSpriteBatchCenter 는 위치를 Point2(int) 로
//   받아 정수 픽셀에 스냅 → 분수 줌에서 누적 트런케이션 드리프트로 ~1/frac
//   타일마다 1 px 갭 발생. 여기서는 모서리 4개 모두 screenXFromTileX/(Y) 로
//   직접 도출해 인접 타일 quad 가 비트 동일한 float 경계를 공유 → 갭 X.
namespace
{
    struct TileBatchEntry { float left, top, right, bottom; int sprIdx; };

    inline void flushTileBatch(TileBatchEntry* entries, int count)
    {
        if (count <= 0) return;

        SDL_Texture* tex = spr::tileset->getTexture();
        float texW, texH;
        SDL_GetTextureSize(tex, &texW, &texH);

        const int   srcSize = spr::tileset->getW();           // 16
        const float uW      = (float)srcSize / texW;
        const float vH      = (float)srcSize / texH;
        const int   atlasW  = (int)texW;

        // atlas bleeding 방지 — UV 를 0.5 텍셀 안쪽으로 inset 해서 샘플이 항상
        //   픽셀 중심을 향하게 함. 분수 줌에서 옆 스프라이트의 첫 텍셀이 새는
        //   것 (예: 화면 가로로 한 줄 초록 띠) 차단.
        const float insetU = 0.5f / texW;
        const float insetV = 0.5f / texH;

        static SDL_Vertex vertices[MAX_BATCH * 4];
        static int        indices [MAX_BATCH * 6];
        const SDL_FColor white = { 1.0f, 1.0f, 1.0f, 1.0f };

        for (int i = 0; i < count; i++)
        {
            const int sprIdx = entries[i].sprIdx;
            const float u  = (float)((srcSize * sprIdx) % atlasW) / texW;
            const float vY = (float)(srcSize * ((srcSize * sprIdx) / atlasW)) / texH;

            const float u0 = u + insetU,        u1 = u + uW - insetU;
            const float v0 = vY + insetV,       v1 = vY + vH - insetV;

            const float l = entries[i].left;
            const float t = entries[i].top;
            const float r = entries[i].right;
            const float b = entries[i].bottom;

            const int vBase = i * 4;
            vertices[vBase    ] = { { l, t }, white, { u0, v0 } };
            vertices[vBase + 1] = { { r, t }, white, { u1, v0 } };
            vertices[vBase + 2] = { { r, b }, white, { u1, v1 } };
            vertices[vBase + 3] = { { l, b }, white, { u0, v1 } };

            const int iBase = i * 6;
            indices[iBase    ] = vBase;
            indices[iBase + 1] = vBase + 1;
            indices[iBase + 2] = vBase + 2;
            indices[iBase + 3] = vBase;
            indices[iBase + 4] = vBase + 2;
            indices[iBase + 5] = vBase + 3;
        }

        SDL_RenderGeometry(renderer, tex, vertices, count * 4, indices, count * 6);
    }
}

static void drawTileSpriteLayer(const MapView& v)
{
    double minTX, minTY, maxTX, maxTY;
    v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

    static thread_local TileBatchEntry batch[MAX_BATCH];
    int count = 0;

    auto flush  = [&]() { flushTileBatch(batch, count); count = 0; };
    auto submit = [&](int sprIdx, int tx, int ty)
    {
        if (count >= MAX_BATCH) flush();
        // 인접 타일이 비트 동일한 float 경계를 공유하도록 모서리를 직접 도출.
        //   (tx+1) 호출은 다음 타일의 left 와 비트 동일 (같은 함수, 같은 입력).
        batch[count].left   = (float)v.screenXFromTileX((double)tx);
        batch[count].top    = (float)v.screenYFromTileY((double)ty);
        batch[count].right  = (float)v.screenXFromTileX((double)(tx + 1));
        batch[count].bottom = (float)v.screenYFromTileY((double)(ty + 1));
        batch[count].sprIdx = sprIdx;
        count++;
    };

    World::ins()->forEachChunkAtZ(v.z, [&](int cx, int cy)
    {
        // 청크 바운딩으로 가시 컬링
        double tx0 = (double)cx * CHUNK_SIZE_X;
        double ty0 = (double)cy * CHUNK_SIZE_Y;
        if (tx0 + CHUNK_SIZE_X < minTX || tx0 > maxTX) return;
        if (ty0 + CHUNK_SIZE_Y < minTY || ty0 > maxTY) return;

        for (int ly = 0; ly < CHUNK_SIZE_Y; ly++)
        {
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++)
            {
                int tx = (int)tx0 + lx;
                int ty = (int)ty0 + ly;
                const TileData& td = World::ins()->getTile(tx, ty, v.z);

                int floorId = td.floor;
                if (floorId != 0)
                {
                    int sprIdx = itemDex[floorId].tileSprIndex
                               + itemDex[floorId].extraSprIndexSingle
                               + 16 * itemDex[floorId].extraSprIndex16;
                    submit(sprIdx, tx, ty);
                }

                int wallId = td.wall;
                if (wallId != 0)
                {
                    int sprIdx = itemDex[wallId].tileSprIndex
                               + itemDex[wallId].extraSprIndexSingle
                               + 16 * itemDex[wallId].extraSprIndex16;
                    submit(sprIdx, tx, ty);
                }
            }
        }
    });

    flush();
}

// (3) 플레이어 마커 — 화면 안이면 펄스 마커, 화면 밖이면 가장자리 클램프
static void drawPlayerMarker(const MapView& v)
{
    double sxd = v.screenXFromTileX(PlayerX());
    double syd = v.screenYFromTileY(PlayerY());

    bool offscreen = (sxd < 0 || sxd > v.viewW || syd < 0 || syd > v.viewH);
    if (offscreen)
    {
        int ex = (int)std::clamp(sxd, 16.0, (double)v.viewW - 16.0);
        int ey = (int)std::clamp(syd, 16.0, (double)v.viewH - 16.0);
        drawFillRect(SDL_Rect{ ex - 7, ey - 7, 14, 14 }, mappal::playerMarker());
        drawRect(SDL_Rect{ ex - 7, ey - 7, 14, 14 }, SDL_Color{ 255, 255, 255, 255 });
        return;
    }

    int sx = (int)std::round(sxd);
    int sy = (int)std::round(syd);
    bool on = (SDL_GetTicks() % 900) < 600;
    if (on)
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
// §6  UI 크롬 (좌상단 좌표, 좌하단 줌, 우상단 Tab, 우하단 로딩)
// ════════════════════════════════════════════════════════════════════════

// 좌표 패널 — 헤더 + 페이드 separator + X/Y/Z 우측정렬 표
static void drawCoordPanel()
{
    SDL_Rect panel{ 22, 22, 240, 134 };
    drawStadium(panel.x, panel.y, panel.w, panel.h, mappal::uiPanel(), 220, 3);

    setFontSize(16);
    const std::wstring header = L"Player Coordinates";
    setFont(fontType::mainFontBold);
    drawText(header, panel.x + 14, panel.y + 8, mappal::uiText());
    setFont(fontType::mainFont);

    // 헤더 글자폭 + 8px 까지 솔리드, 그 뒤 알파 페이드
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

    // X/Y/Z — 라벨 좌측, 숫자 우측정렬
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

// 줌 패널 — +/-/@ 버튼 + 현재 배율 표시
struct ZoomButtons { SDL_Rect zoomIn, zoomOut, home; };

static ZoomButtons computeZoomButtons()
{
    constexpr int margin = 22, btnSize = 64, gap = 10;
    int yBase = cameraH - margin - btnSize;
    return {
        { margin,                            yBase, btnSize, btnSize },
        { margin + btnSize + gap,            yBase, btnSize, btnSize },
        { margin + (btnSize + gap) * 2,      yBase, btnSize, btnSize }
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
    std::wostringstream oss;
    oss << L"Zoom  ";
    oss.precision(2);
    oss << std::fixed << v.pxPerTile << L" px/tile";
    drawText(oss.str(), panelX + 14, panelY + 8, mappal::uiText());

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

// 우상단 Tab 버튼 — HUD::drawTab 의 tabFlag::back 케이스 재현
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

// 우하단 로딩 패널 — 스피너 + 진행 카운트. pending 0 이면 호출자가 스킵.
struct LoadingStats
{
    int patchesLoading    = 0;  // PNG 바이옴 미로드 (자동 로드 대기)
    int patchesBuilding   = 0;  // 바이옴 텍스처 빌드 대기
    int total() const { return patchesLoading + patchesBuilding; }
};

// 두 개의 75도 호가 180도 간격으로 회전 — 머리는 또렷하고 꼬리는 제곱 페이드.
// 호 한 개당 36개의 라디얼 라인을 쌓아서 2px 두께를 만들고, 머리 끝에는
// 작은 글로우 점을 더해 회전 방향을 강조한다.
static void drawLoadingSpinner(int cx, int cy)
{
    constexpr float PI    = 3.14159265f;
    constexpr float TWOPI = 6.28318531f;
    constexpr float ARC   = 1.30f;          // ≈ 75°
    constexpr float SPIN  = 3.0f;           // rad/sec ≈ 0.48 회전/초
    constexpr float R_IN  = 12.0f;
    constexpr float R_OUT = 14.0f;
    constexpr int   N     = 36;             // 호당 라디얼 분할

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const float t   = (float)SDL_GetTicks() / 1000.0f;
    const float rot = std::fmod(t * SPIN, TWOPI);
    const SDL_Color col = mappal::uiText();

    for (int k = 0; k < 2; ++k)
    {
        const float head = rot + (float)k * PI;
        for (int s = 0; s < N; ++s)
        {
            const float frac = (float)s / (float)(N - 1);   // 0=머리, 1=꼬리
            const float ang  = head - frac * ARC;
            const float fade = 1.0f - frac;
            const Uint8 a = (Uint8)(245.0f * fade * fade);
            if (a < 6) continue;
            const float c = std::cos(ang), si = std::sin(ang);
            drawLine(
                (int)std::round((float)cx + R_IN  * c),
                (int)std::round((float)cy + R_IN  * si),
                (int)std::round((float)cx + R_OUT * c),
                (int)std::round((float)cy + R_OUT * si),
                col, a);
        }
        // 머리 끝 글로우 점 — 회전 방향성 강조
        const float c = std::cos(head), si = std::sin(head);
        const float rMid = (R_IN + R_OUT) * 0.5f;
        drawFillCircle(
            (int)std::round((float)cx + rMid * c),
            (int)std::round((float)cy + rMid * si),
            2, col, 220);
    }
}

static void drawLoadingPanel(const LoadingStats& stats)
{
    constexpr int margin  = 22;
    constexpr int panelW  = 220;
    constexpr int panelH  = 56;
    int panelX = cameraW - margin - panelW;
    int panelY = cameraH - margin - panelH;
    drawStadium(panelX, panelY, panelW, panelH, mappal::uiPanel(), 220, 3);

    // 좌측 텍스트
    setFontSize(14);
    drawText(L"Loading map data", panelX + 16, panelY + 10, mappal::uiText());

    setFontSize(12);
    std::wstring sub = std::to_wstring(stats.total()) + L" regions pending";
    drawText(sub, panelX + 16, panelY + 30, mappal::uiBorder());

    // 우측 스피너
    drawLoadingSpinner(panelX + panelW - 28, panelY + panelH / 2);
}


// ════════════════════════════════════════════════════════════════════════
// §7  Map 클래스
// ════════════════════════════════════════════════════════════════════════

export class Map : public GUI
{
private:
    inline static Map* ptr = nullptr;
    MapView view;

    // 드래그 상태
    bool   dragging        = false;
    bool   dragMoved       = false;
    double dragAnchorTileX = 0.0;
    double dragAnchorTileY = 0.0;

    // 줌은 Map 인스턴스 수명 넘어 영속. 센터/Z 는 매 열기마다 플레이어 기준으로 리셋.
    inline static double persistedPxPerTile = mapcfg::DEFAULT_PX_PER_TILE;

public:
    Map() : GUI(false)
    {
        errorBox(ptr != nullptr, L"More than one Map instance was generated.");
        ptr = this;

        view.viewW = cameraW;
        view.viewH = cameraH;
        view.z = PlayerZ();
        view.centerTileX = (double)PlayerX();
        view.centerTileY = (double)PlayerY();
        view.pxPerTile = persistedPxPerTile;
        x = 0; y = 0;

        // 플레이어 주변 3x3 패치 PNG 선행 로드 (자동로드가 못 따라잡는 즉시 영역)
        Point2 ps = World::ins()->changeToPatchCoord(PlayerX(), PlayerY());
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
            {
                int sx = ps.x + dx, sy = ps.y + dy;
                if (World::ins()->isEmptyPatch(sx, sy, PlayerZ()))
                    World::ins()->createPatch(sx, sy, PlayerZ());
            }

        // 텍스처 캐시 영속 — clear() 안 함. 다시 열어도 즉시 풀 디테일.

        deactInput();
        deactDraw();
        addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
    }

    ~Map() { persistedPxPerTile = view.pxPerTile; ptr = nullptr; }

    static Map* ins() { return ptr; }

    void changeXY(int /*ix*/, int /*iy*/, bool /*center*/) override { x = 0; y = 0; }

    void drawGUI() override
    {
        if (getStateDraw() == false) return;

        // 펼침/닫힘 애니메이션 — 박스만 그리고 종료. 빌드 작업 안 함.
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

        // 매 프레임 작업 예산 초기화
        PatchTextureCache::ins().resetFrame(mapcfg::FRAME_BUDGET_PATCHES);

        // 가시 미로드 패치 자동 로드 (PNG 만)
        int patchesLoadPending = PatchAutoLoader::loadVisible(view);

        // 렌더
        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, mappal::background());
        drawBiomeLayer       (view);  // 내부에서 cache.getOrBuild → budget 안에서 빌드
        drawTileSpriteLayer  (view);
        drawPlayerMarker     (view);

        drawCoordPanel();
        drawZoomPanel(view, computeZoomButtons());
        drawTabButton();

        // 진행 표시 — 이번 프레임 budget 부족으로 미룬 작업이 있으면
        LoadingStats stats{
            patchesLoadPending,
            PatchTextureCache::ins().pendingThisFrame()
        };
        if (stats.total() > 0) drawLoadingPanel(stats);
    }

    // ────────── 입력 ──────────
    void clickDownGUI() override
    {
        // 좌클릭(또는 터치)만 드래그 시작. 우클릭/휠클릭은 무시.
        if (option::inputMethod == input::mouse && event.button.button != SDL_BUTTON_LEFT) return;

        if (checkCursor(&tab)) return;
        ZoomButtons zb = computeZoomButtons();
        if (checkCursor(&zb.zoomIn) || checkCursor(&zb.zoomOut) || checkCursor(&zb.home)) return;

        dragging = true;
        dragMoved = false;
        dragAnchorTileX = view.centerTileX;
        dragAnchorTileY = view.centerTileY;
    }

    void clickMotionGUI(int dx, int dy) override
    {
        if (!dragging) return;
        if (dx * dx + dy * dy > 16) dragMoved = true;
        view.centerTileX = dragAnchorTileX + dx / view.pxPerTile;
        view.centerTileY = dragAnchorTileY + dy / view.pxPerTile;
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
        if (checkCursor(&zb.zoomIn))
        {
            view.zoomAround(view.viewW / 2, view.viewH / 2, +2);
            return;
        }
        if (checkCursor(&zb.zoomOut))
        {
            view.zoomAround(view.viewW / 2, view.viewH / 2, -2);
            return;
        }
        if (checkCursor(&zb.home))
        {
            view.centerTileX = (double)PlayerX();
            view.centerTileY = (double)PlayerY();
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
