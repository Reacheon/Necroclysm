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

export class Entity : public Ani, public Coord, public Drawable//엔티티는 시야 기능과 개인 FOV, 현재 좌표, 그리고 화면에 표시되는 기능과 고유 텍스쳐를 가진다.
{
private:
	Sprite* sprite = nullptr; //메인 스프라이트
	Sprite* spriteFlash = nullptr; //플래시용 흰색 마스킹 스프라이트
	SDL_Color flash = { 0,0,0,0 }; //플래시 컬러
	int hp;
	int direction = 7;
	int spriteIndex = 0;
	bool canMove;
	bool drawEquip = false;
	int eyeSight = 8; //기본 시야
	bool isPlayer = false;
	int spriteInfimum = 0;
	bool bothHand = false;//양손무기인지 체크? 아직 구현 안함

	bool leftFoot = true; //걷기 애니메이션에서의 왼발, 오른발 순서
	int flashType = 0; // 0 : NULL, 1 : white, 2 : white->red
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	ItemPocket* equipment;
	std::vector<std::array<int, pocketFlagWidth>> pocketInfo;
	ItemStack* dropItemPtr = nullptr; //버리기 및 던지기에 사용하는 아이템스택 포인터
	std::vector<std::array<int, dmgFlagSize>> dmgVec;//현재 이 개체가 받은 데미지의 총량

	bool hasAStarDst = false;
	int aStarDstX = 0;
	int aStarDstY = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////

	std::array<int, talentSize> talentExp = { 0, }; //경험치
	std::array<float, talentSize> talentApt; //적성
	std::array<int, talentSize> talentFocus = { 0, }; //집중도 0:미분배, 1:소분배, 2:일반분배

	///////////////////////////////////////////////////////////////////////////////////////////

	std::vector<int> mutationVec;//현재 이 객체의 돌연변이 목록
	std::vector<int> bionicVec;//현재 이 객체의 바이오닉 목록

	int atkTargetGridX = 0;
	int atkTargetGridY = 0;
	int atkTargetGridZ = 0;
	int atkTargetPart = -1;

	atkType nextAtkType = atkType::bash; //다음 공격에 사용할 공격의 타입
	unsigned __int8 aimStack = 0; //다음 공격에 가산될 조준 스택

	int aimWeaponHand = equip::right;//현재 적에게 겨누는 주무기

