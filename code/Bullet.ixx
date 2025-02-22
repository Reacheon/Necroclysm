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
        if (list.size() > 500) prt(L"[�޸� ���� ���] Bullet�� ��ü ���� 500���� �Ѿ���ϴ�.\n");
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