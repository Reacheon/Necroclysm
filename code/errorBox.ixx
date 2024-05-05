#include <SDL.h>

export module errorBox;

import std;

export void errorBox(bool condition, const char* text)
{
	if (condition == true)
	{
		std::string stringText = text;
		std::string nullText = "     "; //뒤에 넣으면 문자열이 짤린다.
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", (stringText + nullText).c_str(), NULL);
		exit(-1);
	}
}

export void errorBox(const char* text)
{
	errorBox(1,text);
}


export void errorBox(bool condition, std::wstring text)
{
	if (condition == true)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring stringText = text;
		stringText += L"                                                          ";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", converter.to_bytes(stringText).c_str(), NULL);
		exit(-1);
	}
}

export void errorBox(std::wstring text)
{
	errorBox(1, text);
}