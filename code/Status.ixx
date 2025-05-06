#include <SDL.h>

export module Status;

import std;
import util;
import GUI;
import globalVar;
import checkCursor;

export class Status : public GUI
{
private:
	inline static Status* ptr;
	SDL_Rect statusBase;
	SDL_Rect statusWindow;

	int statusCursor;
	int statusScroll;
public:
	Status(int targetGridX, int targetGridY) : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than one Loot instance was generated.");
		ptr = this;

		changeXY((cameraW / 2) + 17, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		tabType = tabFlag::closeWin;
		UIType = act::loot;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);
	}
	~Status()
	{
		prt(L"Status : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		barAct = actSet::null;
		tabType = tabFlag::autoAtk;
	}
	static Status* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		statusBase = { 0,0,335,420 };
		if (center == false)
		{
			statusBase.x += inputX;
			statusBase.y += inputY;
		}
		else
		{
			statusBase.x += inputX - statusBase.w / 2;
			statusBase.y += inputY - statusBase.h / 2;
		}
		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - statusBase.w / 2;
			y = inputY - statusBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		drawStadium(statusWindow.x, statusWindow.y, statusWindow.w, statusWindow.h, { 0,0,0 }, 150, 5);
	}
	void clickUpGUI()//입력
	{
		if (checkCursor(&tab) == true){close(aniFlag::winSlipClose);}
		else
		{
		}
	}
	void clickMotionGUI(int dx, int dy){/*사용 X*/}
	void clickDownGUI(){/*사용 X*/}
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() {}
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};