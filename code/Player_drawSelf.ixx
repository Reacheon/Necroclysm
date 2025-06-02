#include <SDL3/SDL.h>

export module Player_drawSelf;

import util;
import std;
import Player;
import errorBox;
import globalVar;
import textureVar;
import wrapVar;
import drawSprite;
import drawText;
import drawPrimitive;

void Player::drawSelf()
{
	SDL_Texture* targetTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CHAR_TEXTURE_WIDTH, CHAR_TEXTURE_HEIGHT);
	SDL_SetTextureScaleMode(targetTexture, SDL_SCALEMODE_NEAREST);

	SDL_SetRenderTarget(renderer, targetTexture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	if (skin != humanCustom::skin::null)
	{
		if (skin == humanCustom::skin::yellow) drawTexture(spr::skinYellow->getTexture(), 0, 0);
	}

	if (eyes != humanCustom::eyes::null)
	{
		if (eyes == humanCustom::eyes::blue) drawTexture(spr::eyesBlue->getTexture(), 0, 0);
		else if (eyes == humanCustom::eyes::red) drawTexture(spr::eyesRed->getTexture(), 0, 0);
		else if (eyes == humanCustom::eyes::closed) drawTexture(spr::eyesClosed->getTexture(), 0, 0);
	}

	if (scar != humanCustom::scar::null)
	{
	}

	if (beard != humanCustom::beard::null)
	{
		if (beard == humanCustom::beard::mustache) drawTexture(spr::beardMustacheBlack->getTexture(), 0, 0);
	}

	if (hair != humanCustom::hair::null)
	{
		bool noHair = false;
		for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
		{
			ItemData& tgtItem = getEquipPtr()->itemInfo[i];
			if (tgtItem.checkFlag(itemFlag::NO_HAIR_HELMET) == true
				&& tgtItem.equipState == equipHandFlag::normal)
			{
				noHair = true;
				break;
			}
		}

		if (noHair == false)
		{
			switch (hair)
			{
			case humanCustom::hair::commaBlack:
				drawTexture(spr::hairCommaBlack->getTexture(), 0, 0);
				break;
			case humanCustom::hair::bob1Black:
				drawTexture(spr::hairBob1Black->getTexture(), 0, 0);
				break;
			case humanCustom::hair::ponytail:
				drawTexture(spr::hairPonytailBlack->getTexture(), 0, 0);
				break;
			case humanCustom::hair::middlePart:
				drawTexture(spr::hairMiddlePart->getTexture(), 0, 0);
				break;
			}
		}
	}

	if (horn != humanCustom::horn::null)
	{
		switch (horn)
		{
		case humanCustom::horn::coverRed:
			drawTexture(spr::hornCoverRed->getTexture(), 0, 0);
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//캐릭터 장비 그리기
	if (getEquipPtr()->itemInfo.size() > 0)
	{
		std::map<int, Sprite*, std::less<int>> drawOrder;

		for (int equipCounter = 0; equipCounter < getEquipPtr()->itemInfo.size(); equipCounter++)
		{
			int priority = 0;
			Sprite* tgtSpr = nullptr;
			ItemData& tgtItem = getEquipPtr()->itemInfo[equipCounter];
			switch (getEquipPtr()->itemInfo[equipCounter].equipState)
			{
			case equipHandFlag::left:
			case equipHandFlag::both:
				if (entityFlip == false)
				{
					priority = tgtItem.leftWieldPriority;
					tgtSpr = (Sprite*)tgtItem.leftWieldSpr;
				}
				else
				{
					priority = tgtItem.rightWieldPriority;
					tgtSpr = (Sprite*)tgtItem.rightWieldSpr;
				}

				break;
			case equipHandFlag::right:
				if (entityFlip == false)
				{
					priority = tgtItem.rightWieldPriority;
					tgtSpr = (Sprite*)tgtItem.rightWieldSpr;
				}
				else
				{
					priority = tgtItem.leftWieldPriority;
					tgtSpr = (Sprite*)tgtItem.leftWieldSpr;
				}
				break;
			case equipHandFlag::normal:
				if (entityFlip == false)
				{
					priority = tgtItem.equipPriority;
					tgtSpr = (Sprite*)tgtItem.equipSpr;
					if (tgtItem.checkFlag(itemFlag::HAS_TOGGLE_SPRITE) && tgtItem.checkFlag(itemFlag::TOGGLE_ON)) tgtSpr = (Sprite*)tgtItem.equipSprToggleOn;
				}
				else
				{
					priority = tgtItem.flipEquipPriority;
					tgtSpr = (Sprite*)tgtItem.flipEquipSpr;
					if (tgtItem.checkFlag(itemFlag::HAS_TOGGLE_SPRITE) && tgtItem.checkFlag(itemFlag::TOGGLE_ON)) tgtSpr = (Sprite*)tgtItem.flipEquipSprToggleOn;
				}
				break;
			default:
				errorBox(L"장비 그리기 중에 equipState가 비정상적인 값인 장비를 발견");
				break;
			}
			//errorBox(drawOrder.find(priority) != drawOrder.end(), L"이미 존재하는 우선도의 장비가 추가됨 :" + std::to_wstring(priority) + L" 이름: " + getEquipPtr()->itemInfo[equipCounter].name);
			drawOrder[priority] = tgtSpr;
		}

		for (auto it = drawOrder.begin(); it != drawOrder.end(); it++)
		{
			if (it->second != nullptr)
			{
				drawTexture(it->second->getTexture(), 0, 0);
			}
		}

	}

	SDL_SetRenderTarget(renderer, nullptr);
	customSprite = std::make_unique<Sprite>(renderer, targetTexture, 48, 48);






	setZoom(zoomScale);
	if (entityFlip == false) setFlip(SDL_FLIP_NONE);
	else setFlip(SDL_FLIP_HORIZONTAL);


	int localSprIndex = getSpriteIndex();
	int offsetX = 0;
	int offsetY = 0;


	if (getSpriteIndex() >= 0 && getSpriteIndex() <= 2)
	{
		if (walkMode == walkFlag::walk || walkMode == walkFlag::wade)
		{
		}
		else if (walkMode == walkFlag::run)
		{
			localSprIndex += 6;
		}
		else if (walkMode == walkFlag::crouch)
		{
			localSprIndex += 12;
		}
		else if (walkMode == walkFlag::crawl || walkMode == walkFlag::swim)
		{
			localSprIndex += 18;
		}


		if (walkMode != walkFlag::crawl && walkMode != walkFlag::swim)
		{
			for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
			{
				if (getEquipPtr()->itemInfo[i].equipState == equipHandFlag::both)
				{
					if (getEquipPtr()->itemInfo[i].checkFlag(itemFlag::SPR_TH_WEAPON))
					{
						localSprIndex += 3;
						break;
					}
				}
			}
		}
	}

	if (ridingEntity != nullptr && ridingType == ridingFlag::horse)
	{
		offsetX = 0;
		offsetY = -9;

		if (walkMode == walkFlag::walk)
		{
			if (localSprIndex % 3 == 1 || localSprIndex % 3 == 2)
			{
				offsetY += 1;
			}
			localSprIndex = 0;
		}
		else if (walkMode == walkFlag::run)
		{
			if (localSprIndex % 3 == 1 || localSprIndex % 3 == 2)
			{
				offsetY += 1;
			}
			localSprIndex = 6;
		}
	}


	int originX = (cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX());
	int originY = (cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY());

	int drawingX = originX + zoomScale * (offsetX);
	int drawingY = originY + zoomScale * (offsetY);


	//캐릭터 그림자 그리기
	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW) == false && itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP) == false)
	{
		if (ridingEntity == nullptr)
		{
			drawSpriteCenter(spr::shadow, 1, originX, originY);
		}
		else if (ridingEntity != nullptr && ridingType == ridingFlag::horse)
		{
			drawSpriteCenter(spr::shadow, 2, originX, originY);
			drawSpriteCenter(ridingEntity.get()->entitySpr, getSpriteIndex(), originX, originY);
		}
	}


	//캐릭터 커스타미이징 그리기
	SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND);

	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW))
	{
		drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,0,48,24 });
		SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 130); //텍스쳐 투명도 설정
		SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,24 });
		SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 255); //텍스쳐 투명도 설정
	}
	else if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP))
	{
		drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,0,48,27 });
		SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 80); //텍스쳐 투명도 설정
		SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,21 });
		SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 255); //텍스쳐 투명도 설정
	}
	else
	{
		drawSpriteCenter(customSprite.get(), localSprIndex, drawingX, drawingY);//캐릭터 본체 그리기
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool doDrawHP = false;
	if (HP != maxHP) doDrawHP = true;

	if (doDrawHP)//개체 HP 표기
	{
		int pivotX = drawingX - (int)(8 * zoomScale);
		int pivotY = drawingY + (int)((-8 + hpBarHeight) * zoomScale);
		SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::black);

		//페이크 HP
		if (fakeHP > HP) { fakeHP--; }
		else if (fakeHP < HP) fakeHP = HP;
		if (fakeHP != HP)
		{
			if (fakeHPAlpha > 30) { fakeHPAlpha -= 30; }
			else { fakeHPAlpha = 0; }
		}
		else { fakeHPAlpha = 255; }

		//페이크 MP
		if (fakeMP > MP) { fakeMP--; }
		else if (fakeMP < MP) fakeMP = MP;
		if (fakeMP != MP)
		{
			if (fakeMPAlpha > 30) { fakeMPAlpha -= 30; }
			else { fakeMPAlpha = 0; }
		}
		else { fakeMPAlpha = 255; }


		float ratioFakeHP = myMax((float)0.0, (fakeHP) / (float)(maxHP));
		dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::white, fakeHPAlpha);

		float ratioHP = myMax((float)0.0, (float)(HP) / (float)(maxHP));
		dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
		if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (isPlayer) drawFillRect(dst, lowCol::green);
		else drawFillRect(dst, lowCol::red);
	}

	if (0)//개체 이름 표기
	{
		int mouseX = getAbsMouseGrid().x;
		int mouseY = getAbsMouseGrid().y;

		if (getGridX() == mouseX && getGridY() == mouseY && isPlayer == false)
		{
			int pivotX = drawingX - (int)(8 * zoomScale);
			int pivotY = drawingY + (int)((-8 + hpBarHeight) * zoomScale);

			if (zoomScale == 1.0) setFontSize(8);
			else if (zoomScale == 2.0) setFontSize(10);
			else if (zoomScale == 3.0) setFontSize(11);
			else if (zoomScale == 4.0) setFontSize(14);
			else if (zoomScale == 5.0) setFontSize(16);

			int textX = pivotX + (int)(8 * zoomScale);
			int textY = pivotY - (int)(3 * zoomScale);
			if (doDrawHP == true) textY = pivotY - (int)(3 * zoomScale);
			else textY = pivotY - (int)(0 * zoomScale);

			if (zoomScale == 1.0) textY -= (int)(1 * zoomScale);

			renderTextOutlineCenter(name, textX, textY);
		}
	}

	auto drawFlashEffect = [this](Sprite* sprite, int sprIndex, int x, int y)
		{
			if (flashType == 0 || flash.a == 0) return; // 플래시 효과 없음

			// 원본 블렌드 모드와 컬러 저장
			SDL_BlendMode originalBlend;
			Uint8 originalR, originalG, originalB, originalA;
			SDL_GetTextureBlendMode(sprite->getTexture(), &originalBlend);
			SDL_GetTextureColorMod(sprite->getTexture(), &originalR, &originalG, &originalB);
			SDL_GetTextureAlphaMod(sprite->getTexture(), &originalA);

			// 플래시 효과 설정
			SDL_SetTextureBlendMode(sprite->getTexture(), SDL_BLENDMODE_ADD);
			SDL_SetTextureColorMod(sprite->getTexture(), flash.r, flash.g, flash.b);
			SDL_SetTextureAlphaMod(sprite->getTexture(), flash.a);

			// 플래시 효과로 그리기
			drawSpriteCenter(sprite, sprIndex, x, y);

			// 원본 설정 복원
			SDL_SetTextureBlendMode(sprite->getTexture(), originalBlend);
			SDL_SetTextureColorMod(sprite->getTexture(), originalR, originalG, originalB);
			SDL_SetTextureAlphaMod(sprite->getTexture(), originalA);
		};

	if (getFlashType() != 0)
	{
		drawFlashEffect(entitySpr, localSprIndex, drawingX, drawingY);
	}

	if (ridingEntity != nullptr && ridingType == ridingFlag::horse)//말 앞쪽
	{
		drawSpriteCenter(ridingEntity.get()->entitySpr, getSpriteIndex() + 4, originX, originY);
	}

	setZoom(1.0);
	setFlip(SDL_FLIP_NONE);
}