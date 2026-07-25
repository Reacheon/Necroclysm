module;

#include <SDL3/SDL.h>

// 너브드라이브(신경 가속 바이오닉) 발동 중에 적용되는 풀스크린 초록 틴트 필터.
//
// 설계 의도
//  - 기존 렌더 파이프라인(renderTile/renderWeather/renderSticker/renderUI/renderLog)을
//    최소한으로만 건드려서, 이 모듈만 제거하면 필터가 전부 사라지도록 캡슐화.
//  - 월드 렌더링은 오프스크린 RT에 먼저 그린 뒤, 디폴트 타겟으로 복귀할 때 초록
//    틴트를 걸어 블릿한다. 그 위에 플레이어(+잔상)를 원색으로 덧그린다.
//  - 플레이어 skip은 내부 플래그로 관리되며 drawEntities()가 shouldSkipPlayerInWorld()
//    한 줄로 조회한다. 그 외 기존 코드는 수정되지 않는다.
//
// 사용 순서 (main 루프)
//    const bool filtering = nervedriveFilter::beginWorldPass();   // nervedriveOn이면 RT로 우회
//    renderTile(); renderWeather(); renderSticker();              // 기존 호출 그대로
//    if (filtering) {
//        nervedriveFilter::endWorldPassAndBlit();                  // 틴트 걸어 블릿
//        PlayerPtr->drawSelf();                                    // 플레이어를 원색으로 덧그림
//    }
//    renderUI(); renderLog();                                     // UI는 언제나 원색 최상단
//
// 주의: beginWorldPass() 이전 어느 시점엔가 renderer/cameraW/cameraH가 준비된 뒤
//       init()이 호출되어 있어야 한다 (textureLoader 시점).

export module nervedriveFilter;

import globalVar;

namespace
{
    SDL_Texture* g_worldRT = nullptr;
    bool g_skipPlayer = false;   // 월드패스 중 drawEntities에서 플레이어 drawSelf 스킵 여부
    bool g_activePass = false;   // begin 성공 후 end 대기 중인지 여부

    // 원한다면 여기 상수만 바꿔서 다른 필터 색조로 재활용 가능.
    constexpr Uint8 TINT_R = 80;
    constexpr Uint8 TINT_G = 255;
    constexpr Uint8 TINT_B = 80;
}

export namespace nervedriveFilter
{
    // 오프스크린 RT 생성. 텍스처 로더에서 1회 호출.
    // renderer와 cameraW/cameraH가 유효한 이후에 호출되어야 한다.
    void init()
    {
        if (g_worldRT != nullptr) return;
        g_worldRT = SDL_CreateTexture(renderer,
                                      SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET,
                                      cameraW, cameraH);
        if (g_worldRT == nullptr) return;
        SDL_SetTextureScaleMode(g_worldRT, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(g_worldRT, SDL_BLENDMODE_BLEND);
    }

    // RT 해제. 종료 시 호출. 실제로 main.cpp는 OS 종료에 맡기므로 필수는 아님.
    void shutdown()
    {
        if (g_worldRT != nullptr)
        {
            SDL_DestroyTexture(g_worldRT);
            g_worldRT = nullptr;
        }
    }

    // drawEntities()가 매 엔티티마다 조회하는 힌트.
    // true면 플레이어의 drawSelf()를 월드패스에서 건너뛰어야 한다.
    bool shouldSkipPlayerInWorld()
    {
        return g_skipPlayer;
    }

    // 월드패스 시작. nervedriveOn이면 렌더타겟을 오프스크린 RT로 바꾸고 true 반환.
    // 반환값이 true일 때만 endWorldPassAndBlit()을 호출해야 한다.
    // 반환값이 false면 필터가 비활성이므로 호출자는 평소와 동일하게 렌더링하면 됨.
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

    // 월드패스 종료: 디폴트 타겟으로 복귀 후 RT를 초록 틴트로 블릿.
    // 이 함수가 리턴되면 이후 드로우는 디폴트 타겟(화면)에 원색으로 들어간다.
    // beginWorldPass()가 true를 반환했을 때에만 호출해야 함.
    void endWorldPassAndBlit()
    {
        if (!g_activePass) return;

        g_skipPlayer = false;
        g_activePass = false;

        SDL_SetRenderTarget(renderer, frameTarget);

        SDL_SetTextureBlendMode(g_worldRT, SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(g_worldRT, TINT_R, TINT_G, TINT_B);

        const SDL_FRect dst = { 0.0f, 0.0f,
                                static_cast<float>(cameraW),
                                static_cast<float>(cameraH) };
        SDL_RenderTexture(renderer, g_worldRT, nullptr, &dst);

        // 다음 호출자(다른 용도로 재사용될 가능성)를 위해 상태 복원.
        SDL_SetTextureColorMod(g_worldRT, 255, 255, 255);
    }
}
