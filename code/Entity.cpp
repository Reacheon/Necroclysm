#include <SDL.h>
#include <SDL_image.h>

import Entity;
import std;
import globalVar;
import constVar;
import textureVar;
import log;
import Sprite;
import Ani;
import constVar;
import Coord;
import World;
import Sticker;
import ItemStack;
import EntityData;
import ItemData;
import Damage;
import util;
import Drawable;
import drawSprite;
import SkillData;
import Flame;
import Vehicle;


Entity::Entity(int newEntityIndex, int gridX, int gridY, int gridZ)//������
{
	prt(L"Entity : �����ڰ� ȣ��Ǿ����ϴ�!\n");
	loadDataFromDex(newEntityIndex);
	setAniPriority(1);
	setGrid(gridX, gridY, gridZ);
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = this;
	entityInfo.equipment = new ItemPocket(storageType::equip);
	entityInfo.talentFocus[0] = 1;
	for (int i = 0; i < TALENT_SIZE; i++) entityInfo.talentApt[i] = 2.0;
}
Entity::~Entity()//�Ҹ���
{
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;

	//���߿� �ٴ��� ���� �� �ִ� Ÿ������ �ƴ����� üũ�Ͽ� true�� �Ǵ����� ���θ� �����ϴ� ���ǹ� �߰��Ұ�
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable = true;
	prt(L"Entity : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
	//delete sprite;
	delete spriteFlash;
	delete entityInfo.equipment;
}
#pragma region getset method

void Entity::setSkillTarget(int gridX, int gridY, int gridZ)
{
	skillTarget.x = gridX;
	skillTarget.y = gridY;
	skillTarget.z = gridZ;
}
Point3 Entity::getSkillTarget() { return skillTarget; }
void Entity::addSkill(int index)
{
	prt(L"��ų %ls�� �߰��ߴ�.\n", skillDex[index].name.c_str());
	entityInfo.skillList.insert(index);
}
humanCustom::skin Entity::getSkin() { return entityInfo.skin; }
void Entity::setSkin(humanCustom::skin input) { entityInfo.skin = input; }
humanCustom::eyes Entity::getEyes() { return entityInfo.eyes; }
void Entity::setEyes(humanCustom::eyes input) { entityInfo.eyes = input; }
humanCustom::scar Entity::getScar() { return entityInfo.scar; }
void Entity::setScar(humanCustom::scar input) { entityInfo.scar = input; }
humanCustom::beard Entity::getBeard() { return entityInfo.beard; }
void Entity::setBeard(humanCustom::beard input) { entityInfo.beard = input; }
humanCustom::hair Entity::getHair() { return entityInfo.hair; }
void Entity::setHair(humanCustom::hair input) { entityInfo.hair = input; }
unsigned __int8 Entity::getAimStack() { return aimStack; }
void Entity::initAimStack() { aimStack = 0; }
void Entity::addAimStack() { aimStack++; }
void Entity::setNextAtkType(atkType inputAtkType) { nextAtkType = inputAtkType; }
atkType Entity::getNextAtkType() { return nextAtkType; }
void Entity::setAtkTarget(int inputX, int inputY, int inputZ, int inputPart)
{
	atkTarget.x = inputX;
	atkTarget.y = inputY;
	atkTarget.z = inputZ;
	atkTargetPart = inputPart;
}
void Entity::setAtkTarget(int inputX, int inputY, int inputZ)
{
	setAtkTarget(inputX, inputY, inputZ, -1);
}
ItemPocket* Entity::getEquipPtr() 
{ 
	return entityInfo.equipment; 
}
Sprite* Entity::getSpriteFlash() 
{ 
	if (spriteFlash == nullptr)
	{
		int textureW, textureH;
		if(customSprite == nullptr) SDL_QueryTexture(entityInfo.entitySpr->getTexture(), NULL, NULL, &textureW, &textureH);
		else  SDL_QueryTexture(customSprite->getTexture(), NULL, NULL, &textureW, &textureH);

		SDL_Texture* drawingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, textureW, textureH);
		SDL_SetRenderTarget(renderer, drawingTexture);
		SDL_SetTextureBlendMode(drawingTexture, SDL_BLENDMODE_BLEND);
		SDL_Rect src = { 0, 0, textureW, textureH };
		SDL_Rect dst = src;
		//�Ͼ�� ���� �ؽ��ĸ� �׷�����

		if (customSprite == nullptr) SDL_RenderCopy(renderer, entityInfo.entitySpr->getTexture(), &src, &dst);
		else SDL_RenderCopy(renderer, customSprite->getTexture(), &src, &dst);
		
		//�ؽ��Ŀ� ������� ���� ������ ����� �Ͼ�� ����
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_Rect dstWhite = { 0, 0, textureW, textureH };
		drawFillRect(dstWhite, col::white);
		SDL_SetRenderTarget(renderer, nullptr);
		if (spriteFlash != nullptr) { delete spriteFlash; }
		spriteFlash = new Sprite(renderer, drawingTexture, 48, 48);
	}

	return spriteFlash; 
}
void Entity::setFlashType(int inputType)
{
	flashType = inputType;
	if (inputType == 1) setFlashRGBA(255, 255, 255, 255);
	else setFlashRGBA(0, 0, 0, 0);
}
int Entity::getFlashType() { return flashType; }
bool Entity::getLeftFoot() { return leftFoot; }
void Entity::setLeftFoot(bool input) { leftFoot = input; }
void Entity::setSpriteInfimum(int inputVal) { entityInfo.sprIndexInfimum = inputVal; }
int Entity::getSpriteInfimum() { return entityInfo.sprIndexInfimum; }
void Entity::setSpriteIndex(int index) { entityInfo.sprIndex = index; }
int Entity::getSpriteIndex() { return entityInfo.sprIndex; }
void Entity::setDirection(int dir)
{
	entityInfo.direction = dir;
	if (dir == 2 || dir == 6) {}
	else if (dir == 0 || (dir == 1 || dir == 7)) { entityInfo.sprFlip = false; }
	else { entityInfo.sprFlip = true; }
}
void Entity::startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType)
{
	setDirection(getIntDegree(getGridX(), getGridY(), inputGridX, inputGridY));
	setAtkTarget(inputGridX, inputGridY, inputGridZ);
}
float Entity::endAtk()
{
	setAniType(aniFlag::null);
	return 1 / 0.8; //���� ���⿡ ���ݼӵ��� ������
}
void Entity::loadDataFromDex(int index)
{
	entityInfo = entityDex[index];
	entityInfo.HP = entityInfo.maxHP;
	entityInfo.fakeHP = entityInfo.maxHP;
}
//@brief �ش� ������ �������� �߰��ϰ� ���� HP�� �׸�ŭ ���ϴ�.
void Entity::addDmg(int inputDmg)
{
	new Damage(std::to_wstring(inputDmg), col::white, getGridX(), getGridY(), dmgAniFlag::none);
	entityInfo.HP -= inputDmg;
	if (entityInfo.HP <= 0)//HP 0, ���
	{
		death();
		return;
	}

}
void Entity::updateStatus()
{
	//rPCB rFCECR SH EV ���� ������Ʈ��

	//sh
	entityInfo.sh = entityDex[entityInfo.entityCode].sh;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.sh += getEquipPtr()->itemInfo[i].sh;
	}

	//ev
	entityInfo.ev = entityDex[entityInfo.entityCode].ev;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.ev += getEquipPtr()->itemInfo[i].ev;
	}

	//rFire
	entityInfo.rFire = entityDex[entityInfo.entityCode].rFire;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.rFire += getEquipPtr()->itemInfo[i].rFire;
	}

	//rCold
	entityInfo.rCold = entityDex[entityInfo.entityCode].rCold;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.rCold += getEquipPtr()->itemInfo[i].rCold;
	}

	//rElec
	entityInfo.rElec = entityDex[entityInfo.entityCode].rElec;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.rElec += getEquipPtr()->itemInfo[i].rElec;
	}

	//rCorr
	entityInfo.rCorr = entityDex[entityInfo.entityCode].rCorr;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.rCorr += getEquipPtr()->itemInfo[i].rCorr;
	}

	//rRad
	entityInfo.rRad = entityDex[entityInfo.entityCode].rRad;//�⺻ ��ü������ �缳��
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//��� �������� �� ������Ʈ
	{
		entityInfo.rRad += getEquipPtr()->itemInfo[i].rRad;
	}
}
//�Է��� ���� �ε����� rPierce�� ��ȯ
int Entity::getRPierce(int inputPartIndex)
{
	int totalRPierce = 0;
	return totalRPierce;
}
//�Է��� ���� �ε����� rCut�� ��ȯ
int Entity::getRCut(int inputPartIndex)
{
	int totalRCut = 0;
	return totalRCut;
}
//�Է��� ���� �ε����� rBash�� ��ȯ
int Entity::getRBash(int inputPartIndex)
{
	int totalRBash = 0;
	return totalRBash;
}
//�Է��� ���� �ε����� SH�� ��ȯ
int Entity::getSH()
{
	int totalSH = 0;

	//�⺻ ��ü�� ���ϱ�
	totalSH += entityInfo.sh;

	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
	{
		totalSH += getEquipPtr()->itemInfo[i].sh;
	}
	return totalSH;
}
//�Է��� ���� �ε����� EV�� ��ȯ
int Entity::getEV()
{
	int totalEV = 0;

	//�⺻ ��ü�� ���ϱ�
	totalEV += entityInfo.ev;

	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
	{
		totalEV += getEquipPtr()->itemInfo[i].ev;
	}
	return totalEV;
}
//�Է��� ���� �ε����� enc�� ��ȯ
int Entity::getEnc(int inputPartIndex)
{
	int totalEnc = 0;

	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
	{
		for (int j = 0; j < getEquipPtr()->itemInfo[i].enc.size(); j++)
		{
			if (getEquipPtr()->itemInfo[i].enc[j].first == inputPartIndex)
			{
				totalEnc += getEquipPtr()->itemInfo[i].enc[j].second;
			}
		}

	}
	return totalEnc;
}
bool Entity::getHasAStarDst() { return hasAStarDst; }
void Entity::deactAStarDst() { hasAStarDst = false; }
int Entity::getAStarDstX()
{
	errorBox(hasAStarDst == false, "getAStarDstX activated while hasAStarDst is false");
	return aStarDst.x;
}
int Entity::getAStarDstY()
{
	errorBox(hasAStarDst == false, "getAStarDstY activated while hasAStarDst is false");
	return aStarDst.y;
}
void Entity::setAStarDst(int inputX, int inputY)
{
	isPlayerMoving = true;
	hasAStarDst = true;
	aStarDst.x = inputX;
	aStarDst.y = inputY;
}
#pragma endregion
void Entity::move(int dir, bool jump)
{
	int dstX = 16 * getGridX() + 8;
	int dstY = 16 * getGridY() + 8;
	int dGridX, dGridY;
	dir2Coord(dir, dGridX, dGridY);
	dstX += 16 * dGridX;
	dstY += 16 * dGridY;
	int dstGridX = (dstX - 8) / 16;
	int dstGridY = (dstY - 8) / 16;
	
	(World::ins())->getTile(dstGridX, dstGridY, getGridZ()).EntityPtr = this;
	(World::ins())->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;

	if (pulledCart != nullptr)
	{
		(World::ins())->getTile(pulledCart->getGridX(), pulledCart->getGridY(), getGridZ()).VehiclePtr = nullptr;
		(World::ins())->getTile(getGridX(), getGridY(), getGridZ()).VehiclePtr = pulledCart;
	}

	if (jump == false)
	{
		addAniUSet(this, aniFlag::move);
		setDstGrid(dstGridX, dstGridY);
		if (pulledCart != nullptr)
		{
			addAniUSet(pulledCart, aniFlag::move);
			pulledCart->setDstGrid(getGridX(), getGridY());
		}
	}
	else
	{
		setDstGrid(dstGridX, dstGridY);
		setGrid(dstGridX, dstGridY, getGridZ());

		pulledCart->setDstGrid(getGridX(), getGridY());
		pulledCart->setGrid(getGridX(), getGridY(),getGridZ());
	}
}
void Entity::attack(int gridX, int gridY)
{
	Entity* victimEntity = (Entity*)World::ins()->getTile(gridX, gridY, getGridZ()).EntityPtr;
	if (victimEntity == nullptr)
	{
		prt(L"[�����] ������ ��������.\n");
	}
	else
	{
		//���߷� ���
		float aimAcc;
		aimAcc = 0.98;

		if (aimAcc * 100.0 > randomRange(0, 100))
		{
			victimEntity->setFlashType(1);
			victimEntity->addDmg(randomRange(6,10));
		}
		else
		{
			new Damage(L"dodged", col::yellow, victimEntity->getGridX(), victimEntity->getGridY(), dmgAniFlag::dodged);
			prt(L"[�����] ������ ��������.\n");
		}
	}
}
void Entity::updateWalkable(int gridX, int gridY)//���� �ٸ� ��� ��ü���� �������̵��ؼ� ���ÿ�
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
void Entity::rayCasting(int x1, int y1, int x2, int y2)
{
	int xo = x1;
	int yo = y1;
	int delx = abs(x2 - x1);
	int dely = abs(y2 - y1);
	int i = 0;
	World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
	double slope = fabs(1.0 * dely / delx);
	if (slope < 1)
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
	else if (slope > 1)
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
void Entity::rayCastingDark(int x1, int y1, int x2, int y2)
{
	int xo = x1;
	int yo = y1;
	int delx = abs(x2 - x1);
	int dely = abs(y2 - y1);
	int i = 0;
	World::ins()->getTile(x1, y1, getGridZ()).fov = fovFlag::white;
	double slope = fabs(1.0 * dely / delx);
	if (slope < 1)
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
	else if (slope > 1)
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
void Entity::stepEvent()
{
}
void Entity::startFlash(int inputFlashType)
{
	flashType = inputFlashType;
}
void Entity::setFlashRGBA(Uint8 inputR, Uint8 inputG, Uint8 inputB, Uint8 inputAlpha)
{
	flash = { inputR, inputG, inputB, inputAlpha };
	SDL_SetTextureColorMod(getSpriteFlash()->getTexture(), inputR, inputG, inputB);
	SDL_SetTextureAlphaMod(getSpriteFlash()->getTexture(), inputAlpha);
	//SDL_SetTextureBlendMode(getSpriteFlash()->getTexture(), SDL_BLENDMODE_BLEND);
}
void Entity::getFlashRGBA(Uint8& targetR, Uint8& targetG, Uint8& targetB, Uint8& targetAlpha)
{
	if (targetR != NULL) { targetR = flash.r; }
	if (targetG != NULL) { targetG = flash.g; }
	if (targetB != NULL) { targetB = flash.b; }
	if (targetAlpha != NULL) { targetAlpha = flash.a; }
}
void Entity::drop(ItemPocket* txPtr)
{
	ItemStack* targetStack;
	//������ ������ �̹� �ִ� ���� ���� ���

	if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).ItemStackPtr == nullptr) //�� �ڸ��� �� ���� ���
	{
		//���� ������ ������ ���� ����� �� ptr�� ����
		targetStack = new ItemStack(getGridX(), getGridY(), getGridZ());
		targetStack->setPocket(txPtr);//���丮�� ��ü(�޸𸮴� �޼ҵ� ���ο��� ������)
		targetStack->updateSprIndex();
	}
	else //�̹� �� �ڸ��� �������� �ִ� ���
	{
		//���� ������ ������ �� ������ �״�� ����
		targetStack = (ItemStack*)World::ins()->getTile(getGridX(), getGridY(), getGridZ()).ItemStackPtr;

		targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

		for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
		{
			txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
		}
		delete txPtr; //�����ʰ� ������ �Ű����� drop�� �������ش�.
	}

	addAniUSetPlayer(targetStack, aniFlag::drop);
}
void Entity::throwing(ItemPocket* txPtr, int gridX, int gridY)
{
	ItemStack* targetStack;
	//������ ������ �̹� �ִ� ���� ���� ���

	if (World::ins()->getTile(gridX, gridY, getGridZ()).ItemStackPtr == nullptr) //�� �ڸ��� �� ���� ���
	{
		//���� ������ ������ ���� ����� �� ptr�� ����
		targetStack = new ItemStack(gridX, gridY, getGridZ());
		targetStack->setPocket(txPtr);//���丮�� ��ü(�޸𸮴� �޼ҵ� ���ο��� ������)
		targetStack->updateSprIndex();
	}
	else //�̹� �� �ڸ��� �������� �ִ� ���
	{
		//���� ������ ������ �� ������ �״�� ����
		targetStack = (ItemStack*)World::ins()->getTile(gridX, gridY, getGridZ()).ItemStackPtr;

		targetStack->setTargetSprIndex(targetStack->getSprIndex()); //���� ��ġ�� ��¥ ������ �̹���
		targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

		for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
		{
			txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
		}
		delete txPtr; //�����ʰ� ������ �Ű����� drop�� �������ش�.
	}
	addAniUSetPlayer(targetStack, aniFlag::throwing);

	targetStack->setFakeX(getX() - targetStack->getX());
	targetStack->setFakeY(getY() - targetStack->getY());
}
//@brief ����ġ ���̺�� �������� �����Ͽ� �Է��� index�� ��ɷ����� ��ȯ��
float Entity::getTalentLevel(int index)
{
	float resultLevel = 0;
	for (int i = 18; i >= 0; i--)
	{

		if (entityInfo.talentExp[index] >= expTable[i] / entityInfo.talentApt[index])
		{
			resultLevel = i + 1;
			if (i == 18) { return resultLevel; } //������ 18�̻��� ��� 
			else
			{
				resultLevel += (float)(entityInfo.talentExp[index] - expTable[i] / entityInfo.talentApt[index]) / (float)(expTable[i + 1] / entityInfo.talentApt[index] - expTable[i] / entityInfo.talentApt[index]);
				return resultLevel;
			}
		}
	}
	resultLevel += (float)(entityInfo.talentExp[index]) / (float)(expTable[1] / entityInfo.talentApt[index] - expTable[0] / entityInfo.talentApt[index]);
	return resultLevel;
}
void Entity::addTalentExp(int expVal)
{
	int divider = 0;
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		divider += entityInfo.talentFocus[i];
	}
	errorBox(divider == 0, "You need to enable at least one talent(divider=0 at addTalentExp).");
	int frag = floor((float)(expVal) / (float)divider);
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		entityInfo.talentExp[i] += (frag * entityInfo.talentFocus[i]);
	}
	//������ �� ����� ��Ŀ�� ����
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		if (entityInfo.talentFocus[i] > 0)
		{
			if (getTalentLevel(i) >= 18)
			{
				entityInfo.talentFocus[i] = 0;
			}
		}
	}
}
//�޼ҵ带 ������ ��ü�� ���̰� �������� ����Ѵ�.
//���� ��ü�� ������ ��� ������ ���� ���·� ��ȯ�Ѵ�.
void Entity::aimWeaponRight() { aimWeaponHand = equip::right; }
void Entity::aimWeaponLeft() { aimWeaponHand = equip::left; }
int Entity::getAimHand()
{
	if (aimWeaponHand == equip::right) { return equip::right; }
	else { return equip::left; }
}
//�� ��ü�� �ش� ��ü�� �������� �� ������ ����� ��� ������ ���� ���߷��� Ȯ��(0~1.0)���� ��ȯ����, aim�� false�� aimStack�� 0�� ��� 
int Entity::getAimWeaponIndex()
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

