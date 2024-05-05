#include <SDL.h>

export module Damage;

import std;
import globalVar;
import constVar;
import drawText;
import Sprite;
import util;

export class Damage
{
private:
    std::wstring letters;
    SDL_Color color = { 0xff,0xff,0xff };
    Sprite* sprite = nullptr;
    unsigned __int8 alpha = 255;
    int x = 0;
    int y = 0;
    int timer = 40;
public:
    static std::vector<Damage*> list;
    Damage(std::wstring inputStr, int inputX, int inputY, SDL_Color inputCol, int inputSize)
    {
        list.push_back(this);
        letters = inputStr;
        x = inputX;
        y = inputY;
        color = inputCol;

        // make new texture
        int textureW = queryTextWidth(letters, false) + 2;
        int textureH = queryTextHeight(letters, false) + 2;
        SDL_Texture* drawingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, textureW, textureH);
        SDL_SetRenderTarget(renderer, drawingTexture);
        SDL_SetTextureBlendMode(drawingTexture, SDL_BLENDMODE_BLEND);
        setFontSize(inputSize);
        drawTextCenter(col2Str(col::black) + letters, textureW / 2 + 1, textureH / 2);//외곽선
        drawTextCenter(col2Str(col::black) + letters, textureW / 2 - 1, textureH / 2);//외곽선
        drawTextCenter(col2Str(col::black) + letters, textureW / 2, textureH / 2 + 1);//외곽선
        drawTextCenter(col2Str(col::black) + letters, textureW / 2, textureH / 2 - 1);//외곽선
        drawTextCenter(col2Str(inputCol) + letters, textureW / 2, textureH / 2);
        SDL_SetRenderTarget(renderer, nullptr);
        sprite = new Sprite(renderer, drawingTexture, textureW, textureH);
    }
    ~Damage()
    {
        delete sprite;
        list.erase(std::find(list.begin(), list.end(), this));
    }
    std::wstring getStr() { return letters; }
    int getX() { return x; }
    int getY() { return y; }
    int getAlpha() { return alpha; }
    void step()
    {
        if (timer > 0)
        {
            y--;
            if (timer <= 20)
            {
                if (alpha <= 20) { alpha = 0; }
                else { alpha -= 20; }
            }
            timer--;
            if (timer <= 0) { delete this; }
        }
    }
    Sprite* getSprite() { return sprite; }
};

std::vector<Damage*> Damage::list;