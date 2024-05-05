#include <SDL.h>
#include <SDL_image.h>

export module Entity;

import std;
import globalVar;
import constVar;
import textureVar;
import log;
import Sprite;
import ItemPocket;
import Ani;
import constVar;
import Coord;
import World;
import Sticker;
import ItemStack;
import ItemPocket;
import EntityData;
import ItemData;
import Damage;
import util;
import turnWait;
import Drawable;
import drawSprite;

export class Entity : public Ani, public Coord, public Drawable//��ƼƼ�� �þ� ��ɰ� ���� FOV, ���� ��ǥ, �׸��� ȭ�鿡 ǥ�õǴ� ��ɰ� ���� �ؽ��ĸ� ������.
{
private:
	Sprite* sprite = nullptr; //���� ��������Ʈ
	Sprite* spriteFlash = nullptr; //�÷��ÿ� ��� ����ŷ ��������Ʈ
	SDL_Color flash = { 0,0,0,0 }; //�÷��� �÷�
	int hp;
	int direction = 7;
	int spriteIndex = 0;
	bool canMove;
	bool drawEquip = false;
	int eyeSight = 8; //�⺻ �þ�
	bool isPlayer = false;
	int spriteInfimum = 0;
	bool bothHand = false;//��չ������� üũ? ���� ���� ����

	bool leftFoot = true; //�ȱ� �ִϸ��̼ǿ����� �޹�, ������ ����
	int flashType = 0; // 0 : NULL, 1 : white, 2 : white->red
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	ItemPocket* equipment;
	std::vector<std::array<int, pocketFlagWidth>> pocketInfo;
	ItemStack* dropItemPtr = nullptr; //������ �� �����⿡ ����ϴ� �����۽��� ������
	std::vector<std::array<int, dmgFlagSize>> dmgVec;//���� �� ��ü�� ���� �������� �ѷ�

	bool hasAStarDst = false;
	int aStarDstX = 0;
	int aStarDstY = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////

	std::array<int, talentSize> talentExp = { 0, }; //����ġ
	std::array<float, talentSize> talentApt; //����
	std::array<int, talentSize> talentFocus = { 0, }; //���ߵ� 0:�̺й�, 1:�Һй�, 2:�Ϲݺй�

	///////////////////////////////////////////////////////////////////////////////////////////

	std::vector<int> mutationVec;//���� �� ��ü�� �������� ���
	std::vector<int> bionicVec;//���� �� ��ü�� ���̿��� ���

	int atkTargetGridX = 0;
	int atkTargetGridY = 0;
	int atkTargetGridZ = 0;
	int atkTargetPart = -1;

	atkType nextAtkType = atkType::bash; //���� ���ݿ� ����� ������ Ÿ��
	unsigned __int8 aimStack = 0; //���� ���ݿ� ����� ���� ����

	int aimWeaponHand = equip::right;//���� ������ �ܴ��� �ֹ���

	humanCustom::skin skin = humanCustom::skin::null;
	humanCustom::eyes eyes = humanCustom::eyes::null;
	humanCustom::scar scar = humanCustom::scar::null;
	humanCustom::beard beard = humanCustom::beard::null;
	humanCustom::hair hair = humanCustom::hair::null;
public:
	EntityData entityInfo;
	Entity(int gridX, int gridY, int gridZ)//������
	{
		prt(col::orange, L"Entity : �����ڰ� ȣ��Ǿ����ϴ�!\n");
		setAniPriority(1);
		setGrid(gridX, gridY, gridZ);
		World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = this;
		setSprite(spr::defaultMonster);
		equipment = new ItemPocket(storageType::equip);
		hp = 20;
		direction = 0;
		canMove = true;

		std::fill(talentApt.begin(), talentApt.end(), 2.0f);
	}
	~Entity()//�Ҹ���
	{
		World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;
		prt(col::orange, L"Entity : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
		//delete sprite;
		delete spriteFlash;
		delete equipment;
	}
#pragma region getset method

	humanCustom::skin getSkin() { return skin;  }
	void setSkin(humanCustom::skin input) { skin = input; }

	humanCustom::eyes getEyes() { return eyes; }
	void setEyes(humanCustom::eyes input) { eyes = input; }

	humanCustom::scar getScar() { return scar; }
	void setScar(humanCustom::scar input) { scar = input; }

	humanCustom::beard getBeard() { return beard; }
	void setBeard(humanCustom::beard input) { beard = input; }

	humanCustom::hair getHair() { return hair; }
	void setHair(humanCustom::hair input) { hair = input; }


	unsigned __int8 getAimStack() { return aimStack; }
	void initAimStack() { aimStack = 0; }
	void addAimStack() { aimStack++; }

	void setNextAtkType(atkType inputAtkType) { nextAtkType = inputAtkType; }
	atkType getNextAtkType() { return nextAtkType; }
	void setAtkTarget(int inputX, int inputY, int inputZ, int inputPart)
	{
		atkTargetGridX = inputX;
		atkTargetGridY = inputY;
		atkTargetGridZ = inputZ;
		atkTargetPart = inputPart;
	}
	void setAtkTarget(int inputX, int inputY, int inputZ)
	{
		setAtkTarget(inputX, inputY, inputZ, -1);
	}
	ItemPocket* getEquipPtr() { return equipment; }
	Sprite* getSpriteFlash() { return spriteFlash; }
	void setFlashType(int inputType)
	{
		flashType = inputType;
		if (inputType == 1)
		{
			setFlashRGBA(255, 255, 255, 255);
		}
		else
		{
			setFlashRGBA(0, 0, 0, 0);
		}
	}
	void setSpriteFlash(Sprite* inputSprite) { spriteFlash = inputSprite; }
	int getFlashType() { return flashType; }
	bool getLeftFoot() { return leftFoot; }
	void setLeftFoot(bool input) { leftFoot = input; }
	void setSpriteInfimum(int inputVal) { spriteInfimum = inputVal; }
	int getSpriteInfimum() { return spriteInfimum; }
	SDL_RendererFlip getEntityFlip() { return flip; }
	void setEntityFlip(SDL_RendererFlip inputFlip) { flip = inputFlip; }
	void setSpriteIndex(int index) { spriteIndex = index; }
	int getSpriteIndex() { return spriteIndex; }
	Sprite* getSprite() { return sprite; }
	void setSprite(Sprite* inputSprite)
	{
		sprite = inputSprite;

		// make textureFlash
		int textureW, textureH;
		SDL_QueryTexture(inputSprite->getTexture(), NULL, NULL, &textureW, &textureH);
		SDL_Texture* drawingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, textureW, textureH);
		SDL_SetRenderTarget(renderer, drawingTexture);
		SDL_SetTextureBlendMode(drawingTexture, SDL_BLENDMODE_BLEND);
		SDL_Rect src = { 0, 0, textureW, textureH };
		SDL_Rect dst = src;
		//�Ͼ�� ���� �ؽ��ĸ� �׷�����
		SDL_RenderCopy(renderer, inputSprite->getTexture(), &src, &dst);
		//�ؽ��Ŀ� ������� ���� ������ ����� �Ͼ�� ����
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_Rect dstWhite = { 0, 0, textureW, textureH };
		SDL_RenderFillRect(renderer, &dstWhite);
		SDL_SetRenderTarget(renderer, nullptr);
		if (spriteFlash != nullptr) { delete spriteFlash; }
		spriteFlash = new Sprite(renderer, drawingTexture, 48, 48);
	}
	void setDirection(int dir)
	{
		direction = dir;
		if (dir == 2 || dir == 6) {}
		else if (dir == 0 || (dir == 1 || dir == 7)) { setEntityFlip(SDL_FLIP_NONE); }
		else { setEntityFlip(SDL_FLIP_HORIZONTAL); }
	}
	int getDirection() { return direction; }
	bool getCanMove() { return canMove; }

