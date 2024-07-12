#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

import Equip;
import checkCursor;
import globalVar;
import actFuncSet;


void Equip::clickUpGUI()
{
	if (checkCursor(&tab) == true)// �ǹڽ�
	{
		executeTab();
		return;
	}
	else if (checkCursor(&equipArea)) //������ Ŭ�� -> ���� ��Ʈ
	{
		//������ ���� Ŭ��
		//���� �������� Ŭ�������� Ŀ���� �� ���������� �ű�, �ٸ� �� ������ -1�� �ٲ�
		for (int i = 0; i < equipItemMax; i++)
		{
			if (equipPtr->itemInfo.size() - 1 >= i)
			{
				if (checkCursor(&equipItem[i]))
				{
					if (equipCursor != equipScroll + i) //���ο� Ŀ�� ����
					{
						equipCursor = equipScroll + i;
						updateBarAct();
						tabType = tabFlag::back;
					}
					else //Ŀ�� ����
					{
						equipCursor = -1;
						barAct = actSet::null;
						tabType = tabFlag::closeWin;
					}
					return;
				}
			}
		}
	}
	else if (checkCursor(&lootBase))
	{
		if (checkCursor(&lootArea)) //������ Ŭ�� -> ���� ��Ʈ
		{
			//���� �������� Ŭ�������� Ŀ���� �� ���������� �ű�, �ٸ� �� ������ -1�� �ٲ�
			for (int i = 0; i < lootItemMax; i++)
			{
				if (lootPtr->itemInfo.size() - 1 >= i)
				{
					if (checkCursor(&lootItem[i]))
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
			for (int i = 0; i < lootItemMax; i++)
			{
				if (checkCursor(&lootItemSelect[i]))
				{
					if (lootPtr->itemInfo.size() - 1 >= i)
					{
						if (lootPtr->itemInfo[i + lootScroll].lootSelect == 0)
						{
							if (inputType == input::mouse)
							{
								if (event.button.button == SDL_BUTTON_LEFT)
								{
									executeSelectItem(i + lootScroll);
								}
								else if (event.button.button == SDL_BUTTON_RIGHT)
								{
									CORO(executeSelectItemEx(pocketCursor, i + lootScroll));
								}
							}
							else if (inputType == input::touch)
							{
								executeSelectItem(i + lootScroll);
							}
						}
						else
						{
							lootPtr->itemInfo[i + lootScroll].lootSelect = 0;
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
				//lootPtr->sortPocket(sortFlag::null);
				//lootScroll = 0;
			}
			else if (checkCursor(&lootLabelQuantity))
			{
				executeSort();
			}
		}
		else if (checkCursor(&pocketLeft) || checkCursor(&pocketRight)) // ���� ���� ��ư(�¿�) Ŭ��
		{
			if (checkCursor(&pocketLeft)) //���� ��������
			{
				executePocketLeft();
			}
			else //������ ��������
			{
				executePocketRight();
			}
		}
		else if (checkCursor(&lootBtn))
		{
			executePickDrop();
		}
	}
	else if (checkCursor(&letterbox)) //��ư�� return ����
	{
		for (int i = 0; i < barAct.size(); i++) // �ϴ� UI ��ġ �̺�Ʈ
		{
			if (checkCursor(&barButton[i]))
			{
				switch (barAct[i])
				{
				case act::equip://���
					executeEquip();
					break;
				case act::wield://���
					CORO(executeWield());
					break;
				case act::droping:
				{
					executeDroping();
					break;
				}
				case act::throwing:
				{
					if (lootCursor != -1)
					{
						CORO(executeThrowing(lootPtr, lootCursor));
					}
					else if (equipCursor != -1)
					{
						CORO(executeThrowing(equipPtr, equipCursor));
					}

					break;
				}
				case act::open:
				{
					executeOpen();
					break;
				}
				case act::reload:
				{
					CORO(executeReload());
					break;
				}
				case act::reloadBulletToMagazine:
				case act::reloadBulletToGun:
					if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::MAGAZINE))
					{
						CORO(actFunc::reloadSelf(actEnv::Equip, equipPtr, equipCursor));
					}
					else if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::AMMO))
					{
						CORO(actFunc::reloadOther(actEnv::Equip, equipPtr, equipCursor));
					}
					else if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::GUN))
					{
						CORO(actFunc::reloadSelf(actEnv::Equip, equipPtr, equipCursor));
					}
					break;
				case act::reloadMagazine:
					//�ѿ��� ����ϴ� ���� źâ���� ����ϴ� ��찡 �ٸ�
					//�ѿ��� ����ϸ� �ڱ� �ڽſ��� ������(self)
					//źâ�� ����ϸ� �ٸ� Ÿ���� �ѿ��� �����
					if (equipPtr->itemInfo[equipCursor].checkFlag(itemFlag::MAGAZINE))
					{
						CORO(actFunc::reloadOther(actEnv::Equip, equipPtr, equipCursor));
					}
					else
					{
						CORO(actFunc::reloadSelf(actEnv::Equip, equipPtr, equipCursor));
					}
					break;
				case act::unloadMagazine:
				case act::unloadBulletFromMagazine:
				case act::unloadBulletFromGun:
				{
					if (equipCursor != -1)
					{
						actFunc::unload(equipPtr, equipCursor);
					}
					else
					{
						actFunc::unload(lootPtr, lootCursor);
					}
				}
				}

				if (equipPtr->itemInfo.size() == 0)
				{
					close(aniFlag::null);
					return;
				}

				if (equipPtr->itemInfo.size() - 1 <= equipScroll + equipScrollSize)
				{
					equipScroll = equipPtr->itemInfo.size() - equipScrollSize;
					if (equipScroll < 0) { equipScroll = 0; }
				}
			}
		}
	}

	//���� ��� ��쿡�� return�� ���� �������� ��ư �ܸ̿� ���� ���̹Ƿ� Ŀ���� -1�� ����
	{
		lootCursor = -1;
		equipCursor = -1;
		barAct = actSet::null;
		tabType = tabFlag::closeWin;
	}
}
void Equip::clickMotionGUI(int dx, int dy)
{
	if (checkCursor(&lootArea))
	{
		if (click == true)
		{
			int scrollAccelConst = 20; // ���ӻ��, �۾������� ��ũ�� �ӵ��� ������
			//lootScroll = initLootScroll + dy / scrollAccelConst;
			if (abs(dy / scrollAccelConst) >= 1)
			{
				deactClickUp = true;
				cursorMotionLock = true;
			}
		}
	}
}
void Equip::clickDownGUI()
{
	//������ ���� ����Ʈ Ŭ��
	selectTouchTime = SDL_GetTicks();
	initLootScroll = lootScroll;
	initPocketCursor = pocketCursor;
}