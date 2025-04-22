//타일캐시 없는 버전
#include <SDL.h>


export module renderTile;

import std;
import util;
import globalVar;
import wrapVar;
import constVar;
import textureVar;
import checkCursor;
import ItemStack;
import Entity;
import World;
import Player;
import drawSprite;
import drawText;
import Damage;
import Corpse;
import Craft;
import Vehicle;
import GUI;
import Prop;
import globalTime;
import Drawable;
import TileData;
import Flame;
import mouseGrid;
import HUD;
import Bullet;
import Particle;
import Footprint;

SDL_Rect dst, renderRegion;
int tileSize, cameraGridX, cameraGridY, renderRangeW, renderRangeH, pZ;


__int64 analyseRender();
__int64 drawTiles();
__int64 drawFootprints();
__int64 drawCorpses();
__int64 drawItems();
__int64 drawEntities();
__int64 drawDamages();
__int64 drawBullets();
__int64 drawParticles();
__int64 drawFogs();
__int64 drawMarkers();
__int64 drawDebug();

//차량과 엔티티는 중복을 허용하면 안됨
std::list<Point2> tileList, itemList, floorPropList, gasList, blackFogList, grayFogList, lightFogList, flameList,allTileList;
std::list<Drawable*> renderVehList, renderEntityList;

export __int64 renderTile()
{
	__int64 timeStampStart = getNanoTimer();

	tileSize = 16 * zoomScale;
	cameraGridX = (cameraX - 8) / (16);
	cameraGridY = (cameraY - 8) / (16);
	renderRangeW = 3 + (cameraW + extraCameraLength) / tileSize;
	renderRangeH = 3 + (cameraH + extraCameraLength) / tileSize;
	pZ = PlayerZ();
	renderRegion = { cameraGridX - (renderRangeW / 2), cameraGridY - (renderRangeH / 2), renderRangeW, renderRangeH };

	tileList.clear();
	itemList.clear();
	floorPropList.clear();
	renderVehList.clear();
	renderEntityList.clear();
	gasList.clear();
	blackFogList.clear();
	grayFogList.clear();
	lightFogList.clear();
	flameList.clear();

	dur::analysis = analyseRender();
	dur::tile = drawTiles();
	drawFootprints();
	dur::corpse = drawCorpses();
	dur::item = drawItems();
	dur::entity = drawEntities();
	dur::damage = drawDamages();
	drawBullets();
	drawParticles();
	dur::fog = drawFogs();
	dur::marker = drawMarkers();
	drawDebug();

	//전체광
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_Rect screenRect = { 0,0,cameraW,cameraH };
	drawFillRect(screenRect, mainLightColor, mainLightBright);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	return (getNanoTimer() - timeStampStart);
}
__int64 analyseRender()
{
	__int64 timeStampStart = getNanoTimer();

	//루프 분석
	for (int tgtY = renderRegion.y - 1; tgtY < renderRegion.y + renderRegion.h + 1; tgtY++)
	{
		for (int tgtX = renderRegion.x - 1; tgtX < renderRegion.x + renderRegion.w + 1; tgtX++)
		{
			TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, pZ);

			//바깥쪽이면 타일캐시만 추가하고 거기에 있는 내용은 렌더링하지 않음
			if (tgtX < renderRegion.x || tgtX >= renderRegion.x + renderRegion.w) continue;
			if (tgtY < renderRegion.y || tgtY >= renderRegion.y + renderRegion.h) continue;

			//바닥과 벽
			if (thisTile->fov != fovFlag::black) tileList.push_back({ tgtX,tgtY });

			//아이템
			if (thisTile->fov == fovFlag::white)
			{
				if (TileItemStack(tgtX, tgtY, pZ) != nullptr) itemList.push_back({ tgtX,tgtY });
			}
			//바닥프롭
			Prop* fpPtr = (Prop*)thisTile->PropPtr;
			if (fpPtr != nullptr && fpPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER)) floorPropList.push_back({ tgtX,tgtY });

			//화염
			Flame* flamePtr = (Flame*)thisTile->flamePtr;
			if (flamePtr != nullptr) flameList.push_back({ tgtX,tgtY });

			//차량
			Drawable* vPtr = (Drawable*)((Vehicle*)(thisTile->VehiclePtr));
			if (vPtr != nullptr) renderVehList.push_back(vPtr);

			//플레이어와 겹치는 일반설치물
			Prop* pPtr = (Prop*)(thisTile->PropPtr);
			if (pPtr != nullptr && pPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER) == false) renderEntityList.push_back((Drawable*)pPtr);

			//일반 객체
			Drawable* ePtr = (Drawable*)((Entity*)(thisTile->EntityPtr));
			if (ePtr != nullptr) renderEntityList.push_back(ePtr);

			//가스
			if (thisTile->gasVec.size() > 0) gasList.push_back({ tgtX,tgtY });

			//안개
			if (thisTile->fov == fovFlag::black) blackFogList.push_back({ tgtX, tgtY });
			else if (thisTile->fov == fovFlag::gray) grayFogList.push_back({ tgtX, tgtY });
			else  lightFogList.push_back({ tgtX, tgtY });
		}
	}

	for (auto it = extraRenderVehList.begin(); it != extraRenderVehList.end(); it++) renderVehList.push_back((Drawable*)(Vehicle*)(*it));
	for (auto it = extraRenderEntityList.begin(); it != extraRenderEntityList.end(); it++) renderEntityList.push_back((Drawable*)(Entity*)(*it));

	return getNanoTimer() - timeStampStart;
}


