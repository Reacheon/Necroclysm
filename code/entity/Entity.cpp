import Entity;

#include <SDL3/SDL.h>

import std;
import util;
import globalVar;
import wrapVar;
import constVar;
import log;
import Sprite;
import Ani;
import Coord;
import World;
import ItemStack;
import EntityData;
import ItemData;
import Damage;
import SkillData;
import Flame;
import Vehicle;

Entity::Entity(int newEntityIndex, int gridX, int gridY, int gridZ)//생성자
{
	prt(L"Entity : 생성자가 호출되었습니다!\n");
	loadDataFromDex(newEntityIndex);
	setAniPriority(1);
	setGrid(gridX, gridY, gridZ);
	entityInfo.equipment = std::make_unique<ItemPocket>(storageType::equip);
	entityInfo.proficFocus[0] = 1;

	for (int i = 0; i < TALENT_SIZE; i++) entityInfo.proficApt[i] = 2.0;
}
Entity::~Entity()//소멸자
{
	//나중에 바닥이 걸을 수 있는 타일인지 아닌지를 체크하여 true가 되는지의 여부를 결정하는 조건문 추가할것
	prt(L"Entity : 소멸자가 호출되었습니다..\n");
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
	prt(L"스킬 %ls를 추가했다.\n", skillDex[index].name.c_str());
	entityInfo.skillList.push_back(skillDex[index]);
}


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
	return entityInfo.equipment.get();
}
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
	return 1 / 0.8; //원래 여기에 공격속도가 들어가야함
}
void Entity::loadDataFromDex(int index)
{
	entityInfo = entityDex[index].cloneEntity();
	entityInfo.HP = entityInfo.maxHP;
	entityInfo.fakeHP = entityInfo.maxHP;
}

