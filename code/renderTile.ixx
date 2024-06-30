#include <SDL.h>

export module renderTile;

import std;
import util;
import globalVar;
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

SDL_Rect dst, renderRegion;
int tileSize, cameraGridX, cameraGridY, renderRangeW, renderRangeH, playerZ, markerIndex;

__int64 analyseRender();
__int64 drawTiles();
__int64 drawCorpses();
__int64 drawItems();
__int64 drawEntities();
__int64 drawDamages();
__int64 drawFogs();
__int64 drawMarkers();

template <typename T>
class uniqueVector
{
private:
	std::vector<T> vec;
	std::unordered_set<T> set;

public:
	bool insert(const T& value)
	{
		if (set.find(value) == set.end())
		{
			vec.push_back(value);
			set.insert(value);
			return true;
		}
		return false;
	}

	bool erase(const T& value)
	{
		auto it = set.find(value);
		if (it != set.end())
		{
			set.erase(it);
			vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
			return true;
		}
		return false;
	}

	void reserve(int size)
	{
		vec.reserve(size);
	}

	void clear()
	{
		vec.clear();
		set.clear();
	}

	size_t size() const { return vec.size(); }
	bool empty() const { return vec.empty(); }
	const T& operator[](size_t index) const { return vec[index]; }
	typename std::vector<T>::iterator begin() { return vec.begin(); }
	typename std::vector<T>::iterator end() { return vec.end(); }
	typename std::vector<T>::const_iterator begin() const { return vec.begin(); }
	typename std::vector<T>::const_iterator end() const { return vec.end(); }
};


std::unordered_map<std::array<int, 2>, TileData*, decltype(arrayHasher2)> tileCache;
std::vector<Point2> tileList;
std::vector<Point2> itemList;
std::vector<Point2> floorPropList;
std::vector<Flame*> flameList;
uniqueVector<Drawable*> renderVehList;
uniqueVector<Drawable*> renderEntityList;
std::vector<Point2> gasList;
std::vector<Point2> blackFogList;
std::vector<Point2> grayFogList;
std::vector<Point2> lightFogList;

