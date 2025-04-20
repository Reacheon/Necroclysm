#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

export module readSkillDex;

import std;
import util;
import constVar;
import globalVar;
import SkillData;

namespace csvSkill
{
    constexpr int name = 0;
    constexpr int code = 1;
    constexpr int iconIndex = 2;
    constexpr int descript = 3;
    constexpr int abstract = 4;
    constexpr int src = 5;
    constexpr int type = 6;
    constexpr int flag = 7;
    constexpr int energyPerAct = 8;
    constexpr int energyPerTurn = 9;
    constexpr int energyPerDay = 10;

    constexpr int pietyPerAct = 11;
    constexpr int pietyPerTurn = 12;
    constexpr int pietyPerDay = 13;

    constexpr int mentalPerAct = 14;
    constexpr int mentalPerTurn = 15;
    constexpr int mentalPerDay = 16;

    constexpr int skillRank = 17;
};

export int readSkillDex(const wchar_t* file)
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
            if (str[i] == UNI::TAB || str[i] == UNI::LF)//해당 문자가 쉼표(44)거나 라인피드(10)일 경우
            {
                csvWidth++;
                if (str[i] == UNI::LF)
                {
                    //prt(L"이 csv 파일의 가로사이즈는 %d이다!\n", csvWidth);
                    break;
                }
            }
        }
        // 3,5,7 형태의 문자열을 벡터에 순서대로 3->5->7 순서대로 입력해주는 람다함수
        auto valsToVec = [&](std::wstring strFragment, auto& container) {
            int val;
            for (int j = 0; j < strFragment.size(); j++)
            {
                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                {
                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                    val = std::stoi(strFragment.substr(0, j));
                    strFragment.erase(0, j + 1);
                    j = 0;

                    container.push_back(val);
                }
            }
            };


        // 3x3,5x6,2x6 형태의 문자열을 pair 형태로 벡터에 순서대로 입력해주는 람다함수
        auto pairsToVec = [&](std::wstring strFragment, auto& container) {
            using T1 = decltype(container[0].first);
            using T2 = decltype(container[0].second);

            for (int j = 0; j < strFragment.size(); j++)
            {
                if (strFragment[j] == UNI::ASTERISK)
                {
                    container.push_back({ static_cast<T1>(std::stoi(strFragment.substr(0, j))), 0 });
                    strFragment.erase(0, j + 1);
                    j = 0;
                }

                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                {
                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                    container.back().second = static_cast<T2>(std::stoi(strFragment.substr(0, j)));
                    strFragment.erase(0, j + 1);
                    j = 0;
                }
            }
            };


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

                if (arrayCounter / csvWidth == skillDex.size() + 1) // 만약 벡터 크기(상하)가 부족하면 1칸 늘림
                {
                    SkillData newSkill;
                    skillDex.push_back(newSkill);
                }

                if (arrayCounter / (csvWidth) != 0)
                {
                    switch (arrayCounter % (csvWidth))
                    {
                    case csvSkill::name:
                        skillDex[arrayCounter / (csvWidth)-1].name = strFragment;
                        break;
                    case csvSkill::code:
                        skillDex[arrayCounter / (csvWidth)-1].skillCode = wtoi(strFragment);
                        break;
                    case csvSkill::iconIndex:
                        skillDex[arrayCounter / (csvWidth)-1].iconIndex = wtoi(strFragment);
                        break;
                    case csvSkill::descript:
                        skillDex[arrayCounter / (csvWidth)-1].descript = strFragment;
                        break;
                    case csvSkill::abstract:
                        skillDex[arrayCounter / (csvWidth)-1].abstract = strFragment;
                        break;
                    case csvSkill::src:
                        if (strFragment == L"GENERAL") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::GENERAL;
                        else if (strFragment == L"BIONIC") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::BIONIC;
                        else if (strFragment == L"MUTATION") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::MUTATION;
                        else if (strFragment == L"MAGIC") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::MAGIC;
                        else errorBox(L"잘못된 스킬 소스 %ls를 발견했다.",strFragment.c_str());
                        break;
                    case csvSkill::type:
                        if (strFragment == L"ACTIVE") skillDex[arrayCounter / (csvWidth)-1].type = skillType::ACTIVE;
                        else if (strFragment == L"PASSIVE") skillDex[arrayCounter / (csvWidth)-1].type = skillType::PASSIVE;
                        else if (strFragment == L"TOGGLE") skillDex[arrayCounter / (csvWidth)-1].type = skillType::TOGGLE;
                        else errorBox(L"잘못된 스킬 타입 %ls를 발견했다.", strFragment.c_str()); 
                        break;
                    case csvSkill::energyPerAct:
                        skillDex[arrayCounter / (csvWidth)-1].energyPerAct = wtof(strFragment);
                        break;
                    case csvSkill::energyPerTurn:
                        skillDex[arrayCounter / (csvWidth)-1].energyPerTurn = wtof(strFragment);
                        break;
                    case csvSkill::energyPerDay:
                        skillDex[arrayCounter / (csvWidth)-1].energyPerDay = wtof(strFragment);
                        break;
                    case csvSkill::flag:
                        break;
                    case csvSkill::pietyPerAct:
                        break;
                    case csvSkill::pietyPerTurn:
                        break;
                    case csvSkill::pietyPerDay:
                        break;
                    case csvSkill::mentalPerAct:
                        break;
                    case csvSkill::mentalPerTurn:
                        break;
                    case csvSkill::mentalPerDay:
                        break;
                    case csvSkill::skillRank:
                        skillDex[arrayCounter / (csvWidth)-1].skillRank = strFragment;
                        break;
                    default:
                        prt(L"readSkillDex.ixx에서 오류 발생. csv의 잘못된 장소를 읽었다.\n");
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
        return 1;
    }
    else//읽기 실패
    {

        return 0;
    }
}