	humanCustom::skin skin = humanCustom::skin::null;
	humanCustom::eyes eyes = humanCustom::eyes::null;
	humanCustom::scar scar = humanCustom::scar::null;
	humanCustom::beard beard = humanCustom::beard::null;
	humanCustom::hair hair = humanCustom::hair::null;
public:
	EntityData entityInfo;
	Entity(int gridX, int gridY, int gridZ)//생성자
	{
		prt(col::orange, L"Entity : 생성자가 호출되었습니다!\n");
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
	~Entity()//소멸자
	{
		World::ins()->getTile(getGridX(), getGridY(), getGridZ()).EntityPtr = nullptr;
		prt(col::orange, L"Entity : 소멸자가 호출되었습니다..\n");
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
		//하얗게 만들 텍스쳐를 그려넣음
		SDL_RenderCopy(renderer, inputSprite->getTexture(), &src, &dst);
		//텍스쳐에 흰색으로 가산 블렌딩을 사용해 하얗게 만듬
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
		return 1 / 0.8; //원래 여기에 공격속도가 들어가야함
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
	//@brief 해당 파츠에 데미지를 추가하고 메인 HP도 그만큼 뺍니다.
	void addDmg(int inputPartIndex, int inputDmg)
	{
		for (int i = 0; i < entityInfo.parts.size(); i++)
		{
			if (entityInfo.parts[i][partsFlag::index] == inputPartIndex)
			{
				new Damage(std::to_wstring(inputDmg), this->getX(), this->getY() - 8, col::white, 10);

				entityInfo.parts[i][partsFlag::hp] -= inputDmg;
				entityInfo.HP -= inputDmg;
				prt(col::orange, L"[공격] %d의 데미지를 부위 %d에 가하였다. 해당 부위의 남은 HP는 %d이며 전체 HP는 %d이다.\n", inputDmg, inputPartIndex, entityInfo.parts[i][partsFlag::hp], entityInfo.HP);

				if (entityInfo.HP <= 0)//HP 0, 사망
				{
					death();
					return;
				}

				if (entityInfo.parts[i][partsFlag::hp] <= 0)//부위파괴
				{
					switch (entityInfo.bodyTemplate)
					{
					case bodyTemplateFlag::human://사람일 경우
					{
						//머리 또는 몸 파괴
						if (inputPartIndex == partType::head || inputPartIndex == partType::torso)
						{
							death();
						}
						//팔 파괴
						else if (inputPartIndex == partType::larm || inputPartIndex == partType::rarm)
						{

						}
						//다리 파괴
						else if (inputPartIndex == partType::lleg || inputPartIndex == partType::rleg)
						{

						}
						break;
					}
					case bodyTemplateFlag::zombie://좀비일 경우
					{
						//몸 파괴
						if (inputPartIndex == partType::torso)
						{
							death();
						}
						//머리 파괴
						if (inputPartIndex == partType::head)
						{
						}
						//팔 파괴
						else if (inputPartIndex == partType::larm || inputPartIndex == partType::rarm)
						{

						}
						//다리 파괴
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
		//rPCB rFCECR SH EV 등을 업데이트함

		//sh
		entityInfo.sh = entityDex[entityInfo.entityCode].sh;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.sh += equipment->itemInfo[i].sh;
		}

		//ev
		entityInfo.ev = entityDex[entityInfo.entityCode].ev;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.ev += equipment->itemInfo[i].ev;
		}

		//rFire
		entityInfo.rFire = entityDex[entityInfo.entityCode].rFire;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.rFire += equipment->itemInfo[i].rFire;
		}

		//rCold
		entityInfo.rCold = entityDex[entityInfo.entityCode].rCold;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.rCold += equipment->itemInfo[i].rCold;
		}

		//rElec
		entityInfo.rElec = entityDex[entityInfo.entityCode].rElec;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.rElec += equipment->itemInfo[i].rElec;
		}

		//rCorr
		entityInfo.rCorr = entityDex[entityInfo.entityCode].rCorr;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.rCorr += equipment->itemInfo[i].rCorr;
		}

		//rRad
		entityInfo.rRad = entityDex[entityInfo.entityCode].rRad;//기본 개체값으로 재설정
		for (int i = 0; i < equipment->itemInfo.size(); i++)//장비를 기준으로 값 업데이트
		{
			entityInfo.rRad += equipment->itemInfo[i].rRad;
		}
	}
	//입력한 파츠 인덱스의 rPierce를 반환
	int getRPierce(int inputPartIndex)
	{
		int totalRPierce = 0;
		return totalRPierce;
	}
	//입력한 파츠 인덱스의 rCut를 반환
	int getRCut(int inputPartIndex)
	{
		int totalRCut = 0;
		return totalRCut;
	}
	//입력한 파츠 인덱스의 rBash를 반환
	int getRBash(int inputPartIndex)
	{
		int totalRBash = 0;
		return totalRBash;
	}
	//입력한 파츠 인덱스의 SH를 반환
	int getSH()
	{
		int totalSH = 0;

		//기본 개체값 더하기
		totalSH += entityInfo.sh;

		for (int i = 0; i < equipment->itemInfo.size(); i++)
		{
			totalSH += equipment->itemInfo[i].sh;
		}
		return totalSH;
	}
	//입력한 파츠 인덱스의 EV를 반환
	int getEV()
	{
		int totalEV = 0;

		//기본 개체값 더하기
		totalEV += entityInfo.ev;

		for (int i = 0; i < equipment->itemInfo.size(); i++)
		{
			totalEV += equipment->itemInfo[i].ev;
		}
		return totalEV;
	}
	//입력한 파츠 인덱스의 enc를 반환
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
			prt(col::orange, L"[디버그] 공격이 빗나갔다.\n");
		}
		else
		{
			//명중률 계산
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
				prt(col::orange, L"[디버그] 공격이 빗나갔다.\n");
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
	void updateWalkable(int gridX, int gridY)//만약 다를 경우 개체에서 오버라이드해서 쓰시오
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
		//아이템 스택이 이미 있는 경우와 없는 경우
		if (World::ins()->getItemPos(getGridX(), getGridY(), getGridZ()) == nullptr) //그 자리에 템 없는 경우
		{
			//기존 스택이 없으면 새로 만들고 그 ptr을 전달
			targetStack = new ItemStack(getGridX(), getGridY(), getGridZ());
			targetStack->setPocket(txPtr);//스토리지 교체(메모리는 메소드 내부에서 해제됨)
			targetStack->updateSprIndex();
		}
		else //이미 그 자리에 아이템이 있는 경우
		{
			//기존 스택이 있으면 그 스택을 그대로 전달
			targetStack = World::ins()->getItemPos(getGridX(), getGridY(), getGridZ());

			targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

			for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
			}
			delete txPtr; //잊지않고 기존의 매개변수 drop을 제거해준다.
		}
		aniUSet.insert(targetStack);
		turnCycle = turn::playerAnime;

