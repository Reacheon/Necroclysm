#include <SDL.h>

import Craft;

import globalVar;
import wrapVar;
import textureVar;
import constVar;
import drawWindow;
import drawSprite;
import drawText;
import util;
import checkCursor;
import Msg;
import const2Str;
import drawItemSlot;
import Player;


void Craft::drawGUI()
{
	if (showCraftingTooltip)
	{
		SDL_Rect tooltipBox = { cameraW / 2 - 140, 0, 280,130 };
		drawWindow(&tooltipBox);

		setZoom(1.0);

		int markerIndex = 0;
		if (Msg::ins() == nullptr)
		{
			if (timer::timer600 % 25 < 2) { markerIndex = 0; }
			else if (timer::timer600 % 25 < 4) { markerIndex = 1; }
			else if (timer::timer600 % 25 < 6) { markerIndex = 2; }
			else if (timer::timer600 % 25 < 8) { markerIndex = 3; }
			else if (timer::timer600 % 25 < 10) { markerIndex = 4; }
			else if (timer::timer600 % 25 < 12) { markerIndex = 5; }
			else if (timer::timer600 % 25 < 14) { markerIndex = 6; }
			else if (timer::timer600 % 25 < 16) { markerIndex = 7; }
			else if (timer::timer600 % 25 < 18) { markerIndex = 8; }
			else if (timer::timer600 % 25 < 20) { markerIndex = 9; }
			else if (timer::timer600 % 25 < 22) { markerIndex = 10; }
			else if (timer::timer600 % 25 < 24) { markerIndex = 11; }
			else { markerIndex = 0; }
		}

		drawSprite(spr::loadingAnime, markerIndex, tooltipBox.x + tooltipBox.w / 2 - 78, tooltipBox.y + 6);



		setFontSize(13);
		drawText(col2Str(col::white) + L"������ ���� ��...", tooltipBox.x + tooltipBox.w / 2 - 40, tooltipBox.y + 14);

		//������ ������ �׸���

		int pivotX = tooltipBox.x + 18;
		int pivotY = tooltipBox.y + 42;
		SDL_Rect iconBox = { tooltipBox.x + 36 - 18,tooltipBox.y + 60 - 18,36,36 };
		drawWindow(&iconBox);
		setZoom(2.0);
		drawSpriteCenter(spr::itemset, itemDex[targetItemCode].sprIndex, pivotX + 18, pivotY + 18);
		setZoom(1.0);
		setFontSize(13);
		drawText(col2Str(col::white) + itemDex[targetItemCode].name, pivotX + 50, pivotY + 6);




		SDL_Rect tooltipGauge = { cameraW / 2 - 130, 100, 260,16 };
		drawRect(tooltipGauge, col::white);

		SDL_Rect tooltipInGauge = { cameraW / 2 - 130 + 4, 100 + 4, 252,8 };
		tooltipInGauge.w = 252.0 * ((float)elapsedTime / (float)itemDex[targetItemCode].craftTime);
		drawFillRect(tooltipInGauge, col::white);

		setFontSize(11);
		std::wstring topText = std::to_wstring(itemDex[targetItemCode].craftTime - elapsedTime);
		topText += L" �� ���� ( ";
		topText += std::to_wstring((int)(((float)elapsedTime * 100.0 / (float)itemDex[targetItemCode].craftTime)));
		topText += L"% )";

		drawTextCenter(col2Str(col::white) + topText, tooltipGauge.x + tooltipGauge.w / 2, tooltipGauge.y - 10);

	}


	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
		//������ �ڽ� �׸���
		drawWindow(&craftBase, sysStr[75], 13);


		//���ã�� ��ư
		{
			SDL_Color btnColor = { 0x00, 0x00, 0x00 };
			SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
			if (checkCursor(&bookmarkCategory) && deactColorChange == false)
			{
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				outlineColor = { 0xa6, 0xa6, 0xa6 };
			}
			else if (selectCategory == -2)
			{
				btnColor = lowCol::deepBlue;
				outlineColor = { 0xa6, 0xa6, 0xa6 };
			}
			drawStadium(bookmarkCategory.x, bookmarkCategory.y, bookmarkCategory.w, bookmarkCategory.h, btnColor, 200, 5);
			SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
			SDL_Rect bookmarkInCategory = { bookmarkCategory.x + 3,  bookmarkCategory.y + 3, bookmarkCategory.w - 6, bookmarkCategory.h - 6 };
			drawRect(bookmarkInCategory, outlineColor);
			setFontSize(13);
			drawTextCenter(col2Str(col::white) + L"�� " + sysStr[122], bookmarkCategory.x + (bookmarkCategory.w / 2), bookmarkCategory.y + (bookmarkCategory.h / 2));
		}

		//���� ī�װ� �з� ��� �����
		for (int i = 0; i < 8; i++)
		{
			SDL_Color btnColor = { 0x00, 0x00, 0x00 };
			SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
			if (checkCursor(&craftCategory[i]) && deactColorChange == false)
			{
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
				outlineColor = { 0xa6, 0xa6, 0xa6 };
			}
			else if (selectCategory == i)
			{
				btnColor = lowCol::deepBlue;
				outlineColor = { 0xa6, 0xa6, 0xa6 };
			}

			drawStadium(craftCategory[i].x, craftCategory[i].y, 72, 72, btnColor, 200, 5);
			SDL_Rect craftInCategory = { craftCategory[i].x + 3,  craftCategory[i].y + 3, 72 - 6, 72 - 6 };
			drawRect(craftInCategory, outlineColor);

			std::wstring categoryName = L" ";
			int categoryIndex = 0;
			bool deactRect = false; // true�� ��� ȸ������ ����
			switch (i)
			{
			default://��ü
				categoryName = sysStr[76];
				categoryIndex = 36;
				break;
			case 0://����
				categoryName = sysStr[78];
				categoryIndex = 1;
				break;
			case 1://��
				categoryName = sysStr[77];
				categoryIndex = 50;
				break;
			case 2://����
				categoryName = sysStr[83];
				categoryIndex = 15;
				break;
			case 3://�Ҹ�ǰ
				categoryName = sysStr[81];
				categoryIndex = 35;
				break;
			case 4://����
				categoryName = sysStr[136];
				categoryIndex = 48;
				break;
			case 5://���̿���
				categoryName = sysStr[6];
				categoryIndex = 47;
				break;
			case 6://���๰
				categoryName = sysStr[121];
				categoryIndex = 20;
				break;
			case 7://���
				categoryName = sysStr[79];
				categoryIndex = 6;
				break;

			}
			drawSpriteCenter(spr::icon48, categoryIndex, craftCategory[i].x + (craftCategory[i].w / 2), craftCategory[i].y + (craftCategory[i].h / 2) - 10);

			int fontSize = 13;
			setFontSize(fontSize);
			while (queryTextWidth(categoryName, true) > craftCategory[0].w)
			{
				fontSize--;
				setFontSize(fontSize);
			}
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(categoryName, craftCategory[i].x + (craftCategory[i].w / 2), craftCategory[i].y + (craftCategory[i].h / 2) + 22);

			if (checkCursor(&craftCategory[i]) && deactColorChange == false)
			{
				int markerIndex = 0;
				if (timer::timer600 % 30 < 5) { markerIndex = 0; }
				else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
				else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
				else if (timer::timer600 % 30 < 20) { markerIndex = 1; }
				else { markerIndex = 0; }
				drawSpriteCenter(spr::letterboxBtnMarker, markerIndex, craftCategory[i].x + (craftCategory[i].w / 2), craftCategory[i].y + (craftCategory[i].h / 2));
			}

			if (deactRect == true)
			{
				drawStadium(craftCategory[i].x, craftCategory[i].y, 72, 72, { 0,0,0 }, 120, 5);
			}
		}


		//���� ����ī�װ� �и���(ȸ��)
		drawLine(craftBase.x + 169, craftBase.y + 30, craftBase.x + 169, craftBase.y + 399, col::gray);

		drawLine(craftBase.x + 0, craftBase.y + 29, craftBase.x + 15, craftBase.y + 29, col::lightGray);
		drawLine(craftBase.x + 0, craftBase.y + 30, craftBase.x + 15, craftBase.y + 30, col::lightGray);
		drawLine(craftBase.x + 0, craftBase.y + 31, craftBase.x + 0, craftBase.y + 44, col::lightGray);
		drawLine(craftBase.x + 1, craftBase.y + 31, craftBase.x + 1, craftBase.y + 44, col::lightGray);

		drawLine(craftBase.x + 154, craftBase.y + 29, craftBase.x + 169, craftBase.y + 29, col::lightGray);
		drawLine(craftBase.x + 154, craftBase.y + 30, craftBase.x + 169, craftBase.y + 30, col::lightGray);
		drawLine(craftBase.x + 168, craftBase.y + 31, craftBase.x + 168, craftBase.y + 44, col::lightGray);
		drawLine(craftBase.x + 169, craftBase.y + 31, craftBase.x + 169, craftBase.y + 44, col::lightGray);

		drawLine(craftBase.x + 0, craftBase.y + 384, craftBase.x + 0, craftBase.y + 399, col::lightGray);
		drawLine(craftBase.x + 1, craftBase.y + 384, craftBase.x + 1, craftBase.y + 399, col::lightGray);
		drawLine(craftBase.x + 2, craftBase.y + 398, craftBase.x + 15, craftBase.y + 398, col::lightGray);
		drawLine(craftBase.x + 2, craftBase.y + 399, craftBase.x + 15, craftBase.y + 399, col::lightGray);

		drawLine(craftBase.x + 168, craftBase.y + 384, craftBase.x + 168, craftBase.y + 399, col::lightGray);
		drawLine(craftBase.x + 169, craftBase.y + 384, craftBase.x + 169, craftBase.y + 399, col::lightGray);
		drawLine(craftBase.x + 157, craftBase.y + 398, craftBase.x + 167, craftBase.y + 398, col::lightGray);
		drawLine(craftBase.x + 157, craftBase.y + 399, craftBase.x + 167, craftBase.y + 399, col::lightGray);


		//�˻�â
		{
			//�˻� �ؽ�Ʈ �Է�ĭ
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
				if (checkCursor(&searchTextRect) && deactColorChange == false)
				{
					if (click == false) { btnColor = lowCol::blue; }
					else { btnColor = lowCol::deepBlue; }
					outlineColor = { 0xa6, 0xa6, 0xa6 };
				}

				drawStadium(searchTextRect.x, searchTextRect.y, searchTextRect.w, searchTextRect.h, btnColor, 120, 5);

				SDL_Rect searchInRect = { searchTextRect.x + 2, searchTextRect.y + 2, searchTextRect.w - 4, searchTextRect.h - 4 };
				drawRect(searchInRect, outlineColor);

				drawSpriteCenter(spr::icon16, 14, searchTextRect.x + 20, searchTextRect.y + searchTextRect.h / 2);

				{
					const unsigned __int16 maxTextWidth = 116;
					SDL_Point inputTextPoint = { searchTextRect.x + 33, searchTextRect.y + 6 };

					setFontSize(13);
					std::wstring exInputTextCut = exInputText;
					while (queryTextWidth(exInputTextCut, false) > maxTextWidth)
					{
						exInputTextCut = exInputTextCut.substr(1);
					}
					drawText(col2Str(col::white) + exInputTextCut, inputTextPoint.x, inputTextPoint.y);
					std::wstring cursorText = exInputTextCut.substr(0, exInputCursor + exInputEditing);
					if (timer::timer600 % 30 <= 15 && exInput == true)
					{
						int textWidth = queryTextWidth(cursorText, false);
						int textHeight = queryTextHeight(cursorText, false);
						drawLine(inputTextPoint.x + textWidth - 1, inputTextPoint.y, inputTextPoint.x + textWidth - 1, inputTextPoint.y + textHeight, col::white);
					}
				}
			}

			//�˻� ��ư �׸���
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
				if (checkCursor(&searchBtnRect) && deactColorChange == false)
				{
					if (click == false) { btnColor = lowCol::blue; }
					else { btnColor = lowCol::deepBlue; }
					outlineColor = { 0xa6, 0xa6, 0xa6 };
				}

				drawStadium(searchBtnRect.x, searchBtnRect.y, searchBtnRect.w, searchBtnRect.h, btnColor, 120, 5);

				SDL_Rect searchBtnInRect = { searchBtnRect.x + 2, searchBtnRect.y + 2, searchBtnRect.w - 4, searchBtnRect.h - 4 };
				drawRect(searchBtnInRect, outlineColor);

				setFontSize(12);
				drawTextCenter(L"#FFFFFF" + sysStr[27], searchBtnRect.x + searchBtnRect.w / 2, searchBtnRect.y + searchBtnRect.h / 2 - 1);
			}
		}

		//����ī�װ�(�Һз�) �׸���
		{
			//���� ī�װ� �и���
			drawLine(craftBase.x + 170, craftBase.y + 112, craftBase.x + 648, craftBase.y + 112, col::gray);

			auto drawSubcategoryBox = [](std::wstring boxStr, SDL_Rect box, bool pressed, bool deactColorChange)
				{
					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					SDL_Color letterColor = { 0xff,0xff,0xff };
					if (checkCursor(&box) && deactColorChange == false)
					{
						if (click == false) { btnColor = lowCol::blue; }
						else { btnColor = lowCol::deepBlue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					else if (pressed)
					{
						btnColor = lowCol::deepBlue;
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}

					if (pressed)
					{
						box.h += 2;
						box.y -= 2;
					}


					drawFillRect(box, btnColor);
					drawRect(box, col::gray);

					if (pressed)
					{
						SDL_Rect bottomWhiteRect = { box.x + 9, box.y + 19, 42,2 };
						drawRect(bottomWhiteRect, col::white);
					}

					setFontSize(10);
					drawTextCenter(col2Str(letterColor) + boxStr, box.x + box.w / 2, box.y + box.h / 2);
				};

			std::vector<std::wstring> subcategoryList;
			itemCategory targetCategory;
			//�Һз�(����ī�װ�)
			switch (selectCategory)
			{
			default://��ü
				drawSubcategoryBox(L"��ü", subcategoryBox[0], true, deactColorChange);
				break;
			case -2://���ã��
				drawSubcategoryBox(L"1", subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(L"2", subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(L"3", subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(L"4", subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(L"5", subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(L"6", subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;
			case 0://����
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_piercing), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_cutting), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_bashing), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_shooting), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_throwing), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;
			case 1://��
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_clothing), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_hat), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_gloves), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_shoes), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_accessory), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;

			case 2://����
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_hand), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_power), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_container), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_device), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_document), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_etc), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;

			case 3://�Ҹ�ǰ
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_food), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_medicine), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_ammo), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_fuel), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_etc), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;


			case 4://����
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_frame), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_engine), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_exterior), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_transport), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_energy), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_device), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;

			case 5://���̿���
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_core), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_active), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_passive), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_toggle), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_generator), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_storage), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;

			case 6://������
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_wall), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_floor), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_ceil), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_prop), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_electric), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_pneumatic), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;

			case 7://���
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_chemical), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_biological), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_mechanical), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_electrical), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_etc), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;


			}
		}

		//������ ���� �����۵� �׸���
		//for (int i = 0; i < 12; i++)
		//{
		//	if (i + 2 * craftScroll < recipePtr->itemInfo.size())
		//	{
		//		if (recipePtr->itemInfo[i + 2 * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false)
		//		{
		//			if (checkCursor(&itemBox[i]) && deactColorChange == false)
		//			{
		//				if (click == false) { drawItemRect(cursorFlag::hover, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]); }
		//				else { drawItemRect(cursorFlag::click, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]); }
		//			}
		//			else if (craftCursor == i + 2 * craftScroll)
		//			{
		//				drawItemRect(cursorFlag::hover, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]);
		//			}
		//			else
		//			{
		//				drawItemRect(cursorFlag::none, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]);
		//			}

		//			if (craftCursor == i + 2 * craftScroll)
		//			{
		//				int cursorIndex = 0;
		//				{
		//					if (timer::timer600 % 30 < 5) { cursorIndex = 0; }
		//					else if (timer::timer600 % 30 < 10) { cursorIndex = 1; }
		//					else if (timer::timer600 % 30 < 15) { cursorIndex = 2; }
		//					else if (timer::timer600 % 30 < 20) { cursorIndex = 3; }
		//					else { cursorIndex = 0; }
		//				}
		//				drawSprite(spr::itemCursorShort, cursorIndex, itemBox[i].x - 16, itemBox[i].y - 16);
		//			}
		//		}
		//	}
		//}

		for (int i = 0; i < 24; i++)
		{
			

			if (i + 6 * craftScroll < recipePtr->itemInfo.size())
			{
				if (recipePtr->itemInfo[i + 6 * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false)
				{
					int pivotX = craftBase.x + 183 + (73 * i) % (73 * 6);
					int pivotY = craftBase.y + 122 + 70 * ((73 * i) / (73 * 6));

					if (checkCursor(&itemBox[i]) && deactColorChange == false)
					{
						if (click == false) { drawSprite(spr::craftItemRect, 1, pivotX, pivotY); }
						else { drawSprite(spr::craftItemRect, 2, pivotX, pivotY); }
					}
					else if (craftCursor == i + 6 * craftScroll)
					{
						drawSprite(spr::craftItemRect, 1, pivotX, pivotY);
					}
					else
					{
						drawSprite(spr::craftItemRect, 0, pivotX, pivotY);
					}

					ItemData* iPtr = &(recipePtr->itemInfo[i + 6 * craftScroll]);
					setZoom(2.0); 
					drawSpriteCenter(spr::itemset, iPtr->sprIndex, pivotX + 29, pivotY + 18);
					setZoom(1.0);



					setFontSize(10);
					setSolidText();

					if (queryTextWidth(iPtr->name) > 63)
					{
						for (int i = iPtr->name.size()-1; i >= 0; i--)
						{
							if (queryTextWidth(iPtr->name.substr(0, i + 1)) <= 63)
							{
								drawTextCenter(col2Str(col::white) + iPtr->name.substr(0, i + 1), pivotX + 28, pivotY + 42);
								drawTextCenter(col2Str(col::white) + iPtr->name.substr(i+1), pivotX + 28, pivotY + 54);
								break;
							}
						}
					}
					else
					{
						drawTextCenter(col2Str(col::white) + iPtr->name, pivotX + 28, pivotY + 47);

					}

					disableSolidText();



					if (craftCursor == i + 6 * craftScroll)
					{
						//int cursorIndex = 0;
						//{
						//	if (timer::timer600 % 30 < 5) { cursorIndex = 0; }
						//	else if (timer::timer600 % 30 < 10) { cursorIndex = 1; }
						//	else if (timer::timer600 % 30 < 15) { cursorIndex = 2; }
						//	else if (timer::timer600 % 30 < 20) { cursorIndex = 3; }
						//	else { cursorIndex = 0; }
						//}
						//drawSprite(spr::itemCursorShort, cursorIndex, itemBox[i].x - 16, itemBox[i].y - 16);
					}
				}
			}
		}

		if (numNoneBlackFilter == 0) // ���� �������� ���� ���
		{
			setFontSize(13);
			drawTextCenter(col2Str(col::white) + sysStr[127], craftBase.x + craftBase.w / 2 + 90, craftBase.y + craftBase.h / 2 + 40);
		}

		// ��ũ�� �׸���
		{
			SDL_Rect craftScrollBox = { craftBase.x + craftBase.w - 18, craftBase.y + 36 + 95 - 6, 2, 252 };
			drawFillRect(craftScrollBox, { 120,120,120 });

			SDL_Rect inScrollBox = { craftBase.x + craftBase.w - 18, craftBase.y + 36 + 95 - 6, 2, 252 };
			inScrollBox.h = craftScrollBox.h * myMin(1.0, (double)24 / numNoneBlackFilter);
			inScrollBox.y = craftScrollBox.y + craftScrollBox.h * ((float)6.0 * craftScroll / (float)numNoneBlackFilter);
			if (inScrollBox.y + inScrollBox.h > craftScrollBox.y + craftScrollBox.h) { inScrollBox.y = craftScrollBox.y + craftScrollBox.h - inScrollBox.h; }
			drawFillRect(inScrollBox, col::white);
		}

		setFontSize(10);
		std::wstring whiteNumber = std::to_wstring(numNoneBlackFilter);
		std::wstring totalNumber = std::to_wstring(recipePtr->itemInfo.size());
		drawText(col2Str(col::white) + whiteNumber + L"/" + totalNumber, craftBase.x + 612, craftBase.y + 382);


		//������ �����Ϲڽ�(����) �׸���
		if (aniUSet.find(this) == aniUSet.end() && (pointingCursor >= 0 || craftCursor >= 0))
		{
			ItemPocket* equipPtr = Player::ins()->getEquipPtr();
			bool canCraft = true; //���� �÷��̾��� ���·� ������ �������� üũ�� 
			int targetCursor;
			if (pointingCursor >= 0) targetCursor = pointingCursor + 6*craftScroll;
			else targetCursor = craftCursor;

			if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BLACKFILTER) == false)
			{

				drawStadium(topWindow.x, topWindow.y, topWindow.w, topWindow.h, { 0,0,0 }, 210, 5);

				//������ ������ �׸���
				SDL_Rect iconBox = { topWindow.x + 36 - 24,topWindow.y + 36 - 24,48,48 };
				drawWindow(&iconBox);
				setZoom(3.0);
				drawSpriteCenter(spr::itemset, recipePtr->itemInfo[targetCursor].sprIndex, topWindow.x + 36, topWindow.y + 36);
				setZoom(1.0);
				setFontSize(13);
				drawText(col2Str(col::white) + recipePtr->itemInfo[targetCursor].name, topWindow.x + 68, topWindow.y + 10);

				setFontSize(11);
				std::wstring categoryStr = itemCategory2String(recipePtr->itemInfo[targetCursor].category) + L"-" + itemSubcategory2String(recipePtr->itemInfo[targetCursor].subcategory);

				drawText(col2Str(col::lightGray) + categoryStr, topWindow.x + 68, topWindow.y + 10 + 18);

				std::wstring weightStr = L"���� : ";
				weightStr += decimalCutter(((float)(recipePtr->itemInfo[targetCursor].weight)) / 1000.0, 3);
				weightStr += L"KG";
				drawText(col2Str(col::white) + weightStr, topWindow.x + 68, topWindow.y + 10 + 34);

				std::wstring volumeStr = L"���� : ";
				volumeStr += decimalCutter(((float)(recipePtr->itemInfo[targetCursor].volume)) / 1000.0, 3);
				volumeStr += L"L";
				drawText(col2Str(col::white) + volumeStr, topWindow.x + 168, topWindow.y + 10 + 34);

				std::wstring tooltipText;

				//���տ� �ʿ��� �÷��̾� ���
				std::wstring proficStr = col2Str(col::gray) + L"�ʿ� ��� : ";
				for (int i = 0; i < recipePtr->itemInfo[targetCursor].recipeProficNeed.size(); i++)
				{
					int needLevel = recipePtr->itemInfo[targetCursor].recipeProficNeed[i].second;
					int playerLevel = Player::ins()->getProficLevel(recipePtr->itemInfo[targetCursor].recipeProficNeed[i].first);
					if (playerLevel >= needLevel) proficStr += col2Str(lowCol::green);
					else
					{
						proficStr += col2Str(lowCol::red);
						canCraft = false;
					}

					proficStr += profic2String(recipePtr->itemInfo[targetCursor].recipeProficNeed[i].first);
					proficStr += L" ";
					proficStr += std::to_wstring(recipePtr->itemInfo[targetCursor].recipeProficNeed[i].second);
					proficStr += L"����";
					if (i != recipePtr->itemInfo[targetCursor].recipeProficNeed.size() - 1) proficStr += L", ";
				}
				if (recipePtr->itemInfo[targetCursor].recipeProficNeed.size() == 0) proficStr += col2Str(col::white) + L"����";

				tooltipText += proficStr + L"\n";

				//���տ� �ʿ��� ���(�� ����Ƽ)
				std::wstring qualityStr = col2Str(col::gray) + L"�ʿ� ������� : ";
				for (int i = 0; i < recipePtr->itemInfo[targetCursor].recipeQualityNeed.size(); i++)
				{
					if (equipPtr->checkToolQuality(recipePtr->itemInfo[targetCursor].recipeQualityNeed[i]))
					{
						qualityStr += col2Str(lowCol::green);
					}
					else
					{
						qualityStr += col2Str(lowCol::red);
						canCraft = false;
					}

					qualityStr += toolQuality2String(recipePtr->itemInfo[targetCursor].recipeQualityNeed[i]);
					if (i != recipePtr->itemInfo[targetCursor].recipeQualityNeed.size() - 1) qualityStr += L", ";
				}
				if (recipePtr->itemInfo[targetCursor].recipeQualityNeed.size() == 0) qualityStr += col2Str(col::white) + L"����";
				tooltipText += qualityStr + L"\n";

				//���տ� �ʿ��� ���
				std::wstring materialStr = col2Str(col::gray) + L"�ʿ� ��� : ";
				for (int i = 0; i < recipePtr->itemInfo[targetCursor].recipe.size(); i++)
				{
					//�� ����Ƽ�� ���� ����, ��� ��ȭ
					int playerNumber = equipPtr->numberItem(recipePtr->itemInfo[targetCursor].recipe[i].first);
					int needNumber = recipePtr->itemInfo[targetCursor].recipe[i].second;
					if (playerNumber >= needNumber) materialStr += col2Str(lowCol::green);
					else
					{
						materialStr += col2Str(lowCol::red);
						canCraft = false;
					}


					materialStr += itemDex[recipePtr->itemInfo[targetCursor].recipe[i].first].name;
					materialStr += L"(";
					materialStr += std::to_wstring(playerNumber);//�÷��̾ ������ ����
					materialStr += L"/";
					materialStr += std::to_wstring(needNumber);//���տ� �ʿ��� ������ ����
					materialStr += L")";
					if (i != recipePtr->itemInfo[targetCursor].recipe.size() - 1) materialStr += L", ";
				}
				if (recipePtr->itemInfo[targetCursor].recipe.size() == 0) materialStr += col2Str(col::white) + L"����";
				tooltipText += materialStr + L"\n";

				//������ ����
				tooltipText += col2Str(col::white);
				tooltipText += itemTooltip[recipePtr->itemInfo[targetCursor].tooltipIndex];

				setFontSize(9);
				if (!tooltipUnfold) drawTextWidth(col2Str(lowCol::white) + tooltipText, topWindow.x + 10, topWindow.y + 10 + 58 + 15 * 0, false, 360, 12, 6);
				else drawTextWidth(col2Str(lowCol::white) + tooltipText, topWindow.x + 10, topWindow.y + 10 + 58 + 15 * 0, false, 360, 12);

				//����-��ġ�� ��ư
				{
					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					if (checkCursor(&unfoldBtn))
					{
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					//�⺻ �簢�� 
					drawFillRect(unfoldBtn, btnColor, 180);
					//ȸ�� �׵θ�
					drawRect(unfoldBtn, outlineColor);
					//������
					if (tooltipUnfold == false) drawSpriteCenter(spr::icon16, 38, unfoldBtn.x + 14, unfoldBtn.y + unfoldBtn.h / 2);
					else drawSpriteCenter(spr::icon16, 39, unfoldBtn.x + 14, unfoldBtn.y + unfoldBtn.h / 2);

				}


				//�����ϱ� ��ư
				{

					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					if (checkCursor(&tooltipCraftBtn))
					{
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					drawFillRect(tooltipCraftBtn, btnColor, 180);
					//ȸ�� �׵θ�
					drawRect(tooltipCraftBtn, outlineColor);
					setFontSize(13);

					drawTextCenter(col2Str(col::white) + L"�����ϱ�", tooltipCraftBtn.x + tooltipCraftBtn.w / 2 + 10, tooltipCraftBtn.y + tooltipCraftBtn.h / 2 - 2 - 4);
					setFontSize(9);
					drawTextCenter(col2Str(col::white) + L"1�ð� 3��", tooltipCraftBtn.x + tooltipCraftBtn.w / 2 + 10, tooltipCraftBtn.y + tooltipCraftBtn.h / 2 - 2 - 4 + 12);
					drawSpriteCenter(spr::icon16, 28, tooltipCraftBtn.x + 14, tooltipCraftBtn.y + tooltipCraftBtn.h / 2);

					if (canCraft == false) drawFillRect(tooltipCraftBtn, col::black, 100);
				}

				//���ã�� ��ư
				{
					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					if (checkCursor(&tooltipBookmarkBtn))
					{
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					drawFillRect(tooltipBookmarkBtn, btnColor, 180);
					//ȸ�� �׵θ�
					drawRect(tooltipBookmarkBtn, outlineColor);

					int bookmarkSprIndex;
					SDL_Color textColor = col::white;
					if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK1)) bookmarkSprIndex = 32;
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK2)) bookmarkSprIndex = 33;
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK3)) bookmarkSprIndex = 34;
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK4)) bookmarkSprIndex = 35;
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK5)) bookmarkSprIndex = 36;
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK6)) bookmarkSprIndex = 37;
					else { bookmarkSprIndex = 29; textColor = col::gray; }

					setFontSize(13);
					drawTextCenter(col2Str(textColor) + L"���ã��", tooltipBookmarkBtn.x + tooltipBookmarkBtn.w / 2 + 10, tooltipBookmarkBtn.y + tooltipBookmarkBtn.h / 2 - 2);
					drawSpriteCenter(spr::icon16, bookmarkSprIndex, tooltipBookmarkBtn.x + 14, tooltipBookmarkBtn.y + tooltipBookmarkBtn.h / 2);

					if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK1));
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK2));
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK3));
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK4));
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK5));
					else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK6));
					else
					{
						drawFillRect(tooltipBookmarkBtn, col::black, 100);
					}
				}
			}
		}
	}
	else //���� �ִϸ��̼� ���� ���� Draw
	{
		SDL_Rect vRect = craftBase;
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