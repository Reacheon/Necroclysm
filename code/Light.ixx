#include <SDL.h>

export module Light;

import std;
import util;
import constVar;
import Coord;

export class Light : public Coord
{
private:
	//z값도 사용해야함
	//z값도 사용해야함
	//z값도 사용해야함
	//z값도 사용해야함

public:
	int lightRange = 1;
	SDL_Color lightColor = col::white;
	Uint8 bright = 255;
	std::set<Point3> litTiles;//밝혀진 타일

	Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor);
	~Light();
	void setLightRange(int inputRange);
	void updateLight(int range);
	void releaseLight();
	void rayCasting(int x1, int y1, int x2, int y2);
};