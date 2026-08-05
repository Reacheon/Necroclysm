module;
#include <SDL3/SDL.h>

export module statusEffect;

import std;
import constVar;

export enum statusEffectFlag
{
    none = -1,
    confused = 0,
    bleeding = 1,
    hungry = 2,
    dehydrated = 3,
    blind = 4,
    tired = 5,
    exhausted = 6,

    run = 7,
    crouch = 8,
    crawl = 9,

    STATUS_EFFECT_COUNT = 10,
};

export class statusEffect
{
public:
    statusEffectFlag effectType;
    float duration;
};

// 상태이상 메타데이터: 아이콘, 이름, 색상을 여기서 일괄 관리
// name이 nullptr이면 동적으로 결정됨 (허기/갈증/피로 등 수치 기반)
export struct StatusEffectMeta
{
    int iconIndex;
    const wchar_t* name;
    SDL_Color color;
};

export inline const StatusEffectMeta statusEffectMeta[] = {
    /* confused     */ { 1,  L"Confused",      col::white },
    /* bleeding     */ { 2,  L"Bleeding",      lowCol::red },
    /* hungry       */ { 3,  nullptr,          col::white },
    /* dehydrated   */ { 4,  nullptr,          col::white },
    /* blind        */ { 15, L"Blind",         col::white },
    /* tired        */ { 11, nullptr,          col::white },
    /* exhausted    */ { 11, L"Exhausted",     lowCol::red },
    /* run          */ { 60, L"Run",           col::white },
    /* crouch       */ { 62, L"Crouch",        col::white },
    /* crawl        */ { 61, L"Crawl",         col::white },
};

export inline bool checkStatusEffect(std::vector<statusEffect>& inputStatus, statusEffectFlag inputFlag)
{
    for (const auto& effect : inputStatus)
    {
        if (effect.effectType == inputFlag) return true;
    }
    return false;
}

export inline void eraseStatusEffect(std::vector<statusEffect>& inputStatus, statusEffectFlag inputFlag)
{
    for (auto it = inputStatus.begin(); it != inputStatus.end();)
    {
        if (it->effectType == inputFlag) it = inputStatus.erase(it);
        else ++it;
    }
}