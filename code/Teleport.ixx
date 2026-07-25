module;
#include <SDL3/SDL.h>

export module Teleport;

import std;
import util;
import constVar;
import globalVar;
import drawPrimitive;
import World;
import Player;
import worldSession;
import Sector;

// ════════════════════════════════════════════════════════════════════════
// Teleport — 플레이어 텔레포트 통합 함수.
//
//   유스케이스: 디버그 콘솔, 향후 워프 게이트·스킬 텔레포트 등.
//
//   동작:
//     1) 월드젠 미완료 (시작 영역) → 로딩 화면 없이 직접 이동
//     2) 월드젠 완료 → 로딩 화면 표시 + 동기 ensure:
//          - 섹터 절차생성 (3×3, SectorCache::getOrCompute)
//          - 목적지 청크 선행 생성
//        → EntityPtrMove → setGrid → updateVision
//
//   비동기가 아닌 동기 처리인 이유: 텔레포트 직후 *즉시* 플레이어 주변 청크가
//   페인트되어야 함. 비동기로 두면 로딩 도중 화면 깨짐 / 미생성 청크 접근으로 throw.
//
//   로딩 화면: 검은 배경 + 우측 하단 회전 스피너 (WorldGenScreen 스타일).
// ════════════════════════════════════════════════════════════════════════

namespace teleport
{
    //우측 하단 회전 스피너 — 두 75° 호가 180° 간격으로 회전, 머리에서 꼬리 페이드.
    //  WorldGenScreen.ixx의 활성 단계 스피너와 동일 패턴.
    void drawSpinner(int cx, int cy)
    {
        constexpr float PI    = 3.14159265f;
        constexpr float TWOPI = 6.28318531f;
        constexpr float ARC   = 1.30f;          // ≈ 75°
        constexpr float SPIN  = 3.0f;           // rad/sec ≈ 0.48 회전/초
        constexpr float R_IN  = 12.0f;
        constexpr float R_OUT = 14.0f;
        constexpr int   N     = 36;             // 호당 라디얼 분할

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        const float t   = (float)SDL_GetTicks() / 1000.0f;
        const float rot = std::fmod(t * SPIN, TWOPI);
        const SDL_Color col{ 0xff, 0xff, 0xff, 0xff };

        for (int k = 0; k < 2; ++k)
        {
            const float head = rot + (float)k * PI;
            for (int s = 0; s < N; ++s)
            {
                const float frac = (float)s / (float)(N - 1);    // 0=머리, 1=꼬리
                const float ang  = head - frac * ARC;
                const float fade = 1.0f - frac;
                const Uint8 a = (Uint8)(245.0f * fade * fade);
                if (a < 6) continue;
                const float c = std::cos(ang), si = std::sin(ang);
                drawLine(
                    (int)std::round((float)cx + R_IN  * c),
                    (int)std::round((float)cy + R_IN  * si),
                    (int)std::round((float)cx + R_OUT * c),
                    (int)std::round((float)cy + R_OUT * si),
                    col, a);
            }
            //머리 끝 글로우 점
            const float c = std::cos(head), si = std::sin(head);
            const float rMid = (R_IN + R_OUT) * 0.5f;
            drawFillCircle(
                (int)std::round((float)cx + rMid * c),
                (int)std::round((float)cy + rMid * si),
                2, col, 220);
        }
    }

    //검은 배경 + 우측 하단 스피너 1프레임 렌더링 + present.
    void renderLoadingFrame()
    {
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
        SDL_RenderClear(renderer);

        constexpr int margin = 40;
        drawSpinner(cameraW - margin, cameraH - margin);

        SDL_RenderPresent(renderer);

        // 자체 present 후 메인 루프의 프레임 타겟으로 복귀 — 안 하면 이번 프레임의
        // 남은 그리기가 창에 직접 가서 endFrame 블릿에 덮여버림.
        SDL_SetRenderTarget(renderer, frameTarget);
    }

    //SDL 이벤트 큐 비우기 — Windows "응답 없음" 방지. QUIT 외엔 폐기.
    void pumpEvents()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            //로딩 중 QUIT 발생 시 별도 처리는 안 함 — 다음 프레임 메인 루프에서 처리됨.
        }
    }

    //섹터 3×3 절차생성 동기 ensure (SectorCache::getOrCompute).
    void ensureSectorsAround(Point3 dst, std::uint64_t seed, bool render)
    {
        const SectorCoord cur = sectorFromTile(dst);
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                SectorCache::ins().getOrCompute(
                    SectorCoord{ cur.x + dx, cur.y + dy, cur.z }, seed);
                if (render) { renderLoadingFrame(); pumpEvents(); }
            }
        }
    }

    //목적지 주변 3×3 청크 선행 생성. EntityPtrMove의 getTile이 throw 안 하도록.
    void ensureChunksAround(Point3 dst)
    {
        int tcx, tcy;
        World::ins()->changeToChunkCoord(dst.x, dst.y, tcx, tcy);
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (!World::ins()->existChunk(tcx + dx, tcy + dy, dst.z))
                {
                    World::ins()->createChunk(tcx + dx, tcy + dy, dst.z);
                }
            }
        }
    }
}

//플레이어를 dst로 텔레포트. 모든 사전 준비 (패치·섹터·청크) 동기 완료 후 이동.
//  월드젠 완료 후에는 로딩 화면 표시. 미완료 시(시작 영역) 즉시 이동.
export void teleportPlayer(Point3 dst)
{
    const bool worldgenDone = worldGenResult.has_value();

    //--- 로딩 화면 진입 (월드젠 후에만) ---
    if (worldgenDone)
    {
        teleport::renderLoadingFrame();
        teleport::pumpEvents();
    }

    //--- 1) 섹터 절차생성 동기 (3×3) — 월드젠 완료 시만 ---
    //   도시 CityPlan은 ensureSectorsAround 안의 procGenerate 4단계가 lazy 생성하므로
    //   별도 ensure 불필요.
    if (worldgenDone)
    {
        teleport::ensureSectorsAround(dst, worldSeed, true);
    }

    //--- 2) 목적지 청크 선행 생성 ---
    teleport::ensureChunksAround(dst);

    //--- 3) 이동 (EntityPtrMove → setGrid → updateNearbyChunk → ChunkPainter) ---
    EntityPtrMove(Point3{ PlayerX(), PlayerY(), PlayerZ() }, dst);

    //--- 4) 시야 갱신 ---
    PlayerPtr->updateVision(PlayerInfo().eyeSight);
}