		targetStack->setAniType(aniFlag::drop);
	}
	void throwing(ItemPocket* txPtr, int gridX, int gridY)
	{
		ItemStack* targetStack;
		//아이템 스택이 이미 있는 경우와 없는 경우
		if (World::ins()->getItemPos(gridX, gridY, getGridZ()) == nullptr) //그 자리에 템 없는 경우
		{
			//기존 스택이 없으면 새로 만들고 그 ptr을 전달
			targetStack = new ItemStack(gridX, gridY, getGridZ());
			targetStack->setPocket(txPtr);//스토리지 교체(메모리는 메소드 내부에서 해제됨)
			targetStack->updateSprIndex();
		}
		else //이미 그 자리에 아이템이 있는 경우
		{
			//기존 스택이 있으면 그 스택을 그대로 전달
			targetStack = World::ins()->getItemPos(gridX, gridY, getGridZ());

			targetStack->setTargetSprIndex(targetStack->getSprIndex()); //원래 위치에 가짜 아이템 이미지
			targetStack->setSprIndex(txPtr->itemInfo[0].sprIndex);

			for (int i = txPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				txPtr->transferItem(targetStack->getPocket(), i, txPtr->itemInfo[i].number);
			}
			delete txPtr; //잊지않고 기존의 매개변수 drop을 제거해준다.
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
	//@brief 경험치 테이블과 적성값을 참조하여 입력한 index의 재능레벨을 반환함
	float getTalentLevel(int index)
	{
		float resultLevel = 0;
		for (int i = 18; i >= 0; i--)
		{
			if (talentExp[index] >= expTable[i] / talentApt[index])
			{
				resultLevel = i + 1;
				if (i == 18) { return resultLevel; } //레벨이 18이상일 경우 
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
		//만렙이 된 재능의 포커스 해제
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
		if (getAniType() == aniFlag::move)//만약 플레이어 인스턴스의 좌표와 목적좌표가 다를 경우
		{
			// 1 / 60초마다 runAnimation이 실행됨
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
			case 4://도착
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

			if (shutdown == true)//사망으로 인한 강제종료
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
			//거리에 따라 적이 피격하는데에 걸리는 시간을 길게 만들 것
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

			if (shutdown == true)//사망으로 인한 강제종료
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
			//거리에 따라 적이 피격하는데에 걸리는 시간을 길게 만들 것
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

			if (shutdown == true)//사망으로 인한 강제종료
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
					//명중률 계산 만들기
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

	//메소드를 실행한 객체를 죽이고 아이템을 드랍한다.

	virtual void death() = 0;

	//현재 개체가 보유한 모든 부위를 벡터 형태로 반환한다.
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

	//이 개체가 해당 개체를 공격했을 때 공격한 방법과 상대 부위에 따른 명중률을 확률(0~1.0)으로 반환해줌, aim이 false면 aimStack을 0로 계산 
	float getAimAcc(Entity* victimEntity, int inputPartType, bool aim)
	{
		float aimAcc;
		int victimTargetPartIndex = 0;
		int distance = myMax(abs(getGridX() - victimEntity->getGridX()), abs(getGridY() - victimEntity->getGridY()));

		unsigned __int8 aimStack = 0;
		if (aim == true) { aimStack = getAimStack(); }
		else { aimStack = 0; }

		//입력한 파츠의 종류(팔,다리)를 피해자 개체의 파츠벡터의 index(단순순서)로 변환해줌
		for (int i = 0; i < victimEntity->entityInfo.parts.size(); i++)
		{
			if (victimEntity->entityInfo.parts[i][partsFlag::index] == inputPartType)
			{
				victimTargetPartIndex = i;
			}
		}

		if (getAimWeaponIndex() == -1)//맨손공격
		{
			return aimAcc = calcMelee::acc(victimEntity->entityInfo.parts[victimTargetPartIndex][partsFlag::acc] / 100.0, 0.9, 5, 13.5, 13.5, aimStack);
		}
		else//무기공격
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

	virtual void drawSelf() override
	{
		stepEvent();
		setZoom(zoomScale);
		setFlip(getEntityFlip());

		//prt(L"현재 이 객체의 fake 좌표는 %f,%f이다.\n", getFloatFakeX(), getFloatFakeY());
		int drawingX = (cameraW / 2) + zoomScale * (getX() - cameraX + getFakeX());
		int drawingY = (cameraH / 2) + zoomScale * (getY() - cameraY + getFakeY());

		//캐릭터 그림자 그리기
		drawSpriteCenter(spr::shadow, getSpriteIndex(), drawingX, drawingY);

		//캐릭터 본체 그리기
		drawSpriteCenter(getSprite(), getSpriteIndex(), drawingX, drawingY);


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//캐릭터 커스타미이징 그리기
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
				errorBox(drawOrder.find(priority) != drawOrder.end(), L"이미 존재하는 우선도의 장비가 추가됨 :" + std::to_wstring(priority) + L" 이름: " + getEquipPtr()->itemInfo[equipCounter].name);
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
		if (entityInfo.HP != entityInfo.maxHP)//개체 HP 표기
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

		if (getFlashType() != NULL) //엔티티에 플래시 효과가 있을 경우
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
