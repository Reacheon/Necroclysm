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

	//바닥에 아이템을 떨구고 나무 벌목처럼 통통 튀는 드랍 애니메이션(aniFlag::drop)을 건다
	void dropWithBounce(int tgtX, int tgtY, int tgtZ, int itemCode, int number)
	{
		addItemToTile({ tgtX, tgtY, tgtZ }, itemCode, number);
		ItemStack* dropped = TileItemStack(tgtX, tgtY, tgtZ);
		if (dropped != nullptr) addAniToPlayerTurn(dropped, aniFlag::drop);
	}

	//창문 닫기 — 열린 창문 유리를 닫음(통행 차단). 유리는 원래 시야 통과라 시야 변화 없음
	void closeWindow(int tgtX, int tgtY, int tgtZ)
	{
		Prop* tgtProp = TileProp(tgtX, tgtY, tgtZ);
		if (tgtProp == nullptr) return;
		ItemData& w = tgtProp->leadItem;
		w.eraseFlag(itemFlag::WINDOW_OPEN);
		w.eraseFlag(itemFlag::PROP_WALKABLE);
		PlayerPtr->updateVision(PlayerInfo().eyeSight);
	}

	//커튼 닫기 — 커튼을 닫으면 안쪽 창문도 닫힘(폐쇄)으로 간주, 커튼이 시야 차단
	void closeCurtain(int tgtX, int tgtY, int tgtZ)
	{
		Prop* tgtProp = TileProp(tgtX, tgtY, tgtZ);
		if (tgtProp == nullptr) return;
		ItemData& w = tgtProp->leadItem;
		if (w.checkFlag(itemFlag::CURTAIN) == false) return;
		w.eraseFlag(itemFlag::CURTAIN_OPEN);
		w.eraseFlag(itemFlag::WINDOW_OPEN);
		w.eraseFlag(itemFlag::PROP_WALKABLE);
		w.addFlag(itemFlag::PROP_BLOCKER);
		PlayerPtr->updateVision(PlayerInfo().eyeSight);
	}

	//커튼 뜯기 — 커튼만 제거(창문 개폐 상태는 보존). 테스트 아이템 드롭(천 아이템 생기면 교체)
	void tearCurtain(int tgtX, int tgtY, int tgtZ)
	{
		Prop* tgtProp = TileProp(tgtX, tgtY, tgtZ);
		if (tgtProp == nullptr) return;
		ItemData& w = tgtProp->leadItem;
		if (w.checkFlag(itemFlag::CURTAIN) == false) return;
		w.eraseFlag(itemFlag::CURTAIN);
		w.eraseFlag(itemFlag::CURTAIN_OPEN);
		w.eraseFlag(itemFlag::PROP_BLOCKER); //커튼이 사라졌으니 시야 차단 해제
		dropWithBounce(tgtX, tgtY, tgtZ, itemID::test, 1);
		PlayerPtr->updateVision(PlayerInfo().eyeSight);
	}

	//창문 깨트리기 — 커튼이 있으면 자동 해체(드롭) 후 파손. 깨진 창문은 통행 가능
	void breakWindow(int tgtX, int tgtY, int tgtZ)
	{
		Prop* tgtProp = TileProp(tgtX, tgtY, tgtZ);
		if (tgtProp == nullptr) return;
		ItemData& w = tgtProp->leadItem;
		//2단계 파손: 멀쩡 → 깨진 창문(149) → 창틀만(150). 창틀이면 더 못 부숨
		if (w.checkFlag(itemFlag::WINDOW) == false || w.checkFlag(itemFlag::WINDOW_FRAME)) return;

		if (w.checkFlag(itemFlag::WINDOW_BROKEN))
		{
			//2회차: 깨진 창문 → 창틀만 남김
			w.eraseFlag(itemFlag::WINDOW_BROKEN);
			w.addFlag(itemFlag::WINDOW_FRAME);
		}
		else
		{
			//1회차: 멀쩡한 창문 → 깨진 창문(커튼 자동 해체). 깨진 시점부터 통행 가능
			w.eraseFlag(itemFlag::CURTAIN);
			w.eraseFlag(itemFlag::CURTAIN_OPEN);
			w.eraseFlag(itemFlag::WINDOW_OPEN);
			w.addFlag(itemFlag::WINDOW_BROKEN);
			w.eraseFlag(itemFlag::PROP_BLOCKER);
			w.addFlag(itemFlag::PROP_WALKABLE);
		}
		dropWithBounce(tgtX, tgtY, tgtZ, itemID::test, 1); //임시: 부서진 잔해(천/유리/창틀). 천 아이템 생기면 분기
		PlayerPtr->updateVision(PlayerInfo().eyeSight);
	}

	//롤업도어 한 칸의 개폐 상태 설정 — 나무문 closeDoor 패턴(가스 ON/OFF 전환 포함). 열린 문은 PROP_DEPTH_UPPER로 엔티티/차량 위에 그려짐
	void setRollupDoorState(int tgtX, int tgtY, int tgtZ, bool open)
	{
		Prop* tgtProp = TileProp(tgtX, tgtY, tgtZ);
		if (tgtProp == nullptr) return;
		ItemData& d = tgtProp->leadItem;
		if (d.checkFlag(itemFlag::ROLLUP_DOOR) == false) return;
		if (open)
		{
			//PROP_DEPTH_UPPER: 렌더 최상단 + 차량 충돌 통과(Vehicle::colisionCheck). 닫힘은 DEPTH 플래그 없음 = 엔티티 깊이 + 차량 차단
			d.addFlag(itemFlag::ROLLUP_DOOR_OPEN);
			d.addFlag(itemFlag::PROP_WALKABLE);
			d.addFlag(itemFlag::PROP_DEPTH_UPPER);
			d.eraseFlag(itemFlag::PROP_BLOCKER);
			if (d.checkFlag(itemFlag::PROP_GAS_OBSTACLE_ON))
			{
				d.eraseFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
				d.addFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
			}
		}
		else
		{
			d.eraseFlag(itemFlag::ROLLUP_DOOR_OPEN);
			d.eraseFlag(itemFlag::PROP_WALKABLE);
			d.eraseFlag(itemFlag::PROP_DEPTH_UPPER);
			d.addFlag(itemFlag::PROP_BLOCKER);
			if (d.checkFlag(itemFlag::PROP_GAS_OBSTACLE_OFF))
			{
				d.eraseFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
				d.addFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
			}
		}
	}

	void toggleRollupDoors(Point3 winchPos)
	{
		std::unordered_set<Point3, Point3::Hash> visitedSet;
		bool anyOpened = false, anyClosed = false, anyBlocked = false;
		for (int dir = 0; dir < 8; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);
			Point3 seed = { winchPos.x + dx, winchPos.y + dy, winchPos.z };
			Prop* seedProp = TileProp(seed.x, seed.y, seed.z);
			if (seedProp == nullptr || seedProp->leadItem.checkFlag(itemFlag::ROLLUP_DOOR) == false) continue;
			if (visitedSet.find(seed) != visitedSet.end()) continue; //앞선 시드의 체인에 이미 포함 → 스킵(이중 토글 방지)

			//시드 문의 반전된 상태가 체인 전체를 결정(문별 개별 토글)
			bool newOpen = seedProp->leadItem.checkFlag(itemFlag::ROLLUP_DOOR_OPEN) == false;

			//1차: 체인 수집 BFS — H문은 좌우(±x), V문은 상하(±y)로 같은 itemCode인 문에만 전파
			std::vector<Point3> chainTiles;
			std::queue<Point3> frontierQueue;
			frontierQueue.push(seed);
			while (!frontierQueue.empty())
			{
				Point3 current = frontierQueue.front();
				frontierQueue.pop();
				if (visitedSet.find(current) != visitedSet.end()) continue;
				visitedSet.insert(current);
				chainTiles.push_back(current);
				int curCode = TileProp(current.x, current.y, current.z)->leadItem.itemCode;

				int cdx = (curCode == itemID::rollupDoorH) ? 1 : 0;
				int cdy = (curCode == itemID::rollupDoorV) ? 1 : 0;
				for (int sign = -1; sign <= 1; sign += 2)
				{
					Point3 next = { current.x + cdx * sign, current.y + cdy * sign, current.z };
					Prop* nextProp = TileProp(next.x, next.y, next.z);
					if (nextProp != nullptr && nextProp->leadItem.itemCode == curCode && visitedSet.find(next) == visitedSet.end()) frontierQueue.push(next);
				}
			}

			//닫기는 체인 전체가 비어 있어야 가능 — 한 칸이라도 엔티티/차량/아이템이 깔려 있으면 문이 끼므로 체인째 거부(문은 물리적으로 한 장)
			if (newOpen == false)
			{
				bool chainBlocked = false;
				for (const Point3& tile : chainTiles)
				{
					ItemStack* stackPtr = TileItemStack(tile.x, tile.y, tile.z);
					if (TileEntity(tile.x, tile.y, tile.z) != nullptr || TileVehicle(tile.x, tile.y, tile.z) != nullptr || (stackPtr != nullptr && stackPtr->getPocket()->itemInfo.empty() == false))
					{
						chainBlocked = true;
						break;
					}
				}
				if (chainBlocked)
				{
					anyBlocked = true;
					continue;
				}
			}

			//2차: 체인 전체에 새 상태 적용
			for (const Point3& tile : chainTiles) setRollupDoorState(tile.x, tile.y, tile.z, newOpen);
			if (newOpen) anyOpened = true;
			else anyClosed = true;
		}
		if (anyOpened) updateLog(sysStr[315]);
		if (anyClosed) updateLog(sysStr[316]);
		if (anyBlocked) updateLog(sysStr[317]);
		if (anyOpened || anyClosed) PlayerPtr->updateVision(PlayerInfo().eyeSight);
	}

	void closeVDoor(int tgtX, int tgtY, int tgtZ)
	{
		ItemPocket* tgtPocket = TileVehicle(tgtX, tgtY, PlayerZ())->partInfo[{tgtX, tgtY, PlayerZ() }].get();
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
				tgtPocket->itemInfo[i].vehSprIndex -= 16;
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
						updateLog(sysStr[318]);
					}
					else updateLog(sysStr[319]);
				}
				else updateLog(sysStr[320]);
			}
			else if (inputItem.checkFlag(itemFlag::TOGGLE_ON))
			{
				inputItem.eraseFlag(itemFlag::TOGGLE_ON);
				inputItem.addFlag(itemFlag::TOGGLE_OFF);
				inputItem.lightPtr.reset();
				inputItem.itemSprIndex -= 1;
				PlayerPtr->updateVision();
				updateLog(sysStr[321]);
			}
		}
	}

	void setWireVisibility(Point3 tgtPoint, bool hide)
	{
		errorBox(TileProp(tgtPoint) == nullptr, L"actFunc::setWireVisibility: Start point가 널포인터이다.");
		errorBox(TileProp(tgtPoint)->leadItem.checkFlag(itemFlag::CIRCUIT) == false, L"actFunc::setWireVisibility: CIRCUIT 플래그가 없는데 setWireVisibility가 실행되었다.");

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
		if (dir < 0) return sysStr[182];
		return labels[dir];
	}

	//플레이어 주변에서 CBM 아이템을 수집해서 LstEx 옵션으로 반환
	std::vector<AutodocPocketSource> gatherNearbyPocketsAutodoc()
	{
		std::vector<AutodocPocketSource> result;
		ItemPocket* equipPtr = PlayerEquip();

		//1. 장비 포켓 + 서브포켓
		result.push_back({ equipPtr, sysStr[242] });
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
			replaceStr(sysStr[442], L"(%state)", col2Str(SDL_Color{ 0x00,0xFF,0x80 }) + sysStr[443]),
			sysStr[444],
			{ sysStr[445], sysStr[446] }
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
				updateLog(sysStr[322]);
				co_return;
			}

			new LstEx(sysStr[327], sysStr[328], cbmList, spr::itemset);
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
								updateLog(L"[디버그] 잘못된 바이오닉 모듈을 선택했다.");
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
									updateLog(replaceStr(replaceStr(sysStr[323], L"(%skill)", behavior->name), L"(%number)", std::to_wstring(it->skillLevel)));
								}
								else
								{
									updateLog(replaceStr(sysStr[324], L"(%skill)", behavior->name));
									co_return;
								}
							}
							else
							{
								//신규 설치
								PlayerPtr->addSkill(skillId);
								updateLog(replaceStr(sysStr[325], L"(%skill)", behavior->name));
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
			updateLog(sysStr[326]);
		}

		co_return;
	}
}
