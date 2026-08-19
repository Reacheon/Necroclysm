#include <SDL3/SDL.h>

import std;
import Loot;

import globalVar;
import World;
import constVar;
import textureVar;
import util;
import Sprite;
import Player;
import drawWindow;
import drawSprite;
import checkCursor;
import drawText;
import drawItemSlot;
import GUI;
import Item;

void Loot::drawGUI()
{
	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
	const bool* state = SDL_GetKeyboardState(nullptr);
	Sprite* targetBtnSpr = nullptr;

	bool hasSelect = panel.hasAnySelection();

	std::wstring windowTitle = sysStr[5];

	if (hasSelect == true)
	{
		ItemPocket* equipPtr = PlayerEquip();
		std::vector<int> pocketList;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr) pocketList.push_back(i);
		}
		int numberOfBag = pocketList.size();

		if (numberOfBag > 0 && pocketCursor >= 0 && pocketCursor < numberOfBag)
		{
			std::wstring pocketName = equipPtr->itemInfo[pocketList[pocketCursor]].name;
			windowTitle = sysStr[5] + L" ▶ " + pocketName;
		}
	}
	drawWindow(&lootBase, windowTitle, 1);
	if(arrowDir == dir16::left) drawSprite(spr::newWindowArrow, 0, lootBase.x - 26, lootBase.y + 145);
	else if (arrowDir == dir16::right)
	{
		setFlip(SDL_FLIP_HORIZONTAL);
		drawSprite(spr::newWindowArrow, 0, lootBase.x + lootBase.w - 4, lootBase.y + 145);
		setFlip(SDL_FLIP_NONE);
	}



	//포켓
	if (hasSelect == false)
	{
		drawFillRect(lootBase.x + 16, lootBase.y + 44, 64, 64, col::black);
		//drawSprite(spr::inventoryItemRect, 0, lootBase.x + 16, lootBase.y + 48);

		//타일 또는 타일에 있는 포켓 아이템 대표 스프라이트 그리기


		int tileIndex = 140;
		std::wstring tileName = L"Tile name";
		Sprite* tileSpr = spr::itemset;

		drawSpriteCenter(spr::itemBackgroundRect, 0, lootBase.x + 16 + 32, lootBase.y + 44 + 32);


		if (lootItemData != nullptr)
		{
			//프롭/차량부품 컨테이너는 인벤토리 아이콘(itemset)이 아니라 실제 설치 이미지로 표시.
			//차량 부품은 vehset/vehSprIndex, 그 외 프롭은 propset/propSprIndex 기준.
			const bool isVehPart = (lootItemData->category == itemCategory::vehicles);
			tileSpr = isVehPart ? spr::vehset : spr::propset;
			int baseIndex = isVehPart ? lootItemData->vehSprIndex : lootItemData->propSprIndex;
			tileIndex = baseIndex + lootItemData->extraSprIndexSingle + 16 * lootItemData->extraSprIndex16;
			//루팅창이 열려 있는 동안 = 프롭이 열린 상태이므로 문 열린 스프라이트(+1)로 표시
			if (lootItemData->checkFlag(itemFlag::PROP_POCKET_OPEN_SPRITE)) tileIndex += 1;
			tileName = lootItemData->name;
		}
		else if (lootStack != nullptr)
		{
			Point3 targetTile = { lootStack->getGridX(), lootStack->getGridY(), lootStack->getGridZ() };
			int floorIndex = TileFloor(targetTile.x, targetTile.y, targetTile.z);
			tileIndex = itemDex[floorIndex].getSprIndex();
			tileName = itemDex[floorIndex].name;
		}
		setZoom(3.0);
		drawSpriteCenter(tileSpr, tileIndex, lootBase.x + 16 + 32, lootBase.y + 44 + 34);
		setZoom(1.0);

		setFontSize(24);
		drawText(tileName, lootBase.x + 88, lootBase.y + 50);

		drawLine(lootBase.x + 86, lootBase.y + 81, lootBase.x + 86 + 307, lootBase.y + 81, col::gray);

		if (lootStack == nullptr && lootItemData != nullptr)
		{
			drawVolumeGauge(lootBase.x + 85, lootBase.y + 87, *lootItemData);
		}
		else if (lootStack != nullptr)
		{
			int pivotX = lootBase.x + 85;
			int pivotY = lootBase.y + 87;
			drawSprite(spr::icon16, 66, pivotX, pivotY);

			setFontSize(13);
			drawText(sysStr[13], pivotX + 19, pivotY +1);

			SDL_Rect volumeGaugeRect = { pivotX + 55, pivotY + 3, 125, 11 };
			drawRect(volumeGaugeRect, col::white);

			setFont(fontType::pixel);
			setFontSize(14);
			drawText(L"∞", volumeGaugeRect.x + 132 - 77, volumeGaugeRect.y - 2);
			setFont(fontType::mainFont);
		}

		//좌측상단 버리기 버튼
		SDL_Rect dropBtn = { lootBase.x + 299, lootBase.y + 40, 100, 35 };
		drawFillRect(dropBtn, col::black);
		drawRect(dropBtn, col::gray);
		setZoom(2.0);
		drawSpriteCenter(spr::icon16, 63, dropBtn.x + 20, dropBtn.y + 18);
		setZoom(1.0);
		setFontSize(18);
		drawTextCenter(sysStr[35], dropBtn.x + dropBtn.w / 2 + 14, dropBtn.y + dropBtn.h / 2 + 1);
		drawFillRect(dropBtn, col::black, 150);
	}
	else
	{
		//가방이 몇 개 있는지 체크
		std::vector<int> pocketList;
		int numberOfBag = 0;
		ItemPocket* equipPtr = PlayerEquip();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				pocketList.push_back(i);
				numberOfBag++;
			}
		}

		if (numberOfBag == 0)
		{
			setFontSize(14);
			drawTextCenter(sysStr[14], pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + 18, col::lightGray);
		}
		else
		{
			SDL_Rect pocketItem[7];
			pocketItem[3] = { lootBase.x + 201 - 19, lootBase.y + 60 - 19, 38, 38 };

			pocketItem[2] = { lootBase.x + 201 - 19 - 36 - 32 * 0, lootBase.y + 60 - 19 + 6, 29, 29 };
			pocketItem[1] = { lootBase.x + 201 - 19 - 36 - 32 * 1, lootBase.y + 60 - 19 + 6, 29, 29 };
			pocketItem[0] = { lootBase.x + 201 - 19 - 36 - 32 * 2, lootBase.y + 60 - 19 + 6, 29, 29 };

			pocketItem[4] = { lootBase.x + 201 - 19 + 46 + 32 * 0, lootBase.y + 60 - 19 + 6, 29, 29 };
			pocketItem[5] = { lootBase.x + 201 - 19 + 46 + 32 * 1, lootBase.y + 60 - 19 + 6, 29, 29 };
			pocketItem[6] = { lootBase.x + 201 - 19 + 46 + 32 * 2, lootBase.y + 60 - 19 + 6, 29, 29 };

			//포켓 1~3번째 칸 그리기
			if (pocketCursor != 0)
			{
				setZoom(1.8);
				drawFillRect(pocketItem[2].x, pocketItem[2].y, pocketItem[2].w, pocketItem[2].h, { 0,0,0 }, 200);
				drawRect(pocketItem[2], col::gray);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 1]].getSprIndex(), pocketItem[2].x + (pocketItem[2].w / 2), pocketItem[2].y + (pocketItem[2].h / 2));
				if (pocketCursor != 1)
				{
					drawFillRect(pocketItem[1].x, pocketItem[1].y, pocketItem[1].w, pocketItem[1].h, { 0,0,0 }, 200);
					drawRect(pocketItem[1], col::gray);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 2]].getSprIndex(), pocketItem[1].x + (pocketItem[1].w / 2), pocketItem[1].y + (pocketItem[1].h / 2));
					if (pocketCursor != 2)
					{
						drawFillRect(pocketItem[0].x, pocketItem[0].y, pocketItem[0].w, pocketItem[0].h, { 0,0,0 }, 200);
						drawRect(pocketItem[0], col::gray);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 3]].getSprIndex(), pocketItem[0].x + (pocketItem[0].w / 2), pocketItem[0].y + (pocketItem[0].h / 2));
					}
				}
			}

			//포켓 4번째 칸
			setZoom(2.4);
			drawFillRect(pocketItem[3].x, pocketItem[3].y, pocketItem[3].w, pocketItem[3].h, lowCol::blue, 200);
			drawRect(pocketItem[3], col::white);
			drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor]].getSprIndex(), lootBase.x + 201, lootBase.y + 60);

			//포켓 5~7번째 칸
			if (pocketCursor != numberOfBag - 1)
			{
				setZoom(1.8);
				drawFillRect(pocketItem[4].x, pocketItem[4].y, pocketItem[4].w, pocketItem[4].h, { 0,0,0 }, 200);
				drawRect(pocketItem[4], col::gray);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 1]].getSprIndex(), pocketItem[4].x + (pocketItem[4].w / 2), pocketItem[4].y + (pocketItem[4].h / 2));
				if (pocketCursor != numberOfBag - 2)
				{
					drawFillRect(pocketItem[5].x, pocketItem[5].y, pocketItem[5].w, pocketItem[5].h, { 0,0,0 }, 200);
					drawRect(pocketItem[5], col::gray);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 2]].getSprIndex(), pocketItem[5].x + (pocketItem[5].w / 2), pocketItem[5].y + (pocketItem[5].h / 2));
					if (pocketCursor != numberOfBag - 3)
					{
						drawFillRect(pocketItem[6].x, pocketItem[6].y, pocketItem[6].w, pocketItem[6].h, { 0,0,0 }, 200);
						drawRect(pocketItem[6], col::gray);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 3]].getSprIndex(), pocketItem[6].x + (pocketItem[6].w / 2), pocketItem[6].y + (pocketItem[6].h / 2));
					}
				}
			}
			setZoom(1.0);

			//포켓 질량 게이지
			SDL_Rect weightBar = { pocketWindow.x + 50, pocketWindow.y + 62, 86, 5 };
			drawRect(weightBar, col::white);

			setFontSize(12);
			drawTextCenter(L"132.9/99.9 KG", weightBar.x + (weightBar.w / 2), weightBar.y - 10);

			//루팅 주머니 부피 게이지
			{
				SDL_Rect volumeBar = { pocketWindow.x + pocketWindow.w - 137, pocketWindow.y + 62, 86, 5 };
				drawRect(volumeBar, col::white);
				SDL_Rect volumeGauge = { volumeBar.x + 1, volumeBar.y + 1, volumeBar.w - 2, 2 };
				int maxVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketMaxVolume;

				int currentVolume = 0;

				volumeGauge.w = (volumeBar.w - 2) * ((float)currentVolume / (float)maxVolume);

				std::wstring volumeStr = decimalCutter(currentVolume / 1000.0, 2) + L"/" + decimalCutter(maxVolume / 1000.0, 2) + L" L";
				setFontSize(12);
				drawTextCenter(volumeStr, volumeBar.x + (volumeBar.w / 2), volumeBar.y - 10);
			}

			//포켓 좌우 변경 버튼은 이미 changeXY에서 수정됨
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

				if (option::inputMethod == input::gamepad)
				{
					if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_L1, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
				}
				else
				{
					drawSpriteCenter(spr::windowArrow, 2, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketLeft.h / 2);
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

				if (option::inputMethod == input::gamepad)
				{
					if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
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

		//루팅버튼 그리기 (lootBtn은 changeXY에서 이미 수정됨)
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

			if (option::inputMethod == input::gamepad)
			{
				if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_NORTH)) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_TRI, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}
			else
			{
				drawSpriteCenter(spr::lootBagArrow, 1, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}

			if (hasSelect == false) drawStadium(lootBtn.x, lootBtn.y, lootBtn.w, lootBtn.h, lootBtnColor, 200, 5);
		}
	}

	//여기서부턴 루팅 윈도우
	{
		setFont(fontType::mainFont);
		//우측 아이템 상단바 라벨(선택 이름 물리량)
		drawStadium(panel.label.x, panel.label.y, panel.label.w, panel.label.h, { 0,0,0 }, 183, 5);
		if (GUI::getLastGUI() == this)
		{
			if (checkCursor(&panel.labelSelect) || labelCursor == 0)
			{
				SDL_Color btnColor;
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				drawStadium(panel.labelSelect.x, panel.labelSelect.y, panel.labelSelect.w, panel.labelSelect.h, btnColor, 183, 5);
			}
			else if (checkCursor(&panel.labelName) || labelCursor == 1)
			{
				SDL_Color btnColor;
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				drawStadium(panel.labelName.x, panel.labelName.y, panel.labelName.w, panel.labelName.h, btnColor, 183, 5);
			}
			else if (checkCursor(&panel.labelQuantity) || labelCursor == 2)
			{
				SDL_Color btnColor;
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				drawStadium(panel.labelQuantity.x, panel.labelQuantity.y, panel.labelQuantity.w, panel.labelQuantity.h, btnColor, 183, 5);
			}
		}
		setFontSize(14);
		drawTextCenter(sysStr[10], panel.label.x + 30, panel.label.y + 14);

		{
			std::wstring tailStr = L"";
			int grayNumber = 0;
			for (int i = 0; i < panel.pocket->itemInfo.size(); i++)
			{
				if (panel.pocket->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))
				{
					grayNumber++;
				}
			}

			if (grayNumber > 0)
			{
				int whiteNumber = panel.pocket->itemInfo.size() - grayNumber;
				tailStr = L" (" + std::to_wstring(whiteNumber) + L" " + sysStr[50] + L")";
			}

			drawTextCenter(sysStr[11] + tailStr, panel.label.x + 183, panel.label.y + 14);
		}

		switch (panel.sortType)
		{
		default:
			drawTextCenter(sysStr[15], panel.label.x + 337, panel.label.y + 14);
			break;
		case sortFlag::weightDescend:
			drawTextCenter(sysStr[28], panel.label.x + 337, panel.label.y + 14);
			break;
		case sortFlag::weightAscend:
			drawTextCenter(sysStr[29], panel.label.x + 337, panel.label.y + 14);
			break;
		case sortFlag::volumeDescend:
			drawTextCenter(sysStr[30], panel.label.x + 337, panel.label.y + 14);
			break;
		case sortFlag::volumeAscend:
			drawTextCenter(sysStr[31], panel.label.x + 337, panel.label.y + 14);
			break;
		}

		//개별 아이템
		itemListColorLock = (GUI::getLastGUI() != this);
		panel.drawList(lootArea.x, lootArea.y, true);

		//스크롤바
		panel.drawScrollbar(lootBase.x + 391, panel.itemRect[0].y, lootBase.h - 173);

		//빈 리스트 메시지
		panel.drawEmptyMsg(lootBase.x + 195, lootBase.y + 168);

		//커서 정보
		panel.drawCursorInfo(lootBase.x + 10, lootBase.y + lootBase.h - 16);
	}
	}
	else
	{
		SDL_Rect vRect = lootBase;
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
