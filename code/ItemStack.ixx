#include <SDL.h>

export module ItemStack;

import std;
import util;
import constVar;
import textureVar;
import World;
import ItemPocket;
import Ani;
import Sprite;
import Coord;

export class ItemStack : public Ani, public Coord
{
private:
	Sprite* sprite;
	ItemPocket* storage;
	int sprIndex = 37;
	int targetSprIndex = 0;//던지기 이벤트일 때 타겟의 인덱스
public:
	ItemStack(int gridX, int gridY, int gridZ)
	{
		storage = new ItemPocket(storageType::stack);
		setGrid(gridX, gridY, gridZ);
		World::ins()->setItemPos(getGridX(), getGridY(), getGridZ(), this);
		setSprite(spr::itemset);
	}
	~ItemStack()
	{
		prt(L"ItemStack : 소멸자가 호출되었습니다..\n");
		World::ins()->setItemPos(getGridX(), getGridY(), getGridZ(), nullptr);//itemPos에서 지운다
	}
	Sprite* getSprite()
	{
		return sprite;
	}
	void setSprite(Sprite* inputSprite)
	{
		sprite = inputSprite;
	}
	ItemPocket* getPocket()
	{
		return storage;
	}

	int getSprIndex(){return sprIndex;}
	void setSprIndex(int val){sprIndex = val;}
	int getTargetSprIndex() { return targetSprIndex; }
	void setTargetSprIndex(int val) { targetSprIndex = val; }

	void setPocket(ItemPocket* inputPtr)
	{
		delete storage;
		storage = inputPtr;
	}
	//@brief 자기가 담당하는 storage가 사이즈가 0일 경우 스스로 제거함
	void checkEmpty()
	{
		if (storage->itemInfo.size() == 0)
		{
			delete this;
		}
	}
	//@brief 현재 가지고 있는 스토리지에서 대표 sprIndex를 결정함
	void updateSprIndex()
	{
		//일단 0번째 아이템을 대표로 하도록 설정(임시)
		if (getPocket()->itemInfo.size() >0)
		{
			sprIndex = getPocket()->itemInfo[0].sprIndex;
		}
	}
	bool runAnimation(bool shutdown)
	{
		//prt(L"ItemStack %p의 runAnimation이 실행되었다.\n",this);
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
			int relX = getFakeX();
			int relY = getFakeY();
			float dist = sqrt(pow(relX, 2) + pow(relY, 2));
			prt(L"[전]현재 fake는 (%d,%d)\n", getFakeX(), getFakeY());

			float cosVal = - relX / dist;
			float sinVal = - relY / dist;
			xSpd = spd * cosVal;
			ySpd = spd * sinVal;

			setFloatFakeX(getFloatFakeX() + xSpd);
			setFloatFakeY(getFloatFakeY() + ySpd);

			if (xSpd > 0 && getFloatFakeX() > 0) {setFloatFakeX(0);}
			if (xSpd < 0 && getFloatFakeX() < 0) { setFloatFakeX(0); }
			if (ySpd > 0 && getFloatFakeY() > 0) { setFloatFakeY(0); }
			if (ySpd < 0 && getFloatFakeY() < 0) { setFloatFakeY(0); }

			if (getFakeX() == 0 && getFakeY() == 0)//도착
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

				setFloatFakeX(0);
				setFloatFakeY(0);
				prt(L"애니메이션 종료\n");
				resetTimer();
				setAniType(aniFlag::null);
				return true;
			}
		}
		return false;
	}
};