#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL3/SDL.h>

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
    constexpr int temperature = 5;
    constexpr int weight = 6;
    constexpr int volume = 7;
    constexpr int material = 8;
    constexpr int HD = 9;
    constexpr int maxHP = 10;
    constexpr int rPierce = 11;
    constexpr int rCut = 12;
    constexpr int rBash = 13;
    constexpr int SH = 14;
    constexpr int EV = 15;
    constexpr int rFire = 16;
    constexpr int rCold = 17;
    constexpr int rElec = 18;
    constexpr int rCorr = 19;
    constexpr int rRad = 20;
    constexpr int bionicList = 21;
    constexpr int corpseItemCode = 22;
    constexpr int statStr = 23;
    constexpr int statInt = 24;
    constexpr int statDex = 25;
    constexpr int hpBarHeight = 26;
    constexpr int relation = 27;
    constexpr int isHumanCustomSprite = 28;
    constexpr int atkSpr1 = 29;
    constexpr int atkSpr2 = 30;
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
                    const int tgtIndex = arrayCounter / (csvWidth)-1;
                    const int columnIndex = arrayCounter % csvWidth;

                    switch (columnIndex)
                    {
                        case csvEntity::name:
                            entityDex[tgtIndex].name = strFragment;
                            break;
                        case csvEntity::entityCode:
                            entityDex[tgtIndex].entityCode = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::sprFileName:
                            errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 equip 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                            entityDex[tgtIndex].entitySpr = spr::spriteMapper[strFragment.c_str()];
                            break;
                        case csvEntity::tooltip:
                            entityTooltip.push_back(strFragment);
                            entityDex[tgtIndex].tooltipIndex = tgtIndex;
                            break;
                        case csvEntity::category:
                        {
                            if (strFragment == L"none") { entityDex[tgtIndex].category = entityCategory::none; }
                            else if (strFragment == L"human") { entityDex[tgtIndex].category = entityCategory::human; }
                            else if (strFragment == L"zombie") { entityDex[tgtIndex].category = entityCategory::zombie; }
                            else if (strFragment == L"robot") { entityDex[tgtIndex].category = entityCategory::robot; }
                            else if (strFragment == L"animal") { entityDex[tgtIndex].category = entityCategory::animal; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::category"); }
                            break;
                        }
                        case csvEntity::temperature:
                            entityDex[tgtIndex].temparature = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::weight:
                            entityDex[tgtIndex].weight = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::volume:
                            entityDex[tgtIndex].volume = wtoi(strFragment.c_str());
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

                                    entityDex[tgtIndex].material.push_back(val);
                                }
                            }
                            break;
                        case csvEntity::HD:
                            entityDex[tgtIndex].HD = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::maxHP:
                            entityDex[tgtIndex].maxHP = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rPierce:
                            entityDex[tgtIndex].rPierce = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rCut:
                            entityDex[tgtIndex].rCut = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rBash:
                            entityDex[tgtIndex].rBash = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::SH:
                            entityDex[tgtIndex].sh = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::EV:
                            entityDex[tgtIndex].ev = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rFire:
                            entityDex[tgtIndex].rFire = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rCold:
                            entityDex[tgtIndex].rCold = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rElec:
                            entityDex[tgtIndex].rElec = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rCorr:
                            entityDex[tgtIndex].rCorr = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::rRad:
                            entityDex[tgtIndex].rRad = wtoi(strFragment.c_str());
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

                                    entityDex[tgtIndex].bionicList.push_back(val);
                                }
                            }
                            break;
                        }
                        case csvEntity::corpseItemCode:
                            entityDex[tgtIndex].corpseItemCode = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::statStr:
                            entityDex[tgtIndex].statStr = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::statInt:
                            entityDex[tgtIndex].statInt = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::statDex:
                            entityDex[tgtIndex].statDex = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::hpBarHeight:
                            entityDex[tgtIndex].hpBarHeight = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::relation:
                        {
                            if (strFragment == L"NEUTRAL") { entityDex[tgtIndex].relation = relationFlag::neutral; }
                            else if (strFragment == L"HOSTILE") { entityDex[tgtIndex].relation = relationFlag::hostile; }
                            else if (strFragment == L"FRIENDLY") { entityDex[tgtIndex].relation = relationFlag::friendly; }
                            else { entityDex[tgtIndex].relation = relationFlag::neutral; }
                            break;
                        }
                        case csvEntity::isHumanCustomSprite:
                        {
                            if (strFragment == L"TRUE") { entityDex[tgtIndex].isHumanCustomSprite = true; }
                            else { entityDex[tgtIndex].isHumanCustomSprite = false; }
                            break;
                        }
                        case csvEntity::atkSpr1:
                            entityDex[tgtIndex].atkSpr1 = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::atkSpr2:
                            entityDex[tgtIndex].atkSpr2 = wtoi(strFragment.c_str());
                            break;
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



