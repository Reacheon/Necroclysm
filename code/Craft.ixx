#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Craft;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import drawPrimitive;
import globalVar;
import constVar;
import checkCursor;
import drawItemSlot;
import drawWindow;
import ItemPocket;
import drawItemTooltip;
import Lst;
import const2Str;
import Player;
import World;
import CoordSelect;
import CoordSelectCraft;
import log;
import Msg;
import Vehicle;
import Prop;
import turnWait;

export class Craft : public GUI
{
private:
	inline static Craft* ptr = nullptr;

	SDL_Rect craftBase;
	SDL_Rect topWindow;
	SDL_Rect bookmarkCategory;
	SDL_Rect subcategoryBox[8];
	SDL_Rect searchTextRect;
	SDL_Rect searchBtnRect;
	std::array<SDL_Rect, 8> craftCategory;
	SDL_Rect itemBox[12];

	int selectCategory = -1; //-2이면 즐겨찾기, -1이면 비활성화, 0이면 무기, 1면 방어구
	int selectSubcategory = -1;

	int craftScroll = 0;
	int initCraftScroll = 0;
	int craftCursor = -1;
	int pointingCursor = -1;

	std::wstring searchInfo;

	SDL_Rect tooltipCraftBtn;
	SDL_Rect tooltipBookmarkBtn;
	SDL_Rect unfoldBtn;
	bool tooltipUnfold = false;

	ItemPocket* recipePtr;//플레이어가 보유한 레시피들

	bool deactColorChange = false;//마우스를 가져갔을 때 버튼들의 컬러가 변하는 것을 비활성화

	int numNoneBlackFilter; //블랙필터가 아닌 아이템의 수, 스크롤 제한을 걸 때 사용됨. 동적으로 스텝마다 작동시키면 렉 걸려서 블랙필터가 사용되면 수동으로 변경해줘야함

	bool showCraftingTooltip = false; //제작 중에 화면 상단에 그려지는 진척도, deactDraw와 상관없이 동작함

	bool isNowBuilding = false;
	int targetItemCode = 0;
	int elapsedTime = 0; //총괄 제작 시간
	inline static std::array<int, 3> buildLocation = { -1, -1, -1 };

	inline static int ongoingTargetCode = -1; //제작 중인 아이템
	inline static int ongoingElapsedTime = -1; //제작 경과 시간 아이템

	inline static int ongoingTargetCodeStructure = -1; //제작 중인 건축물
	inline static int ongoingElapsedTimeStructure = -1; //제작 경과 시간 건축물

public:
	Craft() : GUI(false)
	{
		prt(L"Craft : 생성자가 호출되었습니다..\n");
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;
		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this,aniFlag::winUnfoldOpen);


		exInputText = L"";
		recipePtr = ((ItemPocket*)availableRecipe);
		for (int i = 0; i < recipePtr->itemInfo.size(); i++)
		{
			recipePtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
			recipePtr->itemInfo[i].eraseFlag(itemFlag::BLACKFILTER);
		}
		numNoneBlackFilter = recipePtr->itemInfo.size();

