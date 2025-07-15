﻿#include <SDL3/SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Craft;

import std;
import util;
import GUI;
import globalVar;
import wrapVar;
import constVar;
import checkCursor;
import Lst;
import Player;
import World;
import CoordSelect;
import CoordSelectCraft;
import log;
import Msg;
import Vehicle;
import Prop;
import turnWait;
import ItemData;
import ItemPocket;
import GameOver;


export class Craft : public GUI
{
private:
	inline static Craft* ptr = nullptr;

	int CRAFT_MAX_COLUMN = 3; //아이템 박스의 최대 열
	int CRAFT_MAX_ROW = 8; //아이템 박스의 최대 행

	SDL_Rect craftBase;
	SDL_Rect topWindow;
	SDL_Rect bookmarkCategory;
	SDL_Rect subcategoryBox[8];
	SDL_Rect searchTextRect;
	SDL_Rect searchBtnRect;
	std::array<SDL_Rect, 8> craftCategory;
	SDL_Rect itemBox[24];

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
	inline static Point3 buildLocation = { -1, -1, -1 };

	inline static int ongoingTargetCode = -1; //제작 중인 아이템
	inline static int ongoingElapsedTime = -1; //제작 경과 시간 아이템

	inline static int ongoingTargetCodeStructure = -1; //제작 중인 건축물
	inline static int ongoingElapsedTimeStructure = -1; //제작 경과 시간 건축물


public:
	Craft() : GUI(false)
	{
		prt(L"Craft : 생성자가 호출되었습니다..\n");
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;
		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this,aniFlag::winUnfoldOpen);


		exInputText = L"";
		recipePtr = availableRecipe.get();
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

