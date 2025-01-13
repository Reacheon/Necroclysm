#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

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
import Lst;
import ItemData;


export class Loot : public GUI
{
private:
	inline static Loot* ptr = nullptr;

	ItemStack* lootStack = nullptr; //만약 창고같이 스택이 없으면 null로 유지됨
	ItemData* lootItemData = nullptr;
	ItemPocket* lootPocket = nullptr;

	const int lootScrollSize = 6; //한 스크롤에 들어가는 아이템의 수
	int lootScroll = 0; //우측 루팅창의 스크롤
	int lootCursor = -1; //우측 루팅창의 커서
	int pocketCursor = 0; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	//모션 스크롤 이벤트에서 기준으로 잡는 최초 터치 당시의 스크롤
	int initLootScroll = 0; //모션스크롤이 시작되기 직전의 스크롤
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;
	sortFlag sortType = sortFlag::null; //0:기본_1:무게내림_2:무게올림_3:부피내림_4:부피올림

	std::wstring titleInventory = L"배낭";
	int titleItemSprIndex = 60;

	SDL_Rect lootBase;
	SDL_Rect lootTitle;
	SDL_Rect lootItemRect[30];
	SDL_Rect lootItemSelectRect[30];
	SDL_Rect lootLabel;
	SDL_Rect lootLabelSelect;
	SDL_Rect lootLabelName;
	SDL_Rect lootLabelQuantity;
	SDL_Rect lootArea;
	SDL_Rect lootWindow;
	SDL_Rect pocketWeight;
	SDL_Rect pocektVolume;
	SDL_Rect pocketWindow;
	SDL_Rect pocketItem[7];
	SDL_Rect pocketArea;
	SDL_Rect pocketLeft;
	SDL_Rect pocketRight;
	SDL_Rect lootBtn;
	SDL_Rect topWindow;
public:
	Corouter errorFunc();

	Loot(ItemPocket* inputPocket, ItemData* inputData) : GUI(false)
	{
		ptr = this;
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);
		//errorBox(ptr != nullptr, "More than one Loot instance was generated.");

		changeXY(cameraW - 335, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		tabType = tabFlag::closeWin;
		UIType = act::loot;

		lootPocket = inputPocket;
		lootItemData = inputData;
		if(lootPocket->itemInfo.size()>=2) lootPocket->sortByUnicode();

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (option::inputMethod == input::keyboard || option::inputMethod == input::gamepad) lootCursor = 0;
	}

