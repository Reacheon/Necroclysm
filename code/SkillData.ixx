export module SkillData;

import std;
import constVar;

export struct SkillData
{
    std::wstring name = L"DEFAULT ALCHEMY RECIPE";
    int code = 0;
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

    //스킬의 숙련레벨은 어떻게..?
};