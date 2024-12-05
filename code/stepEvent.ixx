#include <SDL.h>

export module stepEvent;

import globalVar;
import constVar;
import util;
import Damage;
import Corpse;
import GUI;
import clickHold;
import log;
import Player;
import World;
import Snowflake;


//GUI의 스텝이벤트를 실행시키는 함수

export __int64 stepEvent()
{
    __int64 timeStampStart = getNanoTimer();

	//게임패드 감지

	if (SDL_NumJoysticks() > 0)
	{
		if (controller == nullptr)
		{
			for (int i = 0; i < SDL_NumJoysticks(); ++i)
			{
				if (SDL_IsGameController(i))
				{
					controller = SDL_GameControllerOpen(i);
					if (controller)
					{
						std::wstring str = L"다음 게임패드가 감지되었다. : ";
						str += stringToWstring(SDL_GameControllerName(controller));
						updateLog(col2Str(col::white) + str);
						inputType = input::gamepad;
						break;
					}
					else errorBox(L"게임패드를 열 수가 없다.");
				}
			}
		}
	}
	else controller = nullptr;



    if (dxClickStack > TOLERANCE_HOLD_DEL_XY || dyClickStack > TOLERANCE_HOLD_DEL_XY) deactHold = true;

    if (getMilliTimer() - clickStartTime > 1000)
    {
        if (deactHold == false)
        {
            clickHold();
        }
    }
	//데미지 객체 스탭 이벤트 실행
	for (int i = 0; i < Damage::list.size(); i++){Damage::list[i]->step();}

    //시체 객체 스텝 이벤트 실행
	for (int i = 0; i < Corpse::list.size(); i++){Corpse::list[i]->step();}

    //GUI 객체 스텝 이벤트 실행
    for (int i = 0; i < GUI::getActiveGUIList().size(); i++){GUI::getActiveGUIList()[i]->step();}

	int cx, cy;
	int pz = Player::ins()->getGridZ();
	World::ins()->changeToChunkCoord(Player::ins()->getGridX(), Player::ins()->getGridY(), cx, cy);
	if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::snow)
	{
		int randX = randomRange(0, cameraW);
		int randY = randomRange(0, cameraH);
		snowflakes.push_back(std::make_unique<Snowflake>(randX,randY));
	}

	for (int i = snowflakes.size() - 1; i >= 0; --i) 
	{
		snowflakes[i]->y += 2;
		if (snowflakes[i]->y > snowflakes[i]->dstY) 
		{
			snowflakes.erase(snowflakes.begin() + i);
		}
	}


    return (getNanoTimer() - timeStampStart);
}