#include <SDL3/SDL.h>

export module Light;

import std;
import util;
import constVar;
import Coord;

export class Light : public Coord
{
private:
public:
	int lightRange;
	SDL_Color lightColor;
	Uint8 bright;
	std::set<Point3> litTiles;//밝혀진 타일

	dir16 dir = dir16::none;

	Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor);
	Light(int inputGridX, int inputGridY, int inputGridZ, int inputRange, Uint8 inputBright, SDL_Color inputColor,dir16 inputDir);
	~Light();
	void setLightRange(int inputRange);
	void updateLight();
	void releaseLight();
    void moveLight(int inputGridX, int inputGridY, int inputGridZ);
	void rayCasting(int x1, int y1, int x2, int y2);
};