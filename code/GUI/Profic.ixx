module;
#include <SDL3/SDL.h>

export module Profic;

import std;
import util;
import GUI;
import constVar;
import globalVar;
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

	std::array<SDL_Rect, TALENT_SIZE> proficButton = { { 0,0,0,0 }, };
public:
	Profic() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}
	~Profic()
	{
		ptr = nullptr;
	}
	static Profic* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		proficBase = { 0, 0, 866, 502 };

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

		int pivotX = proficBase.x + 27;
		int pivotY = proficBase.y + 75;
		for (int i = 0; i < TALENT_SIZE; i++)
		{
			int targetX = pivotX + 280 * (i / 8);
			int targetY = pivotY + (37 * (i % 8));
			proficButton[i] = { targetX, targetY, 267, 32 };
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

            int pivotX = proficBase.x + 27;
            int pivotY = proficBase.y + 75;

            for (int i = 0; i <= 2; i++)
            {
                setFontSize(14);
                setFont(fontType::mainFontMedium);
                int targetX = pivotX + 13;
                int targetY = pivotY + (37 * (-1)) + 27;
                drawTextCenter(L"#234A63" + sysStr[182], targetX + 72 + (280 * i), targetY - 9); //이름
                drawTextCenter(L"#234A63" + sysStr[183], targetX + 167 + (280 * i), targetY - 9); //레벨
                drawTextCenter(L"#234A63" + sysStr[184], targetX + 232 + (280 * i), targetY - 9); //적성
                setFont(fontType::mainFont);
            }

            for (int i = 0; i < TALENT_SIZE; i++)
            {
                SDL_Color btnColor;
                if (checkCursor(&proficButton[i]))
                {
                    if (click == false) btnColor = lowCol::blue;
                    else btnColor = lowCol::deepBlue;
                }
                else btnColor = lowCol::black;
                drawStadium(proficButton[i].x, proficButton[i].y, proficButton[i].w, proficButton[i].h, btnColor, 255, 7);

                int targetX = pivotX + 280 * (i / 8);
                int targetY = pivotY + (37 * (i % 8));

                std::wstring focusStr;
                std::wstring proficStr;

                // 레벨 표시를 "Lv X.X" 형식으로 변경
                std::wstring levelStr = L"";
                float lv = PlayerPtr->getProficLevel(i);

                // 소수점 첫째자리까지 표시
                int wholePart = static_cast<int>(lv);
                int decimalPart = static_cast<int>((lv - wholePart) * 10);
                levelStr += std::to_wstring(wholePart) + L"." + std::to_wstring(decimalPart);

                std::wstring aptStr;
                if(PlayerPtr->entityInfo.proficApt[i]>0) aptStr = L"+";
                else if(PlayerPtr->entityInfo.proficApt[i]<0) aptStr += L"-";
                else aptStr += L" ";
                aptStr += std::to_wstring(PlayerPtr->entityInfo.proficApt[i]);

                switch (PlayerPtr->entityInfo.proficFocus[i])
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

                if (PlayerPtr->getProficLevel(i) >= MAX_PROFIC_LEVEL) focusStr = L" ";

                // 숙련도 이름 설정 (기존과 동일)
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
                case proficFlag::invocations:
                    proficStr = sysStr[295];
                    break;
                }

                setFontSize(14);
                drawText(focusStr, targetX + 40, targetY + 7);

                int yOffset = 0;
                {
                    SDL_Color levelColor = col::white;
                    if (PlayerPtr->getProficLevel(i) >= MAX_PROFIC_LEVEL) levelColor = col::yellow;
                    if (queryTextWidth(proficStr) > 93)
                    {
                        setFontSize(12);
                        yOffset = 2;
                    }
                    drawText(col2Str(levelColor) + proficStr, targetX + 53, targetY + 7 + yOffset);
                }

                setFontSize(14);
                // 레벨과 게이지 그리기
                {
                    SDL_Color levelColor = col::white;
                    if (PlayerPtr->getProficLevel(i) >= MAX_PROFIC_LEVEL) levelColor = col::yellow;
                    drawText(col2Str(levelColor) + levelStr , targetX + 193 - queryTextWidth(levelStr), targetY + 7);

                }

                drawText(L"#FFFFFF" + aptStr, targetX + 233, targetY + 7);

                // 회색 필터 (기존과 동일)
                if (PlayerPtr->entityInfo.proficFocus[i] == 0 && PlayerPtr->getProficLevel(i) < MAX_PROFIC_LEVEL)
                {
                    drawStadium(proficButton[i].x, proficButton[i].y, proficButton[i].w, proficButton[i].h, btnColor, 150, 7);
                }

                // 재능 아이콘 그리기
                setZoom(2.0);
                if (PlayerPtr->getProficLevel(i) < MAX_PROFIC_LEVEL)
                {
                    drawSprite(spr::proficIcon, i, targetX, targetY);
                    if (PlayerPtr->entityInfo.proficFocus[i] == 0)
                    {
                        SDL_Rect rect = { targetX,targetY,32,32 };
                        drawFillRect(rect, col::black, 150);
                    }
                }
                else {
                    drawSprite(spr::proficIconGold, i, targetX, targetY);
                }
                setZoom(1.0);
            }
            setZoom(1.0);

            setFontSize(15);
            drawTextWidth(sysStr[230]/*When gaining experience points for a specific proficiency through actions, the same amount of bonus experience points will be additionally distributed to the selected proficiencies.*/,
                proficBase.x + 27, proficBase.y + proficBase.h - 56, false, 790, 19);

            {
                bool anyTraining = false;
                for (int i = 0; i < TALENT_SIZE; i++)
                {
                    if (PlayerPtr->entityInfo.proficFocus[i] > 0) { anyTraining = true; break; }
                }
                if (!anyTraining)
                {
                    setFontSize(16);
                    drawText(col2Str(lowCol::red) + sysStr[74]/*You must train at least one proficiency.*/, proficBase.x + 27, proficBase.y + proficBase.h - 88);
                }
            }

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
			for (int i = 0; i < TALENT_SIZE; i++)
			{
				if (PlayerPtr->entityInfo.proficFocus[i] > 0) //수련 중인 재능을 1개라도 발견했을 경우
				{
					close(aniFlag::winUnfoldClose);
					return;
				}
			}
			//수련 중인 재능이 없으면 닫기 차단 (경고는 drawGUI에서 실시간 표시)
			return;
		}
		else
		{
			for (int i = 0; i < TALENT_SIZE; i++)
			{
				if (checkCursor(&proficButton[i]))
				{
					if (PlayerPtr->getProficLevel(i) < MAX_PROFIC_LEVEL)
					{
						switch (PlayerPtr->entityInfo.proficFocus[i])
						{
						case 0:
							PlayerPtr->entityInfo.proficFocus[i] = 1;
							break;
						case 1:
							PlayerPtr->entityInfo.proficFocus[i] = 2;
							break;
						case 2:
							PlayerPtr->entityInfo.proficFocus[i] = 0;
							break;
						default:
							errorBox(L"Unvalid profic focus point");
							break;
						}
					}
				}
			}
		}
	}


    void step()
    {
        tabType = tabFlag::back;
    }
};