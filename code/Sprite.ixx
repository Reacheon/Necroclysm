#include <SDL.h>
#include <SDL_image.h>

export module Sprite;

import std;
import util;

export class Sprite
{
private:
    SDL_Texture* texturePtr;
    int w;
    int h;
public:
    Sprite(SDL_Renderer* renderer, const char* adr, int inputW, int inputH)
    {
        texturePtr = IMG_LoadTexture(renderer, adr);
        if (texturePtr == nullptr)
        {
            std::string errorMsg = "Failed to load PNG file in the Sprite class. Path:  ";
            errorMsg += adr;
            errorBox(errorMsg.c_str());
        }
        w = inputW;
        h = inputH;
    }
    Sprite(SDL_Renderer* renderer, std::string adr, int inputW, int inputH)
    {
        texturePtr = IMG_LoadTexture(renderer, adr.c_str());
        w = inputW;
        h = inputH;
    }
    Sprite(SDL_Renderer* renderer, SDL_Texture* inputTexture, int inputW, int inputH)
    {
        texturePtr = inputTexture;
        w = inputW;
        h = inputH;
    }
    ~Sprite()
    {
        prt(L"Sprite : 소멸자가 호출되었습니다..\n");
        SDL_DestroyTexture(texturePtr);
    }
    SDL_Texture* getTexture() { return texturePtr; }
    int getW() { return w; }
    int getH() { return h; }
};