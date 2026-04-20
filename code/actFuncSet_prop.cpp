module actFuncSet;

import util;
import constVar;
import globalVar;
import World;
import Player;
import log;
import Prop;
import Vehicle;
import Lst;
import LstEx;
import ItemStack;
import textureVar;
import SkillBehavior;
import SkillRegistry;

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
		PlayerPtr->updateVision(PlayerInfo().eyeSight);
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
				PlayerPtr->updateVision(PlayerInfo().eyeSight);
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

	//──────────────────────────────────────────────────────────────
	//  오토닥 인터페이스
	//──────────────────────────────────────────────────────────────

	//서브포켓을 재귀적으로 수집 (출처 = 컨테이너 아이템 이름)
	struct AutodocPocketSource
	{
		ItemPocket* pocket;
		std::wstring source;
	};

	void collectSubPocketsAutodoc(ItemPocket* pocket, std::vector<AutodocPocketSource>& outVec)
	{
		for (auto& item : pocket->itemInfo)
		{
			if (item.pocketPtr != nullptr)
			{
				outVec.push_back({ item.pocketPtr.get(), item.name });
				collectSubPocketsAutodoc(item.pocketPtr.get(), outVec);
			}
		}
	}

	std::wstring autodocDirLabel(int dir)
	{
		static const std::wstring labels[] = { L"E", L"NE", L"N", L"NW", L"W", L"SW", L"S", L"SE" };
		if (dir < 0) return L"Floor";
		return labels[dir];
	}

	//플레이어 주변에서 CBM 아이템을 수집해서 LstEx 옵션으로 반환
	std::vector<AutodocPocketSource> gatherNearbyPocketsAutodoc()
	{
		std::vector<AutodocPocketSource> result;
		ItemPocket* equipPtr = PlayerEquip();

		//1. 장비 포켓 + 서브포켓
		result.push_back({ equipPtr, L"Equip" });
		collectSubPocketsAutodoc(equipPtr, result);

		//2. 주변 9타일 순회
		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);
			int x = PlayerX() + dx;
			int y = PlayerY() + dy;
			int z = PlayerZ();

			std::wstring tileLabel = autodocDirLabel(dir);

			//바닥 ItemStack + 서브포켓
			ItemStack* stack = TileItemStack(x, y, z);
			if (stack != nullptr)
			{
				result.push_back({ stack->getPocket(), tileLabel });
				collectSubPocketsAutodoc(stack->getPocket(), result);
			}

			//Prop 내부 포켓 + 서브포켓
			Prop* prop = TileProp(x, y, z);
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				std::wstring propLabel = prop->leadItem.name;
				if (dir >= 0) propLabel += L" (" + tileLabel + L")";
				result.push_back({ prop->leadItem.pocketPtr.get(), propLabel });
				collectSubPocketsAutodoc(prop->leadItem.pocketPtr.get(), result);
			}
		}

		return result;
	}

	Corouter useAutodoc(int tgtX, int tgtY, int tgtZ)
	{
		//메인 메뉴 표시
		new Lst(
			L"AUTODOC #00FF80[ONLINE]",
			L"Bionic implant procedure standing by...",
			{ L"Implant Bionic", L"Extract Bionic" }
		);
		co_await std::suspend_always();
		if (coAnswer.empty()) co_return;

		int selected = wtoi(coAnswer.c_str());

		if (selected == 0) //Implant Bionic
		{
			//주변에서 CBM 아이템 수집
			std::vector<AutodocPocketSource> pockets = gatherNearbyPocketsAutodoc();

			std::vector<LstExOption> cbmList;
			for (int j = 0; j < pockets.size(); j++)
			{
				for (int i = 0; i < pockets[j].pocket->itemInfo.size(); i++)
				{
					ItemData& item = pockets[j].pocket->itemInfo[i];
					if (item.subcategory == itemSubcategory::tech_bionics)
					{
						cbmList.push_back({ item.getSprIndex(), item.name, pockets[j].source });
					}
				}
			}

			if (cbmList.size() == 0)
			{
				updateLog(L"No bionic modules found nearby.");
				co_return;
			}

			new LstEx(L"Implant Bionic", L"Select a CBM to implant.", cbmList, spr::itemset);
			co_await std::suspend_always();

			if (coAnswer.empty()) co_return;

			//선택된 CBM 찾기
			int counter = 0;
			for (int j = 0; j < pockets.size(); j++)
			{
				for (int i = 0; i < pockets[j].pocket->itemInfo.size(); i++)
				{
					ItemData& item = pockets[j].pocket->itemInfo[i];
					if (item.subcategory == itemSubcategory::tech_bionics)
					{
						if (counter == wtoi(coAnswer.c_str()))
						{
							const std::wstring& skillId = item.bionicId;
							auto* behavior = SkillRegistry::get(skillId);
							if (!behavior)
							{
								updateLog(L"Error: Invalid bionic module.");
								co_return;
							}

							//중복 설치 체크 - skillList에서 동일 ID 검색
							auto& skillList = PlayerInfo().skillList;
							auto it = std::find_if(skillList.begin(), skillList.end(),
								[&skillId](const SkillData& sd) { return sd.skillId == skillId; });

							if (it != skillList.end())
							{
								//중첩 설치 가능한 바이오닉
								if (skillId == L"BION_POWER_STORAGE")
								{
									it->skillLevel++;
									updateLog(std::format(L"The autodoc installs an additional {}. (x{})", behavior->name, it->skillLevel));
								}
								else
								{
									updateLog(std::format(L"You already have {} installed.", behavior->name));
									co_return;
								}
							}
							else
							{
								//신규 설치
								PlayerPtr->addSkill(skillId);
								updateLog(std::format(L"The autodoc successfully installs {}.", behavior->name));
							}

							//Power Storage 설치 시 maxEnergy & energy 증가
							if (skillId == L"BION_POWER_STORAGE")
							{
								PlayerInfo().maxEnergy += 500;
								PlayerInfo().energy += 500;
							}

							//CBM 아이템 소모
							pockets[j].pocket->subtractItemIndex(i, 1);
							co_return;
						}
						counter++;
					}
				}
			}
		}
		else if (selected == 1) //Extract Bionic
		{
			//TODO: 바이오닉 추출 로직
			updateLog(L"The autodoc begins the bionic extraction procedure...");
		}

		co_return;
	}
}
