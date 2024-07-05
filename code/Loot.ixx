#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Loot;

import std;
import util;
import globalVar;
import constVar;
import drawWindowArrow;
import ItemPocket;
import drawItemList;
import checkCursor;
import drawSprite;
import drawText;
import Player;
import World;
import textureVar;
import log;
import ItemStack;
import Msg;
import GUI;
import actFuncSet;
import drawWindow;
import Lst;

export class Loot : public GUI
{
private:
	inline static Loot* ptr = nullptr;
	ItemPocket* lootPtr = nullptr;
	const int lootScrollSize = 6; //한 스크롤에 들어가는 아이템의 수
	int lootScroll = 0; //우측 루팅창의 스크롤
	int lootCursor = -1; //우측 루팅창의 커서
	int pocketCursor = 0; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	//모션 스크롤 이벤트에서 기준으로 잡는 최초 터치 당시의 스크롤
	int initLootScroll = 0; //모션스크롤이 시작되기 직전의 스크롤
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;
	sortFlag sortType = sortFlag::null; //0:기본_1:무게내림_2:무게올림_3:부피내림_4:부피올림

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
	SDL_Rect topWindow;
public:
	Corouter errorFunc();

	Loot(int targetGridX, int targetGridY) : GUI(false)
	{
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);
		//errorBox(ptr != nullptr, "More than one Loot instance was generated.");
		ptr = this;
		lootTile[axis::x] = targetGridX;
		lootTile[axis::y] = targetGridY;
		lootTile[axis::z] = Player::ins()->getGridZ();