		if (existCraftData() || existCraftDataStructure())
		{
			CORO(executeCraft());
		}
	}
	~Craft()
	{
		prt(L"Craft : 소멸자가 호출되었습니다..\n");
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Craft* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		craftBase = { 0, 0, 650, 400 };

		if (center == false)
		{
			craftBase.x += inputX;
			craftBase.y += inputY;
		}
		else
		{
			craftBase.x += inputX - craftBase.w / 2;
			craftBase.y += inputY - craftBase.h / 2;
		}

		topWindow = { 0, 0, 410,148 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

		int categoryX = craftBase.x + 9;
		int categoryY = craftBase.y + 36 + 14 + 28;
		int categoryIntervalW = 80;
		int categoryIntervalH = 80;


		bookmarkCategory.x = craftBase.x + 9;
		bookmarkCategory.y = craftBase.y + 39;
		bookmarkCategory.w = 152;
		bookmarkCategory.h = 33;

		for (int i = 0; i < 8; i++)
		{
			craftCategory[i].x = categoryX + categoryIntervalW * (i % 2);
			craftCategory[i].y = categoryY + categoryIntervalH * (i / 2);
			craftCategory[i].w = 72;
			craftCategory[i].h = 72;
		}

		for (int i = 0; i < 8; i++)
		{
			subcategoryBox[i] = { craftBase.x + 169 + 60 * i, craftBase.y + 93, 60, 20 };
		}

		searchTextRect = { craftBase.x + craftBase.w - 224, craftBase.y + 46, 150, 30 };
		searchBtnRect = { searchTextRect.x + 153, searchTextRect.y, 48, searchTextRect.h };

		for (int i = 0; i < 12; i++)
		{
			itemBox[i].x = craftBase.x + 178 + 222 * (i % 2);
			itemBox[i].y = craftBase.y + 36 + 85 + 44 * (i / 2);
			itemBox[i].w = 210;
			itemBox[i].h = 36;
		}

		tooltipCraftBtn = { topWindow.x + 310,topWindow.y + 10,90,26 };
		tooltipBookmarkBtn = { topWindow.x + 310,topWindow.y + 10 + 32,90,26 };
		unfoldBtn = { topWindow.x + topWindow.w - 32,topWindow.y + topWindow.h - 32, 28 , 28 };

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - craftBase.w / 2;
			y = inputY - craftBase.h / 2;
		}
	}
	void drawGUI()
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
			drawText(col2Str(col::white) + L"아이템 조합 중...", tooltipBox.x + tooltipBox.w / 2 - 40, tooltipBox.y + 14);

			//아이템 아이콘 그리기

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
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			SDL_RenderDrawRect(renderer, &tooltipGauge);

			SDL_Rect tooltipInGauge = { cameraW / 2 - 130 + 4, 100 + 4, 252,8 };
			tooltipInGauge.w = 252.0 * ((float)elapsedTime / (float)itemDex[targetItemCode].craftTime);
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			SDL_RenderFillRect(renderer, &tooltipInGauge);

			setFontSize(11);
			std::wstring topText = std::to_wstring(itemDex[targetItemCode].craftTime - elapsedTime);
			topText += L" 분 남음 ( ";
			topText += std::to_wstring((int)(((float)elapsedTime * 100.0 / (float)itemDex[targetItemCode].craftTime)));
			topText += L"% )";

			drawTextCenter(col2Str(col::white) + topText, tooltipGauge.x + tooltipGauge.w / 2, tooltipGauge.y - 10);

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
				SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
				SDL_Rect bookmarkInCategory = { bookmarkCategory.x + 3,  bookmarkCategory.y + 3, bookmarkCategory.w - 6, bookmarkCategory.h - 6 };
				SDL_RenderDrawRect(renderer, &bookmarkInCategory);
				setFontSize(13);
				drawTextCenter(col2Str(col::white) + L"★ " + sysStr[122], bookmarkCategory.x + (bookmarkCategory.w / 2), bookmarkCategory.y + (bookmarkCategory.h / 2));
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
				SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
				SDL_Rect craftInCategory = { craftCategory[i].x + 3,  craftCategory[i].y + 3, 72 - 6, 72 - 6 };
				SDL_RenderDrawRect(renderer, &craftInCategory);

				std::wstring categoryName = L" ";
				int categoryIndex = 0;
				bool deactRect = false; // true일 경우 회색으로 변함
				switch (i)
				{
				default://전체
					categoryName = sysStr[76];
					categoryIndex = 36;
					break;
				case 0://무기
					categoryName = sysStr[78];
					categoryIndex = 1;
					break;
				case 1://방어구
					categoryName = sysStr[77];
					categoryIndex = 50;
					break;
				case 2://도구
					categoryName = sysStr[83];
					categoryIndex = 15;
					break;
				case 3://소모품
					categoryName = sysStr[81];
					categoryIndex = 35;
					break;
				case 4://차량
					categoryName = sysStr[136];
					categoryIndex = 48;
					break;
				case 5://바이오닉
					categoryName = sysStr[6];
					categoryIndex = 47;
					break;
				case 6://건축물
					categoryName = sysStr[121];
					categoryIndex = 20;
					break;
				case 7://재료
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


			//좌측 대형카테고리 분리선(회색)
			SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
			SDL_RenderDrawLine(renderer, craftBase.x + 169, craftBase.y + 30, craftBase.x + 169, craftBase.y + 399);

			SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 255);
			SDL_RenderDrawLine(renderer, craftBase.x + 0, craftBase.y + 29, craftBase.x + 15, craftBase.y + 29);
			SDL_RenderDrawLine(renderer, craftBase.x + 0, craftBase.y + 30, craftBase.x + 15, craftBase.y + 30);
			SDL_RenderDrawLine(renderer, craftBase.x + 0, craftBase.y + 31, craftBase.x + 0, craftBase.y + 44);
			SDL_RenderDrawLine(renderer, craftBase.x + 1, craftBase.y + 31, craftBase.x + 1, craftBase.y + 44);

			SDL_RenderDrawLine(renderer, craftBase.x + 154, craftBase.y + 29, craftBase.x + 169, craftBase.y + 29);
			SDL_RenderDrawLine(renderer, craftBase.x + 154, craftBase.y + 30, craftBase.x + 169, craftBase.y + 30);
			SDL_RenderDrawLine(renderer, craftBase.x + 168, craftBase.y + 31, craftBase.x + 168, craftBase.y + 44);
			SDL_RenderDrawLine(renderer, craftBase.x + 169, craftBase.y + 31, craftBase.x + 169, craftBase.y + 44);

			SDL_RenderDrawLine(renderer, craftBase.x + 0, craftBase.y + 384, craftBase.x + 0, craftBase.y + 399);
			SDL_RenderDrawLine(renderer, craftBase.x + 1, craftBase.y + 384, craftBase.x + 1, craftBase.y + 399);
			SDL_RenderDrawLine(renderer, craftBase.x + 2, craftBase.y + 398, craftBase.x + 15, craftBase.y + 398);
			SDL_RenderDrawLine(renderer, craftBase.x + 2, craftBase.y + 399, craftBase.x + 15, craftBase.y + 399);

			SDL_RenderDrawLine(renderer, craftBase.x + 168, craftBase.y + 384, craftBase.x + 168, craftBase.y + 399);
			SDL_RenderDrawLine(renderer, craftBase.x + 169, craftBase.y + 384, craftBase.x + 169, craftBase.y + 399);
			SDL_RenderDrawLine(renderer, craftBase.x + 157, craftBase.y + 398, craftBase.x + 167, craftBase.y + 398);
			SDL_RenderDrawLine(renderer, craftBase.x + 157, craftBase.y + 399, craftBase.x + 167, craftBase.y + 399);




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

					drawStadium(searchTextRect.x, searchTextRect.y, searchTextRect.w, searchTextRect.h, btnColor, 120, 5);

					SDL_Rect searchInRect = { searchTextRect.x + 2, searchTextRect.y + 2, searchTextRect.w - 4, searchTextRect.h - 4 };
					SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
					SDL_RenderDrawRect(renderer, &searchInRect);

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
						SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
						if (timer::timer600 % 30 <= 15 && exInput == true)
						{
							int textWidth = queryTextWidth(cursorText, false);
							int textHeight = queryTextHeight(cursorText, false);
							SDL_RenderDrawLine(renderer, inputTextPoint.x + textWidth - 1, inputTextPoint.y, inputTextPoint.x + textWidth - 1, inputTextPoint.y + textHeight);
						}
					}
				}

				//검색 버튼 그리기
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
					SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
					SDL_RenderDrawRect(renderer, &searchBtnInRect);

					setFontSize(12);
					drawTextCenter(L"#FFFFFF" + sysStr[27], searchBtnRect.x + searchBtnRect.w / 2, searchBtnRect.y + searchBtnRect.h / 2 - 1);
				}
			}

			//서브카테고리(소분류) 그리기
			{
				//서브 카테고리 분리선
				SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
				SDL_RenderDrawLine(renderer, craftBase.x + 170, craftBase.y + 112, craftBase.x + 648, craftBase.y + 112);

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

						SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, 255);
						SDL_RenderFillRect(renderer, &box);

						SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 255);
						SDL_RenderDrawRect(renderer, &box);

						if (pressed)
						{
							SDL_Rect bottomWhiteRect = { box.x + 9, box.y + 19, 42,2 };
							SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
							SDL_RenderDrawRect(renderer, &bottomWhiteRect);
						}

						setFontSize(10);
						drawTextCenter(col2Str(letterColor) + boxStr, box.x + box.w / 2, box.y + box.h / 2);
					};

				std::vector<std::wstring> subcategoryList;
				itemCategory targetCategory;
				//소분류(서브카테고리)
				switch (selectCategory)
				{
				default://전체
					drawSubcategoryBox(L"전체", subcategoryBox[0], true, deactColorChange);
					break;
				case -2://즐겨찾기
					drawSubcategoryBox(L"1", subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(L"2", subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(L"3", subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(L"4", subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(L"5", subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					drawSubcategoryBox(L"6", subcategoryBox[5], selectSubcategory == 5, deactColorChange);
					break;
				case 0://무기
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_piercing), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_cutting), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_bashing), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_shooting), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::weapon_throwing), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					break;
				case 1://방어구
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_clothing), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_hat), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_gloves), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_shoes), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::equipment_accessory), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					break;

				case 2://도구
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_hand), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_power), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_container), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_device), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_document), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::tool_etc), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
					break;

				case 3://소모품
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_food), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_medicine), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_ammo), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_fuel), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::consumable_etc), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					break;


				case 4://차량
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_frame), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_engine), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_exterior), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_transport), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_energy), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::vehicle_device), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
					break;

				case 5://바이오닉
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_core), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_active), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_passive), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_toggle), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_generator), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::bionic_storage), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
					break;

				case 6://구조물
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_wall), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_floor), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_ceil), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_prop), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_electric), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::structure_pneumatic), subcategoryBox[5], selectSubcategory == 5, deactColorChange);
					break;

				case 7://재료
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_chemical), subcategoryBox[0], selectSubcategory == 0, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_biological), subcategoryBox[1], selectSubcategory == 1, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_mechanical), subcategoryBox[2], selectSubcategory == 2, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_electrical), subcategoryBox[3], selectSubcategory == 3, deactColorChange);
					drawSubcategoryBox(itemSubcategory2String(itemSubcategory::material_etc), subcategoryBox[4], selectSubcategory == 4, deactColorChange);
					break;


				}
			}

			//가능한 조합 아이템들 그리기
			for (int i = 0; i < 12; i++)
			{
				if (i + 2 * craftScroll < recipePtr->itemInfo.size())
				{
					if (recipePtr->itemInfo[i + 2 * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false)
					{
						if (checkCursor(&itemBox[i]) && deactColorChange == false)
						{
							if (click == false) { drawItemRect(cursorFlag::hover, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]); }
							else { drawItemRect(cursorFlag::click, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]); }
						}
						else if (craftCursor == i + 2 * craftScroll)
						{
							drawItemRect(cursorFlag::hover, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]);
						}
						else
						{
							drawItemRect(cursorFlag::none, itemBox[i].x, itemBox[i].y, recipePtr->itemInfo[i + 2 * craftScroll]);
						}

						if (craftCursor == i + 2 * craftScroll)
						{
							int cursorIndex = 0;
							{
								if (timer::timer600 % 30 < 5) { cursorIndex = 0; }
								else if (timer::timer600 % 30 < 10) { cursorIndex = 1; }
								else if (timer::timer600 % 30 < 15) { cursorIndex = 2; }
								else if (timer::timer600 % 30 < 20) { cursorIndex = 3; }
								else { cursorIndex = 0; }
							}
							drawSprite(spr::itemCursorShort, cursorIndex, itemBox[i].x - 16, itemBox[i].y - 16);
						}
					}
				}
			}

			if (numNoneBlackFilter == 0) // 만약 아이템이 없을 경우
			{
				setFontSize(13);
				drawTextCenter(col2Str(col::white) + sysStr[127], craftBase.x + craftBase.w / 2 + 90, craftBase.y + craftBase.h / 2 + 40);
			}

			// 스크롤 그리기
			{
				SDL_Rect craftScrollBox = { craftBase.x + craftBase.w - 18, craftBase.y + 36 + 95 - 6, 2, 252 };
				SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
				SDL_RenderFillRect(renderer, &craftScrollBox);
				SDL_Rect inScrollBox = { craftBase.x + craftBase.w - 18, craftBase.y + 36 + 95 - 6, 2, 252 };
				inScrollBox.h = craftScrollBox.h * myMin(1.0, (double)12 / numNoneBlackFilter);
				inScrollBox.y = craftScrollBox.y + craftScrollBox.h * ((float)2.0 * craftScroll / (float)numNoneBlackFilter);
				if (inScrollBox.y + inScrollBox.h > craftScrollBox.y + craftScrollBox.h) { inScrollBox.y = craftScrollBox.y + craftScrollBox.h - inScrollBox.h; }
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(renderer, &inScrollBox);
			}

			setFontSize(10);
			std::wstring whiteNumber = std::to_wstring(numNoneBlackFilter);
			std::wstring totalNumber = std::to_wstring(recipePtr->itemInfo.size());
			drawText(col2Str(col::white) + whiteNumber + L"/" + totalNumber, craftBase.x + 600, craftBase.y + 382);


			//아이템 디테일박스(툴팁) 그리기
			if (aniUSet.find(this) == aniUSet.end() && (pointingCursor >= 0 || craftCursor >= 0))
			{
				ItemPocket* equipPtr = Player::ins()->getEquipPtr();
				bool canCraft = true; //현재 플레이어의 상태로 조합이 가능한지 체크함 
				int targetCursor;
				if (pointingCursor >= 0) targetCursor = pointingCursor;
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
					setFontSize(13);
					drawText(col2Str(col::white) + recipePtr->itemInfo[targetCursor].name, topWindow.x + 68, topWindow.y + 10);

					setFontSize(11);
					std::wstring categoryStr = itemCategory2String(recipePtr->itemInfo[targetCursor].category) + L"-" + itemSubcategory2String(recipePtr->itemInfo[targetCursor].subcategory);

					drawText(col2Str(col::lightGray) + categoryStr, topWindow.x + 68, topWindow.y + 10 + 18);

					std::wstring weightStr = L"무게 : ";
					weightStr += decimalCutter(((float)(recipePtr->itemInfo[targetCursor].weight)) / 1000.0, 3);
					weightStr += L"KG";
					drawText(col2Str(col::white) + weightStr, topWindow.x + 68, topWindow.y + 10 + 34);

					std::wstring volumeStr = L"부피 : ";
					volumeStr += decimalCutter(((float)(recipePtr->itemInfo[targetCursor].volume)) / 1000.0, 3);
					volumeStr += L"L";
					drawText(col2Str(col::white) + volumeStr, topWindow.x + 168, topWindow.y + 10 + 34);

					std::wstring tooltipText;

					//조합에 필요한 플레이어 재능
					std::wstring talentStr = col2Str(col::gray) + L"필요 기술 : ";
					for (int i = 0; i < recipePtr->itemInfo[targetCursor].recipeTalentNeed.size(); i++)
					{
						int needLevel = recipePtr->itemInfo[targetCursor].recipeTalentNeed[i].second;
						int playerLevel = Player::ins()->getTalentLevel(recipePtr->itemInfo[targetCursor].recipeTalentNeed[i].first);
						if (playerLevel >= needLevel) talentStr += col2Str(lowCol::green);
						else
						{
							talentStr += col2Str(lowCol::red);
							canCraft = false;
						}

						talentStr += talent2String(recipePtr->itemInfo[targetCursor].recipeTalentNeed[i].first);
						talentStr += L" ";
						talentStr += std::to_wstring(recipePtr->itemInfo[targetCursor].recipeTalentNeed[i].second);
						talentStr += L"레벨";
						if (i != recipePtr->itemInfo[targetCursor].recipeTalentNeed.size() - 1) talentStr += L", ";
					}
					if (recipePtr->itemInfo[targetCursor].recipeTalentNeed.size() == 0) talentStr += col2Str(col::white) + L"없음";

					tooltipText += talentStr + L"\n";

					//조합에 필요한 기술(툴 퀄리티)
					std::wstring qualityStr = col2Str(col::gray) + L"필요 도구기술 : ";
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
					if (recipePtr->itemInfo[targetCursor].recipeQualityNeed.size() == 0) qualityStr += col2Str(col::white) + L"없음";
					tooltipText += qualityStr + L"\n";

					//조합에 필요한 재료
					std::wstring materialStr = col2Str(col::gray) + L"필요 재료 : ";
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
					if (recipePtr->itemInfo[targetCursor].recipe.size() == 0) materialStr += col2Str(col::white) + L"없음";
					tooltipText += materialStr + L"\n";

					//아이템 설명
					tooltipText += col2Str(col::white);
					tooltipText += itemTooltip[recipePtr->itemInfo[targetCursor].tooltipIndex];

					setFontSize(9);
					if (!tooltipUnfold) drawTextWidth(col2Str(lowCol::white) + tooltipText, topWindow.x + 10, topWindow.y + 10 + 58 + 15 * 0, false, 360, 12, 6);
					else drawTextWidth(col2Str(lowCol::white) + tooltipText, topWindow.x + 10, topWindow.y + 10 + 58 + 15 * 0, false, 360, 12);

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
						SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, 180);
						SDL_RenderFillRect(renderer, &unfoldBtn);
						//회색 테두리
						SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
						SDL_RenderDrawRect(renderer, &unfoldBtn);
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
						SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, 180);
						SDL_RenderFillRect(renderer, &tooltipCraftBtn);
						//회색 테두리
						SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
						SDL_RenderDrawRect(renderer, &tooltipCraftBtn);
						setFontSize(13);

						drawTextCenter(col2Str(col::white) + L"조합하기", tooltipCraftBtn.x + tooltipCraftBtn.w / 2 + 10, tooltipCraftBtn.y + tooltipCraftBtn.h / 2 - 2 - 4);
						setFontSize(9);
						drawTextCenter(col2Str(col::white) + L"1시간 3분", tooltipCraftBtn.x + tooltipCraftBtn.w / 2 + 10, tooltipCraftBtn.y + tooltipCraftBtn.h / 2 - 2 - 4 + 12);
						drawSpriteCenter(spr::icon16, 28, tooltipCraftBtn.x + 14, tooltipCraftBtn.y + tooltipCraftBtn.h / 2);

						if (canCraft == false)
						{
							SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
							SDL_RenderFillRect(renderer, &tooltipCraftBtn);
						}
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
						SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, 180);
						SDL_RenderFillRect(renderer, &tooltipBookmarkBtn);
						//회색 테두리
						SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
						SDL_RenderDrawRect(renderer, &tooltipBookmarkBtn);

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
						drawTextCenter(col2Str(textColor) + L"즐겨찾기", tooltipBookmarkBtn.x + tooltipBookmarkBtn.w / 2 + 10, tooltipBookmarkBtn.y + tooltipBookmarkBtn.h / 2 - 2);
						drawSpriteCenter(spr::icon16, bookmarkSprIndex, tooltipBookmarkBtn.x + 14, tooltipBookmarkBtn.y + tooltipBookmarkBtn.h / 2);

						if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK1));
						else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK2));
						else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK3));
						else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK4));
						else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK5));
						else if (recipePtr->itemInfo[targetCursor].checkFlag(itemFlag::BOOKMARK6));
						else
						{
							SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
							SDL_RenderFillRect(renderer, &tooltipBookmarkBtn);
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
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		exInput = false;
		if (checkCursor(&tab))
		{
			if (showCraftingTooltip == false) close(aniFlag::winUnfoldClose);
			else
			{
				updateLog(L"#FFFFFF아이템 조합을 취소했다.");

				//조합 데이터 저장
				if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
				{
					saveCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
					isNowBuilding = false;
				}
				else saveCraftData(targetItemCode, elapsedTime);
				coTurnSkip = false;
				close(aniFlag::null);
			}
		}
		else if (checkCursor(&tooltipCraftBtn) && craftCursor >= 0)//툴팁 조합하기
		{
			CORO(executeCraft());
		}
		else if (checkCursor(&tooltipBookmarkBtn) && craftCursor >= 0)//툴팁 즐겨찾기
		{
			CORO(executeBookmark());
		}
		else if (checkCursor(&unfoldBtn) && craftCursor >= 0)//툴팁 펼치기
		{
			if (tooltipUnfold == false)
			{
				tooltipUnfold = true;
				deactColorChange = true;
				topWindow.h += 300;
				unfoldBtn.y += 300;
			}
			else
			{
				tooltipUnfold = false;
				deactColorChange = false;
				topWindow.h -= 300;
				unfoldBtn.y -= 300;
			}
		}
		else if (checkCursor(&bookmarkCategory))//즐겨찾기 카테고리 클릭업
		{
			craftCursor = -1;
			if (selectCategory != -2)
			{
				craftScroll = 0;
				selectCategory = -2;
				selectSubcategory = 0;
				int matchCount = recipePtr->searchFlag(itemFlag::BOOKMARK1);
				filterUpdate(matchCount);

			}
			else //재차 눌렀을 경우 전체 분류로 변경
			{
				selectCategory = -1;
			}
		}
		else if (checkCursor(&searchTextRect))
		{
			if (exInput == false) exInput = true;
			else exInput = false;
		}
		else if (checkCursor(&searchBtnRect))
		{
			craftCursor = -1;
			int matchCount = recipePtr->searchTxt(exInputText);
			filterUpdate(matchCount);
		}
		else
		{
			//카테고리 8버튼 클릭업
			for (int i = 0; i < 8; i++)
			{
				if (checkCursor(&craftCategory[i]))
				{
					craftCursor = -1;
					if (selectCategory != i)
					{
						craftScroll = 0;
						selectCategory = i;
						selectSubcategory = -1;

						itemCategory targetCategory;
						switch (selectCategory)
						{
						case 0:
							targetCategory = itemCategory::weapon;
							break;
						case 1:
							targetCategory = itemCategory::equipment;
							break;
						case 2:
							targetCategory = itemCategory::tool;
							break;
						case 3:
							targetCategory = itemCategory::consumable;
							break;
						case 4:
							targetCategory = itemCategory::vehicle;
							break;
						case 5:
							targetCategory = itemCategory::bionic;
							break;
						case 6:
							targetCategory = itemCategory::structure;
							break;
						case 7:
							targetCategory = itemCategory::material;
							break;
						}
						int matchCount = recipePtr->searchCategory(targetCategory);
						filterUpdate(matchCount);
					}
					else
					{
						selectCategory = -1;
						selectSubcategory = 0;
						for (int i = 0; i < recipePtr->itemInfo.size(); i++) recipePtr->itemInfo[i].eraseFlag(itemFlag::BLACKFILTER);
						recipePtr->sortByUnicode();
					}
					break;
				}
			}

			//서브카테고리 입력
			{
				int maxSubcategorySize;
				switch (selectCategory)
				{
				case -2://즐겨찾기
					maxSubcategorySize = 6;
					break;
				case -1://전체
					maxSubcategorySize = 1;
					break;
				case 0://무기
					maxSubcategorySize = 5;
					break;
				case 1://방어구
					maxSubcategorySize = 5;
					break;
				case 2://도구
					maxSubcategorySize = 6;
					break;
				case 3://식량
					maxSubcategorySize = 6;
					break;
				case 4://소모품
					maxSubcategorySize = 5;
					break;
				case 5://바이오닉
					maxSubcategorySize = 6;
					break;
				case 6://건축물
					maxSubcategorySize = 6;
					break;
				case 7://재료
					maxSubcategorySize = 5;
					break;
				}
				for (int i = 0; i < maxSubcategorySize; i++)
				{
					if (checkCursor(&subcategoryBox[i]))
					{
						craftCursor = -1;
						if (selectSubcategory != i)
						{
							craftScroll = 0;
							selectSubcategory = i;
							int matchCount;
							itemFlag targetFlag;
							itemSubcategory targetSubcategory;
							switch (selectCategory)
							{
							case -2:
								if (selectSubcategory == 0) { targetFlag = itemFlag::BOOKMARK1; }
								else if (selectSubcategory == 1) { targetFlag = itemFlag::BOOKMARK2; }
								else if (selectSubcategory == 2) { targetFlag = itemFlag::BOOKMARK3; }
								else if (selectSubcategory == 3) { targetFlag = itemFlag::BOOKMARK4; }
								else if (selectSubcategory == 4) { targetFlag = itemFlag::BOOKMARK5; }
								else if (selectSubcategory == 5) { targetFlag = itemFlag::BOOKMARK6; }
								matchCount = recipePtr->searchFlag(targetFlag);
								break;
							case 0:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::weapon_piercing; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::weapon_cutting; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::weapon_bashing; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::weapon_shooting; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::weapon_throwing; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 1:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::equipment_clothing; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::equipment_hat; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::equipment_gloves; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::equipment_shoes; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::equipment_accessory; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 2:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::tool_hand; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::tool_power; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::tool_container; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::tool_device; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::tool_document; }
								else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::tool_etc; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 3:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::consumable_food; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::consumable_medicine; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::consumable_ammo; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::consumable_fuel; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::consumable_etc; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 4:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::vehicle_frame; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::vehicle_engine; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::vehicle_exterior; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::vehicle_transport; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::vehicle_energy; }
								else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::vehicle_device; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 5:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::bionic_core; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::bionic_active; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::bionic_passive; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::bionic_toggle; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::bionic_generator; }
								else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::bionic_storage; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 6:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::structure_wall; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::structure_floor; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::structure_ceil; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::structure_prop; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::structure_electric; }
								else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::structure_pneumatic; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							case 7:
								if (selectSubcategory == 0) { targetSubcategory = itemSubcategory::material_chemical; }
								else if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::material_biological; }
								else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::material_mechanical; }
								else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::material_electrical; }
								else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::material_etc; }
								matchCount = recipePtr->searchSubcategory(targetSubcategory);
								break;
							}
							filterUpdate(matchCount);
						}
						else//같은 서브카테고리 버튼을 재차 눌렀을 경우
						{
							if (selectCategory >= 0)
							{
								selectSubcategory = -1;
								itemCategory targetCategory;
								switch (selectCategory)
								{
								case 0:
									targetCategory = itemCategory::weapon;
									break;
								case 1:
									targetCategory = itemCategory::equipment;
									break;
								case 2:
									targetCategory = itemCategory::tool;
									break;
								case 3:
									targetCategory = itemCategory::consumable;
									break;
								case 4:
									targetCategory = itemCategory::vehicle;
									break;
								case 5:
									targetCategory = itemCategory::bionic;
									break;
								case 6:
									targetCategory = itemCategory::structure;
									break;
								case 7:
									targetCategory = itemCategory::material;
									break;
								}
								int matchCount = recipePtr->searchCategory(targetCategory);
								filterUpdate(matchCount);
							}
						}
						break;
					}
				}
			}

			//아이템박스 클릭

			for (int i = 0; i < 12; i++)
			{
				if (checkCursor(&itemBox[i]))
				{
					if (craftCursor != i + 2 * craftScroll && i + 2 * craftScroll < numNoneBlackFilter)
					{
						craftCursor = i + 2 * craftScroll;
					}
					else
					{
						craftCursor = -1;
					}
				}
			}
		}
	}
	void clickMotionGUI(int dx, int dy)
	{
		if (deactColorChange == false)
		{
			pointingCursor = -1;
			for (int i = 0; i < 12; i++)
			{
				if (checkCursor(&itemBox[i]))
				{
					if (i + 2 * craftScroll < recipePtr->itemInfo.size())
					{
						if (recipePtr->itemInfo[i + 2 * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false) pointingCursor = i;
					}
				}
			}
		}

		if (checkCursor(&craftBase))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // 가속상수, 작아질수록 스크롤 속도가 빨라짐
				craftScroll = initCraftScroll + dy / scrollAccelConst;
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
		initCraftScroll = craftScroll;
	}
	void step()
	{
		//만약 제작 중인 아이템이 있으면
		if (ongoingTargetCode != 0);

		//잘못된 스크롤 위치 조정
		int maxScroll = myMax(0, (numNoneBlackFilter - 11) / 2);
		if (craftScroll > maxScroll) { craftScroll = maxScroll; }
		else if (craftScroll < 0) { craftScroll = 0; }
	}

	void filterUpdate(int matchCount)
	{
		numNoneBlackFilter = matchCount;//커서 위치 제한용

		for (int i = 0; i < recipePtr->itemInfo.size(); i++) recipePtr->itemInfo[i].eraseFlag(itemFlag::WHITEFILTER);
		for (int i = 0; i < recipePtr->itemInfo.size(); i++) recipePtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
		for (int i = 0; i < recipePtr->itemInfo.size(); i++) recipePtr->itemInfo[i].eraseFlag(itemFlag::BLACKFILTER);

		for (int i = 0; i < recipePtr->itemInfo.size(); i++) recipePtr->itemInfo[i].addFlag(itemFlag::BLACKFILTER);
		for (int i = 0; i < matchCount; i++) recipePtr->itemInfo[i].eraseFlag(itemFlag::BLACKFILTER);

		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int targetCursor = 0; targetCursor < matchCount; targetCursor++)
		{
			if (canCraft(recipePtr->itemInfo[targetCursor].itemCode)) recipePtr->itemInfo[targetCursor].addFlag(itemFlag::WHITEFILTER);
			else recipePtr->itemInfo[targetCursor].addFlag(itemFlag::GRAYFILTER);
		}

		int matchCountCanCraft = recipePtr->searchFlag(itemFlag::WHITEFILTER);
		if (matchCountCanCraft >= 2) recipePtr->sortByUnicode(0, matchCountCanCraft - 1); //조합 불가능 아이템만 정렬
	}

	bool canCraft(int itemCode, bool exceptMaterial)
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		//조합에 필요한 플레이어 재능 체크
		for (int i = 0; i < itemDex[itemCode].recipeTalentNeed.size(); i++)
		{
			int needLevel = itemDex[itemCode].recipeTalentNeed[i].second;
			int playerLevel = Player::ins()->getTalentLevel(itemDex[itemCode].recipeTalentNeed[i].first);
			if (playerLevel < needLevel) return false;
		}

		//조합에 필요한 기술(툴 퀄리티) 체크
		for (int i = 0; i < itemDex[itemCode].recipeQualityNeed.size(); i++)
		{
			if (equipPtr->checkToolQuality(itemDex[itemCode].recipeQualityNeed[i]) == false) return false;
		}

		if (exceptMaterial == false)
		{
			//조합에 필요한 재료 체크
			for (int i = 0; i < itemDex[itemCode].recipe.size(); i++)
			{
				//툴 퀄리티에 따라 적색, 녹색 변화
				int playerNumber = equipPtr->numberItem(itemDex[itemCode].recipe[i].first);
				int needNumber = itemDex[itemCode].recipe[i].second;
				if (playerNumber < needNumber) return false;
			}
		}

		return true;
	}

	bool canCraft(int itemCode) { return canCraft(itemCode, false); }

	Corouter executeBookmark()
	{
		if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK1)) recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK1);
		else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK2)) recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK2);
		else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK3)) recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK3);
		else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK4)) recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK4);
		else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK5)) recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK5);
		else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK6)) recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK6);
		else
		{
			//recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK1);
			std::vector<std::wstring> numList = { L"1", L"2", L"3", L"4", L"5", L"6" };
			new Lst(sysStr[95], sysStr[94], numList);//넣기, 넣을 포켓을 선택해주세요.
			deactColorChange = true;
			co_await std::suspend_always();
			deactColorChange = false;
			switch (wtoi(coAnswer.c_str()))
			{
			case 0:
				recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK1);
				break;
			case 1:
				recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK2);
				break;
			case 2:
				recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK3);
				break;
			case 3:
				recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK4);
				break;
			case 4:
				recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK5);
				break;
			case 5:
				recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK6);
				break;
			}
		}
	}

	Corouter executeCraft()
	{
		prt(printMagenta);
		prt(L"executeCraft 실행됨\n");
		prt(printReset);

		bool negateMonster = false;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////1.조합 좌표 및 아이템 설정///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (craftCursor != -1)//최초 조합일 때
		{
			targetItemCode = recipePtr->itemInfo[craftCursor].itemCode;
			elapsedTime = 0;

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////1-1.재료,도구,재능 체크 및 제거///////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (canCraft(targetItemCode) == false)//재료,도구,재능을 만족하는지 체크
			{
				updateLog(L"#FFFFFF재료가 부족하다.");
				co_return;
			}
			else//조합에 필요한 재료 제거
			{
				for (int i = 0; i < itemDex[targetItemCode].recipe.size(); i++)
				{
					//툴 퀄리티에 따라 적색, 녹색 변화
					int meterialItemCode = itemDex[meterialItemCode].recipe[i].first;
					int needNumber = itemDex[meterialItemCode].recipe[i].second;
					Player::ins()->getEquipPtr()->subtractItemCode(meterialItemCode, needNumber);
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////1-2.좌표 선택///////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
			{
				if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))
				{
					//새로운 차량을 설치하시겠습니까?
					std::vector<std::array<int, 2>> selectableTile;
					for (int dir = -1; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						if (World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).VehiclePtr == nullptr) selectableTile.push_back({ Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy });
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						new CoordSelect(L"차량 프레임을 설치할 위치를 선택해주세요.");
						co_await std::suspend_always();
						actDraw();

						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetZ = wtoi(targetStr.c_str());

						buildLocation = { targetX,targetY,targetZ };

					}
					else
					{
						updateLog(L"#FFFFFF주변에 차량 프레임을 설치할만한 공간이 없다.");
						co_return;
					}
				}
				else if (itemDex[targetItemCode].checkFlag(itemFlag::VPART))
				{
					std::vector<std::array<int, 2>> selectableTile;
					for (int dir = -1; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						//차량부품이므로 이미 있는 프레임 위에 건설되어야 함
						Vehicle* targetVehicle = (Vehicle*)(World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).VehiclePtr);
						if (targetVehicle != nullptr)
						{
							if (targetVehicle->hasFrame(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy))
							{
								selectableTile.push_back({ Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy });
							}
						}
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						new CoordSelect(L"차량 부품을 설치할 프레임을 선택해주세요.");
						co_await std::suspend_always();
						actDraw();

						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetZ = wtoi(targetStr.c_str());

						buildLocation = { targetX,targetY,targetZ };

					}
					else
					{
						updateLog(L"#FFFFFF주변에 차량 부품을 설치할만한 프레임이 없다.");
						co_return;
					}
				}
				else
				{
					//새로운 차량을 설치하시겠습니까?
					std::vector<std::array<int, 2>> selectableTile;
					for (int dir = -1; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						if (World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).PropPtr == nullptr) selectableTile.push_back({ Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy });
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						new CoordSelectCraft(targetItemCode, L"조합한 아이템을 설치할 위치를 선택해주세요.", selectableTile);
						co_await std::suspend_always();
						actDraw();

						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						targetItemCode = wtoi(targetStr.c_str());

						buildLocation = { targetX,targetY,Player::ins()->getGridZ() };

					}
					else
					{
						updateLog(L"#FFFFFF주변에 해당 아이템을 설치할만한 공간이 없다.");
						co_return;
					}
				}
			}
		}
		else // 조합 재개 : 최초로 조합창을 오픈할 때 조합데이터 확인
		{
			if (existCraftData())
			{
				int percent = (int)(100.0 * (float)ongoingElapsedTime / (float)itemDex[ongoingTargetCode].craftTime);
				new Msg(msgFlag::normal, L"제작", std::to_wstring(percent) + L"%에서 조합을 중단한 아이템이 존재합니다.(%item3)계속 조합하시겠습니까?", { L"네",L"아니오" });
				deactColorChange = true;
				co_await std::suspend_always();
				deactColorChange = false;

				if (coAnswer == L"네")
				{
					loadCraftData(targetItemCode, elapsedTime);
				}
				else
				{
					deleteCraftData();
					co_return;
				}
			}
			else if (existCraftDataStructure())
			{
				int dx = abs(Player::ins()->getGridX() - buildLocation[0]);
				int dy = abs(Player::ins()->getGridY() - buildLocation[1]);
				int dz = abs(Player::ins()->getGridZ() - buildLocation[2]);

				if (dx <= 1 && dy <= 1 && dz == 0)
				{
					new Msg(msgFlag::normal, L"제작", L"주변에 조합 중인 건축물이 존재합니다. (%item3) 건축물의 조합을 계속하시겠습니까?", { L"네",L"파기" });
					deactColorChange = true;
					co_await std::suspend_always();
					deactColorChange = false;
					if (coAnswer == L"네")
					{
						loadCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
						prt(L"현재 빌드 로케이션의 좌표는 %d,%d,%d이다\n", buildLocation[0], buildLocation[1], buildLocation[2]);
					}
					else
					{
						deleteCraftDataStructure();
						co_return;
					}
				}
				else
				{
					std::wstring text = replaceStr(L"떨어진 좌표 (▲,▲,▲)에 조합 중인 건축물이 존재합니다. (%item3) 파기하고 새로운 아이템을 제작하시겠습니까?", L"▲", { std::to_wstring(buildLocation[0]),std::to_wstring(buildLocation[1]),std::to_wstring(buildLocation[2]) });
					new Msg(msgFlag::normal, L"제작", text, { L"네",L"아니오" });
					deactColorChange = true;
					co_await std::suspend_always();
					deactColorChange = false;
					if (coAnswer == L"네")
					{
						deleteCraftDataStructure();
						co_return;
					}
					else
					{
						delete this;
						co_return;
					}
				}

			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////2.좌표 선정 완료 후 조합 루프///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) isNowBuilding = true;
		deactDraw();
		showCraftingTooltip = true;
		while (1)
		{
			prt(L"Craft While 루프 실행됨\n");
			if (negateMonster == false)
			{
				for (int x = Player::ins()->getGridX() - 1; x <= Player::ins()->getGridX() + 1; x++)
				{
					for (int y = Player::ins()->getGridY() - 1; y <= Player::ins()->getGridY() + 1; y++)
					{
						if (!(x == Player::ins()->getGridX() && y == Player::ins()->getGridY()))
							if (World::ins()->getTile(x, y, Player::ins()->getGridZ()).fov == fovFlag::white)
								if (World::ins()->getTile(x, y, Player::ins()->getGridZ()).EntityPtr != nullptr)
								{
									new Msg(msgFlag::normal, L"경고", L"주변에 적이 있습니다. 계속 조합하시겠습니까?", { L"네",L"아니오",L"무시하기" });
									deactColorChange = true;
									co_await std::suspend_always();
									if (coAnswer == L"네") goto loopEnd;
									else if (coAnswer == L"무시하기")
									{
										negateMonster = true;
										goto loopEnd;
									}
									else
									{
										//조합 데이터 저장
										if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
										{
											saveCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
											isNowBuilding = false;
										}
										else saveCraftData(targetItemCode, elapsedTime);
										coTurnSkip = false;
										close(aniFlag::null);
										co_return;
									}
								}
					}
				}
			}

		loopEnd:

			turnWait(1.0);
			coTurnSkip = true;

			prt(printRed);
			prt(L"exeCraft 코루틴 실행 전\n");
			prt(printReset);

			co_await std::suspend_always();

			prt(printRed);
			prt(L"exeCraft 코루틴 실행 후\n");
			prt(printReset);

			elapsedTime++;
			if (elapsedTime >= itemDex[targetItemCode].craftTime) break;
		}


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////3.조합 종료 : 아이템 생성///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
		{
			if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))//새로운 차량 설치
			{
				//설치점 주변에 연결 가능한 차량이 있는지 체크
				std::vector<Vehicle*> canConnect;
				for (int dir = 0; dir < 8; dir++)
				{
					if (dir % 2 == 1) continue; //대각선 비허용
					int dx, dy;
					dir2Coord(dir, dx, dy);
					Vehicle* targetCoordPtr = (Vehicle*)World::ins()->getTile(buildLocation[0] + dx, buildLocation[1] + dy, buildLocation[2]).VehiclePtr;
					if (targetCoordPtr != nullptr)
					{
						if (std::find(canConnect.begin(), canConnect.end(), targetCoordPtr) == canConnect.end())
						{
							canConnect.push_back(targetCoordPtr);
						}
					}
				}

				if (canConnect.size() == 0)
				{
					Vehicle* newVehicle = new Vehicle(buildLocation[0], buildLocation[1], buildLocation[2], targetItemCode);
					new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//새로운 차량의 이름을 입력해주세요. 결정, 취소
					co_await std::suspend_always();
					if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
					else newVehicle->name = L"차량 " + std::to_wstring(randomRange(1, 9999999));
				}
				else
				{
					new Msg(msgFlag::normal, L"경고", L"설치한 프레임을 주변 차량에 연결하시겠습니까?", { L"네",L"아니오" });
					co_await std::suspend_always();
					if (coAnswer == L"네")
					{
						Vehicle* targetVehicle;
						if (canConnect.size() == 1)
						{
							targetVehicle = canConnect[0];
							targetVehicle->extendPart(buildLocation[0], buildLocation[1], targetItemCode);
						}
						else
						{
							std::vector<std::wstring> vehicleNameList;
							for (int i = 0; i < canConnect.size(); i++) vehicleNameList.push_back(canConnect[i]->name);
							new Lst(sysStr[95], sysStr[94], vehicleNameList);//넣기, 넣을 포켓을 선택해주세요.
							co_await std::suspend_always();
							errorBox(wtoi(coAnswer.c_str()) >= canConnect.size() || wtoi(coAnswer.c_str()) < 0, "Lst error, unknown vehicle selected");
							targetVehicle = canConnect[wtoi(coAnswer.c_str())];
							targetVehicle->extendPart(buildLocation[0], buildLocation[1], targetItemCode);
						}
					}
					else
					{
						Vehicle* newVehicle = new Vehicle(buildLocation[0], buildLocation[1], buildLocation[2], targetItemCode);
						new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//새로운 차량의 이름을 입력해주세요. 결정, 취소
						co_await std::suspend_always();
						if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
						else newVehicle->name = L"차량 " + std::to_wstring(randomRange(1, 9999999));
					}
				}
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::VPART))
			{
				Vehicle* targetVehicle = (Vehicle*)(World::ins()->getTile(buildLocation[0], buildLocation[1], buildLocation[2]).VehiclePtr);
				errorBox(targetVehicle == nullptr, "targetVehicle is nullptr in Craft.ixx");
				errorBox(!targetVehicle->hasFrame(buildLocation[0], buildLocation[1]), "first part doesn't have VFRAME flag");
				targetVehicle->addPart(buildLocation[0], buildLocation[1], targetItemCode);
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::PROP))
			{
				errorBox(World::ins()->getTile(buildLocation[0], buildLocation[1], buildLocation[2]).PropPtr != nullptr, L"이미 해당 좌표에 설치물이 존재하여 새로운 설치물을 설치할 수 없다.");
				new Prop(buildLocation[0], buildLocation[1], buildLocation[2], targetItemCode);
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////4.조합 데이터 초기화///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) deleteCraftDataStructure();
		else deleteCraftData();
		updateLog(L"#FFFFFF아이템 조합이 완료되었다.");
		isNowBuilding = false;
		close(aniFlag::null);
	}

	void saveCraftData(int code, int time)
	{
		ongoingTargetCode = code;
		ongoingElapsedTime = time;
		prt(L"저장이 이루어졌다. 코드는 %d 시간은 %d이다.\n", ongoingTargetCode, ongoingElapsedTime);
	}

	void loadCraftData(int& code, int& time)
	{
		code = ongoingTargetCode;
		time = ongoingElapsedTime;
	}

	void deleteCraftData()
	{
		ongoingTargetCode = -1;
		ongoingElapsedTime = -1;
	}

	bool existCraftData()
	{
		if (ongoingTargetCode == -1) return false;
		else return true;
	}


	void saveCraftDataStructure(int code, int time, std::array<int, 3> coord)
	{
		ongoingTargetCodeStructure = code;
		ongoingElapsedTimeStructure = time;
		buildLocation = coord;
		prt(L"저장이 이루어졌다. 코드는 %d 시간은 %d이다. 좌표는 %d,%d,%d이다.\n", ongoingTargetCode, ongoingElapsedTime, buildLocation[0], buildLocation[1], buildLocation[2]);
	}

	void loadCraftDataStructure(int& code, int& time, std::array<int, 3> coord)
	{
		code = ongoingTargetCodeStructure;
		time = ongoingElapsedTimeStructure;
		coord = buildLocation;
	}

	void deleteCraftDataStructure()
	{
		ongoingTargetCodeStructure = -1;
		ongoingElapsedTimeStructure = -1;
		buildLocation = { -1,-1,-1 };
	}

	bool existCraftDataStructure()
	{
		if (ongoingTargetCodeStructure == -1) return false;
		else return true;
	}

	bool getIsNowBuilding()
	{
		return isNowBuilding;
	}

	int getProcessPercent()
	{
		return (int)(100.0 * (float)elapsedTime / (float)itemDex[targetItemCode].craftTime);
	}

	int getProcessPercentOngoingStructure()
	{
		return (int)(100.0 * (float)ongoingElapsedTimeStructure / (float)itemDex[ongoingTargetCodeStructure].craftTime);
	}

	static std::array<int, 3> getBuildLocation()
	{
		return buildLocation;
	}
};