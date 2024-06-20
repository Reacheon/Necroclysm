#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL.h>

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
};

export int readSkillDex(const wchar_t* file)
{
    std::wifstream in(file);
    std::wstring str;
    std::wstring strFragment;//ǥ �� ĭ�� ���ڿ��� ����Ǵ� ��ü, �Ź� �ʱ�ȭ��

    if (in.is_open())
    {
        in.imbue(std::locale(in.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
        in.seekg(0, std::ios::end);
        long long size = in.tellg();
        str.resize(size);
        in.seekg(0, std::ios::beg);
        in.read(&str[0], size);
        in.close();
        //�б� ����

        //�迭�� ���� ����� ���Ѵ�.
        int csvWidth = 0;
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == UNI::TAB || str[i] == UNI::LF)//�ش� ���ڰ� ��ǥ(44)�ų� �����ǵ�(10)�� ���
            {
                csvWidth++;
                if (str[i] == UNI::LF)
                {
                    //prt(L"�� csv ������ ���λ������ %d�̴�!\n", csvWidth);
                    break;
                }
            }
        }
        // 3,5,7 ������ ���ڿ��� ���Ϳ� ������� 3->5->7 ������� �Է����ִ� �����Լ�
        auto valsToVec = [&](std::wstring strFragment, auto& container) {
            int val;
            for (int j = 0; j < strFragment.size(); j++)
            {
                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                {
                    if (j == strFragment.size() - 1) { j++; } //�������̸� j���� 1 ���Ͽ� ����

                    val = std::stoi(strFragment.substr(0, j));
                    strFragment.erase(0, j + 1);
                    j = 0;

                    container.push_back(val);
                }
            }
            };


        // 3x3,5x6,2x6 ������ ���ڿ��� pair ���·� ���Ϳ� ������� �Է����ִ� �����Լ�
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
                    if (j == strFragment.size() - 1) { j++; } //�������̸� j���� 1 ���Ͽ� ����

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
            if (str[i] == UNI::TAB || str[i] == 10)//ASCII�� 44(,)�� �Ǵ� �����ǵ�(\n)�� ���� ��
            {
                if (i == str.size() - 1) { i++; }//������ ���ڸ� endP-startP ������ ���� i�� 1 ����.
                endPoint = i;

                strFragment = str.substr(startPoint, endPoint - startPoint);

                if (arrayCounter / csvWidth == skillDex.size() + 1) // ���� ���� ũ��(����)�� �����ϸ� 1ĭ �ø�
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
                        if (strFragment == L"NONE") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::NONE;
                        else if (strFragment == L"BIONIC") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::BIONIC;
                        else if (strFragment == L"MUTATION") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::MUTATION;
                        else if (strFragment == L"MARTIAL_ART") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::MARTIAL_ART;
                        else if (strFragment == L"DIVINE_POWER") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::DIVINE_POWER;
                        else if (strFragment == L"MAGIC") skillDex[arrayCounter / (csvWidth)-1].src = skillSrc::MAGIC;
                        else errorBox(L"�߸��� ��ų �ҽ� %ls�� �߰��ߴ�.",strFragment.c_str());
                        break;
                    case csvSkill::type:
                        if (strFragment == L"ACTIVE") skillDex[arrayCounter / (csvWidth)-1].type = skillType::ACTIVE;
                        else if (strFragment == L"PASSIVE") skillDex[arrayCounter / (csvWidth)-1].type = skillType::PASSIVE;
                        else if (strFragment == L"TOGGLE") skillDex[arrayCounter / (csvWidth)-1].type = skillType::TOGGLE;
                        else errorBox(L"�߸��� ��ų Ÿ�� %ls�� �߰��ߴ�.", strFragment.c_str()); 
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
                    default:
                        prt(L"readSkillDex.ixx���� ���� �߻�. csv�� �߸��� ��Ҹ� �о���.\n");
                        break;
                    }

                    //prt(L"[���ڿ�] %ws ", strFragment.c_str());
                    //prt(L"�� (%d,%d)�� �Է��ߴ�.\n", arrayCounter / (csvWidth)-1, arrayCounter % (csvWidth));
                }

                arrayCounter++;

                startPoint = -1;
                endPoint = -1;
                strFragment.clear();

                if (i != str.size() - 1)//���� ���� ���ڰ� �������� ��ǥ�� �����̹Ƿ� �ǳʶ�
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
    else//�б� ����
    {

        return 0;
    }
}



