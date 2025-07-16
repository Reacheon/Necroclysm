#include <SDL3/SDL.h>

export module SystemMenu;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import drawPrimitive;
import globalVar;
import checkCursor;
import drawWindow;
import drawText;

export class SystemMenu : public GUI
{
private:
	inline static SystemMenu* ptr = nullptr;
	SDL_Rect systemMenuBase;
	int systemMenuCursor = -1;
	int systemMenuScroll = 0;
	Uint64 startTime = 0;
public:
	SystemMenu() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		deactInput();
		startTime = SDL_GetTicks();
	}
	~SystemMenu()
	{
		ptr = nullptr;
	}
	static SystemMenu* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		float alphaRatio = myMin(1.0,1.0 * ((SDL_GetTicks() - startTime) / 200.0));
        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, col::black, 120* alphaRatio);

		if (SDL_GetTicks() - startTime >= 200)
		{
			setFontSize(32);
			renderTextOutlineCenter(L"Necroclysm", cameraW / 2 + 195, cameraH / 2 - 69);

			for (int i = 0; i < 181; i++)
			{
				drawPoint(cameraW / 2 + 102+i, cameraH / 2 - 48, col::white);
			}
			
			setFontSize(22);
			renderTextOutline(L"Back to Game", cameraW / 2 + 255 - queryTextWidth(L"Back to Game"), cameraH / 2 ,col::white);
			renderTextOutline(L"Option", cameraW / 2 + 255 - queryTextWidth(L"Option"), cameraH / 2 + 28, col::white);
			renderTextOutline(L"Save and Quit", cameraW / 2 + 255 - queryTextWidth(L"Save and Quit"), cameraH / 2 + 56, col::white);

		}
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }
	}

	void keyUpGUI()
	{
		switch (event.key.key)
		{
		case SDLK_ESCAPE:
			close(aniFlag::null);
			break;
		}


        
	}
};