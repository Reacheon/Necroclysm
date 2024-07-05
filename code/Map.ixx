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
import checkCursor;
import drawWindow;

export class Map : public GUI
{
private:
	inline static Map* ptr = nullptr;
	SDL_Rect mapBase;

	SDL_Texture* mapTexture = nullptr;
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

			SDL_Rect src = { 0,0, 700, 350 };
			SDL_Rect dst = { 0,0, 700, 350 };
			dst.x = mapBase.x + 1;
			dst.y = mapBase.y + 30;
			SDL_RenderCopy(renderer, mapTexture, &src, &dst);

			setFontSize(10);
			drawTextCenter(L"[ 플레이어 좌표 ]", mapBase.x + 66, mapBase.y + 389);
			drawText(L"x = 39,273", mapBase.x + 8, mapBase.y + 396 );
			drawText(L"y = 21,732", mapBase.x + 8, mapBase.y + 396 + 11);
			drawText(L"z = 0", mapBase.x + 8, mapBase.y + 396 + 22);

			setFontSize(10);
			drawTextCenter(L"마킹", mapBase.x + 397, mapBase.y + 388);

			SDL_Rect markingBtnRed = { mapBase.x + 345, mapBase.y + 396, 32, 32 };
			drawRect(markingBtnRed,col::white);
			drawSpriteCenter(spr::icon16, 49, markingBtnRed.x + 16, markingBtnRed.y + 16);
			SDL_Rect markingBtnYellow = { mapBase.x + 345 + 38, mapBase.y + 396, 32, 32 };
			drawRect(markingBtnYellow, col::white);
			drawSpriteCenter(spr::icon16, 50, markingBtnYellow.x + 16, markingBtnYellow.y + 16);
			SDL_Rect markingBtnBlue = { mapBase.x + 345 + 76, mapBase.y + 396, 32, 32 };
			drawRect(markingBtnBlue, col::white);
			drawSpriteCenter(spr::icon16, 51, markingBtnBlue.x + 16, markingBtnBlue.y + 16);


			setFontSize(10);
			drawTextCenter(L"배율", mapBase.x + 585, mapBase.y + 388);

			SDL_Rect reduceBtn = { mapBase.x + 487, mapBase.y + 396, 32, 32 };
			drawRect(reduceBtn, col::white);
			drawSpriteCenter(spr::icon16, 46, reduceBtn.x + 16, reduceBtn.y + 16);

			SDL_Rect magnifyBtn = { mapBase.x + 487 + 172, mapBase.y + 396, 32, 32 };
			drawRect(magnifyBtn, col::white);
			drawSpriteCenter(spr::icon16, 47, magnifyBtn.x + 16, magnifyBtn.y + 16);

			drawLine(mapBase.x + 525, mapBase.y + 411, mapBase.x + 525 + 127, mapBase.y + 411,col::white);
			drawLine(mapBase.x + 525, mapBase.y + 411 + 1, mapBase.x + 525 + 127, mapBase.y + 411 + 1,col::white);

			drawLine(mapBase.x + 525, mapBase.y + 411 - 2, mapBase.x + 525, mapBase.y + 411 + 2,col::white);
			drawLine(mapBase.x + 525 + 1, mapBase.y + 411 - 2, mapBase.x + 525 + 1, mapBase.y + 411 + 2,col::white);

			drawLine(mapBase.x + 525 + 127, mapBase.y + 411 - 2, mapBase.x + 525 + 127, mapBase.y + 411 + 2,col::white);
			drawLine(mapBase.x + 525 + 1 + 127, mapBase.y + 411 - 2, mapBase.x + 525 + 1 + 127, mapBase.y + 411 + 2,col::white);

			drawSprite(spr::mapHereMarker, 0, mapBase.x+339, mapBase.y + 193);
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
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};