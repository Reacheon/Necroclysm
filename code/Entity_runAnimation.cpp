﻿import util;
import Entity;
import globalVar;
import wrapVar;
import constVar;
import textureVar;
import turnWait;
import World;
import Sticker;
import Flame;
import Player;
import Prop;

import Bullet;
import Aim;
import TileData;
import ItemData;
import ItemStack;
import Particle;

bool Entity::runAnimation(bool shutdown)
{
	//if (isPlayer) prt(L"Player의 runAnimation이 실행되었다.\n");
	//else prt(L"Entity %p의 runAnimation이 실행되었다.\n",this);

	if (getAniType() == aniFlag::move)//만약 플레이어 인스턴스의 좌표와 목적좌표가 다를 경우
	{
		//8프레임-4(스피드2)
		//6프레임-3(스피드3)
		//4프레임-2(스피드4)

		// 1 / 60초마다 runAnimation이 실행됨

		addTimer();

		if (getTimer() == 1)
		{
			footChanged = false;
		}

		if (getFakeX() > 0)
		{
			addFakeX(-gridMoveSpd);
			if (getFakeX() < 0) setFakeX(0);
		}
		else if (getFakeX() < 0)
		{
			addFakeX(+gridMoveSpd);
			if (getFakeX() > 0) setFakeX(0);
		}

		if (getFakeY() > 0)
		{
			addFakeY(-gridMoveSpd);
			if (getFakeY() < 0) setFakeY(0);
		}
		else if (getFakeY() < 0)
		{
			addFakeY(+gridMoveSpd);
			if (getFakeY() > 0) setFakeY(0);
		}


		if (isPlayer)
		{
			cameraFix = false;
			cameraX = getX() + getIntegerFakeX();
			cameraY = getY() + getIntegerFakeY();
		}

		if (std::abs(getIntegerFakeX()) <= 8.0 || std::abs(getIntegerFakeY()) <= 8.0)
		{
			if (footChanged == false)
			{
				if (getLeftFoot() == true)
				{
					sprState = sprFlag::walkLeft;
					setSpriteIndex(1);
					setLeftFoot(false);
				}
				else
				{
					sprState = sprFlag::walkRight;
					setSpriteIndex(2);
					setLeftFoot(true);
				}
				footChanged = true;
			}
		}

		if (std::abs(getIntegerFakeX()) == 0.0 && std::abs(getIntegerFakeY()) == 0.0)
		{
			setSpriteIndex(0);
			resetTimer();
			setAniType(aniFlag::null);
			setFakeX(0);
			setFakeY(0);

			sprState = sprFlag::stand;
			turnWait(1.0);
			endMove();
			if (isPlayer) cameraFix = true;
			return true;
		}
	}
	else if (getAniType() == aniFlag::atk)
	{
		addTimer();

		char dx;
		char dy;

		static bool dualAtk = false;
		static bool leftAtk = false;
		static bool rightAtk = false;
		static bool twoHandedAtk = false;
		static bool unarmedAtk = false;
		static equipHandFlag prevDualAtk = equipHandFlag::right;
		

		if (getTimer() == 1)
		{
			if (isPlayer)
			{
				dualAtk = false;
				leftAtk = false;
				rightAtk = false;
				twoHandedAtk = false;
				unarmedAtk = false;

				bool findLeft = false;
				bool findRight = false;
				bool findTwoHanded = false;

				auto& equip = PlayerPtr->getEquipPtr()->itemInfo;
				for (int i = 0; i < equip.size(); i++)
				{
					if (equip[i].equipState == equipHandFlag::left && !equip[i].checkFlag(itemFlag::SHIELD)) findLeft = true;
					else if (equip[i].equipState == equipHandFlag::right && !equip[i].checkFlag(itemFlag::SHIELD)) findRight = true;
					else if (equip[i].equipState == equipHandFlag::both) findTwoHanded = true;
				}

				if (findLeft == true && findRight == true) dualAtk = true;
				else if (findLeft == true) leftAtk = true;
				else if (findRight == true) rightAtk = true;
				else if (findTwoHanded == true) twoHandedAtk = true;
				else unarmedAtk = true;
			}
		}


		switch (direction)
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

		Entity* address = TileEntity(atkTarget.x, atkTarget.y, atkTarget.z);
		std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

		if (shutdown == true)//사망으로 인한 강제종료
		{
			if (address != nullptr) address->flash.a = 0;
			aniUSet.erase(aniUSet.find(this));
			delete(((Sticker*)(StickerList.find(stickerID))->second));
			setFakeX(0);
			setFakeY(0);
			return true;
		}

		switch (getTimer())
		{
		case 1:
			if (isPlayer)
			{
				if (dualAtk)
				{
					if (prevDualAtk == equipHandFlag::right) PlayerPtr->setSpriteIndex(charSprIndex::RATK1);
					else  PlayerPtr->setSpriteIndex(charSprIndex::LATK1);
				}
				else if (leftAtk) PlayerPtr->setSpriteIndex(charSprIndex::LATK1);
				else if (rightAtk) PlayerPtr->setSpriteIndex(charSprIndex::RATK1);
				else if (twoHandedAtk) PlayerPtr->setSpriteIndex(charSprIndex::MINING1);
				else if (unarmedAtk) PlayerPtr->setSpriteIndex(charSprIndex::LATK1);
			}
			else sprState = sprFlag::attack1;
			break;
		case 3:
			setFakeX(getFakeX() + 2.5 * dx);
			setFakeY(getFakeY() + 2.5 * dy);
			break;
		case 4:
			setFakeX(getFakeX() + 2.0 * dx);
			setFakeY(getFakeY() + 2.0 * dy);
			break;
		case 5:
			setFakeX(getFakeX() + 1.5 * dx);
			setFakeY(getFakeY() + 1.5 * dy);
			break;
		case 6:
			setFakeX(getFakeX() + 1.0 * dx);
			setFakeY(getFakeY() + 1.0 * dy);
			break;
		case 7:
			setFakeX(getFakeX() + 0.5 * dx);
			setFakeY(getFakeY() + 0.5 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() + 2 * dx);
				address->setFakeY(address->getIntegerFakeY() + 2 * dy);
			}
			attack(atkTarget.x, atkTarget.y);
			new Sticker(false, getX() + (16 * (atkTarget.x - getGridX())), getY() + (16 * (atkTarget.y - getGridY())), spr::effectCut1, 0, stickerID, true);
			if (isPlayer)
			{
				if (dualAtk)
				{
					if (prevDualAtk == equipHandFlag::right)
					{
						PlayerPtr->setSpriteIndex(charSprIndex::RATK2);
						prevDualAtk = equipHandFlag::left;
					}
					else
					{
						PlayerPtr->setSpriteIndex(charSprIndex::LATK2);
						prevDualAtk = equipHandFlag::right;
					}
				}
				else if (leftAtk) PlayerPtr->setSpriteIndex(charSprIndex::LATK2);
				else if (rightAtk) PlayerPtr->setSpriteIndex(charSprIndex::RATK2);
				else if (twoHandedAtk) PlayerPtr->setSpriteIndex(charSprIndex::MINING2);
				else if (unarmedAtk) PlayerPtr->setSpriteIndex(charSprIndex::LATK2);
			}
			else sprState = sprFlag::attack2;
			break;
		case 9:
			setFakeX(getFakeX() - 0.5 * dx);
			setFakeY(getFakeY() - 0.5 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() - 1 * dx);
				address->setFakeY(address->getIntegerFakeY() - 1 * dy);
			}
			break;
		case 10:
			setFakeX(getFakeX() - 1.0 * dx);
			setFakeY(getFakeY() - 1.0 * dy);
			break;
		case 11:
			setFakeX(getFakeX() - 1.5 * dx);
			setFakeY(getFakeY() - 1.5 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() - 1 * dx);
				address->setFakeY(address->getIntegerFakeY() - 1 * dy);
			}
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
			break;
		case 12:
			setFakeX(getFakeX() - 2.0 * dx);
			setFakeY(getFakeY() - 2.0 * dy);
			break;
		case 13:
			setFakeX(getFakeX() - 2.5 * dx);
			setFakeY(getFakeY() - 2.5 * dy);
			break;
		case 15:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
			break;
		case 17:
			delete(((Sticker*)(StickerList.find(stickerID))->second));
		case 19:
			setFakeX(0);
			setFakeY(0);
			resetTimer();
			setAniType(aniFlag::null);
			if (isPlayer)
			{
				PlayerPtr->setSpriteIndex(charSprIndex::WALK);
			}
			else sprState = sprFlag::stand;
			if (isPlayer == true) { turnWait(endAtk()); }
			else { endAtk(); }
			return true;
		}

	}
	else if (getAniType() == aniFlag::felling)
	{
		addTimer();

		char dx;
		char dy;

		switch (direction)
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

		Prop* address = TileProp(PlayerX()+dx, PlayerY()+dy, PlayerZ());
		std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

		if (shutdown == true)//사망으로 인한 강제종료
		{
			aniUSet.erase(aniUSet.find(this));
			delete(((Sticker*)(StickerList.find(stickerID))->second));
			setFakeX(0);
			setFakeY(0);
			return true;
		}

		switch (getTimer())
		{
		case 1:
			PlayerPtr->setSpriteIndex(charSprIndex::MINING1);
			break;
		case 3:
			setFakeX(getFakeX() + 2.5 * dx);
			setFakeY(getFakeY() + 2.5 * dy);
			break;
		case 4:
			setFakeX(getFakeX() + 2.0 * dx);
			setFakeY(getFakeY() + 2.0 * dy);
			break;
		case 5:
			setFakeX(getFakeX() + 1.5 * dx);
			setFakeY(getFakeY() + 1.5 * dy);
			break;
		case 6:
			setFakeX(getFakeX() + 1.0 * dx);
			setFakeY(getFakeY() + 1.0 * dy);
			break;
		case 7:
			setFakeX(getFakeX() + 0.5 * dx);
			setFakeY(getFakeY() + 0.5 *dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() + 2 * dx);
				address->setFakeY(address->getIntegerFakeY() + 2 * dy);
			}
			PlayerPtr->setSpriteIndex(charSprIndex::MINING2);
			address->leadItem.propHP -= 180;
			address->displayHPBarCount = 100;
			address->alphaHPBar = 255;
			address->alphaFakeHPBar = 255;

			if (address->leadItem.propHP <= 0)
			{
				if (address->leadItem.checkFlag(itemFlag::STUMP) == false)
				{
					addAniUSetPlayer(address, aniFlag::treeFalling);
					address->displayHPBarCount = 50;
					address->leadItem.eraseFlag(itemFlag::PROP_BLOCKER);
					PlayerPtr->updateVision(PlayerPtr->eyeSight);
				}
				else
				{
					address->displayHPBarCount = 50;
				}
			}

			new Sticker(false, getX() + (16 * dx), getY() + (16 * dy), spr::effectCut1, 0, stickerID, true);
			break;
		case 9:
			setFakeX(getIntegerFakeX() - 0.5 * dx);
			setFakeY(getIntegerFakeY() - 0.5 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() - 1 * dx);
				address->setFakeY(address->getIntegerFakeY() - 1 * dy);
			}
			break;
		case 10:
			setFakeX(getFakeX() - 1.0 * (float)dx);
			setFakeY(getIntegerFakeY() - 1.0 * (float)dy);
			break;
		case 11:
			setFakeX(getFakeX() - 1.5 * dx);
			setFakeY(getFakeY() - 1.5 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() - 1 * dx);
				address->setFakeY(address->getIntegerFakeY() - 1 * dy);
			}
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
			break;
		case 12:
			setFakeX(getFakeX() - 2.0 * (float)dx);
			setFakeY(getFakeY() - 2.0 * (float)dy);
			break;
		case 13:
			setFakeX(getFakeX() - 2.5 * dx);
			setFakeY(getFakeY() - 2.5 * dy);
			break;
		case 15:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
			break;
		case 17:
			delete(((Sticker*)(StickerList.find(stickerID))->second));
		case 19:
			setFakeX(0);
			setFakeY(0);
			resetTimer();
			setAniType(aniFlag::null);
			PlayerPtr->setSpriteIndex(charSprIndex::WALK_2H);
			if (isPlayer == true) { turnWait(1.0); }
			else {  }
			return true;
		}
	}
	else if (getAniType() == aniFlag::miningWall)
	{
		addTimer();

		char dx;
		char dy;

		switch (direction)
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

		std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

		if (shutdown == true)//사망으로 인한 강제종료
		{
			aniUSet.erase(aniUSet.find(this));
			delete(((Sticker*)(StickerList.find(stickerID))->second));
			setFakeX(0);
			setFakeY(0);
			return true;
		}

		TileData& tile = World::ins()->getTile(PlayerX() + dx, PlayerY() + dy, PlayerZ());

		switch (getTimer())
		{
		case 1:
			PlayerPtr->setSpriteIndex(charSprIndex::MINING1);
			break;
		case 3:
			setFakeX(getFakeX() + 2.5 * dx);
			setFakeY(getFakeY() + 2.5 * dy);
			break;
		case 4:
			setFakeX(getFakeX() + 2.0 * dx);
			setFakeY(getFakeY() + 2.0 * dy);
			break;
		case 5:
			setFakeX(getFakeX() + 1.5 * dx);
			setFakeY(getFakeY() + 1.5 * dy);
			break;
		case 6:
			setFakeX(getFakeX() + 1.0 * dx);
			setFakeY(getFakeY() + 1.0 * dy);
			break;
		case 7:
			setFakeX(getFakeX() + 0.5 * dx);
			setFakeY(getFakeY() + 0.5 * dy);
			PlayerPtr->setSpriteIndex(charSprIndex::MINING2);
			tile.wallHP -= 40;
			tile.displayHPBarCount = 100;
			tile.alphaHPBar = 255;
			tile.alphaFakeHPBar = 255;

			if (tile.wallHP <= 0)
			{
				tile.displayHPBarCount = 50;


				if (TileWall(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == itemRefCode::dirtWall)
				{
					if(randomRange(0,100)<=25) createItemStack({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }, { {396,1} });
					for (int i = 0; i < 8; i++)
					{
						new Particle(getX() + 16 * dx + randomRange(-3, 3), getY() + 16 * dy + 4 + randomRange(-3, 3), randomRange(24, 31), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
					}
				}
				else if (TileWall(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == itemRefCode::stoneWall)
				{
					if (randomRange(0, 100) <= 25) createItemStack({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }, { {398,1} });
					for (int i = 0; i < 8; i++)
					{
						new Particle(getX() + 16 * dx + randomRange(-3, 3), getY() + 16 * dy + 4 + randomRange(-3, 3), randomRange(8, 15), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
					}
				}
				else if (TileWall(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == itemRefCode::glassWall)
				{
					for (int i = 0; i < 8; i++)
					{
						new Particle(getX() + 16 * dx + randomRange(-3, 3), getY() + 16 * dy + 4 + randomRange(-3, 3), randomRange(32, 39), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
					}
				}
				else if (TileWall(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == itemRefCode::wireFence)
				{
					for (int i = 0; i < 8; i++)
					{
						new Particle(getX() + 16 * dx + randomRange(-3, 3), getY() + 16 * dy + 4 + randomRange(-3, 3), randomRange(40, 47), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
					}
				}
				else
				{
					for (int i = 0; i < 8; i++)
					{
						new Particle(getX() + 16 * dx + randomRange(-3, 3), getY() + 16 * dy + 4 + randomRange(-3, 3), randomRange(8, 15), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
					}
				}

				ItemStack* itemPtr = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
				if(itemPtr != nullptr) addAniUSetPlayer(itemPtr, aniFlag::drop);

				DestroyWall(PlayerX() + dx, PlayerY() + dy, PlayerZ());
				PlayerPtr->updateVision(PlayerPtr->eyeSight);
			}
			new Sticker(false, getX() + (16 * dx), getY() + (16 * dy), spr::effectCut1, 0, stickerID, true);
			break;
		case 9:
			setFakeX(getIntegerFakeX() - 0.5 * dx);
			setFakeY(getIntegerFakeY() - 0.5 * dy);
			break;
		case 10:
			setFakeX(getFakeX() - 1.0 * (float)dx);
			setFakeY(getIntegerFakeY() - 1.0 * (float)dy);
			break;
		case 11:
			setFakeX(getFakeX() - 1.5 * dx);
			setFakeY(getFakeY() - 1.5 * dy);
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
			break;
		case 12:
			setFakeX(getFakeX() - 2.0 * (float)dx);
			setFakeY(getFakeY() - 2.0 * (float)dy);
			break;
		case 13:
			setFakeX(getFakeX() - 2.5 * dx);
			setFakeY(getFakeY() - 2.5 * dy);
			break;
		case 15:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
			break;
		case 17:
			delete(((Sticker*)(StickerList.find(stickerID))->second));
		case 19:
			setFakeX(0);
			setFakeY(0);
			resetTimer();
			setAniType(aniFlag::null);
			PlayerPtr->setSpriteIndex(charSprIndex::WALK_2H);
			if (isPlayer == true) { turnWait(1.0); }
			else {}
			return true;
		}
	}
	else if (getAniType() == aniFlag::throwing)
	{
		//거리에 따라 적이 피격하는데에 걸리는 시간을 길게 만들 것
		addTimer();

		char dx;
		char dy;

		switch (direction)
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

		Entity* address = TileEntity(atkTarget.x, atkTarget.y, atkTarget.z);
		std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

		if (shutdown == true)//사망으로 인한 강제종료
		{
			if (address != nullptr) address->flash.a = 0;
			aniUSet.erase(aniUSet.find(this));
			delete(((Sticker*)(StickerList.find(stickerID))->second));
			setFakeX(0);
			setFakeY(0);
			return true;
		}

		switch (getTimer())
		{
		case 2:
			setFakeX(getIntegerFakeX() + 3 * dx);
			setFakeY(getIntegerFakeY() + 3 * dy);
			break;
		case 3:
			setFakeX(getIntegerFakeX() + 2 * dx);
			setFakeY(getIntegerFakeY() + 2 * dy);
			break;
		case 4:
			setFakeX(getIntegerFakeX() + 1 * dx);
			setFakeY(getIntegerFakeY() + 1 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() + 2 * dx);
				address->setFakeY(address->getIntegerFakeY() + 2 * dy);
			}

			attack(atkTarget.x, atkTarget.y);
			new Sticker(false, getX() + (16 * (atkTarget.x - getGridX())), getY() + (16 * (atkTarget.y - getGridY())), spr::effectCut1, 0, stickerID, true);
			break;
		case 5:
			setFakeX(getIntegerFakeX() - 1 * dx);
			setFakeY(getIntegerFakeY() - 1 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() - 1 * dx);
				address->setFakeY(address->getIntegerFakeY() - 1 * dy);
			}
			break;
		case 6:
			setFakeX(getIntegerFakeX() - 2 * dx);
			setFakeY(getIntegerFakeY() - 2 * dy);
			if (address != nullptr)
			{
				address->setFakeX(address->getIntegerFakeX() - 1 * dx);
				address->setFakeY(address->getIntegerFakeY() - 1 * dy);
			}
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
			break;
		case 7:
			setFakeX(getIntegerFakeX() - 3 * dx);
			setFakeY(getIntegerFakeY() - 3 * dy);
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


	}
	else if (getAniType() == aniFlag::shotSingle)
	{
		addTimer();
		static Bullet* bulletPtr = nullptr;

		int dx, dy;
		dir2Coord(direction, dx, dy);

		Entity* ePtr = TileEntity(atkTarget.x, atkTarget.y, atkTarget.z);
		std::wstring stickerID = L"BASEATK" + std::to_wstring((unsigned __int64)this);

		if (shutdown == true)//사망으로 인한 강제종료
		{
			if (ePtr != nullptr) ePtr->flash.a = 0;
			aniUSet.erase(aniUSet.find(this));
			delete(((Sticker*)(StickerList.find(stickerID))->second));
			setFakeX(0);
			setFakeY(0);
			return true;
		}

		float spd = 7.0;
		float delX = 16.0 * (atkTarget.x - getGridX());
		float delY = 16.0 * (atkTarget.y - getGridY());
		float dist = std::sqrt(std::pow(delX, 2) + std::pow(delY, 2));
		float cosVal = delX / dist;
		float sinVal = delY / dist;
		float xSpd = spd * cosVal;
		float ySpd = spd * sinVal;
		static int hitTimer = -1;
		if (getTimer() == 1)
		{
			bulletPtr = new Bullet(getX(), getY());
			bulletPtr->sprite = spr::bulletset;
			bulletPtr->sprIndex = 0 + del2Dir(delX, delY);
		}

		if (bulletPtr != nullptr)
		{
			bulletPtr->addFakeX(xSpd);
			bulletPtr->addFakeY(ySpd);


			std::wprintf(L"bullet fake X:%f, fake Y:%f, delX :%f, delY: %f\n", bulletPtr->getFakeX(), bulletPtr->getFakeY(), delX, delY);
			if (std::fabs(bulletPtr->getFakeX()) >= std::fabs(delX) && std::fabs(bulletPtr->getFakeY()) >= std::fabs(delY))
			{
				delete bulletPtr;
				bulletPtr = nullptr;
				hitTimer = getTimer();
			}
		}


		switch (getTimer())
		{
		case 2:

			setFakeX(getIntegerFakeX() - 2 * dx);
			setFakeY(getIntegerFakeY() - 2 * dy);
			break;
		case 3:
			setFakeX(getIntegerFakeX() - 1 * dx);
			setFakeY(getIntegerFakeY() - 1 * dy);
			break;
		case 5:
			setFakeX(getIntegerFakeX() + 1 * dx);
			setFakeY(getIntegerFakeY() + 1 * dy);
			break;
		case 6:
			setFakeX(getIntegerFakeX() + 2 * dx);
			setFakeY(getIntegerFakeY() + 2 * dy);
			break;
		}

		if (bulletPtr == nullptr)
		{
			if (getTimer() == hitTimer)
			{
				if (ePtr != nullptr)
				{
					ePtr->setFakeX(ePtr->getIntegerFakeX() + 2 * dx);
					ePtr->setFakeY(ePtr->getIntegerFakeY() + 2 * dy);
				}
				attack(atkTarget.x, atkTarget.y);
				new Sticker(false, getX() + (16 * (atkTarget.x - getGridX())), getY() + (16 * (atkTarget.y - getGridY())), spr::effectCut1, 0, stickerID, true);
			}
			else if (getTimer() == hitTimer + 1)
			{
				if (ePtr != nullptr)
				{
					ePtr->setFakeX(ePtr->getIntegerFakeX() - 1 * dx);
					ePtr->setFakeY(ePtr->getIntegerFakeY() - 1 * dy);
				}
			}
			else if (getTimer() == hitTimer + 2)
			{
				if (ePtr != nullptr)
				{
					ePtr->setFakeX(ePtr->getIntegerFakeX() - 1 * dx);
					ePtr->setFakeY(ePtr->getIntegerFakeY() - 1 * dy);
				}
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
			}
			else if (getTimer() == hitTimer + 4)
			{
				((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);
			}
			else if (getTimer() == hitTimer + 6)
			{
				delete(((Sticker*)(StickerList.find(stickerID))->second));
			}
			else if (getTimer() == hitTimer + 8)
			{
				setFakeX(0);
				setFakeY(0);
				resetTimer();
				setAniType(aniFlag::null);

				if (Aim::ins() != nullptr)
				{
					Aim::ins()->close(aniFlag::null);
				}

				if (isPlayer == true) { turnWait(endAtk()); }
				else { endAtk(); }
				return true;
			}

		}
	}
	else if (getAniType() == aniFlag::fireStorm)
	{
		addTimer();

		std::wstring stickerID = L"FIRESTORM";

		switch (getTimer())
		{
		case 1:
		{
			new Sticker(false, getX() + (16 * (getSkillTarget().x - getGridX())), getY() + (16 * (getSkillTarget().y - getGridY())), spr::fireStorm, 0, stickerID, true);
			createFlame({ getSkillTarget().x,getSkillTarget().y,getGridZ() }, flameFlag::BIG);
			break;
		}
		case 5:
		{
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(1);
			
			createFlame({ getSkillTarget().x + 1, getSkillTarget().y, getGridZ() }, flameFlag::NORMAL);
			createFlame({ getSkillTarget().x - 1, getSkillTarget().y, getGridZ() }, flameFlag::NORMAL);
			createFlame({ getSkillTarget().x, getSkillTarget().y + 1, getGridZ() }, flameFlag::NORMAL);
			createFlame({ getSkillTarget().x, getSkillTarget().y - 1, getGridZ() }, flameFlag::NORMAL);
			break;
		}
		case 9:
		{
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(2);

			createFlame({ getSkillTarget().x, getSkillTarget().y - 2, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x - 1, getSkillTarget().y - 1, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x - 2, getSkillTarget().y, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x - 1, getSkillTarget().y + 1, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x, getSkillTarget().y + 2, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x + 1, getSkillTarget().y + 1, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x + 2, getSkillTarget().y, getGridZ() }, flameFlag::SMALL);
			createFlame({ getSkillTarget().x + 1, getSkillTarget().y - 1, getGridZ() }, flameFlag::SMALL);
			break;
		}
		case 13:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(3);
			break;
		case 17:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(4);
			break;
		case 21:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(5);
			break;
		case 25:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(6);
			break;
		case 29:
			((Sticker*)(StickerList.find(stickerID))->second)->setSpriteIndex(7);
			break;
		case 33:
			delete(((Sticker*)(StickerList.find(stickerID))->second));
			resetTimer();
			setAniType(aniFlag::null);
			return true;
			break;
		}
	}
	else if (getAniType() == aniFlag::entityThrow)
	{
		addTimer();
		
		static int arriveTimer = 0;

		Point3 dstGrid = { throwCoord.x,throwCoord.y,throwCoord.z };
		static Point3 prevCoor;

		errorBox(throwingItemPocket->itemInfo.size() > 1, L"2개 이상의 아이템을 동시에 던질 수 없다!");
        errorBox(throwingItemPocket->itemInfo[0].number > 1, L"2개 이상의 아이템을 동시에 던질 수 없다!");

		Sticker* sPtr = nullptr;
		std::wstring stickerID = L"THROW" + std::to_wstring((unsigned __int64)this);
		if (getTimer()==1)
		{
			int gX = getGridX();
            int gY = getGridY();
            int gZ = getGridZ();

			Sticker* sPtr = new Sticker(false, getX(), getY(), spr::itemset, throwingItemPocket->itemInfo[0].sprIndex, stickerID, true);
			arriveTimer = 0;
			prevCoor = { getGridX(),getGridY(),getGridZ() };

		}
		
		float spd = 4.0;
		int relX = 16 * (dstGrid.x - getGridX());
		int relY = 16 * (dstGrid.y - getGridY());
		float dist = std::sqrt(std::pow(relX, 2) + std::pow(relY, 2));
		float xSpd, ySpd;

		if (StickerList.find(stickerID) != StickerList.end()) sPtr = ((Sticker*)(StickerList.find(stickerID))->second);
		else sPtr = nullptr;
		
		if (sPtr != nullptr && arriveTimer == 0 && (relX != 0 || relY != 0))
		{
			float cosVal = relX / dist;
			float sinVal = relY / dist;
			xSpd = spd * cosVal;
			ySpd = spd * sinVal;
			sPtr->addFakeX(xSpd);
			sPtr->addFakeY(ySpd);

			Point3 cGrid = sPtr->getClosestGridWithFake();
			if (cGrid != prevCoor)
			{
				prevCoor = cGrid;
				if (throwingItemPocket->itemInfo[0].lightPtr != nullptr)
				{
					throwingItemPocket->itemInfo[0].lightPtr.get()->moveLight(cGrid.x, cGrid.y, getGridZ());
					PlayerPtr->updateVision();
				}
			}
        }



		if (arriveTimer != 0 || sPtr == nullptr || (relX==0 && relY == 0)|| (std::abs(sPtr->getFakeX()) >= std::abs(relX) && std::abs(sPtr->getFakeY()) >= std::abs(relY)))
		{
			if (arriveTimer == 0) arriveTimer = getTimer();

			ItemStack* targetStack;

			bool throwToProp = false;
            Prop* propPtr = TileProp(dstGrid.x, dstGrid.y, dstGrid.z);
			throwToProp = propPtr != nullptr
				&& propPtr->leadItem.pocketPtr != nullptr
				&& propPtr->leadItem.pocketPtr->getPocketVolume() + throwingItemPocket->itemInfo[0].volume < propPtr->leadItem.pocketMaxVolume;
            
			Vehicle* vPtr = TileVehicle(dstGrid.x, dstGrid.y, dstGrid.z);
			bool throwToVehicle = false;
			ItemPocket* throwTargetPocket = nullptr;
			if (vPtr != nullptr)
			{
				ItemPocket* vParts = vPtr->partInfo[{ dstGrid.x, dstGrid.y }].get();
				for (int i = vParts->itemInfo.size()-1; i >= 0; i--)
				{
					if (vParts->itemInfo[i].pocketMaxVolume > vParts->getPocketVolume() + throwingItemPocket->itemInfo[0].volume)
					{
						throwToVehicle = true;
						throwTargetPocket = vParts->itemInfo[i].pocketPtr.get();
						break;
					}
                }
			}

			if (throwToVehicle)
			{
				static int arriveFakeX = 0;
				static int arriveFakeY = 0;
				if (getTimer() == arriveTimer)
				{
                    arriveFakeX = sPtr->getFakeX();
                    arriveFakeY = sPtr->getFakeY();
					sPtr->setFakeY(arriveFakeY -4);
				}
				else if (getTimer() == arriveTimer + 1) sPtr->setFakeY(arriveFakeY -5);
				else if (getTimer() == arriveTimer + 3) sPtr->setFakeY(arriveFakeY -6);
				else if (getTimer() == arriveTimer + 6) sPtr->setFakeY(arriveFakeY -7);
				else if (getTimer() == arriveTimer + 9) sPtr->setFakeY(arriveFakeY -6);
				else if (getTimer() == arriveTimer + 11) sPtr->setFakeY(arriveFakeY -5);
				else if (getTimer() == arriveTimer + 12) sPtr->setFakeY(arriveFakeY -4);
				else if (getTimer() >= arriveTimer + 15)
				{
					sPtr->setFakeY(arriveFakeY);
					throwingItemPocket->transferItem(throwTargetPocket, 0, 1);

					delete sPtr;
					resetTimer();
					setAniType(aniFlag::null);
					return true;
				}
			}
			else if (throwToProp)
			{
				if (getTimer() == arriveTimer)
				{
					propPtr->setFakeY(-4);
					delete sPtr;
				}
				else if (getTimer() == arriveTimer+1) propPtr->setFakeY(-5);
				else if (getTimer() == arriveTimer+3) propPtr->setFakeY(-6);
				else if (getTimer() == arriveTimer+6) propPtr->setFakeY(-7);
				else if (getTimer() == arriveTimer+9) propPtr->setFakeY(-6);
				else if (getTimer() == arriveTimer+11) propPtr->setFakeY(-5);
				else if (getTimer() == arriveTimer+12) propPtr->setFakeY(-4);
				else if (getTimer() >= arriveTimer+15)
				{
					propPtr->setFakeY(0);
					throwingItemPocket->transferItem(propPtr->leadItem.pocketPtr.get(), 0, 1);
					
					resetTimer();
					setAniType(aniFlag::null);
					return true;
				}
			}
			else if (TileItemStack(dstGrid.x, dstGrid.y, dstGrid.z) == nullptr) //그 자리에 템 없는 경우
			{
				//기존 스택이 없으면 새로 만들고 그 ptr을 전달
				
				createItemStack(dstGrid);
				targetStack = TileItemStack(dstGrid);
				throwingItemPocket->transferItem(targetStack->getPocket(), 0, 1);
				targetStack->updateSprIndex();
				addAniUSetPlayer(targetStack, aniFlag::drop);

				delete sPtr;
				resetTimer();
				setAniType(aniFlag::null);
				return true;
			}
			else //이미 그 자리에 아이템이 있는 경우
			{
				//기존 스택이 있으면 그 스택을 그대로 전달
				targetStack = TileItemStack(dstGrid);
				targetStack->setSprIndex(throwingItemPocket->itemInfo[0].sprIndex);
				targetStack->setTargetSprIndex(targetStack->getSprIndex()); //원래 위치에 가짜 아이템 이미지
				throwingItemPocket->transferItem(targetStack->getPocket(), 0, 1);
				addAniUSetPlayer(targetStack, aniFlag::drop);

				delete sPtr;
				resetTimer();
				setAniType(aniFlag::null);
				return true;
			}
		}
	}
	else if (getAniType() == aniFlag::faint)
	{
		addTimer();
		int pX = getX();
		int pY = getY();

		switch (getTimer())
		{
		case 1:
			PlayerPtr->setSpriteIndex(charSprIndex::CRAWL);
			PlayerPtr->eyes = humanCustom::eyes::closed;
			setFakeY(-4);
			break;
		case 2:
			setFakeY(-5);
			break;
		case 4:
			setFakeY(-6);
			break;
		case 7:
			setFakeY(-7);
			break;
		case 10:
			setFakeY(-6);
			break;
		case 12:
			setFakeY(-5);
			break;
		case 13:
			setFakeY(-4);
			break;
		case 16:
			setFakeY(0);
			resetTimer();
			setAniType(aniFlag::null);
			return true;
		}
	}


	return false;
}