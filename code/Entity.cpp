#include <SDL.h>
#include <SDL_image.h>

import Entity;
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
import Drawable;
import drawSprite;
import SkillData;
import Flame;
import Vehicle;

Entity::Entity(int gridX, int gridY, int gridZ)//������
{
	prt(L"Entity : �����ڰ� ȣ��Ǿ����ϴ�!\n");
	setAniPriority(1);
	setGrid(gridX, gridY, gridZ);
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = this;
	setSprite(spr::defaultMonster);
	equipment = new ItemPocket(storageType::equip);
	entityInfo.HP = 20;
	entityInfo.direction = 0;
	std::fill(talentApt.begin(), talentApt.end(), 2.0f);
}
Entity::~Entity()//�Ҹ���
{
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;
	prt(L"Entity : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
	//delete sprite;
	delete spriteFlash;
	delete equipment;
}
#pragma region getset method
std::vector<SkillData>& Entity::getBionicList() { return bionicList; }
std::vector<SkillData>& Entity::getMutationList() { return mutationList; }
std::vector<SkillData>& Entity::getMartialArtList() { return martialArtList; }
std::vector<SkillData>& Entity::getDivinePowerList() { return divinePowerList; }
std::vector<SkillData>& Entity::getMagicList() { return magicList; }
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
	if (skillDex[index].src == skillSrc::BIONIC) bionicList.push_back(skillDex[index]);
	else if (skillDex[index].src == skillSrc::MUTATION) mutationList.push_back(skillDex[index]);
	else if (skillDex[index].src == skillSrc::MARTIAL_ART) martialArtList.push_back(skillDex[index]);
	else if (skillDex[index].src == skillSrc::DIVINE_POWER) divinePowerList.push_back(skillDex[index]);
	else if (skillDex[index].src == skillSrc::MAGIC) magicList.push_back(skillDex[index]);
}
int Entity::searchBionicCode(int inputCode)
{
	for (int i = 0; i < bionicList.size(); i++)
	{
		if (bionicList[i].skillCode == inputCode) return i;
	}
	return -1;
}
bool Entity::eraseBionicCode(int inputCode)
{
	for (int i = 0; i < bionicList.size(); i++) if (bionicList[i].skillCode == inputCode)
	{
		bionicList.erase(bionicList.begin() + i);
		return true;
	}
	return false;
}
bool Entity::eraseBionicIndex(int inputIndex)
{
	if (inputIndex >= 0 && inputIndex < bionicList.size())
	{
		bionicList.erase(bionicList.begin() + inputIndex);
		return true;
	}
	return false;
}
int Entity::searchMutationCode(int inputCode)
{
	for (int i = 0; i < mutationList.size(); i++)
	{
		if (mutationList[i].skillCode == inputCode) return i;
	}
	return -1;
}
bool Entity::eraseMutationCode(int inputCode)
{
	for (int i = 0; i < mutationList.size(); i++) if (mutationList[i].skillCode == inputCode)
	{
		mutationList.erase(mutationList.begin() + i);
		return true;
	}
	return false;
}
bool Entity::eraseMutationIndex(int inputIndex)
{
	if (inputIndex >= 0 && inputIndex < mutationList.size())
	{
		mutationList.erase(mutationList.begin() + inputIndex);
		return true;
	}
	return false;
}
int Entity::searchMartialArtCode(int inputCode)
{
	for (int i = 0; i < martialArtList.size(); i++)
	{
		if (martialArtList[i].skillCode == inputCode) return i;
	}
	return -1;
}
bool Entity::eraseMartialArtCode(int inputCode)
{
	for (int i = 0; i < martialArtList.size(); i++) if (martialArtList[i].skillCode == inputCode)
	{
		martialArtList.erase(martialArtList.begin() + i);
		return true;
	}
	return false;
}
bool Entity::eraseMartialArtIndex(int inputIndex)
{
	if (inputIndex >= 0 && inputIndex < martialArtList.size())
	{
		martialArtList.erase(martialArtList.begin() + inputIndex);
		return true;
	}
	return false;
}
int Entity::searchDivinePowerCode(int inputCode)
{
	for (int i = 0; i < divinePowerList.size(); i++)
	{
		if (divinePowerList[i].skillCode == inputCode) return i;
	}
	return -1;
}
bool Entity::eraseDivinePowerCode(int inputCode)
{
	for (int i = 0; i < divinePowerList.size(); i++) if (divinePowerList[i].skillCode == inputCode)
	{
		divinePowerList.erase(divinePowerList.begin() + i);
		return true;
	}
	return false;
}
bool Entity::eraseDivinePowerIndex(int inputIndex)
{
	if (inputIndex >= 0 && inputIndex < divinePowerList.size())
	{
		divinePowerList.erase(divinePowerList.begin() + inputIndex);
		return true;
	}
	return false;
}
int Entity::searchMagicCode(int inputCode)
{
	for (int i = 0; i < magicList.size(); i++)
	{
		if (magicList[i].skillCode == inputCode) return i;
	}
	return -1;
}
bool Entity::eraseMagicCode(int inputCode)
{
	for (int i = 0; i < magicList.size(); i++) if (magicList[i].skillCode == inputCode)
	{
		magicList.erase(magicList.begin() + i);
		return true;
	}
	return false;
}
bool Entity::eraseMagicIndex(int inputIndex)
{
	if (inputIndex >= 0 && inputIndex < magicList.size())
	{
		magicList.erase(magicList.begin() + inputIndex);
		return true;
	}
	return false;
}
humanCustom::skin Entity::getSkin() { return skin; }
void Entity::setSkin(humanCustom::skin input) { skin = input; }
humanCustom::eyes Entity::getEyes() { return eyes; }
void Entity::setEyes(humanCustom::eyes input) { eyes = input; }
humanCustom::scar Entity::getScar() { return scar; }
void Entity::setScar(humanCustom::scar input) { scar = input; }
humanCustom::beard Entity::getBeard() { return beard; }
void Entity::setBeard(humanCustom::beard input) { beard = input; }
humanCustom::hair Entity::getHair() { return hair; }
void Entity::setHair(humanCustom::hair input) { hair = input; }
unsigned __int8 Entity::getAimStack() { return aimStack; }
void Entity::initAimStack() { aimStack = 0; }
void Entity::addAimStack() { aimStack++; }
void Entity::setNextAtkType(atkType inputAtkType) { nextAtkType = inputAtkType; }
atkType Entity::getNextAtkType() { return nextAtkType; }
void Entity::setAtkTarget(int inputX, int inputY, int inputZ, int inputPart)
{
	atkTargetGridX = inputX;
	atkTargetGridY = inputY;
	atkTargetGridZ = inputZ;
	atkTargetPart = inputPart;
}
void Entity::setAtkTarget(int inputX, int inputY, int inputZ)
{
	setAtkTarget(inputX, inputY, inputZ, -1);
}
ItemPocket* Entity::getEquipPtr() { return equipment; }
Sprite* Entity::getSpriteFlash() { return spriteFlash; }
void Entity::setFlashType(int inputType)
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
void Entity::setSpriteFlash(Sprite* inputSprite) { spriteFlash = inputSprite; }
int Entity::getFlashType() { return flashType; }
bool Entity::getLeftFoot() { return leftFoot; }
void Entity::setLeftFoot(bool input) { leftFoot = input; }
void Entity::setSpriteInfimum(int inputVal) { spriteInfimum = inputVal; }
int Entity::getSpriteInfimum() { return spriteInfimum; }
SDL_RendererFlip Entity::getEntityFlip() { return flip; }
void Entity::setEntityFlip(SDL_RendererFlip inputFlip) { flip = inputFlip; }
void Entity::setSpriteIndex(int index) { spriteIndex = index; }
int Entity::getSpriteIndex() { return spriteIndex; }
Sprite* Entity::getSprite() { return sprite; }
void Entity::setSprite(Sprite* inputSprite)
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
	drawFillRect(dstWhite, col::white);
	SDL_SetRenderTarget(renderer, nullptr);
	if (spriteFlash != nullptr) { delete spriteFlash; }
	spriteFlash = new Sprite(renderer, drawingTexture, 48, 48);
}
void Entity::setDirection(int dir)
{
	entityInfo.direction = dir;
	if (dir == 2 || dir == 6) {}
	else if (dir == 0 || (dir == 1 || dir == 7)) { setEntityFlip(SDL_FLIP_NONE); }
	else { setEntityFlip(SDL_FLIP_HORIZONTAL); }
}
void Entity::startAtk(int inputGridX, int inputGridY, int inputGridZ, int inputTarget, aniFlag inputAniType)
{
	setDirection(getIntDegree(getGridX(), getGridY(), inputGridX, inputGridY));
	setAtkTarget(inputGridX, inputGridY, inputGridZ, inputTarget);
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
void Entity::addDmg(int inputPartIndex, int inputDmg)
{
	for (int i = 0; i < entityInfo.parts.size(); i++)
	{
		if (entityInfo.parts[i][partsFlag::index] == inputPartIndex)
		{
			new Damage(std::to_wstring(inputDmg), this->getX(), this->getY() - 8, col::white, 10);

			entityInfo.parts[i][partsFlag::hp] -= inputDmg;
			entityInfo.HP -= inputDmg;
			prt(L"[����] %d�� �������� ���� %d�� ���Ͽ���. �ش� ������ ���� HP�� %d�̸� ��ü HP�� %d�̴�.\n", inputDmg, inputPartIndex, entityInfo.parts[i][partsFlag::hp], entityInfo.HP);

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
bool Entity::existPart(int inputPartIndex)
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
	return aStarDstX;
}
int Entity::getAStarDstY()
{
	errorBox(hasAStarDst == false, "getAStarDstY activated while hasAStarDst is false");
	return aStarDstY;
}
void Entity::setAStarDst(int inputX, int inputY)
{
	hasAStarDst = true;
	aStarDstX = inputX;
	aStarDstY = inputY;
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
void Entity::attack(int gridX, int gridY, int inputPartType)
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
		aimAcc = getAimAcc(victimEntity, inputPartType, true);

		if (aimAcc * 100.0 > randomRange(0, 100))
		{
			victimEntity->setFlashType(1);
			victimEntity->addDmg(inputPartType, 9);
		}
		else
		{
			new Damage(L"dodged", victimEntity->getX(), victimEntity->getY() - 8, col::yellow, 9);
			prt(L"[�����] ������ ��������.\n");
		}
	}
}
void Entity::attack(int gridX, int gridY)
{
	Entity* victimEntity = (Entity*)World::ins()->getTile(gridX, gridY, getGridZ()).EntityPtr;
	int targetPart;
	if (randomRange(1, 10) <= 8) { targetPart = 0; }
	else { targetPart = randomRange(1, victimEntity->entityInfo.parts.size() - 1); }
	attack(gridX, gridY, victimEntity->entityInfo.parts[targetPart][partsFlag::index]);
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
	SDL_SetTextureColorMod(spriteFlash->getTexture(), inputR, inputG, inputB);
	SDL_SetTextureAlphaMod(spriteFlash->getTexture(), inputAlpha);
	SDL_SetTextureBlendMode(spriteFlash->getTexture(), SDL_BLENDMODE_BLEND);
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

	addAniUSetPlayer(targetStack, aniFlag::drop);
}
void Entity::throwing(ItemPocket* txPtr, int gridX, int gridY)
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
	addAniUSetPlayer(targetStack, aniFlag::throwing);

	targetStack->setFakeX(getX() - targetStack->getX());
	targetStack->setFakeY(getY() - targetStack->getY());
}
float Entity::getTalentApt(int index) { return talentApt[index]; }
int Entity::getSKillExp(int index) { return talentExp[index]; }
int Entity::getTalentFocus(int index) { return talentFocus[index]; }
//@brief ����ġ ���̺�� �������� �����Ͽ� �Է��� index�� ��ɷ����� ��ȯ��
float Entity::getTalentLevel(int index)
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
void Entity::addTalentExp(int expVal)
{
	int divider = 0;
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		divider += talentFocus[i];
	}
	errorBox(divider == 0, "You need to enable at least one talent(divider=0 at addTalentExp).");
	int frag = floor((float)(expVal) / (float)divider);
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		talentExp[i] += (frag * talentFocus[i]);
	}
	//������ �� ����� ��Ŀ�� ����
	for (int i = 0; i < TALENT_SIZE; i++)
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
void Entity::setTalentApt(int index, float val) { talentApt[index] = val; }
void Entity::setTalentExp(int index, int val) { talentExp[index] = val; }
void Entity::setTalentFocus(int index, int val) { talentFocus[index] = val; }

