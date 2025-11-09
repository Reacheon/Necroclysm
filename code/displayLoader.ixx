#include <SDL3/SDL.h>

export module displayLoader;
export import globalVar;

import util;
import constVar;

export void displayLoader()
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_DisplayID disp = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
    if (!mode) errorBox(L"디스플레이 정보를 읽을 수 없습니다.");

    int winW = mode->w;
    int winH = mode->h;

    // 논리적 해상도는 항상 1:1 비율 (1080x1080)로 고정
    cameraW = 1080;
    cameraH = 1080;

    option::fullScreen = false;

    // 풀스크린이 아닐 때는 창 크기 조정 가능
    if (!option::fullScreen)
    {
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
        }
        else
        {
            // 윈도우 모드일 때 적당한 크기로
            winW = 1080;
            winH = 1080;
        }
    }
    // 풀스크린일 때는 모니터 해상도 그대로 사용 (이미 winW, winH에 설정됨)

    if (!SDL_CreateWindowAndRenderer("Necroclysm", winW, winH,
        option::fullScreen ? SDL_WINDOW_FULLSCREEN : 0,
        &window, &renderer))
        errorBox(L"창·렌더러 생성 실패");
    setPrimitiveRenderer(renderer);

    // 논리적 해상도는 1:1 비율 유지, 레터박스/필러박스 자동 추가
    SDL_SetRenderLogicalPresentation(renderer,
        cameraW,
        cameraH,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
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