//@brief 해당 파츠에 데미지를 추가하고 메인 HP에는 절반만 전달합니다.
void Entity::takeDamage(int inputDmg, dmgFlag inputType, humanPartFlag inputPart)
{
	entityInfo.displayHPBarCount = 600;//100초
	entityInfo.alphaHPBar = 255;
	int calcDmg = inputDmg;
	int partDmg = 0;

	if (inputType == dmgFlag::pierce)
	{
		if (entityInfo.isPlayer)
		{
			if (inputPart == humanPartFlag::head)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResPierceHead()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::lArm)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResPierceLArm()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::rArm)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResPierceRArm()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::lLeg)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResPierceLLeg()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::rLeg)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResPierceRLeg()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else // torso 직접 공격
			{
				calcDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResPierceTorso()));
			}
		}
		else calcDmg = myMax(0, inputDmg - randomRange(0, entityInfo.rPierce));
	}
	else if (inputType == dmgFlag::cut)
	{
		if (entityInfo.isPlayer)
		{
			if (inputPart == humanPartFlag::head)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResCutHead()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::lArm)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResCutLArm()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::rArm)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResCutRArm()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::lLeg)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResCutLLeg()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::rLeg)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResCutRLeg()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else // torso 직접 공격
			{
				calcDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResCutTorso()));
			}
		}
		else calcDmg = myMax(0, inputDmg - randomRange(0, entityInfo.rCut));
	}
	else if (inputType == dmgFlag::bash)
	{
		if (entityInfo.isPlayer)
		{
			if (inputPart == humanPartFlag::head)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResBashHead()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::lArm)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResBashLArm()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::rArm)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResBashRArm()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::lLeg)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResBashLLeg()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else if (inputPart == humanPartFlag::rLeg)
			{
				partDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResBashRLeg()));
				calcDmg = partDmg / 2; // 부위 데미지의 절반만 몸통으로
			}
			else // torso 직접 공격
			{
				calcDmg = myMax(0, inputDmg - randomRange(0, PlayerPtr->getResBashTorso()));
			}
		}
		else calcDmg = myMax(0, inputDmg - randomRange(0, entityInfo.rBash));
	}
	else if (inputType == dmgFlag::fire)
	{
		if (entityInfo.isPlayer) calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(PlayerPtr->getResFire(), 99))) / 100.0f)));
		else calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(entityInfo.rFire, 99))) / 100.0f)));
	}
	else if (inputType == dmgFlag::ice)
	{
		if (entityInfo.isPlayer) calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(PlayerPtr->getResCold(), 99))) / 100.0f)));
		else calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(entityInfo.rCold, 99))) / 100.0f)));
	}
	else if (inputType == dmgFlag::elec)
	{
		if (entityInfo.isPlayer) calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(PlayerPtr->getResElec(), 99))) / 100.0f)));
		else calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(entityInfo.rElec, 99))) / 100.0f)));
	}
	else if (inputType == dmgFlag::corr)
	{
		if (entityInfo.isPlayer) calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(PlayerPtr->getResCorr(), 99))) / 100.0f)));
		else calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(entityInfo.rCorr, 99))) / 100.0f)));
	}
	else if (inputType == dmgFlag::rad)
	{
		if (entityInfo.isPlayer) calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(PlayerPtr->getResRad(), 99))) / 100.0f)));
		else calcDmg = myMax(0, static_cast<int>(static_cast<float>(inputDmg) * ((100.0f - static_cast<float>(myMin(entityInfo.rRad, 99))) / 100.0f)));
	}

	new Damage(std::to_wstring(calcDmg), col::white, getGridX(), getGridY(), dmgAniFlag::none);

	if (entityInfo.isPlayer)
	{
		if (inputPart == humanPartFlag::head) PlayerPtr->headHP -= partDmg;
		else if (inputPart == humanPartFlag::lArm) PlayerPtr->lArmHP -= partDmg;
		else if (inputPart == humanPartFlag::rArm) PlayerPtr->rArmHP -= partDmg;
		else if (inputPart == humanPartFlag::lLeg) PlayerPtr->lLegHP -= partDmg;
		else if (inputPart == humanPartFlag::rLeg) PlayerPtr->rLegHP -= partDmg;
	}

	entityInfo.HP -= calcDmg;
	if (entityInfo.HP <= 0)//HP 0, 사망
	{
		death();
		return;
	}
}

