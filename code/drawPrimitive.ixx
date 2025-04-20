#include <SDL.h>

export module drawPrimitive;

import std;

static SDL_Renderer* localRenderer;
export void setPrimitiveRenderer(SDL_Renderer* inputRenderer) { localRenderer = inputRenderer; }


export void drawPoint(int x1, int y1, const SDL_Color& col, Uint8 alpha)
{
	SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
	SDL_RenderDrawPoint(localRenderer, x1, y1);
};
export void drawPoint(int x1, int y1, const SDL_Color& col) { drawPoint(x1, y1, col, 255);}

/////////////////////////////////////////////////////////////////////////////////////////////
export void drawLine(int x1, int y1, int x2, int y2, SDL_Color col, Uint8 alpha)
{
    SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
    SDL_RenderDrawLine(localRenderer, x1, y1, x2, y2);
};
export void drawLine(int x1, int y1, int x2, int y2, SDL_Color col){ drawLine(x1, y1, x2, y2, col, 255); };
export void drawLine(int x1, int y1, int x2, int y2) { drawLine(x1, y1, x2, y2, { 255,255,255 }, 255); }

//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

export void drawFillRect(int x, int y, int w, int h, const SDL_Color& col, Uint8 alpha)
{
    SDL_Rect targetRect = { x,y,w,h };
    SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
    SDL_RenderFillRect(localRenderer, &targetRect);
};
export void drawFillRect(const SDL_Rect& rect, const SDL_Color& col, Uint8 alpha) { drawFillRect(rect.x, rect.y, rect.w, rect.h, col, alpha); };
export void drawFillRect(int x, int y, int w, int h, const SDL_Color& col) { drawFillRect(x, y, w, h, col, 255); };
export void drawFillRect(const SDL_Rect& rect, const SDL_Color& col) { drawFillRect(rect, col, 255); };

/////////////////////////////////////////////////////////////////////////////////////////////

export void drawRect(int x, int y, int w, int h, const SDL_Color& col, Uint8 alpha)
{
    SDL_Rect targetRect = { x,y,w,h };
    SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
    SDL_RenderDrawRect(localRenderer, &targetRect);
};
export void drawRect(const SDL_Rect& rect, const SDL_Color& col, Uint8 alpha) { drawRect(rect.x, rect.y, rect.w, rect.h, col, alpha); };
export void drawRect(int x, int y, int w, int h, const SDL_Color& col) { drawRect(x, y, w, h, col, 255); };
export void drawRect(const SDL_Rect& rect, const SDL_Color& col) { drawRect(rect, col, 255); };

//////////////////////////////////////////////////////////////////////////////////////////////

//두께 1 십자가
export void drawCross(int x, int y, int north, int south, int west, int east)
{
    SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawLine(localRenderer, x, y, x, y - north);
    SDL_RenderDrawLine(localRenderer, x, y, x, y + south);
    SDL_RenderDrawLine(localRenderer, x, y, x - west, y);
    SDL_RenderDrawLine(localRenderer, x, y, x + east, y);
}

//두께 2 십자가
export void drawCross2(int x, int y, int north, int south, int west, int east)
{
    SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawLine(localRenderer, x, y, x, y - north);
    SDL_RenderDrawLine(localRenderer, x+1, y, x+1, y - north);

    SDL_RenderDrawLine(localRenderer, x, y, x, y + south + 1);
    SDL_RenderDrawLine(localRenderer, x+1, y, x+1, y + south + 1);

    SDL_RenderDrawLine(localRenderer, x, y, x - west, y);
    SDL_RenderDrawLine(localRenderer, x, y+1, x - west, y+1);

    SDL_RenderDrawLine(localRenderer, x, y, x + east + 1, y);
    SDL_RenderDrawLine(localRenderer, x, y+1, x + east + 1, y+1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//5는 일반적인 둥근 테두리, 6은 하단이 평평한 5번, 7은 상단이 평평한 5번
export void drawStadium(int x, int y, int w, int h, SDL_Color color, int alpha, int edge)
{
	int xEnd = x + w - 1;
	int yEnd = y + h - 1;
	switch (edge)
	{
	default:
	{
		break;
	}
	case 1:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y + 1;
		rect.w = w;
		rect.h = h - 2;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 1, y, xEnd - 1, y);

		SDL_RenderDrawLine(localRenderer, x + 1, yEnd, xEnd - 1, yEnd);
		break;
	}
	case 2:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y + 2;
		rect.w = w;
		rect.h = h - 4;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 2, y, xEnd - 2, y);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 1, xEnd - 1, y + 1);

		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 1, xEnd - 1, yEnd - 1);
		SDL_RenderDrawLine(localRenderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
		break;
	}
	case 3:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y + 3;
		rect.w = w;
		rect.h = h - 6;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 3, y, xEnd - 3, y);
		SDL_RenderDrawLine(localRenderer, x + 2, y + 1, xEnd - 2, y + 1);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 2, xEnd - 1, y + 2);

		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 2, xEnd - 1, yEnd - 2);
		SDL_RenderDrawLine(localRenderer, x + 2, yEnd - 1, xEnd - 2, yEnd - 1);
		SDL_RenderDrawLine(localRenderer, x + 3, yEnd, xEnd - 3, yEnd);
		break;
	}
	case 4:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y + 4;
		rect.w = w;
		rect.h = h - 8;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 4, y, xEnd - 4, y);
		SDL_RenderDrawLine(localRenderer, x + 2, y + 1, xEnd - 2, y + 1);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 2, xEnd - 1, y + 2);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 3, xEnd - 1, y + 3);

		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 2, xEnd - 1, yEnd - 2);
		SDL_RenderDrawLine(localRenderer, x + 2, yEnd - 1, xEnd - 2, yEnd - 1);
		SDL_RenderDrawLine(localRenderer, x + 4, yEnd, xEnd - 4, yEnd);
		break;
	}
	case 5:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y + 5;
		rect.w = w;
		rect.h = h - 10;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 5, y, xEnd - 5, y);
		SDL_RenderDrawLine(localRenderer, x + 3, y + 1, xEnd - 3, y + 1);
		SDL_RenderDrawLine(localRenderer, x + 2, y + 2, xEnd - 2, y + 2);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 3, xEnd - 1, y + 3);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 4, xEnd - 1, y + 4);

		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 4, xEnd - 1, yEnd - 4);
		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
		SDL_RenderDrawLine(localRenderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
		SDL_RenderDrawLine(localRenderer, x + 3, yEnd - 1, xEnd - 3, yEnd - 1);
		SDL_RenderDrawLine(localRenderer, x + 5, yEnd, xEnd - 5, yEnd);

		break;
	}
	case 6://하단 직선
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y + 5;
		rect.w = w;
		rect.h = h - 5;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 5, y, xEnd - 5, y);
		SDL_RenderDrawLine(localRenderer, x + 3, y + 1, xEnd - 3, y + 1);
		SDL_RenderDrawLine(localRenderer, x + 2, y + 2, xEnd - 2, y + 2);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 3, xEnd - 1, y + 3);
		SDL_RenderDrawLine(localRenderer, x + 1, y + 4, xEnd - 1, y + 4);
		break;
	}
	case 7://상단 직선
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h - 5;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 4, xEnd - 1, yEnd - 4);
		SDL_RenderDrawLine(localRenderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
		SDL_RenderDrawLine(localRenderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
		SDL_RenderDrawLine(localRenderer, x + 3, yEnd - 1, xEnd - 3, yEnd - 1);
		SDL_RenderDrawLine(localRenderer, x + 5, yEnd, xEnd - 5, yEnd);

		break;
	}
	}
}