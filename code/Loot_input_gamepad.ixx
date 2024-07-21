#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

import std;
import globalVar;
import util;
import Loot;
import ItemData;

void Loot::gamepadBtnDown() 
{ 
	if (labelCursor != -1)//�� Ŀ�� ���� ��
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			lootCursor = 0;
			labelCursor = -1;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			if (labelCursor != 0) labelCursor--;
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			if (labelCursor < 2) labelCursor++;
			break;
		case SDL_CONTROLLER_BUTTON_B:
			executeTab();
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			executePocketLeft();
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			executePocketRight();
			break;
		}
	}
	else if (barActCursor == -1)//�Ϲ� ���� ������ ���� ���� ��
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			if (lootCursor > 0)
			{
				if (lootCursor % lootItemMax == 0)//��ũ�� ����
				{
					lootScroll -= lootItemMax;
					if (lootScroll < 0) { lootScroll = 0; }
				}
				lootCursor--;
			}
			else
			{
				lootCursor = -1;
				labelCursor = 1;
			}
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			if (lootCursor < lootPocket->itemInfo.size() - 1)
			{
				if (lootCursor % lootItemMax == 5 && lootCursor != lootPocket->itemInfo.size() - 1)
				{
					lootScroll += lootItemMax;
				}
				lootCursor++;
			}
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		{
			int currentNumber = lootPocket->itemInfo[lootCursor].lootSelect;
			if (currentNumber > 0)
			{
				lootPocket->itemInfo[lootCursor].lootSelect = currentNumber - 1;
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		{
			int currentNumber = lootPocket->itemInfo[lootCursor].lootSelect;
			if (currentNumber < lootPocket->itemInfo[lootCursor].number)
			{
				lootPocket->itemInfo[lootCursor].lootSelect = currentNumber + 1;
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_B://���
		{
			executeTab();
			break;
		}
		case SDL_CONTROLLER_BUTTON_A://������ �� �ൿ
		{
			updateBarAct();
			barActCursor = 0;
			break;
		}
		case SDL_CONTROLLER_BUTTON_X://������ ����
		{
			if (lootPocket->itemInfo[lootCursor].lootSelect == 0)
			{
				executeSelectItem(lootCursor);
			}
			else
			{
				lootPocket->itemInfo[lootCursor].lootSelect = 0;
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_Y://������ �ݱ�
		{
			executePickSelect();
			break;
		}
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			executePocketLeft();
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			executePocketRight();
			break;
		}
	}
	else //���� ������ �� �پ�Ʈ ����
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			if (barActCursor != 0) { barActCursor--; }
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			if (barActCursor != 6 && barActCursor < barAct.size() - 1)
			{
				barActCursor++;
			}
			break;
		case SDL_CONTROLLER_BUTTON_A:
			switch (barAct[barActCursor])
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
			case act::insert://������ �ֱ�
				CORO(executeInsert());
			}
			break;
		case SDL_CONTROLLER_BUTTON_B:
		{
			barActCursor = -1;
			barAct = actSet::null;
		}
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			executePocketLeft();
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			executePocketRight();
			break;
		}
		}
}
void Loot::gamepadBtnMotion() 
{

}
void Loot::gamepadBtnUp()
{
	//Ű�ٿ�� ó���ϸ� exText�� �� �� ���� ���ڰ� ���� ���� �߻��ؼ� Ű���� ����
	if (labelCursor != -1)//�� Ŀ�� ���� ��
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_A://Ȯ��
		{
			if (labelCursor == 0)
			{
				executeSelectAll();
			}
			else if (labelCursor == 1)
			{
				//�����е�δ� �� �Է��� ���ϹǷ�
				//CORO(executeSearch());
			}
			else
			{
				executeSort();
			}
			break;
		}
		}
	}
}
