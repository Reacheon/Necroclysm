export module SkillData;

import std;
import constVar;

export struct SkillData
{
    std::wstring name = L"DEFAULT ALCHEMY RECIPE";
    int skillCode = 0;
    int iconIndex = 0;
    std::wstring descript = L"TEST_DESCRIPTION";
    std::wstring abstract = L"TEST_ABSTRACT";
    skillSrc src = skillSrc::MUTATION;
    skillType type = skillType::PASSIVE;

    float energyPerAct = 0.0;
    float energyPerTurn = 0.0;
    float energyPerDay = 0.0;
    //비데이터베이스 변수
    bool isLearned = false;
    bool isQuickSlot = false;
    bool toggle = false;

    int skillLevel = 1;
    float skillExp = 0.0; //100.0 이상이면 자동으로 레벨업됨
    int maxSkillLevel = 5;

    float maxCooldown = 30.0;
    float currentCooldown = 0.0;   
    std::wstring skillRank = L"F";
};