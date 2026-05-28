#include <SDL3/SDL.h>

import Player;
import std;
import util;
import Entity;
import World;
import globalVar;
import textureVar;
import constVar;
import statusEffect;
import log;
import TileData;
import Chunk;
import Prop;
import ItemPocket;
import ItemData;
import nanoTimer;
import globalTime;
import Footprint;
import GameOver;
import turnWait;
import Wave;
import Wake;
import Sector;
import worldSession;
import Sprite;
import drawSprite;
import Vehicle;

Player::Player(int gridX, int gridY, int gridZ) : Entity(1, gridX, gridY, gridZ)//생성자입니다.
{
	static Player* ptr = this;
	prt(L"[디버그] 플레이어 생성 완료 ID : %p\n", this);

	entityInfo.skinColor = L"LIGHT";
	entityInfo.gender = L"MALE";
	entityInfo.eyeColor = L"BLUE";
	entityInfo.hairStyle = L"HAIR_SHAG";

	entityInfo.isPlayer = true;

	int i = 0;

	//getEquipPtr()->addItemFromDex(itemID::katana);
	//getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::both;

	getEquipPtr()->addItemFromDex(itemID::backpack);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(itemID::fieldJacket);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(itemID::shoes);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(itemID::jeans);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(itemID::blackTshirt);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	//남자 시작 캐릭터 기본 속옷
	getEquipPtr()->addItemFromDex(itemID::briefs);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	updateStatus();

	//방독면
	//getEquipPtr()->addItemFromDex(374);
	//getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	addSkill(L"SKILL_FIRESTORM");
	quickSlot[2] = { quickSlotFlag::SKILL, L"SKILL_FIRESTORM" };

	addSkill(L"SKILL_ROLL");
	quickSlot[0] = { quickSlotFlag::SKILL, L"SKILL_ROLL" };

	addSkill(L"SKILL_LEAP");
	quickSlot[1] = { quickSlotFlag::SKILL, L"SKILL_LEAP" };

	for (int i = 0; i < TALENT_SIZE; i++) entityInfo.proficApt[i] = 2.0;
}
Player::~Player()
{
	prt(L"Player : 소멸자가 호출되었습니다..\n");
}


void Player::startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType)
{
	Entity::startAtk(inputGridX, inputGridY, inputGridZ, inputAniType);
	addAniToPlayerTurn(this, inputAniType);
}

void Player::startAtk(int inputGridX, int inputGridY, int inputGridZ) { startAtk(inputGridX, inputGridY, inputGridZ, aniFlag::atk); }


void Player::startMove(int inputDir)
{
	if (PlayerPtr->getAniType() == aniFlag::null)
	{
		//errorBox(PlayerPtr->getAniType() == aniFlag::null, L"Player's startMove activated while player's aniFlag is not null.");
		errorBox(((PlayerPtr)->getX() - 8) % 16 != 0, L"This instance moved from non-integer coordinates.");

		int dx, dy;
		dir2Coord(inputDir, dx, dy);
		Player* player = PlayerPtr;
		//걸을 수 있는 타일이면
		if (isWalkable({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }))
		{
			player->setDirection(inputDir);
			if (TileSnow(PlayerX(), PlayerY(), PlayerZ()) || TileFloor(PlayerX(),PlayerY(),PlayerZ()) == itemID::sandFloor)
			{
				new Footprint(getGridX(), getGridY(), getGridZ(), entityInfo.direction);
			}
			else if (TileFloor(PlayerX(), PlayerY(), PlayerZ()) == itemID::deepFreshWater || TileFloor(PlayerX(), PlayerY(), PlayerZ()) == itemID::deepSeaWater)
			{
				new Wake(getGridX(), getGridY(), getGridZ(), entityInfo.direction);
			}

			player->move(inputDir, false);
			turnCycle = turn::playerAnime;
		}
		else
		{
			player->setDirection(inputDir);
			if (TileEntity(player->getGridX() + dx, player->getGridY() + dy, player->getGridZ()) != nullptr)
			{
				player->startAtk(player->getGridX() + dx, player->getGridY() + dy, player->getGridZ());
				turnWait(1.0);
				PlayerPtr->deactAStarDst();
			}
		}
	}
}