void Entity::updateStatus()
{
	//rPCB rFCECR SH EV 등을 업데이트함

	//sh
	entityInfo.sh = entityDex[entityInfo.entityCode].sh;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.sh += getEquipPtr()->itemInfo[i].sh;
	}

	//ev
	entityInfo.ev = entityDex[entityInfo.entityCode].ev;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.ev += getEquipPtr()->itemInfo[i].ev;
	}

	//rFire
	entityInfo.rFire = entityDex[entityInfo.entityCode].rFire;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.rFire += getEquipPtr()->itemInfo[i].rFire;
	}

	//rCold
	entityInfo.rCold = entityDex[entityInfo.entityCode].rCold;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.rCold += getEquipPtr()->itemInfo[i].rCold;
	}

	//rElec
	entityInfo.rElec = entityDex[entityInfo.entityCode].rElec;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.rElec += getEquipPtr()->itemInfo[i].rElec;
	}

	//rCorr
	entityInfo.rCorr = entityDex[entityInfo.entityCode].rCorr;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.rCorr += getEquipPtr()->itemInfo[i].rCorr;
	}

	//rRad
	entityInfo.rRad = entityDex[entityInfo.entityCode].rRad;//기본 개체값으로 재설정
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
	{
		entityInfo.rRad += getEquipPtr()->itemInfo[i].rRad;
	}
}
//입력한 파츠 인덱스의 rPierce를 반환
int Entity::getRPierce(int inputPartIndex)
{
	int totalRPierce = 0;
	return totalRPierce;
}
//입력한 파츠 인덱스의 rCut를 반환
int Entity::getRCut(int inputPartIndex)
{
	int totalRCut = 0;
	return totalRCut;
}
//입력한 파츠 인덱스의 rBash를 반환
int Entity::getRBash(int inputPartIndex)
{
	int totalRBash = 0;
	return totalRBash;
}
//입력한 파츠 인덱스의 SH를 반환
int Entity::getSH()
{
	int totalSH = 0;

	//기본 개체값 더하기
	totalSH += entityInfo.sh;

	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
	{
		totalSH += getEquipPtr()->itemInfo[i].sh;
	}
	return totalSH;
}
//입력한 파츠 인덱스의 EV를 반환
int Entity::getEV()
{
	int totalEV = 0;

	//기본 개체값 더하기
	totalEV += entityInfo.ev;

	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
	{
		totalEV += getEquipPtr()->itemInfo[i].ev;
	}
	return totalEV;
}
//입력한 파츠 인덱스의 enc를 반환
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
	errorBox(hasAStarDst == false, L"getAStarDstX activated while hasAStarDst is false");
	return aStarDst.x;
}
int Entity::getAStarDstY()
{
	errorBox(hasAStarDst == false, L"getAStarDstY activated while hasAStarDst is false");
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
	int prevGridX = getGridX();
	int prevGridY = getGridY();
	int dstX = 16 * getGridX() + 8;
	int dstY = 16 * getGridY() + 8;
	int dGridX, dGridY;
	dir2Coord(dir, dGridX, dGridY);
	dstX += 16 * dGridX;
	dstY += 16 * dGridY;
	int dstGridX = (dstX - 8) / 16;
	int dstGridY = (dstY - 8) / 16;

	errorBox(dGridX == 0 && dGridY == 0, L"Entity::move에서 같은 좌표로 이동했다.");


	if (jump == false)
	{
		if (entityInfo.walkMode == walkFlag::run) entityInfo.gridMoveSpd = 3.5;
		else if (entityInfo.walkMode == walkFlag::wade) entityInfo.gridMoveSpd = 1.8;
		else if (entityInfo.walkMode == walkFlag::walk) entityInfo.gridMoveSpd = 3.0;
		else if (entityInfo.walkMode == walkFlag::crawl || entityInfo.walkMode == walkFlag::swim) entityInfo.gridMoveSpd = 2.0;
		else if (entityInfo.walkMode == walkFlag::crouch) entityInfo.gridMoveSpd = 2.0;

		if (pulledCart != nullptr)
		{
			int dx = prevGridX - pulledCart->getGridX();
			int dy = prevGridY - pulledCart->getGridY();
			int dir = coord2Dir(dx, dy);
			pulledCart->setFakeX(-16 * dx);
			pulledCart->setFakeY(-16 * dy);
			addAniUSet(pulledCart, aniFlag::move);

			pulledCart->shift(dx, dy);
			pulledCart->pullMoveSpd = entityInfo.gridMoveSpd;
		}

		EntityPtrMove({ prevGridX,prevGridY, getGridZ() }, { prevGridX + dGridX, prevGridY + dGridY, getGridZ() });
		setFakeX(-16 * dGridX);
		setFakeY(-16 * dGridY);
		if (entityInfo.isPlayer)
		{
			cameraFix = false;
			cameraX = getX() + getIntegerFakeX();
			cameraY = getY() + getIntegerFakeY();
		}
		addAniUSet(this, aniFlag::move);


	}
	else
	{
		EntityPtrMove({ prevGridX,prevGridY, getGridZ() }, { prevGridX + dGridX, prevGridY + dGridY, getGridZ() });
		setDstGrid(dstGridX, dstGridY);

		if (pulledCart != nullptr)
		{
			pulledCart->setDstGrid(getGridX(), getGridY());
			pulledCart->setGrid(getGridX(), getGridY(), getGridZ());
		}
	}
}
void Entity::attack(int gridX, int gridY)
{
	Entity* victimEntity = TileEntity(gridX, gridY, getGridZ());
	if (victimEntity == nullptr)
	{
		prt(L"[디버그] 공격이 빗나갔다.\n");
	}
	else
	{
		//명중률 계산
		float aimAcc;
		aimAcc = 0.98;

		if (aimAcc * 100.0 > randomRange(0, 100))
		{
			victimEntity->flash = { 255, 0, 0, 120 };
			if (victimEntity->entityInfo.isPlayer)
			{
                int prob = randomRange(0, 100);	
                humanPartFlag part = humanPartFlag::torso;

				if (prob <= 10) part = humanPartFlag::head;
				else if(prob <= 20) part = humanPartFlag::lArm;
				else if (prob <= 30) part = humanPartFlag::rArm;
				else if (prob <= 40) part = humanPartFlag::lLeg;
                else if (prob <= 50) part = humanPartFlag::rLeg;	

				victimEntity->takeDamage(randomRange(6, 10), dmgFlag::pierce, part);
			}
			else
			{
				victimEntity->takeDamage(randomRange(6, 10), dmgFlag::pierce);
			}
		}
		else
		{
			new Damage(L"MISS", col::yellow, victimEntity->getGridX(), victimEntity->getGridY(), dmgAniFlag::dodged);
			prt(L"[디버그] 공격이 빗나갔다.\n");
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
	TileFov(x1, y1, getGridZ()) = fovFlag::white;
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
				TileFov(x1, y1, getGridZ()) = fovFlag::white;
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
				p = p + (2 * dely);
			}
			else
			{
				if (x2 > xo && y2 >= yo) { x1++; y1++; }
				else if (x2 > xo && yo > y2) { x1++; y1--; }
				else if (xo > x2 && y2 > yo) { x1--; y1++; }
				else { x1--; y1--; }
				TileFov(x1, y1, getGridZ()) = fovFlag::white;
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
				TileFov(x1, y1, getGridZ()) = fovFlag::white;
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
				p = p + (2 * delx);
			}
			else
			{
				if (x2 >= xo && y2 > yo) { x1++; y1++; }
				else if (x2 > xo && yo > y2) { x1++; y1--; }
				else if (xo > x2 && y2 > yo) { x1--; y1++; }
				else { x1--; y1--; }
				TileFov(x1, y1, getGridZ()) = fovFlag::white;
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
			TileFov(x1, y1, getGridZ()) = fovFlag::white;
			if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
	TileFov(x1, y1, getGridZ()) = fovFlag::white;
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
					TileFov(x1, y1, getGridZ()) = fovFlag::white;
				}
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
					TileFov(x1, y1, getGridZ()) = fovFlag::white;
				}
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
					TileFov(x1, y1, getGridZ()) = fovFlag::white;
				}
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
					TileFov(x1, y1, getGridZ()) = fovFlag::white;
				}
				if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
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
				TileFov(x1, y1, getGridZ()) = fovFlag::white;
			}
			if (isRayBlocker({ x1, y1, getGridZ() })) { return; }
			i++;
		}
	}
}
void Entity::stepEvent()
{
}

