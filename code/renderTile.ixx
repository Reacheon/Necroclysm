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
import Install;
import globalTime;

SDL_Rect dst, renderRegion;
int tileSize, cameraGridX, cameraGridY, renderRangeW, renderRangeH, playerZ, markerIndex;

__int64 drawTiles();
__int64 drawCorpses();
__int64 drawItems();
__int64 drawEntities();
__int64 drawDamages();
__int64 drawFogs();
__int64 drawMarkers();

export __int64 renderTile()
{
	__int64 timeStampStart = getNanoTimer();

	tileSize = 16 * zoomScale;
	cameraGridX = (cameraX - 8) / (16);
	cameraGridY = (cameraY - 8) / (16);
	renderRangeW = 2 + (cameraW + extraCameraLength) / tileSize;
	renderRangeH = 2 + (cameraH + extraCameraLength) / tileSize;
	playerZ = Player::ins()->getGridZ();
	renderRegion = { cameraGridX - (renderRangeW / 2), cameraGridY - (renderRangeH / 2), renderRangeW, renderRangeH };

	if (timer::timer600 % 30 < 5) markerIndex = 0;
	else if (timer::timer600 % 30 < 10) markerIndex = 1;
	else if (timer::timer600 % 30 < 15) markerIndex = 2;
	else if (timer::timer600 % 30 < 20) markerIndex = 3;
	else if (timer::timer600 % 30 < 25) markerIndex = 4;
	else markerIndex = 0;

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


__int64 drawTiles()
{
	__int64 timeStampStart = getNanoTimer();

	bool targetMarker = false;

	for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
	{
		for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
		{
			if (World::ins()->getTile(tgtX, tgtY, playerZ).fov != fovFlag::black)
			{
				switch (World::ins()->getTile(tgtX, tgtY, playerZ).floor)//바닥 타일 그리기 
				{
				default:
					setZoom(zoomScale);

					int dirCorrection = 0;

					if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].tileConnectGroup != -1)
					{
						bool topCheck, botCheck, leftCheck, rightCheck;
						if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].tileConnectGroup == 0)
						{
							int currentTile = World::ins()->getTile(tgtX, tgtY, playerZ).floor;
							int	topTile = World::ins()->getTile(tgtX, tgtY - 1, playerZ).floor;
							int	botTile = World::ins()->getTile(tgtX, tgtY + 1, playerZ).floor;
							int	leftTile = World::ins()->getTile(tgtX - 1, tgtY, playerZ).floor;
							int	rightTile = World::ins()->getTile(tgtX + 1, tgtY, playerZ).floor;

							topCheck = currentTile == topTile;
							botCheck = currentTile == botTile;
							leftCheck = currentTile == leftTile;
							rightCheck = currentTile == rightTile;
						}
						else
						{
							int currentTile = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].tileConnectGroup;
							int	topTile = itemDex[World::ins()->getTile(tgtX, tgtY - 1, playerZ).floor].tileConnectGroup;
							int	botTile = itemDex[World::ins()->getTile(tgtX, tgtY + 1, playerZ).floor].tileConnectGroup;
							int	leftTile = itemDex[World::ins()->getTile(tgtX - 1, tgtY, playerZ).floor].tileConnectGroup;
							int	rightTile = itemDex[World::ins()->getTile(tgtX + 1, tgtY, playerZ).floor].tileConnectGroup;

							topCheck = currentTile == topTile;
							botCheck = currentTile == botTile;
							leftCheck = currentTile == leftTile;
							rightCheck = currentTile == rightTile;
						}

						dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

						if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].animeSize > 1)
						{
							int animeFPS = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].animeFPS;
							int animeSize = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].animeSize;

							if (timer::timer600 % animeFPS == 0)
							{
								itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndex16++;
								if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndex16 >= animeSize)
								{
									itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndex16 = 0;
								}
							}
						}
					}
					else
					{
						if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].animeSize > 1)
						{
							int animeFPS = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].animeFPS;
							int animeSize = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].animeSize;

							if (timer::timer600 % animeFPS == 0)
							{
								itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndexSingle++;
								if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndexSingle >= animeSize)
								{
									itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndexSingle = 0;
								}
							}
						}
					}

					int sprIndex = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].tileSprIndex + itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndexSingle + 16 * itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).floor].extraSprIndex16;
					if (World::ins()->getTile(tgtX, tgtY, playerZ).floor == 0) sprIndex = 506;

					drawSpriteCenter
					(
						spr::tileset,
						sprIndex + dirCorrection,
						cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX),
						cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY)
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
								if (checkCursor(&bottomBox) == false)
								{
									if (checkCursor(&tabBox) == false)
									{
										setZoom(zoomScale);
										drawSpriteCenter
										(
											spr::blueMarker,
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
					}
					break;
				}

				switch (World::ins()->getTile(tgtX, tgtY, playerZ).wall)//벽 그리기
				{
				case 0:
					break;
				default:
					setZoom(zoomScale);
					int dirCorrection = 0;
					if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).wall].tileConnectGroup != -1)
					{
						bool topCheck, botCheck, leftCheck, rightCheck;
						if (itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).wall].tileConnectGroup == 0)
						{
							int currentTile = World::ins()->getTile(tgtX, tgtY, playerZ).wall;
							int	topTile = World::ins()->getTile(tgtX, tgtY - 1, playerZ).wall;
							int	botTile = World::ins()->getTile(tgtX, tgtY + 1, playerZ).wall;
							int	leftTile = World::ins()->getTile(tgtX - 1, tgtY, playerZ).wall;
							int	rightTile = World::ins()->getTile(tgtX + 1, tgtY, playerZ).wall;

							topCheck = currentTile == topTile;
							botCheck = currentTile == botTile;
							leftCheck = currentTile == leftTile;
							rightCheck = currentTile == rightTile;
						}
						else
						{
							int currentTile = itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).wall].tileConnectGroup;
							int	topTile = itemDex[World::ins()->getTile(tgtX, tgtY - 1, playerZ).wall].tileConnectGroup;
							int	botTile = itemDex[World::ins()->getTile(tgtX, tgtY + 1, playerZ).wall].tileConnectGroup;
							int	leftTile = itemDex[World::ins()->getTile(tgtX - 1, tgtY, playerZ).wall].tileConnectGroup;
							int	rightTile = itemDex[World::ins()->getTile(tgtX + 1, tgtY, playerZ).wall].tileConnectGroup;

							topCheck = currentTile == topTile;
							botCheck = currentTile == botTile;
							leftCheck = currentTile == leftTile;
							rightCheck = currentTile == rightTile;
						}

						dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);
					}


					drawSpriteCenter
					(
						spr::tileset,
						itemDex[World::ins()->getTile(tgtX, tgtY, playerZ).wall].tileSprIndex + dirCorrection,
						cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX),
						cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY)
					);
					setZoom(1.0);
				}
			}
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

	for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
	{
		for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
		{
			if (World::ins()->getTile(tgtX, tgtY, playerZ).fov == fovFlag::white)
			{
				if ((World::ins())->getItemPos(tgtX, tgtY, playerZ) != nullptr)
				{
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
			}
		}
	}

	return getNanoTimer() - timeStampStart;
}

