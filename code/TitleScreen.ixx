#include <SDL.h>

export module TitleScreen;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import drawPrimitive;

export class TitleScreen : public GUI
{
private:
	inline static TitleScreen* ptr = nullptr;
	SDL_Rect titleScreenBase;
	int titleScreenCursor = -1;
public:
	TitleScreen() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

	}
	~TitleScreen()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static TitleScreen* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		drawFillRect({ 0,0,cameraW,cameraH }, col::black);

		setFontSize(30);
		drawTextCenter(col2Str(col::white) + L"▶ New game",cameraW/2 + 212 ,cameraH - 380 + 40*0);
		drawTextCenter(col2Str(col::gray) + L"Continue", cameraW / 2 + 212, cameraH - 380 + 40 * 1);
		drawTextCenter(col2Str(col::gray) + L"Sprints", cameraW / 2 + 212, cameraH - 380 + 40 * 2);
		drawTextCenter(col2Str(col::gray) + L"Options", cameraW / 2 + 212, cameraH - 380 + 40 * 3);
		drawTextCenter(col2Str(col::gray) + L"Quit", cameraW / 2 + 212, cameraH - 380 + 40 * 4);

		setFontSize(20);
		drawText(col2Str(col::gray) + L"v0.10.0 alpha 20240907", cameraW / 2 - 349, cameraH - 32);

	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

	}
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() {}
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};