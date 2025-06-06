import Monster;

#include <sol/sol.hpp>

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

Monster::Monster(int index, int gridX, int gridY, int gridZ) : Entity(index, gridX, gridY, gridZ)
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
Monster::~Monster()
{
	prt(lowCol::red, L"Monster : 소멸자가 호출되었습니다..\n");
}

void Monster::startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType)
{
	Entity::startAtk(inputGridX, inputGridY, inputGridZ, inputAniType);
	addAniUSet(this, inputAniType);
}

void Monster::startAtk(int inputGridX, int inputGridY, int inputGridZ) { startAtk(inputGridX, inputGridY, inputGridZ, aniFlag::atk); }

void Monster::endMove()
{
}

bool Monster::runAI()
{
	//prt(L"[Monster]%p의 AI를 실행시켰다.\n", this);
	while (1)
	{
		//prt(lowCol::red, L"[ID : %p]의 timeResource는 %f입니다.\n", this, getTimeResource());
		if (getTurnResource() >= 2.0)
		{
			clearTurnResource();
			addTurnResource(2.0);
		}




		if (getTurnResource() >= 1.2 && entityInfo.entityCode != 5 && entityInfo.relation == relationFlag::hostile)//플레이어에게 직선 경로로 이동
		{
			if (std::abs(PlayerPtr->getGridX() - getGridX()) > 1 || std::abs(PlayerPtr->getGridY() - getGridY()) > 1)//1칸 이내에 있지 않으면
			{
				useTurnResource(1.2);
				prt(lowCol::red, L"[Monster 시간] %f만큼의 시간을 소모했다. 남은 시간은 %f\n", 1.2, getTurnResource());
				prt(lowCol::red, L"[Monster 이동] 이동 인공지능 실행\n");
				int toDir = -1;
				int x1 = getGridX();
				int y1 = getGridY();
				int x2 = PlayerPtr->getGridX();
				int y2 = PlayerPtr->getGridY();
				int xo = x1;
				int yo = y1;
				int delx = std::abs(x2 - x1);
				int dely = std::abs(y2 - y1);
				if (std::fabs(1.0 * dely / delx) < 1)
				{
					int p = 2 * dely - delx;
					if (p < 0)
					{
						if (x2 > xo && y2 >= yo) { x1++; }
						else if (x2 > xo && yo > y2) { x1++; }
						else if (xo > x2 && y2 > yo) { x1--; }
						else { x1--; }
					}
					else
					{
						if (x2 > xo && y2 >= yo) { x1++; y1++; }
						else if (x2 > xo && yo > y2) { x1++; y1--; }
						else if (xo > x2 && y2 > yo) { x1--; y1++; }
						else { x1--; y1--; }
					}
				}
				else if (std::fabs(1.0 * dely / delx) > 1)
				{
					int p = (2 * delx) - dely;
					if (p < 0)
					{
						if (x2 >= xo && y2 > yo) { y1++; }
						else if (x2 > xo && yo > y2) { y1--; }
						else if (xo > x2 && y2 > yo) { y1++; }
						else { y1--; }
					}
					else
					{
						if (x2 >= xo && y2 > yo) { x1++; y1++; }
						else if (x2 > xo && yo > y2) { x1++; y1--; }
						else if (xo > x2 && y2 > yo) { x1--; y1++; }
						else { x1--; y1--; }
					}
				}
				else
				{
					if (x2 > x1 && y2 > y1) { x1++; y1++; }
					else if (x2 > x1 && y1 > y2) { x1++; y1--; }
					else if (x1 > x2 && y2 > y1) { x1--; y1++; }
					else { x1--; y1--; }
				}


				int dx = x1 - xo;
				int dy = y1 - yo;
				if (dx == 1 && dy == 0) { toDir = 0; }
				else if (dx == 1 && dy == -1) { toDir = 1; }
				else if (dx == 0 && dy == -1) { toDir = 2; }
				else if (dx == -1 && dy == -1) { toDir = 3; }
				else if (dx == -1 && dy == 0) { toDir = 4; }
				else if (dx == -1 && dy == 1) { toDir = 5; }
				else if (dx == 0 && dy == 1) { toDir = 6; }
				else { toDir = 7; }


				updateWalkable(getGridX() + dx, getGridY() + dy);
				if (World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).walkable)//이동하려는 위치에 장애물이 없을 경우
				{
					if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).fov == fovFlag::white || World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).fov == fovFlag::white)//P가 보고있으면
					{
						setDirection(toDir);
						move(toDir, false);
						useTurnResource(1.2);
						return false;
					}
					else
					{
						setDirection(toDir);
						move(toDir, true);
					}
				}
				//else//이동하려는 위치에 장애물이 있을 경우
				{
				}
				continue;
			}
		}


		if (getTurnResource() >= 1.3 && entityInfo.entityCode != 5 && entityInfo.relation == relationFlag::hostile)//추적스택이 0보다 크고 플레이어가 1칸 이내에 잇을 경우 평타를 날림
		{
			if (std::abs(PlayerPtr->getGridX() - getGridX()) <= 1 && std::abs(PlayerPtr->getGridY() - getGridY()) <= 1)//1칸 이내에 있으면
			{
				useTurnResource(1.3);
				prt(lowCol::red, L"[Monster 시간] %f만큼의 시간을 소모했다. 남은 시간은 %f\n", 1.3, getTurnResource());
				prt(lowCol::red, L"[Monster 공격] 적이 당신을 강타했다!\n");
				setDirection(coord2Dir(PlayerPtr->getGridX() - getGridX(), PlayerPtr->getGridY() - getGridY()));
				startAtk(PlayerPtr->getGridX(), PlayerPtr->getGridY(), PlayerPtr->getGridZ());
				return false;
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//위의 모든 패턴 조건을 만족하지않을시 return true
		//prt(lowCol::red, L"[Monster] AI가 true를 반환했다. AI를 종료합니다.\n");
		return true;
	}
}

void Monster::death()
{
	if (aniUSet.find(this) != aniUSet.end())
	{
		prt(lowCol::red, L"[Monster] runAnimation 강제 종료\n");
		runAnimation(true);
	}
	new Corpse(corpseType::bloodM, getX(), getY(), getGridZ());
	std::unique_ptr<ItemPocket> dropItems = std::make_unique<ItemPocket>(storageType::temp);
	dropItems.get()->addItemFromDex(entityInfo.corpseItemCode, 1);
	if(entityInfo.sprFlip) dropItems->itemInfo[0].sprIndex++;
	drop(dropItems.get());
	World::ins()->getTile({ getGridX(),getGridY(),getGridZ() }).EntityPtr.reset();
}
