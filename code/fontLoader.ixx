#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

export module fontLoader;

import std;
import util;
import globalVar;
import constVar;
import drawText;

export void fontLoader()
{
    if (option::language == L"Korean")
    {
        for (int i = 8; i < MAX_FONT_SIZE; i++)
        {
            notoSansFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Regular.ttf", i);
            notoSansBoldFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Bold.ttf", i);
            pixelFont[i] = TTF_OpenFont("font/gulim/gulim-Regular.ttc", i);
        }

        setFont(fontType::notoSans);
    }
    else if (option::language == L"English")
    {
        for (int i = 8; i < MAX_FONT_SIZE; i++)
        {
            notoSansFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Regular.ttf", i);
            notoSansBoldFont[i] = TTF_OpenFont("font/notoSans/NotoSansKR-Bold.ttf", i);
            pixelFont[i] = TTF_OpenFont("font/gulim/gulim-Regular.ttc", i);
        }

        setFont(fontType::notoSans);

    }
    else
    {

    }
}