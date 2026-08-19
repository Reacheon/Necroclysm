module;
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

export module displayLoader;
import std;
export import globalVar;
import util;
import constVar;

namespace dispOption
{
    bool fullScreen = false;            // 풀스크린 여부
    bool useLetterbox = false;         // 풀스크린 시 레터박스 사용 여부 (창모드에서는 상관없음)
    bool fixScreenRatio = true;       // 창모드 시 비율 고정 여부 (전체화면에서는 상관없음)
    int resolutionPreset = 0;          // 비율 프리셋 (0~1) fixScreenRatio가 true일 때만 적용 (전체화면에서는 상관없음)
    double windowScale = 1.0;          // 창 크기 배율 (전체화면에서는 상관없음)
}

void recreateFrameTarget();

export void displayLoader()
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_DisplayID disp = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
    if (!mode) errorBox(L"디스플레이 정보를 읽는데 실패하였다.");

    int screenW = mode->w;
    int screenH = mode->h;
    int winW, winH;

    if (dispOption::fullScreen)
    {
        winW = screenW;
        winH = screenH;

        if (dispOption::useLetterbox)
        {
            cameraW = 1080;
            cameraH = 1080;
        }
        else
        {
            float ratio = (float)screenW / (float)screenH;

            if (ratio >= 1.0f)  // 가로 화면 (또는 정사각형)
            {
                cameraH = 1080;
                cameraW = (int)(ratio * 1080.0f);
            }
            else  // 세로 화면
            {
                cameraW = 1080;
                cameraH = (int)(1080.0f / ratio);
            }
        }
    }
    else  // 창모드
    {
        if (dispOption::fixScreenRatio)
        {
            switch (dispOption::resolutionPreset)
            {
            case 1:  // FHD (1920x1080)
                winW = (int)(1920 * dispOption::windowScale);
                winH = (int)(1080 * dispOption::windowScale);
                cameraW = 1920;
                cameraH = 1080;
                break;
            default: // 1:1 (1080x1080)
                winW = (int)(1080 * dispOption::windowScale);
                winH = (int)(1080 * dispOption::windowScale);
                cameraW = 1080;
                cameraH = 1080;
                break;
            }
        }
        else
        {
            // 자유 비율: 모니터 비율 유지, 스케일 적용
            cameraH = 1080;
            cameraW = (int)(((float)screenW / (float)screenH) * 1080.0f);
            winH = (int)(1080 * dispOption::windowScale);
            winW = (int)(((float)screenW / (float)screenH) * winH);
        }
    }

    if (!SDL_CreateWindowAndRenderer("Necroclysm", winW, winH,dispOption::fullScreen ? SDL_WINDOW_FULLSCREEN : 0,&window, &renderer))
        errorBox(L"창과 렌더러를 생성하는데 실패하였다.");

    // 타이틀바/작업표시줄 아이콘
    if (SDL_Surface* icon = IMG_Load("image/windowIcon.png"))
    {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }

    setPrimitiveRenderer(renderer);

    SDL_SetRenderLogicalPresentation(renderer, cameraW, cameraH,
        (dispOption::fullScreen && dispOption::useLetterbox)
        ? SDL_LOGICAL_PRESENTATION_LETTERBOX
        : SDL_LOGICAL_PRESENTATION_STRETCH);

    recreateFrameTarget();

    std::printf("Monitor: %dx%d | Window: %dx%d | Camera: %dx%d (%.2f:1) | %s %s\n",
        screenW, screenH, winW, winH, cameraW, cameraH,
        (float)cameraW / (float)cameraH,
        dispOption::fullScreen ? "Fullscreen" : "Windowed",
        (dispOption::fullScreen && dispOption::useLetterbox) ? "Letterbox" : "Stretch");
}

// 런타임 카메라 해상도 변경. 논리 해상도를 갈아끼우고
// 창모드면 창 크기도 모니터의 90% 안에 들어오도록 스케일해 재조정한다.
// 전체화면에서는 창 크기는 그대로 두고 논리 해상도만 갱신 (stretch로 채워짐).
export void applyResolution(int camW, int camH)
{
    // 해상도 규약: 기본 1080x1080에서 한 축만 늘어남 - 짧은 축은 반드시 1080
    if ((camW < camH ? camW : camH) != 1080)
    {
        std::printf("[debug] Resolution out of convention (shorter axis must be exactly 1080)\n");
        return;
    }

    cameraW = camW;
    cameraH = camH;

    SDL_SetRenderTarget(renderer, nullptr);

    if (dispOption::fullScreen == false)
    {
        double scale = dispOption::windowScale;
        SDL_DisplayID disp = SDL_GetPrimaryDisplay();
        const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
        if (mode)
        {
            double fitW = (double)mode->w * 0.9 / (double)camW;
            double fitH = (double)mode->h * 0.9 / (double)camH;
            if (fitW < scale) scale = fitW;
            if (fitH < scale) scale = fitH;
        }
        SDL_SetWindowSize(window, (int)(camW * scale), (int)(camH * scale));
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    SDL_SetRenderLogicalPresentation(renderer, cameraW, cameraH,
        (dispOption::fullScreen && dispOption::useLetterbox)
        ? SDL_LOGICAL_PRESENTATION_LETTERBOX
        : SDL_LOGICAL_PRESENTATION_STRETCH);

    recreateFrameTarget();
    // 중단된 프레임의 남은 그리기가 새 타겟으로 가도록 재바인딩 (메인 루프 끝 블릿이 처리)
    SDL_SetRenderTarget(renderer, frameTarget);

    std::printf("[debug] Camera: %dx%d (%.2f:1)\n", cameraW, cameraH, (float)cameraW / (float)cameraH);
}

void recreateFrameTarget()
{
    const bool wasBound = (frameTarget != nullptr && SDL_GetRenderTarget(renderer) == frameTarget);
    if (frameTarget != nullptr) SDL_DestroyTexture(frameTarget);
    frameTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cameraW, cameraH);
    SDL_SetTextureScaleMode(frameTarget, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(frameTarget, SDL_BLENDMODE_NONE);

    SDL_SetRenderTarget(renderer, frameTarget);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    if (wasBound == false) SDL_SetRenderTarget(renderer, nullptr);
}
