#include <SDL.h>

export module Aim;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import Player;
import World;
import Entity;

export class Aim : public GUI
{
private:
	inline static Aim* ptr = nullptr;
	SDL_Rect aimBase;
	std::vector<std::array<int, 3>> aimTrailRev;
	Point3 aimCoord = { 0,0,0 };

	double aimAcc = 0;
	double fakeAimAcc = aimAcc;
	int aimStack = 0;
public:
	Aim() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeAim;
		Player::ins()->setSpriteIndex(charSprIndex::AIM_RIFLE);

		int pX = Player::ins()->getGridX(), pY = Player::ins()->getGridY(), pZ = Player::ins()->getGridZ();
		
		if(Player::ins()->entityInfo.sprFlip == false) aimCoord = { pX + 1, pY, pZ };
		else  aimCoord = { pX - 1, pY, pZ };
		
	}
	~Aim()
	{
		Player::ins()->setSpriteIndex(charSprIndex::WALK);
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Aim* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		aimBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			aimBase.x += inputX;
			aimBase.y += inputY;
		}
		else
		{
			aimBase.x += inputX - aimBase.w / 2;
			aimBase.y += inputY - aimBase.h / 2;
		}



		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - aimBase.w / 2;
			y = inputY - aimBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		SDL_Rect dst;

		setFontSize(13);


		if (aimAcc != 0)
		{
			std::wstring accStr = decimalCutter(fakeAimAcc * 100.0, 1);
			accStr += L"%";
			//if(aimStack>0) accStr += L" (" + std::to_wstring(aimStack) + L")";

			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2 + 2, cameraH / 2 - 20 * zoomScale);
			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2 - 2, cameraH / 2 - 20 * zoomScale);
			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2, cameraH / 2 + 2 - 20 * zoomScale);
			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2, cameraH / 2 - 2 - 20 * zoomScale);

			drawTextCenter(col2Str(col::white) + accStr, cameraW / 2, cameraH / 2 - 20 * zoomScale);
		}


		{
			int tgtX = aimCoord.x;
			int tgtY = aimCoord.y;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = 16.0 * zoomScale;
			dst.h = 16.0 * zoomScale;
			setZoom(zoomScale);
			drawSpriteCenter
			(
				spr::aimMarkerTmp,
				0,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			setZoom(1.0);
		}

		for (int i = 0; i < aimTrailRev.size(); i++)
		{
			int tgtX = aimTrailRev[i][0];
			int tgtY = aimTrailRev[i][1];
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = 16.0 * zoomScale;
			dst.h = 16.0 * zoomScale;
			setZoom(zoomScale);
			drawSpriteCenter
			(
				spr::trail,
				1,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			setZoom(1.0);
		}


		drawSpriteCenter(spr::floatLog, 0, cameraW / 2, 52);
		drawTextCenter(L"#FFFFFF사격할 위치를 선택해주세요.", cameraW / 2, 52);
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::null);
		}
	}
	void clickMotionGUI(int dx, int dy) {}
	void clickDownGUI() {}
	void clickRightGUI() {}
	void clickHoldGUI() {}
	void gamepadBtnDown() 
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			if (aimAcc != 0)
			{
				aimAcc += randomRangeFloat(0.03, 0.07);
				if (aimAcc > 0.999) aimAcc = 0.999;
				Player::ins()->setFlashType(1);
				aimStack++;
			}
			break;
		}
	}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}
	void step() 
	{

		{
			Uint8 targetR, targetG, targetB, targetAlpha;
			Player::ins()->getFlashRGBA(targetR, targetG, targetB, targetAlpha);
			if (Player::ins()->getFlashType() == 1)
			{
				if (targetAlpha > 17) { targetAlpha -= 17; }
				else { targetAlpha = 0; }
				Player::ins()->setFlashRGBA(targetR, targetG, targetB, targetAlpha);
			}
		}


		if (fabs(aimAcc - fakeAimAcc) > 0.001)
		{
			if (aimAcc > fakeAimAcc)
			{
				fakeAimAcc += 0.002;
			}
			else if (aimAcc < fakeAimAcc)
			{
				fakeAimAcc -= 0.002;
			}
		}



		if (inputType == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 1000)
			{
				prt(L"탭이 실행되었다.\n");
				executeTab();
				delayR2 = 20;
			}
			else delayR2--;


			if (dpadDelay <= 0)
			{
				dpadDelay = 6;
				int dir = -1;
				SDL_PollEvent(&event);
				bool dpadUpPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
				bool dpadDownPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				bool dpadLeftPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				bool dpadRightPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

				if (dpadUpPressed && dpadLeftPressed) dir = 3;
				else if (dpadUpPressed && dpadRightPressed) dir = 1;
				else if (dpadDownPressed && dpadLeftPressed) dir = 5;
				else if (dpadDownPressed && dpadRightPressed) dir = 7;
				else if (dpadUpPressed) dir = 2;
				else if (dpadDownPressed) dir = 6;
				else if (dpadLeftPressed) dir = 4;
				else if (dpadRightPressed) dir = 0;

				if (dir != -1)
				{
					int dx, dy;
					dir2Coord(dir, dx, dy);
					aimCoord.x += dx;
					aimCoord.y += dy;

					if (World::ins()->getTile(aimCoord.x, aimCoord.y, aimCoord.z).EntityPtr != nullptr)
					{
						aimAcc = randomRangeFloat(0.5, 0.8);
					}
					else aimAcc = 0;
					fakeAimAcc = aimAcc;
					aimStack = 0;

					if (aimCoord.x < Player::ins()->getGridX())
					{
						Player::ins()->setDirection(4);
					}
					else if (aimCoord.x > Player::ins()->getGridX())
					{
						Player::ins()->setDirection(0);
					}

					std::vector<std::array<int, 2>> lineCoord;
					makeLine(lineCoord, aimCoord.x - Player::ins()->getGridX(), aimCoord.y - Player::ins()->getGridY());
					aimTrailRev.clear();
					for (int i = 0; i < lineCoord.size(); i++)
					{
						if (lineCoord[i][0] != 0 || lineCoord[i][1] != 0)
						{
							aimTrailRev.push_back({ Player::ins()->getGridX() + lineCoord[i][0],Player::ins()->getGridY() + lineCoord[i][1],Player::ins()->getGridZ() });
						}
					}
				}
			}
			else dpadDelay--;
		}
	}

	void executeTab()
	{
		close(aniFlag::null);
	}
};