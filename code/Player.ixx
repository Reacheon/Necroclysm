#include <SDL.h>

export module Player;

import std;
import util;
import Entity;
import World;
import globalVar;
import textureVar;
import constVar;
import log;
import turnWait;
import ItemData;
import nanoTimer;
import globalTime;
import updateBarAct;

export class Player :public Entity //�÷��̾�� ��ƼƼ�� ��ӹް� �þ߿� ���� �̴ϸ��� ������Ʈ�ϴ� ����� ������ �ִ�.
{
private:
	Player();
public:
	Player(int gridX, int gridY, int gridZ) : Entity(gridX, gridY, gridZ)//�������Դϴ�.
	{
		static Player* ptr = this;
		prt(L"[�����] �÷��̾� ���� �Ϸ� ID : %p\n", this);
		setDrawEquip(true);
		setSprite(spr::charsetHero);
		loadDataFromDex(1);
		(World::ins())->getTile(0, 0, 0).EntityPtr = this;
		setTalentFocus(talentFlag::fighting, 1);
		setTalentFocus(talentFlag::dodging, 1);
		setTalentFocus(talentFlag::stealth, 1);


		setSkin(humanCustom::skin::yellow);
		setEyes(humanCustom::eyes::blue);
		setHair(humanCustom::hair::middlePart);

		int i = 0;

		getEquipPtr()->itemInfo.push_back(itemDex[2]);
		getEquipPtr()->itemInfo[i++].equipState = equip::normal;

		getEquipPtr()->itemInfo.push_back(itemDex[290]);
		getEquipPtr()->itemInfo[i++].equipState = equip::normal;

		getEquipPtr()->itemInfo.push_back(itemDex[105]);
		getEquipPtr()->itemInfo[i++].equipState = equip::normal;

		getEquipPtr()->itemInfo.push_back(itemDex[106]);
		getEquipPtr()->itemInfo[i++].equipState = equip::normal;

		getEquipPtr()->itemInfo.push_back(itemDex[107]);
		getEquipPtr()->itemInfo[i++].equipState = equip::normal;

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
	}
	~Player()
	{
		prt(L"Player : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
	}
	static Player* ins()//�̱��� �Լ�
	{
		static Player* ptr = new Player(0, 0, 0);
		ptr->setIsPlayer(true);
		return ptr;
	}

	virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, int inputTarget, aniFlag inputAniType) override
	{
		Entity::startAtk(inputGridX, inputGridY, inputGridZ, inputTarget, inputAniType);
		addAniUSetPlayer(this, inputAniType);
	}

	void startAtk(int inputGridX, int inputGridY, int inputGridZ, int inputTarget) { startAtk(inputGridX, inputGridY, inputGridZ, inputTarget, aniFlag::atk); }

	void startAtk(int inputGridX, int inputGridY, int inputGridZ) { startAtk(inputGridX, inputGridY, inputGridZ, -1); }

	void startMove(int inputDir)
	{
		if (Player::ins()->getAniType() == aniFlag::null)
		{
			//errorBox(Player::ins()->getAniType() == aniFlag::null, "Player's startMove activated while player's aniFlag is not null.");
			errorBox(((Player::ins())->getX() - 8) % 16 != 0, "This instance moved from non-integer coordinates.");

			int dx, dy;
			dir2Coord(inputDir, dx, dy);
			Player* player = Player::ins();
			player->updateWalkable(player->getGridX() + dx, player->getGridY() + dy);
			//���� �� �ִ� Ÿ���̸�
			if (World::ins()->getTile(player->getGridX() + dx, player->getGridY() + dy, player->getGridZ()).walkable)
			{
				player->setDirection(inputDir);
				player->move(inputDir, false);
				turnCycle = turn::playerAnime;
			}
			else
			{
				player->setDirection(inputDir);
				if (World::ins()->getTile(player->getGridX() + dx, player->getGridY() + dy, player->getGridZ()).EntityPtr != nullptr)
				{
					player->startAtk(player->getGridX() + dx, player->getGridY() + dy, player->getGridZ());
					turnWait(1.0);
					Player::ins()->deactAStarDst();
				}
			}
		}
	}

