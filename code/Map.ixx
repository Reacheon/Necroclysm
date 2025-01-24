#include <SDL.h>
#include <SDL_image.h>

export module Map;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import wrapVar;
import checkCursor;
import drawWindow;
import Player;
import World;
import Mapmaker;

export class Map : public GUI
{
private:
	inline static Map* ptr = nullptr;
	SDL_Rect mapBase;
	SDL_Rect mapGrids;

	SDL_Texture* mapTexture = nullptr;

	int initCursorX = 0;
	int initCursorY = 0;
	int cursorX = 0;
	int cursorY = 0;
public:
	Map() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one Map instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);


		tabType = tabFlag::closeWin;
		mapTexture = IMG_LoadTexture(renderer, "image/map/map1.png");
		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Map()
	{
		SDL_DestroyTexture(mapTexture);
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Map* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		mapBase = { 0, 0, 702, 434 };

		if (center == false)
		{
			mapBase.x += inputX;
			mapBase.y += inputY;
		}
		else
		{
			mapBase.x += inputX - mapBase.w / 2;
			mapBase.y += inputY - mapBase.h / 2;
		}

		mapGrids = { mapBase.x + 7,mapBase.x + 37,688,336 };


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - mapBase.w / 2;
			y = inputY - mapBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&mapBase, sysStr[150],45);

			//SDL_Rect src = { 0,0, 700, 350 };
			//SDL_Rect dst = { 0,0, 700, 350 };
			//dst.x = mapBase.x + 1;
			//dst.y = mapBase.y + 30;
			//SDL_RenderCopy(renderer, mapTexture, &src, &dst);



			int pChunkX, pChunkY;
			int pChunkZ = PlayerZ();
			World::ins()->changeToChunkCoord(PlayerX(), PlayerY(), pChunkX, pChunkY);
			for (int x = 0; x < 43; x++)
			{
				for (int y = 0; y < 21; y++)
				{
					chunkFlag tgtChunk = Mapmaker::ins()->getProphecy(pChunkX, pChunkY, pChunkZ);

					if (tgtChunk == chunkFlag::seawater)
					{
						drawFillRect({ mapBase.x + 7 + 16 * x,mapBase.y + 90 + 16 * y,16,16 }, col::blue);
						drawRect({ mapBase.x + 7 + 16 * x,mapBase.y + 90 + 16 * y,16,16 }, col::black);
					}
					else drawFillRect({ mapBase.x + 7 + 16 * x,mapBase.y + 90 + 16 * y,16,16 }, col::green);
					
					if (x == 21 && y == 10)
					{
						if (SDL_GetTicks() % 600 < 300)
						{
							drawSprite(spr::cursorMarker, mapBase.x + 7 + 16 * x, mapBase.y + 90 + 16 * y);
						}
					}
					
				}
			}


			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



			setFontSize(10);
			std::wstring pCoordStr = L"[ 플레이어 좌표 ]";

			drawRect({ mapBase.x + 6, mapBase.y + 38,128,46 }, col::white);

			SDL_Rect backBlackRect;
			backBlackRect.w = queryTextWidth(pCoordStr) + 10;
			backBlackRect.h = 13;
			backBlackRect.x = mapBase.x + 71 - backBlackRect.w / 2;
			backBlackRect.y = mapBase.y + 39 - backBlackRect.h / 2;
			drawFillRect(backBlackRect, col::black);
			drawTextCenter(col2Str(col::white) + pCoordStr, mapBase.x + 71, mapBase.y + 39);
			drawText(col2Str(col::white) + L"X = 39,273", mapBase.x + 13, mapBase.y + 46);
			drawText(col2Str(col::white) + L"Y = 21,732", mapBase.x + 13, mapBase.y + 46 + 11);
			drawText(col2Str(col::white) + L"Z = 0", mapBase.x + 13, mapBase.y + 46 + 22);

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			drawRect({ mapBase.x + 6 + 134, mapBase.y + 38,188,46 }, col::lightGray);

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			setFontSize(10);
			drawTextCenter(col2Str(col::white) + L"마킹", mapBase.x + 397, mapBase.y + 39);


			SDL_Rect markingBtnRed = { mapBase.x + 345, mapBase.y + 47, 32, 32 };
			drawFillRect(markingBtnRed, col::black);
			drawRect(markingBtnRed,col::white);
			drawSpriteCenter(spr::icon16, 49, markingBtnRed.x + 16, markingBtnRed.y + 16);
			SDL_Rect markingBtnYellow = { mapBase.x + 345 + 38, mapBase.y + 47, 32, 32 };
			drawFillRect(markingBtnYellow, col::black);
			drawRect(markingBtnYellow, col::white);
			drawSpriteCenter(spr::icon16, 50, markingBtnYellow.x + 16, markingBtnYellow.y + 16);
			SDL_Rect markingBtnBlue = { mapBase.x + 345 + 76, mapBase.y + 47, 32, 32 };
			drawFillRect(markingBtnBlue, col::black);
			drawRect(markingBtnBlue, col::white);
			drawSpriteCenter(spr::icon16, 51, markingBtnBlue.x + 16, markingBtnBlue.y + 16);

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



			setFontSize(10);
			drawTextCenter(L"배율", mapBase.x + 585, mapBase.y + 39);

			SDL_Rect reduceBtn = { mapBase.x + 487, mapBase.y + 47, 32, 32 };
			drawFillRect(reduceBtn, col::black);
			drawRect(reduceBtn, col::white);
			drawSpriteCenter(spr::icon16, 46, reduceBtn.x + 16, reduceBtn.y + 16);

			SDL_Rect magnifyBtn = { mapBase.x + 487 + 172, mapBase.y + 47, 32, 32 };
			drawFillRect(magnifyBtn, col::black);
			drawRect(magnifyBtn, col::white);
			drawSpriteCenter(spr::icon16, 47, magnifyBtn.x + 16, magnifyBtn.y + 16);

			drawSpriteCenter(spr::mapMagnifyIcon, 2, mapBase.x + 586, mapBase.y + 62);


			//drawLine(mapBase.x + 525, mapBase.y + 62, mapBase.x + 525 + 127, mapBase.y + 62,col::white);
			//drawLine(mapBase.x + 525, mapBase.y + 62 + 1, mapBase.x + 525 + 127, mapBase.y + 62 + 1,col::white);

			//drawLine(mapBase.x + 525, mapBase.y + 62 - 2, mapBase.x + 525, mapBase.y + 62 + 2,col::white);
			//drawLine(mapBase.x + 525 + 1, mapBase.y + 62 - 2, mapBase.x + 525 + 1, mapBase.y + 62 + 2,col::white);

			//drawLine(mapBase.x + 525 + 127, mapBase.y + 62 - 2, mapBase.x + 525 + 127, mapBase.y + 62 + 2,col::white);
			//drawLine(mapBase.x + 525 + 1 + 127, mapBase.y + 62 - 2, mapBase.x + 525 + 1 + 127, mapBase.y + 62 + 2,col::white);

			//drawSprite(spr::mapHereMarker, 0, mapBase.x+339, mapBase.y + 193);
		}
		else
		{
			SDL_Rect vRect = mapBase;
			int type = 1;
			switch (type)
			{
			case 0:
				vRect.w = vRect.w * getFoldRatio();
				vRect.h = vRect.h * getFoldRatio();
				break;
			case 1:
				vRect.x = vRect.x + vRect.w * (1 - getFoldRatio()) / 2;
				vRect.w = vRect.w * getFoldRatio();
				break;
			}
			drawWindow(&vRect);
		}
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
	}
	void clickMotionGUI(int dx, int dy) 
	{
		if (checkCursor(&mapGrids))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // 가속상수, 작아질수록 스크롤 속도가 빨라짐
				cursorX = initCursorX + dx / scrollAccelConst;
				cursorY = initCursorY + dy / scrollAccelConst;
			}
		}
	}
	void clickDownGUI() 
	{ 
		initCursorX = cursorX;
		initCursorY = cursorY;
	}
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};