#include <SDL.h>

export module Footprint;

import Coord;
import std;
import globalVar;
import textureVar;
import constVar;
import drawText;
import Sprite;
import util;

export class Footprint : public Coord
{
private:

public:
    Sprite* sprite = spr::footprint;
    int sprIndex = 0;
    int lifetime = 30;
    int alpha = 80;
    static std::vector<Footprint*> list;
    Footprint(int inputGridX, int inputGridY, int inputDir, int inputLifetime = 80)
    {
        sprIndex += inputDir;
        lifetime = inputLifetime;
        list.push_back(this);
        setXY(16 * inputGridX, 16 * inputGridY);
        if (list.size() > 500) prt(L"[메모리 누수 경고] Footprint의 객체 수가 500개를 넘어갔습니다.\n");
    }
    ~Footprint()
    {
        list.erase(std::find(list.begin(), list.end(), this));
    }
    void step()
    {
        lifetime--;
        if (lifetime <= 0)
        {
            alpha -= 5;
            if (alpha <= 0) delete this;
        }
    }

};

std::vector<Footprint*> Footprint::list;