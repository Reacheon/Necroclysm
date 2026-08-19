#include <SDL3/SDL.h>
import Loot;
import std;
import util;
import constVar;
import Player;
import checkCursor;
import globalVar;
import World;
import actFuncSet;
import Item;

void Loot::clickUpGUI()
{
	if (checkCursor(&tab) == true)// 탭박스
	{
		executeTab();
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
			Corouter::start(actFunc::searchItems(panel.pocket, panel.scroll));
		}
		else if (checkCursor(&panel.labelQuantity))
		{
			panel.sort();
		}
	}
	else if (checkCursor(&lootBase)) //아이템 클릭 -> 에러 파트
	{
		if (checkCursor(&pocketLeft))
		{
			if (pocketCursor != 0) { pocketCursor--; }
		}
		else if (checkCursor(&pocketRight))
		{
			int numberOfBag = 0;
			ItemPocket* equipPtr = PlayerEquip();
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketPtr !=nullptr)
				{
					numberOfBag++;
				}
			}
			if (pocketCursor != numberOfBag - 1) { pocketCursor++; }
		}
		else if (checkCursor(&lootBtn))
		{
			executePickSelect();
		}
		else
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
	}
	else if (checkCursor(&letterbox)) //버튼은 return 없음
	{
		for (int i = 0; i < barAct.size(); i++) // 하단 UI 터치 이벤트
		{
			if (checkCursor(&barButton[i]))
			{
				switch (barAct[i])
				{
				case act::pick://넣기
					executePick();
					break;
				case act::equip://장비
					actFunc::executeEquip(panel.pocket, panel.cursor);
					break;
				case act::wield://들기
					Corouter::start(actFunc::executeWield(panel.pocket, panel.cursor));
					break;
				case act::reloadBulletToMagazine:
				case act::reloadBulletToGun:
					if (panel.pocket->itemInfo[panel.cursor].checkFlag(itemFlag::MAGAZINE))
					{
						Corouter::start(actFunc::reloadSelf(actEnv::Loot, panel.pocket, panel.cursor));
					}
					else if (panel.pocket->itemInfo[panel.cursor].checkFlag(itemFlag::AMMO))
					{
						Corouter::start(actFunc::reloadOther(actEnv::Loot, panel.pocket, panel.cursor));
					}
					else if (panel.pocket->itemInfo[panel.cursor].checkFlag(itemFlag::GUN))
					{
						Corouter::start(actFunc::reloadSelf(actEnv::Equip, panel.pocket, panel.cursor));
					}
					break;
				case act::reloadMagazine:
					//총에서 사용하는 경우와 탄창에서 사용하는 경우가 다름
					//총에서 사용하면 자기 자신에게 장전함(self)
					//탄창에 사용하면 다른 타일의 총에게 장비함
					if (panel.pocket->itemInfo[panel.cursor].checkFlag(itemFlag::MAGAZINE))
					{
						Corouter::start(actFunc::reloadOther(actEnv::Loot, panel.pocket, panel.cursor));
					}
					else
					{
						Corouter::start(actFunc::reloadSelf(actEnv::Loot, panel.pocket, panel.cursor));
					}
					break;
				case act::unloadMagazine:
				case act::unloadBulletFromMagazine:
				case act::unloadBulletFromGun:
					actFunc::unload(panel.pocket, panel.cursor);
					break;
				case act::toggleOff:
				case act::toggleOn:
					actFunc::toggle(panel.pocket->itemInfo[panel.cursor]);
					updateBarAct();
					return;
				case act::drink:
					actFunc::drinkBottle(panel.pocket->itemInfo[panel.cursor]);
					close(aniFlag::null);
					return;
				case act::eat:
					actFunc::eatFood(panel.pocket, panel.cursor);
					updateBarAct();
					return;
				case act::dump:
					actFunc::spillPocket(panel.pocket->itemInfo[panel.cursor]);
					updateBarAct();
					return;
				case act::insertBattery:
					Corouter::start(actFunc::insertBattery(actEnv::Loot, panel.pocket, panel.cursor));
					break;
				case act::removeBattery:
					actFunc::removeBattery(panel.pocket, panel.cursor);
					updateBarAct();
					return;
				case act::extractSeed:
					actFunc::extractSeed(actEnv::Loot, panel.pocket, panel.cursor);
					updateBarAct();
					return;
				case act::dye:
					Corouter::start(actFunc::executeDye(panel.pocket, panel.cursor));
					return;
				}
			}
		}
	}

	//위의 모든 경우에서 return을 받지 못했으면 버튼 이외를 누른 것이므로 커서를 -1로 복구
	{
		panel.cursor = -1;
		barAct = actSet::null();
	}
}
void Loot::clickMotionGUI(int dx, int dy)
{
	if (checkCursor(&lootBase))
	{
		if (click == true)
		{
			int scrollAccelConst = 20; // 가속상수, 작아질수록 스크롤 속도가 빨라짐
			panel.scroll = initLootScroll + dy / scrollAccelConst;
			if (abs(dy / scrollAccelConst) >= 1)
			{
				deactClickUp = true;
				itemListColorLock = true;
			}
		}
	}
}
void Loot::clickDownGUI()
{
	//아이템 좌측 셀렉트 클릭
	selectTouchTime = getMilliTimer();
	initLootScroll = panel.scroll;
	initPocketCursor = pocketCursor;
}

void Loot::clickRightGUI()
{
	//아이템 좌측 셀렉트 우클릭
	int idx = panel.getSelectRightClickIndex();
	if (idx >= 0)
	{
		Corouter::start(actFunc::selectItemEx(panel.pocket, idx));
	}
}
