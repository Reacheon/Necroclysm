#include <SDL.h>

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

//입력한 해당 위치에 윈도우를 그린다. 만약 뒤에 인자가 더 올 경우 타이틀도 그린다
export void drawWindow(int x, int y, int w, int h)
{
	//윈도우 박스 그리기
	SDL_Rect windowRect = { x,y,w,h };
	//기본 사각형 
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
	SDL_RenderFillRect(renderer, &windowRect);
	//회색 테두리
	SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
	SDL_RenderDrawRect(renderer, &windowRect);
}

export void drawWindow(int x, int y, int w, int h, std::wstring titleName, int titleSprIndex)
{
	drawWindow(x,y,w,h);

	//제목 부분
	SDL_Rect windowRect = { x,y,w,h };
	SDL_Rect titleRect = { windowRect.x, windowRect.y, windowRect.w, 30 };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
	SDL_RenderFillRect(renderer, &titleRect);
	SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
	SDL_RenderDrawRect(renderer, &titleRect);

	setFontSize(16);
	drawTextCenter(col2Str(col::white)+titleName, x + w/2, y + 14);
	setZoom(1.5);
	drawSpriteCenter(spr::icon16, titleSprIndex, x + w/2 - queryTextWidth(titleName, true) / 2 - 20, y + 14);
	setZoom(1.0);
}

export void drawWindow(const SDL_Rect* rect)
{
	drawWindow(rect->x, rect->y, rect->w, rect->h);
}

export void drawWindow(const SDL_Rect* rect, std::wstring titleName, int titleSprIndex)
{
	drawWindow(rect->x, rect->y, rect->w, rect->h,titleName, titleSprIndex);
}

export void drawEdgeWindow(int x, int y, int w, int h, int edgeWidth, dir16 arrowDir)
{
	//윈도우 박스 그리기
	SDL_Rect windowRect = { x,y,w,h };
	//기본 사각형 
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, windowAlpha);
	SDL_RenderFillRect(renderer, &windowRect);


	int leftTopX = x;
	int leftTopY = y;
	int rightTopX = x + w - 1;
	int rightTopY = y;
	int leftBotX = x;
	int leftBotY = y + h - 1;
	int rightBotX = x + w - 1;
	int rightBotY = y + h - 1;

	//회색 테두리
	SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
	if(arrowDir != dir16::dir2) SDL_RenderDrawLine(renderer, leftTopX, leftTopY, rightTopX, rightTopY);
	else
	{
		int topPivotX = x + w / 2;
		int topPivotY = y;
		SDL_RenderDrawLine(renderer, leftTopX, leftTopY, topPivotX - 10, topPivotY);
		SDL_RenderDrawLine(renderer, rightTopX, rightTopY, topPivotX + 10, topPivotY);
		drawSprite(spr::windowArrow2, 1, topPivotX - 11, topPivotY - 10);
	}
	if (arrowDir != dir16::dir4) SDL_RenderDrawLine(renderer, leftTopX, leftTopY, leftBotX, leftBotY);
	else
	{
		int leftPivotX = x;
		int leftPivotY = y + h / 2;
		SDL_RenderDrawLine(renderer, leftTopX, leftTopY, leftPivotX, leftPivotY - 10);
		SDL_RenderDrawLine(renderer, leftBotX, leftBotY, leftPivotX, leftPivotY + 10);
		drawSprite(spr::windowArrow2, 2, leftPivotX - 11, leftPivotY - 11);

	}
	if (arrowDir != dir16::dir0) SDL_RenderDrawLine(renderer, rightBotX, rightBotY, rightTopX, rightTopY);
	else
	{
		int rightPivotX = x + w - 1;
		int rightPivotY = y + h / 2;
		SDL_RenderDrawLine(renderer, rightTopX, rightTopY, rightPivotX, rightPivotY - 10);
		SDL_RenderDrawLine(renderer, rightBotX, rightBotY, rightPivotX, rightPivotY + 10);
		drawSprite(spr::windowArrow2, 0, rightPivotX + 1, rightPivotY - 11);
	}
	if (arrowDir != dir16::dir6) SDL_RenderDrawLine(renderer, rightBotX, rightBotY, leftBotX, leftBotY);
	else
	{
		int botPivotX = x + w / 2;
		int botPivotY = y + h - 1;
		SDL_RenderDrawLine(renderer, leftBotX, leftBotY, botPivotX - 10, botPivotY);
		SDL_RenderDrawLine(renderer, rightBotX, rightBotY, botPivotX + 10, botPivotY);
		drawSprite(spr::windowArrow2, 3, botPivotX - 11, botPivotY + 1);
	}


	SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 255);
	SDL_RenderDrawLine(renderer, leftTopX, leftTopY, leftTopX + (edgeWidth-1), leftTopY);
	SDL_RenderDrawLine(renderer, leftTopX, leftTopY + 1, leftTopX + (edgeWidth - 1), leftTopY + 1);
	SDL_RenderDrawLine(renderer, leftTopX, leftTopY, leftTopX, leftTopY + (edgeWidth - 1));
	SDL_RenderDrawLine(renderer, leftTopX + 1, leftTopY, leftTopX + 1, leftTopY + (edgeWidth - 1));

	SDL_RenderDrawLine(renderer, rightTopX, rightTopY, rightTopX - (edgeWidth - 1), rightTopY);
	SDL_RenderDrawLine(renderer, rightTopX, rightTopY + 1, rightTopX - (edgeWidth - 1), rightTopY + 1);
	SDL_RenderDrawLine(renderer, rightTopX, rightTopY, rightTopX, rightTopY + (edgeWidth - 1));
	SDL_RenderDrawLine(renderer, rightTopX-1, rightTopY, rightTopX-1, rightTopY + (edgeWidth - 1));

	SDL_RenderDrawLine(renderer, leftBotX, leftBotY, leftBotX + (edgeWidth - 1), leftBotY);
	SDL_RenderDrawLine(renderer, leftBotX, leftBotY - 1, leftBotX + (edgeWidth - 1), leftBotY - 1);
	SDL_RenderDrawLine(renderer, leftBotX, leftBotY, leftBotX, leftBotY - (edgeWidth - 1));
	SDL_RenderDrawLine(renderer, leftBotX + 1, leftBotY, leftBotX + 1, leftBotY - (edgeWidth - 1));

	SDL_RenderDrawLine(renderer, rightBotX, rightBotY, rightBotX - (edgeWidth - 1), rightBotY);
	SDL_RenderDrawLine(renderer, rightBotX, rightBotY - 1, rightBotX - (edgeWidth - 1), rightBotY - 1);
	SDL_RenderDrawLine(renderer, rightBotX, rightBotY, rightBotX, rightBotY - (edgeWidth - 1));
	SDL_RenderDrawLine(renderer, rightBotX - 1, rightBotY, rightBotX - 1, rightBotY - (edgeWidth - 1));
}

export void drawEdgeWindow(int x, int y, int w, int h, int edgeWidth)
{
	drawEdgeWindow(x, y, w, h, edgeWidth, dir16::dir0_5);
}