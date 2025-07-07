﻿export module actFuncSet;

import std;
import util;
import globalVar;
import wrapVar;
import World;
import Entity;
import ItemStack;
import ItemPocket;
import log;
import Lst;
import Player;
import Prop;
import ItemData;

//액트가 실행되는 환경은 3가지 경우가 가능
// 0:기본 HUD, 1:Loot, 2:Equip 

export enum class actEnv
{
	HUD,
	Loot,
	Equip,
	Inventory
};

export namespace actFunc
{
	//장전 : 총이나 탄창에 사용, 자기 자신의 탄환을 채워넣음
	export Corouter reloadSelf(actEnv envType, ItemPocket* reloadItemPocket, int reloadItemCursor)
	{
		prt(L"executeReloadSelf이 실행되었다.\n");
		int targetLootCursor = reloadItemCursor;
		std::vector<std::wstring> bulletList;
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		std::vector<ItemPocket*> targetSearchPtr;
		std::vector<ItemData>& equipInfo = PlayerPtr->getEquipPtr()->itemInfo;

		if (reloadItemPocket->itemInfo[targetLootCursor].checkFlag(itemFlag::BOW))
		{
			for (int j = 0; j < equipInfo.size(); j++)
			{
				if (equipInfo[j].itemCode == itemRefCode::arrowQuiver)
				{
					if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
					{
						equipInfo[j].pocketPtr.get()->transferItem(reloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get(), 0, 1);
						updateLog(col2Str(col::white) + L"당신은 화살을 시위에 걸었다.");
						co_return;
					}
				}
			}
		}
		else if (reloadItemPocket->itemInfo[targetLootCursor].checkFlag(itemFlag::CROSSBOW))
		{
			for (int j = 0; j < equipInfo.size(); j++)
			{
				if (equipInfo[j].itemCode == itemRefCode::boltQuiver)
				{
					if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
					{
						equipInfo[j].pocketPtr.get()->transferItem(reloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get(), 0, 1);
						updateLog(col2Str(col::white) + L"당신은 화살을 시위에 걸었다.");
						co_return;
					}
				}
			}
		}

		//탐사할 타일 추가 (장비, 주변타일 9칸)
		{
			//장비타일
			targetSearchPtr.push_back(equipPtr);
			//바닥타일(주변9타일)
			for (int dir = -1; dir < 8; dir++)
			{
				int dx = 0, dy = 0;
				dir2Coord(dir, dx, dy);

				ItemStack* stack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
				if (stack != nullptr)
				{
					ItemPocket* lootPtr = stack->getPocket();
					targetSearchPtr.push_back(lootPtr);
				}
			}
		}

		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			//장비 중인 아이템에서 bulletList(또는 magazine) 추가
			for (int i = 0; i < targetSearchPtr[j]->itemInfo.size(); i++)
			{
				//만약 이 아이템에 넣을 수 있는 아이템코드가 equip에 있는 아이템과 같으면
				if (std::find(reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.begin(), reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end(), targetSearchPtr[j]->itemInfo[i].itemCode) != reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end())
				{
					bulletList.push_back(targetSearchPtr[j]->itemInfo[i].name);
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
		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			for (int i = 0; i < targetSearchPtr[j]->itemInfo.size(); i++)
			{
				//만약 이 아이템에 넣을 수 있는 아이템코드가 equip에 있는 아이템과 같으면
				if (std::find(reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.begin(), reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end(), targetSearchPtr[j]->itemInfo[i].itemCode) != reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end())
				{
					if (counter == wtoi(coAnswer.c_str()))
					{
						//넣을 수 있는만큼 가득 넣음
						targetSearchPtr[j]->transferItem
						(
							reloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get(),
							i,
							1//일단은 전부 넣는걸로
						);

						co_return;
					}
					counter++;
				}
			}
		}
		updateQuiverSpr(PlayerPtr->getEquipPtr());

	}

	export Corouter reloadOther(actEnv envType, ItemPocket* reloadItemPocket, int reloadItemCursor)//삽탄 : 총알에 사용, 이 탄환을 넣을 수 있는 탄창 리스트를 표시하고 거기에 넣음
	{
		//탄창이 장착한 총에 바로 넣는 기능 추가? -> 탄창을 안 빼고 총알을 넣는게 현실적으로 가능할리가 없다

		prt(L"executeReloadOther이 실행되었다.\n");
		int targetLootCursor = reloadItemCursor;
		std::vector<std::wstring> pocketList;
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		std::vector<ItemPocket*> targetSearchPtr;

		//탐사할 타일 추가 (장비, 주변타일 9칸)
		{
			targetSearchPtr.push_back(equipPtr);
			//바닥타일(주변9타일)
			for (int dir = -1; dir < 8; dir++)
			{
				int dx = 0, dy = 0;
				dir2Coord(dir, dx, dy);
				ItemStack* stack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
				if (stack != nullptr)
				{
					ItemPocket* lootPtr = stack->getPocket();
					targetSearchPtr.push_back(lootPtr);
				}
			}
		}

		//장비 중인 아이템에서 pocketList 추가
		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			for (int i = 0; i < targetSearchPtr[j]->itemInfo.size(); i++)
			{
				if (targetSearchPtr[j]->itemInfo[i].pocketMaxNumber > 0 && countPocketItemNumber(targetSearchPtr[j]->itemInfo[i].pocketPtr.get()) < targetSearchPtr[j]->itemInfo[i].pocketMaxNumber)
				{
					if (std::find(targetSearchPtr[j]->itemInfo[i].pocketOnlyItem.begin(), targetSearchPtr[j]->itemInfo[i].pocketOnlyItem.end(), reloadItemPocket->itemInfo[targetLootCursor].itemCode) != targetSearchPtr[j]->itemInfo[i].pocketOnlyItem.end())
					{
						pocketList.push_back(targetSearchPtr[j]->itemInfo[i].name);
					}
				}
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
		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			for (int i = 0; i < targetSearchPtr[j]->itemInfo.size(); i++)
			{
				if (targetSearchPtr[j]->itemInfo[i].pocketMaxNumber > 0 && countPocketItemNumber(targetSearchPtr[j]->itemInfo[i].pocketPtr.get()) < targetSearchPtr[j]->itemInfo[i].pocketMaxNumber)
				{
					if (std::find(targetSearchPtr[j]->itemInfo[i].pocketOnlyItem.begin(), targetSearchPtr[j]->itemInfo[i].pocketOnlyItem.end(), reloadItemPocket->itemInfo[targetLootCursor].itemCode) != targetSearchPtr[j]->itemInfo[i].pocketOnlyItem.end())
					{
						if (counter == wtoi(coAnswer.c_str()))
						{
							reloadItemPocket->transferItem
							(
								targetSearchPtr[j]->itemInfo[i].pocketPtr.get(),
								targetLootCursor,
								1
							);

							co_return;
						}

						counter++;
					}
				}
			}
		}

		updateQuiverSpr(PlayerPtr->getEquipPtr());
	}


	export void unload(ItemPocket* unloadItemPocket, int unloadItemCursor)//장전해제 : 타겟아이템에 들어있는 아이템을 드랍하거나 인벤토리에 넣는다.
	{
		int targetLootCursor = unloadItemCursor;
		ItemPocket* targetPocket = unloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get();
		std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
		for (int i = 0; i < targetPocket->itemInfo.size(); i++) { targetPocket->transferItem(drop.get(), i, targetPocket->itemInfo[i].number); }
		PlayerPtr->drop(drop.get());
		updateQuiverSpr(PlayerPtr->getEquipPtr());
	}

	export void closeDoor(int tgtX, int tgtY, int tgtZ)
	{
		Prop* tgtProp = TileProp(tgtX, tgtY, tgtZ);
		tgtProp->leadItem.eraseFlag(itemFlag::DOOR_OPEN);
		tgtProp->leadItem.addFlag(itemFlag::DOOR_CLOSE);

		if (tgtProp->leadItem.checkFlag(itemFlag::PROP_GAS_OBSTACLE_OFF))
		{
			tgtProp->leadItem.eraseFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
			tgtProp->leadItem.addFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
		}

		tgtProp->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
		tgtProp->leadItem.addFlag(itemFlag::PROP_BLOCKER);
		tgtProp->leadItem.extraSprIndexSingle--;
		PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
	}

	export void closeVDoor(int tgtX, int tgtY, int tgtZ)
	{
		ItemPocket* tgtPocket = TileVehicle(tgtX, tgtY, PlayerZ())->partInfo[{tgtX, tgtY }].get();
		for (int i = 0; i < tgtPocket->itemInfo.size(); i++)
		{
			if (tgtPocket->itemInfo[i].checkFlag(itemFlag::VPART_DOOR_OPEN))
			{
				tgtPocket->itemInfo[i].eraseFlag(itemFlag::VPART_DOOR_OPEN);
				tgtPocket->itemInfo[i].addFlag(itemFlag::VPART_DOOR_CLOSE);

				tgtPocket->itemInfo[i].addFlag(itemFlag::VPART_NOT_WALKABLE);

				if (tgtPocket->itemInfo[i].checkFlag(itemFlag::PROP_GAS_OBSTACLE_OFF))
				{
					tgtPocket->itemInfo[i].eraseFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
					tgtPocket->itemInfo[i].addFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
				}
				tgtPocket->itemInfo[i].propSprIndex -= 16;
				PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
			}
		}

    }

	export void toggle(ItemData& inputItem)
	{
		if (inputItem.checkFlag(itemFlag::TOGGLE_OFF))
		{
			inputItem.eraseFlag(itemFlag::TOGGLE_OFF);
			inputItem.addFlag(itemFlag::TOGGLE_ON);

			inputItem.lightPtr = std::make_unique<Light>(PlayerX(), PlayerY(), PlayerZ(), 6, 80, SDL_Color{ 150, 150, 250 });
			inputItem.sprIndex += 1;
			PlayerPtr->updateVision();
			updateLog(L"#FFFFFF헤드랜턴의 전원을 켰다.");
		}
		else if (inputItem.checkFlag(itemFlag::TOGGLE_ON))
		{
			inputItem.eraseFlag(itemFlag::TOGGLE_ON);
			inputItem.addFlag(itemFlag::TOGGLE_OFF);

			inputItem.lightPtr.reset();
			inputItem.sprIndex -= 1;

			PlayerPtr->updateVision();
			updateLog(L"#FFFFFF헤드랜턴의 전원을 껐다.");

		}
	}

	export void drinkBottle(ItemData& inputData)
	{
		errorBox(inputData.pocketPtr == nullptr, L"drinkBottle: inputData.pocketPtr is nullptr.");
		errorBox(inputData.pocketPtr->itemInfo.size() == 0, L"drinkBottle: inputData.pocketPtr->itemInfo.size() is 0.");

		int needHydration = PLAYER_MAX_HYDRATION - thirst;

		if (needHydration <= 0)
		{
			updateLog(col2Str(col::white) + L"더 이상 마실 필요가 없다.");
			return;
		}

		for (int i = 0; i < inputData.pocketPtr->itemInfo.size(); i++)
		{
			if (inputData.pocketPtr->itemInfo[i].itemCode == itemRefCode::water)
			{
				int hydrationPerWater = inputData.pocketPtr->itemInfo[i].hydrationPerML;
				int waterCount = inputData.pocketPtr->itemInfo[i].number;
				int waterNeeded = (needHydration + hydrationPerWater - 1) / hydrationPerWater;
				int waterToConsume = myMin(waterNeeded, waterCount);
				int actualHydration = waterToConsume * hydrationPerWater;
				actualHydration = myMin(actualHydration, needHydration);

				thirst += actualHydration;
				if (thirst > PLAYER_MAX_HYDRATION) thirst = PLAYER_MAX_HYDRATION;

				inputData.pocketPtr->subtractItemIndex(i, waterToConsume);
				updateLog(col2Str(col::white) + L"아이템을 마셨다. 갈증이 해소되었다.");
				return;
			}
		}

		updateLog(col2Str(col::white) + L"물병에 물이 없다.");
	}

	export void eatFood(ItemPocket* inputPocket, int inputCursor)
	{
		errorBox(inputPocket == nullptr, L"eatFood: inputPocket is nullptr.");
		errorBox(inputCursor < 0 || inputCursor >= inputPocket->itemInfo.size(), L"eatFood: inputCursor is out of bounds.");

		ItemData& targetItem = inputPocket->itemInfo[inputCursor];

		// 아이템의 칼로리 확인
		int itemCalorie = targetItem.calorie;

		// 현재 허기 상태와 최대 허기 수치 확인
		int needCalorie = PLAYER_MAX_CALORIE - hunger;

		if (needCalorie <= 0)
		{
			updateLog(col2Str(col::white) + L"배가 불러서 더 이상 먹을 수 없다.");
			return;
		}

		// 칼로리 회복
		hunger += itemCalorie;
		if (hunger > PLAYER_MAX_CALORIE)
		{
			hunger = PLAYER_MAX_CALORIE;
		}

		// 아이템 1개 제거
		inputPocket->subtractItemIndex(inputCursor, 1);

		updateLog(col2Str(col::white) + L"음식을 먹었다. 허기가 해소되었다.");
	}

	export void spillPocket(ItemData& inputData)
	{
		ItemPocket* pPtr = inputData.pocketPtr.get();
		errorBox(pPtr == nullptr, L"spillPocket: inputData.pocketPtr is nullptr.");
		errorBox(pPtr->itemInfo.size() == 0, L"spillPocket: inputData.pocketPtr->itemInfo.size() is 0.");

		std::wstring itemName = inputData.name;
		Point3 playerPos = { PlayerX(), PlayerY(), PlayerZ() };

		// 플레이어 위치에 ItemStack이 있는지 확인
		ItemStack* existingStack = TileItemStack(playerPos.x, playerPos.y, playerPos.z);

		if (existingStack == nullptr)
		{
			// ItemStack이 없으면 새로 생성
			createItemStack(playerPos);
			existingStack = TileItemStack(playerPos.x, playerPos.y, playerPos.z);
			errorBox(existingStack == nullptr, L"spillPocket: Failed to create ItemStack.");
		}

		ItemPocket* targetPocket = existingStack->getPocket();

		// 포켓의 모든 아이템을 바닥의 ItemStack으로 이동
		while (pPtr->itemInfo.size() > 0)
		{
			// 첫 번째 아이템의 전체 수량을 이동
			int itemCount = pPtr->itemInfo[0].number;
			pPtr->transferItem(targetPocket, 0, itemCount);
		}

		std::wstring logText = replaceStr(sysStr[297],L"(%container)" , itemName);
		updateLog(col2Str(col::white) + logText);
	}
};