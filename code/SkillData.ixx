export module SkillData;

import std;
import constVar;

// 랭크 문자열 → 난이도 숫자 (F=1, E=2, D=3, C=4, B=5, A=6, S=7)
export int rankDifficulty(const std::wstring& rank)
{
    if (rank == L"F") return 1;
    if (rank == L"E") return 2;
    if (rank == L"D") return 3;
    if (rank == L"C") return 4;
    if (rank == L"B") return 5;
    if (rank == L"A") return 6;
    if (rank == L"S") return 7;
    return 1;
}

// 엔티티별 스킬 런타임 상태 (메타데이터는 SkillBehavior에서 관리)
export struct SkillData
{
    std::wstring skillId;           // 예: L"SKILL_ROLL", L"BION_POWER_STORAGE", L"MUT_FUR"

    bool isLearned = false;
    bool isQuickSlot = false;
    bool toggle = false;

    int skillLevel = 1;             // 바이오닉 중복 설치 수 (오토닥 추가 설치 시 증가)

    // 현재 랭크 (F~S). 습득 시 SkillBehavior의 시작 랭크로 초기화됨.
    std::wstring skillRank = L"F";
    float skillExp = 0.0f;          // 숙련치 0~100. 100에서 멈추며 초과분은 증발됨.

    float currentCooldown = 0.0f;
};