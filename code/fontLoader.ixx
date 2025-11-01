#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

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

        for (int i = 6; i < MAX_FONT_SIZE; i++)
        {
            if(i==8) mainFont[i] = TTF_OpenFont("font/Galmuri7.bdf", i);
            else if (i == 10) mainFont[i] = TTF_OpenFont("font/Galmuri9.bdf", i);
            else if (i == 12) mainFont[i] = TTF_OpenFont("font/Galmuri11.bdf", i);
            else if (i == 15) mainFont[i] = TTF_OpenFont("font/Galmuri14.bdf", i);
            else if (i >= 16)  mainFont[i] = TTF_OpenFont("font/neodgm.ttf", i);
            else mainFont[i] = TTF_OpenFont("font/Galmuri9.bdf", i);

            if (mainFont[i] == nullptr)
            {
                errorBox(L"Failed to open the font file.");
            }
        }

        for (int i = 11; i < MAX_FONT_SIZE; i++)
        {
            notoSansFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Regular.ttf", i);
            notoSansBoldFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Bold.ttf", i);
            pixelFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Bold.ttf", i);
        }
    }
    else if (option::language == L"English")
    {
        //영어일 경우에도 갈무리 폰트
        for (int i = 6; i < MAX_FONT_SIZE; i++)
        {
            if (i == 8) mainFont[i] = TTF_OpenFont("font/Galmuri7.bdf", i);
            else if (i == 10) mainFont[i] = TTF_OpenFont("font/Galmuri9.bdf", i);
            else if (i == 12) mainFont[i] = TTF_OpenFont("font/Galmuri11.bdf", i);
            else if (i == 15) mainFont[i] = TTF_OpenFont("font/Galmuri14.bdf", i);
            else if (i >= 16)  mainFont[i] = TTF_OpenFont("font/neodgm.ttf", i);
            else mainFont[i] = TTF_OpenFont("font/Galmuri9.bdf", i);

            if (mainFont[i] == nullptr)
            {
                errorBox(L"Failed to open the font file.");
            }
        }

        for (int i = 11; i < MAX_FONT_SIZE; i++)
        {
            notoSansFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Regular.ttf", i);
            notoSansBoldFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Bold.ttf", i);
            pixelFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Bold.ttf", i);
        }
    }
    else
    {

    }
}