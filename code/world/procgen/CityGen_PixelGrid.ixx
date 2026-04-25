module;
#include <SDL3_image/SDL_image.h>

export module CityGen.PixelGrid;

import std;
import util;
import constVar;
import SectorBiome;

// ──────────────────────────────────────────────────────────
// CityGen.PixelGrid — 월드 PNG의 픽셀을 도시 생성용으로 분류·캐시
//   섹터 PNG 라인업과 색상 테이블에 종속. 1픽셀 = TILE_PER_PIXEL 타일.
// ──────────────────────────────────────────────────────────

export enum class PixelType : std::uint8_t
{
    unknown, seawater, land, city, river, bridge, portal,
};

export inline PixelType classifyPixel(SDL_Color c)
{
    auto eq = [](SDL_Color a, SDL_Color b) { return a.r == b.r && a.g == b.g && a.b == b.b; };
    constexpr SDL_Color bridgeCol{ 0x77, 0x77, 0x77, 0xFF };
    constexpr SDL_Color portalCol{ 0xFF, 0x00, 0x00, 0xFF };
    if (eq(c, chunkCol::seawater)) return PixelType::seawater;
    if (eq(c, chunkCol::land))     return PixelType::land;
    if (eq(c, chunkCol::city))     return PixelType::city;
    if (eq(c, chunkCol::river))    return PixelType::river;
    if (eq(c, bridgeCol))          return PixelType::bridge;
    if (eq(c, portalCol))          return PixelType::portal;
    return PixelType::unknown;
}

export inline bool isCityTerritory(PixelType t)
{
    return t == PixelType::city || t == PixelType::bridge || t == PixelType::portal;
}

// 전역 픽셀 좌표 기반 캐시 — 도시가 섹터 경계를 넘어 퍼져 있을 수 있어
// 필요한 섹터를 늦게 로드해 저장한다.
export class PixelGrid
{
public:
    std::unordered_map<Point2, PixelType, Point2::Hash> pixels;
    std::unordered_set<Point2, Point2::Hash> loadedSectors;

    bool loadSector(int sx, int sy, int sz)
    {
        if (loadedSectors.contains({ sx, sy })) return true;
        if (sy > 26 || sy < -27 || sx > 53 || sx < -54) { loadedSectors.insert({ sx, sy }); return false; }

        std::string filePath = "map/worldSector-";
        int number = 2971 + sx + 108 * sy;
        if (number < 100) filePath += "0";
        filePath += std::to_string(number);
        filePath += ".png";

        SDL_Surface* s = IMG_Load(filePath.c_str());
        if (!s) { loadedSectors.insert({ sx, sy }); return false; }

        Uint32* px = (Uint32*)s->pixels;
        int baseX = sx * PIXEL_PER_SECTOR;
        int baseY = sy * PIXEL_PER_SECTOR;

        for (int y = 0; y < PIXEL_PER_SECTOR; y++)
        {
            for (int x = 0; x < PIXEL_PER_SECTOR; x++)
            {
                Uint32 p = px[y * s->w + x];
                SDL_Color c;
                SDL_GetRGB(p, SDL_GetPixelFormatDetails(s->format), SDL_GetSurfacePalette(s), &c.r, &c.g, &c.b);
                PixelType t = classifyPixel(c);
                if (t != PixelType::unknown)
                    pixels[{ baseX + x, baseY + y }] = t;
            }
        }
        SDL_DestroySurface(s);
        loadedSectors.insert({ sx, sy });
        return true;
    }

    PixelType at(int gx, int gy) const
    {
        auto it = pixels.find({ gx, gy });
        return (it == pixels.end()) ? PixelType::unknown : it->second;
    }
};

// 4-연결 flood로 도시 컴포넌트 추출.
//   섹터 경계 너머로 도시가 이어지면 미로드 섹터를 즉시 로드한다.
export std::vector<Point2> floodCity(PixelGrid& grid, Point2 start, int sz,
    std::unordered_set<Point2, Point2::Hash>& visited)
{
    std::vector<Point2> out;
    std::queue<Point2> q;
    q.push(start); visited.insert(start);
    constexpr int dx[] = { 1,-1,0,0 }, dy[] = { 0,0,1,-1 };
    while (!q.empty())
    {
        Point2 cur = q.front(); q.pop();
        out.push_back(cur);
        for (int k = 0; k < 4; k++)
        {
            Point2 n{ cur.x + dx[k], cur.y + dy[k] };
            if (visited.contains(n)) continue;
            grid.loadSector(sectorFromPixel(n.x), sectorFromPixel(n.y), sz);
            PixelType t = grid.at(n.x, n.y);
            if (isCityTerritory(t)) { visited.insert(n); q.push(n); }
        }
    }
    return out;
}
