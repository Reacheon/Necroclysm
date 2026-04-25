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
import SectorBiome;
import TileData;
import BuildingTemplate;
import WorldLandmark;

// ════════════════════════════════════════════════════════════════════════
// Map — 풀스크린 인터랙티브 월드맵 (구글지도 스타일)
//
//   파일 구성 (위→아래)
//     §1  설정/팔레트       — mapcfg, mappal
//     §2  카메라             — MapView
//     §3  텍스처 캐시        — SectorBiomeTextureCache, ChunkTextureCache
//     §4  데이터 로딩        — SectorAutoLoader (PNG 바이옴 백그라운드 로드)
//     §5  렌더링 계층        — drawBiomeLayer, drawChunkOverlay,
//                              drawLandmarkLabels, drawPlayerMarker
//     §6  UI 크롬             — drawCoordPanel, drawZoomPanel, drawTabButton,
//                              drawLoadingPanel
//     §7  Map 클래스         — 입력 처리 + 레이아웃 합성
//
//   디자인 원칙
//     · 게으른 빌드 (lazy build): Map 열릴 때 일괄 빌드 ❌. 매 프레임
//       프레임-budget 만큼만 점진 빌드. 사용자는 점차 채워지는 시각 피드백.
//     · 영속 캐시: Map 닫혀도 텍스처 유지 → 다시 열어도 즉시 풀 디테일.
//     · 명시적 진행 표시: 미완 빌드가 있는 동안 우하단에 로딩 스피너.
//     · 단일 스레드 안전: 모든 SDL 호출은 메인 스레드. 멀티스레드 미사용.
// ════════════════════════════════════════════════════════════════════════


// ════════════════════════════════════════════════════════════════════════
// §1  설정 / 팔레트
// ════════════════════════════════════════════════════════════════════════

namespace mapcfg
{
    // 줌 한계 — pxPerTile = 스크린 픽셀 / 월드 타일
    inline constexpr double MIN_PX_PER_TILE      = 0.04;  // 1 sector px ≈ 2 screen px
    inline constexpr double MAX_PX_PER_TILE      = 6.0;   // 1 tile = 6 screen px
    inline constexpr double DEFAULT_PX_PER_TILE  = 0.6;

    // 청크 디테일 오버레이 표시 임계 줌
    inline constexpr double CHUNK_OVERLAY_MIN_ZOOM = 0.25;

    // 랜드마크 라벨 표시 임계 줌
    inline constexpr double LANDMARK_MIN_ZOOM      = 0.30;

    // 마우스 휠 한 칸당 줌 배율
    inline constexpr double WHEEL_ZOOM_FACTOR      = 1.18;

    // 프레임당 신규 빌드 한도 (영속 캐시이므로 첫 방문 영역에만 쓰임).
    //   값을 키우면 더 빠르게 채워지지만 프레임 시간 증가.
    //   현재 값: 60fps 한 프레임(≈16ms) 안에 안전히 들어갈 정도.
    inline constexpr int FRAME_BUDGET_SECTORS = 2;     // 섹터 1개 ≈ 5~8ms
    inline constexpr int FRAME_BUDGET_CHUNKS  = 32;    // 청크 1개 ≈ 0.2ms

    // 가시 미로드 섹터 자동 로드 한도
    inline constexpr int FRAME_BUDGET_SECTOR_LOAD = 2;

