module;
#include <SDL3/SDL.h>

export module errorBox;

import std;
import utf8Decoder; //utf8Encoder 사용

export void errorBox(bool condition, std::wstring text)
{
	if (condition == true)
	{
		std::wstring stringText = text;
		stringText += L"                                                          ";
		const std::string utf8 = utf8Encoder(stringText);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", utf8.c_str(), NULL);
		throw std::runtime_error(utf8.c_str());
	}
}

export [[noreturn]] void errorBox(std::wstring text) { errorBox(1, text); }