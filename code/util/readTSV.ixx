#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL3/SDL.h>
#include <codecvt>
export module readTSV;
import std;
import wstring2Number;

template <typename T1, typename T2>
struct isSame { enum { value = false }; };
template <typename T0>
struct isSame<T0, T0> { enum { value = true }; };

export template<std::size_t SIZEW, typename T>
int readTSV(const wchar_t* file, std::vector<std::array<T, SIZEW>>& arr)
{
    std::wifstream fin(file);
    if (!fin.is_open())
        return 0;
    fin.imbue(std::locale(fin.getloc(),
        new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

    std::wstring line;
    std::size_t  row = 0;

    auto ensure_row = [&]() {
        if (row == arr.size())
            arr.emplace_back();
        };

    while (std::getline(fin, line))
    {
        if (!line.empty() && line.back() == L'\r')
            line.pop_back();

        std::wstringstream ss(line);
        std::wstring cell;
        std::size_t col = 0;
        ensure_row();

        while (std::getline(ss, cell, L'\t'))
        {
            if (col >= SIZEW) {
                ++col;
                continue;
            }
            if constexpr (isSame<T, std::wstring>::value) {
                arr[row][col] = cell.empty() ? L" " : std::move(cell);
            }
            else {
                arr[row][col] = cell.empty() ? 0 : wtoi(cell.c_str());
            }
            ++col;
        }
        ++row;
    }
    return 1;
}