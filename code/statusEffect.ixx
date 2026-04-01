export module statusEffect;

import std;

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
};

export class statusEffect
{
public:
    statusEffectFlag effectType;
    float duration;
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
        if (it->effectType == inputFlag)
        {
            it = inputStatus.erase(it);
        }
        else
        {
            ++it;
        }
    }
}