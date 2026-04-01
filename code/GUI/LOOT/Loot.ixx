module;
#include <SDL3/SDL.h>

export module Loot;

import std;
import util;
import globalVar;
import constVar;
import drawWindowArrow;
import ItemPocket;
import drawItemList;
import checkCursor;
import drawSprite;
import drawText;
import Player;
import World;
import textureVar;
import log;
import ItemStack;
import Msg;
import GUI;
import actFuncSet;
import drawWindow;
import ItemData;
import ItemListPanel;
import barActCommon;


export class Loot : public GUI
{
private:
	inline static Loot* ptr = nullptr;

	ItemStack* lootStack = nullptr; //만약 창고같이 스택이 없으면 null로 유지됨
	ItemData* lootItemData = nullptr;

	int pocketCursor = 0; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	int initLootScroll = 0; //모션스크롤이 시작되기 직전의 스크롤
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;

	std::wstring titleInventory;

	SDL_Rect lootBase;
	SDL_Rect lootTitle;
	SDL_Rect lootArea;
	SDL_Rect pocketWindow;
	SDL_Rect pocketItem[7];
	SDL_Rect pocketLeft;
	SDL_Rect pocketRight;
	SDL_Rect lootBtn;

	dir16 arrowDir = dir16::left;
public:
	ItemListPanel panel{ LOOT_ITEM_MAX };

	Corouter errorFunc();

	Loot(ItemPocket* inputPocket, ItemData* inputData, Point3 tgtPoint) : GUI(false)
	{
		ptr = this;
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);

		int revX = tgtPoint.x - PlayerX();
		int revY = tgtPoint.y - PlayerY();

		int arrowEndX, arrowEndY, targetX, targetY;
		if (revX >= 0)
		{
			arrowDir = dir16::left;
			arrowEndX = cameraW / 2 + 8 * zoomScale + 16*revX * zoomScale;
			arrowEndY = cameraH / 2 + 16 * revY * zoomScale;
			targetX = arrowEndX + 26;
			targetY = arrowEndY - 170;
		}
		else
		{
			arrowDir = dir16::right;
			arrowEndX = cameraW / 2 - 8 * zoomScale + 16 * revX * zoomScale;
			arrowEndY = cameraH / 2 + 16 * revY * zoomScale;
			targetX = arrowEndX - 429;
			targetY = arrowEndY - 170;
		}

		changeXY(targetX, targetY, false);

		UIType = act::loot;

