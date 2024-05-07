#include <SDL.h>

export module Talent;

import std;
import util;
import GUI;
import globalVar;
import drawPrimitive;
import drawText;
import checkCursor;
import drawSprite;
import textureVar;
import Player;
import drawWindow;

export class Talent : public GUI
{
private:
	inline static Talent* ptr = nullptr;
	int talentCursor;

	SDL_Rect talentBase;

	bool warningIndex = 0;

	std::array<SDL_Rect, talentSize> talentButton = { { 0,0,0,0 }, };
public:
	Talent() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		setAniType(aniFlag::winUnfoldOpen);
		aniUSet.insert(this);
		turnCycle = turn::playerAnime;
	}
	~Talent()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Talent* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		talentBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			talentBase.x += inputX;
			talentBase.y += inputY;
		}
		else
		{
			talentBase.x += inputX - talentBase.w / 2;
			talentBase.y += inputY - talentBase.h / 2;
		}

		int pivotX = talentBase.x + 20;
		int pivotY = talentBase.y + 56;
		for (int i = 0; i < talentSize; i++)
		{
			int targetX = pivotX + 210 * (i / 8);
			int targetY = pivotY + (28 * (i % 8));
			talentButton[i] = { targetX, targetY, 200, 24 };
		}

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - talentBase.w / 2;
			y = inputY - talentBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			
			drawWindow(&talentBase, sysStr[7], 6);

			///////////////////////////////////////////////////////////////////////////

			
			int pivotX = talentBase.x + 20;
			int pivotY = talentBase.y + 56;

			for (int i = 0; i <= 2; i++)
			{
				setFontSize(10);
				int targetX = pivotX + 10;
				int targetY = pivotY + (28 * (-1)) + 20;
				drawTextCenter(L"#234A63재능", targetX + 54 + (210 * i), targetY + 1);
				drawTextCenter(L"#234A63랭크", targetX + 125 + (210 * i), targetY + 1);
				drawTextCenter(L"#234A63적성", targetX + 174 + (210 * i), targetY + 1);
			}


			setZoom(1.5);
			for (int i = 0; i < talentSize; i++)
			{
				SDL_Color btnColor;
				if (checkCursor(&talentButton[i]))
				{
					if (click == false) btnColor = lowCol::blue;
					else btnColor = lowCol::deepBlue;
				}
				else btnColor = lowCol::black;
				drawStadium(talentButton[i].x, talentButton[i].y, talentButton[i].w, talentButton[i].h, btnColor, 255, 5);

				int targetX = pivotX + 210 * (i / 8);
				int targetY = pivotY + (28 * (i % 8));

				std::wstring focusStr;
				std::wstring colorStr;
				std::wstring talentStr;
				std::wstring levelStr = L"Rank ";
				if (Player::ins()->getTalentLevel(i) < 1.0) levelStr += L"F-";
				else if (Player::ins()->getTalentLevel(i) < 2.0) levelStr += L"F";
				else if (Player::ins()->getTalentLevel(i) < 3.0) levelStr += L"F+";
				else if (Player::ins()->getTalentLevel(i) < 4.0) levelStr += L"E-";
				else if (Player::ins()->getTalentLevel(i) < 5.0) levelStr += L"E";
				else if (Player::ins()->getTalentLevel(i) < 6.0) levelStr += L"E+";
				else if (Player::ins()->getTalentLevel(i) < 7.0) levelStr += L"D-";
				else if (Player::ins()->getTalentLevel(i) < 8.0) levelStr += L"D";
				else if (Player::ins()->getTalentLevel(i) < 9.0) levelStr += L"D+";
				else if (Player::ins()->getTalentLevel(i) < 10.0) levelStr += L"C-";
				else if (Player::ins()->getTalentLevel(i) < 11.0) levelStr += L"C";
				else if (Player::ins()->getTalentLevel(i) < 12.0) levelStr += L"C+";
				else if (Player::ins()->getTalentLevel(i) < 13.0) levelStr += L"B-";
				else if (Player::ins()->getTalentLevel(i) < 14.0) levelStr += L"B";
				else if (Player::ins()->getTalentLevel(i) < 15.0) levelStr += L"B+";
				else if (Player::ins()->getTalentLevel(i) < 16.0) levelStr += L"A-";
				else if (Player::ins()->getTalentLevel(i) < 17.0) levelStr += L"A";
				else if (Player::ins()->getTalentLevel(i) < 18.0) levelStr += L"A+";
				else levelStr += L"S";

				std::wstring aptStr;
				aptStr = L"x" + decimalCutter(Player::ins()->getTalentApt(i), 1);

				switch (Player::ins()->getTalentFocus(i))
				{
					case 1:
						focusStr = L"+";
						break;
					case 2:
						focusStr = L"*";

						break;
					default:
						focusStr = L"-";
						break;
				}

				if (Player::ins()->getTalentLevel(i) >= 18) focusStr = L" ";

				switch (i)
				{
					case talentFlag::fighting:
						talentStr = sysStr[55];
						break;
					case talentFlag::dodging:
						talentStr = sysStr[56];
						break;
					case talentFlag::stealth:
						talentStr = sysStr[57];
						break;
					case talentFlag::throwing:
						talentStr = sysStr[58];
						break;
					case talentFlag::unarmedCombat:
						talentStr = sysStr[59];
						break;
					case talentFlag::piercingWeapon:
						talentStr = sysStr[60];
						break;
					case talentFlag::cuttingWeapon:
						talentStr = sysStr[61];
						break;
					case talentFlag::bashingWeapon:
						talentStr = sysStr[62];
						break;
					case talentFlag::archery:
						talentStr = sysStr[63];
						break;
					case talentFlag::gun:
						talentStr = sysStr[64];
						break;
					case talentFlag::electronics:
						talentStr = sysStr[65];
						break;
					case talentFlag::chemistry:
						talentStr = sysStr[66];
						break;
					case talentFlag::mechanics:
						talentStr = sysStr[67];
						break;
					case talentFlag::computer:
						talentStr = sysStr[68];
						break;
					case talentFlag::medicine:
						talentStr = sysStr[69];
						break;
					case talentFlag::cooking:
						talentStr = sysStr[70];
						break;
					case talentFlag::fabrication:
						talentStr = sysStr[71];
						break;
					case talentFlag::social:
						talentStr = sysStr[72];
						break;
					case talentFlag::architecture:
						talentStr = sysStr[73];
						break;
				}

				setFontSize(10);
				drawText(L"#FFFFFF" + focusStr, targetX + 30, targetY + 4);

				{
					SDL_Color rankColor = col::white;
					if (Player::ins()->getTalentLevel(i) >= 18) rankColor = col::yellow;
					drawText(col2Str(rankColor) + talentStr, targetX + 40, targetY + 4);
				}
				
				//재능 랭크와 게이지 그리기
				{
					SDL_Color rankColor = col::white;
					if (Player::ins()->getTalentLevel(i) >= 18) rankColor = col::yellow;
					drawText(col2Str(rankColor) + levelStr, targetX + 120, targetY + 4);

					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					SDL_Rect gauge = { targetX + 118, targetY + 18, 36, 3 };
					SDL_RenderDrawRect(renderer, &gauge);
					if (Player::ins()->getTalentLevel(i) < 18)
					{
						SDL_SetRenderDrawColor(renderer, col::yellowGreen.r, col::yellowGreen.g, col::yellowGreen.b, 0xff);
						float levelPercent = (Player::ins()->getTalentLevel(i) - (int)Player::ins()->getTalentLevel(i));
						int gaugeLength = floor(34.0 * levelPercent);
						if(gaugeLength>0) SDL_RenderDrawLine(renderer, targetX + 118 + 1, targetY + 18 + 1, targetX + 118 + gaugeLength, targetY + 18 + 1);
						int q = ((int)(levelPercent*100.0)) / 10;
						int r = ((int)(levelPercent * 100.0)) % 10;
						int qIndex = q + 27;
						int rIndex = r + 27;
						setZoom(1.0);
						if(q>0) drawSprite(spr::epsilonFont, qIndex, targetX + 118 + 37, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, rIndex, targetX + 118 + 37 + 4, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, 43, targetX + 118 + 37 + 8, targetY + 18 - 1); //%
						

					}
					else
					{
						SDL_SetRenderDrawColor(renderer, col::yellow.r, col::yellow.g, col::yellow.b, 0xff);
						SDL_RenderDrawLine(renderer, targetX + 118 + 1, targetY + 18 + 1, targetX + 118 + 34, targetY + 18 + 1);
						setZoom(1.0);
						drawSprite(spr::epsilonFont, 13, targetX + 118 + 37, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, 1, targetX + 118 + 37 + 4, targetY + 18 - 1);
						drawSprite(spr::epsilonFont, 24, targetX + 118 + 37 + 8, targetY + 18 - 1);
					}
				}

				drawText(L"#FFFFFF" + aptStr, targetX + 175, targetY + 4);

				//회색 필터
				if (Player::ins()->getTalentFocus(i) == 0 && Player::ins()->getTalentLevel(i) < 18)
				{
					drawStadium(talentButton[i].x, talentButton[i].y, talentButton[i].w, talentButton[i].h, btnColor, 150, 5);
				}

				//재능 아이콘 그리기
				setZoom(1.5);
				if (Player::ins()->getTalentLevel(i) < 18)
				{
					drawSprite(spr::talentIcon, i, targetX, targetY);
					if (Player::ins()->getTalentFocus(i) == 0)
					{
						SDL_Rect rect = { targetX,targetY,24,24 };
						SDL_SetRenderDrawColor(renderer, col::black.r, col::black.g, col::black.b, 150);
						SDL_RenderFillRect(renderer, &rect);
					}
				}
				else { drawSprite(spr::talentIconGold, i, targetX, targetY); }
				setZoom(1.0);



			}
			setZoom(1.0);

			if (warningIndex > 0) { drawText(L"#FF0000" + sysStr[74], talentBase.x + 20, talentBase.y + talentBase.h - 70); }
			drawText(L"#FFFFFF클릭하여 재능포인트의 분배 우선 순위를 결정합니다. 예로 3개의 재능을 우선분배하면 각 재능 당 경험치가 33%씩 쌓입니다.", talentBase.x + 20, talentBase.y + talentBase.h - 50);
			drawText(L"#FFFFFF적성이 높을 경우 레벨업에 필요한 경험치가 줄어들며 재능 레벨은 최대 18레벨까지 올릴 수 있습니다.", talentBase.x + 20, talentBase.y + talentBase.h - 30);
		}
		else
		{
			SDL_Rect vRect = talentBase;
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
			for (int i = 0; i < talentSize; i++)
			{
				if (Player::ins()->getTalentFocus(i) > 0) //수련 중인 재능을 1개라도 발견했을 경우
				{
					close(aniFlag::winUnfoldClose);
					return;
				}

				if (i == talentSize - 1) //만약 수련 중인 재능이 없으면
				{
					warningIndex = 1;
					return;
				}
			}
		}
		else
		{
			for (int i = 0; i < talentSize; i++)
			{
				if (checkCursor(&talentButton[i]))
				{
					if (Player::ins()->getTalentLevel(i) < 18)
					{
						switch (Player::ins()->getTalentFocus(i))
						{
							case 0:
								Player::ins()->setTalentFocus(i, 1);
								break;
							case 1:
								Player::ins()->setTalentFocus(i, 2);
								break;
							case 2:
								Player::ins()->setTalentFocus(i, 0);
								break;
							default:
								errorBox("Unvalid talent focus point");
								break;
						}
					}
				}
			}
		}
	}
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void step() { }

	void setWarningIndex(int inputVal) { warningIndex = inputVal; }
};