void Player::updateMinimap()
{
	auto timeStampStart = getNanoTimer();


	// 현재 플레이어가 있는 청크 좌표 계산
	int playerChunkX, playerChunkY;
	World::ins()->changeToChunkCoord(getGridX(), getGridY(), playerChunkX, playerChunkY);

	if (ctrlVeh == nullptr)
	{
		SDL_SetRenderTarget(renderer, texture::minimap);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		// Map.ixx의 타일 스프라이트 레이어와 동일한 방식 — floor + wall을 실제 spr::tileset으로 그림.
		// 타일 1개 = MINIMAP_TILE_PX × MINIMAP_TILE_PX 픽셀 (예: 6×6). 16×16 스프라이트를 축소(NEAREST)해서 표시.
		const int R = MINIMAP_DIAMETER / 2;
		const int TPX = MINIMAP_TILE_PX;
		const int pgx = getGridX();
		const int pgy = getGridY();
		const int pgz = getGridZ();

		// 16×16 타일 스프라이트를 TPX×TPX로 축소 렌더 — drawSprite는 s_zoomScale을 사용.
		setZoom(static_cast<float>(TPX) / 16.0f);

		auto drawTileSpr = [&](int sprIdx, int destX, int destY)
		{
			drawSprite(spr::tileset, sprIdx, destX, destY);
		};

		for (int dy = -R; dy <= R; dy++)
		{
			for (int dx = -R; dx <= R; dx++)
			{
				if (isCircle(R, dx, dy) == false) continue;

				const int destX = (dx + R) * TPX;
				const int destY = (dy + R) * TPX;
				SDL_FRect cell = { static_cast<float>(destX), static_cast<float>(destY),
				                   static_cast<float>(TPX), static_cast<float>(TPX) };

				const TileData* tgtTile = &World::ins()->getTile(pgx + dx, pgy + dy, pgz);
				if (tgtTile->fov == fovFlag::white || tgtTile->fov == fovFlag::gray)
				{
					if (tgtTile->floor != itemID::none)
					{
						int sprIdx = itemDex[tgtTile->floor].tileSprIndex
							+ itemDex[tgtTile->floor].extraSprIndexSingle
							+ 16 * itemDex[tgtTile->floor].extraSprIndex16;
						drawTileSpr(sprIdx, destX, destY);
					}
					if (tgtTile->wall != itemID::none)
					{
						int sprIdx = itemDex[tgtTile->wall].tileSprIndex
							+ itemDex[tgtTile->wall].extraSprIndexSingle
							+ 16 * itemDex[tgtTile->wall].extraSprIndex16;
						drawTileSpr(sprIdx, destX, destY);
					}
					//prop / vehicle도 실제 스프라이트로 그림. propset(48×48)의 중앙 16×16만 잘라
					//floor/wall과 동일한 6×6 셀로 렌더 — 트리/대형 prop의 over-tile 부분은 잘림.
					const int cellCx = destX + TPX / 2;
					const int cellCy = destY + TPX / 2;
					if (tgtTile->PropPtr != nullptr)
					{
						const ItemData& propItem = tgtTile->PropPtr->leadItem;
						//RAMP는 메인뷰에서도 prop sprite를 안 그리고 별도 화살표로 표시 → 미니맵에서도 스킵.
						if (propItem.checkFlag(itemFlag::RAMP_UP) == false
						 && propItem.checkFlag(itemFlag::RAMP_DOWN) == false)
						{
							int sprIdx = propItem.propSprIndex
								+ propItem.extraSprIndexSingle
								+ 16 * propItem.extraSprIndex16;
							drawSpriteCenterExSrc(spr::propset, sprIdx, cellCx, cellCy, SDL_Rect{ 16, 16, 16, 16 });
						}
					}
					if (tgtTile->VehiclePtr != nullptr)
					{
						auto it = tgtTile->VehiclePtr->partInfo.find({ pgx + dx, pgy + dy, pgz });
						if (it != tgtTile->VehiclePtr->partInfo.end())
						{
							for (const ItemData& part : it->second->itemInfo)
							{
								int sprIdx = part.propSprIndex
									+ part.extraSprIndexSingle
									+ 16 * part.extraSprIndex16;
								drawSpriteCenterExSrc(spr::propset, sprIdx, cellCx, cellCy, SDL_Rect{ 16, 16, 16, 16 });
							}
						}
					}
					if (tgtTile->fov == fovFlag::gray)
					{
						SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
						SDL_RenderFillRect(renderer, &cell);
					}
				}
				else
				{
					SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderFillRect(renderer, &cell);
				}
			}
		}

		setZoom(1.0f);

		// 플레이어 마커 (중앙 타일)
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_FRect playerRect = {
			static_cast<float>(R * TPX),
			static_cast<float>(R * TPX),
			static_cast<float>(TPX),
			static_cast<float>(TPX)
		};
		SDL_RenderFillRect(renderer, &playerRect);
	}
	else
	{
		SDL_SetRenderTarget(renderer, texture::navimap);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		for (int dx = -(NAVIMAP_WIDTH / 2); dx <= (NAVIMAP_WIDTH / 2); dx++)
		{
			for (int dy = -(NAVIMAP_HEIGHT / 2); dy <= (NAVIMAP_HEIGHT / 2); dy++)
			{
				SDL_Color ptCol;
				const TileData* tgtTile = &World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ());
				ptCol = { 255,255, 255 };
				drawPoint(dx + (NAVIMAP_WIDTH / 2), dy + (NAVIMAP_HEIGHT / 2), ptCol);
				if (tgtTile->fov == fovFlag::white || tgtTile->fov == fovFlag::gray)
				{
					//floor
					switch (tgtTile->floor)
					{
					case 0:
						break;
					default:
						ptCol = { 112,112, 112 };
						break;
					}
					//wall
					switch (tgtTile->wall)
					{
					case 0:
						break;
					default:
						ptCol = { 29,29, 29 };
						break;
					}
					//prop
					if (tgtTile->PropPtr != nullptr) ptCol = lowCol::yellow;
					//vehicle
					if (tgtTile->VehiclePtr != nullptr) ptCol = lowCol::orange;
					drawPoint(dx + (NAVIMAP_WIDTH / 2), dy + (NAVIMAP_HEIGHT / 2), ptCol);
					if (tgtTile->fov == fovFlag::gray) drawPoint(dx + (NAVIMAP_WIDTH / 2), dy + (NAVIMAP_HEIGHT / 2), col::black, 100);
				}
				else drawPoint(dx + (NAVIMAP_WIDTH / 2), dy + (NAVIMAP_HEIGHT / 2), col::black);
			}
		}
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderPoint(renderer, (NAVIMAP_WIDTH / 2), (NAVIMAP_HEIGHT / 2));
	}
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_SetRenderTarget(renderer, nullptr);
}



