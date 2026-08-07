module;
#include <SDL3/SDL.h>

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
	int mapCursor = -1;
	int mapScroll = 0;
public:
	Map() : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than one map instance was generated.");
		ptr = this;

		changeXY(cameraW / 2, cameraH / 2, true);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}
	~Map()
	{
		ptr = nullptr;
	}
	static Map* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		mapBase = { 0, 0, 650, 376 };

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
			drawWindow(&mapBase, sysStr[198], 3);

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
		else
		{
		}
	}

	void step()
	{
		tabType = tabFlag::back;
	}
};
