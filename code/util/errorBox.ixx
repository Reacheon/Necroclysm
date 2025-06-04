#include <SDL3/SDL.h>

export module errorBox;

import std;

export void errorBox(bool condition, std::wstring text)
{
	if (condition == true)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring stringText = text;
		stringText += L"                                                          ";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", converter.to_bytes(stringText).c_str(), NULL);
		throw std::runtime_error(converter.to_bytes(stringText).c_str());
		exit(-1);
	}
}

export void errorBox(std::wstring text) {errorBox(1, text);}