	void updateMinimap()
	{
		SDL_SetRenderTarget(renderer, texture::minimap);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		for (int dx = -(minimapDiameter / 2); dx <= (minimapDiameter / 2); dx++)
		{
			for (int dy = -(minimapDiameter / 2); dy <= (minimapDiameter / 2); dy++)
			{
				if (isCircle(minimapDiameter / 2, dx, dy))
				{
					TileData* tgtTile = &World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ());
					if (tgtTile->fov == fovFlag::white || tgtTile->fov == fovFlag::gray)
					{
						//floor
						switch (tgtTile->floor)
						{
						case 0:
							break;
						default:
							SDL_SetRenderDrawColor(renderer, 112, 112, 112, 255);
							break;
						}

						//wall
						switch (tgtTile->wall)
						{
						case 0:
							break;
						default:
							SDL_SetRenderDrawColor(renderer, 29, 29, 29, 255);
							break;
						}

						//prop
						if (tgtTile->PropPtr != nullptr)
						{
							SDL_SetRenderDrawColor(renderer, lowCol::yellow.r, lowCol::yellow.g, lowCol::yellow.b, 255);
						}

						//vehicle
						if (tgtTile->VehiclePtr != nullptr)
						{
							SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 255);
						}

						SDL_RenderDrawPoint(renderer, dx + (minimapDiameter / 2), dy + (minimapDiameter / 2));

						if (tgtTile->fov == fovFlag::gray)
						{
							SDL_SetRenderDrawColor(renderer, col::black.r, col::black.g, col::black.b, 100);
							SDL_RenderDrawPoint(renderer, dx + (minimapDiameter / 2), dy + (minimapDiameter / 2));
						}
					}
					else
					{
						SDL_SetRenderDrawColor(renderer, col::black.r, col::black.g, col::black.b, 255);
						SDL_RenderDrawPoint(renderer, dx + (minimapDiameter / 2), dy + (minimapDiameter / 2));
					}
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawPoint(renderer, (minimapDiameter / 2), (minimapDiameter / 2));

		SDL_SetRenderTarget(renderer, nullptr);
	}
	void updateVision(int range, int cx, int cy)
	{
		//updateVison�� ���� �������� �ʴ´�. �þ߰� 7���� 8�̾ �������� 0.5ns�� �����̸� �����Ѵ�
		//__int64 timeStampStart = getNanoTimer();
		//prt(L"[updateVision] %d,%d���� �þ߾�����Ʈ�� ����Ǿ���.\n",cx,cy);

		int correctionRange = range;
		if (getHour() >= 6 && getHour() < 18) correctionRange = range;
		else correctionRange = range / 2;

		for (int i = cx - userVisionHalfW; i <= cx + userVisionHalfW; i++)
		{
			for (int j = cy - userVisionHalfH; j <= cy + userVisionHalfH; j++)
			{
				TileData* tgtTile = &World::ins()->getTile(i, j, getGridZ());
				if (tgtTile->fov == fovFlag::white) tgtTile->fov = fovFlag::gray;
			}
		}

		for (int i = cx - userVisionHalfW; i <= cx + userVisionHalfW; i++)
		{
			for (int j = cy - userVisionHalfH; j <= cy + userVisionHalfH; j++)
			{
				if (isCircle(correctionRange, i - cx, j - cy) == true)
				{
					rayCasting(cx, cy, i, j);
				}
				else
				{
					rayCastingDark(cx, cy, i, j);
				}
			}
		}
		//prt(L"updateVision ���࿡ %f ns��ŭ�� �ð��� �ɷȴ�.\n", (float)(getNanoTimer() - timeStampStart) / 10000000.0);
	}

	void updateVision(int range) {
		updateVision(range, getGridX(), getGridY());
	}
	void updateNearbyChunk(int range)
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

	void setGrid(int inputGridX, int inputGridY, int inputGridZ) override
	{
		Coord::setGrid(inputGridX, inputGridY, inputGridZ);

		std::array<int, 2> sectorXY = World::ins()->changeToSectorCoord(getGridX(), getGridY());
		for (int dir = -1; dir <= 7; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);
			if (World::ins()->isEmptySector(sectorXY[0] + dx, sectorXY[1] + dy, getGridZ()) == true) World::ins()->createSector(sectorXY[0] + dx, sectorXY[1] + dy, getGridZ());
		}
		updateNearbyChunk(chunkLoadingRange);
		updateNearbyBarAct(inputGridX, inputGridY, inputGridZ);
	}

	void endMove() override //aStar�� ���� �̵��� ������ ���
	{
		updateVision(getEyeSight());
		updateMinimap();
	}

	void death()
	{
		updateLog(L"#FFFFFF�׾���ȴ�~~!!!!.");
	}

	int checkItemSur(int index)//�ֺ��� �ִ� Ÿ���� ������ �������� ������ �ִ��� ����
	{
		int itemNumber = 0;
		//�ֺ� 9Ÿ���� �����۽��� �˻�
		for (int i = 0; i < 9; i++)
		{
			ItemStack* ptr = World::ins()->getItemPos(getGridX(), getGridY(), getGridZ());
		}
		//�ڱ� �ڽ��� ��� �˻� 
		{
			itemNumber++;
		}
		return itemNumber;
	}
	void eraseItemSur(int index, int number) //�ֺ���ü�� �߽����� �� 9ĭ
	{
		int residue = number;
		//�ֺ� 9Ÿ���� �����۽��� �˻�
		for (int i = 0; i < 9; i++)
		{
			residue--;
			if (residue == 0) { return; }
		}
		//�ڱ� �ڽ��� ��� �˻� 
		{
			residue--;
			if (residue == 0) { return; }
		}
	}
	int checkToolQualitySur(int index) //������ 0 ��ȯ, ������ �������� ��ȯ
	{
		int itemNumber = 0;
		//�ֺ� 9Ÿ���� �����۽��� �˻�
		for (int i = 0; i < 9; i++)
		{
			itemNumber++;
		}
		//�ڱ� �ڽ��� ��� �˻� 
		{
			itemNumber++;
		}
		return itemNumber;
	}


};