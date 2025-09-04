#include <SDL3/SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Loot;

import std;
import util;
import globalVar;
import wrapVar;
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
import Lst;
import ItemData;


export class Loot : public GUI
{
private:
	inline static Loot* ptr = nullptr;

	ItemStack* lootStack = nullptr; //만약 창고같이 스택이 없으면 null로 유지됨
	ItemData* lootItemData = nullptr;
	ItemPocket* lootPocket = nullptr;

	int lootScroll = 0; //우측 루팅창의 스크롤
	int lootCursor = -1; //우측 루팅창의 커서
	int pocketCursor = 0; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	//모션 스크롤 이벤트에서 기준으로 잡는 최초 터치 당시의 스크롤
	int initLootScroll = 0; //모션스크롤이 시작되기 직전의 스크롤
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;
	sortFlag sortType = sortFlag::null; //0:기본_1:무게내림_2:무게올림_3:부피내림_4:부피올림

	std::wstring titleInventory;

	SDL_Rect lootBase;
	SDL_Rect lootTitle;
	SDL_Rect lootItemRect[30];
	SDL_Rect lootItemSelectRect[30];
	SDL_Rect lootLabel;
	SDL_Rect lootLabelSelect;
	SDL_Rect lootLabelName;
	SDL_Rect lootLabelQuantity;
	SDL_Rect lootArea;
	SDL_Rect pocketWindow;
	SDL_Rect pocketItem[7];
	SDL_Rect pocketLeft;
	SDL_Rect pocketRight;
	SDL_Rect lootBtn;
public:
	Corouter errorFunc();

	Loot(ItemPocket* inputPocket, ItemData* inputData) : GUI(false)
	{
		ptr = this;
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);
		//errorBox(ptr != nullptr, L"More than one Loot instance was generated.");

		changeXY(cameraW - 335, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		UIType = act::loot;

		lootPocket = inputPocket;
		lootItemData = inputData;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (option::inputMethod == input::gamepad) lootCursor = 0;
	}

	Loot(ItemStack* inputStack) : GUI(false)
	{
		ptr = this;
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);
		//errorBox(ptr != nullptr, L"More than one Loot instance was generated.");

		changeXY(cameraW - 335, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		UIType = act::loot;

		lootStack = inputStack;
		lootPocket = inputStack->getPocket();
		lootItemData = nullptr;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (option::inputMethod == input::gamepad) lootCursor = 0;
	}

