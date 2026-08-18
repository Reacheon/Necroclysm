module;
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
            pixelFont[i] = TTF_OpenFont("font/gulim/gulim-Regular.ttc", i);

            mainFont[i] = TTF_OpenFont("font/mulmaru/Mulmaru.ttf", i);
        }

        setFont(fontType::mainFont);
    }
    else
    {

    }
}