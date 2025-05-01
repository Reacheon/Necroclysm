#include <sol/sol.hpp>

export module Monster;

import std;
import Entity;
import globalVar;
import wrapVar;
import textureVar;
import Player;
import World;
import log;
import Corpse;
import util;
import AI;
import ItemData;

export class Monster : public Entity, public AI
{
private:
	std::string scriptAI;
public:
	Monster(int index, int gridX, int gridY, int gridZ) : Entity(index, gridX, gridY, gridZ)
	{
		if (entityInfo.isHumanCustomSprite == true && entityInfo.isPlayer == false)
		{
			if (entityInfo.entityCode == 6)
			{

				entityInfo.skin = humanCustom::skin::yellow;
				entityInfo.eyes = humanCustom::eyes::red;
				entityInfo.hair = humanCustom::hair::ponytail;
				entityInfo.horn = humanCustom::horn::coverRed;

				int i = 0;
				getEquipPtr()->addItemFromDex(390);
				getEquipPtr()->itemInfo[i++].equipState = equipHandFlag::normal;


				entityInfo.sprFlip = true;

				updateStatus();
				updateCustomSpriteHuman();
			}
		}


		//AI 로드
		{
			std::string filepath = "testAI.lua";
			std::ifstream file(filepath);
			if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);
			std::stringstream buffer;
			buffer << file.rdbuf();
			scriptAI = buffer.str();
		}

		prt(entityInfo.name.c_str());
		prt(lowCol::red, L"\nMonster : 생성자가 호출되었습니다! ID : %p\n", this);
	}
	~Monster()
	{
		prt(lowCol::red, L"Monster : 소멸자가 호출되었습니다..\n");
	}

	virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType) override
	{
		Entity::startAtk(inputGridX, inputGridY, inputGridZ, inputAniType);
		addAniUSet(this, inputAniType);
	}

	void startAtk(int inputGridX, int inputGridY, int inputGridZ) { startAtk(inputGridX, inputGridY, inputGridZ, aniFlag::atk); }

	void endMove()
	{
	}

	bool runAI()
	{
		lua["selfX"] = [this]()->int {return this->getGridX(); };
		lua["selfY"] = [this]()->int {return this->getGridY(); };
		lua["selfZ"] = [this]()->int {return this->getGridZ(); };

		lua["getMyTurn"] = [this]()->int {return this->getTurnResource(); };
		lua["addMyTurn"] = [this](double val)->void { this->addTurnResource(val); };
		lua["clearMyTurn"] = [this]()->void { this->clearTurnResource(); };
		lua["useMyTurn"] = [this](double val)->void { this->useTurnResource(val); };

		lua["isHostile"] = [this]()->bool { return this->entityInfo.relation == relationFlag::hostile; };
		lua["getEntityCode"] = [this]()->int { return this->entityInfo.entityCode; };
		lua["updateWalkable"] = [this](int x, int y)->void { updateWalkable(x, y); };



		lua["setDirection"] = [this](int dir)->void { this->setDirection(dir); };
		lua["moveStart"] = [this](int dir, bool jump)->void { this->move(dir, jump); };
		lua["attackStart"] = [this](int x, int y, int z)->void { this->startAtk(x, y, z); };



		try
		{
			sol::object result = lua.script(scriptAI);
			return result.as<bool>();
		}
		catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }

	}

	void death()
	{
		if (aniUSet.find(this) != aniUSet.end())
		{
			prt(lowCol::red, L"[Monster] runAnimation 강제 종료\n");
			runAnimation(true);
		}
		new Corpse(corpseType::bloodM, getX(), getY(), getGridZ());
		std::unique_ptr<ItemPocket> dropItems = std::make_unique<ItemPocket>(storageType::temp);
		dropItems.get()->addItemFromDex(20, 1);
		drop(dropItems.get());
		delete this;
	}


};
