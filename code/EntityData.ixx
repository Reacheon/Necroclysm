export module EntityData;

import std;
import constVar;
import Sprite;

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

    __int16 maxHP = 100; //hp
    std::vector<std::array<int, 4>> parts = { { partType::head, 100, 100,100 } };

    std::vector<std::array<__int16, 2>> rPierce;
    std::vector<std::array<__int16, 2>> rCut;
    std::vector<std::array<__int16, 2>> rBash;
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

    void* pocketPtr = nullptr;


    ////////////////////////////////csv에 없는 데이터/////////////////////////////////////

    __int16 HP = 100;
    __int16 fakeHP = 100;
    unsigned __int8 fakeHPAlpha = 255;

    __int16 recentDmg = 0;
    __int16 recentDmgTimer = 0;

    unsigned __int8 statStr = 5;
    unsigned __int8 statInt = 5;
    unsigned __int8 statDex = 5;

    bool isPlayer = false;
};