void Player::updateVision(int range, int cx, int cy)
{
	std::int64_t tStart = getNanoTimer();

	int correctionRange = range;
	if (getHour() >= 6 && getHour() < 18) correctionRange = range;
	else correctionRange = myMax(1, range - 2);

	const int z = getGridZ();
	World* world = World::ins();

	// 청크 포인터 캐시: ray와 gray 루프 모두 공간적으로 인접한 타일에 연쇄 접근하므로
	// 같은 청크 안에서는 unordered_map 룩업을 회피한다.
	int cachedCX = 0;
	int cachedCY = 0;
	Chunk* cachedChunk = nullptr;
	bool cacheValid = false;

	auto fetchTile = [&](int x, int y) -> TileData*
	{
		int chunkX, chunkY;
		world->changeToChunkCoord(x, y, chunkX, chunkY);
		if (!cacheValid || chunkX != cachedCX || chunkY != cachedCY)
		{
			cachedChunk = world->tryGetChunk(chunkX, chunkY, z);
			cachedCX = chunkX;
			cachedCY = chunkY;
			cacheValid = true;
		}
		if (cachedChunk == nullptr) return nullptr;
		int localX = x - chunkX * CHUNK_SIZE_X;
		int localY = y - chunkY * CHUNK_SIZE_Y;
		return &cachedChunk->getChunkTile(localX, localY);
	};

	// Phase 1: 시야권 내 white → gray (이전 프레임 기억 다운그레이드)
	for (int j = cy - DARK_VISION_RADIUS; j <= cy + DARK_VISION_RADIUS; j++)
	{
		for (int i = cx - DARK_VISION_RADIUS; i <= cx + DARK_VISION_RADIUS; i++)
		{
			TileData* t = fetchTile(i, j);
			if (t != nullptr && t->fov == fovFlag::white) t->fov = fovFlag::gray;
		}
	}

	std::int64_t tAfterGray = getNanoTimer();

	// Phase 2: 단일 스레드 인라인 Bresenham + 캐시된 타일 접근
	// markStep은 타일을 visible 마킹하고, blocker면 true를 반환해 ray를 종료시킴.
	auto castRay = [&](int x2, int y2, bool darkMode)
	{
		int x1 = cx;
		int y1 = cy;
		const int xo = cx;
		const int yo = cy;
		int delx = std::abs(x2 - x1);
		int dely = std::abs(y2 - y1);

		// origin은 무조건 visible
		TileData* originTile = fetchTile(x1, y1);
		if (originTile != nullptr) originTile->fov = fovFlag::white;

		if (delx == 0 && dely == 0) return;

		auto markStep = [&](int sx, int sy) -> bool
		{
			TileData* t = fetchTile(sx, sy);
			if (t == nullptr) return true; // 청크 누락 → ray 종료
			if (darkMode)
			{
				if (!t->lightVec.empty()) t->fov = fovFlag::white;
			}
			else
			{
				t->fov = fovFlag::white;
			}
			// 인라인 isRayBlocker — 동일 TileData 참조에서 wall + prop을 한 번에 판정
			if (t->wall != itemID::none && itemDex[t->wall].checkFlag(itemFlag::TRANSPARENT_WALL) == false) return true;
			if (t->PropPtr != nullptr && t->PropPtr->leadItem.checkFlag(itemFlag::PROP_BLOCKER) == true) return true;
			return false;
		};

		if (delx > dely)
		{
			// slope < 1: x를 매 step 진행, y는 가끔
			int p = 2 * dely - delx;
			for (int i = 0; i < delx; ++i)
			{
				if (p < 0)
				{
					if (x2 > xo) ++x1; else --x1;
					p += 2 * dely;
				}
				else
				{
					if (x2 > xo) ++x1; else --x1;
					if (y2 > yo) ++y1; else if (y2 < yo) --y1;
					p += 2 * (dely - delx);
				}
				if (markStep(x1, y1)) return;
			}
		}
		else if (dely > delx)
		{
			// slope > 1: y를 매 step 진행, x는 가끔
			int p = 2 * delx - dely;
			for (int i = 0; i < dely; ++i)
			{
				if (p < 0)
				{
					if (y2 > yo) ++y1; else --y1;
					p += 2 * delx;
				}
				else
				{
					if (x2 > xo) ++x1; else if (x2 < xo) --x1;
					if (y2 > yo) ++y1; else --y1;
					p += 2 * (delx - dely);
				}
				if (markStep(x1, y1)) return;
			}
		}
		else
		{
			// slope == 1: 매 step 대각 진행
			for (int i = 0; i < delx; ++i)
			{
				if (x2 > x1) ++x1; else --x1;
				if (y2 > y1) ++y1; else --y1;
				if (markStep(x1, y1)) return;
			}
		}
	};

	// Phase 3: 시야권 그리드를 직접 순회 (tasksVec / dispatch 제거)
	int rayCount = 0;
	for (int dy = -DARK_VISION_RADIUS; dy <= DARK_VISION_RADIUS; ++dy)
	{
		for (int dx = -DARK_VISION_RADIUS; dx <= DARK_VISION_RADIUS; ++dx)
		{
			if (isCircle(correctionRange, dx, dy))
			{
				castRay(cx + dx, cy + dy, false);
				++rayCount;
			}
			else if (isCircle(DARK_VISION_RADIUS, dx, dy))
			{
				castRay(cx + dx, cy + dy, true);
				++rayCount;
			}
		}
	}

	std::int64_t tEnd = getNanoTimer();

	double total = (tEnd - tStart)       / 1000000.0;
	double gray  = (tAfterGray - tStart) / 1000000.0;
	double work  = (tEnd - tAfterGray)   / 1000000.0;

	//prt(L"[updateVision perf] (%d,%d) total=%.3fms | gray=%.3fms work=%.3fms | rays=%d (single-thread)\n",
	//	cx, cy, total, gray, work, rayCount);
}


