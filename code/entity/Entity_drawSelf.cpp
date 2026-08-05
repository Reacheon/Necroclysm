import Entity;

#include <SDL3/SDL.h>

import globalVar;
import constVar;
import World;
import ItemData;
import textureVar;
import drawSprite;
import drawText;
import util;
import drawPrimitive;
import SkillBehavior;
import SkillRegistry;
import Sprite;
import EntityData;

// 돌연변이 스프라이트 조회. 없으면 nullptr.
// 색 소스가 지정된 경우 "<base>_<색>" -> "<base>_<기본색>" -> "<base>" 순서로 폴백.
// 색상명은 palette/*.tsv의 헤더와 일치해야 함 (팔레트 스왑 결과가 spriteMapper에 등록됨).
static Sprite* resolveMutSprite(const std::wstring& base, const EntityData& info, mutColorSource src)
{
	if (base.empty()) return nullptr;

	auto tryFind = [](const std::wstring& key) -> Sprite* {
		auto it = spr::spriteMapper.find(key);
		return (it != spr::spriteMapper.end()) ? it->second : nullptr;
	};

	switch (src)
	{
	case mutColorSource::none:
		return tryFind(base);

	case mutColorSource::fur:
		if (Sprite* s = tryFind(base + L"_" + info.furColor)) return s;
		if (Sprite* s = tryFind(base + L"_GRAY")) return s;
		return tryFind(base);

	case mutColorSource::horn:
		if (Sprite* s = tryFind(base + L"_" + info.hornColor)) return s;
		if (Sprite* s = tryFind(base + L"_BROWN")) return s;
		return tryFind(base);
	}
	return nullptr;
}