__int64 drawEntities()
{
	__int64 timeStampStart = getNanoTimer();

	auto drawVehicleComponent = [=](Vehicle* vPtr, int tgtX, int tgtY, int layer, int alpha)
		{
			SDL_Rect dst;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = tileSize;
			dst.h = tileSize;

			setZoom(zoomScale);
			SDL_SetTextureAlphaMod(spr::propset->getTexture(), alpha); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;
			drawSpriteCenter
			(
				spr::propset,
				sprIndex,
				dst.x + dst.w / 2 + zoomScale * vPtr->getFakeX(),
				dst.y + dst.h / 2 + zoomScale * vPtr->getFakeY()
			);
			SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
			setZoom(1.0);
		};


	//바닥 설치물 그리기
	for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
	{
		for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
		{
			Install* iPtr = (Install*)World::ins()->getTile(tgtX, tgtY, Player::ins()->getGridZ()).InstallPtr;
			if (iPtr != nullptr)
			{
				if (iPtr->leadItem.checkFlag(itemFlag::INSTALL_DEPTH_LOWER))
				{
					int bigShift = 16 * (iPtr->leadItem.checkFlag(itemFlag::INSTALL_BIG));
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
			}
		}
	}

	std::vector<std::array<int, 2>> rotorList;
	
	//차량 그리기
	{
		//화면에 그려질 renderVehList 작성
		std::vector<Drawable*> renderVehList;
		for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
		{
			for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
			{
				Drawable* vPtr = (Drawable*)((Vehicle*)(World::ins()->getTile(tgtX, tgtY, Player::ins()->getGridZ()).VehiclePtr));
				if (vPtr != nullptr)
				{
					if (std::find(renderVehList.begin(), renderVehList.end(), vPtr) == renderVehList.end()) //만약 이 차량 포인터가 존재하지 않으면
					{
						renderVehList.push_back(vPtr);
					}
				}
			}
		}

		for (auto it = extraRenderVehList.begin(); it != extraRenderVehList.end(); it++)
		{
			if (std::find(renderVehList.begin(), renderVehList.end(), (*it)) == renderVehList.end()) //만약 이 차량 포인터가 존재하지 않으면
			{
				renderVehList.push_back((Drawable*)(Vehicle*)(*it));
			}
		}
		
		for (int i = 0; i < renderVehList.size(); i++) renderVehList[i]->drawSelf();
	}


	///////////////////////////////////▼엔티티 그리기//////////////////////////////////////////////////////////////////

	//엔티티&일반설치물 그리기
	{
		//화면에 그려질 renderVehList 작성
		std::vector<Drawable*> renderEntityList;
		for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
		{
			for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
			{
				//플레이어와 겹치는 설치물
				Install* iPtr = (Install*)(World::ins()->getTile(tgtX, tgtY, Player::ins()->getGridZ()).InstallPtr);
				if (iPtr != nullptr)
				{
					if (iPtr->leadItem.checkFlag(itemFlag::INSTALL_DEPTH_LOWER) == false)
					{
						if (std::find(renderEntityList.begin(), renderEntityList.end(), iPtr) == renderEntityList.end()) //만약 이 설치물 포인터가 존재하지 않으면
						{
							renderEntityList.push_back((Drawable*)iPtr);
						}
					}
				}

				//일반 객체
				Drawable* ePtr = (Drawable*)((Entity*)(World::ins()->getTile(tgtX, tgtY, Player::ins()->getGridZ()).EntityPtr));
				if (ePtr != nullptr)
				{
					if (std::find(renderEntityList.begin(), renderEntityList.end(), ePtr) == renderEntityList.end()) //만약 이 차량 포인터가 존재하지 않으면
					{
						renderEntityList.push_back(ePtr);
					}
				}


			}
		}

		for (auto it = extraRenderEntityList.begin(); it != extraRenderEntityList.end(); it++) //추가로 렌더링되는 객체(화면밖)
		{
			if (std::find(renderEntityList.begin(), renderEntityList.end(), (*it)) == renderEntityList.end()) //만약 이 차량 포인터가 존재하지 않으면
			{
				renderEntityList.push_back((Drawable*)(Entity*)(*it));
			}
		}

		for (int i = 0; i < renderEntityList.size(); i++)
		{
			renderEntityList[i]->drawSelf();
		}
	}


	//헬기 로터 그리기
	for (int i = 0; i < rotorList.size(); i++)
	{
		int tgtX = rotorList[i][0];
		int tgtY = rotorList[i][1];
		Vehicle* vPtr = (Vehicle*)World::ins()->getTile(tgtX, tgtY, Player::ins()->getGridZ()).VehiclePtr;


		SDL_Rect dst;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;

		setZoom(zoomScale);
		if (World::ins()->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).VehiclePtr == vPtr)
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

	for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
	{
		for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
		{
			if (World::ins()->getTile(tgtX, tgtY, playerZ).fov == fovFlag::black)
			{
				dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
				dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
				dst.w = tileSize;
				dst.h = tileSize;
				SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 255);
				SDL_RenderFillRect(renderer, &dst);
			}
		}
	}

	for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
	{
		for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
		{
			if (World::ins()->getTile(tgtX, tgtY, playerZ).fov == fovFlag::gray)
			{
				dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
				dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
				dst.w = tileSize;
				dst.h = tileSize;

				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 185);
				SDL_RenderFillRect(renderer, &dst);
			}
		}
	}

	for (int tgtY = renderRegion.y; tgtY <= renderRegion.y + renderRegion.h; tgtY++)
	{
		for (int tgtX = renderRegion.x; tgtX <= renderRegion.x + renderRegion.w; tgtX++)
		{
			if (World::ins()->getTile(tgtX, tgtY, playerZ).fov == fovFlag::white)
			{
				dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
				dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
				dst.w = tileSize;
				dst.h = tileSize;

				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer, 0x16, 0x16, 0x16, 90);
				SDL_RenderFillRect(renderer, &dst);

				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
				Uint8 targetR = myMin(255, World::ins()->getTile(tgtX, tgtY, playerZ).redLight);
				Uint8 targetG = myMin(255, World::ins()->getTile(tgtX, tgtY, playerZ).greenLight);
				Uint8 targetB = myMin(255, World::ins()->getTile(tgtX, tgtY, playerZ).blueLight);
				SDL_SetRenderDrawColor(renderer, targetR, targetG, targetB, 200);
				SDL_RenderFillRect(renderer, &dst);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			}
		}
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