	~Loot()
	{
		prt(L"Loot : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		lootCursor = -1;
		lootScroll = 0;
		for (int i = 0; i < lootPocket->itemInfo.size(); i++) { lootPocket->itemInfo[i].lootSelect = 0; }
		barAct = actSet::null;
		barActCursor = -1;
	}
	static Loot* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		lootBase = { 0,0,335,420 };
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
		lootTitle = { lootBase.x + 102, lootBase.y + 0, 130, 30 };

		lootArea = { lootBase.x + 10, lootBase.y + 125,312, 246 };
		for (int i = 0; i < LOOT_ITEM_MAX; i++)
		{
			lootItemRect[i] = { lootBase.x + 52, lootBase.y + 125 + 32*i, 270, 26 };
			lootItemSelectRect[i] = { lootBase.x + 10, lootBase.y + 125 + 32*i, 36, 26 };
		}
		lootLabel = { lootBase.x + 10, lootBase.y + 95, lootBase.w - 20 , 26 };
		lootLabelSelect = { lootLabel.x, lootLabel.y, 62 , 26 };
		lootLabelName = { lootLabel.x + lootLabelSelect.w, lootLabel.y, 182 , 26 };
		lootLabelQuantity = { lootLabel.x + lootLabelName.w + lootLabelSelect.w, lootLabel.y, 71 , 26 };

		pocketWindow = { lootBase.x + 0, lootBase.y + 34, 335, 70 };

		//pocketItem[0] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38 * 3,pocketWindow.y + 11,32,32 };
		//pocketItem[1] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38 * 2,pocketWindow.y + 11,32,32 };
		//pocketItem[2] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38,pocketWindow.y + 11,32,32 };
		//pocketItem[3] = { pocketWindow.x + (pocketWindow.w / 2) - 24,pocketWindow.y + 11 - 8,48,48 };
		//pocketItem[4] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38,pocketWindow.y + 11,32,32 };
		//pocketItem[5] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38 * 2,pocketWindow.y + 11,32,32 };
		//pocketItem[6] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38 * 3,pocketWindow.y + 11,32,32 };

		pocketLeft = { lootBase.x + 6,lootBase.y + 32, 24,44 };
		pocketRight = { lootBase.x + lootBase.w - 29,lootBase.y + 32,24,44 };

		lootBtn = { lootBase.x + lootBase.w / 2 - 32, lootBase.y + 67, 64, 25 };

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
	void clickRightGUI() 
	{

		//아이템 좌측 셀렉트 우클릭
		for (int i = 0; i < LOOT_ITEM_MAX; i++)
		{
			if (checkCursor(&lootItemSelectRect[i]))
			{
				if (lootPocket->itemInfo.size() - 1 >= i)
				{
					if (lootPocket->itemInfo[i + lootScroll].lootSelect == 0)
					{
						CORO(executeSelectItemEx(i + lootScroll));
					}
					else
					{
						lootPocket->itemInfo[i + lootScroll].lootSelect = 0;
					}
				}
			}
		}
	}
	void clickHoldGUI() { }
	void mouseWheel() 
	{
		if (checkCursor(&lootBase))
		{
			if (event.wheel.y > 0 && lootScroll > 0) lootScroll -= 1;
			else if (event.wheel.y < 0 && lootScroll + LOOT_ITEM_MAX < lootPocket->itemInfo.size()) lootScroll += 1;
		}
	}
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		tabType = tabFlag::back;

		lootBase.h = 164 + 32 * myMax(0, (myMin(LOOT_ITEM_MAX - 1, lootPocket->itemInfo.size() - 1)));

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

		if (lootStack != nullptr && lootPocket->itemInfo.size() == 0)
		{
			destroyItemStack({ lootStack->getGridX(), lootStack->getGridY(), lootStack->getGridZ()});
			delete this;
			return;
		}


		//잘못된 커서 위치 조정
		if (lootCursor > (int)(lootPocket->itemInfo.size() - 1)) 
		{ 
			lootCursor = lootPocket->itemInfo.size() - 1; 
		}

		//잘못된 스크롤 위치 조정
		if (option::inputMethod == input::mouse || option::inputMethod == input::touch)
		{
			if (lootScroll + LOOT_ITEM_MAX >= lootPocket->itemInfo.size()) { lootScroll = myMax(0, (int)lootPocket->itemInfo.size() - LOOT_ITEM_MAX); }
			else if (lootScroll < 0) { lootScroll = 0; }
		}

		//루트 아이템의 갯수가 0이 되었을 경우 창을 닫음
		if (lootPocket->itemInfo.size() == 0 && lootItemData == nullptr)
		{
			close(aniFlag::null);
			//클로즈 후의 애니메이션이 문제가 된다. 애니메이션이 모두 실행되고 제거해야됨
			delete TileItemStack(lootTile.x, lootTile.y, PlayerZ());
			return;
		}
	}

