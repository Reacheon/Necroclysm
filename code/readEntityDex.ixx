#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL.h>

export module readEntityDex;

import std;
import util;
import constVar;
import globalVar;
import EntityData;
import textureVar;

namespace csvEntity
{
    constexpr int name = 0;
    constexpr int entityCode = 1;
    constexpr int sprFileName = 2;
    constexpr int tooltip = 3;
    constexpr int category = 4;
    constexpr int bodyTemplate = 5;
    constexpr int temperature = 6;
    constexpr int weight = 7;
    constexpr int volume = 8;
    constexpr int material = 9;
    constexpr int HD = 10;
    constexpr int maxHP = 11;
    constexpr int parts = 12;
    constexpr int rPierce = 13;
    constexpr int rCut = 14;
    constexpr int rBash = 15;
    constexpr int SH = 16;
    constexpr int EV = 17;
    constexpr int rFire = 18;
    constexpr int rCold = 19;
    constexpr int rElec = 20;
    constexpr int rCorr = 21;
    constexpr int rRad = 22;
    constexpr int bionicList = 23;
    constexpr int corpseItemCode = 24;
    constexpr int statStr = 25;
    constexpr int statInt = 26;
    constexpr int statDex = 27;
    constexpr int hpBarHeight = 28;
    constexpr int partsPosition = 29;
    constexpr int partsSprIndexStart = 30;
    constexpr int relation = 31;
    constexpr int isHumanCustomSprite = 32;
};


