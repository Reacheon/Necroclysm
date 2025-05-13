#include <SDL.h>

import ItemStack;
import std;
import util;
import globalVar;
import constVar;
import wrapVar;
import textureVar;
import World;
import ItemPocket;
import ItemData;
import Player;
import Ani;
import Sprite;
import Coord;

ItemStack::ItemStack(Point3 inputCoor)
{
	storage = std::make_unique<ItemPocket>(storageType::stack);
	setGrid(inputCoor.x, inputCoor.y, inputCoor.z);
	setSprite(spr::itemset);
}

ItemStack::ItemStack(Point3 inputCoor, std::vector<std::pair<int, int>> inputItems)
{
	storage = std::make_unique<ItemPocket>(storageType::stack);
	setGrid(inputCoor.x, inputCoor.y, inputCoor.z);
	setSprite(spr::itemset);

	for (int i = 0; i < inputItems.size(); i++) getPocket()->addItemFromDex(inputItems[i].first, inputItems[i].second);
}

ItemStack::~ItemStack()
{
	prt(L"ItemStack : 소멸자가 호출되었습니다..\n");
}
Sprite* ItemStack::getSprite()
{
	return sprite;
}
void ItemStack::setSprite(Sprite* inputSprite)
{
	sprite = inputSprite;
}
ItemPocket* ItemStack::getPocket()
{
	return storage.get();
}

int ItemStack::getSprIndex() { return sprIndex; }
void ItemStack::setSprIndex(int val) { sprIndex = val; }
int ItemStack::getTargetSprIndex() { return targetSprIndex; }
void ItemStack::setTargetSprIndex(int val) { targetSprIndex = val; }

//@brief 자기가 담당하는 storage가 사이즈가 0일 경우 스스로 제거함
void ItemStack::checkEmpty()
{
	if (storage->itemInfo.size() == 0)
	{
		delete this;
	}
}
//@brief 현재 가지고 있는 스토리지에서 대표 sprIndex를 결정함
void ItemStack::updateSprIndex()
{
	//일단 0번째 아이템을 대표로 하도록 설정(임시)
	if (getPocket()->itemInfo.size() > 0)
	{
		sprIndex = getPocket()->itemInfo[0].sprIndex;
	}
}
void ItemStack::pullStackLights()
{
	for (int i = 0; i < storage.get()->itemInfo.size(); i++)
	{
		if (storage.get()->itemInfo[i].lightPtr != nullptr)
		{
			storage.get()->itemInfo[i].lightPtr.get()->moveLight(getGridX(), getGridY(), getGridZ());
		}
	}
}

void ItemStack::pullStackLights(Point3 tgtCoor)
{
	for (int i = 0; i < storage.get()->itemInfo.size(); i++)
	{
		if (storage.get()->itemInfo[i].lightPtr != nullptr)
		{
			storage.get()->itemInfo[i].lightPtr.get()->moveLight(tgtCoor.x, tgtCoor.y, tgtCoor.z);
		}
	}
}

bool ItemStack::runAnimation(bool shutdown)
{
	//prt(L"ItemStack %p의 runAnimation이 실행되었다.\n",this);
	auto aniTyhpe = getAniType();
	if (getAniType() == aniFlag::drop)
	{
		addTimer();
		int pX = getX();
		int pY = getY();

		switch (getTimer())
		{
		case 1:
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
	else if (getAniType() == aniFlag::throwing)
	{
		addTimer();

		float spd = 6;
		float xSpd, ySpd;
		int relX = getIntegerFakeX();
		int relY = getIntegerFakeY();
		float dist = sqrt(pow(relX, 2) + pow(relY, 2));
		prt(L"[전]현재 fake는 (%d,%d)\n", getIntegerFakeX(), getIntegerFakeY());
		static Point3 prevCoor;

		if (getTimer() == 1) prevCoor = getClosestGridWithFake();

		float cosVal = -relX / dist;
		float sinVal = -relY / dist;
		xSpd = spd * cosVal;
		ySpd = spd * sinVal;

		setFakeX(getFakeX() + xSpd);
		setFakeY(getFakeY() + ySpd);

		if (xSpd > 0 && getFakeX() > 0) { setFakeX(0); }
		if (xSpd < 0 && getFakeX() < 0) { setFakeX(0); }
		if (ySpd > 0 && getFakeY() > 0) { setFakeY(0); }
		if (ySpd < 0 && getFakeY() < 0) { setFakeY(0); }


		Point3 cGrid = getClosestGridWithFake();
		if (cGrid != prevCoor)
		{
			prevCoor = cGrid;
			pullStackLights(cGrid);
			PlayerPtr->updateVision();
		}

		if (getIntegerFakeX() == 0 && getIntegerFakeY() == 0)//도착
		{
			for (int i = 0; i < storage->itemInfo.size(); i++)//만약 탄두가 있으면 그걸 납으로 바꿈
			{
				if (storage->itemInfo[i].itemCode == 25)
				{
					storage->eraseItemInfo(i);
					storage->addItemFromDex(11, 1);
					updateSprIndex();
				}
			}

			setFakeX(0);
			setFakeY(0);
			prt(L"애니메이션 종료\n");
			resetTimer();
			setAniType(aniFlag::null);
			pullStackLights();
			return true;
		}
	}
	return false;
}