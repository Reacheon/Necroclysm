module;
#include <SDL3/SDL.h>
export module Inventory;

import std;
import util;
import constVar;
import GUI;
import textureVar;
import World;
import Player;
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
import ItemListPanel;
import barActCommon;

export class Inventory : public GUI
{
private:
	ItemData* inventoryItemData;
	ItemPocket* inventoryPocket;

	SDL_Rect inventoryBase;
	std::array<SDL_Rect, 12> bionicRect;

	std::wstring titleInventory = sysStr[185];
	int titleItemSprIndex = 60;

	SDL_Rect dropBtn;
public:
	ItemListPanel panel{ INVENTORY_ITEM_MAX };

	Inventory(int inputX, int inputY, ItemData* inputData) : GUI(false)
	{
		inventoryItemData = inputData;
		inventoryPocket = inputData->pocketPtr.get();
		panel.pocket = inventoryPocket;

		//메세지 박스 렌더링
		changeXY(inputX, inputY, false);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}
	~Inventory()
	{
		panel.clearAllSelections();
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

		panel.initRects(inventoryBase.x + 12, inventoryBase.y + 150, 51);
		panel.label = { inventoryBase.x + 12, inventoryBase.y + 114, inventoryBase.w - 24, 31 };
		panel.labelSelect = { panel.label.x, panel.label.y, 75, 31 };
		panel.labelName = { panel.label.x + panel.labelSelect.w, panel.label.y, 219, 31 };
		panel.labelQuantity = { panel.label.x + panel.labelName.w + panel.labelSelect.w, panel.label.y, 85, 31 };
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
		else if (checkCursor(&panel.label))
		{
			if (checkCursor(&panel.labelSelect))
			{
				panel.selectAll();
			}
			else if (checkCursor(&panel.labelName))
			{
				// 나중에 검색 기능 추가 시 사용
				// Corouter::start(actFunc::searchItems(panel.pocket, panel.scroll));
			}
			else if (checkCursor(&panel.labelQuantity))
			{
				// 나중에 정렬 기능 추가 시 사용
				// panel.sort();
			}
		}
		else if (checkCursor(&dropBtn))
		{
			if (panel.hasAnySelection())
			{
				Corouter::start(executeDropInventory(inventoryPocket));
			}
			return;
		}
		else if (checkCursor(&inventoryBase))
		{
			//아이템 클릭 → 커서 토글
			{
				int result = panel.handleItemClick();
				if (result == 1) { updateBarAct(); return; }
				else if (result == -1) { return; }
			}

			//아이템 좌측 셀렉트 클릭
			if (panel.handleSelectClick()) { return; }
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
						Corouter::start(actFunc::executeWield(inventoryPocket, panel.cursor));
						break;
					case act::equip:
						actFunc::executeEquip(inventoryPocket, panel.cursor);
						break;
					case act::throwing:
						deactDraw();
						Corouter::start(actFunc::executeThrowing(inventoryPocket, panel.cursor));
						return;
					case act::eat:
						actFunc::eatFood(inventoryPocket, panel.cursor);
						updateBarAct();
						return;
					case act::drink:
						actFunc::drinkBottle(inventoryPocket->itemInfo[panel.cursor]);
						updateBarAct();
						return;
					case act::toggleOff:
					case act::toggleOn:
						actFunc::toggle(inventoryPocket->itemInfo[panel.cursor]);
						updateBarAct();
						return;
					case act::dump:
						actFunc::spillPocket(inventoryPocket->itemInfo[panel.cursor]);
						updateBarAct();
						return;
					case act::reloadBulletToMagazine:
					case act::reloadBulletToGun:
						// ... 총기 관련 로직
						break;
					case act::open:
						executeOpen();
						return;
					case act::plant:
					{
						Corouter::start(actFunc::executePlant(inventoryPocket, panel.cursor));
						break;
					}
					case act::extractSeed:
						actFunc::extractSeed(actEnv::Inventory, inventoryPocket, panel.cursor, inventoryItemData->pocketMaxVolume);
						updateBarAct();
						return;
					case act::dye:
						Corouter::start(actFunc::executeDye(inventoryPocket, panel.cursor));
						return;
					}

					// 아이템이 삭제되었을 때 처리
					if (inventoryPocket->itemInfo.size() == 0)
					{
						close(aniFlag::winUnfoldClose);
						return;
					}

					// 스크롤 조정
					panel.adjustScrollAfterAction();
					break;
				}
			}
		}

		// 위의 모든 경우에서 return을 받지 못했으면 커서를 -1로 복구
		{
			panel.cursor = -1;
			barAct = actSet::null();
		}
	}
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void clickRightGUI()
	{
		//아이템 좌측 셀렉트 우클릭
		int idx = panel.getSelectRightClickIndex();
		if (idx >= 0)
		{
			Corouter::start(actFunc::selectItemEx(panel.pocket, idx));
		}
	}
	void mouseWheel()
	{
		panel.handleWheel(inventoryBase);
	}
	void clickHoldGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		tabType = tabFlag::back;

		// 윈도우 높이 조정 (동적으로 변경)
		inventoryBase.h = panel.calcWindowHeight();

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

		panel.adjustScrollAndCursor();
	}

	void updateBarAct()
	{
		if (inventoryPocket->itemInfo.size() > 0)
		{
			ItemData& targetItem = inventoryPocket->itemInfo[panel.cursor];
			barAct.clear();
			if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
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

			//설치 및 심기 추가
			if (targetItem.propInstallCode != 0)
			{
				if (targetItem.checkFlag(itemFlag::CAN_PLANT))
					barAct.push_back(act::plant);
				else
					barAct.push_back(act::propInstall);
			}

			//씨앗 추출 추가
			if (targetItem.checkFlag(itemFlag::SEED_FRUIT))
			{
				barAct.push_back(act::extractSeed);
			}

			//염색 앰플 추가
			if (targetItem.itemCode == itemID::dyeAmpule)
			{
				barAct.push_back(act::dye);
			}
		}
	}

	Corouter executeDropInventory(ItemPocket* inventoryPocket)
	{
		// 선택된 아이템이 있는지 확인
		if (!panel.hasAnySelection())
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
		new Inventory(404, (cameraH / 2) - 210, &inventoryPocket->itemInfo[panel.cursor]);
		close(aniFlag::null);
	}
};
