module;
#include <SDL3/SDL.h>

export module TitleScreen;

import std;
import util;
import GUI;
import constVar;
import drawText;
import globalVar;

export class TitleScreen : public GUI
{
private:
	inline static TitleScreen* ptr = nullptr;
	bool savedDrawHUD = true;
public:
	TitleScreen() : GUI(false)
	{
		//1개 이상의 타이틀 화면 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one TitleScreen instance was generated.");
		ptr = this;

		changeXY(0, 0, false);

		savedDrawHUD = drawHUD;
		drawHUD = false;
	}
	~TitleScreen()
	{
		drawHUD = savedDrawHUD;
		ptr = nullptr;
	}
	static TitleScreen* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		x = inputX;
		y = inputY;
	}
	void drawGUI()
	{
		drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, col::black);

		setFont(fontType::mainFontExtraBold);
		setFontSize(48);
		drawTextCenter(L"NECROCLYSM", cameraW / 2, cameraH / 4, col::white);

		setFont(fontType::mainFont);
		setFontSize(15);
	}
	void clickUpGUI()
	{
		//임시: 아무 곳이나 클릭하면 타이틀 화면 닫기
		close(aniFlag::null);
	}
};