//�޼ҵ带 ������ ��ü�� ���̰� �������� ����Ѵ�.
//���� ��ü�� ������ ��� ������ ���� ���·� ��ȯ�Ѵ�.
std::vector<int> Entity::getAllParts()
{
	std::vector<int> partsVec;

	for (int i = 0; i < entityInfo.parts.size(); i++)
	{
		partsVec.push_back(entityInfo.parts[i][partsFlag::index]);
	}

	return partsVec;
}
void Entity::aimWeaponRight() { aimWeaponHand = equip::right; }
void Entity::aimWeaponLeft() { aimWeaponHand = equip::left; }
int Entity::getAimHand()
{
	if (aimWeaponHand == equip::right) { return equip::right; }
	else { return equip::left; }
}
//�� ��ü�� �ش� ��ü�� �������� �� ������ ����� ��� ������ ���� ���߷��� Ȯ��(0~1.0)���� ��ȯ����, aim�� false�� aimStack�� 0�� ��� 
float Entity::getAimAcc(Entity* victimEntity, int inputPartType, bool aim)
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

void Entity::drawSelf()
{
	stepEvent();
	setZoom(zoomScale);
	setFlip(getEntityFlip());

	//prt(L"���� �� ��ü�� fake ��ǥ�� %f,%f�̴�.\n", getFakeX(), getFakeY());
	int drawingX = (cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX());
	int drawingY = (cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY());

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
			//errorBox(drawOrder.find(priority) != drawOrder.end(), L"�̹� �����ϴ� �켱���� ��� �߰��� :" + std::to_wstring(priority) + L" �̸�: " + getEquipPtr()->itemInfo[equipCounter].name);
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
		drawFillRect(dst, col::black);

		//����ũ HP
		if (entityInfo.fakeHP > entityInfo.HP) { entityInfo.fakeHP--; }

		if (entityInfo.fakeHP != entityInfo.HP)
		{
			if (entityInfo.fakeHPAlpha > 30) { entityInfo.fakeHPAlpha -= 30; }
			else { entityInfo.fakeHPAlpha = 0; }
		}
		else { entityInfo.fakeHPAlpha = 255; }

		float ratioFakeHP = myMax((float)0.0, (entityInfo.fakeHP) / (float)(entityInfo.maxHP));
		dst = { drawingX - (int)(7 * zoomScale), drawingY + (int)(4 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::white);

		float ratioHP = myMax((float)0.0, (float)(entityInfo.HP) / (float)(entityInfo.maxHP));
		dst = { drawingX - (int)(7 * zoomScale), drawingY + (int)(4 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
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
			getSpriteIndex(),
			(cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY())
		);
	}
	setZoom(1.0);
	setFlip(SDL_FLIP_NONE);
};