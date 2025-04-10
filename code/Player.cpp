#include <SDL.h>

import Player;
import std;
import util;
import Entity;
import World;
import globalVar;
import wrapVar;
import textureVar;
import constVar;
import log;
import ItemData;
import nanoTimer;
import globalTime;
import Footprint;

Player::Player(int gridX, int gridY, int gridZ) : Entity(1, gridX, gridY, gridZ)//생성자입니다.
{
	static Player* ptr = this;
	prt(L"[디버그] 플레이어 생성 완료 ID : %p\n", this);
	(World::ins())->getTile(0, 0, 0).EntityPtr = this;

	entityInfo.skin = humanCustom::skin::yellow;
	entityInfo.eyes = humanCustom::eyes::blue;
	entityInfo.hair = humanCustom::hair::bob1Black;

	int i = 0;

	getEquipPtr()->addItemFromDex(383);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::both;

	getEquipPtr()->addItemFromDex(2);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(290);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(105);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(106);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	getEquipPtr()->addItemFromDex(107);
	getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	updateStatus();
	updateCustomSpriteHuman();

	//방독면
	//getEquipPtr()->addItemFromDex(374);
	//getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;

	addSkill(27);
	quickSlot[0] = { quickSlotFlag::SKILL, 27 };

	addSkill(19);
	quickSlot[1] = { quickSlotFlag::SKILL, 19 };

	addSkill(30);
	quickSlot[2] = { quickSlotFlag::SKILL, 30 };

	addSkill(14);
	quickSlot[3] = { quickSlotFlag::SKILL, 14 };

	addSkill(10);
	quickSlot[4] = { quickSlotFlag::SKILL, 10 };

	addSkill(1);
	quickSlot[5] = { quickSlotFlag::SKILL, 1 };

	for (int i = 0; i < TALENT_SIZE; i++) entityInfo.proficApt[i] = 2.0;
}
Player::~Player()
{
	prt(L"Player : 소멸자가 호출되었습니다..\n");
}


void Player::startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType)
{
	Entity::startAtk(inputGridX, inputGridY, inputGridZ, inputAniType);
	addAniUSetPlayer(this, inputAniType);
}

void Player::startAtk(int inputGridX, int inputGridY, int inputGridZ) { startAtk(inputGridX, inputGridY, inputGridZ, aniFlag::atk); }


void Player::startMove(int inputDir)
{
	if (Player::ins()->getAniType() == aniFlag::null)
	{
		//errorBox(Player::ins()->getAniType() == aniFlag::null, "Player's startMove activated while player's aniFlag is not null.");
		errorBox(((Player::ins())->getX() - 8) % 16 != 0, "This instance moved from non-integer coordinates.");

		int dx, dy;
		dir2Coord(inputDir, dx, dy);
		Player* player = Player::ins();
		player->updateWalkable(player->getGridX() + dx, player->getGridY() + dy);
		//걸을 수 있는 타일이면
		if (isWalkable({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }))
		{
			player->setDirection(inputDir);
			if (TileSnow(PlayerX(), PlayerY(), PlayerZ()) || TileFloor(PlayerX(),PlayerY(),PlayerZ()) == itemVIPCode::sandFloor)
			{
				new Footprint(getGridX(), getGridY(), entityInfo.direction);
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
				Player::ins()->deactAStarDst();
			}
		}
	}
}

void Player::updateMinimap()
{
	auto timeStampStart = getNanoTimer();
	SDL_SetRenderTarget(renderer, texture::minimap);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	for (int dx = -(MINIMAP_DIAMETER / 2); dx <= (MINIMAP_DIAMETER / 2); dx++)
	{
		for (int dy = -(MINIMAP_DIAMETER / 2); dy <= (MINIMAP_DIAMETER / 2); dy++)
		{
			if (isCircle(MINIMAP_DIAMETER / 2, dx, dy))
			{
				SDL_Color ptCol;
				const TileData* tgtTile = &World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ());
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

					drawPoint(dx + (MINIMAP_DIAMETER / 2), dy + (MINIMAP_DIAMETER / 2), ptCol);

					if (tgtTile->fov == fovFlag::gray) drawPoint(dx + (MINIMAP_DIAMETER / 2), dy + (MINIMAP_DIAMETER / 2), col::black, 100);
				}
				else drawPoint(dx + (MINIMAP_DIAMETER / 2), dy + (MINIMAP_DIAMETER / 2), col::black);
			}
		}
	}

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderDrawPoint(renderer, (MINIMAP_DIAMETER / 2), (MINIMAP_DIAMETER / 2));

	SDL_SetRenderTarget(renderer, nullptr);
}


