module;
#include <SDL3/SDL.h>

module Damage;

import globalVar;
import drawText;
import util;

std::vector<Damage*> Damage::list;

// 데미지 폰트 설정
static constexpr int DMG_FONT_SIZE = 12;
static const SDL_Color DMG_SHADOW_COL = { 0x29, 0x29, 0x29, 0xff }; // 회색 그림자

// ── 일반 데미지 (공작새 궤적) ──
static constexpr float DMG_SPEED_MIN  = 0.9f;
static constexpr float DMG_SPEED_MAX  = 1.4f;
static constexpr float DMG_FRICTION   = 0.94f;
static constexpr float DMG_ANGLE_MIN  = 255.0f;
static constexpr float DMG_ANGLE_MAX  = 285.0f;
static constexpr int   DMG_TIMER      = 55;
static constexpr int   DMG_FADE_START = 20;
static constexpr int   DMG_FADE_SPEED = 10;

// ── MISS (수직 부유) ──
static constexpr float MISS_RISE_SPEED = -0.4f;   // 매 프레임 y 이동량 (음수=위)
static constexpr int   MISS_TIMER      = 45;
static constexpr int   MISS_FADE_START = 22;
static constexpr int   MISS_FADE_SPEED = 12;

// ── BLOCK (퉁! 점프 바운스) ──
static constexpr float BLOCK_JUMP_VEL  = -0.7f;   // 초기 y속도 (살짝 위로)
static constexpr float BLOCK_GRAVITY   = 0.07f;    // 중력 (느리게 복귀)
static constexpr int   BLOCK_TIMER     = 40;
static constexpr int   BLOCK_FADE_START = 12;
static constexpr int   BLOCK_FADE_SPEED = 22;
static const SDL_Color BLOCK_COLOR = { 0x55, 0xBB, 0xFF, 0xFF }; // 하늘색

// 픽셀폰트 + 그림자 + 8방위 외곽선으로 데미지 텍스처 생성
static Sprite* createDmgSprite(const std::wstring& text, SDL_Color mainCol)
{
    setFont(fontType::pixel);
    setFontSize(DMG_FONT_SIZE);

    int textW = queryTextWidth(text, false);
    int textH = queryTextHeight(text, false);

    // 외곽선(1px) + 그림자 오프셋(1px) 여유
    int texW = textW + 4;
    int texH = textH + 4;

    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texW, texH);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_Texture* prevTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, tex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    int cx = texW / 2;
    int cy = texH / 2;

    // [최하층] 회색 그림자의 검은 외곽선 8방위 (그림자 위치 기준)
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if (dx != 0 || dy != 0)
                drawTextCenter(text, cx + 1 + dx, cy + 1 + dy, col::black);

    // [하층] 흰색 본문의 검은 외곽선 8방위
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if (dx != 0 || dy != 0)
                drawTextCenter(text, cx + dx, cy + dy, col::black);

    // [중층] 회색 그림자 (x+1, y+1)
    drawTextCenter(text, cx + 1, cy + 1, DMG_SHADOW_COL);

    // [최상층] 흰색 본문
    drawTextCenter(text, cx, cy, mainCol);

    SDL_SetRenderTarget(renderer, prevTarget);
    return new Sprite(renderer, tex, texW, texH);
}

// 각도(도) → 초기속도 벡터 계산
static void initDmgVelocity(float& outVelX, float& outVelY)
{
    float angle = DMG_ANGLE_MIN + (float)randomRange(0, 1000) / 1000.f * (DMG_ANGLE_MAX - DMG_ANGLE_MIN);
    float speed = DMG_SPEED_MIN + (float)randomRange(0, 1000) / 1000.f * (DMG_SPEED_MAX - DMG_SPEED_MIN);
    float rad = angle * 3.14159265f / 180.f;
    outVelX = cosf(rad) * speed;
    outVelY = sinf(rad) * speed;
}

Damage::Damage(std::wstring inputStr, int inputX, int inputY, SDL_Color inputCol, int inputSize)
{
    list.push_back(this);
    letters = inputStr;
    x = static_cast<float>(inputX);
    y = static_cast<float>(inputY);
    color = inputCol;
    timer = DMG_TIMER;
    initDmgVelocity(velX, velY);
    sprite = createDmgSprite(letters, col::white);
}

Damage::Damage(std::wstring inputStr, SDL_Color inputCol, int gridX, int gridY, dmgAniFlag inputFlag)
{
    myDmgAniFlag = inputFlag;
    list.push_back(this);
    letters = inputStr;
    x = 16.f * gridX + 8.f;
    y = 16.f * gridY + 8.f - 3.f;
    color = inputCol;

    if (inputFlag == dmgAniFlag::dodged)
    {
        // MISS: x 흔들림 없이, 수직 부유
        velX = 0.f;
        velY = MISS_RISE_SPEED;
        timer = MISS_TIMER;
        sprite = createDmgSprite(letters, inputCol);
    }
    else if (inputFlag == dmgAniFlag::blocked)
    {
        // BLOCK: 제자리에서 퉁! 점프
        originY = y;
        velX = 0.f;
        velY = BLOCK_JUMP_VEL;
        timer = BLOCK_TIMER;
        sprite = createDmgSprite(letters, BLOCK_COLOR);
    }
    else
    {
        // 일반 데미지: 공작새 궤적
        x += randomRange(-2, 2);
        y += randomRange(-2, 2);
        timer = DMG_TIMER;
        initDmgVelocity(velX, velY);
        sprite = createDmgSprite(letters, inputCol);
    }
}

Damage::~Damage()
{
    delete sprite;
    list.erase(std::find(list.begin(), list.end(), this));
}

void Damage::step()
{
    if (myDmgAniFlag == dmgAniFlag::dodged)
    {
        // ── MISS: 수직 부유 ──
        y += velY;

        if (timer <= MISS_FADE_START)
        {
            if (alpha <= MISS_FADE_SPEED) alpha = 0;
            else alpha -= MISS_FADE_SPEED;
        }
    }
    else if (myDmgAniFlag == dmgAniFlag::blocked)
    {
        // ── BLOCK: 퉁! 점프 후 원위치 복귀 ──
        y += velY;
        velY += BLOCK_GRAVITY;
        if (y >= originY) { y = originY; velY = 0.f; } // 생성지점에서 정지

        if (timer <= BLOCK_FADE_START)
        {
            if (alpha <= BLOCK_FADE_SPEED) alpha = 0;
            else alpha -= BLOCK_FADE_SPEED;
        }
    }
    else
    {
        // ── 일반 데미지: 직선 + 마찰 ──
        x += velX;
        y += velY;
        velX *= DMG_FRICTION;
        velY *= DMG_FRICTION;

        if (timer <= DMG_FADE_START)
        {
            if (alpha <= DMG_FADE_SPEED) alpha = 0;
            else alpha -= DMG_FADE_SPEED;
        }
    }

    timer--;
    if (timer <= 0) { delete this; }
}
