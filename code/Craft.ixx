#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Craft;

import std;
import util;
import GUI;
import globalVar;
import wrapVar;
import constVar;
import checkCursor;
import ItemPocket;
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
	SDL_Rect itemBox[24];

	int selectCategory = -1; //-2�̸� ���ã��, -1�̸� ��Ȱ��ȭ, 0�̸� ����, 1�� ��
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

	ItemPocket* recipePtr;//�÷��̾ ������ �����ǵ�

	bool deactColorChange = false;//���콺�� �������� �� ��ư���� �÷��� ���ϴ� ���� ��Ȱ��ȭ

	int numNoneBlackFilter; //�����Ͱ� �ƴ� �������� ��, ��ũ�� ������ �� �� ����. �������� ���ܸ��� �۵���Ű�� �� �ɷ��� �����Ͱ� ���Ǹ� �������� �����������

	bool showCraftingTooltip = false; //���� �߿� ȭ�� ��ܿ� �׷����� ��ô��, deactDraw�� ������� ������

	bool isNowBuilding = false;
	int targetItemCode = 0;
	int elapsedTime = 0; //�Ѱ� ���� �ð�
	inline static std::array<int, 3> buildLocation = { -1, -1, -1 };

	inline static int ongoingTargetCode = -1; //���� ���� ������
	inline static int ongoingElapsedTime = -1; //���� ��� �ð� ������

	inline static int ongoingTargetCodeStructure = -1; //���� ���� ���๰
	inline static int ongoingElapsedTimeStructure = -1; //���� ��� �ð� ���๰