	Loot(ItemStack* inputStack) : GUI(false)
	{
		ptr = this;
		prt(L"Loot : 생성자가 생성되었습니다..\n");
		prt(L"현재 loot의 ptr 변수는 %p입니다.\n", ptr);
		//errorBox(ptr != nullptr, "More than one Loot instance was generated.");

		changeXY(cameraW - 335, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		tabType = tabFlag::closeWin;
		UIType = act::loot;

		lootStack = inputStack;
		lootPocket = inputStack->getPocket();
		lootItemData = nullptr;
		if (lootPocket->itemInfo.size() >= 2) lootPocket->sortByUnicode();

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item의 크기는 %d입니다.\n", sizeof(ItemData));

		if (option::inputMethod == input::keyboard || option::inputMethod == input::gamepad) lootCursor = 0;
	}

	~Loot()
	{
		prt(L"Loot : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		lootCursor = -1;
		lootScroll = 0;
		for (int i = 0; i < lootPocket->itemInfo.size(); i++) { lootPocket->itemInfo[i].lootSelect = 0; }
		tabType = tabFlag::autoAtk;
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
		lootWindow = { lootBase.x + 0, lootBase.y + 120, 335, 300 };

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
		pocketWeight = { pocketWindow.x + 12, pocketWindow.y + 64, 72, 4 };
		pocektVolume = { pocketWindow.x + pocketWindow.w - 12 - 72, pocketWindow.h + 64, 72, 4 };


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

		topWindow = { 0, 0, 410,140 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

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
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		lootBase.h = 164 + 32 * myMax(0, (myMin(LOOT_ITEM_MAX - 1, lootPocket->itemInfo.size() - 1)));

		if (option::inputMethod == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 1000)
			{
				prt(L"탭이 실행되었다.\n");
				executeTab();
				delayR2 = 20;
			}
			else delayR2--;
		}

		if (lootStack != nullptr && lootPocket->itemInfo.size() == 0)
		{
			delete lootStack;
			delete this;
			return;
		}

		//셀렉트 홀드 이벤트
		if (coFunc == nullptr)
		{
			if (selectTouchTime != -1)
			{
				//아이템 좌측 셀렉트 클릭
				for (int i = 0; i < LOOT_ITEM_MAX; i++)
				{
					if (checkCursor(&lootItemSelectRect[i]))
					{
						if (lootPocket->itemInfo.size() - 1 >= i)
						{
							if (SDL_GetTicks() - selectTouchTime > 800)
							{
								selectTouchTime = -1;
								CORO(executeSelectItemEx(i + lootScroll));
							}
						}
						break;
					}

					if (i == LOOT_ITEM_MAX - 1)
					{
						selectTouchTime = -1;
					}
				}
			}
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
			delete World::ins()->getTile(lootTile.x, lootTile.y, Player::ins()->getGridZ()).ItemStackPtr;
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

			if (option::inputMethod == input::keyboard) close(aniFlag::winSlipClose);
			else if (option::inputMethod == input::gamepad) close(aniFlag::winSlipClose);
			else
			{
				lootCursor = -1;
				barAct = actSet::null;
				tabType = tabFlag::closeWin;
			}
		}
	}
	void executeSort()
	{
		switch (sortType)
		{
		default:
			errorBox("undefined sorting in Loot.ixx");
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
			lootPocket->sortVolumeDescend();
			sortType = sortFlag::volumeDescend;
			break;
		case sortFlag::volumeDescend:
			lootPocket->sortVolumeAscend();
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
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		ItemPocket* bagPtr = nullptr;
		int bagIndex = -1;
		int bagNumber = 0;

		if (equipPtr->itemInfo.size() == 0)
		{
			updateLog(col2Str(col::white) + sysStr[123]);
			return;
		}

		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			//가방일 경우
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0) { bagNumber++; }

			//커서가 가리키는 포켓의 주소를 저장
			if (bagNumber - 1 == pocketCursor)
			{
				bagPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				bagIndex = i;
				break;
			}

			//가방을 찾지 못했을 경우
			if (i == equipPtr->itemInfo.size() - 1 && bagPtr == nullptr)
			{
				updateLog(col2Str(col::white) + sysStr[123]);
				return;
			}
		}

		int currentVol = equipPtr->itemInfo[bagIndex].pocketVolume;
		int maxVol = equipPtr->itemInfo[bagIndex].pocketMaxVolume;

		int itemsVol = 0;
		for (int i = 0; i < lootPocket->itemInfo.size(); i++)
		{
			if (lootPocket->itemInfo[i].lootSelect > 0)
			{
				itemsVol += lootPocket->itemInfo[i].lootSelect * lootPocket->itemInfo[i].volume;
			}
		}

		if (maxVol < itemsVol + currentVol)
		{
			updateLog(col2Str(col::white) + sysStr[124]);
			return;
		}
		else
		{
			int fixedLootSize = lootPocket->itemInfo.size();
			for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPocket->itemInfo[i].lootSelect > 0)
				{
					lootPocket->transferItem(bagPtr, i, lootPocket->itemInfo[i].lootSelect);
				}
			}
			for (int i = lootPocket->itemInfo.size() - 1; i >= 0; i--) { lootPocket->itemInfo[i].lootSelect = 0; }

			updateLog(col2Str(col::white) + L"아이템을 가방에 집어 넣었다.");
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
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
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
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				pocketList.push_back(i);
				numberOfBag++;
			}
		}
		if (numberOfBag == 0)
		{
			updateLog(col2Str(col::white) + sysStr[123]);
			return;
		}
		else
		{
			//지금 가리키는 lootCursor의 select를 풀로 만듬
			int itemNumber = lootPocket->itemInfo[lootCursor].number;
			lootPocket->itemInfo[lootCursor].lootSelect = itemNumber;
			//아직 질량&부피 체크 추가하지 않았음
			lootPocket->transferItem((ItemPocket*)equipPtr->itemInfo[pocketList[pocketCursor]].pocketPtr, lootCursor, lootPocket->itemInfo[lootCursor].lootSelect);
			if (option::inputMethod == input::keyboard)
			{
				doPopDownHUD = true;
				barActCursor = -1;
			}
		}
	}
	void executeEquip()
	{
		updateLog(col2Str(col::white) + sysStr[125]);
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		int returnIndex = lootPocket->transferItem(Player::ins()->getEquipPtr(), lootCursor, 1);
		equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::normal;
		Player::ins()->updateStatus();
		Player::ins()->updateCustomSpriteHuman();
		if (option::inputMethod == input::keyboard)
		{
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}

	void updateBarAct()
	{
		if (lootPocket->itemInfo.size() > 0)
		{
			ItemData targetItem = lootPocket->itemInfo[lootCursor];
			barAct.clear();
			barAct.push_back(act::wield);

			//업데이트할 아이템이 총일 경우
			if (targetItem.checkFlag(itemFlag::GUN))
			{
				//전용 아이템이 탄창일 경우(일반 소총)
				if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);

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
					ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);
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
				barAct.push_back(act::reloadMagazine);

				//탄창 장전
				ItemPocket* magazinePtr = ((ItemPocket*)targetItem.pocketPtr);
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
				updateLog(col2Str(col::white) + sysStr[86]);//검색 상태를 해제했다.
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
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		int inputSelectNumber = wtoi(exInputText.c_str()); // 셀렉트 박스에 넣어질 숫자(플레이어가 입력한 값)
		if (inputSelectNumber > lootPocket->itemInfo[index].number) //만약 실제 있는 숫자보다 많은 값을 입력했을 경우
		{
			inputSelectNumber = lootPocket->itemInfo[index].number; //Select의 값을 최댓값으로 맞춤
		}
		lootPocket->itemInfo[index].lootSelect = inputSelectNumber;
	}

	Corouter executeWield()
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		if (lootPocket->itemInfo[lootCursor].checkFlag(itemFlag::TWOHANDED)) //양손장비일 경우
		{
			bool isWield = false;
			ItemPocket* drop = new ItemPocket(storageType::null);
			for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (equipPtr->itemInfo[i].equipState == equipHandFlag::left || equipPtr->itemInfo[i].equipState == equipHandFlag::right || equipPtr->itemInfo[i].equipState == equipHandFlag::both)
				{
					equipPtr->transferItem(drop, i, 1);
					isWield = true;
				}
			}
			if (isWield == true) { Player::ins()->drop(drop); }
			else { delete drop; }
			int returnIndex = lootPocket->transferItem(equipPtr, lootCursor, 1);
			equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::both; //양손
			equipPtr->sortEquip();
			updateLog(L"#FFFFFF아이템을 들었다.");
		}
		else
		{
			bool hasLeft = false;
			bool hasRight = false;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				switch (equipPtr->itemInfo[i].equipState)
				{
				case equipHandFlag::left:
					hasLeft = true;
					break;
				case equipHandFlag::right:
					hasRight = true;
					break;
				case equipHandFlag::both:
					hasLeft = true;
					hasRight = true;
					break;
				}
			}

			if (hasLeft == true && hasRight == true)
			{
				int fixedLootCursor = lootCursor;//Msg가 켜지면 lootCursor가 -1로 초기화되기에 미리 지역변수로 저장

				//왼손, 오른손
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//선택, 어느 손에 들까?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				equipHandFlag handDir = equipHandFlag::none;
				if (coAnswer == sysStr[49])//왼손
				{
					handDir = equipHandFlag::left;
				}
				else//오른손
				{
					handDir = equipHandFlag::right;
				}

				//왼손 아이템 떨구기
				ItemPocket* drop = new ItemPocket(storageType::null);
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == handDir)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				//양손 아이템 떨구기
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == equipHandFlag::both)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				Player::ins()->drop(drop);

				int returnIndex = lootPocket->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == false)
			{
				int fixedLootCursor = lootCursor;//Msg가 켜지면 lootCursor가 -1로 초기화되기에 미리 지역변수로 저장

				//왼손, 오른손
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//선택, 어느 손에 들까?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				equipHandFlag handDir = equipHandFlag::none;
				if (coAnswer == sysStr[49])//왼손
				{
					handDir = equipHandFlag::left;
				}
				else//오른손
				{
					handDir = equipHandFlag::right;
				}

				int returnIndex = lootPocket->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == true)//왼손에 들기
			{
				int returnIndex = lootPocket->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::left;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
			else//오른손에 들기
			{
				int returnIndex = lootPocket->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::right;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
		}
		Player::ins()->updateStatus();
		Player::ins()->updateCustomSpriteHuman();
		if (option::inputMethod == input::keyboard)
		{
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}

	Corouter executeInsert()//삽탄 : 총알에 사용, 이 탄환을 넣을 수 있는 탄창 리스트를 표시하고 거기에 넣음
	{
		int targetLootCursor = lootCursor;
		std::vector<std::wstring> pocketList;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0 || equipPtr->itemInfo[i].pocketMaxNumber > 0)
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
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], pocketList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0 || equipPtr->itemInfo[i].pocketMaxNumber > 0)
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
						(ItemPocket*)equipPtr->itemInfo[i].pocketPtr,
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
