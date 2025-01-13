#include <SDL.h>

import Loot;
import globalVar;
import constVar;
import textureVar;
import util;
import Sprite;
import Player;
import drawWindow;
import drawSprite;
import checkCursor;
import drawText;
import drawItemList;

void Loot::drawGUI()
{
	if (getStateDraw() == false) { return; }

	const Uint8* state = SDL_GetKeyboardState(NULL);
	Sprite* targetBtnSpr = nullptr;

	bool hasSelect = false; //아이템에 셀렉트가 하나라도 있을 경우
	for (int i = 0; i < lootPocket->itemInfo.size(); i++)
	{
		if (lootPocket->itemInfo[i].lootSelect > 0)
		{
			hasSelect = true;
			break;
		}
		if (i == lootPocket->itemInfo.size() - 1)
		{
			hasSelect = false;
		}
	}

	drawWindow(&lootBase, sysStr[10], 1);

	//포켓
	if (hasSelect == false)
	{
		drawFillRect(lootBase.x + 13, lootBase.y + 40, 50, 50, col::black);
		drawSprite(spr::inventoryItemRect, 0, lootBase.x + 13, lootBase.y + 40);

		setZoom(3.0);
		int tileIndex = 140;
		if (lootItemData != nullptr) tileIndex = lootItemData->sprIndex;
		drawSpriteCenter(spr::itemset, tileIndex, lootBase.x + 13 + 25, lootBase.y + 40 + 25);
		setZoom(1.0);

		setFontSize(16);
		std::wstring tileName = L"Tile name";
		if (lootItemData != nullptr) tileName = lootItemData->name;
		drawText(col2Str(col::white) + tileName, lootBase.x + 73, lootBase.y + 39);

		drawLine(lootBase.x + 72, lootBase.y + 63, lootBase.x + 72 + 255, lootBase.y + 63, col::gray);//회색 분리선

		SDL_Rect volumeGaugeRect = { lootBase.x + 123,lootBase.y + 72,104,9 };
		drawRect(volumeGaugeRect, col::white);
		drawFillRect({ volumeGaugeRect.x + 2,volumeGaugeRect.y + 2,70,5 }, lowCol::yellow);
		drawSpriteCenter(spr::icon16, 62, volumeGaugeRect.x - 47, volumeGaugeRect.y + 4);
		setFontSize(10);
		drawText(col2Str(col::white) + sysStr[18], volumeGaugeRect.x - 38, volumeGaugeRect.y - 2);
		setFontSize(8);
		drawText(col2Str(col::white) + L"72.5 / 92.3 L", volumeGaugeRect.x + 110, volumeGaugeRect.y - 1);

		//좌측상단 버리기 버튼
		SDL_Rect dropBtn = { lootBase.x + 259,lootBase.y + 36,69,23 };
		drawFillRect(dropBtn, col::black);
		drawRect(dropBtn, col::gray);
		drawSpriteCenter(spr::icon16, 63, dropBtn.x + 11, dropBtn.y + 12);
		setFontSize(10);
		drawTextCenter(col2Str(col::white) + sysStr[52], dropBtn.x + dropBtn.w / 2 + 8, dropBtn.y + dropBtn.h / 2);
		drawFillRect(dropBtn, col::black, 150);
	}
	else
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
			setFontSize(16);
			drawTextCenter(col2Str(col::white)+sysStr[19], pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + 35);
		}
		else
		{
			SDL_Rect pocketItem[7];
			pocketItem[3] = { lootBase.x + 167 - 16, lootBase.y + 50 - 16, 32, 32 };

			pocketItem[2] = { lootBase.x + 167 - 16 - 30 - 27 * 0, lootBase.y + 50 - 16 + 5, 24, 24 };
			pocketItem[1] = { lootBase.x + 167 - 16 - 30 - 27 * 1, lootBase.y + 50 - 16 + 5, 24, 24 };
			pocketItem[0] = { lootBase.x + 167 - 16 - 30 - 27 * 2, lootBase.y + 50 - 16 + 5, 24, 24 };

			pocketItem[4] = { lootBase.x + 167 - 16 + 38 + 27 * 0, lootBase.y + 50 - 16 + 5, 24, 24 };
			pocketItem[5] = { lootBase.x + 167 - 16 + 38 + 27 * 1, lootBase.y + 50 - 16 + 5, 24, 24 };
			pocketItem[6] = { lootBase.x + 167 - 16 + 38 + 27 * 2, lootBase.y + 50 - 16 + 5, 24, 24 };

			//포켓 1~3번째 칸 그리기
			if (pocketCursor != 0)
			{

				setZoom(1.5);
				drawFillRect(pocketItem[2].x, pocketItem[2].y, pocketItem[2].w, pocketItem[2].h, { 0,0,0 }, 200);
				drawRect(pocketItem[2], col::gray);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 1]].sprIndex, pocketItem[2].x + (pocketItem[2].w / 2), pocketItem[2].y + (pocketItem[2].h / 2));
				if (pocketCursor != 1)
				{
					drawFillRect(pocketItem[1].x, pocketItem[1].y, pocketItem[1].w, pocketItem[1].h, { 0,0,0 }, 200);
					drawRect(pocketItem[1], col::gray);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 2]].sprIndex, pocketItem[1].x + (pocketItem[1].w / 2), pocketItem[1].y + (pocketItem[1].h / 2));
					if (pocketCursor != 2)
					{
						drawFillRect(pocketItem[0].x, pocketItem[0].y, pocketItem[0].w, pocketItem[0].h, { 0,0,0 }, 200);
						drawRect(pocketItem[0], col::gray);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 3]].sprIndex, pocketItem[0].x + (pocketItem[0].w / 2), pocketItem[0].y + (pocketItem[0].h / 2));
					}
				}
			}

			//포켓 4번째 칸
			setZoom(2.0);
			drawFillRect(pocketItem[3].x, pocketItem[3].y, pocketItem[3].w, pocketItem[3].h, lowCol::blue, 200);
			drawRect(pocketItem[3], col::white);
			drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor]].sprIndex, lootBase.x + 167, lootBase.y + 50);

			//포켓 5~7번째 칸 
			if (pocketCursor != numberOfBag - 1)
			{
				setZoom(1.5);
				drawFillRect(pocketItem[4].x, pocketItem[4].y, pocketItem[4].w, pocketItem[4].h, { 0,0,0 }, 200);
				drawRect(pocketItem[4], col::gray);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 1]].sprIndex, pocketItem[4].x + (pocketItem[4].w / 2), pocketItem[4].y + (pocketItem[4].h / 2));
				if (pocketCursor != numberOfBag - 2)
				{
					drawFillRect(pocketItem[5].x, pocketItem[5].y, pocketItem[5].w, pocketItem[5].h, { 0,0,0 }, 200);
					drawRect(pocketItem[5], col::gray);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 2]].sprIndex, pocketItem[5].x + (pocketItem[5].w / 2), pocketItem[5].y + (pocketItem[5].h / 2));
					if (pocketCursor != numberOfBag - 3)
					{
						drawFillRect(pocketItem[6].x, pocketItem[6].y, pocketItem[6].w, pocketItem[6].h, { 0,0,0 }, 200);
						drawRect(pocketItem[6], col::gray);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 3]].sprIndex, pocketItem[6].x + (pocketItem[6].w / 2), pocketItem[6].y + (pocketItem[6].h / 2));
					}
				}
			}
			setZoom(1.0);


			//포켓 질량 게이지
			SDL_Rect weightBar = { pocketWindow.x + 42, pocketWindow.y + 52, 72, 4 };
			drawRect(weightBar, col::white);

			setFontSize(10);
			drawTextCenter(col2Str(col::white)+L"132.9/99.9 KG", weightBar.x + (weightBar.w / 2), weightBar.y - 8);

			//루팅 주머니 부피 게이지
			SDL_Rect volumeBar = { pocketWindow.x + pocketWindow.w - 114, pocketWindow.y + 52, 72, 4 };
			drawRect(volumeBar, col::white);
			SDL_Rect volumeGauge = { volumeBar.x + 1, volumeBar.y + 1, volumeBar.w - 2, 2 };
			int maxVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketMaxVolume;
			int currentVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketVolume;
			volumeGauge.w = (volumeBar.w - 2) * ((float)currentVolume / (float)maxVolume);
			drawFillRect(volumeGauge, lowCol::green);

			std::wstring volumeStr = decimalCutter(currentVolume / 1000.0, 2) + L"/" + decimalCutter(maxVolume / 1000.0, 2) + L" L";
			setFontSize(10);
			drawTextCenter(col2Str(col::white)+volumeStr, volumeBar.x + (volumeBar.w / 2), volumeBar.y - 8);

			//setFontSize(11);
			//drawTextCenter(L"테스트 아이템", pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + pocketWindow.h - 10);

			//포켓 좌우 변경 버튼
			{
				SDL_Color leftBtnColor;
				if (checkCursor(&pocketLeft))
				{
					if (click == true) { leftBtnColor = lowCol::deepBlue; }
					else { leftBtnColor = lowCol::blue; }
				}
				else { leftBtnColor = lowCol::black; }
				drawFillRect(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, leftBtnColor, 200);
				drawRect(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, col::lightGray, 200);

				if (option::inputMethod == input::keyboard)
				{
					if (state[SDL_SCANCODE_LSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_LShift, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
				}
				else if (option::inputMethod == input::gamepad)
				{
					if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_L1, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
				}
				else
				{
					drawSpriteCenter(spr::windowArrow, 2, pocketLeft.x + pocketLeft.w/2, pocketLeft.y + pocketLeft.h / 2);
				}

				if (pocketCursor == 0) { drawFillRect(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, col::black, 200); }

			}

			{
				SDL_Color rightBtnColor;
				if (checkCursor(&pocketRight))
				{
					if (click == true) { rightBtnColor = lowCol::deepBlue; }
					else { rightBtnColor = lowCol::blue; }
				}
				else { rightBtnColor = lowCol::black; }
				drawFillRect(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, rightBtnColor, 200);
				drawRect(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, col::lightGray);

				if (option::inputMethod == input::keyboard)
				{
					if (state[SDL_SCANCODE_RSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_RShift, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
				}
				else if (option::inputMethod == input::gamepad)
				{
					if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_R1, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
				}
				else
				{
					drawSpriteCenter(spr::windowArrow, 0, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
				}

				if (pocketCursor == numberOfBag - 1) { drawFillRect(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, col::black, 200); }
			}
		}


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

			if (option::inputMethod == input::keyboard)
			{
				if (state[SDL_SCANCODE_V]) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_V, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}
			else if (option::inputMethod == input::gamepad)
			{
				if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y)) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_TRI, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}
			else
			{
				drawSpriteCenter(spr::lootBagArrow, 1, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}

			if(hasSelect==false) drawStadium(lootBtn.x, lootBtn.y, lootBtn.w, lootBtn.h, lootBtnColor, 200, 5);
		}

	}
	

	//여기서부턴 루팅 윈도우
	{
		//우측 아이템 상단바 라벨(선택 이름 물리량)
		drawStadium(lootLabel.x, lootLabel.y, lootLabel.w, lootLabel.h, { 0,0,0 }, 183, 5);
		if (GUI::getLastGUI() == this)
		{
			if (checkCursor(&lootLabelSelect) || labelCursor == 0)
			{
				SDL_Color btnColor;
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				drawStadium(lootLabelSelect.x, lootLabelSelect.y, lootLabelSelect.w, lootLabelSelect.h, btnColor, 183, 5);
			}
			else if (checkCursor(&lootLabelName) || labelCursor == 1)
			{
				SDL_Color btnColor;
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				drawStadium(lootLabelName.x, lootLabelName.y, lootLabelName.w, lootLabelName.h, btnColor, 183, 5);
			}
			else if (checkCursor(&lootLabelQuantity) || labelCursor == 2)
			{
				SDL_Color btnColor;
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				drawStadium(lootLabelQuantity.x, lootLabelQuantity.y, lootLabelQuantity.w, lootLabelQuantity.h, btnColor, 183, 5);
			}
		}
		setFontSize(13);
		drawTextCenter(col2Str(col::white)+sysStr[15], lootLabel.x + 32, lootLabel.y + 12); //선택(상단바)

		{ //이름(상단바)
			std::wstring tailStr = L"";
			int grayNumber = 0;
			int whiteNumber = 0;
			//이미 검색 중인지 체크
			for (int i = 0; i < lootPocket->itemInfo.size(); i++)
			{
				if (lootPocket->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
				{
					grayNumber++;
				}
			}

			if (grayNumber > 0)
			{
				whiteNumber = lootPocket->itemInfo.size() - grayNumber;
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
		if (GUI::getLastGUI() != this) itemListColorLock = true;
		else  itemListColorLock = false;
		drawItemList(lootPocket, lootArea.x, lootArea.y, LOOT_ITEM_MAX, lootCursor, lootScroll, true);

		// 아이템 스크롤 그리기
		SDL_Rect lootScrollBox = { lootBase.x + 325, lootItemRect[0].y, 2, lootBase.h - 135 };
		drawFillRect(lootScrollBox, { 120,120,120 });
		SDL_Rect inScrollBox = lootScrollBox; // 내부 스크롤 커서
		inScrollBox.h = lootScrollBox.h * myMin(1.0, (double)LOOT_ITEM_MAX / lootPocket->itemInfo.size());
		inScrollBox.y = lootScrollBox.y + lootScrollBox.h * ((float)lootScroll / (float)lootPocket->itemInfo.size());
		if (inScrollBox.y + inScrollBox.h > lootScrollBox.y + lootScrollBox.h) { inScrollBox.y = lootScrollBox.y + lootScrollBox.h - inScrollBox.h; }
		drawFillRect(inScrollBox, col::white);


		if (lootPocket->itemInfo.size() == 0)
		{
			setFontSize(10);
			drawTextCenter(col2Str(col::lightGray) + sysStr[162], lootBase.x + 162, lootBase.y + 140); //선택(상단바)
		}

		setFontSize(10);
		drawText(col2Str(col::white) + std::to_wstring(lootCursor + 1) + L"/" + std::to_wstring(lootPocket->itemInfo.size()), lootBase.x + 6, lootBase.y + lootBase.h - 16);

	}


}