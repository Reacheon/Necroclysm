#include <SDL.h>

export module renderSticker;

import std;
import util;
import Sticker;
import drawSprite;
import constVar;
import drawText;
import globalVar;

export __int64 renderSticker(int cameraX, int cameraY)
{
	__int64 timeStampStart = getNanoTimer();

	/*
	고정텍스쳐의 깊이(Depth)
	key 문자열 값 앞에 붙어있는 숫자가 큰 것부터 먼저 그림
	예로 123BASEATK가 그려진 후에 121 BASEATK가 그려짐
	*/
	for (auto it = StickerList.begin(); it != StickerList.end(); it++)
	{
		Sticker* address = (Sticker*)it->second;

		if (address->getViewFix() == true)
		{
			cameraX = 0;
			cameraY = 0;
		}

		if (address->getSpriteMode() == 0)
		{
			if (address->getIsCenter() == true)
			{
				setZoom(zoomScale);
				drawSpriteCenter
				(
					address->getSprite(),
					address->getSpriteIndex(),
					(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getIntegerFakeX()),
					(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
				);
				setZoom(1.0);
			}
			else
			{
				setZoom(zoomScale);
				drawSprite
				(
					address->getSprite(),
					address->getSpriteIndex(),
					(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getIntegerFakeX()),
					(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
				);
				setZoom(1.0);
			}

		}
		else
		{
			if (address->getFont() != nullptr)
			{
				SDL_SetRenderDrawColor(renderer, address->getColor().r, address->getColor().g, address->getColor().b, 0xff);
				setFontSize(address->getFontSize());
				drawTextEx(address->getString(), cameraW / 2 + address->getX() - cameraX, cameraH / 2 + address->getY() - cameraY, address->getIsCenter());
			}
		}
	}

	return (getNanoTimer() - timeStampStart);
}