	sortFlag getSortType() { return sortType; }
	//탭 키를 눌렀을 때의 연산
	void executeTab()
	{
		if (lootCursor == -1) //아이템을 선택 중이지 않을 경우
		{
			close(aniFlag::winSlipClose);
		}
		else
		{

			if (option::inputMethod == input::gamepad) close(aniFlag::winSlipClose);
			else
			{
				lootCursor = -1;
				barAct = actSet::null;
			}
		}
	}
	void executeSort()
	{
		switch (sortType)
		{
		default:
			errorBox(L"undefined sorting in Loot.ixx");
			break;
		case sortFlag::null:
			lootPocket->sortWeightDescend();
			sortType = sortFlag::weightDescend;
			break;
		case sortFlag::weightDescend:
			lootPocket->sortWeightAscend();
			sortType = sortFlag::weightAscend;
			break;
		case sortFlag::weightAscend:
			sortVolumeDescend(lootPocket->itemInfo);
			sortType = sortFlag::volumeDescend;
			break;
		case sortFlag::volumeDescend:
			sortVolumeAscend(lootPocket->itemInfo);
			sortType = sortFlag::volumeAscend;
			break;
		case sortFlag::volumeAscend:
			lootPocket->sortByUnicode();
			sortType = sortFlag::null;
			break;
		}
		lootScroll = 0;
	}
	void executePickSelect()
	{
		//나중에 가독성 및 로직 수정할것

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
			for (int i = 0; i < lootPocket->itemInfo.size(); i++)
			{
				if (lootPocket->itemInfo[i].lootSelect > 0)
				{
					itemsVol += lootPocket->itemInfo[i].lootSelect * getVolume(lootPocket->itemInfo[i]);
				}
			}

			if (maxVol < itemsVol + currentVol)
			{
				updateLog(sysStr[124]);
				return;
			}

			bool moved = false; //실제로 옮긴 아이템이 있는지 체크
			for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPocket->itemInfo[i].lootSelect > 0)
				{
					lootPocket->transferItem(bagPtr, i, lootPocket->itemInfo[i].lootSelect);
					moved = true;
				}
			}

			if (moved)updateLog(L"You put the items into the pocket.");

			for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--) { lootPocket->itemInfo[i].lootSelect = 0; }
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
			bool hasIllegal = false; // ★ 선택된 아이템 가운데 가방이 허용하지 않는 것이 있는가?

			for (int i = 0; i < lootPocket->itemInfo.size(); i++)
			{
				if (lootPocket->itemInfo[i].lootSelect > 0)
				{
					if (equipPtr->itemInfo[bagIndex].isPocketOnlyItem(lootPocket->itemInfo[i].itemCode)) itemNumber += lootPocket->itemInfo[i].lootSelect;
					else hasIllegal = true; //넣을 수 없는 아이템을 발견
				}
			}

			//공간이 충분할 때 --------------------------------------------------
			if (maxNumber >= itemNumber + currentNumber)
			{
				bool moved = false;
				for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--)
				{
					if (lootPocket->itemInfo[i].lootSelect > 0 &&
						equipPtr->itemInfo[bagIndex].isPocketOnlyItem(lootPocket->itemInfo[i].itemCode))
					{
						lootPocket->transferItem(bagPtr, i, lootPocket->itemInfo[i].lootSelect);
						moved = true;
					}
				}

				if (moved) updateLog(L"You put the items into the pocket.");
				if (hasIllegal) updateLog(L"Some selected items cannot be put into this pocket.");

				for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--) { lootPocket->itemInfo[i].lootSelect = 0; }
				
			}
			//공간이 부족하지만 약간은 남았을 때 ------------------------------
			else
			{
				int remainingSpace = maxNumber - currentNumber;
				if (remainingSpace > 0)
				{
					bool moved = false;
					for (int i = lootPocket->itemInfo.size() - 1; i >= 0 && remainingSpace > 0; i--)
					{
						if (lootPocket->itemInfo[i].lootSelect > 0 &&
							equipPtr->itemInfo[bagIndex].isPocketOnlyItem(lootPocket->itemInfo[i].itemCode))
						{
							int transferAmount = myMin(remainingSpace, lootPocket->itemInfo[i].lootSelect);
							lootPocket->transferItem(bagPtr, i, transferAmount);
							remainingSpace -= transferAmount;
							moved = true;
						}
						else if (lootPocket->itemInfo[i].lootSelect > 0)
						{
							hasIllegal = true;
						}
					}

					for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--) lootPocket->itemInfo[i].lootSelect = 0;

					if (moved)updateLog(L"You put items into the pocket up to the limit.");
					if (hasIllegal)updateLog(L"Some selected items cannot be put into this pocket.");
				}
				//완전히 가득 찬 경우 ----------------------------------------
				else
				{
					updateLog(L"The bag is full and cannot hold any more items.");
					if (hasIllegal) updateLog(L"Some selected items cannot be put into this pocket.");
				}
			}
			updateQuiverSpr(PlayerPtr->getEquipPtr());

		}
	}


	void executeSelectAll()
	{
		bool isSelectAll = true;
		for (int i = 0; i < lootPocket->itemInfo.size(); i++)
		{
			if (lootPocket->itemInfo[i].lootSelect != lootPocket->itemInfo[i].number)
			{
				isSelectAll = false;
				break;
			}
		}

		if (isSelectAll == false)
		{
			for (int i = 0; i < lootPocket->itemInfo.size(); i++)
			{
				lootPocket->itemInfo[i].lootSelect = lootPocket->itemInfo[i].number;
			}
		}
		else
		{
			for (int i = 0; i < lootPocket->itemInfo.size(); i++)
			{
				lootPocket->itemInfo[i].lootSelect = 0;
			}
		}
	}
	void executeSelectItem(int index)
	{
		int itemNumber = lootPocket->itemInfo[index].number;
		lootPocket->itemInfo[index].lootSelect = itemNumber;
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
		//pick 구현하기 전에 select 기능 먼저 구현해야됨
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
			//지금 가리키는 lootCursor의 select를 풀로 만듬
			int itemNumber = lootPocket->itemInfo[lootCursor].number;
			lootPocket->itemInfo[lootCursor].lootSelect = itemNumber;
			//아직 질량&부피 체크 추가하지 않았음
			lootPocket->transferItem(equipPtr->itemInfo[pocketList[pocketCursor]].pocketPtr.get(), lootCursor, lootPocket->itemInfo[lootCursor].lootSelect);
		}
	}
	void executeEquip()
	{
        
		updateLog(replaceStr(sysStr[125], L"(%item)", lootPocket->itemInfo[lootCursor].name));//(%item)를(을) 장착했다.
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		int returnIndex = lootPocket->transferItem(PlayerPtr->getEquipPtr(), lootCursor, 1);
		equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::normal;
		PlayerPtr->pullEquipLights();
		PlayerPtr->updateStatus();
	}

	void updateBarAct()
	{
		if (lootPocket->itemInfo.size() > 0)
		{
			ItemData& targetItem = lootPocket->itemInfo[lootCursor];
			barAct.clear();
			barAct.push_back(act::wield);

			//업데이트할 아이템이 총일 경우
			if (targetItem.checkFlag(itemFlag::GUN))
			{
				//전용 아이템이 탄창일 경우(일반 소총)
				if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					ItemPocket* gunPtr = targetItem.pocketPtr.get();

					if (gunPtr->itemInfo.size() == 0)
					{
						barAct.push_back(act::reloadMagazine);
					}
					else
					{
						barAct.push_back(act::unloadMagazine);
					}
				}
				//전용 아이템이 탄일 경우(리볼버류)
				else if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
				{
					ItemPocket* gunPtr = targetItem.pocketPtr.get();
					//탄환 분리
					if (gunPtr->itemInfo.size() > 0)
					{
						barAct.push_back(act::unloadBulletFromGun);
					}

					//탄환 장전
					int bulletNumber = 0;
					for (int i = 0; i < gunPtr->itemInfo.size(); i++)
					{
						bulletNumber += gunPtr->itemInfo[i].number;
					}

					if (bulletNumber < targetItem.pocketMaxNumber)
					{
						barAct.push_back(act::reloadBulletToGun);
					}
				}
			}
			//업데이트할 아이템이 탄창일 경우
			else if (targetItem.checkFlag(itemFlag::MAGAZINE))
			{
				if(targetItem.itemCode != itemRefCode::arrowQuiver && targetItem.itemCode != itemRefCode::boltQuiver) barAct.push_back(act::reloadMagazine);

				//탄창 장전
				ItemPocket* magazinePtr = targetItem.pocketPtr.get();
				if (magazinePtr->itemInfo.size() > 0)
				{
					barAct.push_back(act::unloadBulletFromMagazine);
				}

				//총알 장전
				int bulletNumber = 0;
				for (int i = 0; i < magazinePtr->itemInfo.size(); i++)
				{
					bulletNumber += magazinePtr->itemInfo[i].number;
				}

				if (bulletNumber < targetItem.pocketMaxNumber)
				{
					barAct.push_back(act::reloadBulletToMagazine);
				}
			}
			//업데이트할 아이템이 탄환일 경우
			else if (targetItem.checkFlag(itemFlag::AMMO))
			{
				barAct.push_back(act::reloadBulletToGun);
			}

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
				// 포켓 내부에 CAN_DRINK 플래그가 있는 아이템이 있는지 확인
				for (int i = 0; i < pocketPtr->itemInfo.size(); i++)
				{
					if (pocketPtr->itemInfo[i].checkFlag(itemFlag::CAN_DRINK))
					{
						barAct.push_back(act::drink);
						break; // 하나라도 찾으면 중단
					}
				}
			}

			//쏟기 추가
			if (targetItem.pocketMaxVolume > 0 && targetItem.pocketPtr.get()->itemInfo.size() > 0)
			{
				barAct.push_back(act::dump);
			}
		}
	}

	//▼코루틴 함수▼
	Corouter executeSearch()
	{
		//이미 검색 중인지 체크
		for (int i = 0; i < lootPocket->itemInfo.size(); i++)
		{
			if (lootPocket->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
			{
				for (int j = 0; j < lootPocket->itemInfo.size(); j++)
				{
					lootPocket->itemInfo[j].eraseFlag(itemFlag::GRAYFILTER);
				}
				lootPocket->sortByUnicode();
				updateLog(sysStr[86]);//검색 상태를 해제했다.
				co_return;
			}

			if (i == lootPocket->itemInfo.size() - 1)//검색 중이 아닐 경우
			{
				std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
				new Msg(msgFlag::input, sysStr[27], sysStr[97], choiceVec);//검색, 검색할 키워드를 입력해주세요
				lootScroll = 0;
				co_await std::suspend_always();
				if (coAnswer == sysStr[38])
				{
					int matchCount = lootPocket->searchTxt(exInputText);
					for (int i = 0; i < lootPocket->itemInfo.size(); i++) lootPocket->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) lootPocket->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}

	Corouter executeSelectItemEx(int index)
	{
		//입력형 메시지 박스 열기
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
		exInputText.clear();
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		if (exInputText.empty() == false)
		{
			int inputSelectNumber = 0;
			try {inputSelectNumber = wtoi(exInputText.c_str());}
			catch (...) {  }


			if (inputSelectNumber < 0) inputSelectNumber = 0;
			else if (inputSelectNumber > lootPocket->itemInfo[index].number) inputSelectNumber = lootPocket->itemInfo[index].number;

			lootPocket->itemInfo[index].lootSelect = inputSelectNumber;
		}
	}



	Corouter executeInsert()//삽탄 : 총알에 사용, 이 탄환을 넣을 수 있는 탄창 리스트를 표시하고 거기에 넣음
	{
		int targetLootCursor = lootCursor;
		std::vector<std::wstring> pocketList;
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				if (equipPtr->itemInfo[i].pocketMaxNumber > 0) //전용 아이템이 있을 경우
				{
					if (std::find(equipPtr->itemInfo[i].pocketOnlyItem.begin(), equipPtr->itemInfo[i].pocketOnlyItem.end(), lootPocket->itemInfo[targetLootCursor].itemCode) == equipPtr->itemInfo[i].pocketOnlyItem.end())
					{
						//루트커서의 해당 아이템을 넣을 수 없는 포켓이면 continue
						continue;
					}
				}

				pocketList.push_back(equipPtr->itemInfo[i].name);
			}
		}

		if (pocketList.size() == 0) //넣을만한 포켓을 찾지 못했을 경우
		{
			//이 아이템을 넣을만한 포켓이 없다.
			updateLog(sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], pocketList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();
		if (coAnswer.empty()) co_return;

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				if (equipPtr->itemInfo[i].pocketMaxNumber > 0) //전용 아이템이 있을 경우
				{
					if (std::find(equipPtr->itemInfo[i].pocketOnlyItem.begin(), equipPtr->itemInfo[i].pocketOnlyItem.end(), lootPocket->itemInfo[targetLootCursor].itemCode) == equipPtr->itemInfo[i].pocketOnlyItem.end())
					{
						//루트커서의 해당 아이템을 넣을 수 없는 포켓이면 continue
						continue;
					}
				}

				if (counter == wtoi(coAnswer.c_str()))
				{

					int transferNumber;
					if (lootPocket->itemInfo[targetLootCursor].lootSelect != 0)
					{
						transferNumber = lootPocket->itemInfo[targetLootCursor].lootSelect;
						lootPocket->itemInfo[targetLootCursor].lootSelect = 0;
					}
					else { transferNumber = lootPocket->itemInfo[targetLootCursor].number; }

					//몇 개 넣을까?
					lootPocket->transferItem
					(
						equipPtr->itemInfo[i].pocketPtr.get(),
						targetLootCursor,
						transferNumber
					);

					co_return;
				}

				counter++;
			}
		}
	}
};