export __int64 renderTile()
{
	__int64 timeStampStart = getNanoTimer();

	tileSize = 16 * zoomScale;
	cameraGridX = (cameraX - 8) / (16);
	cameraGridY = (cameraY - 8) / (16);
	renderRangeW = 3 + (cameraW + extraCameraLength) / tileSize;
	renderRangeH = 3 + (cameraH + extraCameraLength) / tileSize;
	playerZ = Player::ins()->getGridZ();
	renderRegion = { cameraGridX - (renderRangeW / 2), cameraGridY - (renderRangeH / 2), renderRangeW, renderRangeH };

	if (timer::timer600 % 30 < 5) markerIndex = 0;
	else if (timer::timer600 % 30 < 10) markerIndex = 1;
	else if (timer::timer600 % 30 < 15) markerIndex = 2;
	else if (timer::timer600 % 30 < 20) markerIndex = 3;
	else if (timer::timer600 % 30 < 25) markerIndex = 4;
	else markerIndex = 0;

	tileList.clear();
	itemList.clear();
	floorPropList.clear();
	renderVehList.clear();
	renderEntityList.clear();
	gasList.clear();
	blackFogList.clear();
	grayFogList.clear();
	lightFogList.clear();
	tileCache.clear();
	flameList.clear();

	//tileList.reserve(10000);
	//itemList.reserve(10000);
	//floorPropList.reserve(10000);
	//renderVehList.reserve(10000);
	//renderEntityList.reserve(10000);
	//blackFogList.reserve(10000);
	//grayFogList.reserve(10000);
	//lightFogList.reserve(10000);

	dur::analysis = analyseRender();
	dur::tile = drawTiles();
	dur::corpse = drawCorpses();
	dur::item = drawItems();
	dur::entity = drawEntities();
	dur::damage = drawDamages();
	dur::fog = drawFogs();
	dur::marker = drawMarkers();

	//전체광
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_Rect screenRect = { 0,0,cameraW,cameraH };
	SDL_SetRenderDrawColor(renderer, mainLightColor.r, mainLightColor.g, mainLightColor.b, mainLightBright);
	SDL_RenderFillRect(renderer, &screenRect);
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
			TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, playerZ);
			tileCache[{tgtX, tgtY}] = thisTile;

			//바깥쪽이면 타일캐시만 추가하고 거기에 있는 내용은 렌더링하지 않음
			if (tgtX < renderRegion.x || tgtX >= renderRegion.x + renderRegion.w) continue;
			if (tgtY < renderRegion.y || tgtY >= renderRegion.y + renderRegion.h) continue;

			//바닥과 벽
			if (thisTile->fov != fovFlag::black) tileList.push_back({ tgtX,tgtY });

			//아이템
			if (thisTile->fov == fovFlag::white)
			{
				if ((World::ins())->getItemPos(tgtX, tgtY, playerZ) != nullptr) itemList.push_back({ tgtX,tgtY });
			}
			//바닥프롭
			Prop* fpPtr = (Prop*)thisTile->PropPtr;
			if (fpPtr != nullptr && fpPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER)) floorPropList.push_back({ tgtX,tgtY });

			//화염
			Flame* flamePtr = (Flame*)thisTile->flamePtr;
			if (flamePtr != nullptr ) flameList.push_back(flamePtr);
			
			//차량
			Drawable* vPtr = (Drawable*)((Vehicle*)(thisTile->VehiclePtr));
			if (vPtr != nullptr) renderVehList.insert(vPtr);
			for (auto it = extraRenderVehList.begin(); it != extraRenderVehList.end(); it++) renderVehList.insert((Drawable*)(Vehicle*)(*it));

			//플레이어와 겹치는 일반설치물
			Prop* pPtr = (Prop*)(thisTile->PropPtr);
			if (pPtr != nullptr && pPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER) == false) renderEntityList.insert((Drawable*)pPtr);

			//일반 객체
			Drawable* ePtr = (Drawable*)((Entity*)(thisTile->EntityPtr));
			if (ePtr != nullptr) renderEntityList.insert(ePtr);
			//추가로 렌더링되는 객체(화면밖)
			for (auto it = extraRenderEntityList.begin(); it != extraRenderEntityList.end(); it++) renderEntityList.insert((Drawable*)(Entity*)(*it));

			//가스
			if (thisTile->gasVec.size() > 0)
			{
				gasList.push_back({ tgtX,tgtY });
			}

			//안개
			if (thisTile->fov == fovFlag::black) blackFogList.push_back({ tgtX, tgtY });
			else if (thisTile->fov == fovFlag::gray) grayFogList.push_back({ tgtX, tgtY });
			else  lightFogList.push_back({ tgtX, tgtY });
		}
	}

	return getNanoTimer() - timeStampStart;
}


