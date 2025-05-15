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
        for (int i = 0; i < MAX_FONT_SIZE; i++)
        {
            if(i==8) mainFont[i] = TTF_OpenFont("font/Galmuri7.ttf", i);
            else if (i == 10) mainFont[i] = TTF_OpenFont("font/Galmuri9.ttf", i);
            else if (i == 12) mainFont[i] = TTF_OpenFont("font/Galmuri11.ttf", i);
            else if (i == 15) mainFont[i] = TTF_OpenFont("font/Galmuri14.ttf", i);
            else if (i >= 16)  mainFont[i] = TTF_OpenFont("font/neodgm.ttf", i);
            else mainFont[i] = TTF_OpenFont("font/Galmuri9.ttf", i);

            errorBox(mainFont[i] == nullptr, L"Failed to open the font file.");
        }
    }
    else if (option::language == L"English")
    {
        //영어일 경우에도 갈무리 폰트
        for (int i = 0; i < MAX_FONT_SIZE; i++)
        {
            mainFont[i] = TTF_OpenFont("font/Galmuri9.ttf", i);
            errorBox(mainFont[i] == nullptr, L"Failed to open the font file.");
        }
    }
    else
    {

    }
}