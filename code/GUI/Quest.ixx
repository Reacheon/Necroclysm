#include <SDL3/SDL.h>

export module Quest;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;

export class Quest : public GUI
{
private:
	inline static Quest* ptr = nullptr;
	SDL_Rect questBase;
	int questCursor = -1;
	int questScroll = 0;
public:
	Quest() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Quest()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Quest* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		questBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			questBase.x += inputX;
			questBase.y += inputY;
		}
		else
		{
			questBase.x += inputX - questBase.w / 2;
			questBase.y += inputY - questBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - questBase.w / 2;
			y = inputY - questBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&questBase, sysStr[198], 3);

		}
		else
		{
			SDL_Rect vRect = questBase;
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
};