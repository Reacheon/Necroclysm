module;
#include <SDL3/SDL.h>

export module CharSelectScreen;

import std;
import util;
import GUI;
import constVar;
import drawText;
import globalVar;

export class CharSelectScreen : public GUI
{
private:
	inline static CharSelectScreen* ptr = nullptr;
	bool savedDrawHUD = true;
public:
	CharSelectScreen() : GUI(false)
	{
		//1개 이상의 캐릭터 선택 화면 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one CharSelectScreen instance was generated.");
		ptr = this;

		changeXY(0, 0, false);

		savedDrawHUD = drawHUD;
		drawHUD = false;
	}
	~CharSelectScreen()
	{
		drawHUD = savedDrawHUD;
		ptr = nullptr;
	}
	static CharSelectScreen* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		x = inputX;
		y = inputY;
	}
	void drawGUI()
	{
		drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, col::black);
	}
	void clickUpGUI()
	{
		//임시: 아무 곳이나 클릭하면 캐릭터 선택 화면 닫기
		close(aniFlag::null);
	}
};
