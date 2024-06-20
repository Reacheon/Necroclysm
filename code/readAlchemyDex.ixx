#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL.h>

export module readAlchemyDex;

import std;
import util;
import constVar;
import globalVar;
import AlchemyData;

namespace csvAlchemy
{
    constexpr int name = 0;
    constexpr int code = 1;
    constexpr int reactant = 2;
    constexpr int product = 3;
    constexpr int time = 4;
    constexpr int qualityNeed = 5;
    constexpr int talentNeed = 6;
    constexpr int difficulty = 7;
    constexpr int heatSourceNeed = 8;
};

export int readAlchemyDex(const wchar_t* file)
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

                if (arrayCounter / csvWidth == alchemyDex.size() + 1) // ���� ���� ũ��(����)�� �����ϸ� 1ĭ �ø�
                {
                    AlchemyData newAlchemy;
                    alchemyDex.push_back(newAlchemy);
                }

                if (arrayCounter / (csvWidth) != 0)
                {
                    switch (arrayCounter % (csvWidth))
                    {
                    case csvAlchemy::name:
                        alchemyDex[arrayCounter / (csvWidth)-1].name = strFragment;
                        break;
                    case csvAlchemy::code:
                        alchemyDex[arrayCounter / (csvWidth)-1].code = wtoi(strFragment);
                        break;
                    case csvAlchemy::reactant:
                        pairsToVec(strFragment, alchemyDex[arrayCounter / (csvWidth)-1].reactant);
                        break;
                    case csvAlchemy::product:
                        pairsToVec(strFragment, alchemyDex[arrayCounter / (csvWidth)-1].product);
                        break;
                    case csvAlchemy::time:
                        alchemyDex[arrayCounter / (csvWidth)-1].time = wtoi(strFragment);
                        break;
                    case csvAlchemy::qualityNeed:
                        alchemyDex[arrayCounter / (csvWidth)-1].qualityNeed = wtoi(strFragment);
                        break;
                    case csvAlchemy::talentNeed:
                        valsToVec(strFragment, alchemyDex[arrayCounter / (csvWidth)-1].talentNeed);
                        break;
                    case csvAlchemy::difficulty:
                        alchemyDex[arrayCounter / (csvWidth)-1].difficulty = wtoi(strFragment);
                        break;
                    case csvAlchemy::heatSourceNeed:
                        alchemyDex[arrayCounter / (csvWidth)-1].heatSourceNeed = wtoi(strFragment);
                        break;

                    default:
                        prt(L"readAlchemyDex.ixx���� ���� �߻�. csv�� �߸��� ��Ҹ� �о���.\n");
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



