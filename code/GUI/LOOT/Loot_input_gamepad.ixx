﻿#include <SDL3/SDL.h>
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
		switch (event.gbutton.button)
		{
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			lootCursor = 0;
			labelCursor = -1;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
			if (labelCursor != 0) labelCursor--;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
			if (labelCursor < 2) labelCursor++;
			break;
		case SDL_GAMEPAD_BUTTON_EAST:
			executeTab();
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			executePocketLeft();
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			executePocketRight();
			break;
		}
	}
	else if (barActCursor == -1)//일반 루팅 아이템 상하 조작 중
	{
		switch (event.gbutton.button)
		{
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			if (lootCursor > 0)
			{
				if (lootCursor % LOOT_ITEM_MAX == 0)//스크롤 변경
				{
					lootScroll -= LOOT_ITEM_MAX;
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
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			if (lootCursor < lootPocket->itemInfo.size() - 1)
			{
				if (lootCursor % LOOT_ITEM_MAX == 5 && lootCursor != lootPocket->itemInfo.size() - 1)
				{
					lootScroll += LOOT_ITEM_MAX;
				}
				lootCursor++;
			}
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
		{
			int currentNumber = lootPocket->itemInfo[lootCursor].lootSelect;
			if (currentNumber > 0)
			{
				lootPocket->itemInfo[lootCursor].lootSelect = currentNumber - 1;
			}
			break;
		}
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
		{
			int currentNumber = lootPocket->itemInfo[lootCursor].lootSelect;
			if (currentNumber < lootPocket->itemInfo[lootCursor].number)
			{
				lootPocket->itemInfo[lootCursor].lootSelect = currentNumber + 1;
			}
			break;
		}
		case SDL_GAMEPAD_BUTTON_EAST://취소
		{
			executeTab();
			break;
		}
		case SDL_GAMEPAD_BUTTON_SOUTH://아이템 상세 행동
		{
			updateBarAct();
			barActCursor = 0;
			break;
		}
		case SDL_GAMEPAD_BUTTON_WEST://아이템 선택
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
		case SDL_GAMEPAD_BUTTON_NORTH://아이템 줍기
		{
			executePickSelect();
			break;
		}
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			executePocketLeft();
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			executePocketRight();
			break;
		}
	}
	else //루팅 아이템 상세 바액트 조작
	{
		switch (event.gbutton.button)
		{
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
			if (barActCursor != 0) { barActCursor--; }
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
			if (barActCursor != 6 && barActCursor < barAct.size() - 1)
			{
				barActCursor++;
			}
			break;
		case SDL_GAMEPAD_BUTTON_SOUTH:
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
		case SDL_GAMEPAD_BUTTON_EAST:
		{
			barActCursor = -1;
			barAct = actSet::null;
		}
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			executePocketLeft();
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
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
		switch (event.gbutton.button)
		{
		case SDL_GAMEPAD_BUTTON_SOUTH://확인
		{
			if (labelCursor == 0)
			{
				executeSelectAll();
			}
			else if (labelCursor == 1)
			{
				//게임패드로는 글 입력을 못하므로
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
