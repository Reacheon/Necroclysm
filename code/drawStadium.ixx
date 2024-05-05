#include <SDL.h>

export module drawStadium;

import globalVar;


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
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y + 1;
			rect.w = w;
			rect.h = h - 2;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 1, y, xEnd - 1, y);

			SDL_RenderDrawLine(renderer, x + 1, yEnd, xEnd - 1, yEnd);
			break;
		}
		case 2:
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y + 2;
			rect.w = w;
			rect.h = h - 4;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 2, y, xEnd - 2, y);
			SDL_RenderDrawLine(renderer, x + 1, y + 1, xEnd - 1, y + 1);

			SDL_RenderDrawLine(renderer, x + 1, yEnd - 1, xEnd - 1, yEnd - 1);
			SDL_RenderDrawLine(renderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
			break;
		}
		case 3:
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y + 3;
			rect.w = w;
			rect.h = h - 6;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 3, y, xEnd - 3, y);
			SDL_RenderDrawLine(renderer, x + 2, y + 1, xEnd - 2, y + 1);
			SDL_RenderDrawLine(renderer, x + 1, y + 2, xEnd - 1, y + 2);

			SDL_RenderDrawLine(renderer, x + 1, yEnd - 2, xEnd - 1, yEnd - 2);
			SDL_RenderDrawLine(renderer, x + 2, yEnd - 1, xEnd - 2, yEnd - 1);
			SDL_RenderDrawLine(renderer, x + 3, yEnd, xEnd - 3, yEnd);
			break;
		}
		case 4:
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y + 4;
			rect.w = w;
			rect.h = h - 8;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 4, y, xEnd - 4, y);
			SDL_RenderDrawLine(renderer, x + 2, y + 1, xEnd - 2, y + 1);
			SDL_RenderDrawLine(renderer, x + 1, y + 2, xEnd - 1, y + 2);
			SDL_RenderDrawLine(renderer, x + 1, y + 3, xEnd - 1, y + 3);

			SDL_RenderDrawLine(renderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
			SDL_RenderDrawLine(renderer, x + 1, yEnd - 2, xEnd - 1, yEnd - 2);
			SDL_RenderDrawLine(renderer, x + 2, yEnd - 1, xEnd - 2, yEnd - 1);
			SDL_RenderDrawLine(renderer, x + 4, yEnd, xEnd - 4, yEnd);
			break;
		}
		case 5:
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y + 5;
			rect.w = w;
			rect.h = h - 10;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 5, y, xEnd - 5, y);
			SDL_RenderDrawLine(renderer, x + 3, y + 1, xEnd - 3, y + 1);
			SDL_RenderDrawLine(renderer, x + 2, y + 2, xEnd - 2, y + 2);
			SDL_RenderDrawLine(renderer, x + 1, y + 3, xEnd - 1, y + 3);
			SDL_RenderDrawLine(renderer, x + 1, y + 4, xEnd - 1, y + 4);

			SDL_RenderDrawLine(renderer, x + 1, yEnd - 4, xEnd - 1, yEnd - 4);
			SDL_RenderDrawLine(renderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
			SDL_RenderDrawLine(renderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
			SDL_RenderDrawLine(renderer, x + 3, yEnd - 1, xEnd - 3, yEnd - 1);
			SDL_RenderDrawLine(renderer, x + 5, yEnd, xEnd - 5, yEnd);

			break;
		}
		case 6://하단 직선
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y + 5;
			rect.w = w;
			rect.h = h - 5;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 5, y, xEnd - 5, y);
			SDL_RenderDrawLine(renderer, x + 3, y + 1, xEnd - 3, y + 1);
			SDL_RenderDrawLine(renderer, x + 2, y + 2, xEnd - 2, y + 2);
			SDL_RenderDrawLine(renderer, x + 1, y + 3, xEnd - 1, y + 3);
			SDL_RenderDrawLine(renderer, x + 1, y + 4, xEnd - 1, y + 4);
			break;
		}
		case 7://상단 직선
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
			SDL_Rect rect;
			rect.x = x;
			rect.y = y;
			rect.w = w;
			rect.h = h - 5;
			SDL_RenderFillRect(renderer, &rect);

			SDL_RenderDrawLine(renderer, x + 1, yEnd - 4, xEnd - 1, yEnd - 4);
			SDL_RenderDrawLine(renderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
			SDL_RenderDrawLine(renderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
			SDL_RenderDrawLine(renderer, x + 3, yEnd - 1, xEnd - 3, yEnd - 1);
			SDL_RenderDrawLine(renderer, x + 5, yEnd, xEnd - 5, yEnd);

			break;
		}
	}


}