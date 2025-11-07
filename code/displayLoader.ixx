#include <SDL3/SDL.h>

export module displayLoader;
export import globalVar;

import util;
import constVar;

export void displayLoader()
{
    cameraW = 1080;
    cameraH = 1080;
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_DisplayID disp = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
    if (!mode) errorBox(L"디스플레이 정보를 읽을 수 없습니다.");
    int winW = mode->w;
    int winH = mode->h;

    // 2배 확대 적용
    option::fullScreen = false;
    if (option::fullScreen) option::fixScreenRatio = false;



    double scaleFactor = 1.0;
    if (option::fixScreenRatio)
    {
        switch (1)
        {
        default:  winW = 1080 * scaleFactor;   winH = 1080 * scaleFactor;   break;
        case 1:   winW = 1920 * scaleFactor;   winH = 1080 * scaleFactor;  break;
        case 2:   winW = 960 * scaleFactor;   winH = 1080 * scaleFactor;   break;
        case 3:   winW = 1080 * scaleFactor;   winH = 1440 * scaleFactor;  break;
        case 4:   winW = 1280 * scaleFactor;  winH = 1080 * scaleFactor;   break;
        }
        cameraW = winW / scaleFactor;  // 게임 해상도는 원래대로
        cameraH = winH / scaleFactor;
    }
    else
    {
        // 윈도우 크기만 2배로 확대
        winW = winW;  // 모니터 해상도 그대로 사용하거나
        winH = winH;
        // 또는 고정 크기로 2배 확대하려면:
        // winW = 1080 * scaleFactor;
        // winH = 1080 * scaleFactor;

        cameraW = winW;
        cameraH = winH;
        if (cameraW > cameraH)
        {
            cameraW = (int)(((float)cameraW / (float)cameraH) * 1080.0f);
            cameraH = 1080;
        }
        else if (cameraH > cameraW)
        {
            cameraH = (int)(((float)cameraH / (float)cameraW) * 1080.0f);
            cameraW = 1080;
        }
        else
        {
            cameraW = 1080;
            cameraH = 1080;
        }
    }

    if (!SDL_CreateWindowAndRenderer("Necroclysm", winW, winH,
        option::fullScreen ? SDL_WINDOW_FULLSCREEN : 0,
        &window, &renderer))
        errorBox(L"창·렌더러 생성 실패");
    setPrimitiveRenderer(renderer);

    // 논리적 해상도는 원래대로 유지 (1080x1080)
    SDL_SetRenderLogicalPresentation(renderer,
        cameraW,
        cameraH,
        SDL_LOGICAL_PRESENTATION_STRETCH);
    {
        SDL_DisplayID disp = SDL_GetPrimaryDisplay();
        const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
        if (mode)
        {
            int screenW = mode->w;
            int screenH = mode->h;
            std::printf("Monitor size: %d x %d\n", screenW, screenH);
            std::printf("Window size: %d x %d\n", winW, winH);
            std::printf("Logical size: %d x %d\n", cameraW, cameraH);
        }
    }
    letterbox = { 0, 0, 630, 140 };
    letterbox.x = (cameraW - letterbox.w) / 2;
    letterbox.y = cameraH - letterbox.h + 6;
    for (int i = 0; i < 7; ++i)
        barButton[i] = { cameraW / 2 - 300 + 88 * i, cameraH - 80, 72, 72 };
    letterboxPopUpButton = { letterbox.x + letterbox.w - 39,
                             letterbox.y - 33,
                             29, 29 };
}