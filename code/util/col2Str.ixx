module;
#include <SDL3/SDL.h>

export module col2Str;

import std;

export std::wstring col2Str(SDL_Color color)
{
	constexpr wchar_t hex[] = L"0123456789ABCDEF";
	std::wstring instantColor = L"#000000";
	instantColor[1] = hex[color.r / 16];
	instantColor[2] = hex[color.r % 16];
	instantColor[3] = hex[color.g / 16];
	instantColor[4] = hex[color.g % 16];
	instantColor[5] = hex[color.b / 16];
	instantColor[6] = hex[color.b % 16];
	return instantColor;
}