void Player::updateVision(int range, int cx, int cy)
{
	__int64 timeStampStart = getNanoTimer();
	//prt(L"[updateVision] %d,%d에서 시야업데이트가 진행되었다.\n",cx,cy);

	int correctionRange = range;
	if (getHour() >= 6 && getHour() < 18) correctionRange = range;
	else correctionRange = range / 2;

	//줌스케일이 최대일 때 45칸 정도가 최대로 들어옴
	for (int i = cx - GRAY_VISION_HALF_W; i <= cx + GRAY_VISION_HALF_W; i++)
	{
		for (int j = cy - GRAY_VISION_HALF_H; j <= cy + GRAY_VISION_HALF_H; j++)
		{
			TileData* tgtTile = &World::ins()->getTile(i, j, getGridZ());
			if (tgtTile->fov == fovFlag::white) tgtTile->fov = fovFlag::gray;
		}
	}

	//for (int tgtX = cx - DARK_VISION_HALF_W; tgtX <= cx + DARK_VISION_HALF_W; tgtX++)
	//{
	//	for (int tgtY = cy - DARK_VISION_HALF_H; tgtY <= cy + DARK_VISION_HALF_H; tgtY++)
	//	{
	//		if (isCircle(correctionRange, tgtX, tgtY)) rayCasting(cx, cy, tgtX, tgtY);
	//		else rayCastingDark(cx, cy, tgtX, tgtY);
	//	}
	//}


	std::vector<Point2> tasksVec;
	tasksVec.reserve((2 * DARK_VISION_HALF_W + 1) * (2 * DARK_VISION_HALF_H + 1));
	for (int tgtX = cx - DARK_VISION_HALF_W; tgtX <= cx + DARK_VISION_HALF_W; tgtX++)
	{
		for (int tgtY = cy - DARK_VISION_HALF_H; tgtY <= cy + DARK_VISION_HALF_H; tgtY++)
		{
			tasksVec.push_back({ tgtX,tgtY });
		}
	}

	auto rayCastingWorker = [=](int cx, int cy, const std::vector<Point2>& points, int correctionRange)
		{
			for (const auto& point : points)
			{
				if (isCircle(correctionRange, point.x - cx, point.y - cy)) rayCasting(cx, cy, point.x, point.y);
				else rayCastingDark(cx, cy, point.x, point.y);
			}
		};

	int numThreads = threadPoolPtr->getAvailableThreads();
	int chunkSize = tasksVec.size() / numThreads;
	for (int i = 0; i < numThreads; i++) {
		std::vector<Point2>::iterator startPoint = tasksVec.begin() + i * chunkSize;

		std::vector<Point2>::iterator endPoint;
		if (i == numThreads - 1) endPoint = tasksVec.end(); //만약 마지막 스레드일 경우 벡터의 끝을 강제로 설정
		else endPoint = startPoint + chunkSize;
		std::vector<Point2> chunk(startPoint, endPoint);

		threadPoolPtr->addTask([=]() {
			rayCastingWorker(cx, cy, chunk, correctionRange);
			});
	}

	threadPoolPtr->waitForThreads();
}


void Player::updateVision(int range) {
	updateVision(range, getGridX(), getGridY());
}

void Player::updateNearbyChunk(int range)
{
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

	for (int y = chunkY - 1; y <= chunkY + 1; y++)
	{
		for (int x = chunkX - 1; x <= chunkX + 1; x++)
		{
			World::ins()->activate(x, y, ins()->getGridZ());
		}
	}
}

void Player::setGrid(int inputGridX, int inputGridY, int inputGridZ)
{
	Coord::setGrid(inputGridX, inputGridY, inputGridZ);

	std::array<int, 2> sectorXY = World::ins()->changeToSectorCoord(getGridX(), getGridY());
	for (int dir = -1; dir <= 7; dir++)
	{
		int dx, dy;
		dir2Coord(dir, dx, dy);
		if (World::ins()->isEmptySector(sectorXY[0] + dx, sectorXY[1] + dy, getGridZ()) == true) World::ins()->createSector(sectorXY[0] + dx, sectorXY[1] + dy, getGridZ());
	}
	updateNearbyChunk(CHUNK_LOADING_RANGE);
}

void Player::endMove()//aStar로 인해 이동이 끝났을 경우
{

	if (Player::ins()->entityInfo.walkMode == walkFlag::run)
	{
		entityInfo.STA -= 7;
		if (entityInfo.STA < 0)
		{
			entityInfo.STA = 0;
			entityInfo.walkMode = walkFlag::walk;
		}
	}


	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW))
	{
		entityInfo.walkMode = walkFlag::wade;
	}
	else if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP))
	{
		entityInfo.walkMode = walkFlag::swim;
	}
	else if (entityInfo.walkMode == walkFlag::swim || entityInfo.walkMode == walkFlag::wade)
	{
		entityInfo.walkMode = walkFlag::walk;
	}

	
	updateVision(entityInfo.eyeSight);
	updateMinimap();
	if (getHasAStarDst())
	{
		if (getAStarDstX() == getGridX() && getAStarDstY() == getGridY())
		{
			Player::ins()->deactAStarDst();
			aStarTrail.clear();
		}
	}
}

void Player::death()
{
	updateLog(L"#FFFFFF죽어버렸다~~!!!!.");
}

int Player::checkItemSur(int index)//주변에 있는 타일을 포함해 아이템을 가지고 있는지 조사
{
	int itemNumber = 0;
	//주변 9타일의 아이템스택 검사
	for (int i = 0; i < 9; i++)
	{
		ItemStack* ptr = (ItemStack*)World::ins()->getTile(getGridX(), getGridY(), getGridZ()).ItemStackPtr;
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