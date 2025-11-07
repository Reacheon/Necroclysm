#include <SDL3/SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Inventory;

import std;
import util;
import GUI;
import textureVar;
import wrapVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import ItemData;
import ItemPocket;
import drawItemList;
import Msg;
import actFuncSet;
import log;
import CoordSelect;

export class Inventory : public GUI
{
private:
	ItemData* inventoryItemData;
	ItemPocket* inventoryPocket;

	SDL_Rect inventoryBase;
	std::array<SDL_Rect, 12> bionicRect;
	int inventoryCursor = -1;
	int inventoryScroll = 0;

	std::wstring titleInventory = sysStr[185];
	int titleItemSprIndex = 60;

	std::array<SDL_Rect, INVENTORY_ITEM_MAX> inventoryItemRect; //마우스를 위한 인벤아이템렉트 판정 박스
	std::array<SDL_Rect, INVENTORY_ITEM_MAX> inventoryItemSelectRect; //마우스를 위한 인벤아이템렉트 셀렉트 판정 박스
	SDL_Rect inventoryLabel;
	SDL_Rect inventoryLabelSelect;
	SDL_Rect inventoryLabelName;
	SDL_Rect inventoryLabelQuantity;
	SDL_Rect inventoryScrollBox;
	SDL_Rect dropBtn;
public:
	Inventory(int inputX, int inputY, ItemData* inputData) : GUI(false)
	{
		inventoryItemData = inputData;
		inventoryPocket = inputData->pocketPtr.get();

		//메세지 박스 렌더링
		changeXY(inputX, inputY, false);

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Inventory()
	{
		for (int i = 0; i < inventoryPocket->itemInfo.size(); i++) { inventoryPocket->itemInfo[i].lootSelect = 0; }
	}
	void changeXY(int inputX, int inputY, bool center)
	{
		inventoryBase = { 0, 0, 404, 506 };
		inventoryBase.h = 197 + 38 * myMax(0, (myMin(INVENTORY_ITEM_MAX, inventoryPocket->itemInfo.size() - 1)));

		if (center == false)
		{
			inventoryBase.x += inputX;
			inventoryBase.y += inputY;
		}
		else
		{
			inventoryBase.x += inputX - inventoryBase.w / 2;
			inventoryBase.y += inputY - inventoryBase.h / 2;
		}

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - inventoryBase.w / 2;
			y = inventoryBase.h / 2;
		}

		for (int i = 0; i < INVENTORY_ITEM_MAX; i++)
		{
			inventoryItemRect[i] = { inventoryBase.x + 63, inventoryBase.y + 150 + 37*i, 325, 32 };
			inventoryItemSelectRect[i] = { inventoryBase.x + 12, inventoryBase.y + 150 + 37 * i, 43, 32 };
		}
		inventoryLabel = { inventoryBase.x + 12, inventoryBase.y + 114, inventoryBase.w - 24 , 31 };
		inventoryLabelSelect = { inventoryLabel.x, inventoryLabel.y, 75 , 31 };
		inventoryLabelName = { inventoryLabel.x + inventoryLabelSelect.w, inventoryLabel.y, 219 , 31 };
		inventoryLabelQuantity = { inventoryLabel.x + inventoryLabelName.w + inventoryLabelSelect.w, inventoryLabel.y, 85 , 31 };
		dropBtn = { inventoryBase.x + 299, inventoryBase.y + 40, 100, 35 };
	}
	void drawGUI();
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
			return;
		}
		else if (checkCursor(&inventoryLabel))
		{
			if (checkCursor(&inventoryLabelSelect))
			{
				executeSelectAll();
			}
			else if (checkCursor(&inventoryLabelName))
			{
				// 나중에 검색 기능 추가 시 사용
				// CORO(executeSearch());
			}
			else if (checkCursor(&inventoryLabelQuantity))
			{
				// 나중에 정렬 기능 추가 시 사용
				// executeSort();
			}
		}
		else if (checkCursor(&dropBtn))
		{
			// 선택된 아이템이 있는지 확인
			bool hasSelectedItems = false;
			for (int i = 0; i < inventoryPocket->itemInfo.size(); i++)
			{
				if (inventoryPocket->itemInfo[i].lootSelect > 0)
				{
					hasSelectedItems = true;
					break;
				}
			}

			if (hasSelectedItems)
			{
				CORO(executeDropInventory(inventoryPocket));

			}
			return;
		}
		else if (checkCursor(&inventoryBase))
		{
			// 만약 아이템을 클릭했으면 커서를 그 아이템으로 옮김, 다른 곳 누르면 -1로 바꿈
			for (int i = 0; i < INVENTORY_ITEM_MAX; i++)
			{
				if (inventoryPocket->itemInfo.size() - 1 >= i + inventoryScroll)
				{
					if (checkCursor(&inventoryItemRect[i]))
					{
						if (inventoryCursor != inventoryScroll + i) //새로운 커서 생성
						{
							inventoryCursor = inventoryScroll + i;
							updateBarAct();
						}
						else //커서 삭제
						{
							inventoryCursor = -1;
							barAct = actSet::null;
						}
						return;
					}
				}
			}

			// 아이템 좌측 셀렉트 클릭
			for (int i = 0; i < INVENTORY_ITEM_MAX; i++)
			{
				if (checkCursor(&inventoryItemSelectRect[i]))
				{
					if (inventoryPocket->itemInfo.size() - 1 >= i + inventoryScroll)
					{
						if (inventoryPocket->itemInfo[i + inventoryScroll].lootSelect == 0)
						{
							if (option::inputMethod == input::mouse)
							{
								executeSelectItem(i + inventoryScroll);
							}
							else if (option::inputMethod == input::touch)
							{
								executeSelectItem(i + inventoryScroll);
							}
						}
						else
						{
							inventoryPocket->itemInfo[i + inventoryScroll].lootSelect = 0;
						}
					}
				}
			}
		}
		else if (checkCursor(&letterbox)) // 하단 액션 버튼들
		{
			for (int i = 0; i < barAct.size(); i++)
			{
				if (checkCursor(&barButton[i]))
				{
					switch (barAct[i])
					{
					case act::wield:
						CORO(actFunc::executeWield(inventoryPocket, inventoryCursor));
						break;
					case act::equip:
						actFunc::executeEquip(inventoryPocket, inventoryCursor);
						break;
					case act::throwing:
						deactDraw();
						CORO(actFunc::executeThrowing(inventoryPocket, inventoryCursor));
						// close(aniFlag::null); // 필요에 따라
						return;
					case act::eat:
						actFunc::eatFood(inventoryPocket, inventoryCursor);
						updateBarAct();
						return;
					case act::drink:
						actFunc::drinkBottle(inventoryPocket->itemInfo[inventoryCursor]);
						updateBarAct();
						return;
					case act::toggleOff:
					case act::toggleOn:
						actFunc::toggle(inventoryPocket->itemInfo[inventoryCursor]);
						updateBarAct();
						return;
					case act::dump:
						actFunc::spillPocket(inventoryPocket->itemInfo[inventoryCursor]);
						updateBarAct();
						return;
						// 총기 관련 액션들도 필요하면 추가
					case act::reloadBulletToMagazine:
					case act::reloadBulletToGun:
						// ... 총기 관련 로직
						break;
						// 기타 액션들...
					case act::open:
						executeOpen();
						return;
					}

					// 아이템이 삭제되었을 때 처리
					if (inventoryPocket->itemInfo.size() == 0)
					{
						close(aniFlag::winUnfoldClose);
						return;
					}

					// 스크롤 조정
					if (inventoryPocket->itemInfo.size() - 1 <= inventoryScroll + INVENTORY_ITEM_MAX)
					{
						inventoryScroll = inventoryPocket->itemInfo.size() - INVENTORY_ITEM_MAX;
						if (inventoryScroll < 0) { inventoryScroll = 0; }
					}
					break;
				}
			}
		}

		// 위의 모든 경우에서 return을 받지 못했으면 커서를 -1로 복구
		{
			inventoryCursor = -1;
			barAct = actSet::null;
		}
	}
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void clickRightGUI()
	{
		//아이템 좌측 셀렉트 우클릭
		for (int i = 0; i < INVENTORY_ITEM_MAX; i++)
		{
			if (checkCursor(&inventoryItemSelectRect[i]))
			{
				if (inventoryPocket->itemInfo.size() - 1 >= i + inventoryScroll)
				{
					if (inventoryPocket->itemInfo[i + inventoryScroll].lootSelect == 0)
					{
						CORO(executeSelectItemEx(i + inventoryScroll));
					}
					else
					{
						inventoryPocket->itemInfo[i + inventoryScroll].lootSelect = 0;
					}
				}
			}
		}
	}
	void mouseWheel() 
	{
		if (checkCursor(&inventoryBase))
		{
			if (event.wheel.y > 0 && inventoryScroll > 1) inventoryScroll -= 1;
			else if (event.wheel.y < 0 && inventoryScroll + INVENTORY_ITEM_MAX < inventoryPocket->itemInfo.size()) inventoryScroll += 1;
		}
	}
	void clickHoldGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		tabType = tabFlag::back;

		// 윈도우 높이 조정 (동적으로 변경)
		inventoryBase.h = 197 + 38 * myMax(0, (myMin(INVENTORY_ITEM_MAX - 1, inventoryPocket->itemInfo.size() - 1)));

		// 게임패드 지원이 필요하다면
		if (option::inputMethod == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000)
			{
				prt(L"탭이 실행되었다.\n");
				close(aniFlag::winUnfoldClose);
				delayR2 = 20;
			}
			else delayR2--;
		}

		// 잘못된 커서 위치 조정
		if (inventoryCursor > (int)(inventoryPocket->itemInfo.size() - 1))
		{
			inventoryCursor = inventoryPocket->itemInfo.size() - 1;
		}

		// 잘못된 스크롤 위치 조정
		if (option::inputMethod == input::mouse || option::inputMethod == input::touch)
		{
			if (inventoryScroll + INVENTORY_ITEM_MAX >= inventoryPocket->itemInfo.size())
			{
				inventoryScroll = myMax(0, (int)inventoryPocket->itemInfo.size() - INVENTORY_ITEM_MAX);
			}
			else if (inventoryScroll < 0)
			{
				inventoryScroll = 0;
			}
		}
	}

	void executeSelectAll()
	{
		bool isSelectAll = true;
		for (int i = 0; i < inventoryPocket->itemInfo.size(); i++)
		{
			if (inventoryPocket->itemInfo[i].lootSelect != inventoryPocket->itemInfo[i].number)
			{
				isSelectAll = false;
				break;
			}
		}

		if (isSelectAll == false)
		{
			for (int i = 0; i < inventoryPocket->itemInfo.size(); i++)
			{
				inventoryPocket->itemInfo[i].lootSelect = inventoryPocket->itemInfo[i].number;
			}
		}
		else
		{
			for (int i = 0; i < inventoryPocket->itemInfo.size(); i++)
			{
				inventoryPocket->itemInfo[i].lootSelect = 0;
			}
		}
	}

	void executeSelectItem(int index)
	{
		int itemNumber = inventoryPocket->itemInfo[index].number;
		inventoryPocket->itemInfo[index].lootSelect = itemNumber;
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
			try { inputSelectNumber = wtoi(exInputText.c_str()); }
			catch (...) {}

			if (inputSelectNumber < 0) inputSelectNumber = 0;
			else if (inputSelectNumber > inventoryPocket->itemInfo[index].number)
				inputSelectNumber = inventoryPocket->itemInfo[index].number;

			inventoryPocket->itemInfo[index].lootSelect = inputSelectNumber;
		}
	}

	void updateBarAct()
	{
		if (inventoryPocket->itemInfo.size() > 0)
		{
			ItemData& targetItem = inventoryPocket->itemInfo[inventoryCursor];
			barAct.clear();
			if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
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
				if (targetItem.itemCode != itemRefCode::arrowQuiver && targetItem.itemCode != itemRefCode::boltQuiver)
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

	Corouter executeDropInventory(ItemPocket* inventoryPocket)
	{


		// 선택된 아이템이 있는지 확인
		bool hasSelectedItems = false;
		for (int i = 0; i < inventoryPocket->itemInfo.size(); i++)
		{
			if (inventoryPocket->itemInfo[i].lootSelect > 0)
			{
				hasSelectedItems = true;
				break;
			}
		}
		if (!hasSelectedItems)
		{
			updateLog(L"No items selected to drop.");
			co_return;
		}

		// 주변 9칸(자신 포함) 좌표 선택
		std::vector<Point2> selectableCoord;
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++)
			{
				selectableCoord.push_back({ PlayerX() + dx, PlayerY() + dy });
			}
		}

		new CoordSelect(L"Drop items where?", selectableCoord);
		co_await std::suspend_always();

		if (coAnswer.empty())
		{
			updateLog(L"Drop cancelled.");
			co_return;
		}

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		// 플레이어 방향 설정 (같은 위치가 아닐 때만)
		if (targetX != PlayerX() || targetY != PlayerY())
		{
			PlayerPtr->setDirection(getIntDegree(PlayerX(), PlayerY(), targetX, targetY));
		}

		// 하나의 포켓에 모든 선택된 아이템을 담기
		std::unique_ptr<ItemPocket> droppingPocket = std::make_unique<ItemPocket>(storageType::null);

		for (int i = inventoryPocket->itemInfo.size() - 1; i >= 0; i--)
		{
			if (inventoryPocket->itemInfo[i].lootSelect > 0)
			{
				int dropAmount = inventoryPocket->itemInfo[i].lootSelect;
				inventoryPocket->transferItem(droppingPocket.get(), i, dropAmount);
			}
		}

		// 한 번에 던지기
		updateLog(L"You drop the selected items.");
		PlayerPtr->throwing(std::move(droppingPocket), targetX, targetY);
		PlayerPtr->updateStatus();

		for (int i = GUI::activeGUIList.size() - 1; i >= 0; i--)
		{
			if (GUI::activeGUIList[i] == this)//Equip과 Inventory를 모두 종료
			{
				GUI::activeGUIList[i]->close(aniFlag::null);
				GUI::activeGUIList[i - 1]->close(aniFlag::null);
				break;
			}
		}
	}

	void executeOpen()
	{
		new Inventory(404, (cameraH / 2) - 210, &inventoryPocket->itemInfo[inventoryCursor]);
		close(aniFlag::null);
	}
};