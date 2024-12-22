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
		//static int index = 0;
		//int sprSize = 12;
		//if (timer::timer600 % 3 == 0)
		//{
		//	index++;
		//	if (index >= sprSize) index = 0;
		//}
		//drawSprite(spr::screenRain, index, 0, 0);
	}
	else if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::snow)
	{
		//{
		//	static int yShift1 = 0;
		//	if (timer::timer600 % 2 == 0)
		//	{
		//		yShift1++;
		//		if (yShift1 == 720) yShift1 = 0;
		//	}

		//	drawSprite(spr::screenSnow, 0, 0, yShift1);
		//	drawSprite(spr::screenSnow, 0, 0, yShift1 - 720);
		//}

		//{
		//	static int yShift2 = 0;
		//	if (timer::timer600 % 3 == 0)
		//	{
		//		yShift2++;
		//		if (yShift2 == 720) yShift2 = 0;
		//	}

		//	setFlip(SDL_FLIP_HORIZONTAL);
		//	SDL_SetTextureAlphaMod(spr::screenSnow->getTexture(), 130); //텍스쳐 투명도 설정
		//	SDL_SetTextureBlendMode(spr::screenSnow->getTexture(), SDL_BLENDMODE_BLEND);
		//	drawSprite(spr::screenSnow, 0, 0, yShift2);
		//	drawSprite(spr::screenSnow, 0, 0, yShift2 - 720);
		//	setFlip(SDL_FLIP_NONE);
		//	SDL_SetTextureAlphaMod(spr::screenSnow->getTexture(), 255); //텍스쳐 투명도 설정
		//}
	}
	else
	{
	}

	for (int i = 0; i < snowflakes.size(); i++)
	{
		drawFillRect({ (snowflakes[i].get())->x, (snowflakes[i].get())->y,(snowflakes[i].get())->size,(snowflakes[i].get())->size }, { 0xff, 0xff, 0xff }, (Uint8)(snowflakes[i].get())->alpha);
		//drawPoint((snowflakes[i].get())->x, (snowflakes[i].get())->y, { 0xff, 0xff, 0xff });
	}

	for (int i = 0; i < raindrops.size(); i++)
	{
		int dx = cos((raindrops[i].get())->angle) * (raindrops[i].get())->length;
		int dy = sin((raindrops[i].get())->angle) * (raindrops[i].get())->length;

		Point2 p1 = { (raindrops[i].get())->x, (raindrops[i].get())->y };
		Point2 p2 = { (raindrops[i].get())->x + dx, (raindrops[i].get())->y - dy };


		drawLine(p1.x, p1.y, p2.x, p2.y, { 0xa5, 0xbe, 0xd8 }, (Uint8)(raindrops[i].get())->alpha);
		drawLine(p1.x+1, p1.y, p2.x+1, p2.y, { 0xa5, 0xbe, 0xd8 }, (Uint8)(raindrops[i].get())->alpha);

	}

	return (getNanoTimer() - timeStampStart);
}