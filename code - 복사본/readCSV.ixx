#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL.h>

export module readCSV;

import std;
import wstring2Number;

template <typename T1, typename T2>
struct isSame
{
    enum { value = false };
};

template <typename T0>
struct isSame<T0, T0>
{
    enum { value = true };
};

export template<std::size_t SIZEW, typename T>
int readCSV(const wchar_t* file, std::vector<std::array<T, SIZEW>>& arr)
{
    int a = 3;
    //prt(L"\nreadCSV.h가 실행되었다.\n");
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
            if (str[i] == SDLK_TAB || str[i] == 10)//해당 문자가 쉼표(44)거나 라인피드(10)일 경우
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
            if (str[i] == SDLK_TAB || str[i] == 10)//ASCII로 44(,)와 또는 라인피드(\n)와 같을 때
            {
                if (i == str.size() - 1) { i++; }//마지막 문자면 endP-startP 보정을 위해 i를 1 더함.
                endPoint = i;

                strFragment = str.substr(startPoint, endPoint - startPoint);

                if (arrayCounter / csvWidth == arr.size()) // 만약 벡터 크기(상하)가 부족하면 1칸 늘림
                {
                    std::array<T, SIZEW> singleArr;
                    arr.push_back(singleArr);
                    //prt(L"새로운 배열을 추가했다. 현재 세로 사이즈는 %d\n", (int)arr.size());
                }

                if constexpr (isSame<T, std::wstring>::value == true) //입력할 배열이 문자열 배열이면
                {
                    //prt(L"[문자열] %ws ", strFragment.c_str());
                    arr[arrayCounter / (csvWidth)][arrayCounter % (csvWidth)] = strFragment;
                }
                else //그외의 경우(정수 배열)
                {
                    //prt(L"[정수] %d ", wtoi(strFragment.c_str()));
                    arr[arrayCounter / (csvWidth)][arrayCounter % (csvWidth)] = wtoi(strFragment.c_str());
                }

                // prt(L"를 (%d,%d)에 입력했다.\n", arrayCounter / (csvWidth), arrayCounter % (csvWidth));

                arrayCounter++;

                startPoint = -1;
                endPoint = -1;
                strFragment.clear();

                if (i != str.size() - 1)//만약 다음 글자가 연속으로 쉼표면 여백이므로 건너뜀
                {
                    while (str[i + 1] == SDLK_TAB || str[i + 1] == 10)
                    {
                        if constexpr (isSame<T, std::wstring>::value == true)
                        {
                            arr[arrayCounter / (csvWidth)][arrayCounter % (csvWidth)] = L" ";//빈칸은 항상 스페이스바!
                        }
                        else
                        {
                            arr[arrayCounter / (csvWidth)][arrayCounter % (csvWidth)] = 0;//빈칸은ㅇ 0으로
                        }

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



