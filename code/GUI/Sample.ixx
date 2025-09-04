#include <SDL3/SDL.h>

export module Sample;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;

export class Sample : public GUI
{
private:
	inline static Sample* ptr = nullptr;
	SDL_Rect sampleBase;
	int sampleCursor = -1;
	int sampleScroll = 0;
public:
	Sample() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Sample()
	{
		ptr = nullptr;
	}
	static Sample* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		sampleBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			sampleBase.x += inputX;
			sampleBase.y += inputY;
		}
		else
		{
			sampleBase.x += inputX - sampleBase.w / 2;
			sampleBase.y += inputY - sampleBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - sampleBase.w / 2;
			y = inputY - sampleBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&sampleBase, sysStr[198], 3);

		}
		else
		{
			SDL_Rect vRect = sampleBase;
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