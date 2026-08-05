module;

#include <SDL3/SDL.h>

export module nervedriveFilter;

import globalVar;

namespace
{
    SDL_Texture* g_worldRT = nullptr;
    bool g_skipPlayer = false;
    bool g_activePass = false;

    constexpr Uint8 TINT_R = 80;
    constexpr Uint8 TINT_G = 255;
    constexpr Uint8 TINT_B = 80;
}

export namespace nervedriveFilter
{
    void init()
    {
        if (g_worldRT != nullptr) return;
        g_worldRT = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cameraW, cameraH);
        if (g_worldRT == nullptr) return;
        SDL_SetTextureScaleMode(g_worldRT, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(g_worldRT, SDL_BLENDMODE_BLEND);
    }

    void shutdown()
    {
        if (g_worldRT != nullptr)
        {
            SDL_DestroyTexture(g_worldRT);
            g_worldRT = nullptr;
        }
    }

    bool shouldSkipPlayerInWorld()
    {
        return g_skipPlayer;
    }

    bool beginWorldPass()
    {
        if (!nervedriveOn) return false;
        if (g_worldRT == nullptr) return false;

        SDL_SetRenderTarget(renderer, g_worldRT);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        g_skipPlayer = true;
        g_activePass = true;
        return true;
    }

    void endWorldPassAndBlit()
    {
        if (!g_activePass) return;

        g_skipPlayer = false;
        g_activePass = false;

        SDL_SetRenderTarget(renderer, frameTarget);

        SDL_SetTextureBlendMode(g_worldRT, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(g_worldRT, TINT_R, TINT_G, TINT_B);

        const SDL_FRect dst = { 0.0f, 0.0f,static_cast<float>(cameraW),static_cast<float>(cameraH) };
        SDL_RenderTexture(renderer, g_worldRT, nullptr, &dst);

        SDL_SetTextureColorMod(g_worldRT, 255, 255, 255);
    }
}
