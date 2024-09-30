#include <SDL.h>

export module Bullet;

import Coord;
import std;
import globalVar;
import textureVar;
import constVar;
import drawText;
import Sprite;
import util;

export class Bullet : public Coord
{
private:
    Sprite* sprite = spr::itemset;
    int sprIndex = 2;
public:
    static std::vector<Bullet*> list;
    Bullet(int inputX, int inputY)
    {
        list.push_back(this);
        setXY(inputX, inputY);
    }
    ~Bullet()
    {
        list.erase(std::find(list.begin(), list.end(), this));
    }
    void step()
    {
    }
    Sprite* getSprite() { return sprite; }
};

std::vector<Bullet*> Bullet::list;