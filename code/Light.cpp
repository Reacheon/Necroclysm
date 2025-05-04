#include <SDL.h>
#include <cmath>
#include <cstdlib>
#include <set>
#include <array>

import Light;
import std;
import util;
import World;
import constVar;
import wrapVar;
import Coord;

Light::Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor)
{
	prt(L"Light : 생성자가 실행되었습니다..\n");
	bright = inputBright;
	lightColor = inputColor;
	setLightRange(inputRange);
	setGrid(inputGridX, inputGridY, inputGridZ);
	updateLight();
}

Light::~Light()
{
	prt(L"Light : 소멸자가 호출되었습니다..\n");
	releaseLight();
}

void Light::setLightRange(int inputRange) { lightRange = inputRange; }

void Light::updateLight()
{
	for (int i = -lightRange; i <= lightRange; i++)
	{
		for (int j = -lightRange; j <= lightRange; j++)
		{
			if (isCircle(lightRange, i, j) == true)
			{
				rayCasting(getGridX(), getGridY(), getGridX() + i, getGridY() + j);
			}
		}
	}

	for (auto it = litTiles.begin(); it != litTiles.end(); it++)
	{
		int x = (*it).x;
		int y = (*it).y;
		int z = (*it).z;

		World::ins()->getTile(x, y, z).light += 1;

		float dist = sqrt(pow(x - getGridX(), 2) + pow(y - getGridY(), 2) + pow(z - getGridZ(), 2));
		int redLight = (float)lightColor.r * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
		int greenLight = (float)lightColor.g * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
		int blueLight = (float)lightColor.b * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
		World::ins()->getTile(x, y, z).redLight += redLight;
		World::ins()->getTile(x, y, z).greenLight += greenLight;
		World::ins()->getTile(x, y, z).blueLight += blueLight;
	}
}

void Light::releaseLight()
{
	for (auto it = litTiles.begin(); it != litTiles.end(); it++)
	{
		int x = (*it).x;
		int y = (*it).y;
		int z = (*it).z;

		World::ins()->getTile(x, y, z).light -= 1;

		float dist = sqrt(pow(x - getGridX(), 2) + pow(y - getGridY(), 2) + pow(z - getGridZ(), 2));
		int redLight = (float)lightColor.r * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
		int greenLight = (float)lightColor.g * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
		int blueLight = (float)lightColor.b * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
		World::ins()->getTile(x, y, z).redLight -= redLight;
		World::ins()->getTile(x, y, z).greenLight -= greenLight;
		World::ins()->getTile(x, y, z).blueLight -= blueLight;
	}
	litTiles.clear();
}

void Light::moveLight(int inputGridX, int inputGridY, int inputGridZ)
{
	releaseLight();
	setGrid(inputGridX, inputGridY, inputGridZ);
	updateLight();
}

void Light::rayCasting(int x1, int y1, int x2, int y2)
{
	int xo = x1;
	int yo = y1;
	int delx = abs(x2 - x1);
	int dely = abs(y2 - y1);
	int i = 0;
	litTiles.insert({ x1,y1,getGridZ() });
	if (fabs(1.0 * dely / delx) < 1)
	{
		int p = 2 * dely - delx;
		while (i < delx)
		{
			if (p < 0)
			{
				if (x2 > xo && y2 >= yo) { x1++; }
				else if (x2 > xo && yo > y2) { x1++; }
				else if (xo > x2 && y2 > yo) { x1--; }
				else { x1--; }
				litTiles.insert({ x1,y1,getGridZ() });
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
				p = p + (2 * dely);
			}
			else
			{
				if (x2 > xo && y2 >= yo) { x1++; y1++; }
				else if (x2 > xo && yo > y2) { x1++; y1--; }
				else if (xo > x2 && y2 > yo) { x1--; y1++; }
				else { x1--; y1--; }
				litTiles.insert({ x1,y1,getGridZ() });
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
				p = p + (2 * dely) - (2 * delx);
			}
			i++;
		}
		return;
	}
	else if (fabs(1.0 * dely / delx) > 1)
	{
		int p = (2 * delx) - dely;
		while (i < dely)
		{
			if (p < 0)
			{
				if (x2 >= xo && y2 > yo) { y1++; }
				else if (x2 > xo && yo > y2) { y1--; }
				else if (xo > x2 && y2 > yo) { y1++; }
				else { y1--; }
				litTiles.insert({ x1,y1,getGridZ() });
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
				p = p + (2 * delx);
			}
			else
			{
				if (x2 >= xo && y2 > yo) { x1++; y1++; }
				else if (x2 > xo && yo > y2) { x1++; y1--; }
				else if (xo > x2 && y2 > yo) { x1--; y1++; }
				else { x1--; y1--; }
				litTiles.insert({ x1,y1,getGridZ() });
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
				p = p + (2 * delx) - (2 * dely);
			}
			i++;
		}
	}
	else
	{
		while (i < delx)
		{
			if (x2 > x1 && y2 > y1) { x1++; y1++; }
			else if (x2 > x1 && y1 > y2) { x1++; y1--; }
			else if (x1 > x2 && y2 > y1) { x1--; y1++; }
			else { x1--; y1--; }
			litTiles.insert({ x1,y1,getGridZ() });
			if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
			i++;
		}
	}
}