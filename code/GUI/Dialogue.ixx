#include <SDL3/SDL.h>

export module Dialogue;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;

export class Dialogue : public GUI
{
private:
	inline static Dialogue* ptr = nullptr;
	SDL_Rect DialogueBase;
public:
	Dialogue() : GUI(false)
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
	~Dialogue()
	{
		ptr = nullptr;
	}
	static Dialogue* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		DialogueBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			DialogueBase.x += inputX;
			DialogueBase.y += inputY;
		}
		else
		{
			DialogueBase.x += inputX - DialogueBase.w / 2;
			DialogueBase.y += inputY - DialogueBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - DialogueBase.w / 2;
			y = inputY - DialogueBase.h / 2;
		}


	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&DialogueBase, sysStr[54], 11);
		}
		else
		{
			SDL_Rect vRect = DialogueBase;
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