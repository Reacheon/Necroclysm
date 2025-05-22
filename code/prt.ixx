#include <SDL3/SDL.h>

export module prt;

import std;

export void prt(const wchar_t* format, auto... args)
{
    std::wprintf(format, args...);
}

export void prt(SDL_Color col, const wchar_t* format, auto... args)
{
    std::wprintf(L"\033[38;2;%d;%d;%dm", col.r, col.g, col.b);
    std::wprintf(format, args...);
    std::wprintf(L"\033[0m"); // Reset color
}                