    // 가시 영역 pxPerTile 이 이 값 이하일 때만 섹터 자동 로드
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
        case chunkFlag::portal:      return { 220, 100, 110, 255 };
        case chunkFlag::underground: return { 130, 130, 130, 255 };
        default:                     return {  18,  18,  22, 255 };
        }
    }

    // 청크 floor 오버레이 색. nullopt 면 바이옴 색 유지.
    inline std::optional<SDL_Color> floorOverlay(int floorId)
    {
        if (floorId == itemID::blackAsphalt)  return SDL_Color{  90,  92,  95, 255 };
        if (floorId == itemID::yellowAsphalt) return SDL_Color{ 220, 200,  90, 255 };
        if (floorId == itemID::whiteAsphalt)  return SDL_Color{ 235, 235, 235, 255 };
        if (floorId == itemID::paver)         return SDL_Color{ 200, 200, 195, 255 };
        return std::nullopt;
    }

    inline SDL_Color wallOverlay()  { return {  60,  60,  62, 255 }; }
    inline SDL_Color background()   { return {  10,  10,  14, 255 }; }
    inline SDL_Color playerMarker() { return { 220,  80,  80, 255 }; }

    // 라벨
    inline SDL_Color labelText()    { return {  35,  35,  35, 255 }; }
    inline SDL_Color labelBg()      { return { 245, 240, 230, 235 }; }
    inline SDL_Color labelStroke()  { return { 110, 105,  95, 255 }; }

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

    void zoomAround(int anchorScreenX, int anchorScreenY, double factor)
    {
        double anchorTX = tileXFromScreenX(anchorScreenX);
        double anchorTY = tileYFromScreenY(anchorScreenY);
        pxPerTile = std::clamp(pxPerTile * factor, mapcfg::MIN_PX_PER_TILE, mapcfg::MAX_PX_PER_TILE);
        centerTileX = anchorTX + (viewW * 0.5 - anchorScreenX) / pxPerTile;
        centerTileY = anchorTY + (viewH * 0.5 - anchorScreenY) / pxPerTile;
    }
};


// ════════════════════════════════════════════════════════════════════════
// §3  텍스처 캐시
//
//   두 캐시는 같은 인터페이스 패턴을 따름:
//     resetFrame(budget) — 매 프레임 시작에 호출. budget 한도 + 카운터 초기화.
//     getOrBuild(key)    — hit 시 즉시 반환. miss 면 budget 소진까지 빌드 시도.
//                          budget 0 이면 nullptr + pendingThisFrame() 증가.
//                          underlying data 부재면 nullptr (pending 카운트 안 함).
//     pendingThisFrame() — 이번 프레임에 budget 부족으로 미룬 빌드 수.
//                          → 0 보다 크면 로딩 진행 중.
//     clear()            — 모든 텍스처 파괴 (월드 리셋 등).
// ════════════════════════════════════════════════════════════════════════

// 섹터 한 장 = PIXEL_PER_SECTOR × PIXEL_PER_SECTOR 텍스처 (1 px = 1 sector pixel).
class SectorBiomeTextureCache
{
public:
    static SectorBiomeTextureCache& ins() { static SectorBiomeTextureCache c; return c; }

    void resetFrame(int budget) { budget_ = budget; pending_ = 0; }
    int  pendingThisFrame() const { return pending_; }

