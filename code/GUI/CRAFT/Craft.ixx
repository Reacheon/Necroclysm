module;
#include <SDL3/SDL.h>

export module Craft;

import std;
import util;
import GUI;
import globalVar;
import wrapFunc;
import constVar;
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

	ItemPocket* recipePtr;//플레이어가 보유한 레시피들

	bool deactColorChange = false;//마우스를 가져갔을 때 버튼들의 컬러가 변하는 것을 비활성화

	//즐겨찾기 드롭다운 메뉴 상태
	static constexpr int BM_DD_BLOCK_H = 36;
	static constexpr int BM_DD_COUNT = 5; //북마크 슬롯 수
	bool bmDdOpen = false;
	float bmDdRatio = 0.0f; //애니메이션 비율 (0.0~1.0)
	SDL_Rect bmDdRect = {};

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

	// 창 상태 저장 변수들
	inline static int savedCraftCursor = -1;
	inline static int savedCraftScroll = 0;
	inline static int savedSelectCategory = -1;
	inline static int savedSelectSubcategory = -1;
	inline static std::wstring savedSearchInfo = L"";

public:
	Craft() : GUI(false)
	{
		prt(L"Craft : 생성자가 호출되었습니다..\n");
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;
		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);


		deactInput();
		deactDraw();
		addAniToPlayerTurn(this,aniFlag::winUnfoldOpen);


		exInputText = L"";
		recipePtr = ItemPocket::unlockRecipes.get();
		for (int i = 0; i < recipePtr->itemInfo.size(); i++)
		{
			recipePtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
			recipePtr->itemInfo[i].eraseFlag(itemFlag::BLACKFILTER);
		}
		numNoneBlackFilter = recipePtr->itemInfo.size();

		// 저장된 창 상태 복원
		craftCursor = savedCraftCursor;
		craftScroll = savedCraftScroll;
		selectCategory = savedSelectCategory;
		selectSubcategory = savedSelectSubcategory;
		searchInfo = savedSearchInfo;
		exInputText = savedSearchInfo; // 검색어 복원

		// 필터 재적용
		if (savedSearchInfo.empty() == false)
		{
			// 검색어가 있으면 검색 필터 재적용
			int matchCount = recipePtr->searchTxt(exInputText);
			filterUpdate(matchCount);
		}
		else if (selectCategory >= 0)
		{
			// 카테고리가 선택되어 있으면 카테고리 필터 재적용
			itemCategory targetCategory;
			switch (selectCategory)
			{
			case 0: targetCategory = itemCategory::equipment; break;
			case 1: targetCategory = itemCategory::foods; break;
			case 2: targetCategory = itemCategory::tools; break;
			case 3: targetCategory = itemCategory::tech; break;
			case 4: targetCategory = itemCategory::consumables; break;
			case 5: targetCategory = itemCategory::vehicles; break;
			case 6: targetCategory = itemCategory::structures; break;
			case 7: targetCategory = itemCategory::materials; break;
			}

			if (selectSubcategory == 0)
			{
				// 카테고리 전체
				int matchCount = recipePtr->searchCategory(targetCategory);
				filterUpdate(matchCount);
			}
			else
			{
				// 서브카테고리 선택
				itemSubcategory targetSubcategory;
				switch (selectCategory)
				{
				case 0: // 장비
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::equipment_melee;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::equipment_ranged;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::equipment_firearms;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::equipment_throwing;
					else if (selectSubcategory == 5) targetSubcategory = itemSubcategory::equipment_clothing;
					break;
				case 1: // 음식
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::foods_cooked;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::foods_processed;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::foods_preserved;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::foods_drinks;
					else if (selectSubcategory == 5) targetSubcategory = itemSubcategory::foods_ingredients;
					break;
				case 2: // 도구
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::tools_hand;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::tools_power;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::tools_containers;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::tools_etc;
					break;
				case 3: // 기술
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::tech_bionics;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::tech_powerArmor;
					break;
				case 4: // 소모품
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::consumable_medicine;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::consumable_ammo;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::consumable_fuel;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::consumable_etc;
					break;
				case 5: // 차량
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::vehicle_frames;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::vehicle_power;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::vehicle_exteriors;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::vehicle_parts;
					break;
				case 6: // 건축물
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::structure_walls;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::structure_floors;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::structure_ceilings;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::structure_props;
					break;
				case 7: // 재료
					if (selectSubcategory == 1) targetSubcategory = itemSubcategory::material_metals;
					else if (selectSubcategory == 2) targetSubcategory = itemSubcategory::material_organic;
					else if (selectSubcategory == 3) targetSubcategory = itemSubcategory::material_components;
					else if (selectSubcategory == 4) targetSubcategory = itemSubcategory::material_chemicals;
					else if (selectSubcategory == 5) targetSubcategory = itemSubcategory::material_etc;
					break;
				}
				int matchCount = recipePtr->searchSubcategory(targetSubcategory);
				filterUpdate(matchCount);
			}
		}
		else if (selectCategory == -2)
		{
			// 북마크가 선택되어 있으면 북마크 필터 재적용
			itemFlag targetFlag;
			if (selectSubcategory == 0) targetFlag = itemFlag::BOOKMARK1;
			else if (selectSubcategory == 1) targetFlag = itemFlag::BOOKMARK2;
			else if (selectSubcategory == 2) targetFlag = itemFlag::BOOKMARK3;
			else if (selectSubcategory == 3) targetFlag = itemFlag::BOOKMARK4;
			else if (selectSubcategory == 4) targetFlag = itemFlag::BOOKMARK5;
			int matchCount = recipePtr->searchFlag(targetFlag);
			filterUpdate(matchCount);
		}

		if (existCraftData() || existCraftDataStructure())
		{
			Corouter::start(executeCraft());
		}
	}
	~Craft()
	{
		prt(L"Craft : 소멸자가 호출되었습니다..\n");

		// 현재 창 상태 저장
		savedCraftCursor = craftCursor;
		savedCraftScroll = craftScroll;
		savedSelectCategory = selectCategory;
		savedSelectSubcategory = selectSubcategory;
		savedSearchInfo = exInputText; // 검색어 저장 (전역 변수에서)

		PlayerPtr->setFakeX(0);
		PlayerPtr->setFakeY(0);
		changePlayerWalkMode(walkFlag::walk);
		PlayerPtr->entityInfo.sprIndex = charSprIndex::WALK;
		ptr = nullptr;
	}
	static Craft* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		craftBase = { 0, 0, 975, 600 };

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

		topWindow = { 0, 0, 615, 222 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

		int categoryX = craftBase.x + 14;
		int categoryY = craftBase.y + 54 + 21 + 42;
		int categoryIntervalW = 120;
		int categoryIntervalH = 120;


		bookmarkCategory.x = craftBase.x + 14;
		bookmarkCategory.y = craftBase.y + 59;
		bookmarkCategory.w = 228;
		bookmarkCategory.h = 50;

		for (int i = 0; i < 8; i++)
		{
			craftCategory[i].x = categoryX + categoryIntervalW * (i % 2);
			craftCategory[i].y = categoryY + categoryIntervalH * (i / 2);
			craftCategory[i].w = 108;
			craftCategory[i].h = 108;
		}

		for (int i = 0; i < 8; i++)
		{
			subcategoryBox[i] = { craftBase.x + 254 + 102 * i, craftBase.y + 140, 102, 30 };
		}

		searchTextRect = { craftBase.x + craftBase.w - 336, craftBase.y + 69, 225, 45 };
		searchBtnRect = { searchTextRect.x + 230, searchTextRect.y, 72, searchTextRect.h };

		for (int i = 0; i < 24; i++)
		{
			itemBox[i].x = craftBase.x + 266 + (228 * i) % (228 * 3);
			itemBox[i].y = craftBase.y + 191 + 50 * ((228 * i) / (228 * 3));
			itemBox[i].w = 218;
			itemBox[i].h = 41;
		}

		tooltipCraftBtn = { topWindow.x + 465, topWindow.y + 15, 135, 39 };
		tooltipBookmarkBtn = { topWindow.x + 465, topWindow.y + 15 + 48, 135, 39 };

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

	void clickUpGUI();
    void clickMotionGUI(int dx, int dy);
	void clickDownGUI();

	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel();

	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		tabType = tabFlag::back;

		int maxScroll = myMax(0, (numNoneBlackFilter - 1) / CRAFT_MAX_COLUMN - (CRAFT_MAX_ROW - 1));
		if (craftScroll > maxScroll) { craftScroll = maxScroll; }
		else if (craftScroll < 0) { craftScroll = 0; }

		//즐겨찾기 드롭다운 애니메이션
		if (bmDdOpen && bmDdRatio < 1.0f)
		{
			bmDdRatio += 0.1f;
			if (bmDdRatio >= 1.0f) bmDdRatio = 1.0f;
		}
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

	void openBookmarkDropdown();
	void closeBookmarkDropdown();
	void drawBookmarkDropdown();
	void selectBookmark(int slotIndex);

	Corouter executeCraft();

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

	void executeTab()
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
};