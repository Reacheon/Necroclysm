#include <SDL.h>


export module Profic;

import std;
import util;
import GUI;
import globalVar;
import wrapVar;
import drawText;
import checkCursor;
import drawSprite;
import textureVar;
import Player;
import drawWindow;

export class Profic : public GUI
{
private:
	inline static Profic* ptr = nullptr;
	int proficCursor;

	SDL_Rect proficBase;

	bool warningIndex = 0;

	std::array<SDL_Rect, TALENT_SIZE> proficButton = { { 0,0,0,0 }, };
public:
	Profic() : GUI(false)
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
	~Profic()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Profic* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		proficBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			proficBase.x += inputX;
			proficBase.y += inputY;
		}
		else
		{
			proficBase.x += inputX - proficBase.w / 2;
			proficBase.y += inputY - proficBase.h / 2;
		}

		int pivotX = proficBase.x + 20;
		int pivotY = proficBase.y + 56;
		for (int i = 0; i < TALENT_SIZE; i++)
		{
			int targetX = pivotX + 210 * (i / 8);
			int targetY = pivotY + (28 * (i % 8));
			proficButton[i] = { targetX, targetY, 200, 24 };
		}

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - proficBase.w / 2;
			y = inputY - proficBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			
			drawWindow(&proficBase, sysStr[7], 6);

			///////////////////////////////////////////////////////////////////////////

			
			int pivotX = proficBase.x + 20;
			int pivotY = proficBase.y + 56;

			for (int i = 0; i <= 2; i++)
			{
				setFontSize(10);
				int targetX = pivotX + 10;
				int targetY = pivotY + (28 * (-1)) + 20;
				drawTextCenter(L"#234A63" + sysStr[182], targetX + 54 + (210 * i), targetY + 1); //재능
				drawTextCenter(L"#234A63" + sysStr[183], targetX + 125 + (210 * i), targetY + 1); //랭크
				drawTextCenter(L"#234A63" + sysStr[184], targetX + 174 + (210 * i), targetY + 1); //적성
			}


			setZoom(1.5);
			for (int i = 0; i < TALENT_SIZE; i++)
			{
				SDL_Color btnColor;
				if (checkCursor(&proficButton[i]))
				{
					if (click == false) btnColor = lowCol::blue;
					else btnColor = lowCol::deepBlue;
				}
				else btnColor = lowCol::black;
				drawStadium(proficButton[i].x, proficButton[i].y, proficButton[i].w, proficButton[i].h, btnColor, 255, 5);

				int targetX = pivotX + 210 * (i / 8);
				int targetY = pivotY + (28 * (i % 8));

				std::wstring proficStr;

				std::wstring levelStr = L"Lv." + std::to_wstring(static_cast<int>(PlayerPtr->getProficLevel(i)));

				std::wstring aptStr;
				aptStr = L"x" + decimalCutter(PlayerPtr->entityInfo.proficApt[i], 1);

				switch (i)
				{
				case proficFlag::fighting:
					proficStr = sysStr[55];
					break;
				case proficFlag::dodging:
					proficStr = sysStr[56];
					break;
				case proficFlag::stealth:
					proficStr = sysStr[57];
					break;
				case proficFlag::throwing:
					proficStr = sysStr[58];
					break;
				case proficFlag::unarmedCombat:
					proficStr = sysStr[59];
					break;
				case proficFlag::piercingWeapon:
					proficStr = sysStr[60];
					break;
				case proficFlag::cuttingWeapon:
					proficStr = sysStr[61];
					break;
				case proficFlag::bashingWeapon:
					proficStr = sysStr[62];
					break;
				case proficFlag::archery:
					proficStr = sysStr[63];
					break;
				case proficFlag::gun:
					proficStr = sysStr[64];
					break;
				case proficFlag::electronics:
					proficStr = sysStr[65];
					break;
				case proficFlag::chemistry:
					proficStr = sysStr[66];
					break;
				case proficFlag::mechanics:
					proficStr = sysStr[67];
					break;
				case proficFlag::computer:
					proficStr = sysStr[68];
					break;
				case proficFlag::medicine:
					proficStr = sysStr[69];
					break;
				case proficFlag::cooking:
					proficStr = sysStr[70];
					break;
				case proficFlag::fabrication:
					proficStr = sysStr[71];
					break;
				case proficFlag::social:
					proficStr = sysStr[72];
					break;
				case proficFlag::architecture:
					proficStr = sysStr[73];
					break;
				}

				setFontSize(10);

				{
					SDL_Color rankColor = col::white;
					if (PlayerPtr->getProficLevel(i) >= MAX_PROFIC_LEVEL) rankColor = col::yellow;
					drawText(col2Str(rankColor) + proficStr, targetX + 40, targetY + 4);
				}

				//재능 랭크와 게이지 그리기
				{
					SDL_Color rankColor = col::white;
					if (PlayerPtr->getProficLevel(i) >= MAX_PROFIC_LEVEL) rankColor = col::yellow;
					drawTextCenter(col2Str(rankColor) + levelStr, targetX + 136, targetY + 10);

					SDL_Rect gauge = { targetX + 118, targetY + 18, 36, 3 };
					drawRect(gauge, col::white);
					if (PlayerPtr->getProficLevel(i) < MAX_PROFIC_LEVEL)
					{
						float levelPercent = (PlayerPtr->getProficLevel(i) - (int)PlayerPtr->getProficLevel(i));
						int gaugeLength = floor(34.0 * levelPercent);
						if (gaugeLength > 0) drawLine(targetX + 118 + 1, targetY + 18 + 1, targetX + 118 + gaugeLength, targetY + 18 + 1, col::yellowGreen);
						int q = ((int)(levelPercent * 100.0)) / 10;
						int r = ((int)(levelPercent * 100.0)) % 10;
						int qIndex = q + 27;
						int rIndex = r + 27;
						setZoom(1.0);
						if (q > 0) drawSprite(spr::epsilonFont, qIndex, targetX + 118 + 37, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, rIndex, targetX + 118 + 37 + 4, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, 43, targetX + 118 + 37 + 8, targetY + 18 - 1); //%
					}
					else
					{
						drawLine(targetX + 118 + 1, targetY + 18 + 1, targetX + 118 + 34, targetY + 18 + 1, col::yellow);
						setZoom(1.0);
						drawSprite(spr::epsilonFont, 13, targetX + 118 + 37, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, 1, targetX + 118 + 37 + 4, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, 24, targetX + 118 + 37 + 8, targetY + 18 - 1);
					}
				}

				setFontSize(10);
				drawText(L"#FFFFFF" + aptStr, targetX + 175, targetY + 6);
				setFontSize(10);

				//재능 아이콘 그리기
				setZoom(1.5);
				if (PlayerPtr->getProficLevel(i) < MAX_PROFIC_LEVEL)
				{
					drawSprite(spr::proficIcon, i, targetX, targetY);
				}
				else { drawSprite(spr::proficIconGold, i, targetX, targetY); }
				setZoom(1.0);
			}
			setZoom(1.0);

			if (warningIndex > 0) { drawText(L"#FF0000" + sysStr[74], proficBase.x + 20, proficBase.y + proficBase.h - 70); }
			//클릭하여 재능포인트의 분배 우선 순위를 결정합니다. 예로 3개의 재능을 우선분배하면 각 재능 당 경험치가 33%씩 쌓입니다.
			drawText(col2Str(col::white) + sysStr[180], proficBase.x + 20, proficBase.y + proficBase.h - 50);
			//적성이 높을 경우 레벨업에 필요한 경험치가 줄어들며 재능은 최대 S랭크까지 올릴 수 있습니다.
			drawText(col2Str(col::white) + sysStr[181], proficBase.x + 20, proficBase.y + proficBase.h - 30);
		}
		else
		{
			SDL_Rect vRect = proficBase;
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

	void setWarningIndex(int inputVal) { warningIndex = inputVal; }
};