void Player::updateVision(int range) {
	updateVision(range, getGridX(), getGridY());
}

void Player::updateVision() 
{
	updateVision(entityInfo.eyeSight, getGridX(), getGridY());
}

void Player::updateNearbyChunk(int range)
{
    //std::wprintf(L"updateNearbyChunk 호출됨. 플레이어 좌표 : %d,%d,%d\n", getGridX(), getGridY(), getGridZ());
	int chunkX, chunkY;
	World::ins()->changeToChunkCoord(getGridX(), getGridY(), chunkX, chunkY);
	for (int y = chunkY - range; y <= chunkY + range; y++)
	{
		for (int x = chunkX - range; x <= chunkX + range; x++)
		{
			if (World::ins()->existChunk(x, y, getGridZ()) == false)
			{
				World::ins()->createChunk(x, y, getGridZ());
			}
		}
	}

	for (int y = chunkY - range; y <= chunkY + range; y++)
	{
		for (int x = chunkX - range; x <= chunkX + range; x++)
		{
			if (World::ins()->existChunk(x, y, getGridZ() + 1) == false)
			{
				World::ins()->createChunk(x, y, getGridZ() + 1);
			}
		}
	}

	for (int y = chunkY - range; y <= chunkY + range; y++)
	{
		for (int x = chunkX - range; x <= chunkX + range; x++)
		{
			if (World::ins()->existChunk(x, y, getGridZ() - 1) == false)
			{
				World::ins()->createChunk(x, y, getGridZ() - 1);
			}
		}
	}

	World::ins()->deactivate();

	//std::wprintf(L"▼청크 활성화▼\n");
	for (int x = chunkX - 2; x <= chunkX + 2; x++)
	{
		for (int y = chunkY - 2; y <= chunkY + 2; y++)
		{
            //std::wprintf(L"(%d,%d) ", x, y);
			//if (y == chunkY + 2) std::wprintf(L"\n");
			World::ins()->activate(x, y, PlayerZ());
		}
	}
}

