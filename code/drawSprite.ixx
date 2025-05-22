#include <SDL3/SDL.h>
#include <cmath>

export module drawSprite;

import Sprite;
import globalVar;

static float s_zoomScale = 1.0f;
static SDL_FlipMode s_flip = SDL_FLIP_NONE;
static int s_angle = 0;
static SDL_Point s_anglePoint = { 0, 0 };
static unsigned __int8 s_alpha = 255;

export void setAngle(int angle, SDL_Point anglePoint) { s_angle = angle; s_anglePoint = anglePoint; }
export void setFlip(SDL_FlipMode flip) { s_flip = flip; }
export void setZoom(float scale) { s_zoomScale = scale; }
export void setAlpha(float val) { s_alpha = val; }

export void drawTexture(SDL_Texture* texture, int x, int y)
{
    float textureW, textureH;
    SDL_GetTextureSize(texture, &textureW, &textureH);
    SDL_FRect src = { 0, 0, static_cast<int>(textureW), static_cast<int>(textureH) };
    SDL_FRect dst = { static_cast<float>(x),
                      static_cast<float>(y),
                      textureW * s_zoomScale,
                      textureH * s_zoomScale };
    SDL_RenderTextureRotated(renderer, texture, &src, &dst, s_angle, nullptr, s_flip);
}

export void drawTextureCenter(SDL_Texture* texture, int x, int y)
{
    float textureW, textureH;
    SDL_GetTextureSize(texture, &textureW, &textureH);
    SDL_FRect src = { 0, 0, static_cast<int>(textureW), static_cast<int>(textureH) };
    SDL_FRect dst = { static_cast<float>(x) - textureW * s_zoomScale * 0.5f,
                      static_cast<float>(y) - textureH * s_zoomScale * 0.5f,
                      textureW * s_zoomScale,
                      textureH * s_zoomScale };
    SDL_RenderTextureRotated(renderer, texture, &src, &dst, s_angle, nullptr, s_flip);
}

export void drawSprite(Sprite* spr, int index, int x, int y)
{
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);
    SDL_FRect src = { (spr->getW() * index) % static_cast<int>(textureW),
                     spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW)),
                     spr->getW(),
                     spr->getH() };
    SDL_FRect dst = { static_cast<float>(x),
                      static_cast<float>(y),
                      src.w * s_zoomScale,
                      src.h * s_zoomScale };
    SDL_RenderTextureRotated(renderer, spr->getTexture(), &src, &dst, 0, nullptr, s_flip);
}

export void drawSpriteRotate(Sprite* spr, int index, int x, int y, double rotateAngle, SDL_Point* rotateCenter)
{
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);

    SDL_FRect src = { (spr->getW() * index) % static_cast<int>(textureW),
                     spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW)),
                     spr->getW(),
                     spr->getH() };

    SDL_FRect dst = { static_cast<float>(x),
                      static_cast<float>(y),
                      src.w * s_zoomScale,
                      src.h * s_zoomScale };

    SDL_FPoint* centerF = nullptr;
    SDL_FPoint centerBuf;
    if (rotateCenter)
    {
        centerBuf = { static_cast<float>(rotateCenter->x), static_cast<float>(rotateCenter->y) };
        centerF = &centerBuf;
    }

    SDL_RenderTextureRotated(renderer, spr->getTexture(), &src, &dst, rotateAngle, centerF, s_flip);
}

export void drawSprite(Sprite* spr, int x, int y) { drawSprite(spr, 0, x, y); }

export void drawSpriteCenter(Sprite* spr, int index, int x, int y)
{
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);

    SDL_FRect src = { (spr->getW() * index) % static_cast<int>(textureW),
                     spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW)),
                     spr->getW(),
                     spr->getH() };

    float dstW = src.w * s_zoomScale;
    float dstH = src.h * s_zoomScale;

    SDL_FRect dst = { static_cast<float>(x) - dstW * 0.5f,
                      static_cast<float>(y) - dstH * 0.5f,
                      dstW,
                      dstH };

    SDL_RenderTextureRotated(renderer, spr->getTexture(), &src, &dst, 0, nullptr, s_flip);
}

export void drawSpriteCenterRotate(Sprite* spr, int index, int x, int y, double rotateAngle, SDL_Point* rotateCenter)
{
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);

    SDL_FRect src = { (spr->getW() * index) % static_cast<int>(textureW),
                     spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW)),
                     spr->getW(),
                     spr->getH() };

    float dstW = src.w * s_zoomScale;
    float dstH = src.h * s_zoomScale;

    SDL_FRect dst = { static_cast<float>(x) - dstW * 0.5f,
                      static_cast<float>(y) - dstH * 0.5f,
                      dstW,
                      dstH };

    SDL_FPoint centerBuf;
    SDL_FPoint* centerF;
    if (rotateCenter)
    {
        centerBuf = { static_cast<float>(rotateCenter->x), static_cast<float>(rotateCenter->y) };
        centerF = &centerBuf;
    }
    else
    {
        centerBuf = { dstW * 0.5f, dstH * 0.5f };
        centerF = &centerBuf;
    }

    SDL_RenderTextureRotated(renderer, spr->getTexture(), &src, &dst, rotateAngle, centerF, s_flip);
}

export void drawSpriteCenterExSrc(Sprite* spr, int index, int x, int y, SDL_Rect exSrc)
{
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);

    int srcX = (spr->getW() * index) % static_cast<int>(textureW);
    int srcY = spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW));
    SDL_FRect src = { srcX + exSrc.x, srcY + exSrc.y, exSrc.w, exSrc.h };

    float dstW = src.w * s_zoomScale;
    float dstH = src.h * s_zoomScale;

    double offsetX = (exSrc.x + exSrc.w * 0.5) - (spr->getW() * 0.5);
    double offsetY = (exSrc.y + exSrc.h * 0.5) - (spr->getH() * 0.5);
    float dstOffsetX = static_cast<float>(offsetX * s_zoomScale);
    float dstOffsetY = static_cast<float>(offsetY * s_zoomScale);

    SDL_FRect dst = { static_cast<float>(x) - dstW * 0.5f + dstOffsetX,
                      static_cast<float>(y) - dstH * 0.5f + dstOffsetY,
                      dstW,
                      dstH };

    SDL_RenderTextureRotated(renderer, spr->getTexture(), &src, &dst, 0, nullptr, s_flip);
}

export void drawSpriteCenterF(Sprite* spr, int index, double x, double y)
{
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);

    SDL_FRect src = { (spr->getW() * index) % static_cast<int>(textureW),
                     spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW)),
                     spr->getW(),
                     spr->getH() };

    SDL_FRect dst = { static_cast<float>(x) - src.w * s_zoomScale * 0.5f,
                      static_cast<float>(y) - src.h * s_zoomScale * 0.5f,
                      src.w * s_zoomScale,
                      src.h * s_zoomScale };

    SDL_RenderTextureRotated(renderer, spr->getTexture(), &src, &dst, 0, nullptr, s_flip);
}
