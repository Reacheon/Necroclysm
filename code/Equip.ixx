#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();


export module Equip;

import std;
import util;
import Player;
import drawText;
import globalVar;
import wrapVar;
import textureVar;
import ItemPocket;
import checkCursor;
import drawSprite;
import drawItemList;
import drawWindowArrow;
import GUI;
import log;
import Msg;
import actFuncSet;
import drawWindow;
import CoordSelect;
import Lst;
import Inventory;
import ItemData;

export class Equip : public GUI
{
private:
	inline static Equip* ptr = nullptr;
	ItemPocket* equipPtr = PlayerPtr->getEquipPtr();

	int equipScroll = 0; //좌측 장비창의 스크롤
	int equipCursor = -1; //좌측 장비창의 커서
	const int equipScrollSize = 8;

	ItemPocket* lootPtr = nullptr;
	int lootScroll = 0; //우측 루팅창의 스크롤
	int lootCursor = -1; //우측 루팅창의 커서
	int pocketCursor = -1; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	//모션 스크롤 이벤트에서 기준으로 잡는 최초 터치 당시의 스크롤
	int initLootScroll = 0;
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;

	sortFlag sortType = sortFlag::null; //0:기본_1:무게내림_2:무게올림_3:부피내림_4:부피올림

	bool isTargetPocket = false; //우측의 포켓창이 조작 타겟일 경우

	SDL_Rect equipBase;
	SDL_Rect equipTitle;
	SDL_Rect equipItemRect[30];
	SDL_Rect equipItemSelectRect[30];
	SDL_Rect equipLabel;
	SDL_Rect equipLabelSelect;
	SDL_Rect equipLabelName;
	SDL_Rect equipLabelQuantity;
	SDL_Rect equipArea;
	SDL_Rect equipWindow;

	SDL_Rect lootBase;
	SDL_Rect lootTitle;
	SDL_Rect lootItem[30];
	SDL_Rect lootItemSelect[30];
	SDL_Rect lootLabel;
	SDL_Rect lootLabelSelect;
	SDL_Rect lootLabelName;
	SDL_Rect lootLabelQuantity;
	SDL_Rect lootArea;
	SDL_Rect lootScrollBox;
	SDL_Rect lootWindow;
	SDL_Rect pocketWindow;
	SDL_Rect pocketItem[7];
	SDL_Rect pocketScrollBox;
	SDL_Rect pocketLeft;
	SDL_Rect pocketRight;
	SDL_Rect lootBtn;

	SDL_Rect topWindow;//상단에 표시되는 저항이나 방어 상성, 아이템의 설명


public:
	Equip() : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than equip instance was generated.");
		ptr = this;

		changeXY(0, (cameraH / 2) - 210, false);
		setAniSlipDir(4);

