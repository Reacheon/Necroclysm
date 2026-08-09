module;
#include <SDL3/SDL.h>

export module dbgPrt;

import std;

export void dbgPrt(const wchar_t* format, auto... args)
{
    std::wprintf(format, args...);
}

export void dbgPrt(SDL_Color col, const wchar_t* format, auto... args)
{
    std::wprintf(L"\033[38;2;%d;%d;%dm", col.r, col.g, col.b);
    std::wprintf(format, args...);
    std::wprintf(L"\033[0m"); // Reset color
}                
