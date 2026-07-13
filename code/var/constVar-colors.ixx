module;
#include <SDL3/SDL.h>

export module constVar:colors;

export namespace mulCol
{
    //시각(0~24시, 실수) → 시간대별 곱셈(MUL) 틴트 색. 알파=틴트 강도. 밤=남색(곱셈이라 R·G는 0쪽으로
    //  깎이고 파랑만 남음), 18시=노을(따뜻한 갈색), 10~17시=투명(낮, 틴트 없음). 본체 월드 렌더
    //  (renderTile::drawMulFogs)와 월드맵(Map::drawNightOverlay)이 공유하는 단일 팔레트 — 한 곳만
    //  고치면 양쪽에 반영된다.
    inline SDL_Color ambientMulColorAt(float timeOfDay)
    {
        struct TimeColor { float time; SDL_Color color; };
        static constexpr TimeColor timeColors[] =
        {
            { 0.0f,  {   0,  0,  59, 150 } },
            { 6.0f,  {   0,  0,  59, 150 } },
            { 8.0f,  {   0,  0,  49,  50 } },
            { 10.0f, {   0,  0,   0,   0 } },
            { 17.0f, {   0,  0,   0,   0 } },
            { 18.0f, { 121, 78,  59, 130 } },
            { 18.5f, {   0,  0,  59, 150 } },
            { 24.0f, {   0,  0,  59, 150 } },
        };
        constexpr int n = (int)(sizeof(timeColors) / sizeof(timeColors[0]));
        for (int i = 0; i < n - 1; ++i)
        {
            if (timeOfDay >= timeColors[i].time && timeOfDay < timeColors[i + 1].time)
            {
                const float t1 = timeColors[i].time, t2 = timeColors[i + 1].time;
                const float ratio = (timeOfDay - t1) / (t2 - t1);
                const SDL_Color& c1 = timeColors[i].color;
                const SDL_Color& c2 = timeColors[i + 1].color;
                return {
                    (Uint8)(c1.r + (c2.r - c1.r) * ratio),
                    (Uint8)(c1.g + (c2.g - c1.g) * ratio),
                    (Uint8)(c1.b + (c2.b - c1.b) * ratio),
                    (Uint8)(c1.a + (c2.a - c1.a) * ratio)
                };
            }
        }
        return { 0, 0, 0, 0 };   //범위 밖(예: 정확히 24.0) — 틴트 없음
    }
}

export namespace col
{
    constexpr SDL_Color black = { 0x00, 0x00, 0x00 };
    constexpr SDL_Color yellow = { 0xff,0xff,0x00 };
    constexpr SDL_Color brown = { 0x5c,0x33,0x17 };
    constexpr SDL_Color gray = { 0x63,0x63,0x63 };
    constexpr SDL_Color green = { 0x00,0x6e,0x00 };
    constexpr SDL_Color blueberry = { 0x64,0x64,0xff };
    constexpr SDL_Color red = { 0xf9,0x29,0x29 };
    constexpr SDL_Color white = { 0xff,0xff,0xff };
    constexpr SDL_Color lightGray = { 0x96,0x96,0x96 };
    constexpr SDL_Color blue = { 0x21,0x4a,0xea };
    constexpr SDL_Color yellowGreen = { 0x3a, 0xf5, 0x43 };
    constexpr SDL_Color monaLisa = { 0xff,0x96,0x96 };
    constexpr SDL_Color bondiBlue = { 0x00,0x96,0xb4 };
    constexpr SDL_Color hotPink = { 0x8b,0x3a,0x62 };
    constexpr SDL_Color pink = { 0xfe,0x00,0xfe };
    constexpr SDL_Color skyBlue = { 0x00,0xf0,0xff };
    constexpr SDL_Color blueDart = { 0x4e,0x8e,0xd2 };
    constexpr SDL_Color orange = { 0xf2, 0x65, 0x22 };
    constexpr SDL_Color cyan = { 0x00,0xa3,0xd2 };
};

export namespace lowCol
{
    constexpr SDL_Color black = { 0x00,0x00,0x00 };
    constexpr SDL_Color white = { 0xff,0xff,0xff };
    constexpr SDL_Color red = { 0xd0,0x3f,0x3f };
    constexpr SDL_Color orange = { 0xd0,0x7a,0x3f };
    constexpr SDL_Color yellow = { 0xd0,0xc3,0x3f };
    constexpr SDL_Color green = { 0x75,0xd0,0x3f };
    constexpr SDL_Color mint = { 0x3f,0xd0,0x7f };
    constexpr SDL_Color skyBlue = { 0x3f,0xba,0xd0 };
    constexpr SDL_Color deepBlue = { 0x20,0x50,0xa8 };
    constexpr SDL_Color blue = { 0x2b,0x81,0xe8 };
    constexpr SDL_Color purple = { 0x43,0x3e,0x8e };
    constexpr SDL_Color pink = { 0xbe,0x3f,0xd0 };
    constexpr SDL_Color crimson = { 0xd0,0x3f,0x89 };
};

// pngPatchPixelCol namespace 제거됨 — Patch 시스템 폐지로 PNG 색상 매핑은 worldGrid 모듈
// 내부(worldGrid_load.cpp의 colorToTerrain)에서 직접 처리.