		panel.pocket = inputPocket;
		lootItemData = inputData;

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (option::inputMethod == input::gamepad) panel.cursor = 0;
	}

	Loot(ItemStack* inputStack) : GUI(false)
	{
		ptr = this;
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);

		int revX = inputStack->getGridX() - PlayerX();
		int revY = inputStack->getGridY() - PlayerY();

        int arrowEndX, arrowEndY, targetX, targetY;
		if (revX >= 0)
		{
			arrowDir = dir16::left;
			arrowEndX = cameraW / 2 + 8 * zoomScale + 16 * revX * zoomScale;
			arrowEndY = cameraH / 2 + 16 * revY * zoomScale;
			targetX = arrowEndX + 26;
			targetY = arrowEndY - 170;
		}
		else
		{
            arrowDir = dir16::right;
			arrowEndX = cameraW / 2 - 8 * zoomScale + 16 * revX * zoomScale;
			arrowEndY = cameraH / 2 + 16 * revY * zoomScale;
			targetX = arrowEndX - 429;
			targetY = arrowEndY - 170;
		}

		changeXY(targetX, targetY, false);

		UIType = act::loot;

		lootStack = inputStack;
		panel.pocket = inputStack->getPocket();
		lootItemData = nullptr;

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (option::inputMethod == input::gamepad) panel.cursor = 0;
	}

	~Loot()
	{
		prt(L"Loot : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		panel.clearAllSelections();
		panel.cursor = -1;
		panel.scroll = 0;
		barAct = actSet::null;
		barActCursor = -1;
	}
	static Loot* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		lootBase = { 0, 0, 404, 506 };
		if (center == false)
		{
			lootBase.x += inputX;
			lootBase.y += inputY;
		}
		else
		{
			lootBase.x += inputX - lootBase.w / 2;
			lootBase.y += inputY - lootBase.h / 2;
		}
		lootTitle = { lootBase.x + 123, lootBase.y + 0, 157, 36 };
		lootArea = { lootBase.x + 12, lootBase.y + 150, 376, 296 };
		panel.initRects(lootBase.x, lootBase.y + 150);
		panel.label = { lootBase.x + 12, lootBase.y + 114, lootBase.w - 24, 31 };
		panel.labelSelect = { panel.label.x, panel.label.y, 75, 31 };
		panel.labelName = { panel.label.x + panel.labelSelect.w, panel.label.y, 219, 31 };
		panel.labelQuantity = { panel.label.x + panel.labelName.w + panel.labelSelect.w, panel.label.y, 85, 31 };
		pocketWindow = { lootBase.x + 0, lootBase.y + 41, 404, 84 };
		pocketLeft = { lootBase.x + 7, lootBase.y + 38, 29, 53 };
		pocketRight = { lootBase.x + lootBase.w - 35, lootBase.y + 38, 29, 53 };
		lootBtn = { lootBase.x + lootBase.w / 2 - 38, lootBase.y + 80, 77, 30 };
		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - lootBase.w / 2;
			y = inputY - lootBase.h / 2;
		}
	}
	void drawGUI();
	void clickUpGUI();
	void clickMotionGUI(int dx, int dy);
	void clickDownGUI();
	void clickRightGUI();
	void clickHoldGUI() { }
	void mouseWheel()
	{
		panel.handleWheel(lootBase);
	}
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		tabType = tabFlag::back;

		lootBase.h = panel.calcWindowHeight();

		if (option::inputMethod == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000)
			{
				prt(L"탭이 실행되었다.\n");
				executeTab();
				delayR2 = 20;
			}
			else delayR2--;
		}

		if (lootStack != nullptr && panel.pocket->itemInfo.size() == 0)
		{
			destroyItemStack({ lootStack->getGridX(), lootStack->getGridY(), lootStack->getGridZ() });
			delete this;
			return;
		}

		panel.adjustScrollAndCursor();

		if (panel.pocket->itemInfo.size() == 0 && lootItemData == nullptr)
		{
			close(aniFlag::null);
			destroyItemStack({ lootTile.x, lootTile.y, PlayerZ() });
			return;
		}
	}

	//탭 키를 눌렀을 때의 연산
	void executeTab()
	{
		if (panel.cursor == -1) //아이템을 선택 중이지 않을 경우
		{
			close(aniFlag::winUnfoldClose);
		}
		else
		{

			if (option::inputMethod == input::gamepad) close(aniFlag::winUnfoldClose);
			else
			{
				panel.cursor = -1;
				barAct = actSet::null;
			}
		}
	}

	void executePickSelect()
	{
		//1. 포켓 커서가 가리키는 아이템의 Array의 잔여부피와 플레이어의 질량 한계를 참조
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		ItemPocket* bagPtr = nullptr;
		int bagIndex = -1;
		int bagNumber = 0;

		if (equipPtr->itemInfo.size() == 0)
		{
			updateLog(sysStr[123]);//소지한 가방이 하나도 없다
			return;
		}

		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			//가방일 경우
			if (equipPtr->itemInfo[i].pocketPtr != nullptr) { bagNumber++; }

			//커서가 가리키는 포켓의 주소를 저장
			if (bagNumber - 1 == pocketCursor)
			{
				bagPtr = equipPtr->itemInfo[i].pocketPtr.get();
				bagIndex = i;
				break;
			}

			//가방을 찾지 못했을 경우
			if (i == equipPtr->itemInfo.size() - 1 && bagPtr == nullptr)
			{
				updateLog(sysStr[123]);//소지한 가방이 하나도 없다
				return;
			}
		}

		//--------------------------------------------------------------------
		// 가방이 부피로 제한되는 경우 ----------------------------------------
		//--------------------------------------------------------------------
		if (equipPtr->itemInfo[bagIndex].pocketMaxVolume != 0)
		{
			int currentVol = equipPtr->itemInfo[bagIndex].pocketPtr.get()->getPocketVolume();
			int maxVol = equipPtr->itemInfo[bagIndex].pocketMaxVolume;
			int itemsVol = 0;
			for (int i = 0; i < panel.pocket->itemInfo.size(); i++)
			{
				if (panel.pocket->itemInfo[i].lootSelect > 0)
				{
					itemsVol += panel.pocket->itemInfo[i].lootSelect * panel.pocket->itemInfo[i].getVolume();
				}
			}

			if (maxVol < itemsVol + currentVol)
			{
				updateLog(sysStr[124]);
				return;
			}

			bool moved = false;
			for (int i = panel.pocket->itemInfo.size() - 1; i >= 0; i--)
			{
				if (panel.pocket->itemInfo[i].lootSelect > 0)
				{
					panel.pocket->transferItem(bagPtr, i, panel.pocket->itemInfo[i].lootSelect);
					moved = true;
				}
			}

			if (moved)updateLog(L"You put the items into the pocket.");

			panel.clearAllSelections();
			return;
		}
		//--------------------------------------------------------------------
		// 가방이 갯수로 제한되는 경우 ----------------------------------------
		//--------------------------------------------------------------------
		else
		{
			int currentNumber = equipPtr->itemInfo[bagIndex].pocketPtr.get()->getPocketNumber();
			int maxNumber = equipPtr->itemInfo[bagIndex].pocketMaxNumber;
			int itemNumber = 0;
			bool hasIllegal = false;

			for (int i = 0; i < panel.pocket->itemInfo.size(); i++)
			{
				if (panel.pocket->itemInfo[i].lootSelect > 0)
				{
					if (equipPtr->itemInfo[bagIndex].isPocketOnlyItem(panel.pocket->itemInfo[i].itemCode)) itemNumber += panel.pocket->itemInfo[i].lootSelect;
					else hasIllegal = true;
				}
			}

			//공간이 충분할 때
			if (maxNumber >= itemNumber + currentNumber)
			{
				bool moved = false;
				for (int i = panel.pocket->itemInfo.size() - 1; i >= 0; i--)
				{
					if (panel.pocket->itemInfo[i].lootSelect > 0 &&
						equipPtr->itemInfo[bagIndex].isPocketOnlyItem(panel.pocket->itemInfo[i].itemCode))
					{
						panel.pocket->transferItem(bagPtr, i, panel.pocket->itemInfo[i].lootSelect);
						moved = true;
					}
				}

				if (moved) updateLog(L"You put the items into the pocket.");
				if (hasIllegal) updateLog(L"Some selected items cannot be put into this pocket.");

				panel.clearAllSelections();
			}
			//공간이 부족하지만 약간은 남았을 때
			else
			{
				int remainingSpace = maxNumber - currentNumber;
				if (remainingSpace > 0)
				{
					bool moved = false;
					for (int i = panel.pocket->itemInfo.size() - 1; i >= 0 && remainingSpace > 0; i--)
					{
						if (panel.pocket->itemInfo[i].lootSelect > 0 &&
							equipPtr->itemInfo[bagIndex].isPocketOnlyItem(panel.pocket->itemInfo[i].itemCode))
						{
							int transferAmount = myMin(remainingSpace, panel.pocket->itemInfo[i].lootSelect);
							panel.pocket->transferItem(bagPtr, i, transferAmount);
							remainingSpace -= transferAmount;
							moved = true;
						}
						else if (panel.pocket->itemInfo[i].lootSelect > 0)
						{
							hasIllegal = true;
						}
					}

					panel.clearAllSelections();

					if (moved)updateLog(L"You put items into the pocket up to the limit.");
					if (hasIllegal)updateLog(L"Some selected items cannot be put into this pocket.");
				}
				//완전히 가득 찬 경우
				else
				{
					updateLog(L"The bag is full and cannot hold any more items.");
					if (hasIllegal) updateLog(L"Some selected items cannot be put into this pocket.");
				}
			}

		}
	}

	void executePocketLeft()
	{
		if (pocketCursor != 0) { pocketCursor--; }
	}
	void executePocketRight()
	{
		int numberOfBag = 0;
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				numberOfBag++;
			}
		}
		if (pocketCursor != numberOfBag - 1) { pocketCursor++; }
	}
	//act
	void executePick()
	{
		std::vector<int> pocketList;
		int numberOfBag = 0;
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				pocketList.push_back(i);
				numberOfBag++;
			}
		}
		if (numberOfBag == 0)
		{
			updateLog(sysStr[123]);
			return;
		}
		else
		{
			int itemNumber = panel.pocket->itemInfo[panel.cursor].number;
			panel.pocket->itemInfo[panel.cursor].lootSelect = itemNumber;
			panel.pocket->transferItem(equipPtr->itemInfo[pocketList[pocketCursor]].pocketPtr.get(), panel.cursor, panel.pocket->itemInfo[panel.cursor].lootSelect);
		}
	}

	void updateBarAct()
	{
		if (panel.pocket->itemInfo.size() > 0)
		{
			ItemData& targetItem = panel.pocket->itemInfo[panel.cursor];
			barAct.clear();
			barAct.push_back(act::wield);

			appendGunAmmoBarActs(targetItem);

			if (targetItem.checkFlag(itemFlag::CANEQUIP) == true) { barAct.push_back(act::equip); }

			if (targetItem.checkFlag(itemFlag::TOGGLE_ON)) barAct.push_back(act::toggleOff);
			else if (targetItem.checkFlag(itemFlag::TOGGLE_OFF)) barAct.push_back(act::toggleOn);

			if (targetItem.checkFlag(itemFlag::CAN_EAT))
			{
				barAct.push_back(act::eat);
			}

			if (targetItem.pocketMaxVolume > 0)
			{
				ItemPocket* pocketPtr = targetItem.pocketPtr.get();
				for (int i = 0; i < pocketPtr->itemInfo.size(); i++)
				{
					if (pocketPtr->itemInfo[i].checkFlag(itemFlag::CAN_DRINK))
					{
						barAct.push_back(act::drink);
						break;
					}
				}
			}

			if (targetItem.pocketMaxVolume > 0 && targetItem.pocketPtr.get()->itemInfo.size() > 0)
			{
				barAct.push_back(act::dump);
			}

			if (targetItem.checkFlag(itemFlag::POWERED_BY_BATTERY))
			{
				if (targetItem.pocketPtr->itemInfo.size() >= 1) barAct.push_back(act::removeBattery);
				else barAct.push_back(act::insertBattery);
			}

			if (targetItem.checkFlag(itemFlag::SEED_FRUIT))
			{
				barAct.push_back(act::extractSeed);
			}
		}
	}
};
