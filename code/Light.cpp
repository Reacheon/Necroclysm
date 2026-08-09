#include <SDL3/SDL.h>
#include <cmath>
#include <cstdlib>
#include <set>
#include <array>

import Light;
import std;
import util;
import World;
import constVar;
import Coord;

Light::Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor)
{
	dbgPrt(L"Light : 생성자가 실행되었습니다..\n");
	bright = inputBright;
	lightColor = inputColor;
	setLightRange(inputRange);
	setGrid(inputGridX, inputGridY, inputGridZ);
	updateLight();
}

Light::Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor, dir16 inputDir)
{
	dir = inputDir;

	dbgPrt(L"Light : 생성자가 실행되었습니다..\n");
	bright = inputBright;
	lightColor = inputColor;
	setLightRange(inputRange);
	setGrid(inputGridX, inputGridY, inputGridZ);
	updateLight();
}

Light::~Light()
{
	dbgPrt(L"Light : 소멸자가 호출되었습니다..\n");
	releaseLight();
}

void Light::setLightRange(int inputRange) { lightRange = inputRange; }

void Light::updateLight()
{
	const float coneHalf = 65.f;              
	const float dirAngle = dir16toAngle(dir);

	for (int dx = -lightRange; dx <= lightRange; ++dx)
	{
		for (int dy = -lightRange; dy <= lightRange; ++dy)
		{
			if (!isCircle(lightRange, dx, dy)) continue;

			if (dir != dir16::none)
			{
				if (dx == 0 && dy == 0) continue;
					

				float angTo = std::atan2f(-dy, dx) * 180.f / static_cast<float>(3.141592);
				if (angTo < 0) angTo += 360.f;

				float diff = std::fabs(angTo - dirAngle);
				if (diff > 180.f) diff = 360.f - diff;

				if (diff > coneHalf) continue;
			}

			rayCasting(getGridX(), getGridY(), getGridX() + dx, getGridY() + dy);
		}
	}

	for (auto it = litTiles.begin(); it != litTiles.end(); it++)
	{
		int x = (*it).x;
		int y = (*it).y;
		int z = (*it).z;
		float dist = sqrt(pow(x - getGridX(), 2) + pow(y - getGridY(), 2) + pow(z - getGridZ(), 2));
        Uint8 brightness = (float)bright * pow(1 - ((dist) / (float)lightRange), 2);
		World::ins()->getTile(x, y, z).lightVec.push_back({ lightColor.r ,lightColor.g,lightColor.b, brightness});
	}
}

void Light::releaseLight()
{
	for (auto it = litTiles.begin(); it != litTiles.end(); it++)
	{
		int x = (*it).x;
		int y = (*it).y;
		int z = (*it).z;

		// 광역 청크 소멸(게임 종료) 중 — Light 소유자(Prop의 ItemData 등)
		// 가 멤버 소멸로 ~Light를 호출할 때, lit 타일의 청크가 이미 사라져있을 수 있음.
		// getTile()의 .at()이 throw하므로 청크별 안전 조회로 우회 — 청크가 없으면 정리 불필요.
		int chunkX, chunkY;
		World::ins()->changeToChunkCoord(x, y, chunkX, chunkY);
		Chunk* chunk = World::ins()->tryGetChunk(chunkX, chunkY, z);
		if (chunk == nullptr) continue;

		float dist = sqrt(pow(x - getGridX(), 2) + pow(y - getGridY(), 2) + pow(z - getGridZ(), 2));
		Uint8 brightness = (float)bright * pow(1 - ((dist) / (float)lightRange), 2);

		int localX = x - (chunkX * CHUNK_SIZE_X);
		int localY = y - (chunkY * CHUNK_SIZE_Y);
		auto& lightVec = chunk->getChunkTile(localX, localY).lightVec;
		for (int i = 0; i < lightVec.size(); i++)
		{
			if (lightVec[i].r == lightColor.r &&
				lightVec[i].g == lightColor.g &&
				lightVec[i].b == lightColor.b &&
				lightVec[i].a == brightness)
			{
				lightVec.erase(lightVec.begin() + i);
				break;
			}
		}
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