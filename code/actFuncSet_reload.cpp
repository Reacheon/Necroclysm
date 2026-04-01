module actFuncSet;

import util;
import constVar;
import globalVar;
import World;
import Player;
import ItemStack;
import log;
import Prop;
import Vehicle;
import LstEx;
import textureVar;
import turnWait;

namespace
{
	//포켓 내부의 모든 서브포켓을 재귀적으로 수집한다 (가방 안의 가방 등)
	void collectSubPockets(ItemPocket* pocket, std::vector<ItemPocket*>& outVec)
	{
		for (auto& item : pocket->itemInfo)
		{
			if (item.pocketPtr != nullptr)
			{
				outVec.push_back(item.pocketPtr.get());
				collectSubPockets(item.pocketPtr.get(), outVec);
			}
		}
	}

	//플레이어 주변에서 접근 가능한 모든 ItemPocket을 수집한다.
	//탐색 대상: 장비 + 장비 서브포켓(가방 등), 주변 9타일의 바닥/프롭/차량
	std::vector<ItemPocket*> gatherNearbyPockets()
	{
		std::vector<ItemPocket*> result;
		ItemPocket* equipPtr = PlayerEquip();

		//1. 장비 포켓 + 장비 서브포켓 (가방, 파우치 등)
		result.push_back(equipPtr);
		collectSubPockets(equipPtr, result);

		//2. 주변 9타일 순회 (현재 타일 + 인접 8타일)
		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);
			int x = PlayerX() + dx;
			int y = PlayerY() + dy;
			int z = PlayerZ();

			//2a. 바닥 ItemStack + 서브포켓
			ItemStack* stack = TileItemStack(x, y, z);
			if (stack != nullptr)
			{
				result.push_back(stack->getPocket());
				collectSubPockets(stack->getPocket(), result);
			}

			//2b. Prop 내부 포켓 + 서브포켓
			Prop* prop = TileProp(x, y, z);
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				result.push_back(prop->leadItem.pocketPtr.get());
				collectSubPockets(prop->leadItem.pocketPtr.get(), result);
			}

			//2c. Vehicle 파트 중 POCKET 아이템의 포켓 + 서브포켓
			Vehicle* veh = TileVehicle(x, y, z);
			if (veh != nullptr)
			{
				auto it = veh->partInfo.find({ x, y });
				if (it != veh->partInfo.end())
				{
					for (auto& item : it->second->itemInfo)
					{
						if (item.checkFlag(itemFlag::POCKET) && item.pocketPtr != nullptr)
						{
							result.push_back(item.pocketPtr.get());
							collectSubPockets(item.pocketPtr.get(), result);
						}
					}
				}
			}
		}

		return result;
	}

	//──────────────────────────────────────────────────────────────
	//  출처 정보 포함 포켓 수집 (LstEx용)
	//──────────────────────────────────────────────────────────────
	struct PocketSource
	{
		ItemPocket* pocket;
		std::wstring source; //이 포켓의 출처 표시 문자열
	};

	//방향 인덱스를 출처 라벨로 변환
	std::wstring dirToSourceLabel(int dir)
	{
		static const std::wstring labels[] = { L"E", L"NE", L"N", L"NW", L"W", L"SW", L"S", L"SE" };
		if (dir < 0) return L"Floor";
		return labels[dir];
	}

	//서브포켓을 재귀적으로 수집 (출처 = 컨테이너 아이템 이름)
	void collectSubPocketsWithSource(ItemPocket* pocket, std::vector<PocketSource>& outVec)
	{
		for (auto& item : pocket->itemInfo)
		{
			if (item.pocketPtr != nullptr)
			{
				outVec.push_back({ item.pocketPtr.get(), item.name });
				collectSubPocketsWithSource(item.pocketPtr.get(), outVec);
			}
		}
	}

	//플레이어 주변에서 접근 가능한 모든 ItemPocket을 출처 정보와 함께 수집한다.
	std::vector<PocketSource> gatherNearbyPocketsWithSource()
	{
		std::vector<PocketSource> result;
		ItemPocket* equipPtr = PlayerEquip();

		//1. 장비 포켓 + 장비 서브포켓
		result.push_back({ equipPtr, L"Equip" });
		collectSubPocketsWithSource(equipPtr, result);

		//2. 주변 9타일 순회
		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);
			int x = PlayerX() + dx;
			int y = PlayerY() + dy;
			int z = PlayerZ();

			std::wstring tileLabel = dirToSourceLabel(dir);

			//2a. 바닥 ItemStack + 서브포켓
			ItemStack* stack = TileItemStack(x, y, z);
			if (stack != nullptr)
			{
				result.push_back({ stack->getPocket(), tileLabel });
				collectSubPocketsWithSource(stack->getPocket(), result);
			}

			//2b. Prop 내부 포켓 + 서브포켓
			Prop* prop = TileProp(x, y, z);
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				std::wstring propLabel = prop->leadItem.name;
				if (dir >= 0) propLabel += L" (" + tileLabel + L")";
				result.push_back({ prop->leadItem.pocketPtr.get(), propLabel });
				collectSubPocketsWithSource(prop->leadItem.pocketPtr.get(), result);
			}

			//2c. Vehicle 파트 중 POCKET 아이템의 포켓 + 서브포켓
			Vehicle* veh = TileVehicle(x, y, z);
			if (veh != nullptr)
			{
				auto it = veh->partInfo.find({ x, y });
				if (it != veh->partInfo.end())
				{
					for (auto& item : it->second->itemInfo)
					{
						if (item.checkFlag(itemFlag::POCKET) && item.pocketPtr != nullptr)
						{
							std::wstring vehLabel = item.name;
							if (dir >= 0) vehLabel += L" (" + tileLabel + L")";
							result.push_back({ item.pocketPtr.get(), vehLabel });
							collectSubPocketsWithSource(item.pocketPtr.get(), result);
						}
					}
				}
			}
		}

		return result;
	}
}

