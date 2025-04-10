export module EntityData;

import std;
import constVar;
import Sprite;
import SkillData;

//__int8 : -128~127
//__int16 : -32768 ~32767
//__int32 : –2,147,483,648 ~2,147,483,647

export struct EntityData
{
    std::wstring name = L"DEFAULT ENTITY";
    unsigned __int16 entityCode = 1;
    Sprite* entitySpr = nullptr;
    unsigned __int16 tooltipIndex = 0;
    unsigned __int8 category = 0;
    unsigned __int8 bodyTemplate = 0;
    __int16 temparature = 20;
    unsigned __int32 weight = 1000;
    unsigned __int32 volume = 1000;
    std::vector<unsigned __int16> material;
    unsigned __int8 HD = 1;
    //방어 계열 변수
    __int16 maxHP = 100;
    __int16 maxMP = 100;
    __int16 maxSTA = 60;

    std::vector<std::array<__int16, 2>> enc;
    __int16 sh = 0;
    __int16 ev = 0;
    __int16 rFire = 0;
    __int16 rCold = 0;
    __int16 rElec = 0;
    __int16 rCorr = 0;
    __int16 rRad = 0;
    std::vector<unsigned __int16> bionicList;
    unsigned __int16 corpseItemCode = 0;
    int eyeSight = 8; //기본 시야


    std::vector<SkillData> skillList;

    __int8 hpBarHeight = -12;
    __int8 partsStartIndex = 0;
    std::unordered_map<int,std::array<int,2>> partsPosition;

    relationFlag relation = relationFlag::hostile;
    bool isHumanCustomSprite = false;
    
    ////////////////////////////////csv에 없는 데이터/////////////////////////////////////
    __int16 HP = 100;
    __int16 MP = 100;
    __int16 STA = 60;
    __int16 fakeHP = 100;
    unsigned __int8 fakeHPAlpha = 255;
    __int16 fakeMP = 100;
    unsigned __int8 fakeMPAlpha = 255;
    __int16 recentDmg = 0;
    __int16 recentDmgTimer = 0;
    unsigned __int8 statStr = 5;
    unsigned __int8 statInt = 5;
    unsigned __int8 statDex = 5;
    bool isPlayer = false;
    __int8 direction = 0;
    ItemPocket* equipment;
    std::array<int, TALENT_SIZE> proficExp = { 0, }; //경험치
    std::array<float, TALENT_SIZE> proficApt = { 2.0,2.0,2.0,2.0,2.0,2.0,2.0,2.0,2.0, }; //적성
    std::array<int, TALENT_SIZE> proficFocus = { 0, }; //집중도 0:미분배, 1:소분배, 2:일반분배
    __int16 sprIndex = 0;
    __int16 sprIndexInfimum = 0;
    bool sprFlip = false;
    humanCustom::skin skin = humanCustom::skin::null;
    humanCustom::eyes eyes = humanCustom::eyes::null;
    humanCustom::scar scar = humanCustom::scar::null;
    humanCustom::beard beard = humanCustom::beard::null;
    humanCustom::hair hair = humanCustom::hair::null;
    humanCustom::horn horn = humanCustom::horn::null;

    std::vector<std::pair<statEfctFlag, int>> statusEffects;

    walkFlag walkMode = walkFlag::walk;
    double gridMoveSpd = 3.0;//그리드와 그리드 사이를 넘어갈 때의 속도
};