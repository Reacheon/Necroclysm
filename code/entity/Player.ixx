#include <SDL3/SDL.h>

export module Player;

import std;
import util;
import Entity;
import constVar;

export class Player :public Entity //플레이어는 엔티티를 상속받고 시야에 따라 미니맵을 업데이트하는 기능을 가지고 있다.
{
public:
	int headHP = 100;
	int lArmHP = 100;
	int rArmHP = 100;
	int lLegHP = 100;
	int rLegHP = 100;

	int headFakeHP = 100;
	int lArmFakeHP = 100;
	int rArmFakeHP = 100;
	int lLegFakeHP = 100;
	int rLegFakeHP = 100;

	int headFakeHPAlpha = 255;
	int lArmFakeHPAlpha = 255;
	int rArmFakeHPAlpha = 255;
	int lLegFakeHPAlpha = 255;
	int rLegFakeHPAlpha = 255;

	Player(int gridX, int gridY, int gridZ);//생성자입니다.
	~Player();
	virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType) override;
	void startAtk(int inputGridX, int inputGridY, int inputGridZ);
	void startMove(int inputDir);
	void updateMinimap();
	void updateVision(int range, int cx, int cy);
	void updateVision(int range);
	void updateVision();
	void updateNearbyChunk(int range);
	void setGrid(int inputGridX, int inputGridY, int inputGridZ) override;
	void endMove() override; //aStar로 인해 이동이 끝났을 경우
	void death();
	int checkItemSur(int index);//주변에 있는 타일을 포함해 아이템을 가지고 있는지 조사
	void eraseItemSur(int index, int number); //주변객체를 중심으로 총 9칸
	int checkToolQualitySur(int index); //없으면 0 반환, 있으면 공구레벨 반환

	int getResPierceTorso();
	int getResCutTorso();
	int getResBashTorso();

	int getResPierceHead();
	int getResCutHead();
	int getResBashHead();

	int getResPierceLArm();
	int getResCutLArm();
	int getResBashLArm();

	int getResPierceRArm();
	int getResCutRArm();
	int getResBashRArm();

	int getResPierceLLeg();
	int getResCutLLeg();
	int getResBashLLeg();

	int getResPierceRLeg();
	int getResCutRLeg();
	int getResBashRLeg();

	int getSH();
	int getEV();
	int getResFire();
	int getResCold();
	int getResElec();
	int getResCorr();
	int getResRad();

	int getEncTorso();
	int getEncHead();
	int getEncLArm();
	int getEncRArm();
	int getEncLLeg();
	int getEncRLeg();

	void takeDamageHead(int val, dmgFlag inputType)
	{
		takeDamage(val, inputType);
	}
};