__int64 drawTiles()
{
	__int64 timeStampStart = getNanoTimer();

	bool targetMarker = false;
	for (int i = 0; i < tileList.size(); i++)
	{
		int tgtX = tileList[i].x;
		int tgtY = tileList[i].y;
		const TileData* thisTile = tileCache[{tgtX, tgtY}];
		const TileData* topTile = tileCache[{tgtX, tgtY - 1}];
		const TileData* botTile = tileCache[{tgtX, tgtY + 1}];
		const TileData* leftTile = tileCache[{tgtX - 1, tgtY}];
		const TileData* rightTile = tileCache[{tgtX + 1, tgtY}];

		switch (thisTile->floor)//바닥 타일 그리기 
		{
		default:
			setZoom(zoomScale);

			int dirCorrection = 0;

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

					if (timer::timer600 % animeFPS == 0)
					{
						itemDex[thisTile->floor].extraSprIndex16++;
						if (itemDex[thisTile->floor].extraSprIndex16 >= animeSize)
						{
							itemDex[thisTile->floor].extraSprIndex16 = 0;
						}
					}
				}
			}
			else
			{
				if (itemDex[thisTile->floor].animeSize > 1)
				{
					int animeFPS = itemDex[thisTile->floor].animeFPS;
					int animeSize = itemDex[thisTile->floor].animeSize;

					if (timer::timer600 % animeFPS == 0)
					{
						itemDex[thisTile->floor].extraSprIndexSingle++;
						if (itemDex[thisTile->floor].extraSprIndexSingle >= animeSize)
						{
							itemDex[thisTile->floor].extraSprIndexSingle = 0;
						}
					}
				}
			}

			int sprIndex = itemDex[thisTile->floor].tileSprIndex + itemDex[thisTile->floor].extraSprIndexSingle + 16 * itemDex[thisTile->floor].extraSprIndex16;
			if (thisTile->floor == 0) sprIndex = 506;

			drawSpriteCenter
			(
				spr::tileset,
				sprIndex + dirCorrection,
				cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
				cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
			);

			setZoom(1.0);

			//마우스가 가리키는 타일에 마커 그리기
			if (targetMarker == false && GUI::getActiveGUIList().size() == 1)
			{
				SDL_Rect tileRect = { cameraW / 2 + zoomScale * ((16 * tgtX) - cameraX), cameraH / 2 + zoomScale * ((16 * tgtY) - cameraY), 16 * zoomScale, 16 * zoomScale };
				if (UIType == act::null)
				{
					SDL_Rect bottomBox = { 0,0,630, 140 };
					bottomBox.x = (cameraW - bottomBox.w) / 2;
					bottomBox.y = cameraH - bottomBox.h + 6;
					SDL_Rect tabBox = { cameraW - 120 - 20, 20, 120, 120 };

					if (checkCursor(&tileRect) == true)
					{
						if (checkCursor(&bottomBox) == false && checkCursor(&tabBox) == false && checkCursor(&quickSlotRegion) == false)
						{
							setZoom(zoomScale);
							drawSpriteCenter
							(
								spr::cursorMarker,
								markerIndex,
								cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX),
								cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY)
							);
							setZoom(1.0);
							targetMarker = true;
						}
					}
				}
			}
			break;
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

					topCheck = currentTileGroup == topTileGroup;
					botCheck = currentTileGroup == botTileGroup;
					leftCheck = currentTileGroup == leftTileGroup;
					rightCheck = currentTileGroup == rightTileGroup;
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
		}
	}
	return getNanoTimer() - timeStampStart;
}


