#include <SDL.h>
#include <SDL_ttf.h>

export module fontLoader;

import std;
import util;
import globalVar;
import constVar;

export void fontLoader()
{
    //galmuri16 = TTF_OpenFont("font/Galmuri7.ttf", 16);
    if (option::language == L"Korean")
    {
        //한국어일 경우에는 갈무리 폰트
        for (int i = 0; i < maxFontSize; i++)
        {
            mainFont[i] = TTF_OpenFont("font/Galmuri9.ttf", i);
            errorBox(mainFont[i] == nullptr, "Failed to open the font file.");
        }
    }
    else if (option::language == L"English")
    {
        //영어일 경우에도 갈무리 폰트
        for (int i = 0; i < maxFontSize; i++)
        {
            mainFont[i] = TTF_OpenFont("font/Galmuri9.ttf", i);
            errorBox(mainFont[i] == nullptr, "Failed to open the font file.");
        }
    }
    else
    {

    }
}