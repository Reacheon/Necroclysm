#include <SDL3/SDL.h>
import std;
import constVar;
import globalVar;
import util;
import Loot;
import ItemData;
import actFuncSet;

void Loot::gamepadBtnDown()
{
	if (labelCursor != -1)//라벨 커서 조작 중
	{
		switch (event.gbutton.button)
		{
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			panel.cursor = 0;
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
			if (panel.cursor > 0)
			{
				if (panel.cursor % LOOT_ITEM_MAX == 0)//스크롤 변경
				{
					panel.scroll -= LOOT_ITEM_MAX;
					if (panel.scroll < 0) { panel.scroll = 0; }
				}
				panel.cursor--;
			}
			else
			{
				panel.cursor = -1;
				labelCursor = 1;
			}
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			if (panel.cursor < (int)panel.pocket->itemInfo.size() - 1)
			{
				if (panel.cursor % LOOT_ITEM_MAX == 5 && panel.cursor != (int)panel.pocket->itemInfo.size() - 1)
				{
					panel.scroll += LOOT_ITEM_MAX;
				}
				panel.cursor++;
			}
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
		{
			int currentNumber = panel.pocket->itemInfo[panel.cursor].lootSelect;
			if (currentNumber > 0)
			{
				panel.pocket->itemInfo[panel.cursor].lootSelect = currentNumber - 1;
			}
			break;
		}
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
		{
			int currentNumber = panel.pocket->itemInfo[panel.cursor].lootSelect;
			if (currentNumber < panel.pocket->itemInfo[panel.cursor].number)
			{
				panel.pocket->itemInfo[panel.cursor].lootSelect = currentNumber + 1;
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
			if (panel.pocket->itemInfo[panel.cursor].lootSelect == 0)
			{
				panel.selectItem(panel.cursor);
			}
			else
			{
				panel.pocket->itemInfo[panel.cursor].lootSelect = 0;
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
				actFunc::executeEquip(panel.pocket, panel.cursor);
				break;
			case act::wield://들기
				Corouter::start(actFunc::executeWield(panel.pocket, panel.cursor));
				break;
			}
			break;
		case SDL_GAMEPAD_BUTTON_EAST:
		{
			barActCursor = -1;
			barAct = actSet::null();
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
				panel.selectAll();
			}
			else if (labelCursor == 1)
			{
				//게임패드로는 글 입력을 못하므로
				//Corouter::start(actFunc::searchItems(panel.pocket, panel.scroll));
			}
			else
			{
				panel.sort();
			}
			break;
		}
		}
	}
}
