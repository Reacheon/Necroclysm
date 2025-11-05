export module Inventory_draw;

import Inventory;
import globalVar;
import wrapVar;
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
import CoordSelect;

void Inventory::drawGUI()
{
	if (getStateDraw() == false) { return; }
	if (CoordSelect::ins() != nullptr) return;

	if (getFoldRatio() == 1.0)
	{
		SDL_Rect newInventoryBase = inventoryBase;
		newInventoryBase.h = 197 + 38 * myMax(0, (myMin(INVENTORY_ITEM_MAX - 1, inventoryPocket->itemInfo.size() - 1)));
		drawWindowItemset(&newInventoryBase, titleInventory, titleItemSprIndex);

		drawFillRect(inventoryBase.x + 16, inventoryBase.y + 44, 64, 64, col::black);
		drawSprite(spr::inventoryItemRect, 0, inventoryBase.x + 16, inventoryBase.y + 44);


		setZoom(4.0);
		drawSpriteCenter(spr::itemset, getItemSprIndex(*inventoryItemData), inventoryBase.x + 16 + 32, inventoryBase.y + 44 + 32);
		setZoom(1.0);

		setFontSize(24);
		drawText(inventoryItemData->name, inventoryBase.x + 88, inventoryBase.y + 42);

		drawLine(inventoryBase.x + 86, inventoryBase.y + 81, inventoryBase.x + 86 + 307, inventoryBase.y + 81, col::gray);


		SDL_Rect volumeGaugeRect = { inventoryBase.x + 148, inventoryBase.y + 90, 125, 11 };
		drawRect(volumeGaugeRect, col::white);
		drawSpriteCenter(spr::icon16, 62, volumeGaugeRect.x - 56, volumeGaugeRect.y + 5);
		setFontSize(13);
		drawText(sysStr[18], volumeGaugeRect.x - 54, volumeGaugeRect.y - 5);//부피

		 //Inventory에도 같은 코드가 존재
		{
			ItemPocket* pkPtr = inventoryItemData->pocketPtr.get();
			if (inventoryItemData->pocketMaxVolume > 0)
			{
				int currentVolume = 0;
				for (int i = 0; i < pkPtr->itemInfo.size(); i++) currentVolume += getVolume(pkPtr->itemInfo[i]) * (pkPtr->itemInfo[i].number);
				float volumeRatio = (float)currentVolume / (float)inventoryItemData->pocketMaxVolume;
				SDL_Color gaugeCol = lowCol::green;
				if (volumeRatio > 0.6) gaugeCol = lowCol::yellow;
				else if (volumeRatio > 0.9) gaugeCol = lowCol::red;
				drawFillRect(SDL_Rect{ volumeGaugeRect.x + 2, volumeGaugeRect.y + 2, static_cast<int>(120.0 * volumeRatio), 6 }, gaugeCol);

				std::wstring currentVolumeStr = decimalCutter((float)currentVolume / 1000.0, 1);
				std::wstring maxVolumeStr = decimalCutter((float)inventoryItemData->pocketMaxVolume / 1000.0, 1) + L" L";
				setFontSize(13);
				drawText(currentVolumeStr + L" / " + maxVolumeStr, volumeGaugeRect.x + 132, volumeGaugeRect.y - 5);
			}
			else if (inventoryItemData->pocketMaxNumber > 0)
			{
				int currentNumber = 0;
				for (int i = 0; i < pkPtr->itemInfo.size(); i++) currentNumber += pkPtr->itemInfo[i].number;
				float volumeRatio = (float)currentNumber / (float)inventoryItemData->pocketMaxNumber;
				SDL_Color gaugeCol = lowCol::green;
				if (volumeRatio > 0.6) gaugeCol = lowCol::yellow;
				else if (volumeRatio > 0.9) gaugeCol = lowCol::red;
				drawFillRect(SDL_Rect{ volumeGaugeRect.x + 2, volumeGaugeRect.y + 2, static_cast<int>(120.0 * volumeRatio), 6 }, gaugeCol);

				std::wstring currentVolumeStr = decimalCutter((float)currentNumber / 1000.0, 1);
				std::wstring maxVolumeStr = decimalCutter((float)inventoryItemData->pocketMaxNumber / 1000.0, 1);
				setFontSize(13);
				drawText(currentVolumeStr + L" / " + maxVolumeStr, volumeGaugeRect.x + 132, volumeGaugeRect.y - 5);
			}
		}




		// 선택된 아이템이 있는지 확인
		bool hasSelectedItems = false;
		for (int i = 0; i < inventoryPocket->itemInfo.size(); i++)
		{
			if (inventoryPocket->itemInfo[i].lootSelect > 0)
			{
				hasSelectedItems = true;
				break;
			}
		}

		SDL_Color btnColor = { 0x00, 0x00, 0x00 };
		SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };

		if (hasSelectedItems && checkCursor(&dropBtn))
		{
			if (click == false)
			{
				btnColor = lowCol::blue;
			}
			else
			{
				btnColor = lowCol::deepBlue;
			}
			outlineColor = { 0xa6, 0xa6, 0xa6 };
		}

		drawFillRect(dropBtn, btnColor);
		drawRect(dropBtn, outlineColor);
		setZoom(2.0);
		drawSpriteCenter(spr::icon16, 63, dropBtn.x + 20, dropBtn.y + 18);
		setZoom(1.0);
		setFontSize(20);
		drawTextCenter(sysStr[52], dropBtn.x + dropBtn.w / 2 + 14, dropBtn.y + dropBtn.h / 2 - 2);//버리기

		// 선택된 아이템이 없으면 비활성화 효과
		if (!hasSelectedItems)
		{
			drawFillRect(dropBtn, col::black, 150);
		}


		//라벨
		SDL_Rect inventoryLabel = { inventoryBase.x + 12, inventoryBase.y + 114, 376, 31 };
		SDL_Rect inventoryLabelSelect = { inventoryLabel.x, inventoryLabel.y, 75 , 31 };
		SDL_Rect inventoryLabelName = { inventoryLabel.x + inventoryLabelSelect.w, inventoryLabel.y, 219 , 31 };
		SDL_Rect inventoryLabelQuantity = { inventoryLabel.x + inventoryLabelName.w + inventoryLabelSelect.w, inventoryLabel.y, 85 , 31 };

		drawStadium(inventoryLabel.x, inventoryLabel.y, inventoryLabel.w, inventoryLabel.h, { 0,0,0 }, 183, 5);
		if (GUI::getLastGUI() == this)
		{
			if (checkCursor(&inventoryLabelSelect))
			{
				drawStadium(inventoryLabelSelect.x, inventoryLabelSelect.y, inventoryLabelSelect.w, inventoryLabelSelect.h, lowCol::blue, 183, 5);
			}
			else if (checkCursor(&inventoryLabelName))
			{
				drawStadium(inventoryLabelName.x, inventoryLabelName.y, inventoryLabelName.w, inventoryLabelName.h, lowCol::blue, 183, 5);
			}
			else if (checkCursor(&inventoryLabelQuantity))
			{
				drawStadium(inventoryLabelQuantity.x, inventoryLabelQuantity.y, inventoryLabelQuantity.w, inventoryLabelQuantity.h, lowCol::blue, 183, 5);
			}
		}
		setFontSize(14);
		drawTextCenter(sysStr[15], inventoryLabel.x + 30, inventoryLabel.y + 14); //선택(상단바)
		drawTextCenter(sysStr[16], inventoryLabel.x + 183, inventoryLabel.y + 14); //이름(상단바)
		drawTextCenter(sysStr[24], inventoryLabel.x + 337, inventoryLabel.y + 14); //무리량(상단바)

		SDL_Rect invenArea = { inventoryLabel.x, inventoryLabel.y + 36, 376, 296 };

		if (GUI::getLastGUI() != this) itemListColorLock = true;
		else  itemListColorLock = false;
		drawItemList(inventoryPocket, invenArea.x, invenArea.y, myMax(0, (myMin(INVENTORY_ITEM_MAX, inventoryPocket->itemInfo.size()))), inventoryCursor, inventoryScroll, true);

		if (inventoryPocket->itemInfo.size() > INVENTORY_ITEM_MAX)
		{
			SDL_Rect inventoryScrollBox = { inventoryBase.x + 391, inventoryLabel.y + 36, 2, invenArea.h };
			drawFillRect(inventoryScrollBox, { 120, 120, 120 });

			SDL_Rect inScrollBox = inventoryScrollBox;
			inScrollBox.h = inventoryScrollBox.h * myMin(1.0, (double)INVENTORY_ITEM_MAX / inventoryPocket->itemInfo.size());
			if (inScrollBox.h < 5) inScrollBox.h = 5;

			if (!inventoryPocket->itemInfo.empty())
				inScrollBox.y = inventoryScrollBox.y + inventoryScrollBox.h * ((float)inventoryScroll / (float)inventoryPocket->itemInfo.size());
			else
				inScrollBox.y = inventoryScrollBox.y;

			if (inScrollBox.y < inventoryScrollBox.y) inScrollBox.y = inventoryScrollBox.y;
			if (inScrollBox.y + inScrollBox.h > inventoryScrollBox.y + inventoryScrollBox.h)
				inScrollBox.y = inventoryScrollBox.y + inventoryScrollBox.h - inScrollBox.h;

			drawFillRect(inScrollBox, col::white);
		}

		setFontSize(12);
		drawText(std::to_wstring(inventoryCursor + 1) + L"/" + std::to_wstring(inventoryPocket->itemInfo.size()),
			inventoryBase.x + 7, inventoryBase.y + inventoryBase.h - 19);

		if (inventoryPocket->itemInfo.size() == 0)
		{
			setFontSize(12);
			drawTextCenter(sysStr[162], inventoryBase.x + 195, inventoryBase.y + 168, col::lightGray); //가방 안에 아이템이 없다
		}

	}
	else
	{
		SDL_Rect vRect = inventoryBase;
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