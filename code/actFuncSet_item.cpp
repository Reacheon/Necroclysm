module actFuncSet;

import util;
import constVar;
import globalVar;
import World;
import Player;
import ItemStack;
import log;
import Msg;
import textureVar;
import CoordSelect;
import CoordSelectCraft;
import GUI;
import Lst;
import LstEx;
import SkillRegistry;
import SkillBehavior;

namespace actFunc
{
	void drinkBottle(ItemData& inputData)
	{
		errorBox(inputData.pocketPtr == nullptr, L"drinkBottle: inputData.pocketPtr is nullptr.");
		errorBox(inputData.pocketPtr->itemInfo.size() == 0, L"drinkBottle: inputData.pocketPtr->itemInfo.size() is 0.");

		if (thirst <= 0.0)
		{
			updateLog(L"You're not thirsty.");
			return;
		}

		for (int i = 0; i < inputData.pocketPtr->itemInfo.size(); i++)
		{
			if (inputData.pocketPtr->itemInfo[i].itemCode == itemID::water)
			{
				int hydrationPerWater = inputData.pocketPtr->itemInfo[i].hydrationPerML;
				int waterCount = inputData.pocketPtr->itemInfo[i].number;
				double needPercent = thirst; // 현재 갈증%만큼 해소 필요
				double percentPerWater = hydrationPerWater * HYDRATION_TO_PERCENT;
				int waterNeeded = (int)std::ceil(needPercent / percentPerWater);
				int waterToConsume = myMin(waterNeeded, waterCount);
				double actualPercent = waterToConsume * percentPerWater;
				if (actualPercent > thirst) actualPercent = thirst;

				thirst -= actualPercent;
				if (thirst < 0.0) thirst = 0.0;

				inputData.pocketPtr->subtractItemIndex(i, waterToConsume);
				updateLog(L"You drink from the bottle. Your thirst is quenched.");
				return;
			}
		}

		updateLog(L"The bottle is empty.");
	}

	void eatFood(ItemPocket* inputPocket, int inputCursor)
	{
		errorBox(inputPocket == nullptr, L"eatFood: inputPocket is nullptr.");
		errorBox(inputCursor < 0 || inputCursor >= inputPocket->itemInfo.size(), L"eatFood: inputCursor is out of bounds.");

		ItemData& targetItem = inputPocket->itemInfo[inputCursor];
		int itemCalorie = targetItem.calorie;
		double caloriePercent = itemCalorie * CALORIE_TO_PERCENT;

		if (hunger <= 0.0)
		{
			updateLog(L"You're too full to eat anymore.");
			return;
		}

		// 허기% 감소 (음식을 먹으면 허기가 줄어듦)
		hunger -= caloriePercent;
		if (hunger < 0.0) hunger = 0.0;

		inputPocket->subtractItemIndex(inputCursor, 1);
		updateLog(L"You eat the food. Your hunger is satisfied.");
	}

	void spillPocket(ItemData& inputData)
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

