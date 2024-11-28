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


Entity::Entity(int newEntityIndex, int gridX, int gridY, int gridZ)//생성자
{
	prt(L"Entity : 생성자가 호출되었습니다!\n");
	loadDataFromDex(newEntityIndex);
	setAniPriority(1);
	setGrid(gridX, gridY, gridZ);
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = this;
	entityInfo.equipment = new ItemPocket(storageType::equip);
	entityInfo.talentFocus[0] = 1;
	for (int i = 0; i < TALENT_SIZE; i++) entityInfo.talentApt[i] = 2.0;
}
Entity::~Entity()//소멸자
{
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;

	//나중에 바닥이 걸을 수 있는 타일인지 아닌지를 체크하여 true가 되는지의 여부를 결정하는 조건문 추가할것
	World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable = true;
	prt(L"Entity : 소멸자가 호출되었습니다..\n");
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
	prt(L"스킬 %ls를 추가했다.\n", skillDex[index].name.c_str());
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
		//하얗게 만들 텍스쳐를 그려넣음

		if (customSprite == nullptr) SDL_RenderCopy(renderer, entityInfo.entitySpr->getTexture(), &src, &dst);
		else SDL_RenderCopy(renderer, customSprite->getTexture(), &src, &dst);
		
		//텍스쳐에 흰색으로 가산 블렌딩을 사용해 하얗게 만듬
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
	return 1 / 0.8; //원래 여기에 공격속도가 들어가야함
}
void Entity::loadDataFromDex(int index)
{
	entityInfo = entityDex[index];
	entityInfo.HP = entityInfo.maxHP;
	entityInfo.fakeHP = entityInfo.maxHP;
}
//@brief 해당 파츠에 데미지를 추가하고 메인 HP도 그만큼 뺍니다.
void Entity::addDmg(int inputDmg)
{
	new Damage(std::to_wstring(inputDmg), col::white, getGridX(), getGridY(), dmgAniFlag::none);
	entityInfo.HP -= inputDmg;
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
		prt(L"[디버그] 공격이 빗나갔다.\n");
	}
	else
	{
		//명중률 계산
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
			prt(L"[디버그] 공격이 빗나갔다.\n");
		}
	}
}
void Entity::updateWalkable(int gridX, int gridY)//만약 다를 경우 개체에서 오버라이드해서 쓰시오
{
	//만약 걸을 수 있다해도 해당 위치에 엔티티가 존재하면 걷기불가로 만듬

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
	//아이템 스택이 이미 있는 경우와 없는 경우

	if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).ItemStackPtr == nullptr) //그 자리에 템 없는 경우
	{
		//기존 스택이 없으면 새로 만들고 그 ptr을 전달
		targetStack = new ItemStack(getGridX(), getGridY(), getGridZ());
		targetStack->setPocket(txPtr);//스토리지 교체(메모리는 메소드 내부에서 해제됨)
		targetStack->updateSprIndex();
	}
	else //이미 그 자리에 아이템이 있는 경우
	{
		//기존 스택이 있으면 그 스택을 그대로 전달
		targetStack = (ItemStack*)World::ins()->getTile(getGridX(), getGridY(), getGridZ()).ItemStackPtr;

		targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

		for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
		{
			txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
		}
		delete txPtr; //잊지않고 기존의 매개변수 drop을 제거해준다.
	}

	addAniUSetPlayer(targetStack, aniFlag::drop);
}
void Entity::throwing(ItemPocket* txPtr, int gridX, int gridY)
{
	ItemStack* targetStack;
	//아이템 스택이 이미 있는 경우와 없는 경우

	if (World::ins()->getTile(gridX, gridY, getGridZ()).ItemStackPtr == nullptr) //그 자리에 템 없는 경우
	{
		//기존 스택이 없으면 새로 만들고 그 ptr을 전달
		targetStack = new ItemStack(gridX, gridY, getGridZ());
		targetStack->setPocket(txPtr);//스토리지 교체(메모리는 메소드 내부에서 해제됨)
		targetStack->updateSprIndex();
	}
	else //이미 그 자리에 아이템이 있는 경우
	{
		//기존 스택이 있으면 그 스택을 그대로 전달
		targetStack = (ItemStack*)World::ins()->getTile(gridX, gridY, getGridZ()).ItemStackPtr;

		targetStack->setTargetSprIndex(targetStack->getSprIndex()); //원래 위치에 가짜 아이템 이미지
		targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

		for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
		{
			txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
		}
		delete txPtr; //잊지않고 기존의 매개변수 drop을 제거해준다.
	}
	addAniUSetPlayer(targetStack, aniFlag::throwing);

	targetStack->setFakeX(getX() - targetStack->getX());
	targetStack->setFakeY(getY() - targetStack->getY());
}
//@brief 경험치 테이블과 적성값을 참조하여 입력한 index의 재능레벨을 반환함
float Entity::getTalentLevel(int index)
{
	float resultLevel = 0;
	for (int i = 18; i >= 0; i--)
	{

		if (entityInfo.talentExp[index] >= expTable[i] / entityInfo.talentApt[index])
		{
			resultLevel = i + 1;
			if (i == 18) { return resultLevel; } //레벨이 18이상일 경우 
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
	//만렙이 된 재능의 포커스 해제
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
//메소드를 실행한 객체를 죽이고 아이템을 드랍한다.
//현재 개체가 보유한 모든 부위를 벡터 형태로 반환한다.
void Entity::aimWeaponRight() { aimWeaponHand = equip::right; }
void Entity::aimWeaponLeft() { aimWeaponHand = equip::left; }
int Entity::getAimHand()
{
	if (aimWeaponHand == equip::right) { return equip::right; }
	else { return equip::left; }
}
//이 개체가 해당 개체를 공격했을 때 공격한 방법과 상대 부위에 따른 명중률을 확률(0~1.0)으로 반환해줌, aim이 false면 aimStack을 0로 계산 
int Entity::getAimWeaponIndex()
{
	//현재 플레이어가 적에게 겨누는 무기의 인덱스를 반환함(-1이면 맨손)
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
	
	//캐릭터 장비 그리기
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



	//캐릭터 그림자 그리기
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


	//캐릭터 커스타미이징 그리기
	if (customSprite != nullptr)
	{
		SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND);

		if (itemDex[World::ins()->getTile(getGridX(), getGridY(), getGridZ()).floor].checkFlag(itemFlag::WATER_SHALLOW))
		{
			drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, {0,0,48,24});
			SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 130); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(customSprite.get()->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			drawSpriteCenterExSrc(customSprite.get(), localSprIndex, drawingX, drawingY, { 0,24,48,24 });
			SDL_SetTextureAlphaMod(customSprite.get()->getTexture(), 255); //텍스쳐 투명도 설정
		}
		else if (itemDex[World::ins()->getTile(getGridX(), getGridY(), getGridZ()).floor].checkFlag(itemFlag::WATER_DEEP))
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
	}
	else
	{
		drawSpriteCenter(entityInfo.entitySpr, localSprIndex, drawingX, drawingY);//캐릭터 본체 그리기
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (entityInfo.HP != entityInfo.maxHP)//개체 HP 표기
	{
		int pivotX = drawingX - (int)(8 * zoomScale);
		int pivotY = drawingY + (int)((-8 + entityInfo.hpBarHeight) * zoomScale);
		SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawFillRect(dst, col::black);

		//페이크 HP
		if (entityInfo.fakeHP > entityInfo.HP) { entityInfo.fakeHP--; }
		else if (entityInfo.fakeHP < entityInfo.HP) entityInfo.fakeHP = entityInfo.HP;
		if (entityInfo.fakeHP != entityInfo.HP)
		{
			if (entityInfo.fakeHPAlpha > 30) { entityInfo.fakeHPAlpha -= 30; }
			else { entityInfo.fakeHPAlpha = 0; }
		}
		else { entityInfo.fakeHPAlpha = 255; }

		//페이크 MP
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

	if (getFlashType() != NULL) //엔티티에 플래시 효과가 있을 경우
	{
		drawSpriteCenter
		(
			getSpriteFlash(),
			localSprIndex,
			(cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX()),
			(cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY())
		);
	}

	if (ridingEntity != nullptr && ridingType == ridingFlag::horse)//말 앞쪽
	{
		drawSpriteCenter(ridingEntity->entityInfo.entitySpr, getSpriteIndex() + 4, originX, originY);
	}

	setZoom(1.0);
	setFlip(SDL_FLIP_NONE);
};