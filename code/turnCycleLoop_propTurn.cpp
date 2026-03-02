module turnCycleLoop;

import constVar;
import globalVar;
import wrapFunc;
import Prop;
import World;

__int64 propTurn()
{
	debug::printCircuitLog = false;

	nextCircuitStartQueue = std::queue<Point3>();
	auto activePropSet = (World::ins())->getActivePropSet();
	std::unordered_set<Prop*> activeLoadSet;

	//액티브 전자회로 프롭셋 z축 확장
	{
		std::unordered_set<Prop*> frontier = activePropSet;
		while (!frontier.empty())
		{
			std::unordered_set<Prop*> newProps;
			for (auto pPtr : frontier)
			{
				if (pPtr->leadItem.checkFlag(itemFlag::CABLE) || pPtr->leadItem.checkFlag(itemFlag::CIRCUIT))
				{
					const dir16 dirs[] = { dir16::above, dir16::below, dir16::right, dir16::up, dir16::left, dir16::down };
					for (int i = 0; i < 6; i++)
					{
						if (pPtr->isCableLinked(pPtr, dirs[i]))
						{
							int dx, dy, dz;
							dirToXYZ(dirs[i], dx, dy, dz);
							Prop* neighbor = TileProp(pPtr->getGrid() + Point3{ dx, dy, dz });
							if (neighbor && activePropSet.find(neighbor) == activePropSet.end())
							{
								newProps.insert(neighbor);
								activePropSet.insert(neighbor);
							}
						}
					}
				}
			}
			frontier = newProps;
		}
	}

	//액티브 유체회로 프롭셋 z축 확장
	{
		std::unordered_set<Prop*> frontier = activePropSet;
		while (!frontier.empty())
		{
			std::unordered_set<Prop*> newProps;
			for (auto pPtr : frontier)
			{
				if (pPtr->leadItem.checkFlag(itemFlag::FLUID_CIRCUIT))
				{
					const dir16 dirs[] = { dir16::above, dir16::below, dir16::right, dir16::up, dir16::left, dir16::down };
					for (int i = 0; i < 6; i++)
					{
						if (pPtr->isPipeLinked(pPtr, dirs[i]))
						{
							int dx, dy, dz;
							dirToXYZ(dirs[i], dx, dy, dz);
							Prop* neighbor = TileProp(pPtr->getGrid() + Point3{ dx, dy, dz });
							if (neighbor && activePropSet.find(neighbor) == activePropSet.end())
							{
								newProps.insert(neighbor);
								activePropSet.insert(neighbor);
							}
						}
					}
				}
			}
			frontier = newProps;
		}
	}

	std::vector<Prop*> elecComponents;
	std::vector<Prop*> fluidComponents;

	for (auto pPtr : activePropSet)
	{
		//전자회로 변수 초기화
		if (pPtr->leadItem.checkFlag(itemFlag::CIRCUIT)) elecComponents.push_back(pPtr);
		pPtr->runUsed = false;
		pPtr->totalLossCharge = 0;
		pPtr->gndSink = 0;
		pPtr->gndSinkRight = 0;
		pPtr->gndSinkUp = 0;
		pPtr->gndSinkLeft = 0;
		pPtr->gndSinkDown = 0;
		pPtr->initChargeFlux();
		if (pPtr->hasGround()) activeLoadSet.insert(pPtr);
		else if (pPtr->leadItem.checkFlag(itemFlag::FORCE_LOAD)) activeLoadSet.insert(pPtr);

		//유체회로 변수 초기화
		if (pPtr->leadItem.checkFlag(itemFlag::FLUID_CIRCUIT)) fluidComponents.push_back(pPtr);
		pPtr->fluidRunUsed = false;
		pPtr->totalResistFluid = 0;
		pPtr->sinkFluidAmount = 0;
		pPtr->sinkFluidType = fluidType::NONE;
		pPtr->jetFluidType = fluidType::NONE;
		pPtr->jetFluidDir = dir16::none;
		pPtr->initFluidFlux();

		if (pPtr->leadItem.itemCode == itemID::wheatCrop)
			int a = 3;

		//농작물 성장
		if (pPtr->leadItem.checkFlag(itemFlag::CROP))
		{
			if (pPtr->leadItem.itemCode == itemID::riceCrop)//벼는 물에 잠겨야지만 성장
			{
				bool hasWaterPool = false;
				if (TileItemStack(pPtr->getGrid()) != nullptr)
				{
					std::vector<ItemData>& stackInfo = TileItemStack(pPtr->getGrid())->getPocket()->itemInfo;
					for (ItemData& item : stackInfo)
					{
						if (item.itemCode == itemID::water) hasWaterPool = true;
					}
				}

				if (TileFloor(pPtr->getGrid()) == itemID::farmland && hasWaterPool)
				{
					pPtr->plantGrowthPercent += 0.5;
					if (pPtr->plantGrowthPercent > 100.0) pPtr->plantGrowthPercent = 100.0;
				}
			}
			else //그 외 식물은 밭이 젖어있기만 하면 자동으로 성장
			{
				if (TileFloor(pPtr->getGrid()) == itemID::farmland && isWetTile(pPtr->getGrid()))
				{
					pPtr->plantGrowthPercent += 0.5;
					if (pPtr->plantGrowthPercent > 100.0) pPtr->plantGrowthPercent = 100.0;
				}
			}
		}
	}

	int loopCount = 0;
	reserveDelayInit.clear();
	do
	{
		loopCount++;
		if (loopCount >= MAX_CIRCUIT_LOOP_COUNT) break;
		if (debug::printCircuitLog) std::wprintf(L"▼전자회로 루프 카운트: %d\n", loopCount);

		if (nextCircuitStartQueue.empty() == false)
		{
			Point3 tgtCoord = nextCircuitStartQueue.front();
			nextCircuitStartQueue.pop();
			Prop* tgtProp = TileProp(tgtCoord);
			if (tgtProp) tgtProp->updateCircuitNetwork();
		}
		else for (auto pPtr : elecComponents)
		{
			if (pPtr->runUsed) continue;
			if (pPtr->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
			{
				pPtr->updateCircuitNetwork();
			}
		}

		//==============================================================================
		// 전자회로 연산 끝난 후의 부하 부품들 전력 소모
		//==============================================================================

		for (auto pPtr : activeLoadSet)
		{
			Prop* loadProp = pPtr;
			pPtr->loadAct();
		}

	} while (nextCircuitStartQueue.empty() == false);


	for (auto tgtDelay : reserveDelayInit) tgtDelay->delayStartTurn = 0.0;

	//==============================================================================
	// 유체회로 연산 (전자회로 이후 실행 - 펌프 ON/OFF가 확정된 상태)
	//==============================================================================

	// 펌프에서 유체회로 탐색 시작 (runUsed가 아닌 fluidRunUsed 사용)
	for (auto pPtr : fluidComponents)
	{
		if (pPtr->fluidRunUsed) continue;

		pPtr->updateFluidCircuitNetwork();
	}

	return 0;
}