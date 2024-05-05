#include <SDL.h>

export module Light;

import std;
import util;
import World;
import constVar;
import Coord;

export class Light : public Coord
{
private:
	//z값도 사용해야함
	//z값도 사용해야함
	//z값도 사용해야함
	//z값도 사용해야함
	int lightRange = 1;
	SDL_Color lightColor = col::white;
	Uint8 bright = 255;
	std::set< std::array<int, 3>> innateLight;
public:
	Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor)//생성자
	{
		prt(L"Light : 생성자가 실행되었습니다..\n");
		bright = inputBright;
		lightColor = inputColor;
		setLightRange(inputRange);
		setGrid(inputGridX, inputGridY,inputGridZ);
		updateLight(lightRange);
	}
	~Light()//소멸자
	{
		prt(L"Light : 소멸자가 호출되었습니다..\n");
		releaseLight();
	}
	void setLightRange(int inputRange) { lightRange = inputRange; }
	void updateLight(int range)
	{
		//매개인수로 받은 range만큼 원을 true로 만들고 그 방향으로 레이캐스팅을 실행해 tempFol을 gray||white로 만듬

		for (int i = -range; i <= range; i++)
		{
			for (int j = -range; j <= range; j++)
			{
				if (isCircle(range, i, j) == true)
				{
					rayCasting(getGridX(), getGridY(), getGridX() + i, getGridY() + j);
				}
			}
		}

		//모든 원방향의 레이캐스팅이 끝난 후에 tempFol을 체크해 gray나 white일 경우 전역변수 light에 1을 더함
		//여기서 만들어진 tempFol은 나중에 Light를 release할 때 사용됨

		for (auto it = innateLight.begin(); it != innateLight.end(); it++)
		{
			int x = (*it)[axis::x];
			int y = (*it)[axis::y];
			int z = (*it)[axis::z];

			World::ins()->getTile(x, y, z).light += 1;

			float dist = sqrt(pow(x - getGridX(), 2) + pow(y - getGridY(), 2) + pow(z - getGridZ(), 2));
			int redLight = (float)lightColor.r * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
			int greenLight = (float)lightColor.g * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
			int blueLight = (float)lightColor.b * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
			World::ins()->getTile(x, y, z).redLight += redLight;
			World::ins()->getTile(x, y, z).greenLight += greenLight;
			World::ins()->getTile(x, y, z).blueLight += blueLight;
			prt(L"(%d,%d,%d)의 월드 RGB값은 %d,%d,%d, 컬러 RGB값은 %d,%d,%d \n", x, y, z, World::ins()->getTile(x, y, z).redLight, World::ins()->getTile(x, y, z).greenLight, World::ins()->getTile(x, y, z).blueLight, lightColor.r, lightColor.g, lightColor.b);
		}
	}
	void releaseLight()
	{
		for (auto it = innateLight.begin(); it != innateLight.end(); it++)
		{
			int x = (*it)[axis::x];
			int y = (*it)[axis::y];
			int z = (*it)[axis::z];

			World::ins()->getTile(x, y, z).light -= 1;

			float dist = sqrt(pow(x - getGridX(), 2) + pow(y - getGridY(), 2) + pow(z - getGridZ(), 2));
			int redLight = (float)lightColor.r * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
			int greenLight = (float)lightColor.g * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
			int blueLight = (float)lightColor.b * ((float)bright / 255.0) * pow(1 - ((dist) / (float)lightRange), 2);
			World::ins()->getTile(x, y, z).redLight -= redLight;
			World::ins()->getTile(x, y, z).greenLight -= greenLight;
			World::ins()->getTile(x, y, z).blueLight -= blueLight;
		}
		innateLight.clear();
	}
	void rayCasting(int x1, int y1, int x2, int y2)
	{
		int xo = x1;
		int yo = y1;
		int delx = abs(x2 - x1);
		int dely = abs(y2 - y1);
		int i = 0;
		innateLight.insert({ x1,y1,getGridZ() });
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
					innateLight.insert({ x1,y1,getGridZ() });
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * dely);
				}
				else
				{
					if (x2 > xo && y2 >= yo) { x1++; y1++; }
					else if (x2 > xo && yo > y2) { x1++; y1--; }
					else if (xo > x2 && y2 > yo) { x1--; y1++; }
					else { x1--; y1--; }
					innateLight.insert({ x1,y1,getGridZ() });
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
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
					innateLight.insert({ x1,y1,getGridZ() });
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * delx);
				}
				else
				{
					if (x2 >= xo && y2 > yo) { x1++; y1++; }
					else if (x2 > xo && yo > y2) { x1++; y1--; }
					else if (xo > x2 && y2 > yo) { x1--; y1++; }
					else { x1--; y1--; }
					innateLight.insert({ x1,y1,getGridZ() });
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
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
				innateLight.insert({ x1,y1,getGridZ() });
				if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
				i++;
			}
		}
	}
};