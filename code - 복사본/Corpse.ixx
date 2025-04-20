export module Corpse;

import std;
import textureVar;
import Sprite;

class Corpse;

export enum class corpseType
{
    bloodM,
    explosionM,
    warpM,
};

export class Corpse
{
private:
    Sprite* spr = nullptr;
    int sprIndex = 0;
    int timer = 0;
    int frame = 0;
    int x = 0;
    int y = 0;
    int z = 0;
public:
    static std::vector<Corpse*> list;
    Corpse(corpseType inputType, int inputX, int inputY, int inputZ)
    {
        spr = spr::bloodM;
        x = inputX;
        y = inputY;
        z = inputZ;
        list.push_back(this);
    }
    ~Corpse()
    {
        list.erase(std::find(list.begin(), list.end(), this));
    }
    int getX() { return x; }
    int getY() { return y; }
    int getZ() { return z; }
    Sprite* getSprite() { return spr; }
    int getSprIndex() { return frame; }
    void step()
    {
        if (timer == 3)
        {
            frame = 1;
        }
        else if (timer == 6)
        {
            frame = 2;
        }
        else if (timer == 9)
        {
            frame = 3;
        }
        else if (timer == 12)
        {
            frame = 4;
        }
        else if (timer == 15)
        {
            frame = 5;
        }
        else if (timer == 18)
        {
            frame = 6;
        }
        else if (timer == 500)
        {
            frame = 7;
        }
        else if (timer == 1000)
        {
            frame = 8;
        }
        else if (timer == 1500)
        {
            frame = 9;
        }
        else if (timer == 2000)
        {
            delete this;
        }

        timer++;
    }
};

std::vector<Corpse*> Corpse::list;
