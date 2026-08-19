module;

#include <SDL3/SDL.h>

export module readTSV;
import std;
import wstring2Number;
import utf8Decoder;

template <typename T1, typename T2>
struct isSame { enum { value = false }; };
template <typename T0>
struct isSame<T0, T0> { enum { value = true }; };

export template<std::size_t SIZEW, typename T>
int readTSV(const wchar_t* file, std::vector<std::array<T, SIZEW>>& arr)
{
    std::ifstream fin(std::filesystem::path(file), std::ios::binary);
    if (!fin.is_open())
        return 0;

    std::string  rawLine;
    std::wstring line;
    std::size_t  row = 0;
    bool firstLine = true;

    auto ensure_row = [&]() {
        if (row == arr.size())
            arr.emplace_back();
        };

    while (std::getline(fin, rawLine))
    {
        //첫 줄 UTF-8 BOM(EF BB BF) 스킵
        if (firstLine)
        {
            firstLine = false;
            if (rawLine.size() >= 3 && static_cast<unsigned char>(rawLine[0]) == 0xEF
                                    && static_cast<unsigned char>(rawLine[1]) == 0xBB
                                    && static_cast<unsigned char>(rawLine[2]) == 0xBF)
            {
                rawLine.erase(0, 3);
            }
        }

        line = utf8Decoder(rawLine.c_str());

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