// 지정 레이어에 속한 돌연변이 스프라이트를 우선도순으로 현재 렌더타겟에 그림.
static void drawMutationLayer(EntityData& info, mutDrawLayer layer)
{
	std::map<int, Sprite*, std::less<int>> order;
	for (const auto& sd : info.skillList)
	{
		SkillBehavior* bhv = SkillRegistry::get(sd.skillId);
		if (bhv == nullptr) continue;
		if (bhv->src != skillSrc::MUTATION) continue;
		if (bhv->mutLayer != layer) continue;
		Sprite* s = resolveMutSprite(bhv->mutSprBaseName, info, bhv->mutColorSrc);
		if (s == nullptr) continue;
		order[bhv->mutDrawPriority] = s;
	}
	for (auto& [prio, s] : order)
	{
		drawTexture(s->getTexture(), 0, 0);
	}
}

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
		playerTexture = composePlayerTexture();
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

	int originX = (cameraW / 2) + zoomScale * ((getX() - cameraX) + getIntegerFakeX());
	int originY = (cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY());

	int drawingX = originX + zoomScale * (offsetX);
	int drawingY = originY + zoomScale * (offsetY);

	// Nervedrive 잔상 캡처 (애니메이션 중일 때만)
	if (entityInfo.isPlayer && nervedriveOn && getAniType() != aniFlag::null)
	{
		Uint32 now = SDL_GetTicks();
		if (now - lastAfterImageTime >= 16)
		{
			afterImages.push_back({ getX(), getY(), getIntegerFakeX(), getIntegerFakeY(),
				localSprIndex, entityInfo.sprFlip, PlayerInfo().sprAngle,
				entityInfo.jumpOffsetY, offsetX, offsetY, now });
			lastAfterImageTime = now;
			while (afterImages.size() > 12) afterImages.pop_front();
		}
	}
	// 오래된 잔상 제거 (350ms 경과)
	while (!afterImages.empty() && SDL_GetTicks() - afterImages.front().captureTime > 350)
		afterImages.pop_front();

	//캐릭터 그림자 그리기
	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW) == false && itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP) == false)
	{
		if (ridingEntity == nullptr)
		{

			SDL_SetTextureAlphaMod(spr::shadow->getTexture(), 170); //텍스쳐 투명도 설정

			if (TileFloor(getGridX(), getGridY(), getGridZ()) != itemID::shallowSeaWater &&
				TileFloor(getGridX(), getGridY(), getGridZ()) != itemID::deepSeaWater &&
				entityInfo.jumpOffsetY == 0)
			{
				if (TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemID::shallowSeaWater ||
					TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemID::shallowSeaWater ||
					TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemID::shallowSeaWater ||
					TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemID::deepSeaWater ||
					TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemID::deepSeaWater ||
					TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemID::deepSeaWater
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


	// Nervedrive 잔상 렌더링 (플레이어 스프라이트보다 먼저 그려서 뒤에 깔리게)
	if (entityInfo.isPlayer && playerSprite != nullptr && !afterImages.empty())
	{
		Uint32 now = SDL_GetTicks();
		for (auto& ghost : afterImages)
		{
			Uint32 age = now - ghost.captureTime;
			if (age > 350) continue;

			// 페이드아웃: 새로운 잔상은 밝고 오래된 잔상은 서서히 사라짐
			float lifeRatio = 1.0f - (float)age / 350.0f;
			Uint8 alpha = (Uint8)(lifeRatio * 255.0f);
			if (alpha < 3) continue;

			// 월드좌표 → 현재 카메라 기준 스크린좌표 변환
			int gOriginX = (cameraW / 2) + zoomScale * (ghost.worldX - cameraX + ghost.fakeX);
			int gOriginY = (cameraH / 2) + zoomScale * (ghost.worldY - cameraY + ghost.fakeY);
			int gDrawX = gOriginX + zoomScale * ghost.offsetX;
			int gDrawY = gOriginY + zoomScale * ghost.offsetY;

			// 전기빛 푸른색 틴트 + 가산 블렌딩으로 네온 발광 효과
			setFlip(ghost.flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
			SDL_SetTextureColorMod(playerSprite.get()->getTexture(), 40, 140, 255);
			SDL_SetTextureAlphaMod(playerSprite.get()->getTexture(), alpha);
			SDL_SetTextureBlendMode(playerSprite.get()->getTexture(), SDL_BLENDMODE_ADD);
			drawSpriteCenter(playerSprite.get(), ghost.sprIndex, gDrawX, gDrawY + zoomScale * ghost.jumpOffsetY, ghost.angle);
		}
		// 텍스처 상태 복원
		SDL_SetTextureColorMod(playerSprite.get()->getTexture(), 255, 255, 255);
		SDL_SetTextureAlphaMod(playerSprite.get()->getTexture(), 255);
		SDL_SetTextureBlendMode(playerSprite.get()->getTexture(), SDL_BLENDMODE_BLEND);
		setFlip(entityInfo.sprFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
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
			drawSpriteCenter(playerSprite.get(), localSprIndex, drawingX, drawingY + zoomScale*entityInfo.jumpOffsetY, PlayerInfo().sprAngle);//캐릭터 본체 그리기
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

	if (TileFloor(getGridX(), getGridY(), getGridZ()) != itemID::shallowSeaWater&& 
		TileFloor(getGridX(), getGridY(), getGridZ()) != itemID::deepSeaWater&&
		entityInfo.jumpOffsetY == 0)
	{
		if (
			TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemID::deepSeaWater ||
			TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemID::deepSeaWater ||
			TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemID::deepSeaWater
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
		else if (TileFloor(getGridX() + 1, getGridY(), getGridZ()) == itemID::shallowSeaWater ||
			TileFloor(getGridX() - 1, getGridY(), getGridZ()) == itemID::shallowSeaWater ||
			TileFloor(getGridX(), getGridY() + 1, getGridZ()) == itemID::shallowSeaWater
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
		if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemID::shallowSeaWater || TileFloor(getGridX(), getGridY(), getGridZ()) == itemID::shallowFreshWater)
		{
			int tileAniExtraIndexSingle = ((SDL_GetTicks() / 150) % 4);
			Sprite* targetSpr;
			if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemID::shallowSeaWater) targetSpr = spr::seaFoam;
			else targetSpr = spr::waterFoam;
			SDL_SetTextureAlphaMod(targetSpr->getTexture(), 150);
			drawSpriteCenterExSrc(targetSpr, tileAniExtraIndexSingle, originX, originY - 3 * zoomScale, { 3,0,10,16 });
			SDL_SetTextureAlphaMod(targetSpr->getTexture(), 255);
		}
		else if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemID::deepSeaWater || TileFloor(getGridX(), getGridY(), getGridZ()) == itemID::deepFreshWater)
		{
			int tileAniExtraIndexSingle = ((SDL_GetTicks() / 150) % 4);
			Sprite* targetSpr;
			if (TileFloor(getGridX(), getGridY(), getGridZ()) == itemID::deepSeaWater) targetSpr = spr::seaFoam;
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

// 플레이어 외형 전 레이어(스킨/눈/머리/수염/뿔/장비/돌연변이)를 288x384 텍스처로 합성.
// 반환된 SDL_Texture*는 호출자가 SDL_DestroyTexture로 해제.
SDL_Texture* Entity::composePlayerTexture(bool allowBlink)
{
	SDL_Texture* playerTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CHAR_TEXTURE_WIDTH, CHAR_TEXTURE_HEIGHT);
	SDL_SetTextureScaleMode(playerTexture, SDL_SCALEMODE_NEAREST);

	SDL_SetRenderTarget(renderer, playerTexture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	// 피부 레이어
	// - skinColor 빈 문자열이면 피부 없음 (해골 등)
	// - 키 컨벤션: SKIN_<gender>_<skinColor> (예: SKIN_MALE_LIGHT, SKIN_FEMALE_TAN)
	//   gender PNG 추가(예: SKIN_CHILD.png) + skin.tsv 컬럼 추가만으로 자동 확장됨
	if (entityInfo.skinColor.empty() == false)
	{
		auto it = spr::spriteMapper.find(L"SKIN_" + entityInfo.gender + L"_" + entityInfo.skinColor);
		if (it != spr::spriteMapper.end()) drawTexture(it->second->getTexture(), 0, 0);
	}

	//돌연변이 레이어: 전신 털, 꼬리 (skin 위, eyes 아래)
	drawMutationLayer(entityInfo, mutDrawLayer::underEyes);

	// 눈 레이어
	// - eyeColor 빈 문자열이면 눈 없음 (해골/특수 종 등)
	// - isEyesClose / isEyesHalf 는 수면 / 기절 등에서 외부가 강제하는 영구 상태
	// - 두 플래그가 모두 false일 때만 자연 깜빡임(BLINK)을 합성
	// - 자연 깜빡임 주기: BLINK_PERIOD_MS. 2300~2500ms 구간이 HALF→CLOSE→HALF의 짧은 닫힘
	// - 엔티티별 phase offset(this 포인터 기반)으로 다수 NPC가 동시에 깜빡이는 것을 방지
	if (entityInfo.eyeColor.empty() == false)
	{
		constexpr Uint32 BLINK_PERIOD_MS    = 5000;
		constexpr Uint32 BLINK_HALF_IN_MS   = 4800; // [4800,4870) 감기 시작 (HALF)
		constexpr Uint32 BLINK_CLOSE_MS     = 4870; // [4870,4930) 완전히 감김 (CLOSE)
		constexpr Uint32 BLINK_HALF_OUT_MS  = 4930; // [4930,5000) 다시 뜨는 중 (HALF)

		const wchar_t* stateStem = L"EYES_OPEN";
		if (entityInfo.isEyesClose) stateStem = L"EYES_CLOSE";
		else if (entityInfo.isEyesHalf) stateStem = L"EYES_HALF";
		else if (allowBlink)
		{
			Uint32 phaseOffset = (Uint32)((uintptr_t)this % BLINK_PERIOD_MS);
			Uint32 phase = (SDL_GetTicks() + phaseOffset) % BLINK_PERIOD_MS;
			if      (phase >= BLINK_CLOSE_MS    && phase < BLINK_HALF_OUT_MS) stateStem = L"EYES_CLOSE";
			else if (phase >= BLINK_HALF_IN_MS) stateStem = L"EYES_HALF"; // HALF_IN, HALF_OUT 모두 커버
		}

		auto it = spr::spriteMapper.find(std::wstring(stateStem) + L"_" + entityInfo.eyeColor);
		if (it != spr::spriteMapper.end()) drawTexture(it->second->getTexture(), 0, 0);
	}

	if (entityInfo.scar != humanCustom::scar::null)
	{
	}

	if (entityInfo.beard != humanCustom::beard::null)
	{
		if (entityInfo.beard == humanCustom::beard::mustache) drawTexture(spr::beardMustacheBlack->getTexture(), 0, 0);
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

	// 머리카락 렌더를 스킵해야 하는지 먼저 판정 (NO_HAIR_HELMET: 풀페이스 투구 등).
	// DRAW_ABOVE_HAIR과는 독립적인 플래그임에 유의. 후자는 순서만 제어하고 전자는 hair 자체를 숨긴다.
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

	// hair 레이어 draw 람다. 장비 pass 사이에서 정확히 한 번 호출된다.
	auto drawHairLayer = [&]() {
		if (entityInfo.hairStyle.empty()) return;
		if (noHair) return;
		// spriteMapper["<스타일>_<색>"] 조회. 색 매칭 실패 시 BLACK 폴백.
		std::wstring key = entityInfo.hairStyle + L"_" + entityInfo.hairColor;
		auto it = spr::spriteMapper.find(key);
		if (it == spr::spriteMapper.end())
			it = spr::spriteMapper.find(entityInfo.hairStyle + L"_BLACK");
		if (it != spr::spriteMapper.end())
			drawTexture(it->second->getTexture(), 0, 0);
	};

	//캐릭터 장비 그리기
	// 레이어 순서: (horn 위에) 일반장비 → hair → DRAW_ABOVE_HAIR 장비(투구 등) → 손에 든 무기.
	// 아이템은 DRAW_ABOVE_HAIR 플래그 유무로 두 버킷에 분배되며, 각 버킷 안에서는 기존 우선도 정렬을 유지한다.
	if (getEquipPtr()->itemInfo.size() > 0)
	{
		std::map<int, Sprite*, std::less<int>> drawOrderBelowHair;
		std::map<int, Sprite*, std::less<int>> drawOrderAboveHair;

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
				// EQUIP_SPR_GENDERED: 로드 시점에 포인터를 해석하지 않은 장비. 매 프레임 착용자
				// 성별로 spriteMapper에서 "<base>_<gender>"를 조회한다 (TOGGLE과 결합 케이스는 현재 없음).
				if (entityInfo.sprFlip == false)
				{
					priority = tgtItem.equipPriority;
					if (tgtItem.checkFlag(itemFlag::EQUIP_SPR_GENDERED))
					{
						auto it = spr::spriteMapper.find(tgtItem.equipSprName + L"_" + entityInfo.gender);
						tgtSpr = (it != spr::spriteMapper.end()) ? it->second : nullptr;
					}
					else
					{
						tgtSpr = (Sprite*)tgtItem.equipSpr;
						if (tgtItem.checkFlag(itemFlag::HAS_TOGGLE_SPRITE) && tgtItem.checkFlag(itemFlag::TOGGLE_ON)) tgtSpr = (Sprite*)tgtItem.equipSprToggleOn;
					}
				}
				else
				{
					priority = tgtItem.flipEquipPriority;
					if (tgtItem.checkFlag(itemFlag::EQUIP_SPR_GENDERED))
					{
						auto it = spr::spriteMapper.find(tgtItem.flipEquipSprName + L"_" + entityInfo.gender);
						tgtSpr = (it != spr::spriteMapper.end()) ? it->second : nullptr;
					}
					else
					{
						tgtSpr = (Sprite*)tgtItem.flipEquipSpr;
						if (tgtItem.checkFlag(itemFlag::HAS_TOGGLE_SPRITE) && tgtItem.checkFlag(itemFlag::TOGGLE_ON)) tgtSpr = (Sprite*)tgtItem.flipEquipSprToggleOn;
					}
				}
				break;
			default:
				errorBox(L"장비 그리기 중에 equipState가 비정상적인 값인 장비를 발견");
				break;
			}

			auto& bucket = tgtItem.checkFlag(itemFlag::DRAW_ABOVE_HAIR) ? drawOrderAboveHair : drawOrderBelowHair;
			bucket[priority] = tgtSpr;
		}

		// Pass 1: hair 아래에 그릴 장비 (옷/바지/신발 등)
		for (auto it = drawOrderBelowHair.begin(); it != drawOrderBelowHair.end(); it++)
		{
			if (it->second != nullptr)
			{
				drawTexture(it->second->getTexture(), 0, 0);
			}
		}

		// hair
		drawHairLayer();

		// Pass 2: hair 위에 그릴 장비 (투구/모자 등)
		for (auto it = drawOrderAboveHair.begin(); it != drawOrderAboveHair.end(); it++)
		{
			if (it->second != nullptr)
			{
				drawTexture(it->second->getTexture(), 0, 0);
			}
		}

		//개별 아이템을 들었을 때의 범용 스프라이트
		for (int equipCounter = 0; equipCounter < getEquipPtr()->itemInfo.size(); equipCounter++)
		{
			ItemData& tgtItem = getEquipPtr()->itemInfo[equipCounter];
			if (getEquipPtr()->itemInfo[equipCounter].leftWieldSpr == nullptr && getEquipPtr()->itemInfo[equipCounter].rightWieldSpr == nullptr)
			{
				switch (getEquipPtr()->itemInfo[equipCounter].equipState)
				{
				case equipHandFlag::both:
					for (int i = 0; i < 48; i++)
					{
						Point2 itemCoor = equipCoordTwoHanded[i];
						if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, tgtItem.getSprIndex(), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
					}
					break;
				case equipHandFlag::left:
					if (entityInfo.sprFlip == false)
					{
						for (int i = 0; i < 48; i++)
						{
							Point2 itemCoor = equipCoordLArm[i];
							if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, tgtItem.getSprIndex(), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
						}
					}
					else
					{
						for (int i = 0; i < 48; i++)
						{
							Point2 itemCoor = equipCoordRArm[i];
							if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, tgtItem.getSprIndex(), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
						}
					}
					break;
				case equipHandFlag::right:
					if (entityInfo.sprFlip == false)
					{
						for (int i = 0; i < 48; i++)
						{
							Point2 itemCoor = equipCoordRArm[i];
							if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, tgtItem.getSprIndex(), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
						}
					}
					else
					{
						for (int i = 0; i < 48; i++)
						{
							Point2 itemCoor = equipCoordLArm[i];
							if (itemCoor.x != 0 && itemCoor.y != 0) drawSpriteCenter(spr::itemset, tgtItem.getSprIndex(), 48 * (i % 6) + itemCoor.x, 48 * (i / 6) + itemCoor.y);
						}
					}
					break;
				}
			}
		}
	}
	else
	{
		// 장비가 하나도 없어도 hair는 그려야 한다.
		drawHairLayer();
	}

	//돌연변이 레이어: 주둥이, 귀 (모든 장비 위)
	drawMutationLayer(entityInfo, mutDrawLayer::aboveEquip);

	SDL_SetRenderTarget(renderer, frameTarget);
	return playerTexture;
}