module;
#include <SDL3/SDL.h>

export module barActCommon;

import std;
import constVar;
import globalVar;
import ItemData;
import ItemPocket;


///@brief 총/탄창/탄환 관련 barAct 액션을 추가한다.
/// Loot, Equip, Inventory 3곳에서 동일하게 사용되는 로직을 통합한 함수.
export void appendGunAmmoBarActs(ItemData& targetItem)
{
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
		if (targetItem.itemCode != itemID::arrowQuiver && targetItem.itemCode != itemID::boltQuiver)
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
}
