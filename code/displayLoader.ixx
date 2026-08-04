module;
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

export module displayLoader;
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

// 프레임 렌더타겟 (재)생성 — cameraW×cameraH 1:1 버퍼. 모든 그리기는 여기에 정수 좌표
// 그대로 이뤄지고, endFrame에서 전체를 한 번만 창 크기로 스케일한다. 드로우 콜마다
// 개별 스케일될 때 생기는 타일 경계 반올림 어긋남(비정수 배율 가로선)이 원천 차단됨.
void recreateFrameTarget()
{
    // 디버그 콘솔은 프레임 도중(구 타겟 바인딩 상태)에 실행되므로, 파괴 전에 바인딩
    // 여부를 기억해뒀다가 새 타겟으로 갈아끼운다 — 파괴된 포인터로 복귀하면 안 됨.
    const bool wasBound = (frameTarget != nullptr && SDL_GetRenderTarget(renderer) == frameTarget);
    if (frameTarget != nullptr) SDL_DestroyTexture(frameTarget);
    frameTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cameraW, cameraH);
    if (frameTarget == nullptr) errorBox(L"프레임 렌더타겟 생성 실패");
    SDL_SetTextureScaleMode(frameTarget, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(frameTarget, SDL_BLENDMODE_NONE);

    // 생성 직후 내용은 미정의 — 첫 프레임 쓰레기 픽셀 방지용 1회 클리어
    SDL_SetRenderTarget(renderer, frameTarget);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    if (wasBound == false) SDL_SetRenderTarget(renderer, nullptr);
}

// 메인 루프 프레임 시작: 이후의 모든 그리기를 프레임 타겟으로 보낸다.
export void beginFrame()
{
    if (frameTarget != nullptr) SDL_SetRenderTarget(renderer, frameTarget);
}

// 메인 루프 프레임 끝(present 직전): 프레임 타겟을 창에 단일 블릿.
export void endFrame()
{
    if (frameTarget == nullptr) return;
    SDL_SetRenderTarget(renderer, nullptr);
    const SDL_FRect dst = { 0.0f, 0.0f, (float)cameraW, (float)cameraH };
    SDL_RenderTexture(renderer, frameTarget, nullptr, &dst);
}

export void displayLoader()
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_DisplayID disp = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp);
    if (!mode) errorBox(L"디스플레이 정보를 읽을 수 없습니다.");

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

    if (!SDL_CreateWindowAndRenderer("Necroclysm", winW, winH,
        dispOption::fullScreen ? SDL_WINDOW_FULLSCREEN : 0,
        &window, &renderer))
        errorBox(L"창·렌더러 생성 실패");

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

    //letterbox = { 0, 0, 782, 176 };
    //letterbox.x = (cameraW - letterbox.w) / 2;
    //letterbox.y = cameraH - letterbox.h + 6;

    //for (int i = 0; i < 7; ++i)
    //    barButton[i] = { cameraW / 2 - 300 + 88 * i, cameraH - 80, 72, 72 };

    //letterboxPopUpButton = { letterbox.x + letterbox.w - 39,
    //                         letterbox.y - 33,
    //                         29, 29 };
}

// 런타임 카메라 해상도 변경 (디버그 콘솔용). 논리 해상도를 갈아끼우고
// 창모드면 창 크기도 모니터의 90% 안에 들어오도록 스케일해 재조정한다.
// 전체화면에서는 창 크기는 그대로 두고 논리 해상도만 갱신 (stretch로 채워짐).
export void applyDebugResolution(int camW, int camH)
{
    // 해상도 규약: 기본 1080x1080에서 한 축만 늘어남 - 짧은 축은 반드시 1080
    if ((camW < camH ? camW : camH) != 1080)
    {
        std::printf("[debug] Resolution out of convention (shorter axis must be exactly 1080)\n");
        return;
    }

    cameraW = camW;
    cameraH = camH;

    // 렌더타겟이 바인딩된 채로 SDL_SetRenderLogicalPresentation을 부르면 SDL이 출력
    // 크기를 창이 아니라 타겟 텍스처 기준으로 계산해 잘못된 배율이 캐시됨 — 먼저 해제.
    SDL_SetRenderTarget(renderer, nullptr);

    if (dispOption::fullScreen == false)
    {
        // 모니터의 90% 안에 들어오도록 창 크기 축소 — 비정수 배율 테스트 겸용
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
    // 중단된 프레임의 남은 그리기가 새 타겟으로 가도록 재바인딩 (endFrame이 어차피 블릿함)
    SDL_SetRenderTarget(renderer, frameTarget);

    std::printf("[debug] Camera: %dx%d (%.2f:1)\n", cameraW, cameraH, (float)cameraW / (float)cameraH);
}