__int64 drawTiles()
{
	__int64 timeStampStart = getNanoTimer();

	for (const auto& elem : tileList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;
		const TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());
		const TileData* topTile = &World::ins()->getTile(tgtX, tgtY - 1, PlayerZ());
		const TileData* botTile = &World::ins()->getTile(tgtX, tgtY + 1, PlayerZ());
		const TileData* leftTile = &World::ins()->getTile(tgtX - 1, tgtY, PlayerZ());
		const TileData* rightTile = &World::ins()->getTile(tgtX + 1, tgtY, PlayerZ());

		switch (thisTile->floor)//바닥 타일 그리기 
		{
		default:
			setZoom(zoomScale);

			int dirCorrection = 0;
			int tileAniExtraIndex16 = 0;
			int tileAniExtraIndexSingle = 0;
			if (itemDex[thisTile->floor].tileConnectGroup != -1)
			{
				bool topCheck, botCheck, leftCheck, rightCheck;
				if (itemDex[thisTile->floor].tileConnectGroup == 0)
				{
					int currentTileFloor = thisTile->floor;
					int	topTileFloor = topTile->floor;
					int	botTileFloor = botTile->floor;
					int	leftTileFloor = leftTile->floor;
					int	rightTileFloor = rightTile->floor;

					topCheck = currentTileFloor == topTileFloor;
					botCheck = currentTileFloor == botTileFloor;
					leftCheck = currentTileFloor == leftTileFloor;
					rightCheck = currentTileFloor == rightTileFloor;
				}
				else
				{
					int currentTileGroup = itemDex[thisTile->floor].tileConnectGroup;
					int	topTileGroup = itemDex[topTile->floor].tileConnectGroup;
					int	botTileGroup = itemDex[botTile->floor].tileConnectGroup;
					int	leftTileGroup = itemDex[leftTile->floor].tileConnectGroup;
					int	rightTileGroup = itemDex[rightTile->floor].tileConnectGroup;

					topCheck = currentTileGroup == topTileGroup;
					botCheck = currentTileGroup == botTileGroup;
					leftCheck = currentTileGroup == leftTileGroup;
					rightCheck = currentTileGroup == rightTileGroup;
				}

				dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

				if (itemDex[thisTile->floor].animeSize > 1)
				{
					int animeFPS = itemDex[thisTile->floor].animeFPS;
					int animeSize = itemDex[thisTile->floor].animeSize;
					tileAniExtraIndex16 = getMilliTimer() / animeFPS % animeSize;

				}
			}
			else
			{
				if (itemDex[thisTile->floor].animeSize > 1)
				{
					int animeFPS = itemDex[thisTile->floor].animeFPS;
					int animeSize = itemDex[thisTile->floor].animeSize;
					tileAniExtraIndexSingle = getMilliTimer() / animeFPS % animeSize;

				}
			}

			int sprIndex = itemDex[thisTile->floor].tileSprIndex + itemDex[thisTile->floor].extraSprIndexSingle + 16 * itemDex[thisTile->floor].extraSprIndex16;
			sprIndex += 16 * tileAniExtraIndex16 + tileAniExtraIndexSingle;
			if (thisTile->floor == 0) sprIndex = 506;

			if (thisTile->floor == 220)
			{
				if (getSeason() == seasonFlag::winter)
				{
					sprIndex += 16;
				}
				else if (getSeason() == seasonFlag::winter)
				{
					sprIndex += 32;

				}
			}

			drawSpriteCenter
			(
				spr::tileset,
				sprIndex + dirCorrection,
				cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
				cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
			);

			setZoom(1.0);



			break;
		}

		if (thisTile->hasSnow == true)
		{
			setZoom(zoomScale);

			int dirCorrection = 0;

			bool topCheck, botCheck, leftCheck, rightCheck;

			int currentSnow = thisTile->hasSnow;
			int	topTileSnow = topTile->hasSnow;
			int	botTileSnow = botTile->hasSnow;
			int	leftTileSnow = leftTile->hasSnow;
			int	rightTileSnow = rightTile->hasSnow;

			topCheck = currentSnow == topTileSnow;
			botCheck = currentSnow == botTileSnow;
			leftCheck = currentSnow == leftTileSnow;
			rightCheck = currentSnow == rightTileSnow;

			dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

			int sprIndex = 848;
			drawSpriteCenter
			(
				spr::tileset,
				sprIndex + dirCorrection,
				cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
				cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
			);

			setZoom(1.0);
		}

		switch (thisTile->wall)//벽 그리기
		{
		case 0:
			break;
		default:

			setZoom(zoomScale);
			int dirCorrection = 0;
			if (itemDex[thisTile->wall].tileConnectGroup != -1)
			{
				bool topCheck, botCheck, leftCheck, rightCheck;
				if (itemDex[thisTile->wall].tileConnectGroup == 0)
				{
					int currentTileWall = thisTile->wall;
					int	topTileWall = topTile->wall;
					int	botTileWall = botTile->wall;
					int	leftTileWall = leftTile->wall;
					int	rightTileWall = rightTile->wall;

					topCheck = currentTileWall == topTileWall;
					botCheck = currentTileWall == botTileWall;
					leftCheck = currentTileWall == leftTileWall;
					rightCheck = currentTileWall == rightTileWall;
				}
				else
				{
					int currentTileGroup = itemDex[thisTile->wall].tileConnectGroup;
					int	topTileGroup = itemDex[topTile->wall].tileConnectGroup;
					int	botTileGroup = itemDex[botTile->wall].tileConnectGroup;
					int	leftTileGroup = itemDex[leftTile->wall].tileConnectGroup;
					int	rightTileGroup = itemDex[rightTile->wall].tileConnectGroup;

					topCheck = (currentTileGroup == topTileGroup);
					botCheck = (currentTileGroup == botTileGroup);
					leftCheck = (currentTileGroup == leftTileGroup);
					rightCheck = (currentTileGroup == rightTileGroup);
				}

				dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);
			}



			drawSpriteCenter
			(
				spr::tileset,
				itemDex[thisTile->wall].tileSprIndex + dirCorrection,
				cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX),
				cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY)
			);
			setZoom(1.0);

			if (thisTile->displayHPBarCount > 0)//개체 HP 표기
			{
				int drawingX = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX);
				int drawingY = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY);

				TileData& t = World::ins()->getTile(tgtX, tgtY, PlayerZ());
				int pivotX = drawingX - (int)(8 * zoomScale);
				int pivotY = drawingY;
				SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				drawFillRect(dst, col::black, t.alphaHPBar);

				if (t.wallFakeHP > t.wallHP) { t.wallFakeHP -= ((float)t.wallMaxHP / 100.0); }
				else if (t.wallFakeHP < t.wallHP) t.wallFakeHP = t.wallHP;

				if (t.wallFakeHP != t.wallHP)
				{
					if (t.alphaFakeHPBar > 20) { t.alphaFakeHPBar -= 20; }
					else
					{
						t.alphaFakeHPBar = 0;
						t.wallFakeHP = t.wallHP;
					}
				}
				else { t.alphaFakeHPBar = 0; }

				float ratioFakeHP = myMax((float)0.0, (t.wallFakeHP) / (float)(t.wallMaxHP));
				dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				drawFillRect(dst, col::white, t.alphaFakeHPBar);

				float ratioHP = myMax((float)0.0, (float)(t.wallHP) / (float)(t.wallMaxHP));
				dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
				if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				drawFillRect(dst, lowCol::red, t.alphaHPBar);

				if (t.displayHPBarCount > 1) t.displayHPBarCount--;
				else if (t.displayHPBarCount == 1)
				{
					t.alphaHPBar -= 10;
					if (t.alphaHPBar <= 0)
					{
						t.alphaHPBar = 0;
						t.displayHPBarCount = 0;
					}
				}
			}
		}



	}





	return getNanoTimer() - timeStampStart;
}