void Player::setGrid(int inputGridX, int inputGridY, int inputGridZ)
{
	Coord::setGrid(inputGridX, inputGridY, inputGridZ);

	// (Patch 시스템 제거됨 — 청크 페인트는 mmap 활성 시 Sector 경유, 그 외 chunkFlag 디폴트.)
	updateNearbyChunk(CHUNK_LOADING_RANGE);

	// 월드젠 완료 후에만 섹터 ensure — 시작 영역(startArea)에서는 worldSeed=0이라 의미 없음
	if (worldGenResult.has_value())
	{
		loadNearbySectors(Point3{ getGridX(), getGridY(), getGridZ() }, worldSeed);
	}
}

void Player::endMove()//aStar로 인해 이동이 끝났을 경우
{

	if (PlayerInfo().walkMode == walkFlag::run)
	{
		entityInfo.STA -= 7;
		if (entityInfo.STA < 0)
		{
			entityInfo.STA = 0;
            changeWalkMode(walkFlag::walk);
		}
	}


	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW))
	{
        changeWalkMode(walkFlag::wade);
	}
	else if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP))
	{
        changeWalkMode(walkFlag::swim);
	}
	else if (entityInfo.walkMode == walkFlag::swim || entityInfo.walkMode == walkFlag::wade)
	{
		changeWalkMode(walkFlag::walk);
	}

	if(TileFloor(getGridX(),getGridY(),getGridZ()) == itemID::shallowFreshWater ||
	   TileFloor(getGridX(),getGridY(),getGridZ()) == itemID::shallowSeaWater)
	{
		new Wave(getGridX(), getGridY(), getGridZ());
    }


	
	updateVision(entityInfo.eyeSight);
	updateMinimap();
	if (getHasAStarDst())
	{
		if (getAStarDstX() == getGridX() && getAStarDstY() == getGridY())
		{
			PlayerPtr->deactAStarDst();
			aStarTrail.clear();
		}
	}
}

void Player::death()
{
	GameOver::create(L"HP가 0이 되어 사망했다.");
}

int Player::checkItemSur(int index)//주변에 있는 타일을 포함해 아이템을 가지고 있는지 조사
{
	int itemNumber = 0;
	//주변 9타일의 아이템스택 검사
	for (int i = 0; i < 9; i++)
	{
		ItemStack* ptr = TileItemStack(getGridX(), getGridY(), getGridZ());
	}
	//자기 자신의 장비 검사 
	{
		itemNumber++;
	}
	return itemNumber;
}
void Player::eraseItemSur(int index, int number) //주변객체를 중심으로 총 9칸
{
	int residue = number;
	//주변 9타일의 아이템스택 검사
	for (int i = 0; i < 9; i++)
	{
		residue--;
		if (residue == 0) { return; }
	}
	//자기 자신의 장비 검사 
	{
		residue--;
		if (residue == 0) { return; }
	}
}
int Player::checkToolQualitySur(int index) //없으면 0 반환, 있으면 공구레벨 반환
{
	int itemNumber = 0;
	//주변 9타일의 아이템스택 검사
	for (int i = 0; i < 9; i++)
	{
		itemNumber++;
	}
	//자기 자신의 장비 검사 
	{
		itemNumber++;
	}
	return itemNumber;
}



