module actFuncSet;

import util;
import constVar;
import globalVar;
import wrapFunc;
import log;
import Prop;
import Vehicle;

namespace actFunc
{
	void closeDoor(int tgtX, int tgtY, int tgtZ)
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

	void closeVDoor(int tgtX, int tgtY, int tgtZ)
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

	void toggle(ItemData& inputItem)
	{
		if (inputItem.itemCode == itemID::minerHelmet)
		{
			if (inputItem.checkFlag(itemFlag::TOGGLE_OFF))
			{
				if (inputItem.pocketPtr != nullptr && inputItem.pocketPtr->itemInfo.size() == 1)
				{
					if (inputItem.pocketPtr->itemInfo[0].powerStorage >= inputItem.gndUsePower)
					{
						inputItem.eraseFlag(itemFlag::TOGGLE_OFF);
						inputItem.addFlag(itemFlag::TOGGLE_ON);
						inputItem.lightPtr = std::make_unique<Light>(PlayerX(), PlayerY(), PlayerZ(), 8, 110, SDL_Color{ 150, 150, 250 });
						inputItem.itemSprIndex += 1;
						PlayerPtr->updateVision();
						updateLog(L"The headlamp comes on.");
					}
					else updateLog(L"The inserted battery is depleted.");
				}
				else updateLog(L"No battery inserted in the headlamp.");
			}
			else if (inputItem.checkFlag(itemFlag::TOGGLE_ON))
			{
				inputItem.eraseFlag(itemFlag::TOGGLE_ON);
				inputItem.addFlag(itemFlag::TOGGLE_OFF);
				inputItem.lightPtr.reset();
				inputItem.itemSprIndex -= 1;
				PlayerPtr->updateVision();
				updateLog(L"The headlamp goes off.");
			}
		}
	}

	void setWireVisibility(Point3 tgtPoint, bool hide)
	{
		errorBox(TileProp(tgtPoint) == nullptr, L"actFunc::setWireVisibility: Start point is nullptr.");
		errorBox(TileProp(tgtPoint)->leadItem.checkFlag(itemFlag::CIRCUIT) == false, L"actFunc::setWireVisibility: Start point is missing the CIRCUIT flag.");

		//BFS로 연결된 회로 네트워크 전체를 순회
		std::queue<Point3> frontierQueue;
		std::unordered_set<Point3, Point3::Hash> visitedSet;
		frontierQueue.push(tgtPoint);
		while (!frontierQueue.empty())
		{
			Point3 current = frontierQueue.front();
			frontierQueue.pop();
			if (visitedSet.find(current) != visitedSet.end()) continue;
			visitedSet.insert(current);
			Prop* tgtProp = TileProp(current.x, current.y, current.z);

			if (hide) tgtProp->leadItem.addFlag(itemFlag::HIDE_WIRE);
			else tgtProp->leadItem.eraseFlag(itemFlag::HIDE_WIRE);

			//6방향 (상하좌우 + 위층/아래층) 인접 회로 탐색
			const dir16 directions[] = { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below };
			for (int i = 0; i < 6; ++i)
			{
				int dx, dy, dz;
				dirToXYZ(directions[i], dx, dy, dz);
				Point3 nextCoord = { current.x + dx, current.y + dy, current.z + dz };
				Prop* nextProp = TileProp(nextCoord.x, nextCoord.y, nextCoord.z);
				if (nextProp != nullptr)
				{
					ItemData& nextItem = nextProp->leadItem;
					if (nextItem.checkFlag(itemFlag::CIRCUIT))
					{
						if (visitedSet.find(nextCoord) == visitedSet.end())
						{
							frontierQueue.push(nextCoord);
						}
					}
				}
			}
		}
	}

	void hideWire(Point3 tgtPoint) { setWireVisibility(tgtPoint, true); }
	void showWire(Point3 tgtPoint) { setWireVisibility(tgtPoint, false); }
}