//void Entity::setPulledVehicle(Vehicle* inputVeh) { pulledCart = inputVeh; }
//void Entity::releasePulledVehicle() { pulledCart = nullptr; }
//bool Entity::hasPulledVehicle() { return (pulledCart != nullptr); }
//Vehicle* Entity::getPulledVehicle() { return pulledCart; }

void Entity::updateCustomSpriteHuman()
{
	SDL_Texture* targetTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CHAR_TEXTURE_WIDTH, CHAR_TEXTURE_HEIGHT);

	SDL_SetRenderTarget(renderer, targetTexture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	if (getSkin() != humanCustom::skin::null)
	{
		if (getSkin() == humanCustom::skin::yellow) drawTexture(spr::skinYellow->getTexture(), 0,0);
	}

	if (getEyes() != humanCustom::eyes::null)
	{
		if (getEyes() == humanCustom::eyes::blue) drawTexture(spr::eyesBlue->getTexture(), 0, 0);
		else if (getEyes() == humanCustom::eyes::red) drawTexture(spr::eyesRed->getTexture(), 0, 0);
	}

	if (getScar() != humanCustom::scar::null)
	{
	}

	if (getBeard() != humanCustom::beard::null)
	{
		if (getBeard() == humanCustom::beard::mustache) drawTexture(spr::beardMustacheBlack->getTexture(), 0, 0);
	}

	if (getHair() != humanCustom::hair::null)
	{
		bool noHair = false;
		for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
		{
			if (getEquipPtr()->itemInfo[i].checkFlag(itemFlag::NO_HAIR_HELMET) == true)
			{
				noHair = true;
				break;
			}
		}

		if (noHair == false)
		{
			switch (getHair())
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
			//errorBox(drawOrder.find(priority) != drawOrder.end(), L"�̹� �����ϴ� �켱���� ��� �߰��� :" + std::to_wstring(priority) + L" �̸�: " + getEquipPtr()->itemInfo[equipCounter].name);
			drawOrder[priority] = tgtSpr;
		}

		for (auto it = drawOrder.begin(); it != drawOrder.end(); it++)
		{
			if (it->second != nullptr)
			{
				drawTexture(it->second->getTexture(), 0,0);
			}
		}

	}

	SDL_SetRenderTarget(renderer, nullptr);
	customSprite = std::make_unique<Sprite>(renderer, targetTexture, 48, 48);
	delete spriteFlash;
}

