#include <SDL.h>

export module Player;

import std;
import util;
import Entity;
import World;
import globalVar;
import textureVar;
import constVar;
import log;
import ItemData;
import nanoTimer;
import globalTime;

export class Player :public Entity //플레이어는 엔티티를 상속받고 시야에 따라 미니맵을 업데이트하는 기능을 가지고 있다.
{
public:
	Player(int gridX, int gridY, int gridZ);//생성자입니다.
	~Player();
	static Player* ins()//싱글톤 함수
	{
		static Player* ptr = new Player(0, 0, 0);
		ptr->entityInfo.isPlayer = true;
		return ptr;
	}
	virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType) override;
	void startAtk(int inputGridX, int inputGridY, int inputGridZ);
	void startMove(int inputDir);
	void updateMinimap();
	void updateVision(int range, int cx, int cy);
	void updateVision(int range);
	void updateNearbyChunk(int range);
	void setGrid(int inputGridX, int inputGridY, int inputGridZ) override;
	void endMove() override; //aStar로 인해 이동이 끝났을 경우
	void death();
	int checkItemSur(int index);//주변에 있는 타일을 포함해 아이템을 가지고 있는지 조사
	void eraseItemSur(int index, int number); //주변객체를 중심으로 총 9칸
	int checkToolQualitySur(int index); //없으면 0 반환, 있으면 공구레벨 반환
};