namespace actFunc
{
	//장전 : 총이나 탄창에 사용, 자기 자신의 탄환을 채워넣음
	Corouter reloadSelf(actEnv envType, ItemPocket* reloadItemPocket, int reloadItemCursor)
	{
		prt(L"executeReloadSelf이 실행되었다.\n");
		int targetLootCursor = reloadItemCursor;
		std::vector<ItemData>& equipInfo = PlayerEquip()->itemInfo;

		//활/석궁은 장비 중인 화살통/볼트통에서만 장전
		if (reloadItemPocket->itemInfo[targetLootCursor].checkFlag(itemFlag::BOW))
		{
			for (int j = 0; j < equipInfo.size(); j++)
			{
				if (equipInfo[j].itemCode == itemID::arrowQuiver)
				{
					if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
					{
						equipInfo[j].pocketPtr.get()->transferItem(reloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get(), 0, 1);
						updateLog(sysStr[325]);//화살을 시위에 걸었다.
						co_return;
					}
				}
			}
		}
		else if (reloadItemPocket->itemInfo[targetLootCursor].checkFlag(itemFlag::CROSSBOW))
		{
			for (int j = 0; j < equipInfo.size(); j++)
			{
				if (equipInfo[j].itemCode == itemID::boltQuiver)
				{
					if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
					{
						equipInfo[j].pocketPtr.get()->transferItem(reloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get(), 0, 1);
						updateLog(sysStr[326]);//볼트를 장전했다.
						co_return;
					}
				}
			}
		}

		//주변 접근 가능한 모든 포켓 수집 (출처 정보 포함)
		std::vector<PocketSource> targetSearchPtr = gatherNearbyPocketsWithSource();
		//재장전 대상의 내부 포켓 (이미 장전된 탄환이 자기 자신의 후보로 표시되는 것을 방지)
		ItemPocket* selfPocket = reloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get();

		//LstEx 옵션 리스트 구축
		std::vector<LstExOption> bulletList;
		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			if (targetSearchPtr[j].pocket == selfPocket) continue;

			for (int i = 0; i < targetSearchPtr[j].pocket->itemInfo.size(); i++)
			{
				ItemData& item = targetSearchPtr[j].pocket->itemInfo[i];
				if (std::find(reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.begin(), reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end(), item.itemCode) != reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end())
				{
					bulletList.push_back({ item.getSprIndex(), item.name, targetSearchPtr[j].source });
				}
			}
		}

