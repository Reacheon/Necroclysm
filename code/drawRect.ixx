#include <SDL.h>

export module drawRect;

import std;
import globalVar;



/////////////////////////////////////////////////////////////////////////////////////////////

export void drawFillRect(int x, int y, int w, int h, SDL_Color col, Uint8 alpha)
{
    SDL_Rect targetRect = { x,y,w,h };
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
    SDL_RenderFillRect(renderer, &targetRect);
};

export void drawFillRect(SDL_Rect rect, SDL_Color col, Uint8 alpha)
{
    SDL_Rect targetRect = rect;
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
    SDL_RenderFillRect(renderer, &targetRect);
};

export void drawFillRect(int x, int y, int w, int h, SDL_Color col)
{
    drawFillRect(x,y,w,h,col,255);
};

export void drawFillRect(SDL_Rect rect, SDL_Color col)
{
    drawFillRect(rect, col, 255);
};

/////////////////////////////////////////////////////////////////////////////////////////////

export void drawRect(int x, int y, int w, int h, SDL_Color col, Uint8 alpha)
{
    SDL_Rect targetRect = { x,y,w,h };
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
    SDL_RenderDrawRect(renderer, &targetRect);
};

export void drawRect(SDL_Rect rect, SDL_Color col, Uint8 alpha)
{
    SDL_Rect targetRect = rect;
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
    SDL_RenderDrawRect(renderer, &targetRect);
};

export void drawRect(int x, int y, int w, int h, SDL_Color col)
{
    drawRect(x, y, w, h, col, 255);
};

export void drawRect(SDL_Rect rect, SDL_Color col)
{
    drawRect(rect, col, 255);
};