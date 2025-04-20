#define SNOW(x, y, z) (World::ins()->getTile(x, y, z).hasSnow = true)
#include <sol/sol.hpp>

export module startArea;

import globalVar;
import wrapVar;
import HUD;
import Player;
import ItemData;
import ItemStack;
import World;
import Vehicle;
import Prop;
import Monster;
import TitleScreen;

export void startArea()
{
	new HUD();
	//new TitleScreen();

	Player* playerPtr = Player::ins();
	playerPtr->setGrid(0, 0, 0);
	playerPtr->setDstGrid(0, 0);

	playerPtr->updateWalkable(PlayerX() + 1, PlayerY());
	playerPtr->updateWalkable(PlayerX() + 1, PlayerY() - 1);
	playerPtr->updateWalkable(PlayerX(), PlayerY() - 1);
	playerPtr->updateWalkable(PlayerX() - 1, PlayerY() - 1);
	playerPtr->updateWalkable(PlayerX() - 1, PlayerY());
	playerPtr->updateWalkable(PlayerX() - 1, PlayerY() + 1);
	playerPtr->updateWalkable(PlayerX(), PlayerY() + 1);
	playerPtr->updateWalkable(PlayerX() + 1, PlayerY() + 1);

	//�׽�Ʈ ������
	int pX = PlayerX();
	int pY = PlayerY();
	int pZ = PlayerZ();

	new Monster(6, 5, 0, 0);//NPC

	new ItemStack(2, 1, 0, {
		{2, 1}, {0, 5}, {23, 1}, {24, 10}, {1, 4}, {0, 1},
		{3, 1}, {12, 1}, {13, 1}, {14, 1}, {15, 1}, {16, 1},
		{17, 1}, {18, 1}, {4, 1}, {5, 8}, {88, 1}, {89, 1000},
		{91, 1000}, {82, 1},{389,1}, {386,2}, {387,1}
		}
	);

	//Ȱ�� ����
	new ItemStack(3, 8, 0, { {383,1} });
	new ItemStack(4, 8, 0, { {385,30} });
	new ItemStack(3, 9, 0, { {382,1} });
	new ItemStack(4, 9, 0, { {384,30} });

	new Monster(5, 8, 8, 0);//����ƺ�



	new ItemStack(7, -4, 0, { {391,1} }); //���񵵳�

	new ItemStack(-5, 1, 0, { {394,1} }); //���ô�

	new ItemStack(-3, -4, -1, { {388,1} });//���
	new ItemStack(-2, -4, -1, { {393,1} });//�������
	new ItemStack(-1, -4, -1, { {395,1} });//��



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////������� ������ �߰�////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	availableRecipe = new ItemPocket(storageType::recipe);
	for (int i = 1; i <= 212; i++)
	{
		if (itemDex[i].name != L"?")
		{
			if (itemDex[i].checkFlag(itemFlag::CANCRAFT))
			{
				((ItemPocket*)availableRecipe)->addRecipe(i);
			}
		}
	}

	//Ÿ�� ����
	for (int dx = -30; dx <= 30; dx++)
	{
		for (int dy = -30; dy <= 30; dy++)
		{
			setFloor({ dx,dy,0 }, 109);
		}
	}
	for (int dx = -30; dx <= 30; dx++)
	{
		for (int dy = -30; dy <= -2; dy++)
		{
			setFloor({ dx,dy,0 }, 220);
		}
	}
	for (int dx = -6; dx >= -14; dx--)
	{
		for (int dy = -30; dy <= 30; dy++)
		{
			if (dx == -10 && ((dy + 30) % 6 < 3)) setFloor({ dx,dy,0 }, 377); //����ƽ���Ʈ
			else setFloor({ dx,dy,0 }, 296); //�����ƽ���Ʈ
		}
	}

	//���ֱ�
	setFloor({ -3,-1,0 }, 293);
	setFloor({ -2,-1,0 }, 293);
	setFloor({ -2,0,0 }, 293);
	setFloor({ -1,0,0 }, 293);
	setFloor({ 0,0,0 }, 293);
	setFloor({ 1,0,0 }, 293);
	setFloor({ 2,0,0 }, 293);
	setFloor({ 2,1,0 }, 293);
	setFloor({ 3,1,0 }, 293);
	setFloor({ 4,1,0 }, 293);
	setFloor({ 4,2,0 }, 293);
	for (int i = 0; i < 9; i++) setFloor({ 5 + i,2,0 }, 293);

	//������ ��ġ
	//�� �ϴ� 5Ÿ��
	setWall({ -1,-2,0 }, 375);
	setWall({ -2,-2,0 }, 375);
	new Prop(-3, -2, 0, 291);//������ ��ġ
	setWall({ -4,-2,0 }, 375);
	setWall({ -5,-2,0 }, 375);
	//�� ���� 4Ÿ��
	setWall({ -1,-3,0 }, 375);
	setWall({ -1,-4,0 }, 114);
	setWall({ -1,-5,0 }, 375);
	setWall({ -1,-6,0 }, 375);
	//�� ���� 4Ÿ��
	setWall({ -5,-3,0 }, 375);
	setWall({ -5,-4,0 }, 114);
	setWall({ -5,-5,0 }, 375);
	setWall({ -5,-6,0 }, 375);
	//�� ��� �߾� 3Ÿ��
	setWall({ -2,-6,0 }, 375);
	setWall({ -3,-6,0 }, 375);
	setWall({ -4,-6,0 }, 375);

	new Prop(-4, -5, 0, 295);//å��
	new Prop(-2, -5, 0, 294);//ħ��

	new Prop(-4, -3, 0, 298);//��°��
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			setFloor({ -3 + dx, -4 + dy, 1 }, 292);
		}
	}

	new Prop(-2, -3, 0, 299);//�ϰ����


	//ö����
	for (int i = 0; i < 17; i++)//���
	{
		setWall({ -5 + i, -9, 0 }, 376);
	}

	for (int i = 0; i < 20; i++)//����
	{
		if (-8 + i != 1 && -8 + i != 2 && -8 + i != 3)
		{
			setWall({ 11, -8 + i, 0 }, 376);
		}
	}

	for (int i = 0; i < 17; i++)//���
	{
		setWall({ 11 - i, 12, 0 }, 376);
	}

	//ö���� ���� �Ա� ����� 2��
	new Prop(12, 0, 0, 118);//������
	new Prop(12, 4, 0, 118);//������


	//ö���� �Ʒ� ����

	new Prop(-2, 15, 0, itemVIPCode::railBR);
	for(int i=0; i<11; i++)  new Prop(-1 + i, 15, 0, itemVIPCode::railRL);
	new Prop(10, 15, 0, itemVIPCode::railSwitchWS);
	for (int i = 0; i < 7; i++)  new Prop(10, 16 + i, 0, itemVIPCode::railTB);
	new Prop(10, 23, 0, itemVIPCode::railTL);
	for (int i = 0; i < 6; i++)  new Prop(9-i, 23, 0, itemVIPCode::railRL);
	new Prop(3, 23, 0, itemVIPCode::railTR);
	for (int i = 0; i < 3; i++) new Prop(3, 22 - i, 0, itemVIPCode::railTB);
	new Prop(3, 19, 0, itemVIPCode::railBL);
	for (int i = 0; i < 4; i++) new Prop(2 - i, 19, 0, itemVIPCode::railRL);
	new Prop(-2, 19, 0, itemVIPCode::railTR);
	for (int i = 0; i < 3; i++) new Prop(-2, 18 - i, 0, itemVIPCode::railTB);

	for (int i = 0; i < 5; i++)  new Prop(11 + i, 15, 0, itemVIPCode::railRL);

	//����
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -2; dy <= 2; dy++)
			{
				DestroyWall(-2 + dx, -3 + dy, -1);
			}
		}

		for (int dx = -4; dx >= -20; dx--)
		{
			for (int dy = -6; dy <= 20; dy++)
			{
				setWall({ dx, dy, -1 },397);
			}
		}

		new Prop(pX - 2, pY - 5, -1, 211);//�����
		{
			int cx = -1;
			int cy = -1;

			for (int i = 1; i < 9; i++) DestroyWall(cx, cy + i, -1);
			new Prop(cx, cy + 1, -1, 291);//������ ��ġ

			for (int i = 1; i < 9; i++) DestroyWall(cx + i, cy, -1);
			Prop* door2 = new Prop(cx + 1, cy, -1, 291);//������ ��ġ
			door2->leadItem.extraSprIndexSingle = 2;

			int aisleEndX = cx + 8;
			int aisleEndY = cy;
			for (int dx = -3; dx <= 3; dx++)
			{
				for (int dy = -50; dy <= 40; dy++)
				{
					DestroyWall(aisleEndX + dx, aisleEndY + dy, -1);
				}
			}

			int aisleEndX2 = cx + 8 + 12;
			int aisleEndY2 = cy;
			for (int dx = -4; dx <= 4; dx++)
			{
				for (int dy = -40; dy <= 40; dy++)
				{
					DestroyWall(aisleEndX2 + dx, aisleEndY2 + dy, -1);
				}
			}

			int aisleEndX3 = cx + 8;
			int aisleEndY3 = cy - 14;
			for (int dx = -40; dx <= 40; dx++)
			{
				for (int dy = -3; dy <= 3; dy++)
				{
					DestroyWall(aisleEndX3 + dx, aisleEndY3 + dy, -1);
				}
			}

			int aisleEndX4 = 19;
			int aisleEndY4 = 12;
			for (int dx = -15; dx <= 0; dx++)
			{
				for (int dy = -3; dy <= 3; dy++)
				{
					DestroyWall(aisleEndX4 + dx, aisleEndY4 + dy, -1);
				}
			}


			int cursorX = aisleEndX;
			int cursorY = aisleEndY + 12;

			//for (int i = 0; i <= 78; i++)
			//{
			//	new Prop(6, 39 - i, -1, itemVIPCode::wideRailVLeft);
			//	new Prop(7, 39 - i, -1, itemVIPCode::wideRailVMid);
			//	new Prop(8, 39 - i, -1, itemVIPCode::wideRailVRight);
			//	cursorY--;
			//}

			for (int i = 0; i <= 78; i++)
			{
				new Prop(6, 39 - i, -1, itemVIPCode::wideRailVLeft);
				new Prop(7, 39 - i, -1, itemVIPCode::wideRailVMid);
				new Prop(8, 39 - i, -1, itemVIPCode::wideRailVRight);
				cursorY--;
			}


			//����ö ��ġ
			int vX = 7;
			int vY = -1;
			Vehicle* myTrainPower = new Vehicle(vX, vY, -1, 48);//���� ��ġ
			{
				myTrainPower->name = L"������";
				myTrainPower->vehType = vehFlag::train;
				myTrainPower->isPowerTrain = true;

				///////////////////////���� ���� ������//////////////////////////////////////
				myTrainPower->extendPart(vX, vY - 1, 48);
				myTrainPower->extendPart(vX - 1, vY - 1, 48);
				myTrainPower->extendPart(vX + 1, vY - 1, 48);
				myTrainPower->extendPart(vX + 2, vY - 1, 48);
				myTrainPower->extendPart(vX - 2, vY - 1, 48);

				myTrainPower->extendPart(vX - 1, vY - 2, 48);
				myTrainPower->extendPart(vX, vY - 2, 48);
				myTrainPower->extendPart(vX + 1, vY - 2, 48);
				myTrainPower->extendPart(vX + 2, vY - 2, 48);
				myTrainPower->extendPart(vX - 2, vY - 2, 48);

				myTrainPower->extendPart(vX - 1, vY - 3, 48);
				myTrainPower->extendPart(vX, vY - 3, 48);
				myTrainPower->extendPart(vX + 1, vY - 3, 48);
				myTrainPower->extendPart(vX + 2, vY - 3, 48);
				myTrainPower->extendPart(vX - 2, vY - 3, 48);

				myTrainPower->extendPart(vX - 1, vY, 48);
				myTrainPower->extendPart(vX + 1, vY, 48);
				myTrainPower->extendPart(vX + 2, vY, 48);
				myTrainPower->extendPart(vX - 2, vY, 48);

				for (int i = 1; i < 5; i++)
				{
					myTrainPower->extendPart(vX - 1, vY + i, 48);
					myTrainPower->extendPart(vX, vY + i, 48);
					myTrainPower->extendPart(vX + 1, vY + i, 48);
					myTrainPower->extendPart(vX + 2, vY + i, 48);
					myTrainPower->extendPart(vX - 2, vY + i, 48);
				}

				int topLeftX = vX - 2;
				int topLeftY = vY - 3;

				for (int x = topLeftX; x <= topLeftX + 4; x++)
				{
					for (int y = topLeftY; y <= topLeftY + 7; y++)
					{
						if ((x == topLeftX || x == topLeftX + 4) || (y == topLeftY || y == topLeftY + 7))
						{
							if (x == topLeftX + 2 && y == topLeftY + 7)  myTrainPower->addPart(x, y, { 120 });
							else if (y == topLeftY + 4)  myTrainPower->addPart(x, y, { 120 });
							else if (y == topLeftY) myTrainPower->addPart(x, y, { 121 });
							else myTrainPower->addPart(x, y, { 119 });
						}
						else if ((y == topLeftY + 2))
						{
							if (x == topLeftX + 2) myTrainPower->addPart(x, y, { 120 });
							else myTrainPower->addPart(x, y, { 119 });
						}
						else
						{
							myTrainPower->addPart(x, y, { 122 });
						}
					}
				}

				myTrainPower->addPart(topLeftX + 2, topLeftY + 1, { 313 });


				myTrainPower->addPart(topLeftX + 1, topLeftY + 3, { 123 });
				myTrainPower->addPart(topLeftX + 1, topLeftY + 5, { 123 });
				myTrainPower->addPart(topLeftX + 1, topLeftY + 6, { 123 });

				myTrainPower->addPart(topLeftX + 3, topLeftY + 3, { 123 });
				myTrainPower->addPart(topLeftX + 3, topLeftY + 5, { 123 });
				myTrainPower->addPart(topLeftX + 3, topLeftY + 6, { 123 });
			}

			//����ö(ȭ��ĭ) ��ġ

			Vehicle* myTrain = new Vehicle(7, 7, -1, 48);//���� ��ġ

			{
				int vX = 7;
				int vY = 7;
				myTrain->vehType = vehFlag::train;

				///////////////////////���� ���� ������//////////////////////////////////////
				myTrain->extendPart(vX, vY - 1, 48);
				myTrain->extendPart(vX - 1, vY - 1, 48);
				myTrain->extendPart(vX + 1, vY - 1, 48);
				myTrain->extendPart(vX + 2, vY - 1, 48);
				myTrain->extendPart(vX - 2, vY - 1, 48);

				myTrain->extendPart(vX - 1, vY - 2, 48);
				myTrain->extendPart(vX, vY - 2, 48);
				myTrain->extendPart(vX + 1, vY - 2, 48);
				myTrain->extendPart(vX + 2, vY - 2, 48);
				myTrain->extendPart(vX - 2, vY - 2, 48);

				myTrain->extendPart(vX - 1, vY - 3, 48);
				myTrain->extendPart(vX, vY - 3, 48);
				myTrain->extendPart(vX + 1, vY - 3, 48);
				myTrain->extendPart(vX + 2, vY - 3, 48);
				myTrain->extendPart(vX - 2, vY - 3, 48);

				myTrain->extendPart(vX - 1, vY, 48);
				myTrain->extendPart(vX + 1, vY, 48);
				myTrain->extendPart(vX + 2, vY, 48);
				myTrain->extendPart(vX - 2, vY, 48);

				for (int i = 1; i < 5; i++)
				{
					myTrain->extendPart(vX - 1, vY + i, 48);
					myTrain->extendPart(vX, vY + i, 48);
					myTrain->extendPart(vX + 1, vY + i, 48);
					myTrain->extendPart(vX + 2, vY + i, 48);
					myTrain->extendPart(vX - 2, vY + i, 48);
				}

				int topLeftX = vX - 2;
				int topLeftY = vY - 3;

				for (int x = topLeftX; x <= topLeftX + 4; x++)
				{
					for (int y = topLeftY; y <= topLeftY + 7; y++)
					{
						if ((x == topLeftX || x == topLeftX + 4) || (y == topLeftY || y == topLeftY + 7))
						{
							if (x == topLeftX + 2)  myTrain->addPart(x, y, { 120 });
							else if (y == topLeftY + 4)  myTrain->addPart(x, y, { 120 });
							else myTrain->addPart(x, y, { 119 });
						}
						else
						{
							myTrain->addPart(x, y, { 122 });
						}
					}
				}

				myTrain->addPart(topLeftX + 1, topLeftY + 1, { 123 });
				myTrain->addPart(topLeftX + 1, topLeftY + 2, { 123 });
				myTrain->addPart(topLeftX + 1, topLeftY + 3, { 123 });
				myTrain->addPart(topLeftX + 1, topLeftY + 5, { 123 });
				myTrain->addPart(topLeftX + 1, topLeftY + 6, { 123 });

				myTrain->addPart(topLeftX + 3, topLeftY + 1, { 123 });
				myTrain->addPart(topLeftX + 3, topLeftY + 2, { 123 });
				myTrain->addPart(topLeftX + 3, topLeftY + 3, { 123 });
				myTrain->addPart(topLeftX + 3, topLeftY + 5, { 123 });
				myTrain->addPart(topLeftX + 3, topLeftY + 6, { 123 });
			}
			myTrainPower->rearTrain = myTrain;


			//for (int targetY = endY; targetY >= endY - 19; targetY--)
			//{
			//	new Prop(endX + 3, targetY, pZ - 1, 303);//������ ��ġ
			//}

			//new Prop(endX + 3, endY - 20, pZ - 1, 317);//������ ��ġ

			//for (int targetY = endY - 21; targetY >= endY - 50; targetY--)
			//{
			//	new Prop(endX + 3, targetY, pZ - 1, 303);//������ ��ġ
			//}

			//for (int targetX = endX + 1; targetX >= endX - 30; targetX--)
			//{
			//	new Prop(targetX, endY - 20, pZ - 1, 303);//������ ��ġ
			//}
		}
	}

	//�� �ٴ� Ÿ��
	for (int dx = 0; dx < 5; dx++)
	{
		for (int dy = 0; dy < 4; dy++)
		{
			setFloor({ -5 + dx, -5 + dy, 0 }, 292);
		}
	}
	new Prop(2, -1, 0, 297);//ǥ����
	//������ ��ġ
	setWall({ 2,-4,0 }, 114);
	setWall({ 2,-3,0 }, 114);
	setWall({ 2,-2,0 }, 114);
	setWall({ 4,-4,0 }, 114);
	setWall({ 4,-3,0 }, 114);
	//���� �� Ÿ��(����)
	setFloor({ -3,0,0 }, 225);
	setFloor({ -4,0,0 }, 226);
	setFloor({ -3,1,0 }, 225);
	setFloor({ -4,1,0 }, 226);
	setFloor({ -4,2,0 }, 225);
	int startX = -33;
	int startY = 36;
	for (int dy = 0; dy <= 30; dy++)
	{
		for (int dx = 0; dx <= 60; dx++)
		{
			if (dy <= 1) setFloor({ startX + dx, startY + dy, 0 }, 231);//�����ؼ�
			else if (dy <= 5) setFloor({ startX + dx, startY + dy, 0 }, 381);//��
			else if (dy <= 7) setFloor({ startX + dx, startY + dy, 0 }, 231);//�����ؼ�
			else setFloor({ startX + dx, startY + dy, 0 }, 232);//�����ؼ�
		}
	}
	setFloor({ -7,41,0 }, 231);
	setFloor({ 0,38,0 }, 231);
	setFloor({ 1,38,0 }, 231);
	setFloor({ 1,36,0 }, 232);
	setFloor({ 4,41,0 }, 231);
	setFloor({ 5,41,0 }, 231);
	setFloor({ 6,41,0 }, 231);
	setFloor({ 5,43,0 }, 232);
	setFloor({ 6,43,0 }, 232);
	//�ϴܿ���
	setFloor({ 5,8,0 }, 225);
	setFloor({ 5,9,0 }, 225);
	for (int dx = -3; dx <= 2; dx++)
	{
		setFloor({ 5 + dx,10,0 }, 225);
		setFloor({ 5 + dx,11,0 }, 225);
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////�弳ġ�� �߰�////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//new Prop(pX + 1 - 1, pY - 2, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 1, pY - 2 + 1, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 6, pY - 5, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 7, pY - 3, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 7, pY - 4, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 5, pY - 1 + 1, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 5 - 1, pY - 1, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 5 - 1, pY - 1 + 1, pZ, 234);//���ɳ��� ��ġ
	//new Prop(pX + 7 - 1, pY + 1, pZ, 234);//���ɳ��� ��ġ


	/*
	for (int x = 0; x <= 10; x++) {
		for (int y = -8; y <= -2; y++) {
			SNOW(x, y, 0);
		}
	}

	for (int x = 4; x <= 10; x++) SNOW(x, -1, 0);

	for (int x = 6; x <= 10; x++) SNOW(x, 0, 0);

	SNOW(3, 4, 0);
	SNOW(3, 5, 0);
	SNOW(4, 5, 0);

	for (int x = 5; x <= 13; x++) {
		for (int y = 4; y <= 7; y++) {
			SNOW(x, y, 0);
		}
	}

	for (int x = -5; x <= 1; x++) {
		for (int y = 4; y <= 11; y++) {
			SNOW(x, y, 0);
		}
	}

	for (int x = 2; x <= 3; x++) {
		for (int y = 6; y <= 8; y++) {
			SNOW(x, y, 0);
		}
	}

	for (int x = -2; x <= 1; x++) {
		for (int y = 2; y <= 3; y++) {
			SNOW(x, y, 0);
		}
	}

	SNOW(-3, 3, 0);

	for (int x = 11; x <= 13; x++) {
		for (int y = -8; y <= -1; y++) {
			SNOW(x, y, 0);
		}
	}

	for (int x = -6; x <= -1; x++) {
		for (int y = -9; y <= -7; y++) {
			SNOW(x, y, 0);
		}
	}

	SNOW(6, 3, 0);
	SNOW(9, 3, 0);
	SNOW(10, 3, 0);
	SNOW(-5, 3, 0);
	SNOW(7, 1, 0);
	SNOW(-5, 2, 0);
	*/

	//for (int y = -7; y <= -3; y++) SNOW(14, y, 0);


	new Prop(1, -3, 0, 117);//���� ��ġ

	new Prop(pX + 3, pY - 2, 0, 239);//���� ��ġ

	new Prop(pX + 5, pY - 1, 0, 247);//���� ��ġ

	new Prop(0, -5, 0, 238);//���� ��ġ




	new Prop(pX + 7, pY + 1, 0, 237);//���� ��ġ


	new Prop(pX + 4, pY - 5, 0, 248);//���� ��ġ

	new Prop(pX + 9, pY - 4, 0, 237);//���� ��ġ
	new Prop(pX + 10, pY - 1, 0, 244);//������� ��ġ

	new Prop(pX - 2, pY + 39, 0, 242);//���ڳ��� ��ġ


	new Prop(pX, pY - 20, 0, 237);

	new Prop(-4, 5, 0, 245);//������� ��ġ


	new Prop(pX + 3, pY + 3, 0, 338);//��ö ��ġ
	new Prop(pX + 3 + 1, pY + 3, 0, 338);//��ö ��ġ
	new Prop(pX + 3 + 2, pY + 3, 0, 338);//��ö ��ġ
	new Prop(pX + 3 + 1, pY + 3 + 1, 0, 338);//��ö ��ġ

	new Prop(pX + 10, pY + 11, 0, 338);//��ö ��ġ
	new Prop(pX + 10, pY + 11 - 1, 0, 338);//��ö ��ġ
	new Prop(pX + 10, pY + 11 - 2, 0, 338);//��ö ��ġ

	new Prop(pX + 10 - 1, pY + 11, 0, 338);//��ö ��ġ
	new Prop(pX + 10 - 1, pY + 11 - 1, 0, 338);//��ö ��ġ



	//�ܵ�
	setFloor({ pX - 2, pY + 3, 0 }, 220);
	setFloor({ pX - 1, pY + 3, 0 }, 220);
	setFloor({ pX, pY + 3, 0 }, 220);
	setFloor({ pX + 1, pY + 3, 0 }, 220);

	setFloor({ pX - 2, pY + 4, 0 }, 220);
	setFloor({ pX - 1, pY + 4, 0 }, 220);
	setFloor({ pX, pY + 4, 0 }, 220);
	setFloor({ pX + 1, pY + 4, 0 }, 220);

	new Prop(pX - 2, pY + 3, 0, 270);//�� ��ġ
	new Prop(pX - 1, pY + 3, 0, 265);//�� ��ġ
	new Prop(pX, pY + 3, 0, 266);//�� ��ġ
	new Prop(pX + 1, pY + 3, 0, 267);//�� ��ġ

	new Prop(pX - 2, pY + 4, 0, 271);//�� ��ġ
	new Prop(pX - 1, pY + 4, 0, 268);//�� ��ġ
	new Prop(pX, pY + 4, 0, 269);//�� ��ġ
	new Prop(pX + 1, pY + 4, 0, 270);//�� ��ġ


	new Prop(pX + 6, pY - 4, 0, 270);//�� ��ġ

	new Prop(0, -1, 0, 211);//���� �� ��ġ
	new Prop(4, 0, 0, 211);//����� �� ��ġ

	//��Ÿ�� ��ġ
	new Prop(pX - 3, pY + 2, 0, 206);
	new Prop(pX - 2, pY + 2, 0, 206);
	new Prop(pX - 1, pY + 2, 0, 206);
	new Prop(pX, pY + 2, 0, 206);
	new Prop(pX + 1, pY + 2, 0, 206);
	new Prop(pX + 2, pY + 2, 0, 206);
	new Prop(pX + 3, pY + 2, 0, 206);

	new Prop(pX - 3, pY + 3, 0, 206);
	new Prop(pX - 3, pY + 4, 0, 206);

	new Prop(pX + 2, pY + 3, 0, 206);
	new Prop(pX + 2, pY + 4, 0, 206);

	new Prop(pX - 3, pY + 5, 0, 206);
	new Prop(pX - 2, pY + 5, 0, 206);
	new Prop(pX - 1, pY + 5, 0, 206);
	new Prop(pX, pY + 5, 0, 206);
	new Prop(pX + 1, pY + 5, 0, 206);
	new Prop(pX + 2, pY + 5, 0, 206);

	//���� ��ġ
	new Prop(pX + 8, pY + 1, 0, 143);
	new Prop(pX + 8, pY, 0, 143);
	new Prop(pX + 8, pY - 1, 0, 143);
	new Prop(pX + 8, pY - 2, 0, 143);
	new Prop(pX + 9, pY, 0, 143);

	//��� ��ġ
	new Prop(pX + 3, pY + 6, 0, 144);
	new Prop(pX + 4, pY + 6, 0, 144);
	new Prop(pX + 5, pY + 6, 0, 144);
	new Prop(pX + 6, pY + 6, 0, 144);
	new Prop(pX + 5, pY + 7, 0, 144);

	//����

	new Prop(pX - 3, pY - 7, 0, 213);
	new Prop(pX - 1, pY - 7, 0, 214);
	new Prop(pX + 1, pY - 7, 0, 216);
	new Prop(pX + 3, pY - 7, 0, 217);
	new Prop(pX + 5, pY - 7, 0, 218);
	new Prop(pX + 7, pY - 7, 0, 219);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int vX = pY - 8;
	int vY = pY + 3;
	Vehicle* myCar = new Vehicle(vX, vY, 0, 48);//���� ��ġ
	myCar->name = L"SUV";
	myCar->vehType = vehFlag::car;

	///////////////////////���� ���� ������//////////////////////////////////////
	myCar->extendPart(vX, vY - 1, 48);
	myCar->extendPart(vX - 1, vY - 1, 48);
	myCar->extendPart(vX + 1, vY - 1, 48);
	myCar->extendPart(vX + 2, vY - 1, 48);
	myCar->extendPart(vX - 1, vY - 2, 48);
	myCar->extendPart(vX, vY - 2, 48);
	myCar->extendPart(vX + 1, vY - 2, 48);
	myCar->extendPart(vX + 2, vY - 2, 48);
	myCar->extendPart(vX - 1, vY, 48);
	myCar->extendPart(vX + 1, vY, 48);
	myCar->extendPart(vX + 2, vY, 48);
	myCar->extendPart(vX - 1, vY + 1, 48);
	myCar->extendPart(vX, vY + 1, 48);
	myCar->extendPart(vX + 1, vY + 1, 48);
	myCar->extendPart(vX + 2, vY + 1, 48);
	myCar->extendPart(vX - 1, vY + 2, 48);
	myCar->extendPart(vX, vY + 2, 48);
	myCar->extendPart(vX + 1, vY + 2, 48);
	myCar->extendPart(vX + 2, vY + 2, 48);
	myCar->extendPart(vX - 1, vY + 3, 48);
	myCar->extendPart(vX, vY + 3, 48);
	myCar->extendPart(vX + 1, vY + 3, 48);
	myCar->extendPart(vX + 2, vY + 3, 48);

	myCar->extendPart(vX - 1, vY - 3, 130);
	myCar->extendPart(vX, vY - 3, 130);
	myCar->extendPart(vX + 1, vY - 3, 130);
	myCar->extendPart(vX + 2, vY - 3, 130);
	//////////////////////////���ֻ�� 4Ÿ��////////////////////////////////////
	myCar->addPart(vX - 1, vY - 2, { 142,119,126 });
	myCar->addPart(vX, vY - 2, { 119 });
	myCar->addPart(vX + 1, vY - 2, { 119 });
	myCar->addPart(vX + 2, vY - 2, { 142,119,126 });
	//////////////////////////���߻�� 4Ÿ��////////////////////////////////////
	myCar->addPart(vX - 1, vY - 1, 121);
	myCar->addPart(vX, vY - 1, 121);
	myCar->addPart(vX + 1, vY - 1, 121);
	myCar->addPart(vX + 2, vY - 1, 121);
	////////////////////////////////������� 4Ÿ��///////////////////////////////
	myCar->addPart(vX - 1, vY, { 120 });
	myCar->addPart(vX, vY, { 122, 123, 99, 128 });
	myCar->addPart(vX + 1, vY, { 122, 123, 128 });
	myCar->addPart(vX + 2, vY, { 120 });
	//////////////////////////������� �Ʒ� ��� 4Ÿ��/////////////////////////////
	myCar->addPart(vX - 1, vY + 1, { 119 });
	myCar->addPart(vX, vY + 1, { 122, 128 });
	myCar->addPart(vX + 1, vY + 1, { 122, 128,129 });
	myCar->addPart(vX + 2, vY + 1, { 119 });
	///////////////////////////////����ڼ� 4Ÿ��/////////////////////
	myCar->addPart(vX - 1, vY + 2, { 120 });
	myCar->addPart(vX, vY + 2, { 122, 123, 128 });
	myCar->addPart(vX + 1, vY + 2, { 122, 123, 128 });
	myCar->addPart(vX + 2, vY + 2, { 120 });
	///////////////////////////////�����Ĺ� 4Ÿ��///////////////////////////
	myCar->addPart(vX - 1, vY + 3, { 119,127 });
	myCar->addPart(vX, vY + 3, { 124 });
	myCar->addPart(vX + 1, vY + 3, { 124 });
	myCar->addPart(vX + 2, vY + 3, { 119,127 });



	///////////////////�������///////////////////////////////////////////
	Vehicle* myMoto = new Vehicle(vX + 6, vY + 5, 0, 48);
	myMoto->extendPart(vX + 6, vY + 4, 48);
	myMoto->extendPart(vX + 6, vY + 6, 48);

	myMoto->addPart(vX + 6, vY + 4, { 102,134 });
	myMoto->addPart(vX + 6, vY + 5, 132);
	myMoto->addPart(vX + 6, vY + 6, 102);

	////////////////////������////////////////////////////////////////////
	Vehicle* myBike = new Vehicle(vX + 9, vY + 4, 0, 48);
	myBike->extendPart(vX + 9, vY + 3, 48);
	myBike->extendPart(vX + 9, vY + 5, 48);

	myBike->addPart(vX + 9, vY + 3, { 102,133 });
	myBike->addPart(vX + 9, vY + 4, { 135, 132 });
	myBike->addPart(vX + 9, vY + 5, { 102,136 });


	////////////////////////////////////////////////////////////////////////////
	{
		int cx = 15;
		int cy = 0;
		Vehicle* myHeli = new Vehicle(cx, cy, 0, 48);
		myHeli->name = L"���";
		myHeli->vehType = vehFlag::heli;
		myHeli->addPart(cx, cy, { 311 });

		myHeli->extendPart(cx, cy - 1, 48);
		myHeli->extendPart(cx, cy + 1, 48);
		myHeli->extendPart(cx, cy + 2, 48);
		myHeli->extendPart(cx, cy + 3, 48);
		myHeli->extendPart(cx, cy + 4, 48);
		myHeli->extendPart(cx, cy + 5, 48);

		myHeli->extendPart(cx + 1, cy - 1, 48);
		myHeli->extendPart(cx + 1, cy, 48);
		myHeli->extendPart(cx + 1, cy + 1, 48);
		myHeli->extendPart(cx + 1, cy + 2, 48);

		myHeli->extendPart(cx - 1, cy - 1, 48);
		myHeli->extendPart(cx - 1, cy, 48);
		myHeli->extendPart(cx - 1, cy + 1, 48);
		myHeli->extendPart(cx - 1, cy + 2, 48);

		myHeli->addPart(cx + 1, cy - 1, { 121 });
		myHeli->addPart(cx, cy - 1, { 121 });
		myHeli->addPart(cx - 1, cy - 1, { 121 });

		myHeli->addPart(cx + 1, cy, { 120 });
		myHeli->addPart(cx, cy, { 122,123, 128 });
		myHeli->addPart(cx - 1, cy, { 120 });

		myHeli->addPart(cx + 1, cy + 1, { 119 });
		myHeli->addPart(cx, cy + 1, { 122, 128, 314 });
		myHeli->addPart(cx - 1, cy + 1, { 119 });

		myHeli->addPart(cx + 1, cy + 2, { 119 });
		myHeli->addPart(cx, cy + 2, { 119 });
		myHeli->addPart(cx - 1, cy + 2, { 119 });

		myHeli->addPart(cx, cy + 3, { 119 });
		myHeli->addPart(cx, cy + 4, { 119 });
		myHeli->addPart(cx, cy + 5, { 119, 315 });
	}

	//���� 3��
	Vehicle* cart1 = new Vehicle(10, 5, 0, 378);
	cart1->vehType = vehFlag::car;

	Vehicle* cart2 = new Vehicle(8, 5, 0, 379);
	cart2->vehType = vehFlag::car;

	Vehicle* cart3 = new Vehicle(6, 5, 0, 137);
	cart3->vehType = vehFlag::car;

	Vehicle* cart4 = new Vehicle(7, -5, 0, 378);
	cart4->vehType = vehFlag::car;

	//����
	Vehicle* minecart1 = new Vehicle(3, 15, 0, 405);
	minecart1->vehType = vehFlag::minecart;
	minecart1->addPart(3, 15, { itemVIPCode::minecartController });
	minecart1->bodyDir = dir16::dir0;
	minecart1->isPowerCart = true;

	Vehicle* minecart2 = new Vehicle(2, 15, 0, 405);
	minecart2->vehType = vehFlag::minecart;
	minecart2->bodyDir = dir16::dir0;

	Vehicle* minecart3 = new Vehicle(1, 15, 0, 405);
	minecart3->vehType = vehFlag::minecart;
	minecart3->bodyDir = dir16::dir0;

	minecart1->rearCart = minecart2;
	minecart2->rearCart = minecart3;


	///////////////////////////////////////////////////////////////

	World::ins()->createSector(0, 0, 0);

	Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
	
};