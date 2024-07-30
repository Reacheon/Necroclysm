export module Inventory_draw;

import Inventory;
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

void Inventory::drawGUI()
{
	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
		SDL_Rect newInventoryBase = inventoryBase;
		newInventoryBase.h = 164 + 32 * myMax(0, (myMin(INVENTORY_ITEM_MAX - 1, inventoryPocket->itemInfo.size() - 1)));
		drawWindowItemset(&newInventoryBase, titleInventory, titleItemSprIndex);

		drawFillRect(inventoryBase.x + 13, inventoryBase.y + 40, 50, 50, col::black);
		drawSprite(spr::inventoryItemRect, 0, inventoryBase.x + 13, inventoryBase.y + 40);

		setZoom(3.0);
		drawSpriteCenter(spr::itemset, inventoryItemData->sprIndex, inventoryBase.x + 13 + 25, inventoryBase.y + 40 + 25);
		setZoom(1.0);

		setFontSize(16);
		drawText(col2Str(col::white) + inventoryItemData->name, inventoryBase.x + 73, inventoryBase.y + 39);

		drawLine(inventoryBase.x + 72, inventoryBase.y + 63, inventoryBase.x + 72 + 255, inventoryBase.y + 63, col::gray);//회색 분리선

		SDL_Rect volumeGaugeRect = { inventoryBase.x + 123,inventoryBase.y + 72,104,9 };
		drawRect(volumeGaugeRect, col::white);
		drawFillRect({ volumeGaugeRect.x + 2,volumeGaugeRect.y + 2,50,5 }, lowCol::green);
		drawSpriteCenter(spr::icon16, 62, volumeGaugeRect.x - 47, volumeGaugeRect.y + 4);
		setFontSize(10);
		drawText(col2Str(col::white) + sysStr[18], volumeGaugeRect.x - 38, volumeGaugeRect.y - 2);//부피
		setFontSize(8);
		drawText(col2Str(col::white) + L"32.5 / 92.3 L", volumeGaugeRect.x + 110, volumeGaugeRect.y - 1);

		//좌측상단 버리기 버튼
		SDL_Rect dropBtn = { inventoryBase.x + 259,inventoryBase.y + 36,69,23 };
		drawFillRect(dropBtn, col::black);
		drawRect(dropBtn, col::gray);
		drawSpriteCenter(spr::icon16, 63, dropBtn.x + 11, dropBtn.y + 12);
		setFontSize(10);
		drawTextCenter(col2Str(col::white) + sysStr[52], dropBtn.x + dropBtn.w / 2 + 8, dropBtn.y + dropBtn.h / 2);//버리기
		drawFillRect(dropBtn, col::black, 150);


		//라벨
		SDL_Rect inventoryLabel = { inventoryBase.x + 10, inventoryBase.y + 95, 315, 26 };
		SDL_Rect inventoryLabelSelect = { inventoryLabel.x, inventoryLabel.y, 62 , 26 };
		SDL_Rect inventoryLabelName = { inventoryLabel.x + inventoryLabelSelect.w, inventoryLabel.y, 182 , 26 };
		SDL_Rect inventoryLabelQuantity = { inventoryLabel.x + inventoryLabelName.w + inventoryLabelSelect.w, inventoryLabel.y, 71 , 26 };

		drawStadium(inventoryLabel.x, inventoryLabel.y, inventoryLabel.w, inventoryLabel.h, { 0,0,0 }, 183, 5);
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
		setFontSize(13);
		drawText(col2Str(col::white) + sysStr[15], inventoryLabel.x + 10, inventoryLabel.y + 4); //선택(상단바)
		drawText(col2Str(col::white) + sysStr[16], inventoryLabel.x + 140, inventoryLabel.y + 4); //이름(상단바)
		drawText(col2Str(col::white) + sysStr[24], inventoryLabel.x + 250, inventoryLabel.y + 4); //무리량(상단바)

		SDL_Rect invenArea = { inventoryLabel.x, inventoryLabel.y + 30, 315, 200 };
		drawItemList(inventoryPocket, invenArea.x, invenArea.y, myMax(0, (myMin(INVENTORY_ITEM_MAX, inventoryPocket->itemInfo.size()))), inventoryCursor, inventoryScroll, true);

		if (inventoryPocket->itemInfo.size() == 0)
		{
			setFontSize(10);
			drawTextCenter(col2Str(col::lightGray) + sysStr[162], inventoryBase.x + 162, inventoryBase.y + 140); //가방 안에 아이템이 없다
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