		std::wstring logText = replaceStr(sysStr[297], L"(%container)", itemName);
		updateLog(logText);
	}

	Corouter executeWield(ItemPocket* targetPocket, int targetPocketCursor)
	{
		ItemData& tgtItem = targetPocket->itemInfo[targetPocketCursor];
		ItemPocket* equipPtr = PlayerEquip();
		if (tgtItem.checkFlag(itemFlag::TWOHANDED)) //양손장비일 경우
		{
			std::wstring logStr = replaceStr(sysStr[331], L"(%item)", tgtItem.name);
			updateLog(logStr);
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

			int returnIndex = targetPocket->transferItem(equipPtr, targetPocketCursor, 1);
			equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::both; //양손
			equipPtr->sortEquip();
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

			//오른손 우선, 오른손이 차있으면 왼손
			equipHandFlag handDir = equipHandFlag::right;
			if (hasRight == true && hasLeft == false)
			{
				handDir = equipHandFlag::left;
			}
			else if (hasRight == true && hasLeft == true)
			{
				//양손무기 하나가 양손을 차지하고 있는 경우 → 자동으로 떨구고 오른손에 듬
				bool hasBoth = false;
				for (int i = 0; i < equipPtr->itemInfo.size(); i++)
				{
					if (equipPtr->itemInfo[i].equipState == equipHandFlag::both)
					{
						hasBoth = true;
						break;
					}
				}
				if (hasBoth == false)
				{
					//왼손, 오른손 각각 한손 아이템이 있는 경우 → 어느 손을 교체할지 물어봄
					std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
					new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
					co_await std::suspend_always();
					if (coAnswer.empty()) co_return;
					if (coAnswer == sysStr[49]) handDir = equipHandFlag::left;
					else handDir = equipHandFlag::right;
				}

				std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
				//해당 손 아이템 떨구기
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
			}

			int returnIndex = targetPocket->transferItem(equipPtr, targetPocketCursor, 1);
			std::wstring logStr = replaceStr(sysStr[331], L"(%item)", equipPtr->itemInfo[returnIndex].name);
			updateLog(logStr);
			equipPtr->itemInfo[returnIndex].equipState = handDir;
			equipPtr->sortEquip();
		}
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].lightPtr != nullptr)
			{
				equipPtr->itemInfo[i].lightPtr.get()->setGrid(PlayerX(), PlayerY(), PlayerZ());
			}
		}
		PlayerPtr->pullEquipLights();
		PlayerPtr->updateStatus();
		co_return;
	}

	Corouter executeThrowing(ItemPocket* inputPocket, int inputIndex)//던지기
	{
		new CoordSelect(sysStr[131]);
		rangeRay = true;

		co_await std::suspend_always();

		if (coAnswer.empty())
		{
			updateLog(sysStr[330]);
			rangeRay = false;
			co_return;
		}

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		if (targetX == PlayerX() && targetY == PlayerY() && targetZ == PlayerZ());
		else PlayerPtr->setDirection(getIntDegree(PlayerX(), PlayerY(), targetX, targetY));

		prt(L"executeThrowing에서 사용한 좌표의 값은 (%d,%d,%d)이다.\n", targetX, targetY, targetZ);

		std::unique_ptr<ItemPocket> throwing = std::make_unique<ItemPocket>(storageType::null);
		std::wstring logStr = replaceStr(L"You throw the (%item).", L"(%item)", inputPocket->itemInfo[inputIndex].name);
		updateLog(logStr);
		inputPocket->transferItem(throwing.get(), inputIndex, 1);
		PlayerPtr->throwing(std::move(throwing), targetX, targetY);
		PlayerPtr->updateStatus();

		rangeRay = false;
	}

	void executeEquip(ItemPocket* sourcePocket, int sourceIndex)
	{
		updateLog(replaceStr(sysStr[125], L"(%item)", sourcePocket->itemInfo[sourceIndex].name)); // (%item)를(을) 장착했다.

		ItemPocket* equipPtr = PlayerEquip();
		int returnIndex = sourcePocket->transferItem(equipPtr, sourceIndex, 1);
		equipPtr->itemInfo[returnIndex].equipState = equipHandFlag::normal;

		PlayerPtr->pullEquipLights();
		PlayerPtr->updateStatus();
	}

	Corouter executePlant(ItemPocket* tgtPocket,int tgtIndex)
	{
		GUI::deactDrawAll();
		std::vector<Point2> selectableTile;
		for (int dir = 0; dir < 8; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);
			if (TileFloor({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }) == itemID::farmland && TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr)
			{
				selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
			}
		}

		ItemData& tgtItem = tgtPocket->itemInfo[tgtIndex];
		rangeSet.clear();
		for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });
		if (rangeSet.size() == 0)
		{
			updateLog(L"No suitable farmland nearby.");
			GUI::actDrawAll();
			co_return;
		}

		int tgtItemCode = tgtPocket->itemInfo[tgtIndex].itemCode;

		new CoordSelectCraft(tgtItem.propInstallCode, sysStr[299], selectableTile);//조합할 아이템을 설치할 위치를 선택해주세요.
		co_await std::suspend_always();
		rangeSet.clear();
		GUI::actDrawAll();

		if (coAnswer.empty() == false)
		{
			std::wstring targetStr = coAnswer;
			int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);
			int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);

			int targetItemCode = wtoi(targetStr.c_str());
			Point3 buildLocation = { targetX,targetY,PlayerZ() };
			createProp(buildLocation, targetItemCode);

			ItemData& propItem = TileProp(targetX, targetY, PlayerZ())->leadItem;

			tgtPocket->subtractItemIndex(tgtIndex, 1);
			PlayerPtr->updateStatus();

			updateLog(replaceStr(sysStr[329], L"(%item)", itemDex[targetItemCode].name));
		}
		else co_return;
	}

	//염색 앰플 사용 : 2단계 선택으로 염색 대상/색상을 결정
	// 1단계 - 대상 : 머리카락 / 털 / 뿔 (해당 부위가 존재할 때만 노출)
	// 2단계 - 색상 : 현재는 '털' 대상에 한해 GRAY/WHITE 2종만 지원
	// 취소 시 앰플은 소모되지 않는다.
	Corouter executeDye(ItemPocket* tgtPocket, int tgtIndex)
	{
		errorBox(tgtPocket == nullptr, L"executeDye: tgtPocket is nullptr.");
		errorBox(tgtIndex < 0 || tgtIndex >= (int)tgtPocket->itemInfo.size(), L"executeDye: tgtIndex out of bounds.");

		// 1단계 : 플레이어의 현재 외형 상태를 조회해 가능한 선택지 목록 구성
		enum class dyeTarget { hair, fur, horn };

		bool hasHair = PlayerInfo().hair != humanCustom::hair::null;
		bool hasFur = false;
		bool hasHorn = false;
		for (const SkillData& sd : PlayerInfo().skillList)
		{
			if (sd.isLearned == false) continue;
			SkillBehavior* bhv = SkillRegistry::get(sd.skillCode);
			if (bhv == nullptr) continue;
			if (bhv->src != skillSrc::MUTATION) continue;
			if (bhv->mutColorSrc == mutColorSource::fur) hasFur = true;
			else if (bhv->mutColorSrc == mutColorSource::horn) hasHorn = true;
		}

		std::vector<std::wstring> targetLabels;
		std::vector<dyeTarget> targetMap;
		if (hasHair) { targetLabels.push_back(L"Dye hair"); targetMap.push_back(dyeTarget::hair); }
		if (hasFur)  { targetLabels.push_back(L"Dye fur");  targetMap.push_back(dyeTarget::fur); }
		if (hasHorn) { targetLabels.push_back(L"Dye horn"); targetMap.push_back(dyeTarget::horn); }

		if (targetLabels.empty())
		{
			updateLog(L"There is nothing you can dye.");
			co_return;
		}

		new Lst(L"Dye Ampoule", L"Select what to dye.", targetLabels);
		co_await std::suspend_always();
		if (coAnswer.empty()) co_return;

		int targetSel = wtoi(coAnswer.c_str());
		if (targetSel < 0 || targetSel >= (int)targetMap.size()) co_return;
		dyeTarget selected = targetMap[targetSel];

		// 2단계 : 부위별 색상 선택. 현재는 털만 실제 염색을 지원한다.
		// (머리카락/뿔은 후속 작업 예정이며 선택지만 표시 후 즉시 종료)
		if (selected != dyeTarget::fur)
		{
			co_return;
		}

		// 털 색상 선택지 : palette/fur.tsv의 헤더명(GRAY/WHITE)과 1:1 매칭
		std::vector<LstExOption> colorOptions = {
			{ 560, L"Gray",  L"" },
			{ 561, L"White", L"" },
		};

		new LstEx(L"Dye Fur", L"Select a fur color.", colorOptions, spr::itemset, false);
		co_await std::suspend_always();
		if (coAnswer.empty()) co_return;

		int colorSel = wtoi(coAnswer.c_str());
		std::wstring newColor;
		if (colorSel == 0) newColor = L"GRAY";
		else if (colorSel == 1) newColor = L"WHITE";
		else co_return;

		PlayerInfo().furColor = newColor;
		tgtPocket->subtractItemIndex(tgtIndex, 1);
		PlayerPtr->updateStatus();
		updateLog(L"You dye your fur.");
	}

	void extractSeed(actEnv envType, ItemPocket* tgtPocket, int tgtIndex, int pocketMaxVolume)
	{
		int fruitCode = tgtPocket->itemInfo[tgtIndex].itemCode;
		int seedCode = 0;
		int seedAmount = 0;

		if (fruitCode == itemID::tomato)
		{
			seedCode = itemID::tomatoSeed;
			seedAmount = 3;
		}
		else if (fruitCode == itemID::watermelon)
		{
			seedCode = itemID::watermelonSeed;
			seedAmount = 5;
		}

		if (seedCode == 0) return;

		//과일 1개 소모
		tgtPocket->subtractItemIndex(tgtIndex, 1);

		//씨앗 배치 결정
		bool dropToGround = true;

		if (envType == actEnv::Inventory && pocketMaxVolume > 0)
		{
			int seedVolume = itemDex[seedCode].originalVolume * seedAmount;
			int currentVolume = tgtPocket->getPocketVolume();
			if (currentVolume + seedVolume <= pocketMaxVolume)
			{
				tgtPocket->addItemFromDex(seedCode, seedAmount);
				dropToGround = false;
			}
		}

		if (dropToGround)
		{
			Point3 dropPos = { PlayerX(), PlayerY(), PlayerZ() };
			if (TileItemStack(dropPos) == nullptr)
			{
				createItemStack(dropPos, { {seedCode, seedAmount} });
			}
			else
			{
				TileItemStack(dropPos)->getPocket()->addItemFromDex(seedCode, seedAmount);
			}
			addAniToPlayerTurn(TileItemStack(dropPos), aniFlag::drop);
		}

		PlayerPtr->updateStatus();
		updateLog(replaceStr(sysStr[351], L"(%item)", itemDex[fruitCode].name)); //(%item)에서 씨앗을 추출했다.
	}
}
