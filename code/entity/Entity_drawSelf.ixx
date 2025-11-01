import Entity;

#include <SDL3/SDL.h>

import globalVar;
import constVar;
import wrapVar;
import ItemData;
import textureVar;
import drawSprite;
import drawText;
import util;
import drawPrimitive;

constexpr std::array<std::array<int, 2>, 48> equipCoordLArm =
{ {
	{29, 22},{29, 23},{28, 21},{0,0},{0,0},{0,0},
	{30, 23},{31, 26},{28, 22},{0,0},{0,0},{0,0},
	{30,27},{29,26},{31,27},{0,0},{0,0},{0,0},
	{31,29},{32,31},{32,28},{31,24},{31,22},{27,18},
	{31,24},{0,0},{0,0},{0,0},{27,10},{31,24},
	{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
	{29,21},{29,23},{0,0},{0,0},{0,0},{0,0},
	{31,27}, {0,0},{0,0},{0,0},{0,0},{0,0}
} };

constexpr std::array<std::array<int, 2>, 48> equipCoordRArm =
{ {
		{18, 24},{19,23},{24,24},{0,0},{0,0},{0,0},
		{17, 23},{16,22},{23,25},{0,0},{0,0},{0,0},
		{17, 26},{18,27},{17,26},{0,0},{0,0},{0,0},
		{27, 28},{24,29},{27,30},{18,27},{15,22},{26,29},
		{19,24},{0,0},{0,0},{0,0},{23,23},{16,21},
		{16,15},{25,23},{0,0},{0,0},{17,23},{18,24},
		{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
		{17,23}, {0,0},{0,0},{0,0},{0,0},{0,0}
} };

constexpr std::array<std::array<int, 2>, 48> equipCoordTwoHanded =
{ {
	{0,0},{0,0},{0,0},{24,22},{24,23},{24,23},
	{0,0},{0,0},{0,0},{24,23},{24,24},{24,24},
	{0,0},{0,0},{0,0},{25,24},{25,25},{25,25},
	{28,28},{28,30},{29,29},{18,28},{15,23},{27,30},
	{21,25},{0,0},{0,0},{0,0},{0,0},{0,0},
	{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
	{0,0},{0,0},{0,0},{0,0},{16,9},{29,24},
	{16,24}, {0,0},{0,0},{0,0},{0,0},{0,0}
} };



void Entity::drawSelf()
{
	stepEvent();
	std::unique_ptr<Sprite> playerSprite = nullptr;
	SDL_Texture* playerTexture = nullptr;

	if (entityInfo.isPlayer)
	{
		playerTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CHAR_TEXTURE_WIDTH, CHAR_TEXTURE_HEIGHT);
		SDL_SetTextureScaleMode(playerTexture, SDL_SCALEMODE_NEAREST);

		SDL_SetRenderTarget(renderer, playerTexture);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		if (entityInfo.skin != humanCustom::skin::null)
		{
			if (entityInfo.skin == humanCustom::skin::yellow) drawTexture(spr::skinYellow->getTexture(), 0, 0);
		}

		if (entityInfo.eyes != humanCustom::eyes::null)
		{
			if (entityInfo.isEyesClose == false)
			{
				if (entityInfo.eyes == humanCustom::eyes::blue) drawTexture(spr::eyesBlue->getTexture(), 0, 0);
				else if (entityInfo.eyes == humanCustom::eyes::blueHalf) drawTexture(spr::eyesBlue->getTexture(), 0, 0);
				else if (entityInfo.eyes == humanCustom::eyes::red) drawTexture(spr::eyesRed->getTexture(), 0, 0);

				if(entityInfo.isEyesHalf)  drawTexture(spr::eyesHalf->getTexture(), 0, 0);
			}
			else drawTexture(spr::eyesClosed->getTexture(), 0, 0);
		}

		if (entityInfo.scar != humanCustom::scar::null)
		{
		}

		if (entityInfo.beard != humanCustom::beard::null)
		{
			if (entityInfo.beard == humanCustom::beard::mustache) drawTexture(spr::beardMustacheBlack->getTexture(), 0, 0);
		}

		if (entityInfo.hair != humanCustom::hair::null)
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
				switch (entityInfo.hair)
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

		if (entityInfo.horn != humanCustom::horn::null)
		{
			switch (entityInfo.horn)
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
					if (entityInfo.sprFlip == false)
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
					if (entityInfo.sprFlip == false)
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
					if (entityInfo.sprFlip == false)
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

			//개별 아이템을 들었을 때의 범용 스프라이트
			for (int equipCounter = 0; equipCounter < getEquipPtr()->itemInfo.size(); equipCounter++)
			{
				Sprite* tgtSpr = nullptr;
				ItemData& tgtItem = getEquipPtr()->itemInfo[equipCounter];
				if (getEquipPtr()->itemInfo[equipCounter].leftWieldSpr == nullptr && getEquipPtr()->itemInfo[equipCounter].rightWieldSpr == nullptr)
				{
					switch (getEquipPtr()->itemInfo[equipCounter].equipState)
					{
					case equipHandFlag::both:
						for (int i = 0; i < 48; i++)
						{
							Point2 itemCoor = equipCoordTwoHanded[i];
							if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, getItemSprIndex(tgtItem), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);

						}
						break;
					case equipHandFlag::left:
						if (entityInfo.sprFlip == false)
						{
							for (int i = 0; i < 48; i++)
							{
								Point2 itemCoor = equipCoordLArm[i];
								if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, getItemSprIndex(tgtItem), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
							}
						}
						else
						{
							for (int i = 0; i < 48; i++)
							{
								Point2 itemCoor = equipCoordRArm[i];
								if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, getItemSprIndex(tgtItem), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
							}
						}
						break;

					case equipHandFlag::right:
						if (entityInfo.sprFlip == false)
						{
							for (int i = 0; i < 48; i++)
							{
								Point2 itemCoor = equipCoordRArm[i];
								if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, getItemSprIndex(tgtItem), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
							}
						}
						else
						{
							for (int i = 0; i < 48; i++)
							{
								Point2 itemCoor = equipCoordLArm[i];
								if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, getItemSprIndex(tgtItem), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
							}
						}
						break;
					}
				}
			}

		}

		SDL_SetRenderTarget(renderer, nullptr);
		playerSprite = std::make_unique<Sprite>(renderer, playerTexture, 48, 48);
	}





	setZoom(zoomScale);
	if (entityInfo.sprFlip == false) setFlip(SDL_FLIP_NONE);
	else setFlip(SDL_FLIP_HORIZONTAL);


	int localSprIndex = getSpriteIndex();
	int offsetX = 0;
	int offsetY = 0;

	if (entityInfo.isPlayer)
	{
		if (getSpriteIndex() >= 0 && getSpriteIndex() <= 2)
		{
			if (entityInfo.walkMode == walkFlag::walk || entityInfo.walkMode == walkFlag::wade)
			{
			}
			else if (entityInfo.walkMode == walkFlag::run)
			{
				localSprIndex += 6;
			}
			else if (entityInfo.walkMode == walkFlag::crouch)
			{
				localSprIndex += 12;
			}
			else if (entityInfo.walkMode == walkFlag::crawl || entityInfo.walkMode == walkFlag::swim)
			{
				localSprIndex += 18;
			}


			if (entityInfo.walkMode != walkFlag::crawl && entityInfo.walkMode != walkFlag::swim)
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

			if (entityInfo.walkMode == walkFlag::walk)
			{
				if (localSprIndex % 3 == 1 || localSprIndex % 3 == 2)
				{
					offsetY += 1;
				}
				localSprIndex = 0;
			}
			else if (entityInfo.walkMode == walkFlag::run)
			{
				if (localSprIndex % 3 == 1 || localSprIndex % 3 == 2)
				{
					offsetY += 1;
				}
				localSprIndex = 6;
			}
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

			SDL_SetTextureAlphaMod(spr::shadow->getTexture(), 170); //텍스쳐 투명도 설정

			if (TileFloor(getGridX(), getGridY(), getGridZ()) != itemRefCode::shallowSeaWater &&
				TileFloor(getGridX(), getGridY(), getGridZ()) != itemRefCode::deepSeaWater &&
				entityInfo.jumpOffsetY == 0)
			{
				if (TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemRefCode::shallowSeaWater ||
					TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemRefCode::shallowSeaWater ||
					TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemRefCode::shallowSeaWater ||
					TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemRefCode::deepSeaWater ||
					TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemRefCode::deepSeaWater ||
					TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemRefCode::deepSeaWater
					)
				{
					int waveExtraIndex = 16 * ((SDL_GetTicks() / 300) % 7);
					if (waveExtraIndex / 16 == 2 || waveExtraIndex / 16 == 3 || waveExtraIndex / 16 == 4)
					{
						SDL_SetTextureAlphaMod(spr::shadow->getTexture(), 60); //텍스쳐 투명도 설정

					}
				}
			}


			SDL_SetTextureBlendMode(spr::shadow->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			drawSpriteCenter(spr::shadow, 1, originX, originY);
			SDL_SetTextureAlphaMod(spr::shadow->getTexture(), 255); //텍스쳐 투명도 설정
		}
		else if (ridingEntity != nullptr && ridingType == ridingFlag::horse)
		{
			drawSpriteCenter(spr::shadow, 2, originX, originY);
			drawSpriteCenter(ridingEntity.get()->entityInfo.entitySpr, getSpriteIndex(), originX, originY);
		}
	}
	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW))
	{
		if (ridingEntity == nullptr)
		{
			SDL_SetTextureAlphaMod(spr::shadow->getTexture(), 130); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::shadow->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			drawSpriteCenter(spr::shadow, 1, originX, originY);
			SDL_SetTextureAlphaMod(spr::shadow->getTexture(), 255); //텍스쳐 투명도 설정
		}
	}


	//캐릭터 커스타미이징 그리기
	if (entityInfo.isPlayer)
	{
		SDL_SetTextureBlendMode(playerSprite.get()->getTexture(), SDL_BLENDMODE_BLEND);

		
		
		if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW) && entityInfo.jumpOffsetY == 0 && getAniType() != aniFlag::roll)
		{
			drawSpriteCenterExSrc(playerSprite.get(), localSprIndex, drawingX, drawingY, { 0,0,48,24 });
			SDL_SetTextureAlphaMod(playerSprite.get()->getTexture(), 130); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(playerSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			drawSpriteCenterExSrc(playerSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,24 });
			SDL_SetTextureAlphaMod(playerSprite.get()->getTexture(), 255); //텍스쳐 투명도 설정
		}
		else if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP) && entityInfo.jumpOffsetY == 0 && getAniType() != aniFlag::roll)
		{
			drawSpriteCenterExSrc(playerSprite.get(), localSprIndex, drawingX, drawingY, { 0,0,48,27 });
			SDL_SetTextureAlphaMod(playerSprite.get()->getTexture(), 80); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(playerSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			drawSpriteCenterExSrc(playerSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,21 });
			SDL_SetTextureAlphaMod(playerSprite.get()->getTexture(), 255); //텍스쳐 투명도 설정
		}
		else
		{
			drawSpriteCenter(playerSprite.get(), localSprIndex, drawingX, drawingY + zoomScale*entityInfo.jumpOffsetY, PlayerPtr->entityInfo.sprAngle);//캐릭터 본체 그리기
		}
	}
	else
	{
		drawSpriteCenter(entityInfo.entitySpr, localSprIndex, drawingX, drawingY + zoomScale * entityInfo.jumpOffsetY);//캐릭터 본체 그리기
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (entityInfo.displayHPBarCount > 0 && entityInfo.HP>0)//개체 HP 표기
	{

		int pivotX = drawingX - (int)(8 * zoomScale);
		int pivotY = drawingY + (int)((-8 + entityInfo.hpBarHeight) * zoomScale);
		SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::black, entityInfo.alphaHPBar);

		//페이크 HP
		if (entityInfo.fakeHP > entityInfo.HP) { entityInfo.fakeHP--; }
		else if (entityInfo.fakeHP < entityInfo.HP) entityInfo.fakeHP = entityInfo.HP;
		if (entityInfo.fakeHP != entityInfo.HP)
		{
			if (entityInfo.fakeHPAlpha > 30) { entityInfo.fakeHPAlpha -= 30; }
			else { entityInfo.fakeHPAlpha = 0; }
		}
		else { entityInfo.fakeHPAlpha = 0; }

		float ratioFakeHP = myMax((float)0.0, (entityInfo.fakeHP) / (float)(entityInfo.maxHP));
		dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::white, entityInfo.fakeHPAlpha);

		float ratioHP = myMax((float)0.0, (float)(entityInfo.HP) / (float)(entityInfo.maxHP));
		dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
		if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (entityInfo.isPlayer) drawFillRect(dst, lowCol::green, entityInfo.alphaHPBar);
		else drawFillRect(dst, lowCol::red, entityInfo.alphaHPBar);


		if (entityInfo.displayHPBarCount > 1) entityInfo.displayHPBarCount--;
		else if (entityInfo.displayHPBarCount == 1)
		{
			entityInfo.alphaHPBar -= 10;
			if (entityInfo.alphaHPBar <= 0)
			{
				entityInfo.alphaHPBar = 0;
				entityInfo.displayHPBarCount = 0;
			}
		}
	}

	if (0)//개체 이름 표기
	{
		int mouseX = getAbsMouseGrid().x;
		int mouseY = getAbsMouseGrid().y;

		if (getGridX() == mouseX && getGridY() == mouseY && entityInfo.isPlayer == false)
		{
			int pivotX = drawingX - (int)(8 * zoomScale);
			int pivotY = drawingY + (int)((-8 + entityInfo.hpBarHeight) * zoomScale);

			if (zoomScale == 1.0) setFontSize(8);
			else if (zoomScale == 2.0) setFontSize(10);
			else if (zoomScale == 3.0) setFontSize(11);
			else if (zoomScale == 4.0) setFontSize(14);
			else if (zoomScale == 5.0) setFontSize(16);

			int textX = pivotX + (int)(8 * zoomScale);
			int textY = pivotY - (int)(3 * zoomScale);

			if (zoomScale == 1.0) textY -= (int)(1 * zoomScale);

			drawTextOutlineCenter(entityInfo.name, textX, textY);
		}
	}

	if (flash.a > 0)
	{
		if(playerSprite !=nullptr) drawFlashEffectBlendCenter(playerSprite.get(), localSprIndex, drawingX, drawingY, flash);
		else  drawFlashEffectBlendCenter(entityInfo.entitySpr, localSprIndex, drawingX, drawingY, flash);
		
		SDL_Color tgtCol = { 0, 0, 0, flash.a };
		float speed = 0.15f;
		//flash.r = flash.r + (tgtCol.r - flash.r) * speed;
		//flash.g = flash.g + (tgtCol.g - flash.g) * speed;
		//flash.b = flash.b + (tgtCol.b - flash.b) * speed;
		flash.a = (Uint8)(flash.a * 0.85f);
		if (flash.a < 5) flash.a = 0;
	}

	if (ridingEntity != nullptr && ridingType == ridingFlag::horse)//말 앞쪽
	{
		drawSpriteCenter(ridingEntity.get()->entityInfo.entitySpr, getSpriteIndex() + 4, originX, originY);
	}

	if (TileFloor(getGridX(), getGridY(), getGridZ()) != itemRefCode::shallowSeaWater&& 
		TileFloor(getGridX(), getGridY(), getGridZ()) != itemRefCode::deepSeaWater&&
		entityInfo.jumpOffsetY == 0)
	{
		if (
			TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemRefCode::deepSeaWater ||
			TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemRefCode::deepSeaWater ||
			TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemRefCode::deepSeaWater
			)
		{
			int waveExtraIndex = 16 * ((SDL_GetTicks() / 300) % 7);
			if (waveExtraIndex / 16 == 2 || waveExtraIndex / 16 == 3 || waveExtraIndex / 16 == 4)
			{
				SDL_SetTextureAlphaMod(spr::waveFoam->getTexture(), 200);
				drawSpriteCenter(spr::waveFoam, waveExtraIndex / 16 - 2 + 4, originX, originY);
				SDL_SetTextureAlphaMod(spr::waveFoam->getTexture(), 255);
			}
		}
		else if (TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemRefCode::shallowSeaWater ||
			TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemRefCode::shallowSeaWater ||
			TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemRefCode::shallowSeaWater
			)
		{
			int waveExtraIndex = 16 * ((SDL_GetTicks() / 300) % 7);
			if (waveExtraIndex / 16 == 2 || waveExtraIndex / 16 == 3 || waveExtraIndex / 16 == 4)
			{
				SDL_SetTextureAlphaMod(spr::waveFoam->getTexture(), 200);
				drawSpriteCenter(spr::waveFoam, waveExtraIndex / 16 - 2, originX, originY);
				SDL_SetTextureAlphaMod(spr::waveFoam->getTexture(), 255);
			}
		}
	}
	
	if (entityInfo.jumpOffsetY == 0)
	{
		if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemRefCode::shallowSeaWater || TileFloor(getGridX(), getGridY(), getGridZ()) == itemRefCode::shallowFreshWater)
		{
			int tileAniExtraIndexSingle = ((SDL_GetTicks() / 150) % 4);
			Sprite* targetSpr;
			if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemRefCode::shallowSeaWater) targetSpr = spr::seaFoam;
			else targetSpr = spr::waterFoam;
			SDL_SetTextureAlphaMod(targetSpr->getTexture(), 150);
			drawSpriteCenterExSrc(targetSpr, tileAniExtraIndexSingle, originX, originY - 3 * zoomScale, { 3,0,10,16 });
			SDL_SetTextureAlphaMod(targetSpr->getTexture(), 255);
		}
		else if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemRefCode::deepSeaWater || TileFloor(getGridX(), getGridY(), getGridZ()) == itemRefCode::deepFreshWater)
		{
			int tileAniExtraIndexSingle = ((SDL_GetTicks() / 150) % 4);
			Sprite* targetSpr;
			if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemRefCode::deepSeaWater) targetSpr = spr::seaFoam;
			else targetSpr = spr::waterFoam;
			SDL_SetTextureAlphaMod(targetSpr->getTexture(), 150);
			drawSpriteCenterExSrc(targetSpr, tileAniExtraIndexSingle, originX, originY, { 1,0,14,16 });
			SDL_SetTextureAlphaMod(targetSpr->getTexture(), 255);
		}
	}

	setZoom(1.0);
	setFlip(SDL_FLIP_NONE);

	SDL_DestroyTexture(playerTexture);
};