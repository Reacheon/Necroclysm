#include <SDL.h>

export module ItemStack;

import std;
import util;
import constVar;
import textureVar;
import ItemPocket;
import Ani;
import Sprite;
import Coord;

export class ItemStack : public Ani, public Coord
{
private:
	Sprite* sprite;
	ItemPocket* storage;
	int sprIndex = 0;
	int targetSprIndex = 0;//던지기 이벤트일 때 타겟의 인덱스
public:
	ItemStack(int gridX, int gridY, int gridZ);
	ItemStack(int gridX, int gridY, int gridZ, std::vector<std::pair<int, int>> inputItems);
	~ItemStack();
	Sprite* getSprite(); 
	void setSprite(Sprite* inputSprite);
	ItemPocket* getPocket();
	int getSprIndex();
	void setSprIndex(int val);
	int getTargetSprIndex();
	void setTargetSprIndex(int val);
	void setPocket(ItemPocket* inputPtr);
	void checkEmpty();
	void updateSprIndex();
	bool runAnimation(bool shutdown);
};