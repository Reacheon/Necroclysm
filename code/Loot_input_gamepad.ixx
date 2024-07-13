#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

import std;
import globalVar;
import util;
import Loot;
import ItemData;

void Loot::gamepadBtnDown() 
{ 
	if (labelCursor != -1)//라벨 커서 조작 중
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
	else if (barActCursor == -1)//레터박스 액트 조작 중일 때
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			if (lootCursor > 0)
			{
				if (lootCursor % lootItemMax == 0)//스크롤 변경
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
			if (lootCursor < lootPtr->itemInfo.size() - 1)
			{
				if (lootCursor % lootItemMax == 5 && lootCursor != lootPtr->itemInfo.size() - 1)
				{
					lootScroll += lootItemMax;
				}
				lootCursor++;
			}
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		{
			int currentNumber = lootPtr->itemInfo[lootCursor].lootSelect;
			if (currentNumber > 0)
			{
				lootPtr->itemInfo[lootCursor].lootSelect = currentNumber - 1;
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		{
			int currentNumber = lootPtr->itemInfo[lootCursor].lootSelect;
			if (currentNumber < lootPtr->itemInfo[lootCursor].number)
			{
				lootPtr->itemInfo[lootCursor].lootSelect = currentNumber + 1;
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_B://취소
		{
			executeTab();
			break;
		}
		case SDL_CONTROLLER_BUTTON_A://아이템 상세 행동
		{
			if (doPopUpSingleHUD == false)
			{
				doPopUpSingleHUD = true;
				barActCursor = 0;
				//해당 아이템의 고유한 액트로 바꿈
				updateBarAct();
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_X://아이템 선택
		{
			if (lootPtr->itemInfo[lootCursor].lootSelect == 0)
			{
				executeSelectItem(lootCursor);
			}
			else
			{
				lootPtr->itemInfo[lootCursor].lootSelect = 0;
			}
			break;
		}
		case SDL_CONTROLLER_BUTTON_Y://아이템 줍기
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
	else
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
			case act::pick://넣기
				executePick();
				break;
			case act::equip://장비
				executeEquip();
				break;
			case act::wield://들기
				CORO(executeWield());
				break;
			case act::insert://아이템 넣기
				CORO(executeInsert());
			}
			break;
		case SDL_CONTROLLER_BUTTON_B:
		{
			doPopDownHUD = true;
			barActCursor = -1;
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
	//키다운에서 처리하면 exText에 열 때 사용된 문자가 들어가는 버그 발생해서 키업에 넣음
	if (labelCursor != -1)//라벨 커서 조작 중
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_A://확인
		{
			if (labelCursor == 0)
			{
				executeSelectAll();
			}
			else if (labelCursor == 1)
			{
				CORO(executeSearch());
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
