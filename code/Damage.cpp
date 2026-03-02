#include <SDL3/SDL.h>

module Damage;

import globalVar;
import drawText;
import util;
import drawEpsilonText;

std::vector<Damage*> Damage::list;

Damage::Damage(std::wstring inputStr, int inputX, int inputY, SDL_Color inputCol, int inputSize)
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
    SDL_SetTextureScaleMode(drawingTexture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderTarget(renderer, drawingTexture);
    SDL_SetTextureBlendMode(drawingTexture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    drawEplsionText(letters, textureW / 2, textureH / 2, col::white, col::black);
    SDL_SetRenderTarget(renderer, nullptr);
    sprite = new Sprite(renderer, drawingTexture, textureW, textureH);
}

Damage::Damage(std::wstring inputStr, SDL_Color inputCol, int gridX, int gridY, dmgAniFlag inputFlag)
{
    myDmgAniFlag = inputFlag;

    list.push_back(this);
    letters = inputStr;
    x = 16 * gridX + 8 + randomRange(-4, 4);
    y = 16 * gridY + 8 - 3 + randomRange(-4, 4);
    color = inputCol;

    // make new texture
    int textureW = queryTextWidth(letters, false) + 2;
    int textureH = queryTextHeight(letters, false) + 2;
    SDL_Texture* drawingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, textureW, textureH);
    SDL_SetTextureScaleMode(drawingTexture, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderTarget(renderer, drawingTexture);
    SDL_SetTextureBlendMode(drawingTexture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    int epsilonW = getEpsilonTextWidth(letters);
    int epsilonH = getEpsilonTextHeight();
    drawEplsionText(letters, textureW / 2 - epsilonW / 2, textureH / 2 - epsilonH / 2, inputCol, col::black);

    SDL_SetRenderTarget(renderer, nullptr);
    sprite = new Sprite(renderer, drawingTexture, textureW, textureH);
}

Damage::~Damage()
{
    delete sprite;
    list.erase(std::find(list.begin(), list.end(), this));
}

void Damage::step()
{
    if (myDmgAniFlag == dmgAniFlag::dodged)
    {
        y--;
        if (timer <= 32)
        {
            if (alpha <= 30) { alpha = 0; }
            else { alpha -= 30; }
        }
        timer--;
        if (timer <= 0) { delete this; }
    }
    else
    {
        if (timer > 0)
        {
            if (timer <= 25)
            {
                y--;
                if (timer <= 20)
                {
                    if (alpha <= 20) { alpha = 0; }
                    else { alpha -= 20; }
                }
            }
            timer--;
            if (timer <= 0) { delete this; }
        }
    }
}
