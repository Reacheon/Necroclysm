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
   
public:
    Sprite* sprite = spr::itemset;
    int sprIndex = 2;
    static std::vector<Bullet*> list;
    Bullet(int inputX, int inputY)
    {
        list.push_back(this);
        setXY(inputX, inputY);
        if (list.size() > 500) prt(L"[메모리 누수 경고] Bullet의 객체 수가 500개를 넘어갔습니다.\n");
    }
    ~Bullet()
    {
        list.erase(std::find(list.begin(), list.end(), this));
    }
    void step()
    {
    }
    
};

std::vector<Bullet*> Bullet::list;