#include <SDL.h>

export module drawSprite;

import Sprite;
import globalVar;

static float s_zoomScale = 1.0;
static SDL_RendererFlip s_flip = SDL_FLIP_NONE;
static int s_angle = 0;
static SDL_Point s_anglePoint = { 0,0 };
static unsigned __int8 s_alpha = 255;

export void setAngle(int angle, SDL_Point anglePoint) { s_angle = angle; s_anglePoint = anglePoint; }
export void setFlip(SDL_RendererFlip flip) { s_flip = flip; }
export void setZoom(float scale) { s_zoomScale = scale; }
export void setAlpha(float val) { s_alpha = val; }
export void drawTexture(SDL_Texture* texture, int x, int y)
{
	int textureW, textureH;
	SDL_QueryTexture(texture, nullptr, nullptr, &textureW, &textureH);
	SDL_Rect src = { 0,0,textureW,textureH };
	SDL_Rect dst = { x,y,textureW * s_zoomScale,textureH * s_zoomScale };
	SDL_RenderCopyEx(renderer, texture, &src, &dst, s_angle, NULL, s_flip);
}
export void drawTextureCenter(SDL_Texture* texture, int x, int y)
{
	int textureW, textureH;
	SDL_QueryTexture(texture, nullptr, nullptr, &textureW, &textureH);
	SDL_Rect src = { 0,0,textureW,textureH };
	SDL_Rect dst = { x,y,textureW * s_zoomScale,textureH * s_zoomScale };
	dst.x = x - (dst.w / 2);
	dst.y = y - (dst.h / 2);
	SDL_RenderCopyEx(renderer, texture, &src, &dst, s_angle, NULL, s_flip);
}
export void drawSprite(Sprite* spr, int index, int x, int y)
{
	int textureW, textureH;
	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);
	SDL_Rect src = { spr->getW() * index % textureW, spr->getH() * (spr->getW() * index / textureW), spr->getW(), spr->getH() };
	SDL_Rect dst = { x,y,src.w * s_zoomScale, src.h * s_zoomScale };
	SDL_RenderCopyEx(renderer, spr->getTexture(), &src, &dst, 0, NULL, s_flip);
}

export void drawSpriteRotate(Sprite* spr, int index, int x, int y, double rotateAngle, SDL_Point* rotateCenter)
{
	int textureW, textureH;
	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);

	SDL_Rect src = {
		(spr->getW() * index) % textureW,
		spr->getH() * ((spr->getW() * index) / textureW),
		spr->getW(),
		spr->getH()
	};

	SDL_Rect dst = {
		x,
		y,
		src.w * s_zoomScale,
		src.h * s_zoomScale
	};

	SDL_RenderCopyEx(renderer, spr->getTexture(), &src, &dst, rotateAngle, rotateCenter, s_flip);
}

export void drawSprite(Sprite* spr, int x, int y) { drawSprite(spr, 0, x, y); }

export void drawSpriteCenter(Sprite* spr, int index, int x, int y)
{
	int textureW, textureH;
	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);

	int srcX = (spr->getW() * index) % textureW;
	int srcY = (spr->getH() * (spr->getW() * index / textureW));
	SDL_Rect src = { srcX, srcY, spr->getW(), spr->getH() };

	int dstW = static_cast<int>(std::floor(src.w * s_zoomScale));
	int dstH = static_cast<int>(std::floor(src.h * s_zoomScale));
	SDL_Rect dst = { x - dstW / 2, y - dstH / 2, dstW, dstH };

	SDL_RenderCopyEx(renderer, spr->getTexture(), &src, &dst, 0, NULL, s_flip);
}

export void drawSpriteCenterRotate(Sprite* spr, int index, int x, int y, double rotateAngle, SDL_Point* rotateCenter)
{
	int textureW, textureH;
	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);

	int srcX = (spr->getW() * index) % textureW;
	int srcY = (spr->getH() * ((spr->getW() * index) / textureW));
	SDL_Rect src = { srcX, srcY, spr->getW(), spr->getH() };

	int dstW = static_cast<int>(std::floor(src.w * s_zoomScale));
	int dstH = static_cast<int>(std::floor(src.h * s_zoomScale));

	SDL_Rect dst = { x - dstW / 2, y - dstH / 2, dstW, dstH };

	SDL_Point center;
	if (rotateCenter == nullptr) {
		center = { dstW / 2, dstH / 2 };
	}
	else {
		center = *rotateCenter;
	}

	SDL_RenderCopyEx(renderer, spr->getTexture(), &src, &dst, rotateAngle, &center, s_flip);
}

export void drawSpriteCenterExSrc(Sprite* spr, int index, int x, int y, SDL_Rect exSrc)
{
	int textureW, textureH;
	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);

	int srcX = (spr->getW() * index) % textureW;
	int srcY = (spr->getH() * (spr->getW() * index / textureW));
	SDL_Rect src = { srcX + exSrc.x, srcY + exSrc.y, exSrc.w, exSrc.h };

	int dstW = static_cast<int>(std::floor(src.w * s_zoomScale));
	int dstH = static_cast<int>(std::floor(src.h * s_zoomScale));

	double offsetX = (exSrc.x + exSrc.w / 2.0) - (spr->getW() / 2.0);
	double offsetY = (exSrc.y + exSrc.h / 2.0) - (spr->getH() / 2.0);
	int dstOffsetX = static_cast<int>(std::floor(offsetX * s_zoomScale));
	int dstOffsetY = static_cast<int>(std::floor(offsetY * s_zoomScale));

	SDL_Rect dst = { x - dstW / 2 + dstOffsetX, y - dstH / 2 + dstOffsetY, dstW, dstH };

	SDL_RenderCopyEx(renderer, spr->getTexture(), &src, &dst, 0, NULL, s_flip);
}

//export void drawSpriteCenter(Sprite* spr, int index, int x, int y)
//{
//	int textureW, textureH;
//	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);
//	SDL_Rect src = { spr->getW() * index % textureW, spr->getH() * (spr->getW() * index / textureW), spr->getW(), spr->getH() };
//	SDL_Rect dst = { x,y,src.w * s_zoomScale, src.h * s_zoomScale };
//	dst.x = x - (dst.w / 2);
//	dst.y = y - (dst.h / 2);
//	SDL_RenderCopyEx(renderer, spr->getTexture(), &src, &dst, 0, NULL, s_flip);
//}


export void drawSpriteCenterF(Sprite* spr, int index, double x, double y)
{
	int textureW, textureH;
	SDL_QueryTexture(spr->getTexture(), nullptr, nullptr, &textureW, &textureH);
	SDL_Rect src = { spr->getW() * index % textureW, spr->getH() * (spr->getW() * index / textureW), spr->getW(), spr->getH() };
	SDL_FRect dst = { x,y,src.w * s_zoomScale, src.h * s_zoomScale };
	dst.x = x - (dst.w / 2);
	dst.y = y - (dst.h / 2);
	SDL_RenderCopyExF(renderer, spr->getTexture(), &src, &dst, 0, NULL, s_flip);
}


