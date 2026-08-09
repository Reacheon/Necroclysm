module;

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
    constexpr int nativeName = 0;
    constexpr int name = 1;
    constexpr int entityCode = 2;
    constexpr int sprFileName = 3;
    constexpr int nativeDescript = 4;
    constexpr int descript = 5;
    constexpr int category = 6;
    constexpr int temperature = 7;
    constexpr int weight = 8;
    constexpr int volume = 9;
    constexpr int material = 10;
    constexpr int HD = 11;
    constexpr int maxHP = 12;
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
    constexpr int corpseItemCode = 23;
    constexpr int statStr = 24;
    constexpr int statInt = 25;
    constexpr int statDex = 26;
    constexpr int hpBarHeight = 27;
    constexpr int relation = 28;
    constexpr int isHumanCustomSprite = 29;
    constexpr int atkSpr1 = 30;
    constexpr int atkSpr2 = 31;
    constexpr int atkPierce = 32;
    constexpr int atkCut = 33;
    constexpr int atkBash = 34;
    constexpr int atkBalance = 35;
    constexpr int age = 36;
    constexpr int mbti = 37;
    constexpr int maxEnergy = 38;
};

export int readEntityDex(const wchar_t* file)
{
    //파일은 UTF-8(BOM optional)로 저장된 TSV. binary 모드 ifstream으로 raw 바이트 읽고
    //utf8Decoder로 wstring 변환 — <codecvt>/wifstream::imbue(C++26 제거 예정) 대체.
    std::ifstream in(std::filesystem::path(file), std::ios::binary);
    std::wstring str;
    std::wstring strFragment;//표 한 칸의 문자열이 저장되는 객체, 매번 초기화됨

    if (in.is_open())
    {
        in.seekg(0, std::ios::end);
        std::int64_t size = in.tellg();
        std::string raw(static_cast<std::size_t>(size), '\0');
        in.seekg(0, std::ios::beg);
        in.read(raw.data(), size);
        in.close();

        //UTF-8 BOM(EF BB BF) 스킵 — 기존 std::consume_header 동등
        std::size_t skip = 0;
        if (raw.size() >= 3 && static_cast<unsigned char>(raw[0]) == 0xEF
                            && static_cast<unsigned char>(raw[1]) == 0xBB
                            && static_cast<unsigned char>(raw[2]) == 0xBF)
        {
            skip = 3;
        }
        str = utf8Decoder(raw.c_str() + skip);
        //CRLF → LF 정규화 — binary 모드라 \r이 그대로 살아 있음.
        //기존 wifstream(text 모드)이 자동 처리하던 부분. 미처리 시 마지막 셀에 \r이 붙어 wtoi 실패.
        std::erase(str, L'\r');
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
                    //dbgPrt(L"이 csv 파일의 가로사이즈는 %d이다!\n", csvWidth);
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
                        case csvEntity::nativeName:
                            break;
                        case csvEntity::name:
                            entityDex[tgtIndex].name = strFragment;
                            break;
                        case csvEntity::entityCode:
                            entityDex[tgtIndex].entityCode = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::sprFileName:
                            errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"readEntityDex: 이 엔티티의 스프라이트 시트("+ strFragment+L")를 찾지 못하였다.");
                            entityDex[tgtIndex].entitySpr = spr::spriteMapper[strFragment.c_str()];
                            break;
                        case csvEntity::nativeDescript:
                            break;
                        case csvEntity::descript:
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
                            else { errorBox(L"readEntityDex에서 알 수 없는 Entity Category를 읽었다."); }
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
                        case csvEntity::atkPierce:
                            entityDex[tgtIndex].atkPierce = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::atkCut:
                            entityDex[tgtIndex].atkCut = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::atkBash:
                            entityDex[tgtIndex].atkBash = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::atkBalance:
                            entityDex[tgtIndex].atkBalance = wtof(strFragment.c_str());
                            break;
                        case csvEntity::age:
                            entityDex[tgtIndex].age = wtoi(strFragment.c_str());
                            break;
                        case csvEntity::mbti:
                            entityDex[tgtIndex].mbti = strFragment;
                            break;
                        case csvEntity::maxEnergy:
                            entityDex[tgtIndex].maxEnergy = wtoi(strFragment.c_str());
                            break;
                        default:
                            dbgPrt(L"readEntityDex.ixx에서 오류 발생. csv의 잘못된 장소를 읽었다.\n");
                            break;
                    }

                    //dbgPrt(L"[문자열] %ws ", strFragment.c_str());
                    //dbgPrt(L"를 (%d,%d)에 입력했다.\n", arrayCounter / (csvWidth)-1, arrayCounter % (csvWidth));
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



