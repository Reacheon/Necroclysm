#include <SDL3/SDL.h>

export module col2Str;

import std;

export std::wstring col2Str(SDL_Color color)
{
	std::wstring instantColor = L"#000000";
	instantColor[1] = color.r / 16 + 48;
	instantColor[2] = color.r % 16 + 48;
	instantColor[3] = color.g / 16 + 48;
	instantColor[4] = color.g % 16 + 48;
	instantColor[5] = color.b / 16 + 48;
	instantColor[6] = color.b % 16 + 48;
	return instantColor;
}