#include <SDL3/SDL.h>

export module GameOver;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;

export class GameOver : public GUI
{
private:
	inline static GameOver* ptr = nullptr;
	SDL_Rect gameOverBase;
	int gameOverCursor = -1;
	int gameOverScroll = 0;
	float alpha = -100;
	
	std::wstring gameOverText = L"사망했다...";

	SDL_Rect optionRect;
	SDL_Rect newGameBtn;
	SDL_Rect toTitleBtn;

	float cursorSprIndex = 2.99;

	int prevBtn = -1;
public:
	GameOver(std::wstring inputText) : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one Gameover instance was generated.");
		ptr = this;



		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();

		drawHUD = false;

        gameOverText = inputText;	


		addAniUSetPlayer(PlayerPtr, aniFlag::faint);

	}
	~GameOver()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static GameOver* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		gameOverBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			gameOverBase.x += inputX;
			gameOverBase.y += inputY;
		}
		else
		{
			gameOverBase.x += inputX - gameOverBase.w / 2;
			gameOverBase.y += inputY - gameOverBase.h / 2;
		}

		optionRect = { cameraW / 2 - 110, cameraH - 230 - 80, 220,160 };
		newGameBtn = { optionRect.x + 35,optionRect.y + 27,150,42 };
		toTitleBtn = { newGameBtn.x, newGameBtn.y + 58 ,newGameBtn.w, newGameBtn.h };

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - gameOverBase.w / 2;
			y = inputY - gameOverBase.h / 2;
		}

	}
	void drawGUI()
	{

		if (alpha >= 0)
		{
			SDL_SetTextureAlphaMod(spr::youDied->getTexture(), static_cast<int>(alpha));
			drawSpriteCenter(spr::youDied, 0, cameraW / 2, 200);
			SDL_SetTextureAlphaMod(spr::youDied->getTexture(), 255);
		}
		
		if (alpha >= 255)
		{
			int markerIndex = 0;
			if (timer::timer600 % 30 < 5) { markerIndex = 0; }
			else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
			else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
			else if (timer::timer600 % 30 < 20) { markerIndex = 1; }
			else { markerIndex = 0; }

			setFontSize(12);
			renderTextOutlineCenter(gameOverText, cameraW / 2, 100 + 155);

			drawSpriteCenter(spr::gameOverOptionRect, 0, optionRect.x + optionRect.w / 2, optionRect.y + optionRect.h / 2);

			setFontSize(15);
			drawRect(newGameBtn, col::lightGray);
			if (checkCursor(&newGameBtn))
			{
				if (click == false)
				{
					drawFillRect(SDL_Rect{ newGameBtn.x + 1,newGameBtn.y + 1,newGameBtn.w - 2,newGameBtn.h - 2 }, lowCol::blue);
					drawRect(SDL_Rect{ newGameBtn.x + 1,newGameBtn.y + 1,newGameBtn.w - 2,newGameBtn.h - 2 }, col::white, 200);
				}
				else
				{
					drawFillRect(SDL_Rect{ newGameBtn.x + 1,newGameBtn.y + 1,newGameBtn.w - 2,newGameBtn.h - 2 }, lowCol::deepBlue);
				}

				drawSpriteCenter(spr::gameOverOptionMarker, std::ceil(cursorSprIndex), newGameBtn.x + newGameBtn.w / 2, newGameBtn.y + newGameBtn.h / 2);
			}
			else drawFillRect(SDL_Rect{ newGameBtn.x + 1,newGameBtn.y + 1,newGameBtn.w - 2,newGameBtn.h - 2 }, col::black);
			renderTextOutlineCenter(L"새 게임 시작", newGameBtn.x + newGameBtn.w / 2, newGameBtn.y + newGameBtn.h / 2);

			drawRect(toTitleBtn, col::lightGray);
			if (checkCursor(&toTitleBtn))
			{
				if (click == false)
				{
					drawFillRect(SDL_Rect{ toTitleBtn.x + 1,toTitleBtn.y + 1,toTitleBtn.w - 2,toTitleBtn.h - 2 }, lowCol::blue);
					drawRect(SDL_Rect{ toTitleBtn.x + 1,toTitleBtn.y + 1,toTitleBtn.w - 2,toTitleBtn.h - 2 }, col::white, 200);
				}
				else drawFillRect(SDL_Rect{ toTitleBtn.x + 1,toTitleBtn.y + 1,toTitleBtn.w - 2,toTitleBtn.h - 2 }, lowCol::deepBlue);

				drawSpriteCenter(spr::gameOverOptionMarker, std::ceil(cursorSprIndex), toTitleBtn.x + toTitleBtn.w / 2, toTitleBtn.y + toTitleBtn.h / 2);
			}
			else drawFillRect(SDL_Rect{ toTitleBtn.x + 1,toTitleBtn.y + 1,toTitleBtn.w - 2,toTitleBtn.h - 2 }, col::black);
			renderTextOutlineCenter(L"타이틀 화면으로", toTitleBtn.x + toTitleBtn.w / 2, toTitleBtn.y + toTitleBtn.h / 2);
		}
		
	}
	void clickUpGUI()
	{
		if (alpha >= 255)
		{
			if (checkCursor(&toTitleBtn))
			{
				exit(-1);
			}
			else if (checkCursor(&newGameBtn))
			{
				exit(-1);
			}
		}
	}
	void clickMotionGUI(int dx, int dy) {}
	void clickDownGUI() {}
	void clickRightGUI() {}
	void clickHoldGUI() {}
	void mouseWheel() {}
	void gamepadBtnDown() {}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}
	void step() 
	{
		if (alpha < 255) alpha += 1.2;
		else alpha = 255.0f;



		if (checkCursor(&newGameBtn))
		{
			if (prevBtn != 0)
			{
				prevBtn = 0;
				cursorSprIndex = 2.99;
			}
		}
		else if (checkCursor(&toTitleBtn))
		{
			if (prevBtn != 1)
			{
				prevBtn = 1;
				cursorSprIndex = 2.99;
			}
		}
		else
		{
			prevBtn = -1;
			cursorSprIndex = 2.99;
		}

		if (cursorSprIndex > 0) cursorSprIndex -= 0.2;
		else cursorSprIndex = 0;
	}
};