__int64 drawCorpses()
{
	__int64 timeStampStart = getNanoTimer();

	for (int i = 0; i < Corpse::list.size(); i++)
	{
		Corpse* adr = Corpse::list[i];
		if (playerZ == adr->getZ())//플레이어의 z축과 시체의 z축이 같을 경우
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

	for (int i = 0; i < itemList.size(); i++)
	{
		int tgtX = itemList[i].x;
		int tgtY = itemList[i].y;
		ItemStack* address = (World::ins())->getItemPos(tgtX, tgtY, playerZ);
		if (address->getAniType() == aniFlag::throwing)
		{
			setZoom(zoomScale);
			drawSpriteCenter
			(
				address->getSprite(),
				address->getTargetSprIndex(),
				(cameraW / 2) + zoomScale * (address->getX() - cameraX),
				(cameraH / 2) + zoomScale * (address->getY() - cameraY)
			);
			setZoom(1.0);
		}

		setZoom(zoomScale);
		drawSpriteCenter
		(
			address->getSprite(),
			address->getSprIndex(),
			(cameraW / 2) + zoomScale * (address->getX() - cameraX + address->getFakeX()),
			(cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getFakeY())
		);
		setZoom(1.0);
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawEntities()
{
	__int64 timeStampStart = getNanoTimer();

	//바닥 설치물 그리기
	for (int i = 0; i < floorPropList.size(); i++)
	{
		int tgtX = floorPropList[i].x;
		int tgtY = floorPropList[i].y;
		Prop* iPtr = (Prop*)tileCache[{tgtX, tgtY}]->PropPtr;
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
			if (getSeason() == seasonFlag::summer) { sprIndex += 1; }
			else if (getSeason() == seasonFlag::autumn) { sprIndex += 2; }
			else if (getSeason() == seasonFlag::winter) { sprIndex += 3; }
		}
		drawSpriteCenter
		(
			spr::propset,
			sprIndex,
			dst.x + dst.w / 2 + zoomScale * iPtr->getFakeX(),
			dst.y + dst.h / 2 + zoomScale * iPtr->getFakeY()
		);
		SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
		setZoom(1.0);
	}

	//화염 그리기
	for (int i = 0; i < flameList.size(); i++)
	{
		Flame* tgtFlame = flameList[i];
		int tgtX = tgtFlame->gridX;
		int tgtY = tgtFlame->gridY;

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
	for (int i = 0; i < renderVehList.size(); i++) renderVehList[i]->drawSelf();

	//엔티티&일반설치물 그리기
	for (int i = 0; i < renderEntityList.size(); i++) renderEntityList[i]->drawSelf();

	//헬기 로터 그리기
	for (int i = 0; i < rotorList.size(); i++)
	{
		int tgtX = rotorList[i][0];
		int tgtY = rotorList[i][1];
		Vehicle* vPtr = (Vehicle*)tileCache[{tgtX, tgtY}]->VehiclePtr;

		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		if (tileCache[{Player::ins()->getGridX(), Player::ins()->getGridY()}]->VehiclePtr == vPtr)
		{
			SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 50); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::mainRotor->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		}
		drawSpriteCenter
		(
			spr::mainRotor,
			0,
			dst.x + dst.w / 2 + zoomScale * vPtr->getFakeX(),
			dst.y + dst.h / 2 + zoomScale * vPtr->getFakeY()
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
						dst.x + dst.w / 2 + zoomScale * vPtr->getFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getFakeY()
					);
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 150); //텍스쳐 투명도 설정
					SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
					drawSpriteCenter
					(
						spr::dirMarker,
						128 + dir16toInt16(vPtr->wheelDir),
						dst.x + dst.w / 2 + zoomScale * vPtr->getFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getFakeY()
					);
					SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
					SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255); //텍스쳐 투명도 설정
					setZoom(1.0);
				}
			}

			//플레이어 속도 표현
			if (tgtX == Player::ins()->getGridX() && tgtY == Player::ins()->getGridY())
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
						dst.x + dst.w / 2 + zoomScale * vPtr->getFakeX(),
						dst.y + dst.h / 2 + zoomScale * vPtr->getFakeY()
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

__int64 drawFogs()
{
	__int64 timeStampStart = getNanoTimer();


	for (int i = 0; i < gasList.size(); i++)
	{
		int tgtX = gasList[i].x;
		int tgtY = gasList[i].y;
		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		TileData* thisTile = tileCache[{tgtX, tgtY}];
		
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
			sprIndex += 2*j;
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

	for (int i = 0; i < blackFogList.size(); i++)
	{
		int tgtX = blackFogList[i].x;
		int tgtY = blackFogList[i].y;

		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;
		SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 255);
		SDL_RenderFillRect(renderer, &dst);
	}

	for (int i = 0; i < grayFogList.size(); i++)
	{
		int tgtX = grayFogList[i].x;
		int tgtY = grayFogList[i].y;

		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 185);
		SDL_RenderFillRect(renderer, &dst);
	}

	for (int i = 0; i < lightFogList.size(); i++)
	{
		int tgtX = lightFogList[i].x;
		int tgtY = lightFogList[i].y;
		TileData* thisTile = tileCache[{tgtX, tgtY}];

		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 90);
		SDL_RenderFillRect(renderer, &dst);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
		Uint8 targetR = myMin(255, thisTile->redLight);
		Uint8 targetG = myMin(255, thisTile->greenLight);
		Uint8 targetB = myMin(255, thisTile->blueLight);
		SDL_SetRenderDrawColor(renderer, targetR, targetG, targetB, 200);
		SDL_RenderFillRect(renderer, &dst);
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

	if (Craft::ins() != nullptr && Craft::ins()->getIsNowBuilding())
	{
		std::array<int, 3> nowBuildLocation = Craft::ins()->getBuildLocation();
		if (playerZ == nowBuildLocation[2])
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
		if (playerZ == Craft::getBuildLocation()[2])
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