		//barAct = actSet::null;
		tabType = tabFlag::closeWin;
		UIType = act::equip;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);
	}
	~Equip()
	{
		prt(L"Equip : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		equipCursor = -1;
		equipScroll = 0;
		barAct = actSet::null;
		tabType = tabFlag::autoAtk;
	}
	static Equip* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		equipBase = { 0,0,335,420 };
		if (center == false)
		{
			equipBase.x += inputX;
			equipBase.y += inputY;
		}
		else
		{
			equipBase.x += inputX - equipBase.w / 2;
			equipBase.y += inputY - equipBase.h / 2;
		}

		equipTitle = { equipBase.x + 103, equipBase.y, 130, 30 };
		equipWindow = { equipBase.x, equipBase.y + 30, 335, 380 };

		equipLabel = { equipWindow.x + 10, equipWindow.y + 26, equipWindow.w - 20 , 26 };
		equipLabelSelect = { equipLabel.x, equipLabel.y, 62 , 26 };
		equipLabelName = { equipLabel.x + equipLabelSelect.w, equipLabel.y, 182 , 26 };
		equipLabelQuantity = { equipLabel.x + equipLabelName.w + equipLabelSelect.w, equipLabel.y, 71 , 26 };

		equipArea = { equipWindow.x + 10, equipWindow.y + 56,312, 42 * 8 - 6 };
		for (int i = 0; i < EQUIP_ITEM_MAX; i++)
		{
			equipItemRect[i] = { equipArea.x + 42, equipArea.y + 32 * i, 270, 26 };
			equipItemSelectRect[i] = { equipArea.x, equipArea.y + 32 * i, 36, 26 };
		}


		topWindow = { 0, 0, 410,140 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - equipBase.w / 2;
			y = inputY - equipBase.h / 2;
		}
	}
	void drawGUI();
	void clickUpGUI();
	void clickMotionGUI(int dx, int dy);
	void clickDownGUI();
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() 
	{
		if (checkCursor(&equipBase))
		{
			if (event.wheel.y > 0)
			{
				if(equipScroll>0) equipScroll -= 1;
			}
			else if (event.wheel.y < 0)
			{
				if(equipScroll + EQUIP_ITEM_MAX < equipPtr->itemInfo.size()) equipScroll += 1;
			}
		}
	}
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		//셀렉트 홀드 이벤트
		if (coFunc == nullptr)
		{
			if (selectTouchTime != -1)
			{
				//아이템 좌측 셀렉트 클릭
				for (int i = 0; i < LOOT_ITEM_MAX; i++)
				{
					if (checkCursor(&lootItemSelect[i]))
					{
						if (lootPtr->itemInfo.size() - 1 >= i)
						{
							if (SDL_GetTicks() - selectTouchTime > 800)
							{
								selectTouchTime = -1;
								CORO(executeSelectItemEx(pocketCursor, i + lootScroll));
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
	}
	sortFlag getSortType() { return sortType; }

	void executeTab()
	{
		if (pocketCursor == -1)
		{
			if (equipCursor == -1) //아이템을 선택 중이지 않을 경우
			{
				close(aniFlag::winSlipClose);
			}
			else
			{
				//select 아이템이 하나라도 있을 경우 전부 제거
				equipScroll = 0;
				equipCursor = -1;
				for (int i = 0; i < equipPtr->itemInfo.size(); i++) { equipPtr->itemInfo[i].lootSelect = 0; }
				barAct = actSet::null;
				tabType = tabFlag::closeWin;
			}
		}
		else//가방 닫기
		{
			isTargetPocket = false;
			labelCursor = -1;
			pocketCursor = -1;
		}
	}
	//탭 키를 눌렀을 때의 연산
	void executeSort()
	{
		switch (sortType)
		{
			default:
				errorBox(L"undefined sorting in Equip.ixx");
				break;
			case sortFlag::null:
				lootPtr->sortWeightDescend();;
				break;
			case sortFlag::weightDescend:
				lootPtr->sortWeightDescend();
				break;
			case sortFlag::weightAscend:
				lootPtr->sortWeightAscend();
				break;
			case sortFlag::volumeDescend:
				lootPtr->sortVolumeDescend();
				break;
			case sortFlag::volumeAscend:
				lootPtr->sortVolumeAscend();
				break;
		}
		lootScroll = 0;
	}

	void executePickDrop()
	{
		if (lootPtr->itemInfo.size() > 0)
		{
			//선택된 아이템이 하나도 없으면 종료
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0) { break; }

				if (i == 0) { return; }
			}

			std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0)
				{
					lootPtr->transferItem(drop.get(), i, lootPtr->itemInfo[i].lootSelect);
				}
			}
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--) { lootPtr->itemInfo[i].lootSelect = 0; }
			PlayerPtr->drop(drop.get());
			PlayerPtr->updateStatus();
			updateLog(col2Str(col::white) + sysStr[126]);//아이템을 버렸다.
		}
	}
	void executeSelectAll()
	{
		bool isSelectAll = true;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].lootSelect != lootPtr->itemInfo[i].number)
			{
				isSelectAll = false;
				break;
			}
		}

		if (isSelectAll == false)
		{
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				lootPtr->itemInfo[i].lootSelect = lootPtr->itemInfo[i].number;
			}
		}
		else
		{
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				lootPtr->itemInfo[i].lootSelect = 0;
			}
		}
	}
	void executeSelectItem(int index)
	{
		int itemNumber = lootPtr->itemInfo[index].number;
		lootPtr->itemInfo[index].lootSelect = itemNumber;
	}

	void executePocketLeft()
	{
		if (pocketCursor != 0)
		{
			pocketCursor--;

			lootCursor = 0;
			lootScroll = 0;
			labelCursor = -1;

			//포켓커서 변경에 따른 pocketPtr 변경
			int pocketStack = 0;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					if (pocketCursor == pocketStack)
					{
						lootPtr = equipPtr->itemInfo[i].pocketPtr.get();
						break;
					}
					else
					{
						pocketStack++;
					}
				}
			}
		}


	}
	void executePocketRight()
	{
		int numberOfBag = 0;
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				numberOfBag++;
			}
		}
		if (pocketCursor != numberOfBag - 1)
		{
			pocketCursor++;

			lootCursor = 0;
			lootScroll = 0;
			labelCursor = -1;

			//포켓커서 변경에 따른 pocketPtr 변경
			int pocketStack = 0;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					if (pocketCursor == pocketStack)
					{
						lootPtr = equipPtr->itemInfo[i].pocketPtr.get();
						break;
					}
					else
					{
						pocketStack++;
					}
				}
			}
		}


	}
	//act
	void executeEquip()
	{
		updateLog(col2Str(col::white) + sysStr[125]);//아이템을 장착했다.
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		int returnIndex = lootPtr->transferItem(PlayerPtr->getEquipPtr(), lootCursor, 1);
		equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::normal;
		PlayerPtr->updateStatus();
		PlayerPtr->updateCustomSpriteHuman();
	}

	void executeDroping()
	{
		std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
		if (isTargetPocket == false)
		{
			equipPtr->transferItem(drop.get(), equipCursor, 1);
		}
		else
		{
			lootPtr->transferItem(drop.get(), lootCursor, 1);
		}
		PlayerPtr->drop(drop.get());
		PlayerPtr->updateStatus();
		PlayerPtr->updateCustomSpriteHuman();
		updateLog(col2Str(col::white) + sysStr[126]);
	}
	void executeOpen()
	{
		new Inventory(334, (cameraH / 2) - 210, &equipPtr->itemInfo[equipCursor]);
	}
	void updateBarAct()
	{
		if (equipCursor != -1)
		{
			if (equipPtr->itemInfo.size() > 0)
			{
				ItemData& targetItem = equipPtr->itemInfo[equipCursor];
				barAct.clear();
				if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
				if (targetItem.pocketOnlyItem.size() > 0) { barAct.push_back(act::reload); }//전용 아이템 있을 경우 reload 추가
				barAct.push_back(act::droping);//droping은 항상 추가
				barAct.push_back(act::throwing);//throwing도 항상 추가

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
					barAct.push_back(act::reloadMagazine);

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

				if (targetItem.checkFlag(itemFlag::TOGGLE_ON)) barAct.push_back(act::toggleOff);
				else if (targetItem.checkFlag(itemFlag::TOGGLE_OFF)) barAct.push_back(act::toggleOn);
			}
		}
		else if (lootCursor != -1)
		{
			if (lootPtr->itemInfo.size() > 0)
			{
				ItemData& targetItem = lootPtr->itemInfo[lootCursor];
				barAct.clear();
				barAct.push_back(act::wield);
				if (targetItem.checkFlag(itemFlag::CANEQUIP) == true) { barAct.push_back(act::equip); }
				barAct.push_back(act::throwing);//throwing도 항상 추가
			}
		}
		else
		{
			errorBox(L"Equip instance : updateBarAct error occurs, Both lootCursor and equipCursor have the same value -1");
		}
	}

	Corouter executeSearch()
	{
		//이미 검색 중인지 체크
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
			{
				for (int j = 0; j < lootPtr->itemInfo.size(); j++)
				{
					lootPtr->itemInfo[j].eraseFlag(itemFlag::GRAYFILTER);
				}
				lootPtr->sortByUnicode();
				updateLog(col2Str(col::white) + sysStr[86]);//검색 상태를 해제했다.
				co_return;
			}

			if (i == lootPtr->itemInfo.size() - 1)//검색 중이 아닐 경우
			{
				std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
				new Msg(msgFlag::input, sysStr[27], sysStr[97], choiceVec);//검색, 검색할 키워드를 입력해주세요
				lootScroll = 0;
				co_await std::suspend_always();
				if (coAnswer == sysStr[38]) //검색 후 확인 버튼
				{
					int matchCount = lootPtr->searchTxt(exInputText);
					for (int i = 0; i < lootPtr->itemInfo.size(); i++) lootPtr->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) lootPtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}

	Corouter executeSelectItemEx(int pocketCursor, int lootCursor)
	{
		//입력형 메시지 박스 열기
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		//포켓커서에 따른 pocketPtr 변경
		ItemPocket* lootPtr = nullptr;
		int pocketStack = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				if (pocketCursor == pocketStack)
				{
					lootPtr = equipPtr->itemInfo[i].pocketPtr.get();
					break;
				}
				else
				{
					pocketStack++;
				}
			}
		}

		int inputSelectNumber = wtoi(exInputText.c_str()); // 셀렉트 박스에 넣어질 숫자(플레이어가 입력한 값)
		if (inputSelectNumber > lootPtr->itemInfo[lootCursor].number) //만약 실제 있는 숫자보다 많은 값을 입력했을 경우
		{
			inputSelectNumber = lootPtr->itemInfo[lootCursor].number; //Select의 값을 최댓값으로 맞춤
		}
		lootPtr->itemInfo[lootCursor].lootSelect = inputSelectNumber;
	}

	Corouter executeWield()
	{
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::TWOHANDED)) //양손장비일 경우
		{
			bool isWield = false;
			std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
			for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (equipPtr->itemInfo[i].equipState == equipHandFlag::left || equipPtr->itemInfo[i].equipState == equipHandFlag::right || equipPtr->itemInfo[i].equipState == equipHandFlag::both)
				{
					equipPtr->transferItem(drop.get(), i, 1);
					isWield = true;
				}
			}
			if (isWield == true) { PlayerPtr->drop(drop.get()); }
			int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
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
				switch (wtoi(coAnswer.c_str()))
				{
					case 0:
						handDir = equipHandFlag::left;
						break;
					case 1:
						handDir = equipHandFlag::right;
						break;
				}

				//왼손 아이템 떨구기
				std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == handDir)
					{
						equipPtr->transferItem(drop.get(), i, 1);
						break;
					}
				}
				//양손 아이템 떨구기
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == equipHandFlag::both)
					{
						equipPtr->transferItem(drop.get(), i, 1);
						break;
					}
				}
				PlayerPtr->drop(drop.get());

				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
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
				switch (wtoi(coAnswer.c_str()))
				{
					case 0:
						handDir = equipHandFlag::left;
						break;
					case 1:
						handDir = equipHandFlag::right;
						break;
				}
				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == true)//왼손에 들기
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::left;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
			else//오른손에 들기
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::right;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
		}
		PlayerPtr->updateStatus();
		PlayerPtr->updateStatus();
		PlayerPtr->updateCustomSpriteHuman();
	}

	Corouter executeReload()//장전 : 타겟아이템(탄창이나 총)에 넣을 수 있는 탄환을 넣는다.
	{
		int targetEquipCursor = equipCursor;
		std::vector<std::wstring> bulletList;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == equipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				bulletList.push_back(equipPtr->itemInfo[i].name);
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = equipPtr->itemInfo[i].pocketPtr.get();
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						bulletList.push_back(pocketPtr->itemInfo[pocketItr].name);
						break;
					}
				}
			}
		}

		if (bulletList.size() == 0) //넣을만한 포켓을 찾지 못했을 경우
		{
			//이 아이템을 넣을만한 포켓이 없다.
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], bulletList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == targetEquipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(1층)
				{
					if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
					{
						if (equipPtr->itemInfo[i].checkFlag(itemFlag::AMMO))
						{
							//(%bullet) (%number)개를(을) (%gun)에 장전했다!

							std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
							std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str3);
						}
						else
						{
							//(%magazine)를(을) (%gun)에 장전했다!
							std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str2);
						}
					}
					else
					{
						//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
						std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", equipPtr->itemInfo[i].name);
						std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
						std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
						updateLog(col2Str(col::white) + str3);
					}

					equipPtr->transferItem
					(
						equipPtr->itemInfo[targetEquipCursor].pocketPtr.get(),
						i,
						equipPtr->itemInfo[i].number
					);
					co_return;
				}
				counter++;
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = equipPtr->itemInfo[i].pocketPtr.get();
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(2층)
						{
							if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
							{
								if (pocketPtr->itemInfo[pocketItr].checkFlag(itemFlag::AMMO))
								{
									//(%bullet) (%number)개를(을) (%gun)에 장전했다!

									std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
									std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str3);
								}
								else
								{
									//(%magazine)를(을) (%gun)에 장전했다!
									std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str2);
								}
							}
							else
							{
								//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
								std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
								std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
								std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
								updateLog(col2Str(col::white) + str3);
							}


							pocketPtr->transferItem
							(
								equipPtr->itemInfo[targetEquipCursor].pocketPtr.get(),
								pocketItr,
								pocketPtr->itemInfo[pocketItr].number
							);
							co_return;
						}
						counter++;
						break;
					}
				}
			}
		}
	}

	Corouter executeThrowing(ItemPocket* inputPocket, int inputIndex)//던지기
	{
		new CoordSelect(sysStr[131]);
		deactDraw();

		co_await std::suspend_always();

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		PlayerPtr->setDirection(getIntDegree(PlayerX(), PlayerY(), targetX, targetY));

		prt(L"executeThrowing에서 사용한 좌표의 값은 (%d,%d,%d)이다.\n", targetX, targetY, targetZ);

		if (targetX == PlayerX() && targetY == PlayerY() && targetZ == PlayerZ())
		{
			std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
			inputPocket->transferItem(drop.get(), inputIndex, 1);
			PlayerPtr->drop(drop.get());
			updateLog(L"#FFFFFF아이템을 버렸다.");
		}
		else
		{
			std::unique_ptr<ItemPocket> throwing = std::make_unique<ItemPocket>(storageType::null);
			//이큅일 떄는 그렇다쳐도 가방 안에 있는 아이템을 던질 떄 원하는대로 작동하지않아 오류가 생긴다
			inputPocket->transferItem(throwing.get(), inputIndex, 1);
			PlayerPtr->throwing(throwing.get(), targetX, targetY);
			updateLog(L"#FFFFFF아이템을 던졌다.");
		}

		PlayerPtr->updateStatus();
		PlayerPtr->updateCustomSpriteHuman();
		close(aniFlag::null);
	}

};
