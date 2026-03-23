module;
#include <SDL3/SDL.h>

export module Damage;

import std;
import constVar;
import Sprite;

export class Damage
{
private:
    std::wstring letters;
    SDL_Color color = { 0xff,0xff,0xff };
    Sprite* sprite = nullptr;
    unsigned __int8 alpha = 255;
    float x = 0.f;
    float y = 0.f;
    float velX = 0.f;
    float velY = 0.f;
    float originY = 0.f;
    int timer = 40;

    dmgAniFlag myDmgAniFlag = dmgAniFlag::none;
public:
    static std::vector<Damage*> list;
    Damage(std::wstring inputStr, int inputX, int inputY, SDL_Color inputCol, int inputSize);
    Damage(std::wstring inputStr, SDL_Color inputCol, int gridX, int gridY, dmgAniFlag inputFlag);
    ~Damage();
    std::wstring getStr() { return letters; }
    float getX() { return x; }
    float getY() { return y; }
    int getAlpha() { return alpha; }
    void step();
    Sprite* getSprite() { return sprite; }
    dmgAniFlag getAniFlag() { return myDmgAniFlag; }
};
