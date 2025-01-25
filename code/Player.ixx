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

export class Player :public Entity //�÷��̾�� ��ƼƼ�� ��ӹް� �þ߿� ���� �̴ϸ��� ������Ʈ�ϴ� ����� ������ �ִ�.
{
public:
	Player(int gridX, int gridY, int gridZ);//�������Դϴ�.
	~Player();
	static Player* ins()//�̱��� �Լ�
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
	void endMove() override; //aStar�� ���� �̵��� ������ ���
	void death();
	int checkItemSur(int index);//�ֺ��� �ִ� Ÿ���� ������ �������� ������ �ִ��� ����
	void eraseItemSur(int index, int number); //�ֺ���ü�� �߽����� �� 9ĭ
	int checkToolQualitySur(int index); //������ 0 ��ȯ, ������ �������� ��ȯ
};