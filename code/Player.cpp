#include <SDL3/SDL.h>

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
import drawSprite;
import drawText;

Player::Player(int gridX, int gridY, int gridZ) : Entity(1, gridX, gridY, gridZ)//생성자입니다.
{
	static Player* ptr = this;
	prt(L"[디버그] 플레이어 생성 완료 ID : %p\n", this);

	entityCode = entityRefCode::player;
	entitySpr = spr::charsetHero;
	skin = humanCustom::skin::yellow;
	eyes = humanCustom::eyes::blue;
	hair = humanCustom::hair::bob1Black;

	isPlayer = true;

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

	for (int i = 0; i < TALENT_SIZE; i++) proficApt[i] = 2.0;
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
	if (PlayerPtr->getAniType() == aniFlag::null)
	{
		//errorBox(PlayerPtr->getAniType() == aniFlag::null, L"Player's startMove activated while player's aniFlag is not null.");
		errorBox(((PlayerPtr)->getX() - 8) % 16 != 0, L"This instance moved from non-integer coordinates.");

		int dx, dy;
		dir2Coord(inputDir, dx, dy);
		Player* player = PlayerPtr;
		player->updateWalkable(player->getGridX() + dx, player->getGridY() + dy);
		//걸을 수 있는 타일이면
		if (isWalkable({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }))
		{
			player->setDirection(inputDir);
			if (TileSnow(PlayerX(), PlayerY(), PlayerZ()) || TileFloor(PlayerX(),PlayerY(),PlayerZ()) == itemRefCode::sandFloor)
			{
				new Footprint(getGridX(), getGridY(), direction);
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
	SDL_RenderPoint(renderer, (MINIMAP_DIAMETER / 2), (MINIMAP_DIAMETER / 2));
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
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
	for (int i = cx - DARK_VISION_RADIUS; i <= cx + DARK_VISION_RADIUS; i++)
	{
		for (int j = cy - DARK_VISION_RADIUS; j <= cy + DARK_VISION_RADIUS; j++)
		{
			if (TileFov(i, j, getGridZ()) == fovFlag::white) TileFov(i, j, getGridZ()) = fovFlag::gray;
		}
	}

	std::vector<Point2> tasksVec;
	tasksVec.reserve((2 * DARK_VISION_RADIUS + 1) * (2 * DARK_VISION_RADIUS + 1));
	for (int tgtX = cx - DARK_VISION_RADIUS; tgtX <= cx + DARK_VISION_RADIUS; tgtX++)
	{
		for (int tgtY = cy - DARK_VISION_RADIUS; tgtY <= cy + DARK_VISION_RADIUS; tgtY++)
		{
			tasksVec.push_back({ tgtX,tgtY });
		}
	}

	auto rayCastingWorker = [=](int cx, int cy, const std::vector<Point2>& points, int correctionRange)
		{
			for (const auto& point : points)
			{
				if (isCircle(correctionRange, point.x - cx, point.y - cy)) rayCasting(cx, cy, point.x, point.y);
				else if (isCircle(DARK_VISION_RADIUS, point.x - cx, point.y - cy)) rayCastingDark(cx, cy, point.x, point.y);
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

void Player::updateVision() 
{
	updateVision(eyeSight, getGridX(), getGridY());
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
			World::ins()->activate(x, y, PlayerPtr->getGridZ());
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

	if (PlayerPtr->walkMode == walkFlag::run)
	{
		stamina -= 7;
		if (stamina < 0)
		{
			stamina = 0;
			walkMode = walkFlag::walk;
		}
	}


	if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_SHALLOW))
	{
		walkMode = walkFlag::wade;
	}
	else if (itemDex[TileFloor(getGridX(), getGridY(), getGridZ())].checkFlag(itemFlag::WATER_DEEP))
	{
		walkMode = walkFlag::swim;
	}
	else if (walkMode == walkFlag::swim || walkMode == walkFlag::wade)
	{
		walkMode = walkFlag::walk;
	}

	
	updateVision(eyeSight);
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
	updateLog(L"#FFFFFF죽어버렸다~~!!!!.");
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

void Player::drawSelf()
{
	stepEvent();


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