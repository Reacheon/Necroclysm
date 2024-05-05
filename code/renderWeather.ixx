#include <SDL.h>

export module renderWeather;

import util;
import globalVar;
import constVar;
import World;
import Player;
import textureVar;
import drawSprite;

export __int64 renderWeather()
{
	__int64 timeStampStart = getNanoTimer();


	int cx, cy;
	int pz = Player::ins()->getGridZ();
	World::ins()->changeToChunkCoord(Player::ins()->getGridX(), Player::ins()->getGridY(), cx, cy);
	if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::rain)
	{
		static int index = 0;
		int sprSize = 12;
		if (timer::timer600 % 3 == 0)
		{
			index++;
			if (index >= sprSize) index = 0;
		}
		drawSprite(spr::screenRain, index, 0, 0);
	}
	else if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::snow)
	{
		static int yShift = 0;
		if (timer::timer600 % 3 == 0)
		{
			yShift++;
			if (yShift == 720) yShift = 0;
		}

		drawSprite(spr::screenSnow, 0, 0, yShift);
		drawSprite(spr::screenSnow, 0, 0, yShift - 720);
	}
	else
	{
	}

	return (getNanoTimer() - timeStampStart);
}