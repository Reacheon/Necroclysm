#include <SDL3/SDL.h>
import util;
import constVar;
import Equip;
import checkCursor;
import globalVar;
import World;
import actFuncSet;
import Item;
import log;

void Equip::clickUpGUI()
{
	if (checkCursor(&tab) == true)// 탭박스
	{
		executeTab();
		return;
	}
	else if (checkCursor(&equipArea)) //아이템 클릭 -> 에러 파트
	{
		//아이템 메인 클릭
		{
			int result = panel.handleItemClick();
			if (result == 1) { updateBarAct(); return; }
			else if (result == -1) { return; }
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
				case act::droping:
				{
					executeDroping();
					break;
				}
				case act::throwing:
				{
					Corouter::start(actFunc::executeThrowing(equipPtr, panel.cursor));
					close(aniFlag::null);
					return;

				}
				case act::open:
				{
					executeOpen();
					break;
				}
				case act::reloadBulletToMagazine:
				case act::reloadBulletToGun:
					if (equipPtr->itemInfo[panel.cursor].checkFlag(itemFlag::MAGAZINE))
					{
						Corouter::start(actFunc::reloadSelf(actEnv::Equip, equipPtr, panel.cursor));
					}
					else if (equipPtr->itemInfo[panel.cursor].checkFlag(itemFlag::AMMO))
					{
						Corouter::start(actFunc::reloadOther(actEnv::Equip, equipPtr, panel.cursor));
					}
					else if (equipPtr->itemInfo[panel.cursor].checkFlag(itemFlag::GUN))
					{
						Corouter::start(actFunc::reloadSelf(actEnv::Equip, equipPtr, panel.cursor));
					}
					break;
				case act::reloadMagazine:
					//총에서 사용하는 경우와 탄창에서 사용하는 경우가 다름
					//총에서 사용하면 자기 자신에게 장전함(self)
					//탄창에 사용하면 다른 타일의 총에게 장비함
					if (equipPtr->itemInfo[panel.cursor].checkFlag(itemFlag::MAGAZINE))
					{
						Corouter::start(actFunc::reloadOther(actEnv::Equip, equipPtr, panel.cursor));
					}
					else
					{
						Corouter::start(actFunc::reloadSelf(actEnv::Equip, equipPtr, panel.cursor));
					}
					break;
				case act::unloadMagazine:
				case act::unloadBulletFromMagazine:
				case act::unloadBulletFromGun:
				{
					actFunc::unload(equipPtr, panel.cursor);
				}
				break;
				case act::toggleOff:
				case act::toggleOn:
					actFunc::toggle(equipPtr->itemInfo[panel.cursor]);
					updateBarAct();
					return;
				case act::drink:
					actFunc::drinkBottle(equipPtr->itemInfo[panel.cursor]);
					updateBarAct();
					close(aniFlag::null);
					return;
				case act::eat:
					actFunc::eatFood(equipPtr,panel.cursor);
					updateBarAct();
					return;
				case act::dump:
					actFunc::spillPocket(equipPtr->itemInfo[panel.cursor]);
					updateBarAct();
					return;
				case act::propInstall:
				{
					Corouter::start(executePropInstall());
					break;
				}
				case act::plant:
				{
					Corouter::start(actFunc::executePlant(equipPtr, panel.cursor));
					break;
				}
				case act::extractSeed:
					actFunc::extractSeed(actEnv::Equip, equipPtr, panel.cursor);
					updateBarAct();
					return;
				case act::insertBattery:
					Corouter::start(actFunc::insertBattery(actEnv::Equip, equipPtr, panel.cursor));
					break;
				case act::removeBattery:
					actFunc::removeBattery(equipPtr, panel.cursor);
					updateBarAct();
					return;
				case act::dye:
					Corouter::start(actFunc::executeDye(equipPtr, panel.cursor));
					return;
				}

				if (Equip::ins() != nullptr)
				{
					if (equipPtr->itemInfo.size() == 0)
					{
						close(aniFlag::null);
						return;
					}

					if (equipPtr->itemInfo.size() - 1 <= panel.scroll + equipScrollSize)
					{
						panel.scroll = equipPtr->itemInfo.size() - equipScrollSize;
						if (panel.scroll < 0) { panel.scroll = 0; }
					}
				}
				break;
			}
		}
	}

	//위의 모든 경우에서 return을 받지 못했으면 버튼 이외를 누른 것이므로 커서를 -1로 복구
	{
		panel.cursor = -1;
		barAct = actSet::null();
	}
}
void Equip::clickMotionGUI(int dx, int dy)
{
}
void Equip::clickDownGUI()
{
}