		if (bulletList.size() == 0)
		{
			updateLog(sysStr[96]);//이 아이템을 넣을만한 포켓이 없다.
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new LstEx(sysStr[95], sysStr[94], bulletList, spr::itemset);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////
		if (coAnswer.empty() == false)
		{
			int counter = 0;
			for (int j = 0; j < targetSearchPtr.size(); j++)
			{
				if (targetSearchPtr[j].pocket == selfPocket) continue;

				for (int i = 0; i < targetSearchPtr[j].pocket->itemInfo.size(); i++)
				{
					if (std::find(reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.begin(), reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end(), targetSearchPtr[j].pocket->itemInfo[i].itemCode) != reloadItemPocket->itemInfo[targetLootCursor].pocketOnlyItem.end())
					{
						if (counter == wtoi(coAnswer.c_str()))
						{
							targetSearchPtr[j].pocket->transferItem
							(
								selfPocket,
								i,
								1
							);

							co_return;
						}
						counter++;
					}
				}
			}
		}

	}

	//삽탄 : 총알에 사용, 이 탄환을 넣을 수 있는 탄창 리스트를 표시하고 거기에 넣음
	Corouter reloadOther(actEnv envType, ItemPocket* reloadItemPocket, int reloadItemCursor)
	{
		//탄창이 장착한 총에 바로 넣는 기능 추가? -> 탄창을 안 빼고 총알을 넣는게 현실적으로 가능할리가 없다
		prt(L"executeReloadOther이 실행되었다.\n");
		int targetLootCursor = reloadItemCursor;

		//주변 접근 가능한 모든 포켓 수집 (출처 정보 포함)
		std::vector<PocketSource> targetSearchPtr = gatherNearbyPocketsWithSource();

		//LstEx 옵션 리스트 구축 : 리로드 가능한 탄창/총기 수집
		std::vector<LstExOption> pocketList;
		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			std::vector<ItemData>& targetItemInfo = targetSearchPtr[j].pocket->itemInfo;
			for (int i = 0; i < targetItemInfo.size(); i++)
			{
				if (targetItemInfo[i].pocketMaxNumber > 0
					&& targetItemInfo[i].pocketPtr->countPocketItemNumber() < targetItemInfo[i].pocketMaxNumber)
				{
					if (std::find(
						targetItemInfo[i].pocketOnlyItem.begin(),
						targetItemInfo[i].pocketOnlyItem.end(),
						reloadItemPocket->itemInfo[targetLootCursor].itemCode) != targetItemInfo[i].pocketOnlyItem.end())
					{
						pocketList.push_back({ targetItemInfo[i].getSprIndex(), targetItemInfo[i].name, targetSearchPtr[j].source });
					}
				}
			}
		}

		if (pocketList.size() == 0)
		{
			updateLog(sysStr[96]);//이 아이템을 넣을만한 포켓이 없다.
			co_return;
		}

		////////////////////////////////////////////////////////////////////
		new LstEx(sysStr[95], sysStr[94], pocketList, spr::itemset);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();
		////////////////////////////////////////////////////////////////////

