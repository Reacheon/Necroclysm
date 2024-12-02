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
		{
			static int yShift1 = 0;
			if (timer::timer600 % 2 == 0)
			{
				yShift1++;
				if (yShift1 == 720) yShift1 = 0;
			}

			drawSprite(spr::screenSnow, 0, 0, yShift1);
			drawSprite(spr::screenSnow, 0, 0, yShift1 - 720);
		}

		{
			static int yShift2 = 0;
			if (timer::timer600 % 3 == 0)
			{
				yShift2++;
				if (yShift2 == 720) yShift2 = 0;
			}

			setFlip(SDL_FLIP_HORIZONTAL);
			SDL_SetTextureAlphaMod(spr::screenSnow->getTexture(), 130); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::screenSnow->getTexture(), SDL_BLENDMODE_BLEND);
			drawSprite(spr::screenSnow, 0, 0, yShift2);
			drawSprite(spr::screenSnow, 0, 0, yShift2 - 720);
			setFlip(SDL_FLIP_NONE);
			SDL_SetTextureAlphaMod(spr::screenSnow->getTexture(), 255); //텍스쳐 투명도 설정
		}
	}
	else
	{
	}

	return (getNanoTimer() - timeStampStart);
}