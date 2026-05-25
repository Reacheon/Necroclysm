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
import TileData;
import worldGrid;
import worldGen;
import city;
import CityPlan;
import worldSession;

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
    //   조건1: 24*pxPerTile 가 정수 → 패치 텍스처 (1 patch px = 24 tiles) 가
    //          모든 화면 픽셀에 정수 비율로 매핑 → 띠/뭉개짐 없음.
    //   조건2: pxPerTile 가 ≥1 일 때 정수 → 16px 타일 스프라이트가 모든
    //          타일에 동일한 정수 픽셀 크기로 렌더 → 균일.
    //
    //   24 약수: 1, 2, 3, 4, 6, 8, 12, 24.
    //   조건1 만족: pxPerTile = n/24 꼴. 1/24부터 1/12, 1/8, ..., 1.0, ...
    //
    //   1/24 미만 (광역 조망): 조건1 위배 — 패치 텍스처가 nearest scale로
    //   다운샘플링되어 약간 뭉개지지만, 한 패치가 화면에서 ≤1px 정도라 시각적 무관.
    //   최소값 1/96 ≈ 0.01 px/tile — 대륙 1개 정도 한 화면에 표시.
    //   1/48, 1/16은 24-약수 아님 → 약한 sub-pixel (시각 거의 무관).
    inline constexpr double ZOOM_LEVELS[] = {
        // 광역 조망 (sub-pixel-perfect — 텍스처 다운샘플)
        1.0/96,    // ≈ 0.01042
        1.0/64,    // = 0.015625
        1.0/48,    // ≈ 0.0208  (24× = 0.5, 약한 sub-pixel)
        // 픽셀 퍼펙트 단계 (조건1 만족)
        1.0/24,    // ≈ 0.0417  (24× = 1)
        1.0/16,    // = 0.0625  (24× = 1.5, 약한 sub-pixel)
        1.0/12,    // ≈ 0.0833  (24× = 2)
        1.0/8,     // = 0.125   (24× = 3)
        1.0/6,     // ≈ 0.1667  (24× = 4)
        1.0/4,     // = 0.25    (24× = 6)
        1.0/3,     // ≈ 0.3333  (24× = 8)
        1.0/2,     // = 0.5     (24× = 12)
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0
    };
    inline constexpr int    ZOOM_LEVEL_COUNT   = (int)(sizeof(ZOOM_LEVELS) / sizeof(ZOOM_LEVELS[0]));
    inline constexpr int    DEFAULT_ZOOM_LEVEL = 10;  // 0.5 (위 배열에서 1/2 위치)
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
    // Terrain 색 — 구글맵 톤. mmap 픽셀 (worldGrid::Terrain 16종) 기반.
    //   worldGen_generateWorld.cpp의 terrainPreviewColor와 톤 일관 (월드젠 미리보기와 같은 색).
    inline SDL_Color terrainColor(worldGrid::Terrain t)
    {
        switch (t)
        {
        case worldGrid::Terrain::Land:                  return { 192, 215, 168, 255 };  // grass green
        case worldGrid::Terrain::Sea:                   return {  85, 132, 173, 255 };  // sea blue
        case worldGrid::Terrain::River:                 return { 137, 180, 200, 255 };  // light blue
        case worldGrid::Terrain::Lake:                  return { 111, 106, 184, 255 };  // purple-blue
        case worldGrid::Terrain::CityZone:              return { 162, 162, 162, 255 };  // city gray (#a2a2a2)
        case worldGrid::Terrain::CityCenter:            return { 255,  96,  96, 255 };  // city center red
        case worldGrid::Terrain::CityRiver:             return { 166, 193, 234, 255 };  // city river
        case worldGrid::Terrain::CitySea:               return { 115, 112, 184, 255 };  // city sea (strait)
        case worldGrid::Terrain::Mountain:              return { 138, 106,  82, 255 };  // brown
        case worldGrid::Terrain::Polar:                 return { 242, 246, 255, 255 };  // ice white
        case worldGrid::Terrain::Tundra:                return { 142, 198, 205, 255 };  // pale teal
        case worldGrid::Terrain::Subarctic:             return { 110, 155, 200, 255 };  // cold blue
        case worldGrid::Terrain::Monsoon:               return { 150, 163,  85, 255 };  // olive
        case worldGrid::Terrain::InsularRainforest:     return {  53, 119,  73, 255 };  // SE-Asia green
        case worldGrid::Terrain::Desert:                return { 232, 217, 122, 255 };  // sand
        case worldGrid::Terrain::ContinentalRainforest: return {  31,  74,  26, 255 };  // dense jungle
        }
        return { 18, 18, 22, 255 };
    }

    inline SDL_Color background()   { return {  10,  10,  14, 255 }; }
    inline SDL_Color playerMarker() { return { 220,  80,  80, 255 }; }
    inline SDL_Color roadLine()     { return { 255, 140,  30, 255 }; }  // 광역 도로 폴리라인 오버레이
    inline SDL_Color cityRoadLine() { return { 255, 220,  80, 255 }; }  // 도시 내부 도로 세그먼트 (debug)
    inline SDL_Color bridgeLine()   { return { 120, 220, 255, 255 }; }  // 도시 내부 다리 (16단계 z+1 deck)

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

    // 카메라 Y를 월드 영역 안으로 클램프 — 남/북극 너머로 못 나가게.
    //   월드 가장자리가 화면 가장자리에 닿으면 멈춤 (Google Maps 스타일).
    //   X는 cylindrical wrap이라 클램프 안 함.
    //   뷰포트가 월드보다 큰 경우(극단 줌아웃) 월드를 화면 정중앙 정렬.
    void clampCenterY()
    {
        const double worldMinY = static_cast<double>(TILE_BASE_Y);
        const double worldMaxY = worldMinY + static_cast<double>(WORLD_TILE_H);
        const double viewHalfTiles = viewH * 0.5 / pxPerTile;
        if (viewHalfTiles * 2.0 >= worldMaxY - worldMinY)
        {
            centerTileY = (worldMinY + worldMaxY) * 0.5;
        }
        else
        {
            centerTileY = std::clamp(centerTileY, worldMinY + viewHalfTiles, worldMaxY - viewHalfTiles);
        }
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

// 패치 그리드 셀 1개 = 400×400 픽셀 텍스처 (1 px = 1 mmap pixel).
//   데이터 소스는 mmap (worldGrid::worldPixel) — Phase 1 미진입 시 텍스처 빌드 안 함.
//   캐시는 패치 그리드 좌표(sx, sy, sz)로 인덱싱. 빌드된 텍스처는 영구 보관.
class PixelTextureCache
{
public:
    static PixelTextureCache& ins() { static PixelTextureCache c; return c; }

    void resetFrame(int budget) { budget_ = budget; pending_ = 0; }
    int  pendingThisFrame() const { return pending_; }

    SDL_Texture* getOrBuild(int sx, int sy, int sz)
    {
        Key k{ sx, sy, sz };
        if (auto it = textures_.find(k); it != textures_.end()) return it->second;

        //mmap 미진입(월드젠 전)에는 빌드 불가 — pending 아님, 그냥 nullptr.
        if (!worldGrid::worldPixelMmapActive()) return nullptr;

        if (budget_ <= 0) { pending_++; return nullptr; }

        SDL_Texture* tex = build(sx, sy, sz);
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
    //패치 그리드 셀 (sx, sy)에 해당하는 mmap 픽셀 영역을 400×400 텍스처로 빌드.
    //  글로벌 픽셀 좌표 = sx*400+px (단, mmap은 [0, 43200) × [0, 21600) 범위만 유효).
    //  범위 밖은 worldGrid::worldPixel이 Sea 반환 → 자연스럽게 Sea로 처리됨.
    static SDL_Texture* build(int sx, int sy, int /*sz*/)
    {
        SDL_Surface* surf = SDL_CreateSurface(PIXEL_PER_PATCH, PIXEL_PER_PATCH, SDL_PIXELFORMAT_RGBA32);
        if (!surf) return nullptr;

        SDL_LockSurface(surf);
        const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surf->format);

        //글로벌 픽셀 시작점. patch 좌표는 World의 patch 그리드와 동일 인덱싱.
        const int globalPx0 = (sx - PATCH_X_MIN) * PIXEL_PER_PATCH;   // PATCH_X_MIN(-54)가 픽셀 0
        const int globalPy0 = (sy - PATCH_Y_MIN) * PIXEL_PER_PATCH;   // PATCH_Y_MIN(-27)가 픽셀 0

        for (int py = 0; py < PIXEL_PER_PATCH; py++)
        {
            std::uint32_t* row = (std::uint32_t*)((std::uint8_t*)surf->pixels + py * surf->pitch);
            for (int px = 0; px < PIXEL_PER_PATCH; px++)
            {
                const worldGrid::Terrain t = worldGrid::worldPixel(globalPx0 + px, globalPy0 + py);
                const SDL_Color c = mappal::terrainColor(t);
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
// §4  렌더링 계층
//   (PatchAutoLoader는 mmap 도입 후 폐지 — Phase 1 완료 시점에 모든 픽셀이
//    이미 mmap으로 즉시 접근 가능. 별도 로딩 절차 불필요.)
// ════════════════════════════════════════════════════════════════════════

// (1) 바이옴 베이스 — mmap 활성 시 가시 픽셀 텍스처를 적절히 스케일해 blit.
//     mmap 미진입(타이틀/Phase 1 미완료): 호출 자체가 스킵됨 → 검은 배경 + 타일 스프라이트 + 마커만.
static void drawBiomeLayer(const MapView& v)
{
    if (!worldGrid::worldPixelMmapActive()) return;

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
            SDL_Texture* tex = PixelTextureCache::ins().getOrBuild(sx, sy, v.z);
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

// (3) 도로 폴리라인 오버레이 — buildRoadNetwork 가 생성한 도시간 광역 도로.
//     worldGen::activePolyLines 가 nullptr 이면 (월드젠 전) 스킵.
//
//     wrap 보정: 첫 정점만 카메라 기준 signedDeltaTileX 로 화면 좌표로 끌어오고,
//     이후 정점은 "직전 정점 기준" signedDeltaTileX 로 누적 — 폴리라인 전체가
//     단일 분기에서 일관되게 그려짐. (각 정점을 카메라 기준으로 독립 계산하면
//     인접 정점이 카메라 기준 ±W/2 경계를 사이에 둘 때 한쪽은 동쪽, 다른쪽은
//     서쪽으로 분기되어 화면을 가로지르는 가짜 선이 생긴다.)
//
//     verts.z == view.z 인 폴리라인만 그림 (도로는 표면 z 평면).
static void drawRoadOverlay(const MapView& v)
{
    const auto* polys = worldGen::activePolyLines;
    if (!polys || polys->empty()) return;

    const SDL_Color road = mappal::roadLine();
    const float vw = static_cast<float>(v.viewW);
    const float vh = static_cast<float>(v.viewH);
    constexpr float marginPx = 8.0f;

    const int camX = static_cast<int>(std::floor(v.centerTileX));

    auto tileYToScreen = [&](int py) -> double
    {
        return (static_cast<double>(py) - v.centerTileY) * v.pxPerTile + v.viewH * 0.5;
    };

    for (const auto& poly : *polys)
    {
        if (poly.verts.size() < 2) continue;
        if (poly.verts[0].z != v.z) continue;

        //첫 정점 — 카메라 기준 최단 wrap 분기.
        double prevSx = static_cast<double>(worldWrap::signedDeltaTileX(camX, poly.verts[0].x))
                      * v.pxPerTile + v.viewW * 0.5;
        double prevSy = tileYToScreen(poly.verts[0].y);

        for (std::size_t i = 1; i < poly.verts.size(); ++i)
        {
            //직전 정점 기준 누적 — seam 가로지름 방지.
            const int dxSeg = worldWrap::signedDeltaTileX(poly.verts[i - 1].x, poly.verts[i].x);
            const double curSx = prevSx + static_cast<double>(dxSeg) * v.pxPerTile;
            const double curSy = tileYToScreen(poly.verts[i].y);

            //AABB 컬링 — 양 끝 모두 화면 밖이면 스킵.
            const float minX = (float)std::min(prevSx, curSx);
            const float maxX = (float)std::max(prevSx, curSx);
            const float minY = (float)std::min(prevSy, curSy);
            const float maxY = (float)std::max(prevSy, curSy);
            if (maxX >= -marginPx && minX <= vw + marginPx &&
                maxY >= -marginPx && minY <= vh + marginPx)
            {
                drawLine(
                    static_cast<int>(std::round(prevSx)),
                    static_cast<int>(std::round(prevSy)),
                    static_cast<int>(std::round(curSx)),
                    static_cast<int>(std::round(curSy)),
                    road);
            }

            prevSx = curSx;
            prevSy = curSy;
        }
    }
}

// (3.4) 도시 건물 분포 오버레이 (debug) — buildCityPlan 4단계의 건물 픽셀.
//       memberIndex 해시 색상으로 칠해서 같은 건물의 픽셀들이 한 덩어리로 보이게.
//       1픽셀당 24x24 타일 정사각형. 도로 오버레이보다 먼저 그려서 도로가 위에 얹힘.
static void drawCityBuildingOverlay(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities || cities->empty()) return;

    const float vw = static_cast<float>(v.viewW);
    const float vh = static_cast<float>(v.viewH);
    const int camX = static_cast<int>(std::floor(v.centerTileX));

    auto tileYToScreen = [&](int py) -> double
    {
        return (static_cast<double>(py) - v.centerTileY) * v.pxPerTile + v.viewH * 0.5;
    };

    //memberIndex 해시 → 색상. golden ratio 곱셈으로 인접 index끼리 대비.
    //비트 마스크로 어두운 색 회피 + 알파로 반투명 → 밑의 지형 보이게.
    auto colorForIndex = [](int idx) -> SDL_Color
    {
        if (idx < 0) return SDL_Color{ 128, 128, 128, 140 };
        const std::uint32_t h = static_cast<std::uint32_t>(idx) * 2654435761u;
        return SDL_Color{
            static_cast<Uint8>(((h >>  0) & 0x9F) | 0x60),
            static_cast<Uint8>(((h >>  8) & 0x9F) | 0x60),
            static_cast<Uint8>(((h >> 16) & 0x9F) | 0x60),
            150
        };
    };

    constexpr int CITY_VIS_MARGIN_TILES = 4000;

    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const auto cityId = static_cast<city::CityId>(i);
        const auto& cn = (*cities)[i];

        const int dxFromCam = worldWrap::signedDeltaTileX(camX, cn.center.x);
        const double sxd = static_cast<double>(dxFromCam) * v.pxPerTile + v.viewW * 0.5;
        const double syd = (static_cast<double>(cn.center.y) - v.centerTileY) * v.pxPerTile + v.viewH * 0.5;
        const double marginPxScreen = CITY_VIS_MARGIN_TILES * v.pxPerTile;
        const bool inView = (sxd + marginPxScreen >= 0) && (sxd - marginPxScreen <= v.viewW)
                         && (syd + marginPxScreen >= 0) && (syd - marginPxScreen <= v.viewH);
        if (!inView) continue;

        const CityPlan* plan = CityPlanCache::ins().peek(cityId);
        if (!plan) continue;

        const double rectW = static_cast<double>(TILE_PER_PIXEL) * v.pxPerTile;
        const double rectH = rectW;

        for (const auto& bp : plan->buildings)
        {
            if (bp.pos.z != v.z) continue;

            const double sxA = static_cast<double>(worldWrap::signedDeltaTileX(camX, bp.pos.x))
                             * v.pxPerTile + v.viewW * 0.5;
            const double syA = tileYToScreen(bp.pos.y);

            if (sxA + rectW < 0 || sxA > vw || syA + rectH < 0 || syA > vh) continue;

            const SDL_Rect rect{
                static_cast<int>(std::round(sxA)),
                static_cast<int>(std::round(syA)),
                static_cast<int>(std::round(rectW)),
                static_cast<int>(std::round(rectH))
            };
            drawFillRect(rect, colorForIndex(bp.memberIndex));
        }
    }
}

// (3.5) 도시 내부 도로 오버레이 (debug) — buildCityPlan이 생성한 살아남은 segments.
//       이미 캐시된 도시(CityPlanCache::peek 성공)만 그림. 미캐시 도시는 스킵 —
//       대도시 일괄 계산은 비용 크니까 플레이어가 근처로 갈 때 자동 캐시되는 패턴 유지.
//
//       각 세그먼트는 2점 라인. 광역 도로처럼 누적 wrap 불필요 — 한 도시 안의
//       세그먼트라 길이가 짧고 seam을 가로지를 일 거의 없음. 단순히 양 끝점을
//       카메라 기준으로 각각 wrap-clamp 해서 그림.
static void drawCityRoadOverlay(const MapView& v)
{
    const auto* cities = worldGen::activeCities;
    if (!cities || cities->empty()) return;

    const SDL_Color color = mappal::cityRoadLine();
    const float vw = static_cast<float>(v.viewW);
    const float vh = static_cast<float>(v.viewH);
    constexpr float marginPx = 8.0f;

    const int camX = static_cast<int>(std::floor(v.centerTileX));

    auto tileYToScreen = [&](int py) -> double
    {
        return (static_cast<double>(py) - v.centerTileY) * v.pxPerTile + v.viewH * 0.5;
    };

    //── 디버그 카운터 (60프레임당 1회 콘솔 출력) ──
    static int dbgFrameCount = 0;
    const bool dbgPrint = (++dbgFrameCount % 60 == 0);
    int dbgCached = 0;
    int dbgTotalSegs = 0;
    int dbgDrawn = 0;
    int dbgWrongZ = 0;
    int dbgClipped = 0;

    constexpr int CITY_VIS_MARGIN_TILES = 4000;  // 가장 큰 도시 베이징(~2880타일) 커버

    for (std::size_t i = 0; i < cities->size(); ++i)
    {
        const auto cityId = static_cast<city::CityId>(i);
        const auto& cn = (*cities)[i];

        //가시 검사 — city.center가 view 영역 (+margin) 안인지
        const int dxFromCam = worldWrap::signedDeltaTileX(camX, cn.center.x);
        const double sxd = static_cast<double>(dxFromCam) * v.pxPerTile + v.viewW * 0.5;
        const double syd = (static_cast<double>(cn.center.y) - v.centerTileY) * v.pxPerTile + v.viewH * 0.5;
        const double marginPxScreen = CITY_VIS_MARGIN_TILES * v.pxPerTile;
        const bool inView = (sxd + marginPxScreen >= 0) && (sxd - marginPxScreen <= v.viewW)
                         && (syd + marginPxScreen >= 0) && (syd - marginPxScreen <= v.viewH);
        if (!inView) continue;

        //이미 캐시된 도시만 그림 — 미캐시 도시는 스킵.
        //  CityPlan 빌드는 무거우니 카메라 이동으로 강제 발동시키지 않음.
        //  플레이어가 청크 생성 범위에 들어가면 자연스럽게 캐시됨.
        const CityPlan* plan = CityPlanCache::ins().peek(cityId);
        if (!plan) continue;
        ++dbgCached;
        dbgTotalSegs += static_cast<int>(plan->segments.size());

        for (const auto& seg : plan->segments)
        {
            if (seg.verts.size() < 2) continue;
            if (seg.verts[0].z != v.z) { ++dbgWrongZ; continue; }

            //양 끝점을 카메라 기준 최단 wrap 분기로 화면 좌표 산출.
            //  세그먼트가 짧아서 양 끝이 모두 같은 wrap 분기로 떨어짐 → 누적 보정 불필요.
            const double sxA = static_cast<double>(worldWrap::signedDeltaTileX(camX, seg.verts[0].x))
                             * v.pxPerTile + v.viewW * 0.5;
            const double syA = tileYToScreen(seg.verts[0].y);

            const int dxSeg = worldWrap::signedDeltaTileX(seg.verts[0].x, seg.verts[1].x);
            const double sxB = sxA + static_cast<double>(dxSeg) * v.pxPerTile;
            const double syB = tileYToScreen(seg.verts[1].y);

            //AABB 컬링
            const float minX = (float)std::min(sxA, sxB);
            const float maxX = (float)std::max(sxA, sxB);
            const float minY = (float)std::min(syA, syB);
            const float maxY = (float)std::max(syA, syB);
            if (maxX >= -marginPx && minX <= vw + marginPx &&
                maxY >= -marginPx && minY <= vh + marginPx)
            {
                drawLine(
                    static_cast<int>(std::round(sxA)),
                    static_cast<int>(std::round(syA)),
                    static_cast<int>(std::round(sxB)),
                    static_cast<int>(std::round(syB)),
                    color);
                ++dbgDrawn;
            }
            else
            {
                ++dbgClipped;
            }
        }

        //── 다리 (segments와 분리 채널, z+1 deck — segments와 같은 view.z에서 그림) ──
        const SDL_Color bridgeColor = mappal::bridgeLine();
        for (const auto& br : plan->bridges)
        {
            if (br.verts.size() < 2) continue;
            if (br.verts[0].z != v.z) { ++dbgWrongZ; continue; }

            const double sxA = static_cast<double>(worldWrap::signedDeltaTileX(camX, br.verts[0].x))
                             * v.pxPerTile + v.viewW * 0.5;
            const double syA = tileYToScreen(br.verts[0].y);
            const int dxSeg = worldWrap::signedDeltaTileX(br.verts[0].x, br.verts[1].x);
            const double sxB = sxA + static_cast<double>(dxSeg) * v.pxPerTile;
            const double syB = tileYToScreen(br.verts[1].y);

            const float minX = (float)std::min(sxA, sxB);
            const float maxX = (float)std::max(sxA, sxB);
            const float minY = (float)std::min(syA, syB);
            const float maxY = (float)std::max(syA, syB);
            if (maxX >= -marginPx && minX <= vw + marginPx &&
                maxY >= -marginPx && minY <= vh + marginPx)
            {
                drawLine(
                    static_cast<int>(std::round(sxA)),
                    static_cast<int>(std::round(syA)),
                    static_cast<int>(std::round(sxB)),
                    static_cast<int>(std::round(syB)),
                    bridgeColor);
                ++dbgDrawn;
            }
            else
            {
                ++dbgClipped;
            }
        }
    }

    if (dbgPrint)
    {
        prt(L"[CityRoadOverlay] visCities=%d  segs(total=%d drawn=%d wrongZ=%d clipped=%d)  view.z=%d  cache.total=%zu\n",
            dbgCached, dbgTotalSegs, dbgDrawn, dbgWrongZ, dbgClipped, v.z, CityPlanCache::ins().size());
    }
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
    int patchesLoading    = 0;  // (deprecated, 항상 0 — Patch 자동로드 시스템 폐지)
    int patchesBuilding   = 0;  // 픽셀 텍스처 빌드 대기 (frame budget 초과)
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

        // mmap 기반 — 패치 사전 로드 불필요 (Phase 1 완료면 모든 픽셀 즉시 접근).
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
        view.clampCenterY();   // 드래그/줌/리사이즈로 카메라가 남/북극 너머로 갔으면 되돌림

        // 매 프레임 작업 예산 초기화 (텍스처 빌드 한도)
        PixelTextureCache::ins().resetFrame(mapcfg::FRAME_BUDGET_PATCHES);

        // 렌더
        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, mappal::background());
        drawBiomeLayer       (view);  // mmap 활성 시에만 — 내부에서 cache.getOrBuild → budget 안에서 빌드
        drawTileSpriteLayer  (view);  // 항상 — 로드된 청크의 실제 타일 스프라이트
        drawRoadOverlay      (view);  // 도시간 광역 도로 폴리라인 (worldGen 결과)
        drawCityBuildingOverlay(view);  // 도시 내부 건물 분포 디버그 (memberIndex 해시 색상)
        drawCityRoadOverlay  (view);  // 도시 내부 도로 세그먼트 (CityPlan 캐시된 도시만)
        drawPlayerMarker     (view);

        drawCoordPanel();
        drawZoomPanel(view, computeZoomButtons());
        drawTabButton();

        // 진행 표시 — 이번 프레임 budget 부족으로 미룬 텍스처 빌드가 있으면
        LoadingStats stats{
            0,
            PixelTextureCache::ins().pendingThisFrame()
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