void Entity::drop(ItemPocket* txPtr)
{
	ItemStack* targetStack = nullptr;;
	//아이템 스택이 이미 있는 경우와 없는 경우

	for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
	{
		txPtr->itemInfo[i].equipState = equipHandFlag::none;
	}

	if (TileItemStack(getGridX(), getGridY(), getGridZ()) == nullptr) //그 자리에 템 없는 경우
	{
		//기존 스택이 없으면 새로 만들고 그 ptr을 전달
		createItemStack({ getGridX(), getGridY(), getGridZ() });
		targetStack = TileItemStack(getGridX(), getGridY(), getGridZ());
		for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--) txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
		targetStack->updateSprIndex();
	}
	else //이미 그 자리에 아이템이 있는 경우
	{
		//기존 스택이 있으면 그 스택을 그대로 전달
		targetStack = TileItemStack(getGridX(), getGridY(), getGridZ());
		for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--) txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
	}

	addAniUSetPlayer(targetStack, aniFlag::drop);
	targetStack->pullStackLights();
}
void Entity::throwing(std::unique_ptr<ItemPocket> txPtr, int gridX, int gridY)
{
	//아이템 스택이 이미 있는 경우와 없는 경우

	throwingItemPocket = std::move(txPtr);
	throwCoord.x = gridX;
	throwCoord.y = gridY;
	throwCoord.z = getGridZ();
	addAniUSetPlayer(this, aniFlag::entityThrow);

}
//@brief 경험치 테이블과 적성값을 참조하여 입력한 index의 재능레벨을 반환함
float Entity::getProficLevel(int index)
{
	float exp = entityInfo.proficExp[index];

	// 경험치가 0이면 레벨 0.0 반환
	if (exp <= 0) return 0.0f;

	// 첫 번째 레벨업 전까지는 0.X 레벨
	if (exp < expTable[0]) return exp / expTable[0];

	// 각 레벨 구간에서 정확한 레벨 계산
	for (int i = 0; i < MAX_PROFIC_LEVEL - 1; i++)
	{
		float currentExp = expTable[i];
		float nextExp = expTable[i + 1];
		if (exp >= currentExp && exp < nextExp)
		{
			float partial = (exp - currentExp) / (nextExp - currentExp);
			return static_cast<float>(i + 1) + partial;  // i+1로 변경 (0부터 시작)
		}
	}

	// 최대 레벨 도달
	return static_cast<float>(MAX_PROFIC_LEVEL);
}