		if (coAnswer.empty() == false)
		{
			int counter = 0;
			for (int j = 0; j < targetSearchPtr.size(); j++)
			{
				std::vector<ItemData>& targetItemInfo = targetSearchPtr[j].pocket->itemInfo;
				for (int i = 0; i < targetItemInfo.size(); i++)
				{
					if (targetItemInfo[i].pocketMaxNumber > 0
						&& targetItemInfo[i].pocketPtr->countPocketItemNumber() < targetItemInfo[i].pocketMaxNumber)
					{
						if (std::find(targetItemInfo[i].pocketOnlyItem.begin(), targetItemInfo[i].pocketOnlyItem.end(), reloadItemPocket->itemInfo[targetLootCursor].itemCode) != targetItemInfo[i].pocketOnlyItem.end())
						{
							if (counter == wtoi(coAnswer.c_str()))
							{
								reloadItemPocket->transferItem
								(
									targetItemInfo[i].pocketPtr.get(),
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
		}
	}

	//장전해제 : 타겟아이템에 들어있는 아이템을 드랍하거나 인벤토리에 넣는다.
	void unload(ItemPocket* unloadItemPocket, int unloadItemCursor)
	{
		int targetLootCursor = unloadItemCursor;
		ItemPocket* targetPocket = unloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get();
		std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
		for (int i = 0; i < targetPocket->itemInfo.size(); i++) { targetPocket->transferItem(drop.get(), i, targetPocket->itemInfo[i].number); }
		PlayerPtr->drop(drop.get());
	}

	//배터리 장착 : 전자기기에 사용, 자신에게 배터리를 추가함
	Corouter insertBattery(actEnv envType, ItemPocket* targetItemPocket, int targetItemCursor)
	{
		prt(L"insertBattery가 실행되었다.\n");

		//주변 접근 가능한 모든 포켓 수집 (출처 정보 포함)
		std::vector<PocketSource> targetSearchPtr = gatherNearbyPocketsWithSource();

		//주변에서 battery 또는 batteryPack 찾기
		std::vector<LstExOption> batteryList;
		for (int j = 0; j < targetSearchPtr.size(); j++)
		{
			for (int i = 0; i < targetSearchPtr[j].pocket->itemInfo.size(); i++)
			{
				int itemCode = targetSearchPtr[j].pocket->itemInfo[i].itemCode;
				if (itemCode == itemID::battery || itemCode == itemID::batteryPack)
				{
					batteryList.push_back({ targetSearchPtr[j].pocket->itemInfo[i].getSprIndex(), targetSearchPtr[j].pocket->itemInfo[i].name, targetSearchPtr[j].source });
				}
			}
		}

		if (batteryList.size() == 0)
		{
			updateLog(sysStr[344]);//주변에 배터리가 없다.
			co_return;
		}

		////////////////////////////////////////////////////////////////////
		new LstEx(sysStr[342], sysStr[345], batteryList, spr::itemset);//배터리 장착, 장착할 배터리를 선택해주세요.
		co_await std::suspend_always();
		////////////////////////////////////////////////////////////////////

		if (coAnswer.empty() == false)
		{
			int counter = 0;
			for (int j = 0; j < targetSearchPtr.size(); j++)
			{
				for (int i = 0; i < targetSearchPtr[j].pocket->itemInfo.size(); i++)
				{
					int itemCode = targetSearchPtr[j].pocket->itemInfo[i].itemCode;
					if (itemCode == itemID::battery || itemCode == itemID::batteryPack)
					{
						if (counter == wtoi(coAnswer.c_str()))
						{
							//배터리를 전자기기에 장착
							targetSearchPtr[j].pocket->transferItem
							(
								targetItemPocket->itemInfo[targetItemCursor].pocketPtr.get(),
								i,
								1
							);
							updateLog(sysStr[346]);//배터리를 장착했다.
							turnWait(1.0);
							co_return;
						}
						counter++;
					}
				}
			}
		}
	}

	//배터리 분리 : 전자기기 내부에 들어있는 배터리를 분리한다
	void removeBattery(ItemPocket* unloadItemPocket, int unloadItemCursor)
	{
		int targetLootCursor = unloadItemCursor;
		ItemPocket* targetPocket = unloadItemPocket->itemInfo[targetLootCursor].pocketPtr.get();
		if (targetPocket->itemInfo.size() == 0)
		{
			updateLog(sysStr[348]);//분리할 배터리가 없다.
			return;
		}
		std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
		for (int i = 0; i < targetPocket->itemInfo.size(); i++) { targetPocket->transferItem(drop.get(), i, targetPocket->itemInfo[i].number); }
		PlayerPtr->drop(drop.get());
		updateLog(sysStr[347]);//배터리를 분리했다.
		turnWait(1.0);
	}
}