	void startAtk(int inputGridX, int inputGridY, int inputGridZ, int inputTarget, aniFlag inputAniType)
	{
		setDirection(getIntDegree(getGridX(), getGridY(), inputGridX, inputGridY));
		setAtkTarget(inputGridX, inputGridY, inputGridZ, inputTarget);
		aniUSet.insert(this);
		setAniType(inputAniType);
	}

	void startAtk(int inputGridX, int inputGridY, int inputGridZ, int inputTarget)
	{
		setDirection(getIntDegree(getGridX(), getGridY(), inputGridX, inputGridY));
		setAtkTarget(inputGridX, inputGridY, inputGridZ, inputTarget);
		aniUSet.insert(this);
		setAniType(aniFlag::atk);
	}
	void startAtk(int inputGridX, int inputGridY, int inputGridZ)
	{
		startAtk(inputGridX, inputGridY, inputGridZ, -1);
	}
	float endAtk()
	{
		setAniType(aniFlag::null);
		return 1 / 0.8; //���� ���⿡ ���ݼӵ��� ������
	}
	void setDrawEquip(bool val) { drawEquip = val; }
	bool getDrawEquip() { return drawEquip; }
	int getEyeSight() { return eyeSight; }
	void setEyeSight(int inputInt) { eyeSight = inputInt; }
	bool getIsPlayer() { return isPlayer; }
	void setIsPlayer(bool input) { isPlayer = input; }
	void setDropItemPtr(ItemStack* inputPtr) { dropItemPtr = inputPtr; }
	ItemStack* getDropItemPtr() { return dropItemPtr; }
	void loadDataFromDex(int index)
	{
		entityInfo = entityDex[index];
		entityInfo.HP = entityInfo.maxHP;
		entityInfo.fakeHP = entityInfo.maxHP;
	}
	//@brief �ش� ������ �������� �߰��ϰ� ���� HP�� �׸�ŭ ���ϴ�.
	void addDmg(int inputPartIndex, int inputDmg)
	{
		for (int i = 0; i < entityInfo.parts.size(); i++)
		{
			if (entityInfo.parts[i][partsFlag::index] == inputPartIndex)
			{
				new Damage(std::to_wstring(inputDmg), this->getX(), this->getY() - 8, col::white, 10);

				entityInfo.parts[i][partsFlag::hp] -= inputDmg;
				entityInfo.HP -= inputDmg;
				prt(col::orange, L"[����] %d�� �������� ���� %d�� ���Ͽ���. �ش� ������ ���� HP�� %d�̸� ��ü HP�� %d�̴�.\n", inputDmg, inputPartIndex, entityInfo.parts[i][partsFlag::hp], entityInfo.HP);

				if (entityInfo.HP <= 0)//HP 0, ���
				{
					death();
					return;
				}

				if (entityInfo.parts[i][partsFlag::hp] <= 0)//�����ı�
				{
					switch (entityInfo.bodyTemplate)
					{
					case bodyTemplateFlag::human://����� ���
					{
						//�Ӹ� �Ǵ� �� �ı�
						if (inputPartIndex == partType::head || inputPartIndex == partType::torso)
						{
							death();
						}
						//�� �ı�
						else if (inputPartIndex == partType::larm || inputPartIndex == partType::rarm)
						{

						}
						//�ٸ� �ı�
						else if (inputPartIndex == partType::lleg || inputPartIndex == partType::rleg)
						{

						}
						break;
					}
					case bodyTemplateFlag::zombie://������ ���
					{
						//�� �ı�
						if (inputPartIndex == partType::torso)
						{
							death();
						}
						//�Ӹ� �ı�
						if (inputPartIndex == partType::head)
						{
						}
						//�� �ı�
						else if (inputPartIndex == partType::larm || inputPartIndex == partType::rarm)
						{

						}
						//�ٸ� �ı�
						else if (inputPartIndex == partType::lleg || inputPartIndex == partType::rleg)
						{

						}
						break;
					}
					}
				}

				break;
			}
		}
	}
	bool existPart(int inputPartIndex)
	{
		for (int i = 0; i < entityInfo.parts.size(); i++)
		{
			if (entityInfo.parts[i][partsFlag::index] == inputPartIndex)
			{
				return true;
			}
		}
		return false;
	}
	void updateStatus()
	{
		//rPCB rFCECR SH EV ���� ������Ʈ��

		//sh
		entityInfo.sh = entityDex[entityInfo.entityCode].sh;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.sh += equipment->itemInfo[i].sh;
		}

