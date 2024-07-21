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
		for (int i = 0; i < EQUIP_ITEM_MAX; i++)
		{
			if (equipPtr->itemInfo.size() - 1 >= i)
			{
				if (checkCursor(&equipItemRect[i]))
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
					CORO(executeThrowing(equipPtr, equipCursor));
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
					actFunc::unload(equipPtr, equipCursor);
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
		equipCursor = -1;
		barAct = actSet::null;
		tabType = tabFlag::closeWin;
	}
}
void Equip::clickMotionGUI(int dx, int dy)
{
}
void Equip::clickDownGUI()
{
}