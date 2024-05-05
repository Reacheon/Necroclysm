#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();


export module Equip;

import std;
import util;
import Player;
import drawStadium;
import drawText;
import globalVar;
import textureVar;
import ItemPocket;
import checkCursor;
import drawSprite;
import drawItemList;
import drawWindowArrow;
import GUI;
import log;
import Msg;
import actFuncSet;
import drawWindow;
import CoordSelect;
import Lst;

export class Equip : public GUI
{
private:
	inline static Equip* ptr = nullptr;
	ItemPocket* equipPtr = Player::ins()->getEquipPtr();

	int equipScroll = 0; //좌측 장비창의 스크롤
	int equipCursor = -1; //좌측 장비창의 커서
	const int equipScrollSize = 8;

	ItemPocket* lootPtr = nullptr;
	const int lootScrollSize = 6; //한 스크롤에 들어가는 아이템의 수
	int lootScroll = 0; //우측 루팅창의 스크롤
	int lootCursor = -1; //우측 루팅창의 커서
	int pocketCursor = -1; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	//모션 스크롤 이벤트에서 기준으로 잡는 최초 터치 당시의 스크롤
	int initLootScroll = 0;
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;

	sortFlag sortType = sortFlag::null; //0:기본_1:무게내림_2:무게올림_3:부피내림_4:부피올림

	bool isTargetPocket = false; //우측의 포켓창이 조작 타겟일 경우

	SDL_Rect equipBase;
	SDL_Rect equipTitle;
	SDL_Rect equipItem[30];
	SDL_Rect equipItemSelect[30];
	SDL_Rect equipLabel;
	SDL_Rect equipLabelSelect;
	SDL_Rect equipLabelName;
	SDL_Rect equipLabelQuantity;
	SDL_Rect equipArea;
	SDL_Rect equipScrollBox;
	SDL_Rect equipWindow;

	SDL_Rect lootBase;
	SDL_Rect lootTitle;
	SDL_Rect lootItem[30];
	SDL_Rect lootItemSelect[30];
	SDL_Rect lootLabel;
	SDL_Rect lootLabelSelect;
	SDL_Rect lootLabelName;
	SDL_Rect lootLabelQuantity;
	SDL_Rect lootArea;
	SDL_Rect lootScrollBox;
	SDL_Rect lootWindow;
	SDL_Rect pocketWeight;
	SDL_Rect pocektVolume;
	SDL_Rect pocketWindow;
	SDL_Rect pocketItem[7];
	SDL_Rect pocketArea;
	SDL_Rect pocketScrollBox;
	SDL_Rect pocketLeft;
	SDL_Rect pocketRight;
	SDL_Rect lootBtn;