		//ev
		entityInfo.ev = entityDex[entityInfo.entityCode].ev;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.ev += equipment->itemInfo[i].ev;
		}

		//rFire
		entityInfo.rFire = entityDex[entityInfo.entityCode].rFire;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.rFire += equipment->itemInfo[i].rFire;
		}

		//rCold
		entityInfo.rCold = entityDex[entityInfo.entityCode].rCold;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.rCold += equipment->itemInfo[i].rCold;
		}

		//rElec
		entityInfo.rElec = entityDex[entityInfo.entityCode].rElec;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.rElec += equipment->itemInfo[i].rElec;
		}

		//rCorr
		entityInfo.rCorr = entityDex[entityInfo.entityCode].rCorr;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.rCorr += equipment->itemInfo[i].rCorr;
		}

		//rRad
		entityInfo.rRad = entityDex[entityInfo.entityCode].rRad;//�⺻ ��ü������ �缳��
		for (int i = 0; i < equipment->itemInfo.size(); i++)//��� �������� �� ������Ʈ
		{
			entityInfo.rRad += equipment->itemInfo[i].rRad;
		}
	}
	//�Է��� ���� �ε����� rPierce�� ��ȯ
	int getRPierce(int inputPartIndex)
	{
		int totalRPierce = 0;
		return totalRPierce;
	}
	//�Է��� ���� �ε����� rCut�� ��ȯ
	int getRCut(int inputPartIndex)
	{
		int totalRCut = 0;
		return totalRCut;
	}
	//�Է��� ���� �ε����� rBash�� ��ȯ
	int getRBash(int inputPartIndex)
	{
		int totalRBash = 0;
		return totalRBash;
	}
	//�Է��� ���� �ε����� SH�� ��ȯ
	int getSH()
	{
		int totalSH = 0;

		//�⺻ ��ü�� ���ϱ�
		totalSH += entityInfo.sh;

		for (int i = 0; i < equipment->itemInfo.size(); i++)
		{
			totalSH += equipment->itemInfo[i].sh;
		}
		return totalSH;
	}
	//�Է��� ���� �ε����� EV�� ��ȯ
	int getEV()
	{
		int totalEV = 0;

		//�⺻ ��ü�� ���ϱ�
		totalEV += entityInfo.ev;

		for (int i = 0; i < equipment->itemInfo.size(); i++)
		{
			totalEV += equipment->itemInfo[i].ev;
		}
		return totalEV;
	}
	//�Է��� ���� �ε����� enc�� ��ȯ
	int getEnc(int inputPartIndex)
	{
		int totalEnc = 0;

		for (int i = 0; i < equipment->itemInfo.size(); i++)
		{
			for (int j = 0; j < equipment->itemInfo[i].enc.size(); j++)
			{
				if (equipment->itemInfo[i].enc[j].first == inputPartIndex)
				{
					totalEnc += equipment->itemInfo[i].enc[j].second;
				}
			}

		}
		return totalEnc;
	}
	bool getHasAStarDst() { return hasAStarDst; }
	void deactAStarDst() { hasAStarDst = false; }
	int getAStarDstX()
	{
		errorBox(hasAStarDst == false, "getAStarDstX activated while hasAStarDst is false");
		return aStarDstX;
	}
	int getAStarDstY()
	{
		errorBox(hasAStarDst == false, "getAStarDstY activated while hasAStarDst is false");
		return aStarDstY;
	}
	void setAStarDst(int inputX, int inputY)
	{
		hasAStarDst = true;
		aStarDstX = inputX;
		aStarDstY = inputY;
	}
