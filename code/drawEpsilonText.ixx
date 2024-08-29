//#include <SDL.h>;
//
//export module drawEpsilonText;
//
//import std;
//import util;
//import textureVar;
//import constVar;
//import drawSprite;
//
//export void drawEplsionText(std::wstring inputStr, int x, int y, SDL_Color fontCol, SDL_Color shadowCol)
//{
//	for (int i = 0; i < inputStr.size(); i++)
//	{
//		SDL_SetTextureBlendMode(spr::epsilonFont->getTexture(), SDL_BLENDMODE_BLEND);
//		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), shadowCol.r, shadowCol.g, shadowCol.b);
//
//		int sprIndex = 0;
//		if (inputStr[i] >= UNI::A || inputStr[i] <= UNI::Z) sprIndex = inputStr[i] - UNI::A + 1;
//		else if (inputStr[i] >= UNI::ZERO || inputStr[i] <= UNI::NINE)  sprIndex = inputStr[i] - UNI::ZERO + 27;
//		else if (inputStr[i] == UNI::r) sprIndex = 37;
//		else if (inputStr[i] == UNI::MIDDLE_DOT) sprIndex = 38;
//		else if (inputStr[i] == UNI::PLUS_SIGN) sprIndex = 39;
//		else if (inputStr[i] == UNI::MINUS_SIGN) sprIndex = 40;
//		else if (inputStr[i] == UNI::x) sprIndex = 41;
//		else if (inputStr[i] == UNI::SLASH) sprIndex = 42;
//		else if (inputStr[i] == UNI::PERCENT_SIGN) sprIndex = 43;
//		else if (inputStr[i] == UNI::QUESTION_MARK) sprIndex = 44;
//		else if (inputStr[i] == UNI::t) sprIndex = 45;
//
//		drawSprite(spr::epsilonFont, sprIndex, x + 1, y);
//		drawSprite(spr::epsilonFont, sprIndex, x - 1, y);
//		drawSprite(spr::epsilonFont, sprIndex, x, y + 1);
//		drawSprite(spr::epsilonFont, sprIndex, x, y - 1);
//
//		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), fontCol.r, fontCol.g, fontCol.b);
//		drawSprite(spr::epsilonFont, sprIndex, x, y);
//		SDL_SetTextureColorMod(spr::epsilonFont->getTexture(), 255, 255, 255);
//	}
//};
//
//
//export void drawEplsionText(auto* spr, int spriteIndex, int x, int y, SDL_Color fontCol)
//{
//	SDL_SetTextureColorMod(spr->getTexture(), fontCol.r, fontCol.g, fontCol.b);
//	drawSprite(spr, spriteIndex, x, y);
//	SDL_SetTextureColorMod(spr->getTexture(), 255, 255, 255);
//};