void Entity::drawSelf()
{
	stepEvent();
	setZoom(zoomScale);
	if(entityInfo.sprFlip == false) setFlip(SDL_FLIP_NONE);
	else setFlip(SDL_FLIP_HORIZONTAL);


	int localSprIndex = getSpriteIndex();
	if (entityInfo.isPlayer == true)
	{
		if (getSpriteIndex() >= 0 && getSpriteIndex() <= 2)
		{
			if (entityInfo.walkMode == walkFlag::walk)
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
			else if (entityInfo.walkMode == walkFlag::crawl)
			{
				localSprIndex += 18;
			}


			if (entityInfo.walkMode != walkFlag::crawl)
			{
				for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
				{
					if (getEquipPtr()->itemInfo[i].equipState == equip::both)
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
	}

	int offsetX = 0;
	int offsetY = 0;
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


	int originX = (cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX());
	int originY = (cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY());

	int drawingX = originX + zoomScale * (offsetX);
	int drawingY = originY + zoomScale * (offsetY);



	//ĳ���� �׸��� �׸���
	if (itemDex[World::ins()->getTile(getGridX(), getGridY(), getGridZ()).floor].checkFlag(itemFlag::WATER_SHALLOW) == false && itemDex[World::ins()->getTile(getGridX(), getGridY(), getGridZ()).floor].checkFlag(itemFlag::WATER_DEEP) == false)
	{
		if (ridingEntity == nullptr)
		{
			drawSpriteCenter(spr::shadow, 1, originX, originY);
		}
		else if (ridingEntity != nullptr && ridingType == ridingFlag::horse)
		{
			drawSpriteCenter(spr::shadow, 2, originX, originY);
			drawSpriteCenter(ridingEntity->entityInfo.entitySpr, getSpriteIndex(), originX, originY);
		}
	}


	//ĳ���� Ŀ��Ÿ����¡ �׸���
	if (customSprite != nullptr)
	{
		SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND);

		if (itemDex[World::ins()->getTile(getGridX(), getGridY(), getGridZ()).floor].checkFlag(itemFlag::WATER_SHALLOW))
		{
			drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, {0,0,48,24});
			SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 130); //�ؽ��� ���� ����
			SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //������ ����
			drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,24 });
			SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 255); //�ؽ��� ���� ����
		}
		else if (itemDex[World::ins()->getTile(getGridX(), getGridY(), getGridZ()).floor].checkFlag(itemFlag::WATER_DEEP))
		{
			drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,0,48,27 });
			SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 80); //�ؽ��� ���� ����
			SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //������ ����
			drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,21 });
			SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 255); //�ؽ��� ���� ����
		}
		else
		{
			drawSpriteCenter(customSprite.get(), localSprIndex, drawingX, drawingY);//ĳ���� ��ü �׸���
		}
	}
	else
	{
		drawSpriteCenter(entityInfo.entitySpr, localSprIndex, drawingX, drawingY);//ĳ���� ��ü �׸���
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (entityInfo.HP != entityInfo.maxHP)//��ü HP ǥ��
	{
		int pivotX = drawingX - (int)(8 * zoomScale);
		int pivotY = drawingY + (int)((-8 + entityInfo.hpBarHeight) * zoomScale);
		SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::black);

		//����ũ HP
		if (entityInfo.fakeHP > entityInfo.HP) { entityInfo.fakeHP--; }
		else if (entityInfo.fakeHP < entityInfo.HP) entityInfo.fakeHP = entityInfo.HP;
		if (entityInfo.fakeHP != entityInfo.HP)
		{
			if (entityInfo.fakeHPAlpha > 30) { entityInfo.fakeHPAlpha -= 30; }
			else { entityInfo.fakeHPAlpha = 0; }
		}
		else { entityInfo.fakeHPAlpha = 255; }

		//����ũ MP
		if (entityInfo.fakeMP > entityInfo.MP) { entityInfo.fakeMP--; }
		else if (entityInfo.fakeMP < entityInfo.MP) entityInfo.fakeMP = entityInfo.MP;
		if (entityInfo.fakeMP != entityInfo.MP)
		{
			if (entityInfo.fakeMPAlpha > 30) { entityInfo.fakeMPAlpha -= 30; }
			else { entityInfo.fakeMPAlpha = 0; }
		}
		else { entityInfo.fakeMPAlpha = 255; }


		float ratioFakeHP = myMax((float)0.0, (entityInfo.fakeHP) / (float)(entityInfo.maxHP));
		dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::white, entityInfo.fakeHPAlpha);

		float ratioHP = myMax((float)0.0, (float)(entityInfo.HP) / (float)(entityInfo.maxHP));
		dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
		if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (entityInfo.isPlayer) drawFillRect(dst, lowCol::green);
		else drawFillRect(dst, lowCol::red);
	}

	if (getFlashType() != NULL) //��ƼƼ�� �÷��� ȿ���� ���� ���
	{
		drawSpriteCenter
		(
			getSpriteFlash(),
			localSprIndex,
			(cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY())
		);
	}

	if (ridingEntity != nullptr && ridingType == ridingFlag::horse)//�� ����
	{
		drawSpriteCenter(ridingEntity->entityInfo.entitySpr, getSpriteIndex() + 4, originX, originY);
	}

	setZoom(1.0);
	setFlip(SDL_FLIP_NONE);
};