		changeXY((cameraW / 2) + 17, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		tabType = tabFlag::closeWin;
		UIType = act::loot;

		lootPtr = World::ins()->getItemPos(lootTile[axis::x], lootTile[axis::y], Player::ins()->getGridZ())->getPocket();
		lootPtr->sortByUnicode();

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (inputType == input::keyboard)
		{
			lootCursor = 0;
		}
	}
	~Loot()
	{
		prt(L"Loot : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		lootCursor = -1;
		lootScroll = 0;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++) { lootPtr->itemInfo[i].lootSelect = 0; }
		tabType = tabFlag::autoAtk;
		barAct = actSet::null;
		barActCursor = -1;
	}
	static Loot* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		lootBase = { 0,0,335,420 };
		if (center == false)
		{
			lootBase.x += inputX;
			lootBase.y += inputY;
		}
		else
		{
			lootBase.x += inputX - lootBase.w / 2;
			lootBase.y += inputY - lootBase.h / 2;
		}
		lootTitle = { lootBase.x + 102, lootBase.y + 0, 130, 30 };
		lootWindow = { lootBase.x + 0, lootBase.y + 120, 335, 300 };
		lootArea = { lootWindow.x + 10, lootWindow.y + 40,312, 246 };
		for (int i = 0; i < lootItemMax; i++)
		{
			lootItem[i] = { lootArea.x + 42, lootArea.y + 42 * i, 270, 36 };
			lootItemSelect[i] = { lootArea.x, lootArea.y + 42 * i, 36, 36 };
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
			x = inputX - lootBase.w / 2;
			y = inputY - lootBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		const Uint8* state = SDL_GetKeyboardState(NULL);
		Sprite* targetBtnSpr = nullptr;

		drawWindow(&lootBase, sysStr[10], 1);

		//포켓
		{
			//drawStadium(pocketWindow.x, pocketWindow.y, pocketWindow.w, pocketWindow.h, { 0,0,0 }, 150, 5);

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
				drawRect(weightBar, col::white);

				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
				setFontSize(10);
				drawTextCenter(L"132.9/99.9 KG", weightBar.x + (weightBar.w / 2), weightBar.y - 8);

				//루팅 주머니 부피 게이지
				SDL_Rect volumeBar = { pocketWindow.x + pocketWindow.w - 12 - 72, pocketWindow.y + 64, 72, 4 };
				drawRect(volumeBar, col::white);
				SDL_Rect volumeGauge = { volumeBar.x + 1, volumeBar.y + 1, volumeBar.w - 2, 2 };
				int maxVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketMaxVolume;
				int currentVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketVolume;
				volumeGauge.w = (volumeBar.w - 2) * ((float)currentVolume / (float)maxVolume);
				drawFillRect(volumeGauge, lowCol::green);

				std::wstring volumeStr = decimalCutter(currentVolume / 1000.0, 2) + L"/" + decimalCutter(maxVolume / 1000.0, 2) + L" L";
				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
				setFontSize(10);
				drawTextCenter(volumeStr, volumeBar.x + (volumeBar.w / 2), volumeBar.y - 8);
				setFontSize(13);
				drawTextCenter(L"테스트 아이템", pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + pocketWindow.h - 10);



				//포켓 좌우 변경 버튼
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

			// 아이템 스크롤 그리기
			drawFillRect(lootScrollBox, { 120,120,120 });
			SDL_Rect inScrollBox = { lootWindow.x + 328, lootWindow.y + 40, 2, 42 * lootItemMax }; // 내부 스크롤 커서
			inScrollBox.h = lootScrollBox.h * myMin(1.0, (double)lootItemMax / lootPtr->itemInfo.size());
			inScrollBox.y = lootScrollBox.y + lootScrollBox.h * ((float)lootScroll / (float)lootPtr->itemInfo.size());
			if (inScrollBox.y + inScrollBox.h > lootScrollBox.y + lootScrollBox.h) { inScrollBox.y = lootScrollBox.y + lootScrollBox.h - inScrollBox.h; }
			drawFillRect(inScrollBox, col::white);

			//루팅버튼 그리기
			{
				SDL_Color lootBtnColor;
				if (checkCursor(&lootBtn))
				{
					if (click == true) { lootBtnColor = lowCol::deepBlue; }
					else { lootBtnColor = lowCol::blue; }
				}
				else { lootBtnColor = lowCol::black; }

				drawFillRect(lootBtn, lootBtnColor, 200);
				drawRect(lootBtn, { 0x57, 0x57, 0x57 });

				drawSpriteCenter(spr::lootBagArrow, 1, lootWindow.x + lootWindow.w / 2, lootWindow.y - 4);

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

				if (inputType == input::keyboard)
				{
					if (state[SDL_SCANCODE_V]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_V, lootBtn.x, lootBtn.y + lootBtn.h / 2);
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
		else if (checkCursor(&lootArea)) //아이템 클릭 -> 에러 파트
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
									CORO(executeSelectItemEx(i + lootScroll));
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
		else if (checkCursor(&pocketLeft))
		{
			if (pocketCursor != 0) { pocketCursor--; }
		}
		else if (checkCursor(&pocketRight))
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
			if (pocketCursor != numberOfBag - 1) { pocketCursor++; }
		}
		else if (checkCursor(&lootBtn))
		{
			executePickSelect();
		}
		else if (checkCursor(&letterbox)) //버튼은 return 없음
		{
			for (int i = 0; i < barAct.size(); i++) // 하단 UI 터치 이벤트
			{
				if (checkCursor(&barButton[i]))
				{
					switch (barAct[i])
					{
					case act::pick://넣기
						executePick();
						break;
					case act::equip://장비
						executeEquip();
						break;
					case act::wield://들기
						CORO(executeWield());
						break;
						//case act::insert:
						//	CORO(executeInsert());
						//	break;
					case act::reloadBulletToMagazine:
					case act::reloadBulletToGun:
						if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::MAGAZINE))
						{
							CORO(actFunc::reloadSelf(actEnv::Loot, lootPtr, lootCursor));
						}
						else if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::AMMO))
						{
							CORO(actFunc::reloadOther(actEnv::Loot, lootPtr, lootCursor));
						}
						else if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::GUN))
						{
							CORO(actFunc::reloadSelf(actEnv::Equip, lootPtr, lootCursor));
						}
						break;
					case act::reloadMagazine:
						//총에서 사용하는 경우와 탄창에서 사용하는 경우가 다름
						//총에서 사용하면 자기 자신에게 장전함(self)
						//탄창에 사용하면 다른 타일의 총에게 장비함
						if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::MAGAZINE))
						{
							CORO(actFunc::reloadOther(actEnv::Loot, lootPtr, lootCursor));
						}
						else
						{
							CORO(actFunc::reloadSelf(actEnv::Loot, lootPtr, lootCursor));
						}
						break;
					case act::unloadMagazine:
					case act::unloadBulletFromMagazine:
					case act::unloadBulletFromGun:
						actFunc::unload(lootPtr, lootCursor);
						break;
					}
				}
			}
		}

		//위의 모든 경우에서 return을 받지 못했으면 버튼 이외를 누른 것이므로 커서를 -1로 복구
		{
			lootCursor = -1;
			barAct = actSet::null;
			tabType = tabFlag::closeWin;
		}
	}
	void clickMotionGUI(int dx, int dy)
	{
		if (checkCursor(&lootBase))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // 가속상수, 작아질수록 스크롤 속도가 빨라짐
				lootScroll = initLootScroll + dy / scrollAccelConst;
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
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
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
								CORO(executeSelectItemEx(i + lootScroll));
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

		//잘못된 커서 위치 조정
		if (lootCursor > (int)(lootPtr->itemInfo.size() - 1)) { lootCursor = lootPtr->itemInfo.size() - 1; }

		//잘못된 스크롤 위치 조정
		if (lootScroll + lootItemMax >= lootPtr->itemInfo.size()) { lootScroll = myMax(0, (int)lootPtr->itemInfo.size() - lootItemMax); }
		else if (lootScroll < 0) { lootScroll = 0; }

		//루트 아이템의 갯수가 0이 되었을 경우 창을 닫음
		if (lootPtr->itemInfo.size() == 0)
		{
			close(aniFlag::null);
			//클로즈 후의 애니메이션이 문제가 된다. 애니메이션이 모두 실행되고 제거해야됨
			delete World::ins()->getItemPos(lootTile[axis::x], lootTile[axis::y], Player::ins()->getGridZ());
			return;
		}
	}

	sortFlag getSortType() { return sortType; }
	//탭 키를 눌렀을 때의 연산
	void executeTab()
	{
		if (lootCursor == -1) //아이템을 선택 중이지 않을 경우
		{
			close(aniFlag::winSlipClose);
		}
		else
		{
			if (inputType != input::keyboard)
			{
				lootCursor = -1;
				barAct = actSet::null;
				tabType = tabFlag::closeWin;
			}
			else
			{
				close(aniFlag::winSlipClose);
			}
		}
	}
	void executeSort()
	{
		switch (sortType)
		{
		default:
			errorBox("undefined sorting in Loot.ixx");
			break;
		case sortFlag::null:
			lootPtr->sortWeightDescend();
			sortType = sortFlag::weightDescend;
			break;
		case sortFlag::weightDescend:
			lootPtr->sortWeightAscend();
			sortType = sortFlag::weightAscend;
			break;
		case sortFlag::weightAscend:
			lootPtr->sortVolumeDescend();
			sortType = sortFlag::volumeDescend;
			break;
		case sortFlag::volumeDescend:
			lootPtr->sortVolumeAscend();
			sortType = sortFlag::volumeAscend;
			break;
		case sortFlag::volumeAscend:
			lootPtr->sortByUnicode();
			sortType = sortFlag::null;
			break;
		}
		lootScroll = 0;
	}
	void executePickSelect()
	{
		//1. 포켓 커서가 가리키는 아이템의 Array의 잔여부피와 플레이어의 질량 한계를 참조
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		ItemPocket* bagPtr = nullptr;
		int bagIndex = -1;
		int bagNumber = 0;

		if (equipPtr->itemInfo.size() == 0)
		{
			updateLog(col2Str(col::white) + sysStr[123]);
			return;
		}

		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			//가방일 경우
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0) { bagNumber++; }

			//커서가 가리키는 포켓의 주소를 저장
			if (bagNumber - 1 == pocketCursor)
			{
				bagPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				bagIndex = i;
			}

			//가방을 찾지 못했을 경우
			if (i == equipPtr->itemInfo.size() - 1 && bagPtr == nullptr)
			{
				updateLog(col2Str(col::white) + sysStr[123]);
				return;
			}
		}

		int currentVol = equipPtr->itemInfo[bagIndex].pocketVolume;
		int maxVol = equipPtr->itemInfo[bagIndex].pocketMaxVolume;

		int itemsVol = 0;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].lootSelect > 0)
			{
				itemsVol += lootPtr->itemInfo[i].lootSelect * lootPtr->itemInfo[i].volume;
			}
		}

		if (maxVol < itemsVol + currentVol)
		{
			updateLog(col2Str(col::white) + sysStr[124]);
			return;
		}
		else
		{
			int fixedLootSize = lootPtr->itemInfo.size();
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0)
				{
					lootPtr->transferItem(bagPtr, i, lootPtr->itemInfo[i].lootSelect);
				}
			}
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--) { lootPtr->itemInfo[i].lootSelect = 0; }
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
		if (pocketCursor != 0) { pocketCursor--; }
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
		if (pocketCursor != numberOfBag - 1) { pocketCursor++; }
	}
	//act
	void executePick()
	{
		//pick 구현하기 전에 select 기능 먼저 구현해야됨
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
			updateLog(col2Str(col::white) + sysStr[123]);
			return;
		}
		else
		{
			//지금 가리키는 lootCursor의 select를 풀로 만듬
			int itemNumber = lootPtr->itemInfo[lootCursor].number;
			lootPtr->itemInfo[lootCursor].lootSelect = itemNumber;
			//아직 질량&부피 체크 추가하지 않았음
			lootPtr->transferItem((ItemPocket*)equipPtr->itemInfo[pocketList[pocketCursor]].pocketPtr, lootCursor, lootPtr->itemInfo[lootCursor].lootSelect);
			if (inputType == input::keyboard)
			{
				doPopDownHUD = true;
				barActCursor = -1;
			}
		}
	}
	void executeEquip()
	{
		updateLog(col2Str(col::white) + sysStr[125]);
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

	void updateBarAct()
	{
		if (lootPtr->itemInfo.size() > 0)
		{
			ItemData targetItem = lootPtr->itemInfo[lootCursor];
			barAct.clear();
			barAct.push_back(act::wield);

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

			if (targetItem.checkFlag(itemFlag::CANEQUIP) == true) { barAct.push_back(act::equip); }
		}
	}

	//▼코루틴 함수▼
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
				if (coAnswer == sysStr[38])
				{
					int matchCount = lootPtr->searchTxt(exInputText);
					for (int i = 0; i < lootPtr->itemInfo.size(); i++) lootPtr->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) lootPtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}

	Corouter executeSelectItemEx(int index)
	{
		//입력형 메시지 박스 열기
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		int inputSelectNumber = wtoi(exInputText.c_str()); // 셀렉트 박스에 넣어질 숫자(플레이어가 입력한 값)
		if (inputSelectNumber > lootPtr->itemInfo[index].number) //만약 실제 있는 숫자보다 많은 값을 입력했을 경우
		{
			inputSelectNumber = lootPtr->itemInfo[index].number; //Select의 값을 최댓값으로 맞춤
		}
		lootPtr->itemInfo[index].lootSelect = inputSelectNumber;
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
				if (coAnswer == sysStr[49])//왼손
				{
					handDir = equip::left;
				}
				else//오른손
				{
					handDir = equip::right;
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
				if (coAnswer == sysStr[49])//왼손
				{
					handDir = equip::left;
				}
				else//오른손
				{
					handDir = equip::right;
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


	Corouter executeInsert()//삽탄 : 총알에 사용, 이 탄환을 넣을 수 있는 탄창 리스트를 표시하고 거기에 넣음
	{
		int targetLootCursor = lootCursor;
		std::vector<std::wstring> pocketList;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0 || equipPtr->itemInfo[i].pocketMaxNumber > 0)
			{
				if (equipPtr->itemInfo[i].pocketMaxNumber > 0) //전용 아이템이 있을 경우
				{
					if (std::find(equipPtr->itemInfo[i].pocketOnlyItem.begin(), equipPtr->itemInfo[i].pocketOnlyItem.end(), lootPtr->itemInfo[targetLootCursor].itemCode) == equipPtr->itemInfo[i].pocketOnlyItem.end())
					{
						//루트커서의 해당 아이템을 넣을 수 없는 포켓이면 continue
						continue;
					}
				}

				pocketList.push_back(equipPtr->itemInfo[i].name);
			}
		}

		if (pocketList.size() == 0) //넣을만한 포켓을 찾지 못했을 경우
		{
			//이 아이템을 넣을만한 포켓이 없다.
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], pocketList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0 || equipPtr->itemInfo[i].pocketMaxNumber > 0)
			{
				if (equipPtr->itemInfo[i].pocketMaxNumber > 0) //전용 아이템이 있을 경우
				{
					if (std::find(equipPtr->itemInfo[i].pocketOnlyItem.begin(), equipPtr->itemInfo[i].pocketOnlyItem.end(), lootPtr->itemInfo[targetLootCursor].itemCode) == equipPtr->itemInfo[i].pocketOnlyItem.end())
					{
						//루트커서의 해당 아이템을 넣을 수 없는 포켓이면 continue
						continue;
					}
				}

				if (counter == wtoi(coAnswer.c_str()))
				{

					int transferNumber;
					if (lootPtr->itemInfo[targetLootCursor].lootSelect != 0)
					{
						transferNumber = lootPtr->itemInfo[targetLootCursor].lootSelect;
						lootPtr->itemInfo[targetLootCursor].lootSelect = 0;
					}
					else { transferNumber = lootPtr->itemInfo[targetLootCursor].number; }

					//몇 개 넣을까?
					lootPtr->transferItem
					(
						(ItemPocket*)equipPtr->itemInfo[i].pocketPtr,
						targetLootCursor,
						transferNumber
					);

					co_return;
				}

				counter++;
			}
		}
	}
};
