#include <SDL3/SDL.h>;

export module drawEpsilonText;

import std;
import util;
import textureVar;
import constVar;
import drawSprite;

export void drawEplsionText(std::wstring inputStr, int x, int y, SDL_Color fontCol, SDL_Color shadowCol)
{
	int cursorX = x;
	for (int i = 0; i < inputStr.size(); i++)
	{
		int sprIndex = 0;
		if (inputStr[i] >= UNI::A && inputStr[i] <= UNI::Z) sprIndex = inputStr[i] - UNI::A + 1;
		else if (inputStr[i] >= UNI::ZERO && inputStr[i] <= UNI::NINE)  sprIndex = inputStr[i] - UNI::ZERO + 27;
		else if (inputStr[i] == UNI::r) sprIndex = 37;
		else if (inputStr[i] == UNI::MIDDLE_DOT) sprIndex = 38;
		else if (inputStr[i] == UNI::PLUS_SIGN) sprIndex = 39;
		else if (inputStr[i] == UNI::MINUS_SIGN) sprIndex = 40;
		else if (inputStr[i] == UNI::x) sprIndex = 41;
		else if (inputStr[i] == UNI::SLASH) sprIndex = 42;
		else if (inputStr[i] == UNI::PERCENT_SIGN) sprIndex = 43;
		else if (inputStr[i] == UNI::QUESTION_MARK) sprIndex = 44;
		else if (inputStr[i] == UNI::t) sprIndex = 45;
		else if (inputStr[i] == UNI::PERIOD) sprIndex = 48;

		SDL_SetTextureBlendMode(spr::epsilonFont->getTexture(), SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), shadowCol.r, shadowCol.g, shadowCol.b);
		drawSprite(spr::epsilonFont, sprIndex, cursorX + 1, y);
		drawSprite(spr::epsilonFont, sprIndex, cursorX - 1, y);
		drawSprite(spr::epsilonFont, sprIndex, cursorX, y + 1);
		drawSprite(spr::epsilonFont, sprIndex, cursorX, y - 1);

		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), fontCol.r, fontCol.g, fontCol.b);
		drawSprite(spr::epsilonFont, sprIndex, cursorX, y);
		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), 255, 255, 255);

		cursorX += 4;
		if (inputStr[i] == UNI::PERIOD) cursorX -= 2;
	}
};

export void drawEplsionText(std::wstring inputStr, int x, int y, SDL_Color fontCol)
{
	int cursorX = x;
	for (int i = 0; i < inputStr.size(); i++)
	{
		int sprIndex = 0;
		if (inputStr[i] >= UNI::A && inputStr[i] <= UNI::Z) sprIndex = inputStr[i] - UNI::A + 1;
		else if (inputStr[i] >= UNI::ZERO && inputStr[i] <= UNI::NINE)  sprIndex = inputStr[i] - UNI::ZERO + 27;
		else if (inputStr[i] == UNI::r) sprIndex = 37;
		else if (inputStr[i] == UNI::MIDDLE_DOT) sprIndex = 38;
		else if (inputStr[i] == UNI::PLUS_SIGN) sprIndex = 39;
		else if (inputStr[i] == UNI::MINUS_SIGN) sprIndex = 40;
		else if (inputStr[i] == UNI::x) sprIndex = 41;
		else if (inputStr[i] == UNI::SLASH) sprIndex = 42;
		else if (inputStr[i] == UNI::PERCENT_SIGN) sprIndex = 43;
		else if (inputStr[i] == UNI::QUESTION_MARK) sprIndex = 44;
		else if (inputStr[i] == UNI::t) sprIndex = 45;
		else if (inputStr[i] == UNI::PERIOD) sprIndex = 48;

		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), fontCol.r, fontCol.g, fontCol.b);
		drawSprite(spr::epsilonFont, sprIndex, cursorX, y);
		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), 255, 255, 255);

		cursorX += 4;
		if (inputStr[i] == UNI::PERIOD) cursorX -= 2;
	}
};


export void drawEplsionText(auto* spr, int spriteIndex, int x, int y, SDL_Color fontCol)
{
	SDL_SetTextureColorMod(spr->getTexture(), fontCol.r, fontCol.g, fontCol.b);
	drawSprite(spr, spriteIndex, x, y);
	SDL_SetTextureColorMod(spr->getTexture(), 255, 255, 255);
};


export int getEpsilonTextWidth(std::wstring inputStr)
{
	int width = 0;
	for (int i = 0; i < inputStr.size(); i++)
	{
		width += 4; // 기본 글자 간격
		if (inputStr[i] == UNI::PERIOD) width -= 2; // 마침표는 2픽셀 좁게
	}
	return width - 1; // 마지막 글자 뒤 여백 제거
}

export int getEpsilonTextHeight()
{
	return 5;
}