public:
	Craft() : GUI(false)
	{
		prt(L"Craft : �����ڰ� ȣ��Ǿ����ϴ�..\n");
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;
		//�޼��� �ڽ� ������
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
		prt(L"Craft : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
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

		for (int i = 0; i < 24; i++)
		{
			itemBox[i].x = craftBase.x + 183 + (73 * i) % (73 * 6);
			itemBox[i].y = craftBase.y + 122 + 70 * ((73 * i) / (73 * 6));
			itemBox[i].w = 58;
			itemBox[i].h = 58;
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
				updateLog(L"#FFFFFF������ ������ ����ߴ�.");

				//���� ������ ����
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
		else if (checkCursor(&tooltipCraftBtn) && craftCursor >= 0)//���� �����ϱ�
		{
			CORO(executeCraft());
		}
		else if (checkCursor(&tooltipBookmarkBtn) && craftCursor >= 0)//���� ���ã��
		{
			CORO(executeBookmark());
		}
		else if (checkCursor(&unfoldBtn) && craftCursor >= 0)//���� ��ġ��
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
		else if (checkCursor(&bookmarkCategory))//���ã�� ī�װ� Ŭ����
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
			else //���� ������ ��� ��ü �з��� ����
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
			//ī�װ� 8��ư Ŭ����
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

			//����ī�װ� �Է�
			{
				int maxSubcategorySize;
				switch (selectCategory)
				{
				case -2://���ã��
					maxSubcategorySize = 6;
					break;
				case -1://��ü
					maxSubcategorySize = 1;
					break;
				case 0://����
					maxSubcategorySize = 5;
					break;
				case 1://��
					maxSubcategorySize = 5;
					break;
				case 2://����
					maxSubcategorySize = 6;
					break;
				case 3://�ķ�
					maxSubcategorySize = 6;
					break;
				case 4://�Ҹ�ǰ
					maxSubcategorySize = 5;
					break;
				case 5://���̿���
					maxSubcategorySize = 6;
					break;
				case 6://���๰
					maxSubcategorySize = 6;
					break;
				case 7://���
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
						else//���� ����ī�װ� ��ư�� ���� ������ ���
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

			//�����۹ڽ� Ŭ��

			for (int i = 0; i < 24; i++)
			{
				if (checkCursor(&itemBox[i]))
				{
					if (craftCursor != i + 6 * craftScroll && i + 6 * craftScroll < numNoneBlackFilter)
					{
						craftCursor = i + 6 * craftScroll;
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
					if (i + 6 * craftScroll < recipePtr->itemInfo.size())
					{
						if (recipePtr->itemInfo[i + 6 * craftScroll].checkFlag(itemFlag::BLACKFILTER) == false) pointingCursor = i;
					}
				}
			}
		}

		if (checkCursor(&craftBase))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // ���ӻ��, �۾������� ��ũ�� �ӵ��� ������
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
			int maxScroll = (numNoneBlackFilter - 1) / 6 - (CRAFT_MAX_ROW - 1);
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
		int maxScroll = myMax(0, (numNoneBlackFilter - 1) / 6 - (CRAFT_MAX_ROW - 1));

		if (craftScroll > maxScroll) { craftScroll = maxScroll; }
		else if (craftScroll < 0) { craftScroll = 0; }
	}

	void filterUpdate(int matchCount)
	{
		numNoneBlackFilter = matchCount;//Ŀ�� ��ġ ���ѿ�

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
		if (matchCountCanCraft >= 2) recipePtr->sortByUnicode(0, matchCountCanCraft - 1); //���� �Ұ��� �����۸� ����
	}

	bool canCraft(int itemCode, bool exceptMaterial)
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		//���տ� �ʿ��� �÷��̾� ��� üũ
		for (int i = 0; i < itemDex[itemCode].recipeProficNeed.size(); i++)
		{
			int needLevel = itemDex[itemCode].recipeProficNeed[i].second;
			int playerLevel = Player::ins()->getProficLevel(itemDex[itemCode].recipeProficNeed[i].first);
			if (playerLevel < needLevel) return false;
		}

		//���տ� �ʿ��� ���(�� ����Ƽ) üũ
		for (int i = 0; i < itemDex[itemCode].recipeQualityNeed.size(); i++)
		{
			if (equipPtr->checkToolQuality(itemDex[itemCode].recipeQualityNeed[i]) == false) return false;
		}

		if (exceptMaterial == false)
		{
			//���տ� �ʿ��� ��� üũ
			for (int i = 0; i < itemDex[itemCode].recipe.size(); i++)
			{
				//�� ����Ƽ�� ���� ����, ��� ��ȭ
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
			new Lst(sysStr[95], sysStr[94], numList);//�ֱ�, ���� ������ �������ּ���.
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
		prt(L"executeCraft �����\n");

		bool negateMonster = false;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////1.���� ��ǥ �� ������ ����///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (craftCursor != -1)//���� ������ ��
		{
			targetItemCode = recipePtr->itemInfo[craftCursor].itemCode;
			elapsedTime = 0;

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////1-1.���,����,��� üũ �� ����///////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (canCraft(targetItemCode) == false)//���,����,����� �����ϴ��� üũ
			{
				updateLog(L"#FFFFFF��ᰡ �����ϴ�.");
				co_return;
			}
			else//���տ� �ʿ��� ��� ����
			{
				for (int i = 0; i < itemDex[targetItemCode].recipe.size(); i++)
				{
					//�� ����Ƽ�� ���� ����, ��� ��ȭ
					int meterialItemCode = itemDex[meterialItemCode].recipe[i].first;
					int needNumber = itemDex[meterialItemCode].recipe[i].second;
					Player::ins()->getEquipPtr()->subtractItemCode(meterialItemCode, needNumber);
					Player::ins()->updateCustomSpriteHuman();
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////1-2.��ǥ ����///////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
			{
				if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))
				{
					//���ο� ������ ��ġ�Ͻðڽ��ϱ�?
					std::vector<std::array<int, 2>> selectableTile;
					for (int dir = -1; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						if (World::ins()->getTile(PlayerX() + dx, PlayerY() + dy, PlayerZ()).VehiclePtr == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						new CoordSelect(L"���� �������� ��ġ�� ��ġ�� �������ּ���.");
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
						updateLog(L"#FFFFFF�ֺ��� ���� �������� ��ġ�Ҹ��� ������ ����.");
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
						//������ǰ�̹Ƿ� �̹� �ִ� ������ ���� �Ǽ��Ǿ�� ��
						Vehicle* targetVehicle = (Vehicle*)(World::ins()->getTile(PlayerX() + dx, PlayerY() + dy, PlayerZ()).VehiclePtr);
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
						new CoordSelect(L"���� ��ǰ�� ��ġ�� �������� �������ּ���.");
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
						updateLog(L"#FFFFFF�ֺ��� ���� ��ǰ�� ��ġ�Ҹ��� �������� ����.");
						co_return;
					}
				}
				else
				{
					//���ο� ������ ��ġ�Ͻðڽ��ϱ�?
					std::vector<std::array<int, 2>> selectableTile;
					for (int dir = 0; dir < 8; dir++)
					{
						int dx, dy;
						dir2Coord(dir, dx, dy);
						if (TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
					}

					if (selectableTile.size() > 0)
					{
						deactDraw();
						new CoordSelectCraft(targetItemCode, L"������ �������� ��ġ�� ��ġ�� �������ּ���.", selectableTile);
						co_await std::suspend_always();
						actDraw();

						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						targetItemCode = wtoi(targetStr.c_str());

						buildLocation = { targetX,targetY,PlayerZ() };

					}
					else
					{
						updateLog(L"#FFFFFF�ֺ��� �ش� �������� ��ġ�Ҹ��� ������ ����.");
						co_return;
					}
				}
			}
		}
		else // ���� �簳 : ���ʷ� ����â�� ������ �� ���յ����� Ȯ��
		{
			if (existCraftData())
			{
				int percent = (int)(100.0 * (float)ongoingElapsedTime / (float)itemDex[ongoingTargetCode].craftTime);
				new Msg(msgFlag::normal, L"����", std::to_wstring(percent) + L"%���� ������ �ߴ��� �������� �����մϴ�.(%item3)��� �����Ͻðڽ��ϱ�?", { L"��",L"�ƴϿ�" });
				deactColorChange = true;
				co_await std::suspend_always();
				deactColorChange = false;

				if (coAnswer == L"��")
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
				int dx = abs(PlayerX() - buildLocation[0]);
				int dy = abs(PlayerY() - buildLocation[1]);
				int dz = abs(PlayerZ() - buildLocation[2]);

				if (dx <= 1 && dy <= 1 && dz == 0)
				{
					new Msg(msgFlag::normal, L"����", L"�ֺ��� ���� ���� ���๰�� �����մϴ�. (%item3) ���๰�� ������ ����Ͻðڽ��ϱ�?", { L"��",L"�ı�" });
					deactColorChange = true;
					co_await std::suspend_always();
					deactColorChange = false;
					if (coAnswer == L"��")
					{
						loadCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
						prt(L"���� ���� �����̼��� ��ǥ�� %d,%d,%d�̴�\n", buildLocation[0], buildLocation[1], buildLocation[2]);
					}
					else
					{
						deleteCraftDataStructure();
						co_return;
					}
				}
				else
				{
					std::wstring text = replaceStr(L"������ ��ǥ (��,��,��)�� ���� ���� ���๰�� �����մϴ�. (%item3) �ı��ϰ� ���ο� �������� �����Ͻðڽ��ϱ�?", L"��", { std::to_wstring(buildLocation[0]),std::to_wstring(buildLocation[1]),std::to_wstring(buildLocation[2]) });
					new Msg(msgFlag::normal, L"����", text, { L"��",L"�ƴϿ�" });
					deactColorChange = true;
					co_await std::suspend_always();
					deactColorChange = false;
					if (coAnswer == L"��")
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
		////////////////////////////////////////////2.��ǥ ���� �Ϸ� �� ���� ����///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) isNowBuilding = true;
		deactDraw();
		showCraftingTooltip = true;
		while (1)
		{
			prt(L"Craft While ���� �����\n");
			if (negateMonster == false)
			{
				for (int x = PlayerX() - 1; x <= PlayerX() + 1; x++)
				{
					for (int y = PlayerY() - 1; y <= PlayerY() + 1; y++)
					{
						if (!(x == PlayerX() && y == PlayerY()))
							if (World::ins()->getTile(x, y, PlayerZ()).fov == fovFlag::white)
								if (TileEntity(x, y, PlayerZ()) != nullptr)
								{
									new Msg(msgFlag::normal, L"���", L"�ֺ��� ���� �ֽ��ϴ�. ��� �����Ͻðڽ��ϱ�?", { L"��",L"�ƴϿ�",L"�����ϱ�" });
									deactColorChange = true;
									co_await std::suspend_always();
									if (coAnswer == L"��") goto loopEnd;
									else if (coAnswer == L"�����ϱ�")
									{
										negateMonster = true;
										goto loopEnd;
									}
									else
									{
										//���� ������ ����
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

			prt(L"exeCraft �ڷ�ƾ ���� ��\n");

			co_await std::suspend_always();

			prt(L"exeCraft �ڷ�ƾ ���� ��\n");

			elapsedTime++;
			if (elapsedTime >= itemDex[targetItemCode].craftTime) break;
		}


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////3.���� ���� : ������ ����///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
		{
			if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))//���ο� ���� ��ġ
			{
				//��ġ�� �ֺ��� ���� ������ ������ �ִ��� üũ
				std::vector<Vehicle*> canConnect;
				for (int dir = 0; dir < 8; dir++)
				{
					if (dir % 2 == 1) continue; //�밢�� �����
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
					new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//���ο� ������ �̸��� �Է����ּ���. ����, ���
					co_await std::suspend_always();
					if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
					else newVehicle->name = L"���� " + std::to_wstring(randomRange(1, 9999999));
				}
				else
				{
					new Msg(msgFlag::normal, L"���", L"��ġ�� �������� �ֺ� ������ �����Ͻðڽ��ϱ�?", { L"��",L"�ƴϿ�" });
					co_await std::suspend_always();
					if (coAnswer == L"��")
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
							new Lst(sysStr[95], sysStr[94], vehicleNameList);//�ֱ�, ���� ������ �������ּ���.
							co_await std::suspend_always();
							errorBox(wtoi(coAnswer.c_str()) >= canConnect.size() || wtoi(coAnswer.c_str()) < 0, "Lst error, unknown vehicle selected");
							targetVehicle = canConnect[wtoi(coAnswer.c_str())];
							targetVehicle->extendPart(buildLocation[0], buildLocation[1], targetItemCode);
						}
					}
					else
					{
						Vehicle* newVehicle = new Vehicle(buildLocation[0], buildLocation[1], buildLocation[2], targetItemCode);
						new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//���ο� ������ �̸��� �Է����ּ���. ����, ���
						co_await std::suspend_always();
						if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
						else newVehicle->name = L"���� " + std::to_wstring(randomRange(1, 9999999));
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
				errorBox(TileProp(buildLocation[0], buildLocation[1], buildLocation[2]) != nullptr, L"�̹� �ش� ��ǥ�� ��ġ���� �����Ͽ� ���ο� ��ġ���� ��ġ�� �� ����.");
				new Prop(buildLocation[0], buildLocation[1], buildLocation[2], targetItemCode);
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////4.���� ������ �ʱ�ȭ///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) deleteCraftDataStructure();
		else deleteCraftData();
		updateLog(L"#FFFFFF������ ������ �Ϸ�Ǿ���.");
		isNowBuilding = false;
		close(aniFlag::null);
	}

	void saveCraftData(int code, int time)
	{
		ongoingTargetCode = code;
		ongoingElapsedTime = time;
		prt(L"������ �̷������. �ڵ�� %d �ð��� %d�̴�.\n", ongoingTargetCode, ongoingElapsedTime);
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
		prt(L"������ �̷������. �ڵ�� %d �ð��� %d�̴�. ��ǥ�� %d,%d,%d�̴�.\n", ongoingTargetCode, ongoingElapsedTime, buildLocation[0], buildLocation[1], buildLocation[2]);
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