void Entity::addProficExp(int expVal)
{
	int divider = 0;
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		divider += entityInfo.proficFocus[i];
	}
	errorBox(divider == 0, L"You need to enable at least one profic(divider=0 at addProficExp).");
	int frag = floor((float)(expVal) / (float)divider);
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		entityInfo.proficExp[i] += (frag * entityInfo.proficFocus[i]);
	}
	//만렙이 된 재능의 포커스 해제
	for (int i = 0; i < TALENT_SIZE; i++)
	{
		if (entityInfo.proficFocus[i] > 0)
		{
			if (getProficLevel(i) >= MAX_PROFIC_LEVEL)
			{
				entityInfo.proficFocus[i] = 0;
			}
		}
	}
}
//메소드를 실행한 객체를 죽이고 아이템을 드랍한다.
//현재 개체가 보유한 모든 부위를 벡터 형태로 반환한다.
void Entity::aimWeaponRight() { aimWeaponHand = equipHandFlag::right; }
void Entity::aimWeaponLeft() { aimWeaponHand = equipHandFlag::left; }
equipHandFlag Entity::getAimHand()
{
	if (aimWeaponHand == equipHandFlag::right) { return equipHandFlag::right; }
	else { return equipHandFlag::left; }
}
//이 개체가 해당 개체를 공격했을 때 공격한 방법과 상대 부위에 따른 명중률을 확률(0~1.0)으로 반환해줌, aim이 false면 aimStack을 0로 계산 
int Entity::getAimWeaponIndex()
{
	//현재 플레이어가 적에게 겨누는 무기의 인덱스를 반환함(-1이면 맨손)
	equipHandFlag targetHand;
	std::vector<ItemData>& equipInfo = getEquipPtr()->itemInfo;
	if (getAimHand() == equipHandFlag::left) { targetHand = equipHandFlag::left; }
	else { targetHand = equipHandFlag::right; }


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
			else if (equipInfo[i].equipState == equipHandFlag::both)
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


void Entity::pullEquipLights()
{
	for (int i = 0; i < getEquipPtr()->itemInfo.size(); i++)
	{
		if (getEquipPtr()->itemInfo[i].lightPtr != nullptr)
		{
			getEquipPtr()->itemInfo[i].lightPtr.get()->moveLight(getGridX(), getGridY(), getGridZ());
		}
	}
}