export int readEntityDex(const wchar_t* file)
{
    std::wifstream in(file);
    std::wstring str;
    std::wstring strFragment;//표 한 칸의 문자열이 저장되는 객체, 매번 초기화됨

    if (in.is_open())
    {
        in.imbue(std::locale(in.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
        in.seekg(0, std::ios::end);
        long long size = in.tellg();
        str.resize(size);
        in.seekg(0, std::ios::beg);
        in.read(&str[0], size);
        in.close();
        //읽기 종료

        //배열의 가로 사이즈를 구한다.
        int csvWidth = 0;
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == UNI::TAB || str[i] == 10)//해당 문자가 쉼표(44)거나 라인피드(10)일 경우
            {
                csvWidth++;
                if (str[i] == 10)
                {
                    //prt(L"이 csv 파일의 가로사이즈는 %d이다!\n", csvWidth);
                    break;
                }
            }
        }

        int startPoint = -1;
        int endPoint = -1;
        int arrayCounter = 0;

        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == UNI::TAB || str[i] == 10)//ASCII로 44(,)와 또는 라인피드(\n)와 같을 때
            {
                if (i == str.size() - 1) { i++; }//마지막 문자면 endP-startP 보정을 위해 i를 1 더함.
                endPoint = i;

                strFragment = str.substr(startPoint, endPoint - startPoint);

                if (arrayCounter / csvWidth == entityDex.size() + 1) // 만약 벡터 크기(상하)가 부족하면 1칸 늘림
                {
                    EntityData newEntity;
                    entityDex.push_back(std::move(newEntity));
                }

                if (arrayCounter / (csvWidth) != 0)
                {
                    switch (arrayCounter % (csvWidth))
                    {
                        case csvEntity::name:
                            entityDex[arrayCounter / (csvWidth)-1].name = strFragment;
                            break;
                        case csvEntity::entityCode:
                            entityDex[arrayCounter / (csvWidth)-1].entityCode = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::sprFileName:
                            errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 equip 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                            entityDex[arrayCounter / (csvWidth)-1].entitySpr = spr::spriteMapper[strFragment.c_str()];
                            break;
                        case csvEntity::tooltip:
                            entityTooltip.push_back(strFragment);
                            entityDex[arrayCounter / (csvWidth)-1].tooltipIndex = arrayCounter / (csvWidth)-1;
                            break;
                        case csvEntity::category:
                        {
                            if (strFragment == L"none") { entityDex[arrayCounter / (csvWidth)-1].category = entityCategory::none; }
                            else if (strFragment == L"human") { entityDex[arrayCounter / (csvWidth)-1].category = entityCategory::human; }
                            else if (strFragment == L"zombie") { entityDex[arrayCounter / (csvWidth)-1].category = entityCategory::zombie; }
                            else if (strFragment == L"robot") { entityDex[arrayCounter / (csvWidth)-1].category = entityCategory::robot; }
                            else { errorBox("error in readItemDex.ixx, csvItem::category"); }
                            break;
                        }
                        case csvEntity::bodyTemplate:
                        {
                            if (strFragment == L"none") { entityDex[arrayCounter / (csvWidth)-1].bodyTemplate = bodyTemplateFlag::none; }
                            else if (strFragment == L"human") { entityDex[arrayCounter / (csvWidth)-1].bodyTemplate = bodyTemplateFlag::human;}
                            else if (strFragment == L"zombie") { entityDex[arrayCounter / (csvWidth)-1].bodyTemplate = bodyTemplateFlag::zombie; }
                            else if (strFragment == L"tank") { entityDex[arrayCounter / (csvWidth)-1].bodyTemplate = bodyTemplateFlag::tank; }
                            else { errorBox("error in readItemDex.ixx, csvItem::category"); }
                            break;
                        }
                        case csvEntity::temperature:
                            entityDex[arrayCounter / (csvWidth)-1].temparature = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::weight:
                            entityDex[arrayCounter / (csvWidth)-1].weight = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::volume:
                            entityDex[arrayCounter / (csvWidth)-1].volume = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::material:
                            int val;
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::UNDERSCORE || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    val = wtoi(strFragment.substr(0, j).c_str());
                                    strFragment.erase(0, j + 1);
                                    j = 0;

                                    entityDex[arrayCounter / (csvWidth)-1].material.push_back(val);
                                }
                            }
                            break;
                        case csvEntity::HD:
                            entityDex[arrayCounter / (csvWidth)-1].HD = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::maxHP:
                            entityDex[arrayCounter / (csvWidth)-1].maxHP = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::parts:
                        {
                            break;
                        }
                        case csvEntity::rPierce:
                        {
                            break;
                        }
                        case csvEntity::rCut:
                        {
                            break;
                        }
                        case csvEntity::rBash:
                        {
                            break;
                        }
                        case csvEntity::SH:
                            entityDex[arrayCounter / (csvWidth)-1].sh = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::EV:
                            entityDex[arrayCounter / (csvWidth)-1].ev = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rFire:
                            entityDex[arrayCounter / (csvWidth)-1].rFire = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rCold:
                            entityDex[arrayCounter / (csvWidth)-1].rCold = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rElec:
                            entityDex[arrayCounter / (csvWidth)-1].rElec = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rCorr:
                            entityDex[arrayCounter / (csvWidth)-1].rCorr = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rRad:
                            entityDex[arrayCounter / (csvWidth)-1].rRad = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::bionicList:
                        {
                            int val;
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::UNDERSCORE || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    val = wtoi(strFragment.substr(0, j).c_str());
                                    strFragment.erase(0, j + 1);
                                    j = 0;

                                    entityDex[arrayCounter / (csvWidth)-1].bionicList.push_back(val);
                                }
                            }
                            break;
                        }
                        case csvEntity::corpseItemCode:
                            entityDex[arrayCounter / (csvWidth)-1].corpseItemCode = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::statStr:
                            entityDex[arrayCounter / (csvWidth)-1].statStr = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::statInt:
                            entityDex[arrayCounter / (csvWidth)-1].statInt = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::statDex:
                            entityDex[arrayCounter / (csvWidth)-1].statDex = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::hpBarHeight:
                            entityDex[arrayCounter / (csvWidth)-1].hpBarHeight = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::partsPosition:
                        {
                            entityDex[arrayCounter / (csvWidth)-1].partsPosition.clear();
                            std::array<int, 3> val;
                            int counter = 0;
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == SDLK_PERIOD)
                                {
                                    val[counter] = wtoi(strFragment.substr(0, j).c_str());
                                    counter++;
                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                }

                                if (strFragment[j] == UNI::UNDERSCORE || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    val[counter] = wtoi(strFragment.substr(0, j).c_str());
                                    counter = 0;
                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                    entityDex[arrayCounter / (csvWidth)-1].partsPosition[val[0]] = { val[1], val[2] };
                                }
                            }
                            break;
                        }
                        case csvEntity::partsSprIndexStart:
                            entityDex[arrayCounter / (csvWidth)-1].partsStartIndex = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::relation:
                        {
                            if (strFragment == L"NEUTRAL") { entityDex[arrayCounter / (csvWidth)-1].relation = relationFlag::neutral; }
                            else if (strFragment == L"HOSTILE") { entityDex[arrayCounter / (csvWidth)-1].relation = relationFlag::hostile; }
                            else if (strFragment == L"FRIENDLY") { entityDex[arrayCounter / (csvWidth)-1].relation = relationFlag::friendly; }
                            else { entityDex[arrayCounter / (csvWidth)-1].relation = relationFlag::neutral; }
                            break;
                        }
                        case csvEntity::isHumanCustomSprite:
                        {
                            if (strFragment == L"TRUE") { entityDex[arrayCounter / (csvWidth)-1].isHumanCustomSprite = true; }
                            else { entityDex[arrayCounter / (csvWidth)-1].isHumanCustomSprite = false; }
                            break;
                        }
                        default:
                            prt(L"readEntityDex.ixx에서 오류 발생. csv의 잘못된 장소를 읽었다.\n");
                            break;
                    }

                    //prt(L"[문자열] %ws ", strFragment.c_str());
                    //prt(L"를 (%d,%d)에 입력했다.\n", arrayCounter / (csvWidth)-1, arrayCounter % (csvWidth));
                }

                arrayCounter++;

                startPoint = -1;
                endPoint = -1;
                strFragment.clear();

                if (i != str.size() - 1)//만약 다음 글자가 연속으로 쉼표면 여백이므로 건너뜀
                {
                    while (str[i + 1] == UNI::TAB || str[i + 1] == 10)
                    {
                        arrayCounter++;
                        i++;
                    }
                }
            }
            else
            {
                if (startPoint == -1)
                {
                    startPoint = i;
                }
            }
        }
        //entityDex.erase(entityDex.begin());//0번째 라벨 삭제
        return 1;
    }
    else//읽기 실패
    {
        return 0;
    }
}



