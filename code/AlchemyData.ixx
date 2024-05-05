export module AlchemyData;

import std;
import constVar;

//__int8 : -128~127
//__int16 : -32768 ~32767
//__int32 : –2,147,483,648 ~2,147,483,647

export struct AlchemyData
{
    std::wstring name = L"DEFAULT ALCHEMY RECIPE";
    int code = 0;
    std::vector<std::pair<int, int>> reactant;
    std::vector<std::pair<int, int>> product;
    int time = 1;
    int qualityNeed = -1;
    std::vector<int> talentNeed;
    int difficulty = 0;
    bool heatSourceNeed = false;

    //비데이터베이스 변수
    bool isKnown = false;
};