    SDL_Texture* getOrBuild(int sx, int sy, int sz)
    {
        Key k{ sx, sy, sz };
        if (auto it = textures_.find(k); it != textures_.end()) return it->second;

        const SectorBiome* biome = World::ins()->getSectorBiome(sx, sy, sz);
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
    static SDL_Texture* build(const SectorBiome* biome)
    {
        SDL_Surface* surf = SDL_CreateSurface(PIXEL_PER_SECTOR, PIXEL_PER_SECTOR, SDL_PIXELFORMAT_RGBA32);
        if (!surf) return nullptr;

        SDL_LockSurface(surf);
        const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surf->format);
        for (int py = 0; py < PIXEL_PER_SECTOR; py++)
        {
            std::uint32_t* row = (std::uint32_t*)((std::uint8_t*)surf->pixels + py * surf->pitch);
            for (int px = 0; px < PIXEL_PER_SECTOR; px++)
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

// 청크 한 칸 = CHUNK_SIZE_X × CHUNK_SIZE_Y 텍스처 (1 px = 1 tile).
//   투명 픽셀 → 바이옴 베이스가 비쳐 보임 (BLEND 모드).
class ChunkTextureCache
{
public:
    static ChunkTextureCache& ins() { static ChunkTextureCache c; return c; }

    void resetFrame(int budget) { budget_ = budget; pending_ = 0; }
    int  pendingThisFrame() const { return pending_; }

    SDL_Texture* getOrBuild(int cx, int cy, int cz)
    {
        Key k{ cx, cy, cz };
        if (auto it = textures_.find(k); it != textures_.end()) return it->second;

        if (!World::ins()->existChunk(cx, cy, cz)) return nullptr;  // 데이터 부재

        if (budget_ <= 0) { pending_++; return nullptr; }

        SDL_Texture* tex = build(cx, cy, cz);
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
    static SDL_Texture* build(int cx, int cy, int cz)
    {
        SDL_Surface* surf = SDL_CreateSurface(CHUNK_SIZE_X, CHUNK_SIZE_Y, SDL_PIXELFORMAT_RGBA32);
        if (!surf) return nullptr;

        SDL_LockSurface(surf);
        const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surf->format);
        const std::uint32_t transparent = SDL_MapRGBA(fmt, nullptr, 0, 0, 0, 0);

        int originTX = cx * CHUNK_SIZE_X;
        int originTY = cy * CHUNK_SIZE_Y;
        for (int ly = 0; ly < CHUNK_SIZE_Y; ly++)
        {
            std::uint32_t* row = (std::uint32_t*)((std::uint8_t*)surf->pixels + ly * surf->pitch);
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++)
            {
                const TileData& td = World::ins()->getTile(originTX + lx, originTY + ly, cz);
                if (td.wall != 0)
                {
                    SDL_Color c = mappal::wallOverlay();
                    row[lx] = SDL_MapRGBA(fmt, nullptr, c.r, c.g, c.b, c.a);
                }
                else if (auto fc = mappal::floorOverlay(td.floor))
                    row[lx] = SDL_MapRGBA(fmt, nullptr, fc->r, fc->g, fc->b, fc->a);
                else
                    row[lx] = transparent;
            }
        }
        SDL_UnlockSurface(surf);

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
        if (tex)
        {
            SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        }
        return tex;
    }

    struct Key { int cx, cy, cz; bool operator==(const Key&) const = default; };
    struct KeyHash
    {
        std::size_t operator()(const Key& k) const noexcept
        {
            std::size_t h = (std::size_t)(k.cx + 65536) * 1000003ull;
            h ^= (std::size_t)(k.cy + 65536) * 65537ull;
            h ^= (std::size_t)(k.cz + 8) * 31ull;
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

// 가시 영역의 미로드 섹터를 PNG 만 로드 (CityGen 미트리거 — 안전).
//   프레임당 한도 (FRAME_BUDGET_SECTOR_LOAD) 로 휠 스파이크 방지.
//   pendingCount() 로 아직 로드 안 된 섹터 수 조회 → 스피너 트리거.
class SectorAutoLoader
{
public:
    static int loadVisible(const MapView& v)
    {
        if (v.pxPerTile > mapcfg::AUTOLOAD_MAX_ZOOM) return 0;

        double minTX, minTY, maxTX, maxTY;
        v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

        int minSX = (int)std::floor(minTX / TILE_PER_SECTOR);
        int minSY = (int)std::floor(minTY / TILE_PER_SECTOR);
        int maxSX = (int)std::floor(maxTX / TILE_PER_SECTOR);
        int maxSY = (int)std::floor(maxTY / TILE_PER_SECTOR);

        int loaded = 0;
        int stillPending = 0;
        for (int sy = minSY; sy <= maxSY; sy++)
        {
            for (int sx = minSX; sx <= maxSX; sx++)
            {
                if (!World::ins()->isEmptySector(sx, sy, v.z)) continue;
                if (loaded < mapcfg::FRAME_BUDGET_SECTOR_LOAD)
                {
                    World::ins()->createSector(sx, sy, v.z);
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
//
//   각 함수는 MapView 만 받아 화면에 그림. 모듈 내부 자유함수.
//   layer 그리기 도중 cache.getOrBuild 가 호출되며, budget 안에서 빌드.
// ════════════════════════════════════════════════════════════════════════

// (1) 바이옴 베이스 — 가시 섹터 텍스처를 적절히 스케일해 blit
static void drawBiomeLayer(const MapView& v)
{
    double minTX, minTY, maxTX, maxTY;
    v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

    int minSX = (int)std::floor(minTX / TILE_PER_SECTOR);
    int minSY = (int)std::floor(minTY / TILE_PER_SECTOR);
    int maxSX = (int)std::floor(maxTX / TILE_PER_SECTOR);
    int maxSY = (int)std::floor(maxTY / TILE_PER_SECTOR);

    double sectorScreenSize = (double)TILE_PER_SECTOR * v.pxPerTile;

    for (int sy = minSY; sy <= maxSY; sy++)
    {
        for (int sx = minSX; sx <= maxSX; sx++)
        {
            SDL_Texture* tex = SectorBiomeTextureCache::ins().getOrBuild(sx, sy, v.z);
            if (!tex) continue;  // 미빌드 → 다음 프레임에 채워짐

            double dstX = v.screenXFromTileX((double)sx * TILE_PER_SECTOR);
            double dstY = v.screenYFromTileY((double)sy * TILE_PER_SECTOR);
            SDL_FRect dst{
                (float)std::floor(dstX),
                (float)std::floor(dstY),
                (float)std::ceil(sectorScreenSize) + 1.0f,
                (float)std::ceil(sectorScreenSize) + 1.0f
            };
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
        }
    }
}

// (2) 청크 디테일 오버레이 — 가시 청크 텍스처를 blit (충분한 줌일 때만)
static void drawChunkOverlay(const MapView& v)
{
    if (v.pxPerTile < mapcfg::CHUNK_OVERLAY_MIN_ZOOM) return;

    double minTX, minTY, maxTX, maxTY;
    v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

    int minCX = (int)std::floor(minTX / CHUNK_SIZE_X);
    int minCY = (int)std::floor(minTY / CHUNK_SIZE_Y);
    int maxCX = (int)std::ceil (maxTX / CHUNK_SIZE_X);
    int maxCY = (int)std::ceil (maxTY / CHUNK_SIZE_Y);

    double chunkScreenW = (double)CHUNK_SIZE_X * v.pxPerTile;
    double chunkScreenH = (double)CHUNK_SIZE_Y * v.pxPerTile;

    for (int cy = minCY; cy <= maxCY; cy++)
    {
        for (int cx = minCX; cx <= maxCX; cx++)
        {
            SDL_Texture* tex = ChunkTextureCache::ins().getOrBuild(cx, cy, v.z);
            if (!tex) continue;  // 미빌드 → 다음 프레임에 채워짐 (바이옴이 비침)

            double dstX = v.screenXFromTileX((double)cx * CHUNK_SIZE_X);
            double dstY = v.screenYFromTileY((double)cy * CHUNK_SIZE_Y);
            SDL_FRect dst{
                (float)std::floor(dstX),
                (float)std::floor(dstY),
                (float)std::ceil(chunkScreenW) + 1.0f,
                (float)std::ceil(chunkScreenH) + 1.0f
            };
            SDL_RenderTexture(renderer, tex, nullptr, &dst);
        }
    }
}

// (3) 라벨 — 단일 박스 그리기 도우미
static void drawLabelBox(int sx, int sy, const std::wstring& text)
{
    int textW = queryTextWidth(text);
    constexpr int textH = 13, padX = 5, padY = 2;
    SDL_Rect bg{ sx - textW / 2 - padX, sy - textH / 2 - padY, textW + 2 * padX, textH + 2 * padY };
    drawFillRect(bg, mappal::labelBg());
    drawRect(bg, mappal::labelStroke());
    drawTextCenter(text, sx, sy, mappal::labelText());
}

// (3) 랜드마크 라벨 — 가시 + 줌 + 건물 크기 임계값 모두 통과한 것만
static void drawLandmarkLabels(const MapView& v)
{
    if (v.pxPerTile < mapcfg::LANDMARK_MIN_ZOOM) return;

    double minTX, minTY, maxTX, maxTY;
    v.visibleTileBounds(minTX, minTY, maxTX, maxTY);

    int minTXi = (int)std::floor(minTX) - 64;
    int minTYi = (int)std::floor(minTY) - 64;
    int maxTXi = (int)std::ceil (maxTX) + 64;
    int maxTYi = (int)std::ceil (maxTY) + 64;

    setFontSize(11);

    // 라벨 겹침 방지 — 단순 AABB 검사
    std::vector<SDL_Rect> placed;
    placed.reserve(64);

    LandmarkRegistry::ins().forEachIn(minTXi, minTYi, maxTXi, maxTYi, v.z,
        [&](const Landmark& lm)
        {
            if (lm.name.empty()) return;

            int minDim = std::min(lm.w, lm.h);
            int screenSize = (int)(minDim * v.pxPerTile);
            if (screenSize < 22) return;  // 화면상 너무 작은 건물은 스킵

            double centerTX = lm.tileX + lm.w * 0.5;
            double centerTY = lm.tileY + lm.h * 0.5;
            int sx = (int)std::round(v.screenXFromTileX(centerTX));
            int sy = (int)std::round(v.screenYFromTileY(centerTY));

            int tw = queryTextWidth(lm.name);
            SDL_Rect myBox{ sx - tw / 2 - 5, sy - 13 / 2 - 2, tw + 10, 17 };

            for (const auto& p : placed)
            {
                if (myBox.x < p.x + p.w && myBox.x + myBox.w > p.x &&
                    myBox.y < p.y + p.h && myBox.y + myBox.h > p.y) return;
            }
            placed.push_back(myBox);

            drawLabelBox(sx, sy, lm.name);
        });
}

// (4) 플레이어 마커 — 화면 안이면 펄스 마커, 화면 밖이면 가장자리 클램프
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
//   알파 220 으로 좌측 패널들과 톤 일치, 베이스 색은 uiPanel 과 동일.
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

// 우하단 로딩 패널 — 스피너 + 진행 카운트.
//   pending 합계가 0 이면 호출자가 그리지 않음 (이 함수는 항상 그리는 것을 가정).
struct LoadingStats
{
    int sectorsLoading    = 0;  // PNG 바이옴 미로드 (자동 로드 대기)
    int sectorsBuilding   = 0;  // 바이옴 텍스처 빌드 대기
    int chunksBuilding    = 0;  // 청크 오버레이 텍스처 빌드 대기
    int total() const { return sectorsLoading + sectorsBuilding + chunksBuilding; }
};

// 12개 사각 픽셀이 원형으로 배치, 시간에 따라 밝기가 회전 (혜성 트레일).
//   12개로 늘려 팔각형 느낌 제거 + 충분한 반지름 + 정사각 픽셀로 깔끔하게.
static void drawLoadingSpinner(int cx, int cy)
{
    constexpr int    N        = 12;
    constexpr double RADIUS   = 16.0;
    constexpr int    DOT      = 3;            // 3×3 사각형
    constexpr double TWO_PI   = 6.28318530718;
    constexpr double START    = -TWO_PI / 4;  // 12시 방향에서 시작
    constexpr double SPIN_MS  = 1000.0;       // 1 회전당 1초

    double phase = (double)SDL_GetTicks() * (double)N / SPIN_MS;

    for (int i = 0; i < N; i++)
    {
        double angle = START + (double)i / (double)N * TWO_PI;
        int dx = (int)std::round(std::cos(angle) * RADIUS);
        int dy = (int)std::round(std::sin(angle) * RADIUS);

        // 헤드와의 거리(시계방향 진행) → 페이드
        double dist = std::fmod(phase - (double)i + (double)N * 2.0, (double)N);
        Uint8 alpha = (Uint8)std::clamp(255.0 - dist * (200.0 / (double)N), 70.0, 255.0);

        drawFillRect(SDL_Rect{ cx + dx - DOT / 2, cy + dy - DOT / 2, DOT, DOT },
                     mappal::uiText(), alpha);
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
    bool   dragging = false;
    bool   dragMoved = false;
    double dragAnchorTileX = 0.0;
    double dragAnchorTileY = 0.0;

    // 줌은 Map 인스턴스 수명을 넘어 유지 (텍스처 캐시 영속과 동일 철학).
    // 센터/Z 는 매 열기마다 플레이어 기준으로 리셋.
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

        // 플레이어 주변 3x3 섹터 PNG 선행 로드 (자동로드가 못 따라잡는 즉시 영역)
        Point2 ps = World::ins()->changeToSectorCoord(PlayerX(), PlayerY());
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
            {
                int sx = ps.x + dx, sy = ps.y + dy;
                if (World::ins()->isEmptySector(sx, sy, PlayerZ()))
                    World::ins()->createSector(sx, sy, PlayerZ());
            }

        // 텍스처 캐시는 영속 — clear() 안 함. 다시 열어도 즉시 풀 디테일.

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

        // ── 매 프레임 작업 예산 초기화 ──
        SectorBiomeTextureCache::ins().resetFrame(mapcfg::FRAME_BUDGET_SECTORS);
        ChunkTextureCache     ::ins().resetFrame(mapcfg::FRAME_BUDGET_CHUNKS);

        // 가시 미로드 섹터 자동 로드 (PNG 만, CityGen 미트리거)
        int sectorsLoadPending = SectorAutoLoader::loadVisible(view);

        // ── 렌더 ──
        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, mappal::background());
        drawBiomeLayer    (view);  // 내부에서 cache.getOrBuild 호출 (budget 안에서 빌드)
        drawChunkOverlay  (view);
        drawLandmarkLabels(view);
        drawPlayerMarker  (view);

        drawCoordPanel();
        drawZoomPanel(view, computeZoomButtons());
        drawTabButton();

        // ── 진행 표시 — 이 프레임에 budget 부족으로 미룬 작업이 있으면 ──
        LoadingStats stats{
            sectorsLoadPending,
            SectorBiomeTextureCache::ins().pendingThisFrame(),
            ChunkTextureCache     ::ins().pendingThisFrame()
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
            view.zoomAround(view.viewW / 2, view.viewH / 2,
                mapcfg::WHEEL_ZOOM_FACTOR * mapcfg::WHEEL_ZOOM_FACTOR);
            return;
        }
        if (checkCursor(&zb.zoomOut))
        {
            view.zoomAround(view.viewW / 2, view.viewH / 2,
                1.0 / (mapcfg::WHEEL_ZOOM_FACTOR * mapcfg::WHEEL_ZOOM_FACTOR));
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
            view.zoomAround((int)getMouseX(), (int)getMouseY(), mapcfg::WHEEL_ZOOM_FACTOR);
        else if (event.wheel.y < 0)
            view.zoomAround((int)getMouseX(), (int)getMouseY(), 1.0 / mapcfg::WHEEL_ZOOM_FACTOR);
    }

    void keyDownGUI() override
    {
        if (getStateInput() == false) return;
        if (event.key.key == SDLK_M || event.key.key == SDLK_ESCAPE)
            close(aniFlag::winUnfoldClose);
    }

    void step() override { tabType = tabFlag::back; }
};