		PlayerPtr->setFakeX(0);
		PlayerPtr->setFakeY(0);
		PlayerPtr->entityInfo.walkMode = walkFlag::walk;
		PlayerPtr->entityInfo.sprIndex = charSprIndex::WALK;

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
			subcategoryBox[i] = { craftBase.x + 169 + 68 * i, craftBase.y + 93, 68, 20 };
		}

		searchTextRect = { craftBase.x + craftBase.w - 224, craftBase.y + 46, 150, 30 };
		searchBtnRect = { searchTextRect.x + 153, searchTextRect.y, 48, searchTextRect.h };

		for (int i = 0; i < 24; i++)
		{
			itemBox[i].x = craftBase.x + 177 + (152 * i) % (152 * 3);
			itemBox[i].y = craftBase.y + 127 + 33 * ((152 * i) / (152 * 3));
			itemBox[i].w = 145;
			itemBox[i].h = 27;
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
	void drawGUI();
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		exInput = false;
		if (checkCursor(&tab))
		{
			if (showCraftingTooltip == false) close(aniFlag::winUnfoldClose);
			else
			{
				updateLog(sysStr[317]);//아이템 조합을 취소했다.

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
			if (exInput == false)
			{
				exInput = true;
				SDL_StartTextInput(window);  // 추가
			}
			else
			{
				exInput = false;
				SDL_StopTextInput(window);  // 추가
			}
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
						selectSubcategory = 0;

						itemCategory targetCategory;
						switch (selectCategory)
						{
						case 0:
							targetCategory = itemCategory::equipment;
							break;
						case 1:
							targetCategory = itemCategory::foods;
							break;
						case 2:
							targetCategory = itemCategory::tools;
							break;
						case 3:
							targetCategory = itemCategory::tech;
							break;
						case 4:
							targetCategory = itemCategory::consumables;
							break;
						case 5:
							targetCategory = itemCategory::vehicles;
							break;
						case 6:
							targetCategory = itemCategory::structures;
							break;
						case 7:
							targetCategory = itemCategory::materials;
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
						numNoneBlackFilter = recipePtr->itemInfo.size();
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
				case 0://장비
					maxSubcategorySize = 6;
					break;
				case 1://음식
					maxSubcategorySize = 6;
					break;
				case 2://도구
					maxSubcategorySize = 5; 
					break;
				case 3://기술
					maxSubcategorySize = 3;
					break;
				case 4://소모품
					maxSubcategorySize = 5;
					break;
				case 5://차량
					maxSubcategorySize = 5;
					break;
				case 6://건축물
					maxSubcategorySize = 5;
					break;
				case 7://재료
					maxSubcategorySize = 6;
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
							case 0://장비
								if (selectSubcategory == 0)
								{
									// All - 전체 장비 카테고리 표시
									itemCategory targetCategory = itemCategory::equipment;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::equipment_melee; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::equipment_ranged; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::equipment_firearms; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::equipment_throwing; }
									else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::equipment_clothing; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 1://음식
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::foods;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::foods_cooked; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::foods_processed; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::foods_preserved; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::foods_drinks; }
									else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::foods_ingredients; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 2://도구
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::tools;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::tools_hand; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::tools_power; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::tools_containers; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::tools_etc; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 3://기술
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::tech;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::tech_bionics; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::tech_powerArmor; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 4://소모품
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::consumables;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::consumable_medicine; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::consumable_ammo; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::consumable_fuel; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::consumable_etc; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 5://차량
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::vehicles;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::vehicle_frames; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::vehicle_power; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::vehicle_exteriors; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::vehicle_parts; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 6://건축물
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::structures;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::structure_walls; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::structure_floors; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::structure_ceilings; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::structure_props; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							case 7://재료
								if (selectSubcategory == 0)
								{
									itemCategory targetCategory = itemCategory::materials;
									matchCount = recipePtr->searchCategory(targetCategory);
								}
								else
								{
									if (selectSubcategory == 1) { targetSubcategory = itemSubcategory::material_metals; }
									else if (selectSubcategory == 2) { targetSubcategory = itemSubcategory::material_organic; }
									else if (selectSubcategory == 3) { targetSubcategory = itemSubcategory::material_components; }
									else if (selectSubcategory == 4) { targetSubcategory = itemSubcategory::material_chemicals; }
									else if (selectSubcategory == 5) { targetSubcategory = itemSubcategory::material_etc; }
									matchCount = recipePtr->searchSubcategory(targetSubcategory);
								}
								break;
							}
							filterUpdate(matchCount);
						}
						break;
					}
				}
			}

			//아이템박스 클릭
			for (int i = 0; i < 24; i++)
			{
				if (checkCursor(&itemBox[i]))
				{
					if (craftCursor != i + CRAFT_MAX_COLUMN * craftScroll && i + CRAFT_MAX_COLUMN * craftScroll < numNoneBlackFilter)
					{
						craftCursor = i + CRAFT_MAX_COLUMN * craftScroll;
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
			for (int i = 0; i < 24; i++)
			{
				if (checkCursor(&itemBox[i]))
				{
					if (i + CRAFT_MAX_COLUMN * craftScroll < recipePtr->itemInfo.size())
					{
						if (recipePtr->itemInfo[i + CRAFT_MAX_COLUMN * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false) pointingCursor = i;
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
					itemListColorLock = true;
				}
			}
		}
	}
	void clickDownGUI()
	{
		initCraftScroll = craftScroll;
	}
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() 
	{
		if (checkCursor(&craftBase))
		{
			int maxScroll = (numNoneBlackFilter - 1) / CRAFT_MAX_COLUMN - (CRAFT_MAX_ROW - 1);
			if (maxScroll < 0) maxScroll = 0;

			if (event.wheel.y > 0 && craftScroll > 0) craftScroll -= 1;
			else if (event.wheel.y < 0 && craftScroll < maxScroll) craftScroll += 1;
		}
	}
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		int maxScroll = myMax(0, (numNoneBlackFilter - 1) / CRAFT_MAX_COLUMN - (CRAFT_MAX_ROW - 1));
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

		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
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
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		//조합에 필요한 플레이어 재능 체크
		for (int i = 0; i < itemDex[itemCode].recipeProficNeed.size(); i++)
		{
			int needLevel = itemDex[itemCode].recipeProficNeed[i].second;
			int playerLevel = PlayerPtr->getProficLevel(itemDex[itemCode].recipeProficNeed[i].first);
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
			if (coAnswer.empty() == false)
			{
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
	}

	Corouter executeCraft()
	{
		bool negateMonster = false;

		// ============================================================
		// 1. 조합 좌표 및 아이템 설정
		// ============================================================

		if (craftCursor != -1)//최초 조합일 때
		{
			targetItemCode = recipePtr->itemInfo[craftCursor].itemCode;
			elapsedTime = 0;

			// ============================================================
			// 1-1. 재료, 도구, 재능 체크 및 제거
			// ============================================================
			if (debug::noCraftMaterialNeed == false)
			{
				if (canCraft(targetItemCode) == false)//재료,도구,재능을 만족하는지 체크
				{
					updateLog(sysStr[318]);//재료가 부족하다.
					co_return;
				}
				else//조합에 필요한 재료 제거
				{
					for (int i = 0; i < itemDex[targetItemCode].recipe.size(); i++)
					{
						//툴 퀄리티에 따라 적색, 녹색 변화
						int materialItemCode = itemDex[targetItemCode].recipe[i].first;
						int needNumber = itemDex[targetItemCode].recipe[i].second;
						PlayerPtr->getEquipPtr()->subtractItemCode(materialItemCode, needNumber);
					}
				}
			}

			// ============================================================
			// 1-2. 좌표 선택
			// ============================================================
			if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
			{
				if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))
				{
					//새로운 차량을 설치하시겠습니까?
					std::vector<Point2> selectableTile;
					for (int dir = -1; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						if (TileVehicle(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });

						new CoordSelect(sysStr[303]);//차량 프레임을 설치할 위치를 선택해주세요.
						co_await std::suspend_always();
						rangeSet.clear();
						actDraw();

						if (coAnswer.empty() == false)
						{
							std::wstring targetStr = coAnswer;
							int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
							targetStr.erase(0, targetStr.find(L",") + 1);
							int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
							targetStr.erase(0, targetStr.find(L",") + 1);
							int targetZ = wtoi(targetStr.c_str());

							buildLocation = { targetX,targetY,PlayerZ() };

							createProp(buildLocation, 453);

							TileProp(buildLocation)->leadItem.propSprIndex = itemDex[targetItemCode].propWIPSprIndex;

							PlayerPtr->setFakeX(3 * (buildLocation.x - PlayerX()));
							PlayerPtr->setFakeY(3 * (buildLocation.y - PlayerY()));
							PlayerPtr->setDirection(coord2Dir(buildLocation.x - PlayerX(), buildLocation.y - PlayerY()));
						}
						else co_return;
					}
					else
					{
						updateLog(sysStr[300]);//주변에 차량 프레임을 설치할만한 공간이 없다.
						co_return;
					}
				}
				else if (itemDex[targetItemCode].checkFlag(itemFlag::VPART))
				{
					std::vector<Point2> selectableTile;
					for (int dir = -1; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						Vehicle* targetVehicle = TileVehicle(PlayerX() + dx, PlayerY() + dy, PlayerZ()); //차량부품이므로 이미 있는 프레임 위에 건설되어야 함
						if (targetVehicle != nullptr)
						{
							if (targetVehicle->hasFrame(PlayerX() + dx, PlayerY() + dy))
							{
								selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
							}
						}
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i][0],selectableTile[i][1] });

						new CoordSelect(sysStr[304]);//차량 부품을 설치할 프레임을 선택해주세요. 
						co_await std::suspend_always();
						rangeSet.clear();
						actDraw();

						if (coAnswer.empty() == false)
						{
							std::wstring targetStr = coAnswer;
							int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
							targetStr.erase(0, targetStr.find(L",") + 1);
							int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
							targetStr.erase(0, targetStr.find(L",") + 1);
							int targetZ = wtoi(targetStr.c_str());
							buildLocation = { targetX,targetY,targetZ };
						}
						else co_return;

					}
					else
					{
						updateLog(sysStr[301]);//주변에 차량 부품을 설치할만한 프레임이 없다. 
						co_return;
					}
				}
				else
				{
					//새로운 차량을 설치하시겠습니까?
					std::vector<Point2> selectableTile;
					for (int dir = 0; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						if (TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });

						new CoordSelectCraft(targetItemCode, sysStr[299], selectableTile);//조합할 아이템을 설치할 위치를 선택해주세요.
						co_await std::suspend_always();
						rangeSet.clear();
						actDraw();

						if (coAnswer.empty() == false)
						{
							std::wstring targetStr = coAnswer;
							int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
							targetStr.erase(0, targetStr.find(L",") + 1);
							int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
							targetStr.erase(0, targetStr.find(L",") + 1);
							int targetItemCode = wtoi(targetStr.c_str());

							buildLocation = { targetX,targetY,PlayerZ() };

							createProp(buildLocation, 453);

							TileProp(buildLocation)->leadItem.propSprIndex = itemDex[targetItemCode].propWIPSprIndex;

							PlayerPtr->setFakeX(3 * (buildLocation.x - PlayerX()));
							PlayerPtr->setFakeY(3 * (buildLocation.y - PlayerY()));
							PlayerPtr->setDirection(coord2Dir(buildLocation.x - PlayerX(), buildLocation.y - PlayerY()));
						}
						else co_return;

					}
					else
					{
						updateLog(sysStr[302]);//주변에 해당 아이템을 설치할만한 공간이 없다.
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
				//알림, (%number)%에서 조합을 중단한 아이템이 존재합니다.계속 조합하시겠습니까?, 계속, 아니오, 파기
				new Msg(msgFlag::normal, sysStr[307], replaceStr(sysStr[313], L"(%number)", std::to_wstring(percent)), { sysStr[312],sysStr[37],sysStr[314] }, ongoingTargetCode);
				deactColorChange = true;
				co_await std::suspend_always();
				deactColorChange = false;

				if (coAnswer.empty() == false)
				{
					if (coAnswer == sysStr[312])//계속
					{
						loadCraftData(targetItemCode, elapsedTime);
					}
					else if (coAnswer == sysStr[37])//아니오
					{
						close(aniFlag::null);
						co_return;
					}
					else
					{
						deleteCraftData();
						co_return;
					}
				}
				else
				{
					close(aniFlag::null);
					co_return;
				}
			}
			else if (existCraftDataStructure())
			{
				int dx = abs(PlayerX() - buildLocation.x);
				int dy = abs(PlayerY() - buildLocation.y);
				int dz = abs(PlayerZ() - buildLocation.z);

				if (dx <= 1 && dy <= 1 && dz == 0)
				{
					//알림,조합 중인 아이템이 주변에 있습니다.조합을 계속하시겠습니까? ,계속,아니오,파기
					new Msg(msgFlag::normal, sysStr[307], sysStr[315], { sysStr[312],sysStr[37],sysStr[314] }, ongoingTargetCodeStructure);
					deactColorChange = true;
					co_await std::suspend_always();
					deactColorChange = false;
					if (coAnswer.empty() == false)
					{
						if (coAnswer == sysStr[312])//계속
						{
							loadCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
							prt(L"현재 빌드 로케이션의 좌표는 %d,%d,%d이다\n", buildLocation.x, buildLocation.y, buildLocation.z);
						}
						else if (coAnswer == sysStr[37])//아니오
						{
							close(aniFlag::null);
							co_return;
						}
						else
						{
							deleteCraftDataStructure();
							co_return;
						}
					}
					else
					{
						close(aniFlag::null);
						co_return;
					}
				}
				else
				{
					//떨어진 좌표 (▲,▲,▲)에 조합 중인 아이템이 존재합니다. 파기하고 새로운 아이템을 제작하시겠습니까?
					std::wstring text = replaceStr(sysStr[316], L"▲", { std::to_wstring(buildLocation.x),std::to_wstring(buildLocation.y),std::to_wstring(buildLocation.z) });
					//알림,네, 아니오
					new Msg(msgFlag::normal, sysStr[307], text, { sysStr[36],sysStr[37] }, ongoingTargetCodeStructure);
					deactColorChange = true;
					co_await std::suspend_always();
					deactColorChange = false;

					if (coAnswer.empty() == false)
					{
						if (coAnswer == sysStr[36])//네
						{
							deleteCraftDataStructure();
							co_return;
						}
						else//아니오
						{
							close(aniFlag::null);
							co_return;
						}
					}
					else
					{
						close(aniFlag::null);
						co_return;
					}
				}

			}
		}

		// ============================================================
		// 2. 좌표 선정 완료 후 조합 루프
		// ============================================================
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) isNowBuilding = true;
		deactDraw();
		showCraftingTooltip = true;
		while (1)
		{
			if (negateMonster == false)
			{
				for (int x = PlayerX() - 1; x <= PlayerX() + 1; x++)
				{
					for (int y = PlayerY() - 1; y <= PlayerY() + 1; y++)
					{
						if (!(x == PlayerX() && y == PlayerY()))
							if (TileFov(x, y, PlayerZ()) == fovFlag::white)
								if (TileEntity(x, y, PlayerZ()) != nullptr)
								{
									//경고, 주변에 적이 있습니다. 계속 조합하시겠습니까?
									new Msg(msgFlag::normal, sysStr[306], sysStr[310], { sysStr[36],sysStr[37],sysStr[311] });
									deactColorChange = true;
									co_await std::suspend_always();
									if (coAnswer.empty() == false)
									{
										if (coAnswer == sysStr[36]) goto loopEnd;//네
										else if (coAnswer == sysStr[311])//무시하기
										{
											negateMonster = true;
											goto loopEnd;
										}
										else//아니오
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
									else goto loopEnd; //탭 누르면 계속하는걸로 간주
								}
					}
				}
			}

		loopEnd:
			if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT) && itemDex[targetItemCode].checkFlag(itemFlag::VPART) == false)
			{
				int pSprIndex = TileProp(buildLocation)->leadItem.propSprIndex;
				float ratio = (float)elapsedTime / (float)itemDex[targetItemCode].craftTime;

				int baseIndex = (pSprIndex / 4) * 4;
				int progressIndex;

				if (ratio < 0.25) progressIndex = 0;
				else if (ratio < 0.5) progressIndex = 1;
				else if (ratio < 0.75) progressIndex = 2;
				else progressIndex = 3;

				TileProp(buildLocation)->leadItem.propSprIndex = baseIndex + progressIndex;
			}


			turnWait(1.0);
			coTurnSkip = true;

			co_await std::suspend_always();

			elapsedTime++;
			if (elapsedTime >= itemDex[targetItemCode].craftTime)
			{
				if (TileProp(buildLocation) != nullptr) destroyProp(buildLocation);
				break;
			}
		}


		// ============================================================
		// 3. 조합 종료 : 아이템 생성
		// ============================================================

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
					Vehicle* targetCoordPtr = TileVehicle(buildLocation.x + dx, buildLocation.y + dy, buildLocation.z);
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
					Vehicle* newVehicle = new Vehicle(buildLocation.x, buildLocation.y, buildLocation.z, targetItemCode);
					new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//새로운 차량의 이름을 입력해주세요. 결정, 취소
					co_await std::suspend_always();
					if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
					else newVehicle->name = sysStr[305]+L" " + std::to_wstring(randomRange(1, 9999999));//차량+번호
				}
				else
				{
					//확인, 설치한 프레임을 주변 차량에 연결하시겠습니까?, 네, 아니오
					new Msg(msgFlag::normal, sysStr[308], sysStr[309], { sysStr[36],sysStr[37] });
					co_await std::suspend_always();
					
					if (coAnswer == sysStr[36])//네
					{
						Vehicle* targetVehicle;
						if (canConnect.size() == 1)
						{
							targetVehicle = canConnect[0];
							targetVehicle->extendPart(buildLocation.x, buildLocation.y, targetItemCode);
						}
						else
						{
							std::vector<std::wstring> vehicleNameList;
							for (int i = 0; i < canConnect.size(); i++) vehicleNameList.push_back(canConnect[i]->name);
							new Lst(sysStr[95], sysStr[94], vehicleNameList);//넣기, 아이템을 넣을 포켓을 골라주세요.
							co_await std::suspend_always();
							
							int targetNumber = 0;
                            if (coAnswer.empty() == false) targetNumber = wtoi(coAnswer.c_str());
							errorBox(targetNumber >= canConnect.size() || targetNumber < 0, L"Lst error, unknown vehicle selected");
							targetVehicle = canConnect[targetNumber];
							targetVehicle->extendPart(buildLocation.x, buildLocation.y, targetItemCode);
						}
					}
					else
					{
						Vehicle* newVehicle = new Vehicle(buildLocation.x, buildLocation.y, buildLocation.z, targetItemCode);
						new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//새로운 차량의 이름을 입력해주세요. 결정, 취소
						co_await std::suspend_always();
						if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
						else newVehicle->name = sysStr[305] + L" " + std::to_wstring(randomRange(1, 9999999));//차량+번호
					}
				}
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::VPART))
			{
				Vehicle* targetVehicle = TileVehicle(buildLocation.x, buildLocation.y, buildLocation.z);
				errorBox(targetVehicle == nullptr, L"targetVehicle is nullptr in Craft.ixx");
				errorBox(!targetVehicle->hasFrame(buildLocation.x, buildLocation.y), L"first part doesn't have VFRAME flag");
				targetVehicle->addPart(buildLocation.x, buildLocation.y, targetItemCode);
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::PROP))
			{
				errorBox(TileProp(buildLocation.x, buildLocation.y, buildLocation.z) != nullptr, L"Cannot install new structure because one already exists at this location.");
				createProp({ buildLocation.x, buildLocation.y, buildLocation.z }, targetItemCode);
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::WALL))
			{
				int a = targetItemCode;
				setWall(buildLocation, targetItemCode);
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::FLOOR))
			{
				setFloor(buildLocation, targetItemCode);
			}
		}

		// ============================================================
		// 4. 조합 데이터 초기화
		// ============================================================
		updateLog(replaceStr(sysStr[298], L"(%item)", itemDex[targetItemCode].name)); //(%item) 설치를 완료하였다.
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) deleteCraftDataStructure();
		else deleteCraftData();


		
		isNowBuilding = false;
		close(aniFlag::null);
	}

	void saveCraftData(int code, int time)
	{
		ongoingTargetCode = code;
		ongoingElapsedTime = time;
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


	void saveCraftDataStructure(int code, int time, Point3 coord)
	{
		ongoingTargetCodeStructure = code;
		ongoingElapsedTimeStructure = time;
		buildLocation = coord;
	}

	void loadCraftDataStructure(int& code, int& time, Point3 coord)
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

	static Point3 getBuildLocation()
	{
		return buildLocation;
	}
};