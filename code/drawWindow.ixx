#include <SDL3/SDL.h>

export module drawWindow;

import std;
import util;
import globalVar;
import constVar;
import drawText;
import drawSprite;
import textureVar;

static unsigned __int8 windowAlpha = 180;

export void setWindowAlpha(int val) { windowAlpha = val; }
export void resetWindowAlpha() { windowAlpha = 180; }

static inline SDL_FRect makeFRect(int x, int y, int w, int h)
{
    return { static_cast<float>(x),
             static_cast<float>(y),
             static_cast<float>(w),
             static_cast<float>(h) };
}

static inline void renderFillRect(int x, int y, int w, int h)
{
    SDL_FRect r = makeFRect(x, y, w, h);
    SDL_RenderFillRect(renderer, &r);
}

static inline void renderRect(int x, int y, int w, int h)
{
    SDL_FRect r = makeFRect(x, y, w, h);
    SDL_RenderRect(renderer, &r);
}

//─────────────────────────────────────────────────────────────
//  기본 창
//─────────────────────────────────────────────────────────────
export void drawWindow(int x, int y, int w, int h)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
    renderFillRect(x, y, w, h);

    SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
    renderRect(x, y, w, h);
}

export void drawWindow(const SDL_Rect* r)
{
    drawWindow(r->x, r->y, r->w, r->h);
}

//─────────────────────────────────────────────────────────────
//  제목이 있는 창 (아이콘 없음/아이템셋 아님)
//─────────────────────────────────────────────────────────────
export void drawWindow(int x, int y, int w, int h, std::wstring title, int titleSprIndex)
{
    drawWindow(x, y, w, h);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
    renderFillRect(x, y, w, 30);

    SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
    renderRect(x, y, w, 30);

    setFontSize(16);
    renderTextCenter(title, x + w / 2, y + 16);

    setZoom(1.5f);
    drawSpriteCenter(spr::icon16, titleSprIndex,
        x + w / 2 - queryTextWidth(title, true) / 2 - 20,
        y + 14);
    setZoom(1.0f);
}

export void drawWindow(const SDL_Rect* r, std::wstring title, int sprIndex)
{
    drawWindow(r->x, r->y, r->w, r->h, std::move(title), sprIndex);
}

//─────────────────────────────────────────────────────────────
//  아이템셋 창
//─────────────────────────────────────────────────────────────
export void drawWindowItemset(int x, int y, int w, int h,
    std::wstring title, int titleSprIndex)
{
    drawWindow(x, y, w, h);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
    renderFillRect(x, y, w, 30);

    SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
    renderRect(x, y, w, 30);

    setFontSize(14);
    renderTextCenter(title, x + w / 2, y + 14);

    setZoom(1.5f);
    drawSpriteCenter(spr::itemset, titleSprIndex,
        x + w / 2 - queryTextWidth(title, true) / 2 - 20,
        y + 14);
    setZoom(1.0f);
}

export void drawWindowItemset(const SDL_Rect* r, std::wstring title, int sprIndex)
{
    drawWindowItemset(r->x, r->y, r->w, r->h, std::move(title), sprIndex);
}

//─────────────────────────────────────────────────────────────
//  화살표가 달린 모서리 창
//─────────────────────────────────────────────────────────────
static inline void renderLine(int x1, int y1, int x2, int y2)
{
    SDL_RenderLine(renderer,
        static_cast<float>(x1), static_cast<float>(y1),
        static_cast<float>(x2), static_cast<float>(y2));
}

export void drawEdgeWindow(int x, int y, int w, int h,
    int edgeWidth,
    dir16 arrowDir)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
    renderFillRect(x, y, w, h);

    int l = x, t = y;
    int r = x + w - 1, b = y + h - 1;
    int cx = x + w / 2, cy = y + h / 2;

    SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);

    // 상단
    if (arrowDir != dir16::dir2) renderLine(l, t, r, t);
    else {
        renderLine(l, t, cx - 10, t);
        renderLine(r, t, cx + 10, t);
        drawSprite(spr::windowArrow2, 1, cx - 11, t - 10);
    }

    // 좌측
    if (arrowDir != dir16::dir4) renderLine(l, t, l, b);
    else {
        renderLine(l, t, l, cy - 10);
        renderLine(l, b, l, cy + 10);
        drawSprite(spr::windowArrow2, 2, l - 11, cy - 11);
    }

    // 우측
    if (arrowDir != dir16::dir0) renderLine(r, b, r, t);
    else {
        renderLine(r, t, r, cy - 10);
        renderLine(r, b, r, cy + 10);
        drawSprite(spr::windowArrow2, 0, r + 1, cy - 11);
    }

    // 하단
    if (arrowDir != dir16::dir6) renderLine(r, b, l, b);
    else {
        renderLine(l, b, cx - 10, b);
        renderLine(r, b, cx + 10, b);
        drawSprite(spr::windowArrow2, 3, cx - 11, b + 1);
    }

    SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 255);

    // 네 귀퉁이 밝은 강조선
    auto light = [&](int sx, int sy, int dx, int dy) {
        renderLine(sx, sy, dx, dy);
        };

    light(l, t, l + edgeWidth - 1, t);
    light(l, t + 1, l + edgeWidth - 1, t + 1);
    light(l, t, l, t + edgeWidth - 1);
    light(l + 1, t, l + 1, t + edgeWidth - 1);

    light(r, t, r - edgeWidth + 1, t);
    light(r, t + 1, r - edgeWidth + 1, t + 1);
    light(r, t, r, t + edgeWidth - 1);
    light(r - 1, t, r - 1, t + edgeWidth - 1);

    light(l, b, l + edgeWidth - 1, b);
    light(l, b - 1, l + edgeWidth - 1, b - 1);
    light(l, b, l, b - edgeWidth + 1);
    light(l + 1, b, l + 1, b - edgeWidth + 1);

    light(r, b, r - edgeWidth + 1, b);
    light(r, b - 1, r - edgeWidth + 1, b - 1);
    light(r, b, r, b - edgeWidth + 1);
    light(r - 1, b, r - 1, b - edgeWidth + 1);
}

export void drawEdgeWindow(int x, int y, int w, int h,
    int edgeWidth)           // ← 5개
{
    drawEdgeWindow(x, y, w, h, edgeWidth, dir16::dir0_5);
}