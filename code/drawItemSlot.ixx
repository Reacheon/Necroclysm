#include <SDL.h>

export module drawItemSlot;

import std;
import util;
import globalVar;
import drawText;
import constVar;
import checkCursor;
import drawSprite;
import textureVar;
import ItemData;
import ItemPocket;


export enum class cursorFlag
{
	none,
	click,
	hover,
	gray,
};

//아이템이 들어있는 사각형을 그림, 질량과 부피는 포함하지않음
export void drawItemRect(cursorFlag inputCursor, int x, int y, ItemData inputItem)
{
	int fontSize = 16;
	int yCorrection = 0;
	const int widthLimit = 162;
	bool split = false;
	SDL_Rect itemBox = { x, y, 210, 26 };
	SDL_Color stadiumColor = { 0,0,0 };

	switch (inputCursor)
	{
		case cursorFlag::click:
			stadiumColor = lowCol::deepBlue;
			break;
		case cursorFlag::hover:
			stadiumColor = lowCol::blue;
			break;
	}

	drawStadium(itemBox.x, itemBox.y, itemBox.w, itemBox.h, stadiumColor, 183, 5);

	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	setFontSize(16);
	std::wstring mainName = L"";

	//장비 중인 아이템이나 갯수가 1 이하인 아이템은 갯수 표시하지 않음
	if (inputItem.equipState == equip::none && inputItem.number > 1) { mainName += std::to_wstring(inputItem.number) + L" "; }

	//아이템 이름
	mainName += inputItem.name;

	//후열 탄창이랑 탄환수 텍스트
	if (inputItem.checkFlag(itemFlag::GUN) || inputItem.checkFlag(itemFlag::MAGAZINE))
	{
		if (((ItemPocket*)inputItem.pocketPtr)->itemInfo.size() > 0)
		{
			//리볼버
			if (((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].checkFlag(itemFlag::AMMO))
			{
				int bulletNumber = 0;
				ItemPocket* gunInside = (ItemPocket*)inputItem.pocketPtr;
				for (int i = 0; i < gunInside->itemInfo.size(); i++)
				{
					bulletNumber += gunInside->itemInfo[i].number;
				}

				mainName += L" (";
				mainName += std::to_wstring(bulletNumber);
				mainName += L"/";
				mainName += std::to_wstring(inputItem.pocketMaxNumber);
				mainName += L")";
			}
			else if (((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].checkFlag(itemFlag::MAGAZINE))//탄창
			{
				int bulletNumber = 0;
				ItemPocket* magazineInside = (ItemPocket*)(((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].pocketPtr);
				for (int i = 0; i < magazineInside->itemInfo.size(); i++)
				{
					bulletNumber += magazineInside->itemInfo[i].number;
				}
				mainName += L" (";
				mainName += std::to_wstring(bulletNumber);
				mainName += L"/";
				mainName += std::to_wstring(((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].pocketMaxNumber);
				mainName += L")";
			}
		}
		else
		{
			//리볼버 종류 또는 탄창
			if (itemDex[inputItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
			{
				mainName += L" (";
				mainName += L"0";
				mainName += L"/";
				mainName += std::to_wstring(inputItem.pocketMaxNumber);
				mainName += L")";
			}
		}
	}


	setFontSize(fontSize);
	while (queryTextWidth(mainName) > widthLimit)
	{
		if (fontSize >= 14)
		{
			fontSize -= 1;
			yCorrection += 1;
			setFontSize(fontSize);
		}
		else
		{
			split = true;
			break;
		}
	}
	if (!split) drawText(col2Str(col::white) + mainName, itemBox.x + 42, itemBox.y + itemBox.h/2 - 11 + yCorrection);
	else drawTextWidth(col2Str(col::white) + mainName, itemBox.x + 42, itemBox.y + itemBox.h / 2 - 11 + yCorrection - 8, false, widthLimit, 15, 2);

	if (inputItem.checkFlag(itemFlag::GRAYFILTER)) { drawStadium(itemBox.x, itemBox.y, itemBox.w, itemBox.h, stadiumColor, 183, 5); }

	//아이템 아이콘 그리기
	setZoom(2.5);
	if (inputItem.checkFlag(itemFlag::GRAYFILTER))
	{
		SDL_SetTextureBlendMode(spr::itemset->getTexture(), SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(spr::itemset->getTexture(), 128, 128, 128);
		drawSpriteCenter(spr::itemset, inputItem.sprIndex, itemBox.x + 15, itemBox.y + itemBox.h/2);
		SDL_SetTextureColorMod(spr::itemset->getTexture(), 255, 255, 255);
	}
	else
	{
		drawSpriteCenter(spr::itemset, inputItem.sprIndex, itemBox.x + 15, itemBox.y + itemBox.h / 2);
	}
	setZoom(1.0);
}

//질량 부피 표시와 체크박스까지 포함한 확장형 아이템 박스를 그려냄
export void drawItemRectExtend(bool cursor, int x, int y, ItemData inputItem, int quantity, bool hasBox1, bool whiteCursor)
{
	//세로 32 가로폭 312의 아이템 슬롯을 그립니다.
	const int gapWidth = 6;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////     ▼BOX1(셀렉트박스) 그리기    ///////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	SDL_Rect box1 = { x, y, 36, 26 };
	SDL_Color statusColor;
	SDL_Color statusStrColor;
	std::wstring statusStr = L"-";

	if (inputItem.equipState == equip::normal)
	{
		statusColor = col::black;
		statusStr = L"E";
		statusStrColor = lowCol::green;
	}
	else if (inputItem.equipState == equip::left)
	{
		statusColor = col::black;
		statusStr = sysStr[49];//왼손
		statusStrColor = lowCol::yellow;
	}
	else if (inputItem.equipState == equip::right)
	{
		statusColor = col::black;
		statusStr = sysStr[50];
		statusStrColor = lowCol::yellow;
	}
	else if (inputItem.equipState == equip::both)
	{
		statusColor = col::black;
		statusStr = sysStr[51];
		statusStrColor = lowCol::yellow;
	}
	else if (inputItem.lootSelect == inputItem.number) //전체선택
	{
		statusColor = lowCol::deepBlue;
		statusStr = L"+";
		statusStrColor = col::white;
	}
	else if (inputItem.lootSelect == 0)
	{
		statusColor = { 0,0,0 };
		statusStr = L"-";
		statusStrColor = col::gray;
	}
	else
	{
		statusColor = lowCol::purple;
		statusStr = std::to_wstring(inputItem.lootSelect);
		statusStrColor = col::white;
	}

	if (checkCursor(&box1) && itemListColorLock == false)
	{
		if (click == false)
		{
			statusColor = lowCol::blue;
			statusStrColor = col::white;
		}
		else
		{
			statusColor = lowCol::deepBlue;
			statusStrColor = col::white;
		}
	}

	if (hasBox1 == true)
	{
		drawStadium(box1.x, box1.y, box1.w, box1.h, statusColor, 183, 5);
		int fontSize = 16;
		setFontSize(fontSize);
		while (queryTextWidth(statusStr, true) > box1.w)
		{
			fontSize--;
			setFontSize(fontSize);
		}
		drawTextCenter(col2Str(statusStrColor) + statusStr, box1.x + box1.w / 2, box1.y + box1.h / 2);
	}
	else
	{
		box1.w = 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////     ▼BOX2(메인박스) 그리기    ///////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	SDL_Rect box2 = { box1.x + box1.w + gapWidth, box1.y, 210, box1.h };
	if (cursor == true) drawItemRect(cursorFlag::hover, box2.x, box2.y, inputItem);
	else
	{
		SDL_Rect clickRect = { box2.x ,y, 312 - (box1.w + gapWidth), box2.h };
		//박스 3가 존재하지 않을 경우
		if (quantity == -1) { clickRect.w = box2.w; }

		if (checkCursor(&clickRect) && itemListColorLock == false)
		{
			if (click == false) drawItemRect(cursorFlag::hover, box2.x, box2.y, inputItem);
			else drawItemRect(cursorFlag::click, box2.x, box2.y, inputItem);
		}
		else
		{
			drawItemRect(cursorFlag::none, box2.x, box2.y, inputItem);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////     ▼BOX3(부가정보) 그리기    ///////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	if (quantity == 0)
	{
		SDL_Rect box3 = { box2.x + box2.w + gapWidth, box1.y, 54, box1.h };

		if (cursor == true) drawStadium(box3.x, box3.y, box3.w, box3.h, lowCol::blue, 183, 5);
		else
		{
			SDL_Rect clickRect = { box2.x ,y, 312 - (box1.w + gapWidth), box2.h };
			//박스 3가 존재하지 않을 경우
			if (quantity == -1) { clickRect.w = box2.w; }

			if (checkCursor(&clickRect) && itemListColorLock == false)
			{
				if (click == false)  drawStadium(box3.x, box3.y, box3.w, box3.h, lowCol::blue, 183, 5);
				else  drawStadium(box3.x, box3.y, box3.w, box3.h, lowCol::deepBlue, 183, 5);
			}
			else
			{
				drawStadium(box3.x, box3.y, box3.w, box3.h, col::black, 183, 5);
			}
		}



		drawSpriteCenter(spr::icon13, 29, box3.x + 10, box2.y + 9 - 4);
		drawSpriteCenter(spr::icon13, 30, box3.x + 10, box2.y + 9 + 16 - 7);

		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(7);

		std::wstring kgStr = decimalCutter(inputItem.weight / 1000.0, 2);
		std::wstring volStr = decimalCutter(inputItem.volume / 1000.0, 2);
		drawText(kgStr + L" KG", box3.x + 10 + 9, box2.y + 13 - 7 - 4);
		drawText(volStr + L" L", box3.x + 10 + 9, box2.y + 13 + 16 - 7 - 7);
	}

	if (whiteCursor == true)
	{
		int cursorIndex = 0;
		{
			if (timer::timer600 % 30 < 5) { cursorIndex = 0; }
			else if (timer::timer600 % 30 < 10) { cursorIndex = 1; }
			else if (timer::timer600 % 30 < 15) { cursorIndex = 2; }
			else if (timer::timer600 % 30 < 20) { cursorIndex = 3; }
			else { cursorIndex = 0; }
		}
		if (cursor == true)
		{
			if (quantity == -1 && hasBox1 == false)
			{
				drawSprite(spr::itemCursorShort, cursorIndex, box2.x - 16, box2.y - 16);
			}
			else
			{
				drawSprite(spr::itemCursorLong, cursorIndex, box2.x - 16, box2.y - 16);
			}
		}
	}

}

//심플 아이템렉트를 그립니다. 텍스트를 입력하는 타입.
export void drawSimpleItemRect(cursorFlag inputCursor, int x, int y, int iconIndex, std::wstring text, bool gray)
{
	int fontSize = 12;
	int yCorrection = 0;
	const int widthLimit = 170;
	bool split = false;
	SDL_Rect itemBox = { x, y, 210, 18 };

	int btnColorSprIndex = 0;
	switch (inputCursor)
	{
	case cursorFlag::click:
		btnColorSprIndex = 2;
		break;
	case cursorFlag::hover:
		btnColorSprIndex = 1;
		break;
	}
	drawSprite(spr::itemSlotBtn, btnColorSprIndex, itemBox.x, itemBox.y);

	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	setFontSize(16);
	std::wstring mainName = text;

	//사각형에 맞지않으면 폰트를 점차 줄임(최대 5까지)
	setFontSize(fontSize);
	while (queryTextWidthColorCode(mainName) > widthLimit)
	{
		if (fontSize > 6)
		{
			fontSize -= 1;
			yCorrection += 1;
			setFontSize(fontSize);
		}
		else break;
	}

	drawText(mainName, itemBox.x + 34, itemBox.y + 1 + yCorrection);

	if (gray)
	{
		SDL_SetTextureBlendMode(spr::itemSlotBtn->getTexture(), SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(spr::itemSlotBtn->getTexture(), 128, 128, 128);
		drawSpriteCenter(spr::itemSlotBtn, 0, itemBox.x, itemBox.y);
		SDL_SetTextureColorMod(spr::itemSlotBtn->getTexture(), 255, 255, 255);
	}

	//아이템 아이콘 그리기
	setZoom(1.5);
	if (gray)
	{
		SDL_SetTextureBlendMode(spr::itemset->getTexture(), SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(spr::itemset->getTexture(), 128, 128, 128);
		drawSpriteCenter(spr::itemset, iconIndex, itemBox.x + 15, itemBox.y + 9);
		SDL_SetTextureColorMod(spr::itemset->getTexture(), 255, 255, 255);
	}
	else
	{
		drawSpriteCenter(spr::itemset, iconIndex, itemBox.x + 15, itemBox.y + 9);
	}
	setZoom(1.0);
}

//심플 아이템렉트를 그립니다. 아이템 정보를 입력하는 타입. 무게나 부피 표시 없음
export void drawSimpleItemRect(cursorFlag inputCursor, int x, int y, ItemData inputItem)
{

	std::wstring mainName = L"";

	//장비 중인 아이템이나 갯수가 1 이하인 아이템은 갯수 표시하지 않음
	if (inputItem.equipState == equip::none && inputItem.number > 1) { mainName += std::to_wstring(inputItem.number) + L" "; }

	//아이템 이름
	mainName += inputItem.name;

	//후열 탄창이랑 탄환수 텍스트
	if (inputItem.checkFlag(itemFlag::GUN) || inputItem.checkFlag(itemFlag::MAGAZINE))
	{
		if (((ItemPocket*)inputItem.pocketPtr)->itemInfo.size() > 0)
		{
			//리볼버
			if (((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].checkFlag(itemFlag::AMMO))
			{
				int bulletNumber = 0;
				ItemPocket* gunInside = (ItemPocket*)inputItem.pocketPtr;
				for (int i = 0; i < gunInside->itemInfo.size(); i++)
				{
					bulletNumber += gunInside->itemInfo[i].number;
				}

				mainName += L" (";
				mainName += std::to_wstring(bulletNumber);
				mainName += L"/";
				mainName += std::to_wstring(inputItem.pocketMaxNumber);
				mainName += L")";
			}
			else if (((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].checkFlag(itemFlag::MAGAZINE))//탄창
			{
				int bulletNumber = 0;
				ItemPocket* magazineInside = (ItemPocket*)(((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].pocketPtr);
				for (int i = 0; i < magazineInside->itemInfo.size(); i++)
				{
					bulletNumber += magazineInside->itemInfo[i].number;
				}
				mainName += L" (";
				mainName += std::to_wstring(bulletNumber);
				mainName += L"/";
				mainName += std::to_wstring(((ItemPocket*)inputItem.pocketPtr)->itemInfo[0].pocketMaxNumber);
				mainName += L")";
			}
		}
		else
		{
			//리볼버 종류 또는 탄창
			if (itemDex[inputItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
			{
				mainName += L" (";
				mainName += L"0";
				mainName += L"/";
				mainName += std::to_wstring(inputItem.pocketMaxNumber);
				mainName += L")";
			}
		}
	}

	drawSimpleItemRect(inputCursor, x, y, inputItem.sprIndex, col2Str(col::white) + mainName, inputItem.checkFlag(itemFlag::GRAYFILTER));

}

export void drawSimpleItemRectAdd(cursorFlag inputCursor, int x, int y)
{
	switch (inputCursor)
	{
	default:
		drawSprite(spr::itemSlotBtn, 9, x, y);
		break;
	case cursorFlag::hover:
		drawSprite(spr::itemSlotBtn, 10, x, y);
		break;
	case cursorFlag::click:
		drawSprite(spr::itemSlotBtn, 11, x, y);
		break;
	}
	drawSpriteCenter(spr::icon16, 40, x + spr::itemSlotBtn->getW() / 2, y + spr::itemSlotBtn->getH() / 2);
}



