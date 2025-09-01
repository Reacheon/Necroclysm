import Craft;

#include <SDL3/SDL.h>

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
import GameOver;


void Craft::drawGUI()
{
	const bool* state = SDL_GetKeyboardState(nullptr);

	if (showCraftingTooltip && GameOver::ins()==nullptr)
	{
		SDL_Rect tooltipBox = { cameraW / 2 - 75, 50, 180, 58 };
		drawWindow(&tooltipBox);
		setZoom(1.0);

		int yOffset = 12;

		// 아이템 아이콘 (로딩 아이콘 대신)
		SDL_Rect iconBox = { tooltipBox.x + 36 - 18 - 10, tooltipBox.y + 60 - 18 - 37 + yOffset, 36, 36 };
		drawWindow(&iconBox);
		setZoom(2.0);
		drawSpriteCenter(spr::itemset, itemDex[targetItemCode].sprIndex, iconBox.x + 18, iconBox.y + 18);
		setZoom(1.0);

		// 제작 중 텍스트 (점 애니메이션 추가)
		int dotCount = (SDL_GetTicks() / 800) % 4;
		std::wstring craftText = L"Crafting";
		for (int i = 0; i < dotCount; i++) craftText += L".";
		setFontSize(12);
		renderTextCenter(craftText, tooltipBox.x + 113, tooltipBox.y + 12+ yOffset);

		// 진행 바
		SDL_Rect tooltipGauge = { tooltipBox.x + 51, tooltipBox.y + 23 + yOffset, 125, 7 };
		drawRect(tooltipGauge, col::white);

		SDL_Rect tooltipInGauge = { tooltipGauge.x + 2, tooltipGauge.y + 2, 121, 3 };
		tooltipInGauge.w = 121 * ((float)elapsedTime / (float)itemDex[targetItemCode].craftTime);
		drawFillRect(tooltipInGauge, col::white);

		// 남은 시간 텍스트
		setFontSize(8);
		int remainingMinutes = itemDex[targetItemCode].craftTime - elapsedTime;
		std::wstring progressText = std::to_wstring(remainingMinutes) + L" m left ( ";
		progressText += std::to_wstring((int)(((float)elapsedTime * 100.0 / (float)itemDex[targetItemCode].craftTime)));
		progressText += L"% )";
		renderTextCenter(progressText, tooltipGauge.x + tooltipGauge.w / 2, tooltipGauge.y + tooltipGauge.h + 7);

		setFontSize(10);
		renderTextOutlineCenter(itemDex[targetItemCode].name, tooltipBox.x + tooltipBox.w / 2, tooltipBox.y + 7);

		if(itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
		{
			int pivotX = cameraW/2 - (int)(8 * zoomScale) + 16 * (buildLocation.x - PlayerX()) * zoomScale;
			int pivotY = cameraH / 2 + (int)(8 * zoomScale) + 16 * (buildLocation.y - PlayerY()) * zoomScale;

			if (buildLocation.x == PlayerX() && buildLocation.y == PlayerY() - 1)
			{
				pivotY -= 20 * zoomScale;
			}

			SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			drawFillRect(dst, col::black, 200);

			float ratioHP = ((float)elapsedTime / (float)itemDex[targetItemCode].craftTime);

			SDL_Color gaugeCol = lowCol::green;
			if (ratioHP < 0.25) gaugeCol = lowCol::red;
			else if (ratioHP < 0.5) gaugeCol = lowCol::yellow;

			dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
			if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			drawFillRect(dst, gaugeCol, 200);
		}


		if(SDL_GetTicks() % 1000 < 500)
		{
			PlayerPtr->entityInfo.sprIndex = charSprIndex::CRAFT1;
        }
		else
		{
			PlayerPtr->entityInfo.sprIndex = charSprIndex::CRAFT2;
		}
	}


	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
		//윈도우 박스 그리기
		drawWindow(&craftBase, sysStr[75], 13);


		//즐겨찾기 버튼
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
			SDL_Rect bookmarkInCategory = { bookmarkCategory.x + 3,  bookmarkCategory.y + 3, bookmarkCategory.w - 6, bookmarkCategory.h - 6 };
			drawRect(bookmarkInCategory, outlineColor);
			setFontSize(12);
			renderTextCenter(L"★ " + sysStr[122], bookmarkCategory.x + (bookmarkCategory.w / 2), bookmarkCategory.y + (bookmarkCategory.h / 2));
		}

		//좌측 카테고리 분류 기능 만들기
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
			bool deactRect = false; // true일 경우 회색으로 변함
			switch (i)
			{
			default://전체
				categoryName = sysStr[76];
				categoryIndex = 36;
				break;
			case 0://장비
				categoryName = sysStr[77];
				categoryIndex = 50;
				break;
			case 1://음식
				categoryName = sysStr[78];
				categoryIndex = 173;
				break;
			case 2://도구
				categoryName = sysStr[79];
				categoryIndex = 15;
				break;
			case 3://기술
				categoryName = sysStr[80];
				categoryIndex = 47;
				break;
			case 4://소모품
				categoryName = sysStr[81];
				categoryIndex = 35;
				break;
			case 5://차량
				categoryName = sysStr[82];
				categoryIndex = 48;
				break;
			case 6://건축물
				categoryName = sysStr[83];
				categoryIndex = 20;
				break;
			case 7://재료
				categoryName = sysStr[84];
				categoryIndex = 6;
				break;

			}
			drawSpriteCenter(spr::icon48, categoryIndex, craftCategory[i].x + (craftCategory[i].w / 2), craftCategory[i].y + (craftCategory[i].h / 2) - 10);

			int fontSize = 10;
			setFontSize(fontSize);
			if (queryTextWidth(categoryName, true) > craftCategory[0].w - 10)
			{
				fontSize = 8;
				setFontSize(fontSize);
			}
			renderTextCenter(categoryName, craftCategory[i].x + (craftCategory[i].w / 2), craftCategory[i].y + (craftCategory[i].h / 2) + 24);

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


		//좌측 대형카테고리 분리선(회색)
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


		//검색창
		{
			//검색 텍스트 입력칸
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
				if (checkCursor(&searchTextRect) && deactColorChange == false)
				{
					if (click == false) { btnColor = lowCol::blue; }
					else { btnColor = lowCol::deepBlue; }
					outlineColor = { 0xa6, 0xa6, 0xa6 };
				}

				drawStadium(searchTextRect.x, searchTextRect.y, searchTextRect.w, searchTextRect.h, btnColor, 255, 5);

				SDL_Rect searchInRect = { searchTextRect.x + 2, searchTextRect.y + 2, searchTextRect.w - 4, searchTextRect.h - 4 };
				drawRect(searchInRect, outlineColor);

				drawSpriteCenter(spr::icon16, 14, searchTextRect.x + 20, searchTextRect.y + searchTextRect.h / 2);

				{
					const unsigned __int16 maxTextWidth = 116;
					SDL_Point inputTextPoint = { searchTextRect.x + 33, searchTextRect.y + 6 };

					setFontSize(12);
					std::wstring exInputTextCut = exInputText;
					while (queryTextWidth(exInputTextCut, false) > maxTextWidth)
					{
						exInputTextCut = exInputTextCut.substr(1);
					}
					renderText(exInputTextCut, inputTextPoint.x, inputTextPoint.y);
					std::wstring cursorText = exInputTextCut.substr(0, exInputCursor + exInputEditing);
					if (timer::timer600 % 30 <= 15 && exInput == true)
					{
						int textWidth = queryTextWidth(cursorText, false);
						int textHeight = 15;
						drawLine(inputTextPoint.x + textWidth - 1, inputTextPoint.y, inputTextPoint.x + textWidth - 1, inputTextPoint.y + textHeight, col::white);
					}
				}
			}

			//검색 버튼 그리기
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
				if (deactColorChange == false)
				{
					if (checkCursor(&searchBtnRect))
					{
						if (click == false) { btnColor = lowCol::blue; }
						else { btnColor = lowCol::deepBlue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					else if(state[SDL_SCANCODE_RETURN])
					{
						btnColor = lowCol::deepBlue;
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
				}

				drawStadium(searchBtnRect.x, searchBtnRect.y, searchBtnRect.w, searchBtnRect.h, btnColor, 255, 5);

				SDL_Rect searchBtnInRect = { searchBtnRect.x + 2, searchBtnRect.y + 2, searchBtnRect.w - 4, searchBtnRect.h - 4 };
				drawRect(searchBtnInRect, outlineColor);

				setFontSize(12);
				renderTextCenter(sysStr[27], searchBtnRect.x + searchBtnRect.w / 2, searchBtnRect.y + searchBtnRect.h / 2 - 1);
			}
		}

		//서브카테고리(소분류) 그리기
		{
			//서브 카테고리 분리선
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
						SDL_Rect bottomWhiteRect = { box.x + 9, box.y + 19, 52, 2 }; // 42 → 52로 변경
						drawRect(bottomWhiteRect, col::white);
					}

					setFontSize(10);
					renderTextCenter(boxStr, box.x + box.w / 2, box.y + box.h / 2, letterColor);
				};

			std::vector<std::wstring> subcategoryList;
			itemCategory targetCategory;
			//소분류(서브카테고리)
			switch (selectCategory)
			{
			default://전체
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], true, deactColorChange);
				break;
			case -2://즐겨찾기
				drawSubcategoryBox(L"1", subcategoryBox[0], selectSubcategory == 0, deactColorChange);
				drawSubcategoryBox(L"2", subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(L"3", subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(L"4", subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(L"5", subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(L"6", subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;
			case 0://장비
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_melee), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_ranged), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_firearms), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_throwing), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_clothing), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;
			case 1://음식
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::foods_cooked), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::foods_processed), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::foods_preserved), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::foods_drinks), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::foods_ingredients), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;

			case 2://도구
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tools_hand), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tools_power), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tools_containers), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tools_etc), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;

			case 3://기술
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tech_bionics), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tech_powerArmor), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				break;

			case 4://소모품
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_medicine), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_ammo), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_fuel), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_etc), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;

			case 5://차량
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(sysStr[260], subcategoryBox[1], selectSubcategory == 1, deactColorChange); //프레임
				drawSubcategoryBox(sysStr[261], subcategoryBox[2], selectSubcategory == 2, deactColorChange); //파워
				drawSubcategoryBox(sysStr[262], subcategoryBox[3], selectSubcategory == 3, deactColorChange); //외장
				drawSubcategoryBox(sysStr[263], subcategoryBox[4], selectSubcategory == 4, deactColorChange); //부품
				break;

			case 6://건축물
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_walls), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_floors), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_ceilings), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_props), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				break;

			case 7://재료
				drawSubcategoryBox(sysStr[276], subcategoryBox[0], selectSubcategory == 0, deactColorChange);//전체
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_metals), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_organic), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_components), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_chemicals), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
				drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_etc), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
				break;
			}
		}

		for (int i = 0; i < 24; i++)
		{
			

			if (i + CRAFT_MAX_COLUMN * craftScroll < recipePtr->itemInfo.size())
			{
				if (recipePtr->itemInfo[i + CRAFT_MAX_COLUMN * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false)
				{
					int pivotX = itemBox[i].x;
					int pivotY = itemBox[i].y;

					drawRect(itemBox[i], col::gray);

					if (checkCursor(&itemBox[i]) && deactColorChange == false)
					{
						if (click == false) drawFillRect(itemBox[i], lowCol::blue);
						else drawFillRect(itemBox[i], lowCol::deepBlue);
					}
					else if (craftCursor == i + CRAFT_MAX_COLUMN * craftScroll)
					{
						drawFillRect(itemBox[i], lowCol::blue);
					}
					else
					{

					}

					ItemData* iPtr = &(recipePtr->itemInfo[i + CRAFT_MAX_COLUMN * craftScroll]);
					setZoom(2.0); 
					drawSpriteCenter(spr::itemset, iPtr->sprIndex, pivotX + 16, pivotY + 12);
					setZoom(1.0);



					if(queryTextWidth(iPtr->name)<100)setFontSize(10);
					else setFontSize(8);
					
					renderText(iPtr->name, pivotX + 34, pivotY + 7);




					if (craftCursor == i + CRAFT_MAX_COLUMN * craftScroll)
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

		if (numNoneBlackFilter == 0) // 만약 아이템이 없을 경우
		{
			setFontSize(12);
			renderTextCenter(sysStr[127], craftBase.x + craftBase.w / 2 + 90, craftBase.y + craftBase.h / 2 + 40);
		}

		// 스크롤 그리기
		{
			SDL_Rect craftScrollBox = { craftBase.x + craftBase.w - 18, craftBase.y + 36 + 95 - 6, 2, 252 };
			drawFillRect(craftScrollBox, { 120,120,120 });

			SDL_Rect inScrollBox = { craftBase.x + craftBase.w - 18, craftBase.y + 36 + 95 - 6, 2, 252 };
			inScrollBox.h = craftScrollBox.h * myMin(1.0, (double)24 / numNoneBlackFilter);
			inScrollBox.y = craftScrollBox.y + craftScrollBox.h * ((float)CRAFT_MAX_COLUMN * craftScroll / (float)numNoneBlackFilter);
			if (inScrollBox.y + inScrollBox.h > craftScrollBox.y + craftScrollBox.h) { inScrollBox.y = craftScrollBox.y + craftScrollBox.h - inScrollBox.h; }
			drawFillRect(inScrollBox, col::white);
		}

		setFontSize(8);
		std::wstring whiteNumber = std::to_wstring(numNoneBlackFilter);
		std::wstring totalNumber = std::to_wstring(recipePtr->itemInfo.size());
		renderText(whiteNumber + L"/" + totalNumber, craftBase.x + 612, craftBase.y + 385);


		//아이템 디테일박스(툴팁) 그리기
		if (aniUSet.find(this) == aniUSet.end() && (pointingCursor >= 0 || craftCursor >= 0))
		{
			ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
			bool canCraft = true; //현재 플레이어의 상태로 조합이 가능한지 체크함 
			int targetCursor;
			if (pointingCursor >= 0) targetCursor = pointingCursor + CRAFT_MAX_COLUMN *craftScroll;
			else targetCursor = craftCursor;

			if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BLACKFILTER) == false)
			{

				drawStadium(topWindow.x, topWindow.y, topWindow.w, topWindow.h, { 0,0,0 }, 210, 5);

				//아이템 아이콘 그리기
				SDL_Rect iconBox = { topWindow.x + 36 - 24,topWindow.y + 36 - 24,48,48 };
				drawWindow(&iconBox);
				setZoom(3.0);
				drawSpriteCenter(spr::itemset, recipePtr->itemInfo[targetCursor].sprIndex, topWindow.x + 36, topWindow.y + 36);
				setZoom(1.0);
				setFontSize(16);
				renderText(recipePtr->itemInfo[targetCursor].name, topWindow.x + 68, topWindow.y + 12);

				setFontSize(10);
				std::wstring categoryStr = itemCategory2String(recipePtr->itemInfo[targetCursor].category) + L"-" + itemSubcategory2String(recipePtr->itemInfo[targetCursor].subcategory);

				renderText(categoryStr, topWindow.x + 68, topWindow.y + 12 + 18, col::lightGray);

				std::wstring weightStr = sysStr[17]+L" : ";
				weightStr += decimalCutter(((float)(recipePtr->itemInfo[targetCursor].weight)) / 1000.0, 3);
				weightStr += L"kg";
				renderText(weightStr, topWindow.x + 68, topWindow.y + 12 + 32);

				std::wstring volumeStr = sysStr[18] + L" : ";
				volumeStr += decimalCutter(((float)getVolume(recipePtr->itemInfo[targetCursor])) / 1000.0, 3);
				volumeStr += L"L";
				renderText(volumeStr, topWindow.x + 168, topWindow.y + 12 + 32);

				std::wstring tooltipText;

				//조합에 필요한 플레이어 재능
				std::wstring proficStr = col2Str(col::gray) + sysStr[233] + L" : ";//필요 기술
				for (int i = 0; i < recipePtr->itemInfo[targetCursor].recipeProficNeed.size(); i++)
				{
					int needLevel = recipePtr->itemInfo[targetCursor].recipeProficNeed[i].second;
					int playerLevel = PlayerPtr->getProficLevel(recipePtr->itemInfo[targetCursor].recipeProficNeed[i].first);
					if (playerLevel >= needLevel) proficStr += col2Str(lowCol::green);
					else
					{
						proficStr += col2Str(lowCol::red);
						canCraft = false;
					}

					proficStr += profic2String(recipePtr->itemInfo[targetCursor].recipeProficNeed[i].first);
					proficStr += L" ";
					proficStr += std::to_wstring(recipePtr->itemInfo[targetCursor].recipeProficNeed[i].second);
					proficStr += L"레벨";
					if (i != recipePtr->itemInfo[targetCursor].recipeProficNeed.size() - 1) proficStr += L", ";
				}
				if (recipePtr->itemInfo[targetCursor].recipeProficNeed.size() == 0) proficStr += col2Str(col::white) + sysStr[236];

				tooltipText += proficStr + L"\n";

				//조합에 필요한 기술(툴 퀄리티)
				std::wstring qualityStr = col2Str(col::gray) + sysStr[234]+L" : ";//필요 공구
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
				if (recipePtr->itemInfo[targetCursor].recipeQualityNeed.size() == 0) qualityStr += col2Str(col::white) + sysStr[236];
				tooltipText += qualityStr + L"\n";

				//조합에 필요한 재료
				std::wstring materialStr = col2Str(col::gray) + sysStr[235]+L" : ";//필요 재료
				for (int i = 0; i < recipePtr->itemInfo[targetCursor].recipe.size(); i++)
				{
					//툴 퀄리티에 따라 적색, 녹색 변화
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
					materialStr += std::to_wstring(playerNumber);//플레이어가 소유한 갯수
					materialStr += L"/";
					materialStr += std::to_wstring(needNumber);//조합에 필요한 아이템 갯수
					materialStr += L")";
					if (i != recipePtr->itemInfo[targetCursor].recipe.size() - 1) materialStr += L", ";
				}
				if (recipePtr->itemInfo[targetCursor].recipe.size() == 0) materialStr += col2Str(col::white) + sysStr[236];
				tooltipText += materialStr + L"\n";

				//아이템 설명
				tooltipText += col2Str(col::white);
				tooltipText += itemTooltip[recipePtr->itemInfo[targetCursor].tooltipIndex];

				setFontSize(10);
				if (!tooltipUnfold) renderTextWidth(col2Str(lowCol::white) + tooltipText, topWindow.x + 10, topWindow.y + 10 + 58 + 15 * 0, false, 360, 12, 6);
				else renderTextWidth(col2Str(lowCol::white) + tooltipText, topWindow.x + 10, topWindow.y + 10 + 58 + 15 * 0, false, 360, 12);

				//접기-펼치기 버튼
				{
					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					if (checkCursor(&unfoldBtn))
					{
						if (click == true) { btnColor = lowCol::deepBlue; }
						else { btnColor = lowCol::blue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					//기본 사각형 
					drawFillRect(unfoldBtn, btnColor, 180);
					//회색 테두리
					drawRect(unfoldBtn, outlineColor);
					//아이콘
					if (tooltipUnfold == false) drawSpriteCenter(spr::icon16, 38, unfoldBtn.x + 14, unfoldBtn.y + unfoldBtn.h / 2);
					else drawSpriteCenter(spr::icon16, 39, unfoldBtn.x + 14, unfoldBtn.y + unfoldBtn.h / 2);

				}


				//조합하기 버튼
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
					//회색 테두리
					drawRect(tooltipCraftBtn, outlineColor);
					setFontSize(12);

					renderTextCenter(sysStr[237], tooltipCraftBtn.x + tooltipCraftBtn.w / 2 + 10, tooltipCraftBtn.y + tooltipCraftBtn.h / 2 - 2 - 4);//조합하기
					setFontSize(10);
					
					std::wstring remainStr = replaceStr(replaceStr(sysStr[238], L"(%hour)", L"1"), L"(%min)",L"34");
					renderTextCenter(remainStr, tooltipCraftBtn.x + tooltipCraftBtn.w / 2 + 10, tooltipCraftBtn.y + tooltipCraftBtn.h / 2 - 2 - 4 + 12);
					drawSpriteCenter(spr::icon16, 28, tooltipCraftBtn.x + 14, tooltipCraftBtn.y + tooltipCraftBtn.h / 2);

					if (canCraft == false) drawFillRect(tooltipCraftBtn, col::black, 100);
				}

				//즐겨찾기 버튼
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
					//회색 테두리
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

					setFontSize(12);
					renderTextCenter(sysStr[239], tooltipBookmarkBtn.x + tooltipBookmarkBtn.w / 2 + 10, tooltipBookmarkBtn.y + tooltipBookmarkBtn.h / 2 - 2, textColor);//즐겨찾기
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
	else //폴드 애니메이션 실행 중의 Draw
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