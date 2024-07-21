#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

import Loot;
import std;
import util;
import Player;
import checkCursor;
import globalVar;
import actFuncSet;

void Loot::clickUpGUI()
{
	if (checkCursor(&tab) == true)// �ǹڽ�
	{
		executeTab();
		return;
	}
	else if (checkCursor(&lootArea)) //������ Ŭ�� -> ���� ��Ʈ
	{
		//���� �������� Ŭ�������� Ŀ���� �� ���������� �ű�, �ٸ� �� ������ -1�� �ٲ�
		for (int i = 0; i < LOOT_ITEM_MAX; i++)
		{
			if (lootPocket->itemInfo.size() - 1 >= i)
			{
				if (checkCursor(&lootItemRect[i]))
				{
					if (lootCursor != lootScroll + i) //���ο� Ŀ�� ����
					{
						lootCursor = lootScroll + i;
						updateBarAct();
						tabType = tabFlag::back;
					}
					else //Ŀ�� ����
					{
						lootCursor = -1;
						barAct = actSet::null;
						tabType = tabFlag::closeWin;
					}
					return;
				}
			}
		}

		//������ ���� ����Ʈ Ŭ��
		for (int i = 0; i < LOOT_ITEM_MAX; i++)
		{
			if (checkCursor(&lootItemSelectRect[i]))
			{
				if (lootPocket->itemInfo.size() - 1 >= i)
				{
					if (lootPocket->itemInfo[i + lootScroll].lootSelect == 0)
					{
						if (inputType == input::mouse)
						{
							if (event.button.button == SDL_BUTTON_LEFT)
							{
								executeSelectItem(i + lootScroll);
							}
							else if (event.button.button == SDL_BUTTON_RIGHT)
							{
								CORO(executeSelectItemEx(i + lootScroll));
							}
						}
						else if (inputType == input::touch)
						{
							executeSelectItem(i + lootScroll);
						}
					}
					else
					{
						lootPocket->itemInfo[i + lootScroll].lootSelect = 0;
					}
				}
			}
		}
	}
	else if (checkCursor(&lootLabel))
	{
		if (checkCursor(&lootLabelSelect))
		{
			executeSelectAll();
		}
		else if (checkCursor(&lootLabelName))
		{
			CORO(executeSearch());
			//lootPocket->sortPocket(sortFlag::null);
			//lootScroll = 0;
		}
		else if (checkCursor(&lootLabelQuantity))
		{
			executeSort();
		}
	}
	else if (checkCursor(&pocketLeft))
	{
		if (pocketCursor != 0) { pocketCursor--; }
	}
	else if (checkCursor(&pocketRight))
	{
		int numberOfBag = 0;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
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
	else if (checkCursor(&letterbox)) //��ư�� return ����
	{
		for (int i = 0; i < barAct.size(); i++) // �ϴ� UI ��ġ �̺�Ʈ
		{
			if (checkCursor(&barButton[i]))
			{
				switch (barAct[i])
				{
				case act::pick://�ֱ�
					executePick();
					break;
				case act::equip://���
					executeEquip();
					break;
				case act::wield://���
					CORO(executeWield());
					break;
					//case act::insert:
					//	CORO(executeInsert());
					//	break;
				case act::reloadBulletToMagazine:
				case act::reloadBulletToGun:
					if (lootPocket->itemInfo[lootCursor].checkFlag(itemFlag::MAGAZINE))
					{
						CORO(actFunc::reloadSelf(actEnv::Loot, lootPocket, lootCursor));
					}
					else if (lootPocket->itemInfo[lootCursor].checkFlag(itemFlag::AMMO))
					{
						CORO(actFunc::reloadOther(actEnv::Loot, lootPocket, lootCursor));
					}
					else if (lootPocket->itemInfo[lootCursor].checkFlag(itemFlag::GUN))
					{
						CORO(actFunc::reloadSelf(actEnv::Equip, lootPocket, lootCursor));
					}
					break;
				case act::reloadMagazine:
					//�ѿ��� ����ϴ� ���� źâ���� ����ϴ� ��찡 �ٸ�
					//�ѿ��� ����ϸ� �ڱ� �ڽſ��� ������(self)
					//źâ�� ����ϸ� �ٸ� Ÿ���� �ѿ��� �����
					if (lootPocket->itemInfo[lootCursor].checkFlag(itemFlag::MAGAZINE))
					{
						CORO(actFunc::reloadOther(actEnv::Loot, lootPocket, lootCursor));
					}
					else
					{
						CORO(actFunc::reloadSelf(actEnv::Loot, lootPocket, lootCursor));
					}
					break;
				case act::unloadMagazine:
				case act::unloadBulletFromMagazine:
				case act::unloadBulletFromGun:
					actFunc::unload(lootPocket, lootCursor);
					break;
				}
			}
		}
	}

	//���� ��� ��쿡�� return�� ���� �������� ��ư �ܸ̿� ���� ���̹Ƿ� Ŀ���� -1�� ����
	{
		lootCursor = -1;
		barAct = actSet::null;
		tabType = tabFlag::closeWin;
	}
}
void Loot::clickMotionGUI(int dx, int dy)
{
	if (checkCursor(&lootBase))
	{
		if (click == true)
		{
			int scrollAccelConst = 20; // ���ӻ��, �۾������� ��ũ�� �ӵ��� ������
			lootScroll = initLootScroll + dy / scrollAccelConst;
			if (abs(dy / scrollAccelConst) >= 1)
			{
				deactClickUp = true;
				cursorMotionLock = true;
			}
		}
	}
}
void Loot::clickDownGUI()
{
	//������ ���� ����Ʈ Ŭ��
	selectTouchTime = getMilliTimer();
	initLootScroll = lootScroll;
	initPocketCursor = pocketCursor;
}