// Torso (몸통) 저항
int Player::getResPierceTorso()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rPierceTorso;
	return totalVal;
}

int Player::getResCutTorso()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCutTorso;
	return totalVal;
}

int Player::getResBashTorso()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rBashTorso;
	return totalVal;
}

// Head (머리) 저항
int Player::getResPierceHead()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rPierceHead;
	return totalVal;
}

int Player::getResCutHead()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCutHead;
	return totalVal;
}

int Player::getResBashHead()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rBashHead;
	return totalVal;
}

// Left Arm (왼팔) 저항
int Player::getResPierceLArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rPierceLArm;
	return totalVal;
}

int Player::getResCutLArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCutLArm;
	return totalVal;
}

int Player::getResBashLArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rBashLArm;
	return totalVal;
}

// Right Arm (오른팔) 저항
int Player::getResPierceRArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rPierceRArm;
	return totalVal;
}

int Player::getResCutRArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCutRArm;
	return totalVal;
}

int Player::getResBashRArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rBashRArm;
	return totalVal;
}

// Left Leg (왼다리) 저항
int Player::getResPierceLLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rPierceLLeg;
	return totalVal;
}

int Player::getResCutLLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCutLLeg;
	return totalVal;
}

int Player::getResBashLLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rBashLLeg;
	return totalVal;
}

// Right Leg (오른다리) 저항
int Player::getResPierceRLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rPierceRLeg;
	return totalVal;
}

int Player::getResCutRLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCutRLeg;
	return totalVal;
}

int Player::getResBashRLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rBashRLeg;
	return totalVal;
}

////////////////////////////////////////////////////////////////////////////////

int Player::getSH()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].sh;
	return totalVal;
}

int Player::getEV()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].ev;
	return totalVal;
}

int Player::getResFire()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rFire;
	return totalVal;
}

int Player::getResCold()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCold;
	return totalVal;
}

int Player::getResElec()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rElec;
	return totalVal;
}

int Player::getResCorr()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rCorr;
	return totalVal;
}

int Player::getResRad()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].rRad;
	return totalVal;
}

// Torso (몸통) 방해도
int Player::getEncTorso()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].encTorso;
	return totalVal;
}

// Head (머리) 방해도
int Player::getEncHead()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].encHead;
	return totalVal;
}

// Left Arm (왼팔) 방해도
int Player::getEncLArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].encLArm;
	return totalVal;
}

// Right Arm (오른팔) 방해도
int Player::getEncRArm()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].encRArm;
	return totalVal;
}

// Left Leg (왼다리) 방해도
int Player::getEncLLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].encLLeg;
	return totalVal;
}

// Right Leg (오른다리) 방해도
int Player::getEncRLeg()
{
	std::vector<ItemData>& equip = entityInfo.equipment.get()->itemInfo;
	int totalVal = 0;
	for (int i = 0; i < equip.size(); i++) totalVal += equip[i].encRLeg;
	return totalVal;
}

void Player::changeWalkMode(walkFlag inputMode)
{
	auto& pStatus = entityInfo.statusEffectVec;

	pStatus.erase(std::remove_if(pStatus.begin(), pStatus.end(),
		[](statusEffect& effect)
		{
			return effect.effectType == statusEffectFlag::run ||
				effect.effectType == statusEffectFlag::crouch ||
				effect.effectType == statusEffectFlag::crawl;
		}),
		pStatus.end());

	if (inputMode == walkFlag::run) pStatus.push_back({ statusEffectFlag::run, -1 });
	else if (inputMode == walkFlag::crouch) pStatus.push_back({ statusEffectFlag::crouch, -1 });
	else if (inputMode == walkFlag::crawl) pStatus.push_back({ statusEffectFlag::crawl, -1 });

	entityInfo.walkMode = inputMode;
}