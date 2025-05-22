#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

export module Sticker;

import std;
import util;
import Sprite;
import Coord;
import globalVar;

export class Sticker : public Coord
{
private:
	Sprite* sprite;
	std::wstring key;
	int spriteIndex;
	int textureW;
	int textureH;
	bool isCenter;
	int spriteMode; // 0 : image, 1 : font
	bool viewFix;
	std::wstring string;
	TTF_Font* font;
	SDL_Color color;
	int fontSize;
public:
	Uint8 alpha = 255;
	float rotateAngle = 0.0;
	Point2 rotateCenter = { 0,0 };
	Sticker(bool inputViewFix, int inputX, int inputY, Sprite* inputSprite, int inputIndex, std::wstring inputKey, bool inputIsCenter, Uint8 inputAlpha = 255, float inputRotateAngle = 0.0, Point2 inputRotateCenter = { 0,0 })//mode 0 : image
	{
		spriteMode = 0;
		setXY(inputX, inputY);
		sprite = inputSprite;
		spriteIndex = inputIndex;
		key = inputKey;
		StickerList.insert({ inputKey,this });
		isCenter = inputIsCenter;
		viewFix = inputViewFix;

		alpha = inputAlpha;
		rotateAngle = inputRotateAngle;
		rotateCenter = inputRotateCenter;
	}
	Sticker(bool inputViewFix, int inputX, int inputY, std::wstring inputString, TTF_Font* inputFont, SDL_Color inputColor, std::wstring inputKey, bool inputIsCenter, int inputFontSize) //mode 1 : font
	{
		spriteMode = 1;
		setXY(inputX, inputY);
		sprite = nullptr;
		spriteIndex = -1;
		key = inputKey;
		StickerList.insert({ inputKey,this });
		textureW = -1;
		textureH = -1;
		isCenter = inputIsCenter;
		viewFix = inputViewFix;

		string = inputString;
		font = inputFont;// if font is nullptr, print epsilonFont
		color = inputColor;
		fontSize = inputFontSize;
	}
	~Sticker()
	{
		prt(L"Sticker : 소멸자가 호출되었습니다..\n");
		StickerList.erase(key);
	}
	Sprite* getSprite() { return sprite; }
	int getSpriteMode() { return spriteMode; }
	int getSpriteIndex() { return spriteIndex; }
	void setSpriteIndex(int inputIndex) { spriteIndex = inputIndex; }
	bool getIsCenter() { return isCenter; }
	bool getViewFix() { return viewFix; }
	std::wstring getString() { return string; }
	TTF_Font* getFont() { return font; }
	SDL_Color getColor() { return color; }
	int getFontSize() { return fontSize; }
};