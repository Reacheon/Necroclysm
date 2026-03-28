export module SkillData;

import std;
import constVar;

// 엔티티별 스킬 런타임 상태 (메타데이터는 SkillBehavior에서 관리)
export struct SkillData
{
    int skillCode = 0;

    bool isLearned = false;
    bool isQuickSlot = false;
    bool toggle = false;

    int skillLevel = 1;
    float skillExp = 0.0f; //100.0 이상이면 자동으로 레벨업됨

    float currentCooldown = 0.0f;
};