	SDL_Rect topWindow;//상단에 표시되는 저항이나 방어 상성, 아이템의 설명
public:
	Equip() : GUI(false)
	{
		errorBox(ptr != nullptr, "More than equip instance was generated.");
		ptr = this;

		changeXY((cameraW / 2) - 352, (cameraH / 2) - 210, false);
		setAniSlipDir(4);

		//barAct = actSet::null;
		tabType = tabFlag::closeWin;
		UIType = act::equip;

		deactInput();
		deactDraw();
		setAniType(aniFlag::winSlipOpen);
		aniUSet.insert(this);
		turnCycle = turn::playerAnime;

		if (inputType == input::keyboard)
		{
			equipCursor = 0;
		}
	}
	~Equip()
	{
		prt(L"Equip : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		equipCursor = -1;
		equipScroll = 0;
		barAct = actSet::null;
		tabType = tabFlag::autoAtk;
	}
	static Equip* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		equipBase = { 0,0,335,420 };
		if (center == false)
		{
			equipBase.x += inputX;
			equipBase.y += inputY;
		}
		else
		{
			equipBase.x += inputX - equipBase.w / 2;
			equipBase.y += inputY - equipBase.h / 2;
		}

		equipTitle = { equipBase.x + 103, equipBase.y, 130, 30 };
		equipWindow = { equipBase.x, equipBase.y + 30, 335, 380 };
		equipLabel = { equipWindow.x + 10, equipWindow.y + 10, equipWindow.w - 20 , 26 };
		equipLabelSelect = { equipLabel.x, equipLabel.y, 62 , 26 };
		equipLabelName = { equipLabel.x + equipLabelSelect.w, equipLabel.y, 182 , 26 };
		equipLabelQuantity = { equipLabel.x + equipLabelName.w + equipLabelSelect.w, equipLabel.y, 71 , 26 };
		equipArea = { equipWindow.x + 10, equipWindow.y + 40,312, 42 * 8 - 6 };
		for (int i = 0; i < equipItemMax; i++)
		{
			equipItem[i] = { equipArea.x + 42, equipArea.y + 42 * i, 270, 36 };
			equipItemSelect[i] = { equipArea.x, equipArea.y + 42 * i, 36, 36 };
		}
		equipScrollBox = { equipWindow.x + 328, equipWindow.y + 40, 2, 42 * equipItemMax };

		//////////////////////////////////////////////////////////////////////////////////////////////////

		int lootPivotX = (cameraW / 2) + 17;
		int lootPivotY = (cameraH / 2) - 210;
		lootBase = { 0,0,335,420 };
		if (center == false)
		{
			lootBase.x += lootPivotX;
			lootBase.y += lootPivotY;
		}
		else
		{
			lootBase.x += lootPivotX - lootBase.w / 2;
			lootBase.y += lootPivotY - lootBase.h / 2;
		}
		lootTitle = { lootBase.x + 102, lootBase.y + 0, 130, 30 };
		lootWindow = { lootBase.x + 0, lootBase.y + 120, 335, 300 };
		lootArea = { lootWindow.x + 10, lootWindow.y + 40,312, 246 };
		for (int i = 0; i < lootItemMax; i++)
		{
			lootItem[i] = { lootArea.x + 42, lootArea.y + 42 * i, 270, 36 };
			lootItemSelect[i] = { lootArea.x, lootArea.y + 42 * i, 236, 36 };
		}
		lootLabel = { lootWindow.x + 10, lootWindow.y + 10, lootWindow.w - 20 , 26 };
		lootLabelSelect = { lootLabel.x, lootLabel.y, 62 , 26 };
		lootLabelName = { lootLabel.x + lootLabelSelect.w, lootLabel.y, 182 , 26 };
		lootLabelQuantity = { lootLabel.x + lootLabelName.w + lootLabelSelect.w, lootLabel.y, 71 , 26 };
		lootScrollBox = { lootWindow.x + 328, lootWindow.y + 40, 2, 42 * lootItemMax };
		pocketWindow = { lootBase.x + 0, lootBase.y + 34, 335, 70 };
		pocketWeight = { pocketWindow.x + 12, pocketWindow.y + 64, 72, 4 };
		pocektVolume = { pocketWindow.x + pocketWindow.w - 12 - 72, pocketWindow.h + 64, 72, 4 };
		pocketItem[0] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38 * 3,pocketWindow.y + 11,32,32 };
		pocketItem[1] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38 * 2,pocketWindow.y + 11,32,32 };
		pocketItem[2] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38,pocketWindow.y + 11,32,32 };
		pocketItem[3] = { pocketWindow.x + (pocketWindow.w / 2) - 24,pocketWindow.y + 11 - 8,48,48 };
		pocketItem[4] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38,pocketWindow.y + 11,32,32 };
		pocketItem[5] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38 * 2,pocketWindow.y + 11,32,32 };
		pocketItem[6] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38 * 3,pocketWindow.y + 11,32,32 };
		pocketLeft = { pocketWindow.x + 4,pocketWindow.y + 6, 24,44 };
		pocketRight = { pocketWindow.x + pocketWindow.w - pocketLeft.w - 4,pocketWindow.y + 6,24,44 };
		lootBtn = { lootWindow.x + lootWindow.w / 2 - 28, lootWindow.y - 2 - 16 + 2, 56, 24 };

		topWindow = { 0, 0, 410,140 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - equipBase.w / 2;
			y = inputY - equipBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		drawWindow(&equipBase, sysStr[13], 2);

		//여기서부턴 이큅 윈도우
		{
			ItemPocket* equipPtr = Player::ins()->getEquipPtr();
			//이큅 윈도우 본체
			//drawStadium(equipWindow.x, equipWindow.y, equipWindow.w, equipWindow.h, { 0,0,0 }, 150, 5);
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			setFontSize(10);
			drawText(std::to_wstring(equipCursor + 1) + L"/" + std::to_wstring(equipPtr->itemInfo.size()), equipWindow.x + 6, equipWindow.y + equipWindow.h - 16);

			//우측 아이템 상단바(선택 이름 물리량)
			drawStadium(equipLabel.x, equipLabel.y, equipLabel.w, equipLabel.h, { 0,0,0 }, 150, 5);
			if (checkCursor(&equipLabelSelect))
			{
				drawStadium(equipLabelSelect.x, equipLabelSelect.y, equipLabelSelect.w, equipLabelSelect.h, lowCol::blue, 150, 5);
			}
			else if (checkCursor(&equipLabelName))
			{
				drawStadium(equipLabelName.x, equipLabelName.y, equipLabelName.w, equipLabelName.h, lowCol::blue, 150, 5);
			}
			else if (checkCursor(&equipLabelQuantity))
			{
				drawStadium(equipLabelQuantity.x, equipLabelQuantity.y, equipLabelQuantity.w, equipLabelQuantity.h, lowCol::blue, 150, 5);
			}

			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			setFontSize(13);
			drawText(sysStr[15], equipLabel.x + 10, equipLabel.y + 4); //선택(상단바)
			drawText(sysStr[16], equipLabel.x + 140, equipLabel.y + 4); //이름(상단바)
			drawText(sysStr[24], equipLabel.x + 250, equipLabel.y + 4); //무리량(상단바)

			//개별 아이템
			drawItemList(equipPtr, equipArea.x, equipArea.y, equipItemMax, equipCursor, equipScroll, isTargetPocket == false);

			if (equipPtr->itemInfo.size() == 0) // 만약 아이템이 없을 경우
			{
				drawTextCenter(col2Str(col::white) + sysStr[90], equipArea.x + equipArea.w / 2, equipArea.y + equipArea.h / 2);
			}

			// 아이템 스크롤 그리기
			SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
			SDL_RenderFillRect(renderer, &equipScrollBox);
			SDL_Rect inScrollBox = { equipWindow.x + 328, equipWindow.y + 40, 2, 42 * equipItemMax }; // 내부 스크롤 커서
			inScrollBox.h = equipScrollBox.h * myMin(1.0, (double)equipItemMax / equipPtr->itemInfo.size());
			inScrollBox.y = equipScrollBox.y + equipScrollBox.h * ((float)equipScroll / (float)equipPtr->itemInfo.size());
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &inScrollBox);
		}



		if (aniUSet.find(this) == aniUSet.end())
		{
			drawStadium(topWindow.x, topWindow.y, topWindow.w, topWindow.h, { 0,0,0 }, 180, 5);

			setFontSize(14);

			SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);

			drawText(L"HEAD", topWindow.x + 10, topWindow.y + 24 + 18 * 0);
			drawText(L"TORSO", topWindow.x + 10, topWindow.y + 24 + 18 * 1);
			drawText(L"LARM", topWindow.x + 10, topWindow.y + 24 + 18 * 2);
			drawText(L"RARM", topWindow.x + 10, topWindow.y + 24 + 18 * 3);
			drawText(L"LLEG", topWindow.x + 10, topWindow.y + 24 + 18 * 4);
			drawText(L"RLEG", topWindow.x + 10, topWindow.y + 24 + 18 * 5);

			SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 0xff);

			drawTextCenter(L"rPierce", topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * -1 + 9);
			drawTextCenter(L"rCut", topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * -1 + 9);
			drawTextCenter(L"rBash", topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * -1 + 9);
			drawTextCenter(L"Enc", topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * -1 + 9);

			for (int i = 0; i < 6; i++)
			{
				int rPierce, rCut, rBash, enc, maxEnc;
				int targetPart = partType::head;
				switch (i)
				{
					default:
						targetPart = partType::head;
						break;
					case 1:
						targetPart = partType::torso;
						break;
					case 2:
						targetPart = partType::larm;
						break;
					case 3:
						targetPart = partType::rarm;
						break;
					case 4:
						targetPart = partType::lleg;
						break;
					case 5:
						targetPart = partType::rleg;
						break;
				}

				rPierce = Player::ins()->getRPierce(targetPart);
				rCut = Player::ins()->getRCut(targetPart);
				rBash = Player::ins()->getRBash(targetPart);
				enc = Player::ins()->getEnc(targetPart);
				maxEnc = 30;

				SDL_SetRenderDrawColor(renderer, col::white.r, col::white.g, col::white.b, 0xff);
				drawTextCenter(std::to_wstring(rPierce), topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * i + 9);
				drawTextCenter(std::to_wstring(rCut), topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * i + 9);
				drawTextCenter(std::to_wstring(rBash), topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * i + 9);
				SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);
				drawTextCenter(std::to_wstring(enc) + L"/" + std::to_wstring(maxEnc), topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * i + 9);
			}


			SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 0xff);

			drawText(L"Shield", topWindow.x + 290, topWindow.y + 24 + 18 * -1);
			drawText(L"Evade", topWindow.x + 290, topWindow.y + 24 + 18 * 0);

			SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);

			drawText(L"rFire", topWindow.x + 290, topWindow.y + 24 + 18 * 1);
			drawText(L"rCold", topWindow.x + 290, topWindow.y + 24 + 18 * 2);
			drawText(L"rElec", topWindow.x + 290, topWindow.y + 24 + 18 * 3);
			drawText(L"rNeg", topWindow.x + 290, topWindow.y + 24 + 18 * 4);
			drawText(L"rCorr", topWindow.x + 290, topWindow.y + 24 + 18 * 5);

			SDL_SetRenderDrawColor(renderer, col::white.r, col::white.g, col::white.b, 0xff);

			int SH = Player::ins()->getSH();
			int EV = Player::ins()->getEV();

			drawTextCenter(std::to_wstring(SH), topWindow.x + 290 + 70 + 20, topWindow.y + 24 + 18 * -1 + 9);
			drawTextCenter(std::to_wstring(EV), topWindow.x + 290 + 70 + 20, topWindow.y + 24 + 18 * 0 + 9);

			SDL_SetRenderDrawColor(renderer, lowCol::green.r, lowCol::green.g, lowCol::green.b, 0xff);

			int rFire = Player::ins()->entityInfo.rFire;
			int rCold = Player::ins()->entityInfo.rCold;
			int rElec = Player::ins()->entityInfo.rElec;
			int rCorr = Player::ins()->entityInfo.rCorr;
			int rRad = Player::ins()->entityInfo.rRad;

			drawText(L"Lv." + std::to_wstring(rFire), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 1);
			drawText(L"Lv." + std::to_wstring(rCold), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 2);
			drawText(L"Lv." + std::to_wstring(rElec), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 3);
			drawText(L"Lv." + std::to_wstring(rCorr), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 4);
			drawText(L"Lv." + std::to_wstring(rRad), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 5);

			//우측 포켓창 그리기
			if (pocketCursor >= 0)
			{
				const Uint8* state = SDL_GetKeyboardState(NULL);
				Sprite* targetBtnSpr = nullptr;
				
				drawWindow(&lootBase, sysStr[10], 1);

				//가방이 몇 개 있는지 체크
				std::vector<int> pocketList;
				int numberOfBag = 0;
				ItemPocket* equipPtr = Player::ins()->getEquipPtr();
				for (int i = 0; i < equipPtr->itemInfo.size(); i++)
				{
					if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
					{
						pocketList.push_back(i);
						numberOfBag++;
					}
				}

				if (numberOfBag == 0)
				{
					//가방을 가지고 있지 않다.
					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					setFontSize(16);
					drawTextCenter(sysStr[19], pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + 35);
				}
				else
				{
					//포켓 1~3번째 칸 그리기
					if (pocketCursor != 0)
					{
						setZoom(2.0);
						drawStadium(pocketItem[2].x, pocketItem[2].y, pocketItem[2].w, pocketItem[2].h, { 0,0,0 }, 200, 5);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 1]].sprIndex, pocketItem[2].x + (pocketItem[2].w / 2), pocketItem[2].y + (pocketItem[2].h / 2));
						if (pocketCursor != 1)
						{
							drawStadium(pocketItem[1].x, pocketItem[1].y, pocketItem[1].w, pocketItem[1].h, { 0,0,0 }, 200, 5);
							drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 2]].sprIndex, pocketItem[1].x + (pocketItem[1].w / 2), pocketItem[1].y + (pocketItem[1].h / 2));
							if (pocketCursor != 2)
							{
								drawStadium(pocketItem[0].x, pocketItem[0].y, pocketItem[0].w, pocketItem[0].h, { 0,0,0 }, 200, 5);
								drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 3]].sprIndex, pocketItem[0].x + (pocketItem[0].w / 2), pocketItem[0].y + (pocketItem[0].h / 2));
							}
						}
					}

					//포켓 4번째 칸
					setZoom(3.0);
					drawStadium(pocketItem[3].x, pocketItem[3].y, pocketItem[3].w, pocketItem[3].h, lowCol::blue, 200, 5);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor]].sprIndex, pocketItem[3].x + (pocketItem[3].w / 2), pocketItem[3].y + (pocketItem[3].h / 2));

					//포켓 5~7번째 칸 
					if (pocketCursor != numberOfBag - 1)
					{
						setZoom(2.0);
						drawStadium(pocketItem[4].x, pocketItem[4].y, pocketItem[4].w, pocketItem[4].h, { 0,0,0 }, 200, 5);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 1]].sprIndex, pocketItem[4].x + (pocketItem[4].w / 2), pocketItem[4].y + (pocketItem[4].h / 2));
						if (pocketCursor != numberOfBag - 2)
						{
							drawStadium(pocketItem[5].x, pocketItem[5].y, pocketItem[5].w, pocketItem[5].h, { 0,0,0 }, 200, 5);
							drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 2]].sprIndex, pocketItem[5].x + (pocketItem[5].w / 2), pocketItem[5].y + (pocketItem[5].h / 2));
							if (pocketCursor != numberOfBag - 3)
							{
								drawStadium(pocketItem[6].x, pocketItem[6].y, pocketItem[6].w, pocketItem[6].h, { 0,0,0 }, 200, 5);
								drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 3]].sprIndex, pocketItem[6].x + (pocketItem[6].w / 2), pocketItem[6].y + (pocketItem[6].h / 2));
							}
						}
					}
					setZoom(1.0);


					//포켓 질량 게이지
					SDL_Rect weightBar = { pocketWindow.x + 12, pocketWindow.y + 64, 72, 4 };
					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					SDL_RenderDrawRect(renderer, &weightBar);

					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					setFontSize(10);
					drawTextCenter(L"132.9/99.9 KG", weightBar.x + (weightBar.w / 2), weightBar.y - 8);

					//루팅 주머니 부피 게이지
					SDL_Rect volumeBar = { pocketWindow.x + pocketWindow.w - 12 - 72, pocketWindow.y + 64, 72, 4 };
					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					SDL_RenderDrawRect(renderer, &volumeBar);
					SDL_SetRenderDrawColor(renderer, lowCol::green.r, lowCol::green.g, lowCol::green.b, 0xff);
					SDL_Rect volumeGauge = { volumeBar.x + 1, volumeBar.y + 1, volumeBar.w - 2, 2 };

					int maxVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketMaxVolume;
					int currentVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketVolume;
					volumeGauge.w = (volumeBar.w - 2) * ((float)currentVolume / (float)maxVolume);
					SDL_RenderFillRect(renderer, &volumeGauge);
					std::wstring volumeStr = decimalCutter(currentVolume / 1000.0, 2) + L"/" + decimalCutter(maxVolume / 1000.0, 2) + L" L";

					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					setFontSize(10);
					drawTextCenter(volumeStr, volumeBar.x + (volumeBar.w / 2), volumeBar.y - 8);
					setFontSize(13);
					drawTextCenter(L"테스트 아이템", pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + pocketWindow.h - 10);




					{
						SDL_Color leftBtnColor;
						if (checkCursor(&pocketLeft))
						{
							if (click == true) { leftBtnColor = lowCol::deepBlue; }
							else { leftBtnColor = lowCol::blue; }
						}
						else { leftBtnColor = lowCol::black; }
						drawStadium(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, leftBtnColor, 200, 5);
						drawSpriteCenter(spr::windowArrow, 2, pocketWindow.x + 16, pocketWindow.y + 27);

						if (inputType == input::keyboard)
						{
							if (state[SDL_SCANCODE_LSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
							else { targetBtnSpr = spr::buttons; }
							drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_LShift, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
						}

						if (pocketCursor == 0) { drawStadium(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, leftBtnColor, 200, 5); }

					}
					{
						SDL_Color rightBtnColor;
						if (checkCursor(&pocketRight))
						{
							if (click == true) { rightBtnColor = lowCol::deepBlue; }
							else { rightBtnColor = lowCol::blue; }
						}
						else { rightBtnColor = lowCol::black; }
						drawStadium(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, rightBtnColor, 200, 5);
						drawSpriteCenter(spr::windowArrow, 0, pocketWindow.x + pocketWindow.w - 16, pocketWindow.y + 27);

						if (inputType == input::keyboard)
						{
							if (state[SDL_SCANCODE_RSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
							else { targetBtnSpr = spr::buttons; }
							drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_RShift, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
						}

						if (pocketCursor == numberOfBag - 1) { drawStadium(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, rightBtnColor, 200, 5); }
					}
				}

				//여기서부턴 루팅 윈도우
				{
					//루팅 윈도우 본체
					//drawStadium(lootWindow.x, lootWindow.y, lootWindow.w, lootWindow.h, { 0,0,0 }, 150, 5);
					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					setFontSize(10);
					drawText(std::to_wstring(lootCursor + 1) + L"/" + std::to_wstring(lootPtr->itemInfo.size()), lootWindow.x + 6, lootWindow.y + lootWindow.h - 16);

					//우측 아이템 상단바 라벨(선택 이름 물리량)
					drawStadium(lootLabel.x, lootLabel.y, lootLabel.w, lootLabel.h, { 0,0,0 }, 150, 5);
					if (checkCursor(&lootLabelSelect) || labelCursor == 0)
					{
						SDL_Color btnColor;
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						drawStadium(lootLabelSelect.x, lootLabelSelect.y, lootLabelSelect.w, lootLabelSelect.h, btnColor, 150, 5);
					}
					else if (checkCursor(&lootLabelName) || labelCursor == 1)
					{
						SDL_Color btnColor;
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						drawStadium(lootLabelName.x, lootLabelName.y, lootLabelName.w, lootLabelName.h, btnColor, 150, 5);
					}
					else if (checkCursor(&lootLabelQuantity) || labelCursor == 2)
					{
						SDL_Color btnColor;
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						drawStadium(lootLabelQuantity.x, lootLabelQuantity.y, lootLabelQuantity.w, lootLabelQuantity.h, btnColor, 150, 5);
					}

					SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
					setFontSize(13);
					drawTextCenter(sysStr[15], lootLabel.x + 32, lootLabel.y + 12); //선택(상단바)

					{ //이름(상단바)
						std::wstring tailStr = L"";
						int grayNumber = 0;
						int whiteNumber = 0;
						//이미 검색 중인지 체크
						for (int i = 0; i < lootPtr->itemInfo.size(); i++)
						{
							if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
							{
								grayNumber++;
							}
						}

						if (grayNumber > 0)
						{
							whiteNumber = lootPtr->itemInfo.size() - grayNumber;
							tailStr = L"(" + std::to_wstring(whiteNumber) + sysStr[87] + L")";// n개 아이템 검색됨
						}

						drawTextCenter(sysStr[16] + tailStr, lootLabel.x + 152, lootLabel.y + 12);
					}

					switch (getSortType())
					{
						default:
							drawTextCenter(sysStr[24], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
							break;
						case sortFlag::weightDescend:
							drawTextCenter(sysStr[45], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
							break;
						case sortFlag::weightAscend:
							drawTextCenter(sysStr[46], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
							break;
						case sortFlag::volumeDescend:
							drawTextCenter(sysStr[47], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
							break;
						case sortFlag::volumeAscend:
							drawTextCenter(sysStr[48], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
							break;
					}


					//개별 아이템
					drawItemList(lootPtr, lootArea.x, lootArea.y, lootItemMax, lootCursor, lootScroll, true);

					if (lootPtr->itemInfo.size() == 0) // 만약 아이템이 없을 경우
					{
						setFontSize(13);
						drawTextCenter(col2Str(col::white) + sysStr[89], lootArea.x + lootArea.w / 2, lootArea.y + lootArea.h / 2);
					}

					// 아이템 스크롤 그리기
					SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
					SDL_RenderFillRect(renderer, &lootScrollBox);
					SDL_Rect inScrollBox = { lootWindow.x + 328, lootWindow.y + 40, 2, 42 * lootItemMax }; // 내부 스크롤 커서
					inScrollBox.h = lootScrollBox.h * myMin(1.0, (double)lootItemMax / lootPtr->itemInfo.size());
					inScrollBox.y = lootScrollBox.y + lootScrollBox.h * ((float)lootScroll / (float)lootPtr->itemInfo.size());
					if (inScrollBox.y + inScrollBox.h > lootScrollBox.y + lootScrollBox.h) { inScrollBox.y = lootScrollBox.y + lootScrollBox.h - inScrollBox.h; }
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_RenderFillRect(renderer, &inScrollBox);

					//루팅(버리기)버튼 그리기
					{
						SDL_Rect bridgeRect = { pocketWindow.x + pocketWindow.w / 2 - 36, pocketWindow.y + pocketWindow.h, 72,10 };
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
						SDL_RenderFillRect(renderer, &bridgeRect);

						SDL_Color lootBtnColor;
						if (checkCursor(&lootBtn))
						{
							if (click == true) { lootBtnColor = lowCol::deepBlue; }
							else { lootBtnColor = lowCol::blue; }
						}
						else { lootBtnColor = lowCol::black; }

						SDL_SetRenderDrawColor(renderer, lootBtnColor.r, lootBtnColor.g, lootBtnColor.b, 200);
						SDL_RenderFillRect(renderer, &lootBtn);
						SDL_SetRenderDrawColor(renderer, 0x57, 0x57, 0x57, 255);
						SDL_RenderDrawRect(renderer, &lootBtn);

						setZoom(0.7);
						drawSpriteCenter(spr::icon32, 0, lootWindow.x + lootWindow.w / 2, lootWindow.y - 2);
						setZoom(1.0);

						for (int i = 0; i < lootPtr->itemInfo.size(); i++)
						{
							if (lootPtr->itemInfo[i].lootSelect > 0)
							{
								break;
							}
							if (i == lootPtr->itemInfo.size() - 1)
							{
								drawStadium(lootBtn.x, lootBtn.y, lootBtn.w, lootBtn.h, lootBtnColor, 200, 5);
							}
						}

						if (lootPtr->itemInfo.size() == 0)
						{
							drawStadium(lootBtn.x, lootBtn.y, lootBtn.w, lootBtn.h, lootBtnColor, 200, 5);
						}

						if (inputType == input::keyboard)
						{
							if (state[SDL_SCANCODE_V]) { targetBtnSpr = spr::buttonsPressed; }
							else { targetBtnSpr = spr::buttons; }
							drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_V, lootBtn.x, lootBtn.y + lootBtn.h / 2);
						}
					}
				}
			}
		}
	}
	void clickUpGUI()
	{
		if (checkCursor(&tab) == true)// 탭박스
		{
			executeTab();
			return;
		}
		else if (checkCursor(&equipArea)) //아이템 클릭 -> 에러 파트
		{
			//아이템 메인 클릭
			//만약 아이템을 클릭했으면 커서를 그 아이템으로 옮김, 다른 곳 누르면 -1로 바꿈
			for (int i = 0; i < equipItemMax; i++)
			{
				if (equipPtr->itemInfo.size() - 1 >= i)
				{
					if (checkCursor(&equipItem[i]))
					{
						if (equipCursor != equipScroll + i) //새로운 커서 생성
						{
							equipCursor = equipScroll + i;
							updateBarAct();
							tabType = tabFlag::back;
						}
						else //커서 삭제
						{
							equipCursor = -1;
							barAct = actSet::null;
							tabType = tabFlag::closeWin;
						}
						return;
					}
				}
			}
		}
		else if (checkCursor(&lootBase))
		{
			if (checkCursor(&lootArea)) //아이템 클릭 -> 에러 파트
			{
				//만약 아이템을 클릭했으면 커서를 그 아이템으로 옮김, 다른 곳 누르면 -1로 바꿈
				for (int i = 0; i < lootItemMax; i++)
				{
					if (lootPtr->itemInfo.size() - 1 >= i)
					{
						if (checkCursor(&lootItem[i]))
						{
							if (lootCursor != lootScroll + i) //새로운 커서 생성
							{
								lootCursor = lootScroll + i;
								updateBarAct();
								tabType = tabFlag::back;
							}
							else //커서 삭제
							{
								lootCursor = -1;
								barAct = actSet::null;
								tabType = tabFlag::closeWin;
							}
							return;
						}
					}
				}

				//아이템 좌측 셀렉트 클릭
				for (int i = 0; i < lootItemMax; i++)
				{
					if (checkCursor(&lootItemSelect[i]))
					{
						if (lootPtr->itemInfo.size() - 1 >= i)
						{
							if (lootPtr->itemInfo[i + lootScroll].lootSelect == 0)
							{
								if (inputType == input::mouse)
								{
									if (event.button.button == SDL_BUTTON_LEFT)
									{
										executeSelectItem(i + lootScroll);
									}
									else if (event.button.button == SDL_BUTTON_RIGHT)
									{
										CORO(executeSelectItemEx(pocketCursor, i + lootScroll));
									}
								}
								else if (inputType == input::touch)
								{
									executeSelectItem(i + lootScroll);
								}
							}
							else
							{
								lootPtr->itemInfo[i + lootScroll].lootSelect = 0;
							}
						}
					}
				}
			}
			else if (checkCursor(&lootLabel))
			{
				if (checkCursor(&lootLabelSelect))
				{
					executeSelectAll();
				}
				else if (checkCursor(&lootLabelName))
				{
					CORO(executeSearch());
					//lootPtr->sortPocket(sortFlag::null);
					//lootScroll = 0;
				}
				else if (checkCursor(&lootLabelQuantity))
				{
					executeSort();
				}
			}
			else if (checkCursor(&pocketLeft) || checkCursor(&pocketRight)) // 포켓 변경 버튼(좌우) 클릭
			{
				if (checkCursor(&pocketLeft)) //왼쪽 포켓으로
				{
					executePocketLeft();
				}
				else //오른쪽 포켓으로
				{
					executePocketRight();
				}
			}
			else if (checkCursor(&lootBtn))
			{
				executePickDrop();
			}
		}
		else if (checkCursor(&letterbox)) //버튼은 return 없음
		{
			for (int i = 0; i < barAct.size(); i++) // 하단 UI 터치 이벤트
			{
				if (checkCursor(&barButton[i]))
				{
					switch (barAct[i])
					{
						case act::equip://장비
							executeEquip();
							break;
						case act::wield://들기
							CORO(executeWield());
							break;
						case act::droping:
						{
							executeDroping();
							break;
						}
						case act::throwing:
						{
							if (lootCursor != -1)
							{
								CORO(executeThrowing(lootPtr, lootCursor));
							}
							else if (equipCursor != -1)
							{
								CORO(executeThrowing(equipPtr, equipCursor));
							}

							break;
						}
						case act::open:
						{
							executeOpen();
							break;
						}
						case act::reload:
						{
							CORO(executeReload());
							break;
						}
						case act::reloadBulletToMagazine:
						case act::reloadBulletToGun:
							if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::MAGAZINE))
							{
								CORO(actFunc::reloadSelf(actEnv::Equip, equipPtr, equipCursor));
							}
							else if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::AMMO))
							{
								CORO(actFunc::reloadOther(actEnv::Equip, equipPtr, equipCursor));
							}
							else if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::GUN))
							{
								CORO(actFunc::reloadSelf(actEnv::Equip, equipPtr, equipCursor));
							}
							break;
						case act::reloadMagazine:
							//총에서 사용하는 경우와 탄창에서 사용하는 경우가 다름
							//총에서 사용하면 자기 자신에게 장전함(self)
							//탄창에 사용하면 다른 타일의 총에게 장비함
							if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::MAGAZINE))
							{
								CORO(actFunc::reloadOther(actEnv::Equip, equipPtr, equipCursor));
							}
							else
							{
								CORO(actFunc::reloadSelf(actEnv::Equip, equipPtr, equipCursor));
							}
							break;
						case act::unloadMagazine:
						case act::unloadBulletFromMagazine:
						case act::unloadBulletFromGun:
						{
							if (equipCursor != -1)
							{
								actFunc::unload(equipPtr, equipCursor);
							}
							else
							{
								actFunc::unload(lootPtr, lootCursor);
							}
						}
					}

					if (equipPtr->itemInfo.size() == 0)
					{
						close(aniFlag::null);
						return;
					}

					if (equipPtr->itemInfo.size() - 1 <= equipScroll + equipScrollSize)
					{
						equipScroll = equipPtr->itemInfo.size() - equipScrollSize;
						if (equipScroll < 0) { equipScroll = 0; }
					}
				}
			}
		}

		//위의 모든 경우에서 return을 받지 못했으면 버튼 이외를 누른 것이므로 커서를 -1로 복구
		{
			lootCursor = -1;
			equipCursor = -1;
			barAct = actSet::null;
			tabType = tabFlag::closeWin;
		}
	}
	void clickMotionGUI(int dx, int dy)
	{
		if (checkCursor(&lootArea))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // 가속상수, 작아질수록 스크롤 속도가 빨라짐
				//lootScroll = initLootScroll + dy / scrollAccelConst;
				if (abs(dy / scrollAccelConst) >= 1)
				{
					deactClickUp = true;
					cursorMotionLock = true;
				}
			}
		}
	}
	void clickDownGUI()
	{
		//아이템 좌측 셀렉트 클릭
		selectTouchTime = SDL_GetTicks();
		initLootScroll = lootScroll;
		initPocketCursor = pocketCursor;
	}
	void step()
	{
		//셀렉트 홀드 이벤트
		if (coFunc == nullptr)
		{
			if (selectTouchTime != -1)
			{
				//아이템 좌측 셀렉트 클릭
				for (int i = 0; i < lootItemMax; i++)
				{
					if (checkCursor(&lootItemSelect[i]))
					{
						if (lootPtr->itemInfo.size() - 1 >= i)
						{
							if (SDL_GetTicks() - selectTouchTime > 800)
							{
								selectTouchTime = -1;
								CORO(executeSelectItemEx(pocketCursor, i + lootScroll));
							}
						}
						break;
					}

					if (i == lootItemMax - 1)
					{
						selectTouchTime = -1;
					}
				}
			}
		}
	}
	sortFlag getSortType() { return sortType; }

	void executeTab()
	{
		if (pocketCursor == -1)
		{
			if (equipCursor == -1) //아이템을 선택 중이지 않을 경우
			{
				close(aniFlag::winSlipClose);
			}
			else
			{
				if (inputType != input::keyboard)
				{
					//select 아이템이 하나라도 있을 경우 전부 제거
					equipScroll = 0;
					equipCursor = -1;
					for (int i = 0; i < equipPtr->itemInfo.size(); i++) { equipPtr->itemInfo[i].lootSelect = 0; }
					barAct = actSet::null;
					tabType = tabFlag::closeWin;
				}
				else
				{
					close(aniFlag::winSlipClose);
				}
			}
		}
		else//가방 닫기
		{
			isTargetPocket = false;
			labelCursor = -1;
			pocketCursor = -1;
		}
	}
	//탭 키를 눌렀을 때의 연산
	void executeSort()
	{
		switch (sortType)
		{
			default:
				errorBox("undefined sorting in Equip.ixx");
				break;
			case sortFlag::null:
				lootPtr->sortWeightDescend();;
				break;
			case sortFlag::weightDescend:
				lootPtr->sortWeightDescend();
				break;
			case sortFlag::weightAscend:
				lootPtr->sortWeightAscend();
				break;
			case sortFlag::volumeDescend:
				lootPtr->sortVolumeDescend();
				break;
			case sortFlag::volumeAscend:
				lootPtr->sortVolumeAscend();
				break;
		}
		lootScroll = 0;
	}

	void executePickDrop()
	{
		if (lootPtr->itemInfo.size() > 0)
		{
			//선택된 아이템이 하나도 없으면 종료
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0) { break; }

				if (i == 0) { return; }
			}

			ItemPocket* drop = new ItemPocket(storageType::null);
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0)
				{
					lootPtr->transferItem(drop, i, lootPtr->itemInfo[i].lootSelect);
				}
			}
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--) { lootPtr->itemInfo[i].lootSelect = 0; }
			Player::ins()->drop(drop);
			Player::ins()->updateStatus();
			updateLog(col2Str(col::white) + sysStr[126]);//아이템을 버렸다.
		}
	}
	void executeSelectAll()
	{
		bool isSelectAll = true;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].lootSelect != lootPtr->itemInfo[i].number)
			{
				isSelectAll = false;
				break;
			}
		}

		if (isSelectAll == false)
		{
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				lootPtr->itemInfo[i].lootSelect = lootPtr->itemInfo[i].number;
			}
		}
		else
		{
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				lootPtr->itemInfo[i].lootSelect = 0;
			}
		}
	}
	void executeSelectItem(int index)
	{
		int itemNumber = lootPtr->itemInfo[index].number;
		lootPtr->itemInfo[index].lootSelect = itemNumber;
	}

	void executePocketLeft()
	{
		if (pocketCursor != 0)
		{
			pocketCursor--;

			lootCursor = 0;
			lootScroll = 0;
			labelCursor = -1;

			//포켓커서 변경에 따른 pocketPtr 변경
			int pocketStack = 0;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					if (pocketCursor == pocketStack)
					{
						lootPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
						break;
					}
					else
					{
						pocketStack++;
					}
				}
			}
		}


	}
	void executePocketRight()
	{
		int numberOfBag = 0;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				numberOfBag++;
			}
		}
		if (pocketCursor != numberOfBag - 1)
		{
			pocketCursor++;

			lootCursor = 0;
			lootScroll = 0;
			labelCursor = -1;

			//포켓커서 변경에 따른 pocketPtr 변경
			int pocketStack = 0;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					if (pocketCursor == pocketStack)
					{
						lootPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
						break;
					}
					else
					{
						pocketStack++;
					}
				}
			}
		}


	}
	//act
	void executeEquip()
	{
		updateLog(col2Str(col::white) + sysStr[125]);//아이템을 장착했다.
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		int returnIndex = lootPtr->transferItem(Player::ins()->getEquipPtr(), lootCursor, 1);
		equipPtr->itemInfo[returnIndex].equipState = equip::normal;
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
		{
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}

	void executeDroping()
	{
		ItemPocket* drop = new ItemPocket(storageType::null);
		if (isTargetPocket == false)
		{
			equipPtr->transferItem(drop, equipCursor, 1);
		}
		else
		{
			lootPtr->transferItem(drop, lootCursor, 1);
		}
		Player::ins()->drop(drop);
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
			doPopDownHUD = true;
		{
			barActCursor = -1;
		}
		updateLog(col2Str(col::white) + sysStr[126]);
	}
	void executeOpen()
	{
		lootPtr = (ItemPocket*)equipPtr->itemInfo[equipCursor].pocketPtr;
		pocketCursor = 0;
		isTargetPocket = true;
		for (int j = 0; j < equipPtr->itemInfo.size(); j++)
		{
			if (equipPtr->itemInfo[j].pocketMaxVolume > 0)
			{
				if (j == equipCursor)
				{
					break;
				}
				else
				{
					pocketCursor++;
				}
			}
		}

		if (inputType == input::keyboard)
		{
			lootCursor = 0;
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}
	void updateBarAct()
	{
		if (equipCursor != -1)
		{
			if (equipPtr->itemInfo.size() > 0)
			{
				ItemData targetItem = equipPtr->itemInfo[equipCursor];
				barAct.clear();
				if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
				if (targetItem.pocketOnlyItem.size() > 0) { barAct.push_back(act::reload); }//전용 아이템 있을 경우 reload 추가
				barAct.push_back(act::droping);//droping은 항상 추가
				barAct.push_back(act::throwing);//throwing도 항상 추가

				//업데이트할 아이템이 총일 경우
				if (targetItem.checkFlag(itemFlag::GUN))
				{
					//전용 아이템이 탄창일 경우(일반 소총)
					if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
					{
						ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);

						if (gunPtr->itemInfo.size() == 0)
						{
							barAct.push_back(act::reloadMagazine);
						}
						else
						{
							barAct.push_back(act::unloadMagazine);
						}
					}
					//전용 아이템이 탄일 경우(리볼버류)
					else if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
					{
						ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);
						//탄환 분리
						if (gunPtr->itemInfo.size() > 0)
						{
							barAct.push_back(act::unloadBulletFromGun);
						}

						//탄환 장전
						int bulletNumber = 0;
						for (int i = 0; i < gunPtr->itemInfo.size(); i++)
						{
							bulletNumber += gunPtr->itemInfo[i].number;
						}

						if (bulletNumber < targetItem.pocketMaxNumber)
						{
							barAct.push_back(act::reloadBulletToGun);
						}
					}
				}
				//업데이트할 아이템이 탄창일 경우
				else if (targetItem.checkFlag(itemFlag::MAGAZINE))
				{
					barAct.push_back(act::reloadMagazine);

					//탄창 장전
					ItemPocket* magazinePtr = ((ItemPocket*)targetItem.pocketPtr);
					if (magazinePtr->itemInfo.size() > 0)
					{
						barAct.push_back(act::unloadBulletFromMagazine);
					}

					//총알 장전
					int bulletNumber = 0;
					for (int i = 0; i < magazinePtr->itemInfo.size(); i++)
					{
						bulletNumber += magazinePtr->itemInfo[i].number;
					}

					if (bulletNumber < targetItem.pocketMaxNumber)
					{
						barAct.push_back(act::reloadBulletToMagazine);
					}
				}
				//업데이트할 아이템이 탄환일 경우
				else if (targetItem.checkFlag(itemFlag::AMMO))
				{
					barAct.push_back(act::reloadBulletToGun);
				}
			}
		}
		else if (lootCursor != -1)
		{
			if (lootPtr->itemInfo.size() > 0)
			{
				ItemData targetItem = lootPtr->itemInfo[lootCursor];
				barAct.clear();
				barAct.push_back(act::wield);
				if (targetItem.checkFlag(itemFlag::CANEQUIP) == true) { barAct.push_back(act::equip); }
				barAct.push_back(act::throwing);//throwing도 항상 추가
			}
		}
		else
		{
			errorBox("Equip instance : updateBarAct error occurs, Both lootCursor and equipCursor have the same value -1");
		}
	}

	Corouter executeSearch()
	{
		//이미 검색 중인지 체크
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
			{
				for (int j = 0; j < lootPtr->itemInfo.size(); j++)
				{
					lootPtr->itemInfo[j].eraseFlag(itemFlag::GRAYFILTER);
				}
				lootPtr->sortByUnicode();
				updateLog(col2Str(col::white) + sysStr[86]);//검색 상태를 해제했다.
				co_return;
			}

			if (i == lootPtr->itemInfo.size() - 1)//검색 중이 아닐 경우
			{
				std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
				new Msg(msgFlag::input, sysStr[27], sysStr[97], choiceVec);//검색, 검색할 키워드를 입력해주세요
				lootScroll = 0;
				co_await std::suspend_always();
				if (coAnswer == sysStr[38]) //검색 후 확인 버튼
				{
					int matchCount = lootPtr->searchTxt(exInputText);
					for (int i = 0; i < lootPtr->itemInfo.size(); i++) lootPtr->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) lootPtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}

	Corouter executeSelectItemEx(int pocketCursor, int lootCursor)
	{
		//입력형 메시지 박스 열기
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		//포켓커서에 따른 pocketPtr 변경
		ItemPocket* lootPtr = nullptr;
		int pocketStack = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				if (pocketCursor == pocketStack)
				{
					lootPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
					break;
				}
				else
				{
					pocketStack++;
				}
			}
		}

		int inputSelectNumber = wtoi(exInputText.c_str()); // 셀렉트 박스에 넣어질 숫자(플레이어가 입력한 값)
		if (inputSelectNumber > lootPtr->itemInfo[lootCursor].number) //만약 실제 있는 숫자보다 많은 값을 입력했을 경우
		{
			inputSelectNumber = lootPtr->itemInfo[lootCursor].number; //Select의 값을 최댓값으로 맞춤
		}
		lootPtr->itemInfo[lootCursor].lootSelect = inputSelectNumber;
	}

	Corouter executeWield()
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::TWOHANDED)) //양손장비일 경우
		{
			bool isWield = false;
			ItemPocket* drop = new ItemPocket(storageType::null);
			for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (equipPtr->itemInfo[i].equipState == equip::left || equipPtr->itemInfo[i].equipState == equip::right || equipPtr->itemInfo[i].equipState == equip::both)
				{
					equipPtr->transferItem(drop, i, 1);
					isWield = true;
				}
			}
			if (isWield == true) { Player::ins()->drop(drop); }
			else { delete drop; }
			int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
			equipPtr->itemInfo[returnIndex].equipState = equip::both; //양손
			equipPtr->sortEquip();
			updateLog(L"#FFFFFF아이템을 들었다.");
		}
		else
		{

			bool hasLeft = false;
			bool hasRight = false;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				switch (equipPtr->itemInfo[i].equipState)
				{
					case equip::left:
						hasLeft = true;
						break;
					case equip::right:
						hasRight = true;
						break;
					case equip::both:
						hasLeft = true;
						hasRight = true;
						break;
				}
			}

			if (hasLeft == true && hasRight == true)
			{
				int fixedLootCursor = lootCursor;//Msg가 켜지면 lootCursor가 -1로 초기화되기에 미리 지역변수로 저장

				//왼손, 오른손
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//선택, 어느 손에 들까?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				int handDir = 0;
				switch (wtoi(coAnswer.c_str()))
				{
					case 0:
						handDir = equip::left;
						break;
					case 1:
						handDir = equip::right;
						break;
				}

				//왼손 아이템 떨구기
				ItemPocket* drop = new ItemPocket(storageType::null);
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == handDir)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				//양손 아이템 떨구기
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == equip::both)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				Player::ins()->drop(drop);

				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == false)
			{
				int fixedLootCursor = lootCursor;//Msg가 켜지면 lootCursor가 -1로 초기화되기에 미리 지역변수로 저장

				//왼손, 오른손
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//선택, 어느 손에 들까?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				int handDir = 0;
				switch (wtoi(coAnswer.c_str()))
				{
					case 0:
						handDir = equip::left;
						break;
					case 1:
						handDir = equip::right;
						break;
				}
				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == true)//왼손에 들기
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equip::left;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
			else//오른손에 들기
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equip::right;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
		}
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
		{
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}

	Corouter executeReload()//장전 : 타겟아이템(탄창이나 총)에 넣을 수 있는 탄환을 넣는다.
	{
		int targetEquipCursor = equipCursor;
		std::vector<std::wstring> bulletList;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == equipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				bulletList.push_back(equipPtr->itemInfo[i].name);
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						bulletList.push_back(pocketPtr->itemInfo[pocketItr].name);
						break;
					}
				}
			}
		}

		if (bulletList.size() == 0) //넣을만한 포켓을 찾지 못했을 경우
		{
			//이 아이템을 넣을만한 포켓이 없다.
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], bulletList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == targetEquipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(1층)
				{
					if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
					{
						if (equipPtr->itemInfo[i].checkFlag(itemFlag::AMMO))
						{
							//(%bullet) (%number)개를(을) (%gun)에 장전했다!

							std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
							std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str3);
						}
						else
						{
							//(%magazine)를(을) (%gun)에 장전했다!
							std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str2);
						}
					}
					else
					{
						//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
						std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", equipPtr->itemInfo[i].name);
						std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
						std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
						updateLog(col2Str(col::white) + str3);
					}

					equipPtr->transferItem
					(
						(ItemPocket*)equipPtr->itemInfo[targetEquipCursor].pocketPtr,
						i,
						equipPtr->itemInfo[i].number
					);
					co_return;
				}
				counter++;
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(2층)
						{
							if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
							{
								if (pocketPtr->itemInfo[pocketItr].checkFlag(itemFlag::AMMO))
								{
									//(%bullet) (%number)개를(을) (%gun)에 장전했다!

									std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
									std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str3);
								}
								else
								{
									//(%magazine)를(을) (%gun)에 장전했다!
									std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str2);
								}
							}
							else
							{
								//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
								std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
								std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
								std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
								updateLog(col2Str(col::white) + str3);
							}


							pocketPtr->transferItem
							(
								(ItemPocket*)equipPtr->itemInfo[targetEquipCursor].pocketPtr,
								pocketItr,
								pocketPtr->itemInfo[pocketItr].number
							);
							co_return;
						}
						counter++;
						break;
					}
				}
			}
		}
	}

	Corouter executeThrowing(ItemPocket* inputPocket, int inputIndex)//던지기
	{
		new CoordSelect(sysStr[131]);
		deactDraw();

		co_await std::suspend_always();

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		Player::ins()->setDirection(getIntDegree(Player::ins()->getGridX(), Player::ins()->getGridY(), targetX, targetY));

		prt(L"executeThrowing에서 사용한 좌표의 값은 (%d,%d,%d)이다.\n", targetX, targetY, targetZ);

		if (targetX == Player::ins()->getGridX() && targetY == Player::ins()->getGridY() && targetZ == Player::ins()->getGridZ())
		{
			ItemPocket* drop = new ItemPocket(storageType::null);
			inputPocket->transferItem(drop, inputIndex, 1);
			Player::ins()->drop(drop);
			Player::ins()->updateStatus();
			updateLog(L"#FFFFFF아이템을 버렸다.");
		}
		else
		{
			ItemPocket* throwing = new ItemPocket(storageType::null);
			//이큅일 떄는 그렇다쳐도 가방 안에 있는 아이템을 던질 떄 원하는대로 작동하지않아 오류가 생긴다
			inputPocket->transferItem(throwing, inputIndex, 1);
			Player::ins()->throwing(throwing, targetX, targetY);
			Player::ins()->updateStatus();
			updateLog(L"#FFFFFF아이템을 던졌다.");
		}
		close(aniFlag::null);
	}

};
