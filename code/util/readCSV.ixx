#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL3/SDL.h>
#include <codecvt>  // for std::codecvt_utf8

export module readCSV;

import std;               // C++23 standard library module
import wstring2Number;    // provides wtoi / wstod etc.

//------------------------------------------------------------
// Helper meta‑function: compile‑time type comparison
//------------------------------------------------------------

template <typename T1, typename T2>
struct isSame { enum { value = false }; };

template <typename T0>
struct isSame<T0, T0> { enum { value = true }; };

//------------------------------------------------------------
// readCSV
//   ‑ SIZEW : number of columns per row (compile‑time)
//   ‑ T     : element type (std::wstring or numeric)
//            if T == std::wstring  → 문자열로 저장
//            else                 → wtoi 로 정수 변환 후 저장
//------------------------------------------------------------

export template<std::size_t SIZEW, typename T>
int readCSV(const wchar_t* file, std::vector<std::array<T, SIZEW>>& arr)
{
    //--------------------------------------------------------
    // 1) 파일 열기 (UTF‑8 BOM 허용)
    //--------------------------------------------------------
    std::wifstream fin(file);
    if (!fin.is_open())
        return 0;                               // 실패

    // UTF‑8 to UTF‑16 변환용 locale 적용
    fin.imbue(std::locale(fin.getloc(),
        new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

    //--------------------------------------------------------
    // 2) 라인 단위로 읽으면서 쉼표(,) 파싱
    //--------------------------------------------------------
    std::wstring line;          // 한 줄 전체
    std::size_t  row = 0;       // 현재 행 인덱스

    auto ensure_row = [&]() {
        if (row == arr.size())
            arr.emplace_back(); // 필요 시 새 행 추가 (디폴트 초기화)
        };

    while (std::getline(fin, line))
    {
        // Windows CRLF(\r\n) 처리: \r 제거
        if (!line.empty() && line.back() == L'\r')
            line.pop_back();

        std::wstringstream ss(line);
        std::wstring cell;
        std::size_t col = 0;

        ensure_row();

        //----------------------------------------------------
        // 셀 단위 파싱 (쉼표 기준)
        //----------------------------------------------------
        while (std::getline(ss, cell, L','))
        {
            if (col >= SIZEW) {
                // 열 개수 초과 → 무시하거나 필요 시 오류 처리
                ++col;
                continue;
            }

            if constexpr (isSame<T, std::wstring>::value) {
                // 문자열 배열일 때: 빈칸은 스페이스 한 칸으로 채워 구분
                arr[row][col] = cell.empty() ? L" " : std::move(cell);
            }
            else {
                // 숫자 배열일 때: 빈칸은 0
                arr[row][col] = cell.empty() ? 0 : wtoi(cell.c_str());
            }
            ++col;
        }
        ++row; // 다음 행으로
    }

    return 1; // 성공
}