__int64 drawCorpses()
{
	__int64 timeStampStart = getNanoTimer();

	for (const auto& elem : Corpse::list)
	{
		Corpse* adr = elem;
		if (pZ == adr->getZ())//플레이어의 z축과 시체의 z축이 같을 경우
		{
			setZoom(zoomScale);
			drawSpriteCenter(adr->getSprite(), adr->getSprIndex(), (cameraW / 2) + zoomScale * (adr->getX() - cameraX), (cameraH / 2) + zoomScale * (adr->getY() - cameraY));
			setZoom(1.0);
		}
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawItems()
{
	__int64 timeStampStart = getNanoTimer();

	for (const auto& elem : itemList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;

		ItemStack* address = TileItemStack(tgtX, tgtY, pZ);

		int itemSprIndex = 0;
		if (address->getTargetSprIndex() != 0) itemSprIndex = address->getTargetSprIndex();
		else itemSprIndex = address->getPocket()->itemInfo[0].sprIndex;

		setZoom(zoomScale);
		drawSpriteCenter
		(
			spr::itemset,
			itemSprIndex,
			(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
		);
		setZoom(1.0);


	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawEntities()
{
	__int64 timeStampStart = getNanoTimer();

	//바닥 설치물 그리기
	for (const auto& elem : floorPropList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;
		Prop* iPtr = TileProp(tgtX, tgtY, PlayerZ());
		int bigShift = 16 * (iPtr->leadItem.checkFlag(itemFlag::PROP_BIG));
		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8 - bigShift) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
		SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		int sprIndex = iPtr->leadItem.propSprIndex + iPtr->leadItem.extraSprIndexSingle + 16 * iPtr->leadItem.extraSprIndex16;
		if (iPtr->leadItem.checkFlag(itemFlag::PLANT_SEASON_DEPENDENT))
		{
			if (World::ins()->getTile(tgtX, tgtY, PlayerZ()).hasSnow == true) sprIndex += 5;
			else
			{
				if (getSeason() == seasonFlag::summer) { sprIndex += 1; }
				else if (getSeason() == seasonFlag::autumn) { sprIndex += 2; }
				else if (getSeason() == seasonFlag::winter) { sprIndex += 3; }
			}


		}



		drawSpriteCenter
		(
			spr::propset,
			sprIndex,
			dst.x + dst.w / 2 + zoomScale * iPtr->getIntegerFakeX(),
			dst.y + dst.h / 2 + zoomScale * iPtr->getIntegerFakeY()
		);
		SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
		setZoom(1.0);
	}

	//화염 그리기
	for (const auto& elem : flameList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;
		Flame* tgtFlame = (Flame*)(&World::ins()->getTile(tgtX, tgtY, PlayerZ()))->flamePtr;

		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
		SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		int sprIndex = 0;
		int animeVal = timer::timer600 % 30;
		if (animeVal < 6) sprIndex += 0;
		else if (animeVal < 12) sprIndex += 1;
		else if (animeVal < 18) sprIndex += 2;
		else if (animeVal < 24) sprIndex += 3;
		else sprIndex += 4;
		sprIndex += tgtFlame->sprRandomStart;
		sprIndex = sprIndex % 5;

		drawSpriteCenter
		(
			spr::flameSet,
			tgtFlame->sprInfimum + sprIndex,
			dst.x + dst.w / 2 + zoomScale,
			dst.y + dst.h / 2 + zoomScale
		);
		SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
		setZoom(1.0);
	}


	std::vector<std::array<int, 2>> rotorList;
	//차량그리기
	std::unordered_set<Drawable*> vehCache;
	for (const auto& elem : renderVehList)
	{
		if (vehCache.find(elem) == vehCache.end())
		{
			elem->drawSelf();
			vehCache.insert(elem);
		}
	}
	//엔티티&일반설치물 그리기
	std::unordered_set<Drawable*> entityCache;
	for (const auto& elem : renderEntityList)
	{
		if (entityCache.find(elem) == entityCache.end())
		{
			elem->drawSelf();
			entityCache.insert(elem);
		}
	}

	//헬기 로터 그리기
	for (int i = 0; i < rotorList.size(); i++)
	{
		int tgtX = rotorList[i][0];
		int tgtY = rotorList[i][1];
		Vehicle* vPtr = TileVehicle(tgtX, tgtY, PlayerZ());

		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == vPtr)
		{
			SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 50); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::mainRotor->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		}
		drawSpriteCenter
		(
			spr::mainRotor,
			0,
			dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
			dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
		);
		SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 255); //텍스쳐 투명도 설정

		setZoom(1.0);
	}

	//조종 중인 차량의 마커 그리기
	if (ctrlVeh != nullptr)
	{
		Vehicle* vPtr = (Vehicle*)ctrlVeh;
		for (auto it = vPtr->partInfo.begin(); it != vPtr->partInfo.end(); it++)
		{
			int tgtX = it->first[0];
			int tgtY = it->first[1];

			for (int layer = 0; layer < vPtr->partInfo[{tgtX, tgtY}]->itemInfo.size(); layer++)
			{
				if (vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].checkFlag(itemFlag::TIRE_STEER))
				{
					SDL_Rect dst;
					dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
					dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
					dst.w = tileSize;
					dst.h = tileSize;

					setZoom(zoomScale);
					SDL_SetTextureAlphaMod(spr::propset->getTexture(), 150); //텍스쳐 투명도 설정
					SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
					int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;
					drawSpriteCenter
					(
						spr::propset,
						sprIndex,
						dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
					);
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 150); //텍스쳐 투명도 설정
					SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
					drawSpriteCenter
					(
						spr::dirMarker,
						128 + dir16toInt16(vPtr->wheelDir),
						dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
					);
					SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255); //텍스쳐 투명도 설정
					setZoom(1.0);
				}
				else if (vPtr->isPowerCart)
				{
					SDL_Rect dst;
					dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
					dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
					dst.w = tileSize;
					dst.h = tileSize;

					setZoom(zoomScale);
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 110); //텍스쳐 투명도 설정
					SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
					drawSpriteCenter
					(
						spr::dirMarker,
						224 + dir16toInt16(vPtr->bodyDir),
						dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
					);
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255); //텍스쳐 투명도 설정
					setZoom(1.0);
				}
			}

			//플레이어 속도 표현
			if (tgtX == PlayerX() && tgtY == PlayerY())
			{
				if (vPtr->spdVec.isZeroVec() == false)
				{
					SDL_Rect dst;
					dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
					dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
					dst.w = tileSize;
					dst.h = tileSize;

					setZoom(zoomScale);
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 180); //텍스쳐 투명도 설정
					SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
					int spdExtraIndex = 0;
					if (vPtr->spdVec.getLength() < 5) spdExtraIndex = 0;
					else if (vPtr->spdVec.getLength() < 10) spdExtraIndex = 1;
					else if (vPtr->spdVec.getLength() < 15) spdExtraIndex = 2;
					else spdExtraIndex = 3;
					drawSpriteCenter
					(
						spr::dirMarker,
						160 + getNearDir16(vPtr->spdVec) + 16 * spdExtraIndex,
						dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
					);
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255); //텍스쳐 투명도 설정
					setZoom(1.0);
				}
			}
		}
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawDamages()
{
	__int64 timeStampStart = getNanoTimer();

	for (int i = 0; i < Damage::list.size(); i++)
	{
		Damage* address = Damage::list[i];
		setFontSize(10 * zoomScale);

		int drawingX = (cameraW / 2) + zoomScale * (address->getX() - cameraX);
		int drawingY = (cameraH / 2) + zoomScale * (address->getY() - cameraY);
		setZoom(zoomScale);
		SDL_SetTextureAlphaMod(address->getSprite()->getTexture(), address->getAlpha());
		drawSpriteCenter(address->getSprite(), 0, drawingX, drawingY);
		setZoom(1.0);
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawBullets()
{
	__int64 timeStampStart = getNanoTimer();

	for (int i = 0; i < Bullet::list.size(); i++)
	{
		Bullet* address = Bullet::list[i];
		int drawingX = (cameraW / 2) + zoomScale * (address->getX() - cameraX);
		int drawingY = (cameraH / 2) + zoomScale * (address->getY() - cameraY);
		setZoom(zoomScale);
		drawSpriteCenter
		(
			address->sprite,
			address->sprIndex,
			(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
		);
		setZoom(1.0);
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawParticles()
{
	__int64 timeStampStart = getNanoTimer();

	for (int i = 0; i < Particle::list.size(); i++)
	{
		Particle* address = Particle::list[i];
		int drawingX = (cameraW / 2) + zoomScale * (address->getX() - cameraX);
		int drawingY = (cameraH / 2) + zoomScale * (address->getY() - cameraY);
		setZoom(zoomScale);
		SDL_SetTextureAlphaMod(address->sprite->getTexture(), address->alpha);
		drawSpriteCenter
		(
			address->sprite,
			address->sprIndex,
			(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
		);
		SDL_SetTextureAlphaMod(address->sprite->getTexture(), 255);
		setZoom(1.0);
	}

	return getNanoTimer() - timeStampStart;
}


__int64 drawFootprints()
{
	__int64 timeStampStart = getNanoTimer();

	for (int i = 0; i < Footprint::list.size(); i++)
	{
		Footprint* address = Footprint::list[i];
		int drawingX = (cameraW / 2) + zoomScale * (address->getX() - cameraX);
		int drawingY = (cameraH / 2) + zoomScale * (address->getY() - cameraY);
		setZoom(zoomScale);
		SDL_SetTextureAlphaMod(address->sprite->getTexture(), address->alpha);
		drawSprite
		(
			address->sprite,
			address->sprIndex,
			(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
		);
		SDL_SetTextureAlphaMod(address->sprite->getTexture(), 255);
		setZoom(1.0);
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawFogs()
{
	__int64 timeStampStart = getNanoTimer();


	for (const auto& elem : gasList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;
		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());

		for (int j = 0; j < thisTile->gasVec.size(); j++)
		{
			int alpha = 255 * (1 - std::exp(-0.03 * thisTile->gasVec[j].gasVol));
			SDL_SetTextureAlphaMod(spr::steamEffect1->getTexture(), std::min(255, alpha)); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::steamEffect1->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정

			SDL_SetTextureColorMod(spr::steamEffect1->getTexture(), itemDex[thisTile->gasVec[j].gasCode].gasColorR, itemDex[thisTile->gasVec[j].gasCode].gasColorG, itemDex[thisTile->gasVec[j].gasCode].gasColorB);
			int sprIndex = 0;//1초에 5번해야됨, 600 10
			sprIndex += thisTile->randomVal;
			if (timer::timer600 % 60 < 10) sprIndex += 0;
			else if (timer::timer600 % 60 < 20) sprIndex += 1;
			else if (timer::timer600 % 60 < 30) sprIndex += 2;
			else if (timer::timer600 % 60 < 40) sprIndex += 3;
			else if (timer::timer600 % 60 < 50) sprIndex += 4;
			else sprIndex += 5;
			sprIndex += 2 * j;
			sprIndex = sprIndex % 6;
			drawSpriteCenter
			(
				spr::steamEffect1,
				sprIndex,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
		}
		SDL_SetTextureAlphaMod(spr::steamEffect1->getTexture(), 255); //텍스쳐 투명도 설정
		setZoom(1.0);
	}

	for (const auto& elem : blackFogList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;

		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		drawFillRect(dst, { 0x16,0x16,0x16},255);
	}

	for (const auto& elem : grayFogList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;

		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, { 0x16,0x16,0x16 }, 185);
	}

	for (const auto& elem : lightFogList)
	{
		int tgtX = elem.x;
		int tgtY = elem.y;
		TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());

		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, { 0x16,0x16,0x16 }, 90);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
		Uint8 targetR = myMin(255, thisTile->redLight);
		Uint8 targetG = myMin(255, thisTile->greenLight);
		Uint8 targetB = myMin(255, thisTile->blueLight);
		drawFillRect(dst, { targetR,targetG,targetB }, 200);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawMarkers()
{
	__int64 timeStampStart = getNanoTimer();

	auto drawEplsionText = [](auto* spr, int spriteIndex, int x, int y) {
		SDL_SetTextureBlendMode(spr->getTexture(), SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(spr->getTexture(), 78, 115, 70);

		drawSprite(spr, spriteIndex, x + zoomScale, y);
		drawSprite(spr, spriteIndex, x, y - zoomScale);
		drawSprite(spr, spriteIndex, x - zoomScale, y);
		drawSprite(spr, spriteIndex, x, y + zoomScale);

		SDL_SetTextureColorMod(spr->getTexture(), 138, 175, 130);
		drawSprite(spr, spriteIndex, x, y);
		SDL_SetTextureColorMod(spr->getTexture(), 255, 255, 255);
		};



	if (option::inputMethod == input::mouse)
	{
		if (isPlayerMoving == false && turnCycle == turn::playerInput)
		{
			if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false && checkCursor(&quickSlotRegion) == false)
			{
				if (GUI::getLastGUI() == HUD::ins())
				{
					int tgtX = getAbsMouseGrid().x;
					int tgtY = getAbsMouseGrid().y;
					dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
					dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
					dst.w = tileSize;
					dst.h = tileSize;
					setZoom(zoomScale);
					drawSpriteCenter
					(
						spr::cursorMarker,
						0,
						dst.x + dst.w / 2,
						dst.y + dst.h / 2
					);
					setZoom(1.0);
				}
			}
		}
	}

	for (int i = 1; i < aStarTrail.size(); i++)
	{
		int tgtX = aStarTrail[i].x;
		int tgtY = aStarTrail[i].y;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;
		setZoom(zoomScale);
		drawSpriteCenter
		(
			spr::trail,
			0,
			dst.x + dst.w / 2,
			dst.y + dst.h / 2
		);
		setZoom(1.0);
	}


	//화이트마커 그리기
	if (option::inputMethod == input::gamepad)
	{
		if (gamepadWhiteMarker.z == PlayerZ())
		{
			if (std::abs(gamepadWhiteMarker.x - PlayerX()) <= MARKER_LIMIT_DIST)
			{
				if (std::abs(gamepadWhiteMarker.y - PlayerY()) <= MARKER_LIMIT_DIST)
				{
					int tgtX = gamepadWhiteMarker.x;
					int tgtY = gamepadWhiteMarker.y;
					dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
					dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
					dst.w = tileSize;
					dst.h = tileSize;
					setZoom(zoomScale);
					drawSpriteCenter
					(
						spr::cursorMarker,
						0,
						dst.x + dst.w / 2,
						dst.y + dst.h / 2
					);
					setZoom(1.0);
				}
			}
		}
	}
	whiteMarkerEnd:

	if (Craft::ins() != nullptr && Craft::ins()->getIsNowBuilding())
	{
		std::array<int, 3> nowBuildLocation = Craft::ins()->getBuildLocation();
		if (pZ == nowBuildLocation[2])
		{
			SDL_Point pt = { nowBuildLocation[0], nowBuildLocation[1] };
			if (SDL_PointInRect(&pt, &renderRegion))
			{
				int tgtX = nowBuildLocation[0];
				int tgtY = nowBuildLocation[1];
				dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
				dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
				dst.w = tileSize;
				dst.h = tileSize;
				setZoom(zoomScale);
				drawSpriteCenter
				(
					spr::buildCursor,
					0,
					dst.x + dst.w / 2,
					dst.y + dst.h / 2
				);

				int percent = Craft::ins()->getProcessPercent();
				if (percent < 10)
				{
					drawEplsionText(spr::epsilonFont, 27 + percent % 10, dst.x + 5 * zoomScale, dst.y + 11 * zoomScale);
					drawEplsionText(spr::epsilonFont, 43, dst.x + 9 * zoomScale, dst.y + 11 * zoomScale);

				}
				else
				{
					drawEplsionText(spr::epsilonFont, 27 + percent / 10, dst.x + 3 * zoomScale, dst.y + 11 * zoomScale);
					drawEplsionText(spr::epsilonFont, 27 + percent % 10, dst.x + 7 * zoomScale, dst.y + 11 * zoomScale);
					drawEplsionText(spr::epsilonFont, 43, dst.x + 11 * zoomScale, dst.y + 11 * zoomScale);
				}
				setZoom(1.0);
			}
		}
	}
	else if (Craft::ins() == nullptr && Craft::ins()->existCraftDataStructure())
	{
		if (pZ == Craft::getBuildLocation()[2])
		{
			SDL_Point pt = { Craft::getBuildLocation()[0], Craft::getBuildLocation()[1] };
			if (SDL_PointInRect(&pt, &renderRegion))
			{
				int tgtX = Craft::getBuildLocation()[0];
				int tgtY = Craft::getBuildLocation()[1];
				dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
				dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
				dst.w = tileSize;
				dst.h = tileSize;
				setZoom(zoomScale);
				drawSpriteCenter
				(
					spr::buildCursor,
					0,
					dst.x + dst.w / 2,
					dst.y + dst.h / 2
				);


				int percent = Craft::ins()->getProcessPercentOngoingStructure();
				if (percent < 10)
				{
					drawEplsionText(spr::epsilonFont, 27 + percent % 10, dst.x + 5 * zoomScale, dst.y + 11 * zoomScale);
					drawEplsionText(spr::epsilonFont, 43, dst.x + 9 * zoomScale, dst.y + 11 * zoomScale);

				}
				else
				{
					drawEplsionText(spr::epsilonFont, 27 + percent / 10, dst.x + 3 * zoomScale, dst.y + 11 * zoomScale);
					drawEplsionText(spr::epsilonFont, 27 + percent % 10, dst.x + 7 * zoomScale, dst.y + 11 * zoomScale);
					drawEplsionText(spr::epsilonFont, 43, dst.x + 11 * zoomScale, dst.y + 11 * zoomScale);
				}
				setZoom(1.0);
			}
		}
	}




	return getNanoTimer() - timeStampStart;
}

__int64 drawDebug()
{
	__int64 timeStampStart = getNanoTimer();
	if (debug::chunkLineDraw)
	{
		for (const auto& elem : tileList)
		{
			int tgtX = elem.x;
			int tgtY = elem.y;

			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = tileSize;
			dst.h = tileSize;

			if (tgtX % 13 == 0) drawLine(dst.x, dst.y, dst.x, dst.y + dst.h, col::red);
			else if (tgtX % 13 == 12)  drawLine(dst.x + dst.w, dst.y, dst.x + dst.w, dst.y + dst.h, col::red);

			if (tgtY % 13 == 0) drawLine(dst.x, dst.y, dst.x + dst.w, dst.y, col::red);
			else if (tgtY % 13 == 12)  drawLine(dst.x, dst.y + dst.h, dst.x + dst.w, dst.y + dst.h, col::red);

			if (std::abs(tgtX % 13) == 6 && std::abs(tgtY % 13) == 6)
			{
				setFontSize(20);


				drawTextCenter(col2Str(col::red) + L"CHUNK", dst.x + dst.w / 2, dst.y + dst.h / 2-12);

				int cx, cy;
				World::ins()->changeToChunkCoord(tgtX, tgtY, cx, cy);
				std::wstring chunkName = L"";
				chunkName += std::to_wstring(cx);
				chunkName += L",";
				chunkName += std::to_wstring(cy);
				chunkName += L",";
				chunkName += std::to_wstring(PlayerZ());
				drawTextCenter(col2Str(col::red) + chunkName, dst.x + dst.w/2, dst.y + dst.h/2+12);
			}

		}
	}
	return getNanoTimer() - timeStampStart;
}