#pragma endregion
	void move(int dir, bool jump)
	{
		(World::ins())->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;

		int dstX = 16 * getGridX() + 8;
		int dstY = 16 * getGridY() + 8;
		int dGridX, dGridY;
		dir2Coord(dir, dGridX, dGridY);
		dstX += 16 * dGridX;
		dstY += 16 * dGridY;
		int dstGridX = (dstX - 8) / 16;
		int dstGridY = (dstY - 8) / 16;

		(World::ins())->getTile(dstGridX, dstGridY, getGridZ()).EntityPtr = this;

		if (jump == false)
		{
			setAniType(aniFlag::move);
			aniUSet.insert(this);
			setDstGrid(dstGridX, dstGridY);
		}
		else
		{
			setDstGrid(dstGridX, dstGridY);
			setGrid(dstGridX, dstGridY, getGridZ());
		}
	}
	void attack(int gridX, int gridY, int inputPartType)
	{
		Entity* victimEntity = (Entity*)World::ins()->getTile(gridX, gridY, getGridZ()).EntityPtr;
		if (victimEntity == nullptr)
		{
			prt(col::orange, L"[�����] ������ ��������.\n");
		}
		else
		{
			//���߷� ���
			float aimAcc;
			aimAcc = getAimAcc(victimEntity, inputPartType, true);

			if (aimAcc * 100.0 > randomRange(0, 100))
			{
				victimEntity->setFlashType(1);
				victimEntity->addDmg(inputPartType, 9);
			}
			else
			{
				new Damage(L"dodged", victimEntity->getX(), victimEntity->getY() - 8, col::yellow, 9);
				prt(col::orange, L"[�����] ������ ��������.\n");
			}
		}
	}
	void attack(int gridX, int gridY)
	{
		Entity* victimEntity = (Entity*)World::ins()->getTile(gridX, gridY, getGridZ()).EntityPtr;
		int targetPart;
		if (randomRange(1, 10) <= 8) { targetPart = 0; }
		else { targetPart = randomRange(1, victimEntity->entityInfo.parts.size() - 1); }
		attack(gridX, gridY, victimEntity->entityInfo.parts[targetPart][partsFlag::index]);
	}
	void updateWalkable(int gridX, int gridY)//���� �ٸ� ��� ��ü���� �������̵��ؼ� ���ÿ�
	{
		//���� ���� �� �ִ��ص� �ش� ��ġ�� ��ƼƼ�� �����ϸ� �ȱ�Ұ��� ����
		if (World::ins()->getTile(gridX, gridY, getGridZ()).walkable == true)
		{
			if (World::ins()->getTile(gridX, gridY, getGridZ()).EntityPtr != nullptr)
			{
				World::ins()->getTile(gridX, gridY, getGridZ()).walkable = false;
			}
		}
	}
	void rayCasting(int x1, int y1, int x2, int y2)
	{
		int xo = x1;
		int yo = y1;
		int delx = abs(x2 - x1);
		int dely = abs(y2 - y1);
		int i = 0;
		World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
		if (fabs(1.0 * dely / delx) < 1)
		{
			int p = 2 * dely - delx;
			while (i < delx)
			{
				if (p < 0)
				{
					if (x2 > xo && y2 >= yo) { x1++; }
					else if (x2 > xo && yo > y2) { x1++; }
					else if (xo > x2 && y2 > yo) { x1--; }
					else { x1--; }
					World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * dely);
				}
				else
				{
					if (x2 > xo && y2 >= yo) { x1++; y1++; }
					else if (x2 > xo && yo > y2) { x1++; y1--; }
					else if (xo > x2 && y2 > yo) { x1--; y1++; }
					else { x1--; y1--; }
					World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * dely) - (2 * delx);
				}
				i++;
			}
			return;
		}
		else if (fabs(1.0 * dely / delx) > 1)
		{
			int p = (2 * delx) - dely;
			while (i < dely)
			{
				if (p < 0)
				{
					if (x2 >= xo && y2 > yo) { y1++; }
					else if (x2 > xo && yo > y2) { y1--; }
					else if (xo > x2 && y2 > yo) { y1++; }
					else { y1--; }
					World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * delx);
				}
				else
				{
					if (x2 >= xo && y2 > yo) { x1++; y1++; }
					else if (x2 > xo && yo > y2) { x1++; y1--; }
					else if (xo > x2 && y2 > yo) { x1--; y1++; }
					else { x1--; y1--; }
					World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * delx) - (2 * dely);
				}
				i++;
			}
		}
		else
		{
			while (i < delx)
			{
				if (x2 > x1 && y2 > y1) { x1++; y1++; }
				else if (x2 > x1 && y1 > y2) { x1++; y1--; }
				else if (x1 > x2 && y2 > y1) { x1--; y1++; }
				else { x1--; y1--; }
				World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
				if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
				i++;
			}
		}
	}
	void rayCastingDark(int x1, int y1, int x2, int y2)
	{
		int xo = x1;
		int yo = y1;
		int delx = abs(x2 - x1);
		int dely = abs(y2 - y1);
		int i = 0;
		World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
		if (fabs(1.0 * dely / delx) < 1)
		{
			int p = 2 * dely - delx;
			while (i < delx)
			{
				if (p < 0)
				{
					if (x2 > xo && y2 >= yo) { x1++; }
					else if (x2 > xo && yo > y2) { x1++; }
					else if (xo > x2 && y2 > yo) { x1--; }
					else { x1--; }
					if (World::ins()->getTile(x1, y1, getGridZ()).light > 0)
					{
						World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					}
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * dely);
				}
				else
				{
					if (x2 > xo && y2 >= yo) { x1++; y1++; }
					else if (x2 > xo && yo > y2) { x1++; y1--; }
					else if (xo > x2 && y2 > yo) { x1--; y1++; }
					else { x1--; y1--; }
					if (World::ins()->getTile(x1, y1, getGridZ()).light > 0)
					{
						World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					}
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * dely) - (2 * delx);
				}
				i++;
			}
			return;
		}
		else if (fabs(1.0 * dely / delx) > 1)
		{
			int p = (2 * delx) - dely;
			while (i < dely)
			{
				if (p < 0)
				{
					if (x2 >= xo && y2 > yo) { y1++; }
					else if (x2 > xo && yo > y2) { y1--; }
					else if (xo > x2 && y2 > yo) { y1++; }
					else { y1--; }
					if (World::ins()->getTile(x1, y1, getGridZ()).light > 0)
					{
						World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					}
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * delx);
				}
				else
				{
					if (x2 >= xo && y2 > yo) { x1++; y1++; }
					else if (x2 > xo && yo > y2) { x1++; y1--; }
					else if (xo > x2 && y2 > yo) { x1--; y1++; }
					else { x1--; y1--; }
					if (World::ins()->getTile(x1, y1, getGridZ()).light > 0)
					{
						World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
					}
					if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
					p = p + (2 * delx) - (2 * dely);
				}
				i++;
			}
		}
		else
		{
			while (i < delx)
			{
				if (x2 > x1 && y2 > y1) { x1++; y1++; }
				else if (x2 > x1 && y1 > y2) { x1++; y1--; }
				else if (x1 > x2 && y2 > y1) { x1--; y1++; }
				else { x1--; y1--; }
				if (World::ins()->getTile(x1, y1, getGridZ()).light > 0)
				{
					World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
				}
				if (World::ins()->getTile(x1, y1, getGridZ()).blocker) { return; }
				i++;
			}
		}
	}
	void stepEvent()
	{
	}
	void startFlash(int inputFlashType)
	{
		flashType = inputFlashType;
	}
	void setFlashRGBA(Uint8 inputR, Uint8 inputG, Uint8 inputB, Uint8 inputAlpha)
	{
		flash = { inputR, inputG, inputB, inputAlpha };
		SDL_SetTextureColorMod(spriteFlash->getTexture(), inputR, inputG, inputB);
		SDL_SetTextureAlphaMod(spriteFlash->getTexture(), inputAlpha);
		SDL_SetTextureBlendMode(spriteFlash->getTexture(), SDL_BLENDMODE_BLEND);
	}
	void getFlashRGBA(Uint8& targetR, Uint8& targetG, Uint8& targetB, Uint8& targetAlpha)
	{
		if (targetR != NULL) { targetR = flash.r; }
		if (targetG != NULL) { targetG = flash.g; }
		if (targetB != NULL) { targetB = flash.b; }
		if (targetAlpha != NULL) { targetAlpha = flash.a; }
	}
	std::array<int, pocketFlagWidth> getPocketInfo(int vectorIndex) { return pocketInfo[vectorIndex]; }
	int getPocketInfo(int vectorIndex, int arrayIndex) { return pocketInfo[vectorIndex][arrayIndex]; }
	void setPocketInfo(int vectorIndex, int arrayIndex, int val) { pocketInfo[vectorIndex][arrayIndex] = val; }
	int getPocketInfoSize() { return pocketInfo.size(); }
	void drop(ItemPocket* txPtr)
	{
		ItemStack* targetStack;
		//������ ������ �̹� �ִ� ���� ���� ���
		if (World::ins()->getItemPos(getGridX(), getGridY(), getGridZ()) == nullptr) //�� �ڸ��� �� ���� ���
		{
			//���� ������ ������ ���� ����� �� ptr�� ����
			targetStack = new ItemStack(getGridX(), getGridY(), getGridZ());
			targetStack->setPocket(txPtr);//���丮�� ��ü(�޸𸮴� �޼ҵ� ���ο��� ������)
			targetStack->updateSprIndex();
		}
		else //�̹� �� �ڸ��� �������� �ִ� ���
		{
			//���� ������ ������ �� ������ �״�� ����
			targetStack = World::ins()->getItemPos(getGridX(), getGridY(), getGridZ());

			targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

			for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
			}
			delete txPtr; //�����ʰ� ������ �Ű����� drop�� �������ش�.
		}
		aniUSet.insert(targetStack);
		turnCycle = turn::playerAnime;

		targetStack->setAniType(aniFlag::drop);
	}
	void throwing(ItemPocket* txPtr, int gridX, int gridY)
	{
		ItemStack* targetStack;
		//������ ������ �̹� �ִ� ���� ���� ���
		if (World::ins()->getItemPos(gridX, gridY, getGridZ()) == nullptr) //�� �ڸ��� �� ���� ���
		{
			//���� ������ ������ ���� ����� �� ptr�� ����
			targetStack = new ItemStack(gridX, gridY, getGridZ());
			targetStack->setPocket(txPtr);//���丮�� ��ü(�޸𸮴� �޼ҵ� ���ο��� ������)
			targetStack->updateSprIndex();
		}
		else //�̹� �� �ڸ��� �������� �ִ� ���
		{
			//���� ������ ������ �� ������ �״�� ����
			targetStack = World::ins()->getItemPos(gridX, gridY, getGridZ());

			targetStack->setTargetSprIndex(targetStack->getSprIndex()); //���� ��ġ�� ��¥ ������ �̹���
			targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

			for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
			}
			delete txPtr; //�����ʰ� ������ �Ű����� drop�� �������ش�.
		}
		aniUSet.insert(targetStack);
		turnCycle = turn::playerAnime;

		targetStack->setAniType(aniFlag::throwing);
		targetStack->setFakeX(getX() - targetStack->getX());
		targetStack->setFakeY(getY() - targetStack->getY());
	}

	float getTalentApt(int index) { return talentApt[index]; }
	int getSKillExp(int index) { return talentExp[index]; }
	int getTalentFocus(int index) { return talentFocus[index]; }
	//@brief ����ġ ���̺�� �������� �����Ͽ� �Է��� index�� ��ɷ����� ��ȯ��
	float getTalentLevel(int index)
	{
		float resultLevel = 0;
		for (int i = 18; i >= 0; i--)
		{
			if (talentExp[index] >= expTable[i] / talentApt[index])
			{
				resultLevel = i + 1;
				if (i == 18) { return resultLevel; } //������ 18�̻��� ��� 
				else
				{
					resultLevel += (float)(talentExp[index] - expTable[i] / talentApt[index]) / (float)(expTable[i + 1] / talentApt[index] - expTable[i] / talentApt[index]);
					return resultLevel;
				}
			}
		}
		resultLevel += (float)(talentExp[index]) / (float)(expTable[1] / talentApt[index] - expTable[0] / talentApt[index]);
		return resultLevel;
	}
	void addTalentExp(int expVal)
	{
		int divider = 0;
		for (int i = 0; i < talentSize; i++)
		{
			divider += talentFocus[i];
		}
		errorBox(divider == 0, "You need to enable at least one talent(divider=0 at addTalentExp).");
		int frag = floor((float)(expVal) / (float)divider);
		for (int i = 0; i < talentSize; i++)
		{
			talentExp[i] += (frag * talentFocus[i]);
		}
		//������ �� ����� ��Ŀ�� ����
		for (int i = 0; i < talentSize; i++)
		{
			if (talentFocus[i] > 0)
			{
				if (getTalentLevel(i) >= 18)
				{
					talentFocus[i] = 0;
				}
			}
		}
	}

	void setTalentApt(int index, float val) { talentApt[index] = val; }
	void setTalentExp(int index, int val) { talentExp[index] = val; }
	void setTalentFocus(int index, int val) { talentFocus[index] = val; }

	bool runAnimation(bool shutdown)
	{
		if (getAniType() == aniFlag::move)//���� �÷��̾� �ν��Ͻ��� ��ǥ�� ������ǥ�� �ٸ� ���
		{
			// 1 / 60�ʸ��� runAnimation�� �����
			const char speed = 4;
			addTimer();

			if (getX() > getDstX()) { setXY(getX() - speed, getY()); }
			else if (getX() < getDstX()) { setXY(getX() + speed, getY()); }
			if (getY() > getDstY()) { setXY(getX(), getY() - speed); }
			else if (getY() < getDstY()) { setXY(getX(), getY() + speed); }

			switch (getTimer())
			{
			case 2:
				if (getLeftFoot() == true)
				{
					setSpriteIndex(getSpriteInfimum() + 1);
					setLeftFoot(false);
				}
				else
				{
					setSpriteIndex(getSpriteInfimum() + 2);
					setLeftFoot(true);
				}
				break;
			case 4://����
				setSpriteIndex(getSpriteInfimum() + 0);
				resetTimer();
				setAniType(aniFlag::null);
				setGrid(getDstGridX(), getDstGridY(), getGridZ());
				turnWait(1.0);
				return true;
				break;

			}
		}
		else if (getAniType() == aniFlag::atk)
		{
			addTimer();

			char dx;
			char dy;

			switch (getDirection())
			{
			case 0: dx = 1; dy = 0; break;
			case 1: dx = 1; dy = -1; break;
			case 2: dx = 0; dy = -1; break;
			case 3: dx = -1; dy = -1; break;
			case 4: dx = -1; dy = 0; break;
			case 5: dx = -1; dy = 1; break;
			case 6: dx = 0; dy = 1; break;
			case 7: dx = 1; dy = 1; break;
			}

			Entity* address = (Entity*)World::ins()->getTile(atkTargetGridX, atkTargetGridY, atkTargetGridZ).EntityPtr;
			std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

			if (shutdown == true)//������� ���� ��������
			{
				if (address != nullptr)
				{
					address->setFlashRGBA(0, 0, 0, 0);
				}
				aniUSet.erase(aniUSet.find(this));
				delete(((Sticker*)(StickerList.find(stickerID))->second));
				setFakeX(0);
				setFakeY(0);
				return true;
			}

			switch (getTimer())
			{
			case 2:
				setFakeX(getFakeX() + 3 * dx);
				setFakeY(getFakeY() + 3 * dy);
				break;
			case 3:
				setFakeX(getFakeX() + 2 * dx);
				setFakeY(getFakeY() + 2 * dy);
				break;
			case 4:
				setFakeX(getFakeX() + 1 * dx);
				setFakeY(getFakeY() + 1 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() + 2 * dx);
					address->setFakeY(address->getFakeY() + 2 * dy);
				}

				if (atkTargetPart == -1) { attack(atkTargetGridX, atkTargetGridY); }
				else { attack(atkTargetGridX, atkTargetGridY, atkTargetPart); }
				new Sticker(false, getX() + (16 * (atkTargetGridX - getGridX())), getY() + (16 * (atkTargetGridY - getGridY())), spr::effectCut1, 0, stickerID, true);
				break;
			case 5:
				setFakeX(getFakeX() - 1 * dx);
				setFakeY(getFakeY() - 1 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() - 1 * dx);
					address->setFakeY(address->getFakeY() - 1 * dy);
				}
				break;
			case 6:
				setFakeX(getFakeX() - 2 * dx);
				setFakeY(getFakeY() - 2 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() - 1 * dx);
					address->setFakeY(address->getFakeY() - 1 * dy);
				}
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
				break;
			case 7:
				setFakeX(getFakeX() - 3 * dx);
				setFakeY(getFakeY() - 3 * dy);
				break;
			case 8:
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
				break;
			case 10:
				delete(((Sticker*)(StickerList.find(stickerID))->second));
			case 20:
				setFakeX(0);
				setFakeY(0);
				resetTimer();
				setAniType(aniFlag::null);
				if (isPlayer == true) { turnWait(endAtk()); }
				else { endAtk(); }
				return true;
			}

			if (getTimer() >= 5)
			{
				Entity* address = (Entity*)World::ins()->getTile(atkTargetGridX, atkTargetGridY, atkTargetGridZ).EntityPtr;
				if (address != nullptr)
				{
					Uint8 targetR, targetG, targetB, targetAlpha;
					address->getFlashRGBA(targetR, targetG, targetB, targetAlpha);
					if (address->getFlashType() == 1)
					{
						if (targetG > 51) { targetG -= 51; }
						else { targetG = 0; }
						if (targetB > 51) { targetB -= 51; }
						else { targetB = 0; }
						if (targetAlpha > 51) { targetAlpha -= 51; }
						else { targetAlpha = 0; }
						address->setFlashRGBA(targetR, targetG, targetB, targetAlpha);
					}
				}
			}
		}
		else if (getAniType() == aniFlag::throwing)
		{
			//�Ÿ��� ���� ���� �ǰ��ϴµ��� �ɸ��� �ð��� ��� ���� ��
			addTimer();

			char dx;
			char dy;

			switch (getDirection())
			{
			case 0: dx = 1; dy = 0; break;
			case 1: dx = 1; dy = -1; break;
			case 2: dx = 0; dy = -1; break;
			case 3: dx = -1; dy = -1; break;
			case 4: dx = -1; dy = 0; break;
			case 5: dx = -1; dy = 1; break;
			case 6: dx = 0; dy = 1; break;
			case 7: dx = 1; dy = 1; break;
			}

			Entity* address = (Entity*)World::ins()->getTile(atkTargetGridX, atkTargetGridY, atkTargetGridZ).EntityPtr;
			std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

			if (shutdown == true)//������� ���� ��������
			{
				if (address != nullptr)
				{
					address->setFlashRGBA(0, 0, 0, 0);
				}
				aniUSet.erase(aniUSet.find(this));
				delete(((Sticker*)(StickerList.find(stickerID))->second));
				setFakeX(0);
				setFakeY(0);
				return true;
			}

			switch (getTimer())
			{
			case 2:
				setFakeX(getFakeX() + 3 * dx);
				setFakeY(getFakeY() + 3 * dy);
				break;
			case 3:
				setFakeX(getFakeX() + 2 * dx);
				setFakeY(getFakeY() + 2 * dy);
				break;
			case 4:
				setFakeX(getFakeX() + 1 * dx);
				setFakeY(getFakeY() + 1 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() + 2 * dx);
					address->setFakeY(address->getFakeY() + 2 * dy);
				}

				if (atkTargetPart == -1) { attack(atkTargetGridX, atkTargetGridY); }
				else { attack(atkTargetGridX, atkTargetGridY, atkTargetPart); }
				new Sticker(false, getX() + (16 * (atkTargetGridX - getGridX())), getY() + (16 * (atkTargetGridY - getGridY())), spr::effectCut1, 0, stickerID, true);
				break;
			case 5:
				setFakeX(getFakeX() - 1 * dx);
				setFakeY(getFakeY() - 1 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() - 1 * dx);
					address->setFakeY(address->getFakeY() - 1 * dy);
				}
				break;
			case 6:
				setFakeX(getFakeX() - 2 * dx);
				setFakeY(getFakeY() - 2 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() - 1 * dx);
					address->setFakeY(address->getFakeY() - 1 * dy);
				}
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
				break;
			case 7:
				setFakeX(getFakeX() - 3 * dx);
				setFakeY(getFakeY() - 3 * dy);
				break;
			case 8:
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
				break;
			case 10:
				delete(((Sticker*)(StickerList.find(stickerID))->second));
			case 20:
				setFakeX(0);
				setFakeY(0);
				resetTimer();
				setAniType(aniFlag::null);
				if (isPlayer == true) { turnWait(endAtk()); }
				else { endAtk(); }
				return true;
			}

			if (getTimer() >= 5)
			{
				Entity* address = (Entity*)World::ins()->getTile(atkTargetGridX, atkTargetGridY, atkTargetGridZ).EntityPtr;
				if (address != nullptr)
				{
					Uint8 targetR, targetG, targetB, targetAlpha;
					address->getFlashRGBA(targetR, targetG, targetB, targetAlpha);
					if (address->getFlashType() == 1)
					{
						if (targetG > 51) { targetG -= 51; }
						else { targetG = 0; }
						if (targetB > 51) { targetB -= 51; }
						else { targetB = 0; }
						if (targetAlpha > 51) { targetAlpha -= 51; }
						else { targetAlpha = 0; }
						address->setFlashRGBA(targetR, targetG, targetB, targetAlpha);
					}
				}
			}
		}
		else if (getAniType() == aniFlag::shotSingle)
		{
			//�Ÿ��� ���� ���� �ǰ��ϴµ��� �ɸ��� �ð��� ��� ���� ��
			addTimer();

			char dx;
			char dy;

			switch (getDirection())
			{
			case 0: dx = 1; dy = 0; break;
			case 1: dx = 1; dy = -1; break;
			case 2: dx = 0; dy = -1; break;
			case 3: dx = -1; dy = -1; break;
			case 4: dx = -1; dy = 0; break;
			case 5: dx = -1; dy = 1; break;
			case 6: dx = 0; dy = 1; break;
			case 7: dx = 1; dy = 1; break;
			}

			Entity* address = (Entity*)World::ins()->getTile(atkTargetGridX, atkTargetGridY, atkTargetGridZ).EntityPtr;
			std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

			if (shutdown == true)//������� ���� ��������
			{
				if (address != nullptr)
				{
					address->setFlashRGBA(0, 0, 0, 0);
				}
				aniUSet.erase(aniUSet.find(this));
				delete(((Sticker*)(StickerList.find(stickerID))->second));
				setFakeX(0);
				setFakeY(0);
				return true;
			}

			switch (getTimer())
			{
			case 2:
				setFakeX(getFakeX() - 3 * dx);
				setFakeY(getFakeY() - 3 * dy);
				break;
			case 3:
				setFakeX(getFakeX() - 2 * dx);
				setFakeY(getFakeY() - 2 * dy);
				break;
			case 4:
				setFakeX(getFakeX() - 1 * dx);
				setFakeY(getFakeY() - 1 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() + 2 * dx);
					address->setFakeY(address->getFakeY() + 2 * dy);
				}

				if (atkTargetPart == -1)
				{
					//���߷� ��� �����
					attack(atkTargetGridX, atkTargetGridY);
				}
				else
				{
					attack(atkTargetGridX, atkTargetGridY, atkTargetPart);
				}
				new Sticker(false, getX() + (16 * (atkTargetGridX - getGridX())), getY() + (16 * (atkTargetGridY - getGridY())), spr::effectCut1, 0, stickerID, true);
				break;
			case 5:
				setFakeX(getFakeX() + 1 * dx);
				setFakeY(getFakeY() + 1 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() - 1 * dx);
					address->setFakeY(address->getFakeY() - 1 * dy);
				}
				break;
			case 6:
				setFakeX(getFakeX() + 2 * dx);
				setFakeY(getFakeY() + 2 * dy);
				if (address != nullptr)
				{
					address->setFakeX(address->getFakeX() - 1 * dx);
					address->setFakeY(address->getFakeY() - 1 * dy);
				}
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
				break;
			case 7:
				setFakeX(getFakeX() + 3 * dx);
				setFakeY(getFakeY() + 3 * dy);
				break;
			case 8:
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
				break;
			case 10:
				delete(((Sticker*)(StickerList.find(stickerID))->second));
			case 20:
				setFakeX(0);
				setFakeY(0);
				resetTimer();
				setAniType(aniFlag::null);
				if (isPlayer == true) { turnWait(endAtk()); }
				else { endAtk(); }
				return true;
			}

			if (getTimer() >= 5)
			{
				Entity* address = (Entity*)World::ins()->getTile(atkTargetGridX, atkTargetGridY, atkTargetGridZ).EntityPtr;
				if (address != nullptr)
				{
					Uint8 targetR, targetG, targetB, targetAlpha;
					address->getFlashRGBA(targetR, targetG, targetB, targetAlpha);
					if (address->getFlashType() == 1)
					{
						if (targetG > 51) { targetG -= 51; }
						else { targetG = 0; }
						if (targetB > 51) { targetB -= 51; }
						else { targetB = 0; }
						if (targetAlpha > 51) { targetAlpha -= 51; }
						else { targetAlpha = 0; }
						address->setFlashRGBA(targetR, targetG, targetB, targetAlpha);
					}
				}
			}
		}


		return false;
	}

	//�޼ҵ带 ������ ��ü�� ���̰� �������� ����Ѵ�.

	virtual void death() = 0;

	//���� ��ü�� ������ ��� ������ ���� ���·� ��ȯ�Ѵ�.
	std::vector<int> getAllParts()
	{
		std::vector<int> partsVec;

		for (int i = 0; i < entityInfo.parts.size(); i++)
		{
			partsVec.push_back(entityInfo.parts[i][partsFlag::index]);
		}

		return partsVec;
	}

	void aimWeaponRight() { aimWeaponHand = equip::right; }
	void aimWeaponLeft() { aimWeaponHand = equip::left; }
	int getAimHand()
	{
		if (aimWeaponHand == equip::right) { return equip::right; }
		else { return equip::left; }
	}

	//�� ��ü�� �ش� ��ü�� �������� �� ������ ����� ��� ������ ���� ���߷��� Ȯ��(0~1.0)���� ��ȯ����, aim�� false�� aimStack�� 0�� ��� 
	float getAimAcc(Entity* victimEntity, int inputPartType, bool aim)
	{
		float aimAcc;
		int victimTargetPartIndex = 0;
		int distance = myMax(abs(getGridX() - victimEntity->getGridX()), abs(getGridY() - victimEntity->getGridY()));

		unsigned __int8 aimStack = 0;
		if (aim == true) { aimStack = getAimStack(); }
		else { aimStack = 0; }

		//�Է��� ������ ����(��,�ٸ�)�� ������ ��ü�� ���������� index(�ܼ�����)�� ��ȯ����
		for (int i = 0; i < victimEntity->entityInfo.parts.size(); i++)
		{
			if (victimEntity->entityInfo.parts[i][partsFlag::index] == inputPartType)
			{
				victimTargetPartIndex = i;
			}
		}

		if (getAimWeaponIndex() == -1)//�Ǽհ���
		{
			return aimAcc = calcMelee::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, 0.9, 5, 13.5, 13.5, aimStack);
		}
		else//�������
		{
			switch (getNextAtkType())
			{
			case atkType::pierce:
				return aimAcc = calcMelee::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, getEquipPtr()->itemInfo[getAimWeaponIndex()].meleeAtkAcc, 5, 13.5, 13.5, aimStack);
			case atkType::cut:
				return aimAcc = calcMelee::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, getEquipPtr()->itemInfo[getAimWeaponIndex()].meleeAtkAcc, 5, 13.5, 13.5, aimStack);
			case atkType::bash:
				return aimAcc = calcMelee::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, getEquipPtr()->itemInfo[getAimWeaponIndex()].meleeAtkAcc, 5, 13.5, 13.5, aimStack);
			case atkType::shot:
				return aimAcc = calcShot::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, getEquipPtr()->itemInfo[getAimWeaponIndex()].gunAccInit, 5, 13.5, aimStack, distance);
			case atkType::throwing:
				return aimAcc = calcThrow::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, getEquipPtr()->itemInfo[getAimWeaponIndex()].throwAtkAcc, 5, 13.5, aimStack, distance);
			default:
				errorBox("Unknown attack type in getAimAcc(Entity.ixx)");
				break;
			}
		}
	}

	int getAimWeaponIndex()
	{
		//���� �÷��̾ ������ �ܴ��� ������ �ε����� ��ȯ��(-1�̸� �Ǽ�)
		int targetHand;
		std::vector<ItemData> equipInfo = getEquipPtr()->itemInfo;
		if (getAimHand() == equip::left) { targetHand = equip::left; }
		else { targetHand = equip::right; }


		if (equipInfo.size() == 0)
		{
			return -1;
		}
		else
		{
			for (int i = 0; i < equipInfo.size(); i++)
			{
				if (equipInfo[i].equipState == targetHand)
				{
					return i;
					break;
				}
				else if (equipInfo[i].equipState == equip::both)
				{
					return i;
					break;
				}

				if (i == equipInfo.size() - 1)
				{
					return -1;
					break;
				}
			}
		}
	}

	virtual void drawSelf() override
	{
		stepEvent();
		setZoom(zoomScale);
		setFlip(getEntityFlip());

		//prt(L"���� �� ��ü�� fake ��ǥ�� %f,%f�̴�.\n", getFloatFakeX(), getFloatFakeY());
		int drawingX = (cameraW / 2) + zoomScale * (getX() - cameraX + getFakeX());
		int drawingY = (cameraH / 2) + zoomScale * (getY() - cameraY + getFakeY());

		//ĳ���� �׸��� �׸���
		drawSpriteCenter(spr::shadow, getSpriteIndex(), drawingX, drawingY);

		//ĳ���� ��ü �׸���
		drawSpriteCenter(getSprite(), getSpriteIndex(), drawingX, drawingY);


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//ĳ���� Ŀ��Ÿ����¡ �׸���
		if (getSkin() != humanCustom::skin::null)
		{
			if (getSkin() == humanCustom::skin::yellow) drawSpriteCenter(spr::skinYellow, getSpriteIndex(), drawingX, drawingY);
		}

		if (getEyes() != humanCustom::eyes::null)
		{
			if (getEyes() == humanCustom::eyes::blue) drawSpriteCenter(spr::eyesBlue, getSpriteIndex(), drawingX, drawingY);
			else if (getEyes() == humanCustom::eyes::red) drawSpriteCenter(spr::eyesRed, getSpriteIndex(), drawingX, drawingY);
		}

		if (getScar() != humanCustom::scar::null)
		{
		}

		if (getBeard() != humanCustom::beard::null)
		{
			if (getBeard() == humanCustom::beard::mustache) drawSpriteCenter(spr::beardMustacheBlack, getSpriteIndex(), drawingX, drawingY);
		}

		if (getHair() != humanCustom::hair::null)
		{
			switch (getHair())
			{
			case humanCustom::hair::commaBlack:
				drawSpriteCenter(spr::hairCommaBlack, getSpriteIndex(), drawingX, drawingY);
				break;
			case humanCustom::hair::bob1Black:
				drawSpriteCenter(spr::hairBob1Black, getSpriteIndex(), drawingX, drawingY);
				break;
			case humanCustom::hair::ponytail:
				drawSpriteCenter(spr::hairPonytailBlack, getSpriteIndex(), drawingX, drawingY);
				break;
			case humanCustom::hair::middlePart:
				drawSpriteCenter(spr::hairMiddlePart, getSpriteIndex(), drawingX, drawingY);
				break;
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//ĳ���� ��� �׸���
		if (getEquipPtr()->itemInfo.size() > 0)
		{
			std::map<int, Sprite*, std::less<int>> drawOrder;

			for (int equipCounter = 0; equipCounter < getEquipPtr()->itemInfo.size(); equipCounter++)
			{
				int priority = 0;
				Sprite* tgtSpr = nullptr;
				switch (getEquipPtr()->itemInfo[equipCounter].equipState)
				{
				case equip::left:
				case equip::both:
					priority = getEquipPtr()->itemInfo[equipCounter].leftWieldPriority;
					tgtSpr = (Sprite*)getEquipPtr()->itemInfo[equipCounter].leftWieldSpr;
					break;
				case equip::right:
					priority = getEquipPtr()->itemInfo[equipCounter].rightWieldPriority;
					tgtSpr = (Sprite*)getEquipPtr()->itemInfo[equipCounter].rightWieldSpr;
					break;
				case equip::normal:
					priority = getEquipPtr()->itemInfo[equipCounter].equipPriority;
					tgtSpr = (Sprite*)getEquipPtr()->itemInfo[equipCounter].equipSpr;
					break;
				default:
					errorBox(L"��� �׸��� �߿� equipState�� ���������� ���� ��� �߰�");
					break;
				}
				errorBox(drawOrder.find(priority) != drawOrder.end(), L"�̹� �����ϴ� �켱���� ��� �߰��� :" + std::to_wstring(priority) + L" �̸�: " + getEquipPtr()->itemInfo[equipCounter].name);
				drawOrder[priority] = tgtSpr;
			}

			for (auto it = drawOrder.begin(); it != drawOrder.end(); it++)
			{
				if (it->second != nullptr)
				{
					drawSpriteCenter(it->second, getSpriteIndex(), drawingX, drawingY);
				}
			}

		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (entityInfo.HP != entityInfo.maxHP)//��ü HP ǥ��
		{
			SDL_Rect dst = { drawingX - (int)(8 * zoomScale), drawingY + (int)(3 * zoomScale), (int)(16 * zoomScale),(int)(3 * zoomScale) };
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(renderer, col::black.r, col::black.g, col::black.b, 255);
			SDL_RenderFillRect(renderer, &dst);

			float ratioFakeHP = myMax((float)0.0, (entityInfo.fakeHP) / (float)(entityInfo.maxHP));
			dst = { drawingX - (int)(7 * zoomScale), drawingY + (int)(4 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(renderer, col::white.r, col::white.g, col::white.b, 255);
			SDL_RenderFillRect(renderer, &dst);

			float ratioHP = myMax((float)0.0, (float)(entityInfo.HP) / (float)(entityInfo.maxHP));
			dst = { drawingX - (int)(7 * zoomScale), drawingY + (int)(4 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
			if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			if (1/*address == Player::ins()*/)
			{
				SDL_SetRenderDrawColor(renderer, lowCol::green.r, lowCol::green.g, lowCol::green.b, 255);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, lowCol::red.r, lowCol::red.g, lowCol::red.b, 255);
			}
			SDL_RenderFillRect(renderer, &dst);
		}

		if (getFlashType() != NULL) //��ƼƼ�� �÷��� ȿ���� ���� ���
		{
			drawSpriteCenter
			(
				getSpriteFlash(),
				getSpriteIndex(),
				(cameraW / 2) + zoomScale * (getX() - cameraX + getFakeX()),
				(cameraH / 2) + zoomScale * (getY() - cameraY + getFakeY())
			);
		}
		setZoom(1.0);
		setFlip(SDL_FLIP_NONE);
	};
};
