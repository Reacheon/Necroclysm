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


export void drawFlashEffect(Sprite* spr, int index, int x, int y, SDL_Color flash)
{
    if (flash.a == 0) return;

    struct TexCache { SDL_Texture* flashTex{}; int w{}, h{}; };
    static std::unordered_map<SDL_Texture*, TexCache> s_cache;
    static std::deque<SDL_Texture*>                   s_order;
    static constexpr std::size_t                      MAX_CACHE = 4000;

    SDL_Texture* srcTex = spr->getTexture();
    int w = spr->getW();
    int h = spr->getH();

    SDL_Texture* flashTex;
    auto it = s_cache.find(srcTex);

    if (it == s_cache.end())
    {
        flashTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        if (!flashTex) return;
        SDL_SetTextureScaleMode(flashTex, SDL_SCALEMODE_NEAREST);

        s_cache[srcTex] = { flashTex, w, h };
        s_order.push_back(srcTex);

        if (s_order.size() > MAX_CACHE)
        {
            SDL_Texture* victim = s_order.front();
            s_order.pop_front();
            auto vit = s_cache.find(victim);
            if (vit != s_cache.end())
            {
                SDL_DestroyTexture(vit->second.flashTex);
                s_cache.erase(vit);
            }
        }
    }
    else flashTex = it->second.flashTex;

    // 현재 렌더 타겟과 설정 저장
    SDL_Texture* prevTarget = SDL_GetRenderTarget(renderer);
    SDL_BlendMode prevBlend;
    SDL_GetRenderDrawBlendMode(renderer, &prevBlend);
    SDL_FlipMode prevFlip = s_flip;
    float prevZoom = s_zoomScale;

    // 플래시 텍스처를 렌더 타겟으로 설정
    SDL_SetRenderTarget(renderer, flashTex);
    SDL_SetTextureBlendMode(flashTex, SDL_BLENDMODE_BLEND);

    // 1. 먼저 완전히 투명하게 클리어
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // 2. 원하는 색상으로 전체를 채움
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, flash.r, flash.g, flash.b, 255);
    SDL_FRect full = { 0, 0, static_cast<float>(w), static_cast<float>(h) };
    SDL_RenderFillRect(renderer, &full);

    // 3. 원본 스프라이트의 알파를 DST_IN 블렌드로 적용
    // DST_IN: dst.rgb = dst.rgb, dst.a = dst.a * src.a
    SDL_SetTextureBlendMode(spr->getTexture(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 커스텀 블렌드 모드 설정 (DST_IN 효과)
    SDL_BlendMode customBlend = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO,           // srcColorFactor
        SDL_BLENDFACTOR_ONE,            // dstColorFactor  
        SDL_BLENDOPERATION_ADD,         // colorOperation
        SDL_BLENDFACTOR_ZERO,           // srcAlphaFactor
        SDL_BLENDFACTOR_SRC_ALPHA,      // dstAlphaFactor
        SDL_BLENDOPERATION_ADD          // alphaOperation
    );
    SDL_SetTextureBlendMode(spr->getTexture(), customBlend);

    setZoom(1.0f);
    setFlip(SDL_FLIP_NONE);

    // 원본 텍스처 렌더링 (알파 채널만 적용됨)
    float textureW, textureH;
    SDL_GetTextureSize(spr->getTexture(), &textureW, &textureH);
    SDL_FRect src = { (spr->getW() * index) % static_cast<int>(textureW),
                     spr->getH() * ((spr->getW() * index) / static_cast<int>(textureW)),
                     static_cast<float>(spr->getW()),
                     static_cast<float>(spr->getH()) };
    SDL_FRect dst = { 0, 0, static_cast<float>(w), static_cast<float>(h) };
    SDL_RenderTexture(renderer, spr->getTexture(), &src, &dst);

    // 원본 텍스처의 블렌드 모드 복원
    SDL_SetTextureBlendMode(spr->getTexture(), SDL_BLENDMODE_BLEND);

    // 렌더 타겟 복원
    SDL_SetRenderTarget(renderer, prevTarget);
    SDL_SetRenderDrawBlendMode(renderer, prevBlend);
    setZoom(prevZoom);
    setFlip(prevFlip);

    // 최종 플래시 텍스처 렌더링 (알파 적용)
    SDL_SetTextureAlphaMod(flashTex, flash.a);
    drawTexture(flashTex, x, y);
    SDL_SetTextureAlphaMod(flashTex, 255);
}

export void drawFlashEffectCenter(Sprite* spr, int index, int x, int y, SDL_Color flash)
{
    float dstW = spr->getW() * s_zoomScale;
    float dstH = spr->getH() * s_zoomScale;
    int left = static_cast<int>(x - dstW * 0.5f);
    int top = static_cast<int>(y - dstH * 0.5f);

    drawFlashEffect(spr, index, left, top, flash);
}