#include <SDL3/SDL.h>

import Equip;
import util;
import globalVar;
import World;
import constVar;
import textureVar;
import Player;
import drawText;
import checkCursor;
import drawWindow;
import drawSprite;
import Item;
import CoordSelect;
import GUI;

void Equip::drawGUI()
{
	if (getStateDraw() == false) { return; }
	if (CoordSelect::ins() != nullptr) return;

	if (getFoldRatio() == 1.0)
	{
	drawWindow(&equipBase, sysStr[242], 94);
	setFlip(SDL_FLIP_HORIZONTAL);
	drawSprite(spr::newWindowArrow, 0, equipBase.x + equipBase.w - 4, equipBase.y + 255);
    setFlip(SDL_FLIP_NONE);

	//여기서부턴 이큅 윈도우
	{
		//플레이어 무게 제한 게이지 그리기
		SDL_Rect weightGaugeRect = { equipBase.x + 62, equipBase.y + 46, 125, 11 };
		drawRect(weightGaugeRect, col::white);
		drawFillRect(SDL_Rect{ weightGaugeRect.x + 2, weightGaugeRect.y + 2, 50, 6 }, lowCol::green);
		drawSpriteCenter(spr::icon16, 61, weightGaugeRect.x - 40, weightGaugeRect.y + 6);
		setFontSize(12);
		drawText(sysStr[102], weightGaugeRect.x - 31, weightGaugeRect.y);//무게
		setFontSize(10);
		drawText(L"32.5 / 92.3 kg", weightGaugeRect.x + 132, weightGaugeRect.y);


		//이큅 윈도우 본체
		panel.drawCursorInfo(equipBase.x + 7, equipBase.y + equipBase.h - 20);

		//상단바 라벨(선택 이름 물리량)
		panel.drawLabelBar(GUI::getLastGUI() == this);

		//개별 아이템
		itemListColorLock = (GUI::getLastGUI() != this);
		panel.drawList(equipArea.x, equipArea.y, isTargetPocket == false);

		if (panel.pocket->itemInfo.size() == 0) // 만약 아이템이 없을 경우
		{
			drawTextCenter(sysStr[52], equipArea.x + equipArea.w / 2, equipArea.y + equipArea.h / 2);
		}

		//스크롤바
		panel.drawScrollbar(equipBase.x + 391, panel.itemRect[0].y,
			panel.itemRect[EQUIP_ITEM_MAX - 1].y + panel.itemRect[EQUIP_ITEM_MAX - 1].h - panel.itemRect[0].y);


		if (GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1] != this)
		{
			drawFillRect(equipBase, col::black, 150);
		}
	}



	}
	else
	{
		SDL_Rect vRect = equipBase;
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
