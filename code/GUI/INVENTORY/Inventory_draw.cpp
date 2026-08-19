#include <SDL3/sdl.h>

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
import drawItemSlot;
import CoordSelect;
import GUI;

void Inventory::drawGUI()
{
	if (getStateDraw() == false) { return; }
	if (CoordSelect::ins() != nullptr) return;

	if (getFoldRatio() == 1.0)
	{
		setFont(fontType::mainFont);

		SDL_Rect newInventoryBase = inventoryBase;
		newInventoryBase.h = panel.calcWindowHeight();
		drawWindowItemset(&newInventoryBase, titleInventory, titleItemSprIndex);

		drawFillRect(inventoryBase.x + 16, inventoryBase.y + 44, 64, 64, col::black);

		drawSpriteCenter(spr::itemBackgroundRect, 0, inventoryBase.x + 16 + 32, inventoryBase.y + 44 + 32);

		setZoom(4.0);
		drawSpriteCenter(spr::itemset, inventoryItemData->getSprIndex(), inventoryBase.x + 16 + 32, inventoryBase.y + 44 + 32);
		setZoom(1.0);

		setFontSize(24);
		drawText(inventoryItemData->name, inventoryBase.x + 88, inventoryBase.y + 46);

		drawLine(inventoryBase.x + 86, inventoryBase.y + 81, inventoryBase.x + 86 + 307, inventoryBase.y + 81, col::gray);

		//부피 게이지
		drawVolumeGauge(inventoryBase.x + 85, inventoryBase.y + 87, *inventoryItemData);

		// 선택된 아이템이 있는지 확인
		bool hasSelectedItems = panel.hasAnySelection();

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
		drawTextCenter(sysStr[35], dropBtn.x + dropBtn.w / 2 + 14, dropBtn.y + dropBtn.h / 2 - 2);//버리기

		// 선택된 아이템이 없으면 비활성화 효과
		if (!hasSelectedItems)
		{
			drawFillRect(dropBtn, col::black, 150);
		}

		//라벨
		panel.drawLabelBar(GUI::getLastGUI() == this);

		//개별 아이템
		itemListColorLock = (GUI::getLastGUI() != this);
		panel.drawList(panel.label.x, panel.label.y + 36, true);

		//스크롤바
		panel.drawScrollbar(inventoryBase.x + 391, panel.label.y + 36, 296);

		//커서 정보
		panel.drawCursorInfo(inventoryBase.x + 7, inventoryBase.y + inventoryBase.h - 19);

		//빈 리스트 메시지
		panel.drawEmptyMsg(inventoryBase.x + 195, inventoryBase.y + 168);

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
