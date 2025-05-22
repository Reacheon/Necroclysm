#include <SDL3/SDL.h>

export module displayLoader;

export import globalVar;

import util;
import constVar;

export void displayLoader()
{
    /* ────────────────────── 게임 논리 해상도 ────────────────────── */
    cameraW = 720;
    cameraH = 720;           // 1:1 픽셀아트 좌표계는 항상 720×720

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");


    /* ───────────── 실제 창(모니터) 해상도 결정 ───────────── */
    SDL_DisplayID disp = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
    if (!mode) errorBox(L"디스플레이 정보를 읽을 수 없습니다.");

    int winW = mode->w;
    int winH = mode->h;

    if (option::fixScreenRatio)
    {
        switch (0)          // 필요 시 프리셋 변경
        {
        default:  winW = 720;   winH = 720;   break;  // 1:1
        case 1:   winW = 720;   winH = 1280;  break;  // 9:16
        case 2:   winW = 960;   winH = 720;   break;  // 4:3
        case 3:   winW = 720;   winH = 1440;  break;  // 1:2
        case 4:   winW = 1280;  winH = 720;   break;  // 16:9
        }
    }

    /* ───────────── 창·렌더러 생성 ───────────── */
    if (!SDL_CreateWindowAndRenderer("Necroclysm", winW, winH,
        option::fullScreen ? SDL_WINDOW_FULLSCREEN : 0,
        &window, &renderer))
        errorBox(L"창·렌더러 생성 실패");

    setPrimitiveRenderer(renderer);   // (기존 코드)

    /* ───────────── 논리 프레젠테이션 설정 ─────────────
       720×720 좌표계를 창 크기에 맞게 **늘이기(STRETCH)**.
       픽셀 아트를 보존하려면 STRETCH 대신 LETTERBOX + SDL_RenderSetIntegerScale()
       를 쓰면 됩니다. */
    SDL_SetRenderLogicalPresentation(renderer,
        cameraW,          // 논리 폭
        cameraH,          // 논리 높이
        SDL_LOGICAL_PRESENTATION_STRETCH);

    // 필요 시 정수배 스케일 고정
    // SDL_RenderSetIntegerScale(renderer, true);

    /* ───────────── HUD 좌표 등 초기화 ───────────── */
    letterbox = { 0, 0, 630, 140 };
    letterbox.x = (cameraW - letterbox.w) / 2;
    letterbox.y = cameraH - letterbox.h + 6;

    for (int i = 0; i < 7; ++i)
        barButton[i] = { cameraW / 2 - 300 + 88 * i, cameraH - 80, 72, 72 };

    letterboxPopUpButton = { letterbox.x + letterbox.w - 39,
                             letterbox.y - 33,
                             29, 29 };
}