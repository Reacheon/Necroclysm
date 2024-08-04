
export module startSetting;

import globalVar;
import HUD;
import Player;
import ItemData;
import ItemStack;
import World;
import Vehicle;
import Prop;

export void startSetting()
{
	new HUD();
	Player* playerPtr = Player::ins();
	playerPtr->setGrid(0, 0, 0);
	playerPtr->setDstGrid(0, 0);

	playerPtr->updateWalkable(playerPtr->getGridX() + 1, playerPtr->getGridY());
	playerPtr->updateWalkable(playerPtr->getGridX() + 1, playerPtr->getGridY() - 1);
	playerPtr->updateWalkable(playerPtr->getGridX(), playerPtr->getGridY() - 1);
	playerPtr->updateWalkable(playerPtr->getGridX() - 1, playerPtr->getGridY() - 1);
	playerPtr->updateWalkable(playerPtr->getGridX() - 1, playerPtr->getGridY());
	playerPtr->updateWalkable(playerPtr->getGridX() - 1, playerPtr->getGridY() + 1);
	playerPtr->updateWalkable(playerPtr->getGridX(), playerPtr->getGridY() + 1);
	playerPtr->updateWalkable(playerPtr->getGridX() + 1, playerPtr->getGridY() + 1);
	


	//�׽�Ʈ ������
	int pX = (Player::ins())->getGridX();
	int pY = (Player::ins())->getGridY();
	int pZ = (Player::ins())->getGridZ();

	ItemStack* item = new ItemStack(pX + 2, pY + 1, pZ);
	(item->getPocket())->addItemFromDex(2, 1);
	(item->getPocket())->addItemFromDex(0, 5);
	(item->getPocket())->addItemFromDex(23, 1);
	(item->getPocket())->addItemFromDex(24, 10);
	(item->getPocket())->addItemFromDex(1, 4);
	(item->getPocket())->addItemFromDex(0, 1);

	(item->getPocket())->addItemFromDex(3, 1);
	(item->getPocket())->addItemFromDex(12, 1);
	(item->getPocket())->addItemFromDex(13, 1);
	(item->getPocket())->addItemFromDex(14, 1);
	(item->getPocket())->addItemFromDex(15, 1);
	(item->getPocket())->addItemFromDex(16, 1);
	(item->getPocket())->addItemFromDex(17, 1);
	(item->getPocket())->addItemFromDex(18, 1);

	//�������� �Ѿ�
	(item->getPocket())->addItemFromDex(4, 1);
	(item->getPocket())->addItemFromDex(5, 8);

	//��ȿ��ź��, ������, ��ź��, ����
	(item->getPocket())->addItemFromDex(88, 1);
	(item->getPocket())->addItemFromDex(89, 1000);//��ź��
	(item->getPocket())->addItemFromDex(91, 1000);//����
	(item->getPocket())->addItemFromDex(82, 1);//������


	//new Monster(playerPtr->getGridX() + 1, playerPtr->getGridY(), playerPtr->getGridZ(), 2);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////������� ������ �߰�////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	availableRecipe = new ItemPocket(storageType::recipe);
	for (int i = 1; i <= 212; i++) ((ItemPocket*)availableRecipe)->addRecipe(i);

	//Ÿ�� ����
	for (int dx = -30; dx <= 30; dx++)
	{
		for (int dy = -30; dy <= 30; dy++)
		{
			World::ins()->getTile(pX + dx, pY + dy, pZ).floor = 109;
		}
	}

	for (int dx = -30; dx <= 30; dx++)
	{
		for (int dy = -30; dy <= -2; dy++)
		{
			World::ins()->getTile(pX + dx, pY + dy, pZ).floor = 220;
		}
	}

	for (int dx = -6; dx >= -14; dx--)
	{
		for (int dy = -30; dy <= 30; dy++)
		{
			if (dx == -10 && ((dy + 30) % 6 < 3)) World::ins()->getTile(pX + dx, pY + dy, pZ).floor = 377;//����ƽ���Ʈ
			else World::ins()->getTile(pX + dx, pY + dy, pZ).floor = 296;//�����ƽ���Ʈ
		}
	}

	//���ֱ�
	World::ins()->getTile(pX - 2 - 1, pY - 1, pZ).floor = 293;
	World::ins()->getTile(pX - 2, pY - 1, pZ).floor = 293;
	World::ins()->getTile(pX - 2, pY, pZ).floor = 293;
	World::ins()->getTile(pX - 1, pY, pZ).floor = 293;
	World::ins()->getTile(pX - 0, pY, pZ).floor = 293;
	World::ins()->getTile(pX + 1, pY, pZ).floor = 293;
	World::ins()->getTile(pX + 2, pY, pZ).floor = 293;
	World::ins()->getTile(pX + 2, pY + 1, pZ).floor = 293;
	World::ins()->getTile(pX + 3, pY + 1, pZ).floor = 293;
	World::ins()->getTile(pX + 4, pY + 1, pZ).floor = 293;
	World::ins()->getTile(pX + 4, pY + 2, pZ).floor = 293;
	for(int i=0; i<9; i++) World::ins()->getTile(pX + 5 + i, pY + 2, pZ).floor = 293;

	//������ ��ġ
	//�� �ϴ� 5Ÿ��
	World::ins()->getTile(pX - 1, pY - 2, pZ).setWall(375);
	World::ins()->getTile(pX - 2, pY - 2, pZ).setWall(375);
	new Prop(pX - 3, pY - 2, pZ, 291);//������ ��ġ
	World::ins()->getTile(pX - 4, pY - 2, pZ).setWall(375);
	World::ins()->getTile(pX - 5, pY - 2, pZ).setWall(375);

	//�� ���� 4Ÿ��
	World::ins()->getTile(pX - 1, pY - 3, pZ).setWall(375);
	World::ins()->getTile(pX - 1, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX - 1, pY - 5, pZ).setWall(375);
	World::ins()->getTile(pX - 1, pY - 6, pZ).setWall(375);

	//�� ���� 4Ÿ��
	World::ins()->getTile(pX - 5, pY - 3, pZ).setWall(375);
	World::ins()->getTile(pX - 5, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX - 5, pY - 5, pZ).setWall(375);
	World::ins()->getTile(pX - 5, pY - 6, pZ).setWall(375);

	//�� ��� �߾� 3Ÿ��
	World::ins()->getTile(pX - 2, pY - 6, pZ).setWall(375);
	World::ins()->getTile(pX - 3, pY - 6, pZ).setWall(375);
	World::ins()->getTile(pX - 4, pY - 6, pZ).setWall(375);

	new Prop(pX - 4, pY - 5, pZ, 295);//å��
	new Prop(pX - 2, pY - 5, pZ, 294);//ħ��

	new Prop(pX - 4, pY - 3, pZ, 298);//��°��
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			World::ins()->getTile(pX - 3 + dx, pY - 4 + dy, pZ + 1).floor = 292;
		}
	}

	new Prop(pX - 2, pY - 3, pZ, 299);//�ϰ����


	//ö����
	for (int i = 0; i < 17; i++)//���
	{
		World::ins()->getTile(pX - 5 + i, pY - 9, pZ).setWall(376);
	}

	for (int i = 0; i < 20; i++)//����
	{
		if (pY - 8 + i != 1 && pY - 8 + i != 2 && pY - 8 + i != 3)
		{
			World::ins()->getTile(pX + 11, pY - 8 + i, pZ).setWall(376);
		}
	}

	for (int i = 0; i < 17; i++)//���
	{
		World::ins()->getTile(pX + 11 - i, pY + 12, pZ).setWall(376);
	}

	//ö���� ���� �Ա� ����� 2��
	new Prop(pX + 12, pY + 0, pZ + 0, 211);//�����
	new Prop(pX + 12, pY + 4, pZ + 0, 211);//�����


	//����
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -2; dy <= 2; dy++)
			{

				World::ins()->getTile(pX - 2 + dx, pY - 3 + dy, pZ - 1).destoryWall();
			}
		}

		new Prop(pX - 2, pY - 5, pZ - 1, 211);//�����
		{
			int cx = pX - 2 + 1;
			int cy = pY - 3 + 2;

			for (int i = 1; i < 9; i++) World::ins()->getTile(cx, cy + i, pZ - 1).destoryWall();
			new Prop(cx, cy + 1, pZ - 1, 291);//������ ��ġ

			for (int i = 1; i < 9; i++) World::ins()->getTile(cx + i, cy, pZ - 1).destoryWall();
			Prop* door2 = new Prop(cx + 1, cy, pZ - 1, 291);//������ ��ġ
			door2->leadItem.extraSprIndexSingle = 2;

			int aisleEndX = cx + 8;
			int aisleEndY = cy;
			for (int dx = -3; dx <= 3; dx++)
			{
				for (int dy = -40; dy <= 40; dy++)
				{
					World::ins()->getTile(aisleEndX + dx, aisleEndY + dy, pZ - 1).destoryWall();
				}
			}

			int aisleEndX2 = cx + 8 + 12;
			int aisleEndY2 = cy;
			for (int dx = -4; dx <= 4; dx++)
			{
				for (int dy = -40; dy <= 40; dy++)
				{
					World::ins()->getTile(aisleEndX2 + dx, aisleEndY2 + dy, pZ - 1).destoryWall();
				}
			}

			int aisleEndX3 = cx + 8;
			int aisleEndY3 = cy - 14;
			for (int dx = -40; dx <= 40; dx++)
			{
				for (int dy = -3; dy <= 3; dy++)
				{
					World::ins()->getTile(aisleEndX3 + dx, aisleEndY3 + dy, pZ - 1).destoryWall();
				}
			}

			int aisleEndX4 = 19;
			int aisleEndY4 = 12;
			for (int dx = -15; dx <= 0; dx++)
			{
				for (int dy = -3; dy <= 3; dy++)
				{
					World::ins()->getTile(aisleEndX4 + dx, aisleEndY4 + dy, pZ - 1).destoryWall();
				}
			}


			int cursorX = aisleEndX;
			int cursorY = aisleEndY + 12;

			for (int i = 0; i <= 25; i++)
			{
				new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTB);
				cursorY--;
			}

			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railBR);
			cursorX++;

			for (int i = 0; i < 11; i++)
			{
				new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railRL);
				cursorX++;
			}

			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railSwitchWS);//��ȯ��

			int secondCursorX = cursorX+1;
			int secondCursorY = cursorY;
			for (int i = 0; i < 22; i++)
			{
				new Prop(secondCursorX, secondCursorY, pZ - 1, itemVIPCode::railRL);//������ ��ġ
				secondCursorX++;
			}

			cursorY++;

			for (int i = 0; i < 26; i++)
			{
				new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTB);//������ ��ġ
				cursorY++;
			}

			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTL);//������ ��ġ
			cursorX--;

			for (int i = 0; i < 11; i++)
			{
				new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railRL);//������ ��ġ
				cursorX--;
			}
			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTR);//������ ��ġ


			//����ö ��ġ
			int vX = pX - 2 + 1 + 8;
			int vY = pY - 3 + 2;
			Vehicle* myTrainPower = new Vehicle(vX, vY, pZ - 1, 48);//���� ��ġ
			{

				myTrainPower->vehType = vehFlag::train;
				myTrainPower->isVehicle = true;

				///////////////////////���� ���� ������//////////////////////////////////////
				myTrainPower->extendPart(vX, vY - 1, 48);
				myTrainPower->extendPart(vX - 1, vY - 1, 48);
				myTrainPower->extendPart(vX + 1, vY - 1, 48);
				myTrainPower->extendPart(vX + 2, vY - 1, 48);
				myTrainPower->extendPart(vX-2, vY - 1, 48);

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
						if ((x == topLeftX || x == topLeftX + 4)|| (y == topLeftY || y == topLeftY + 7))
						{
							if (x == topLeftX + 2 && y == topLeftY + 7)  myTrainPower->addPart(x, y, { 120 });
							else if( y == topLeftY + 4)  myTrainPower->addPart(x, y, { 120 });
							else if(y == topLeftY) myTrainPower->addPart(x, y, { 121 });
							else myTrainPower->addPart(x, y, { 119 });
						}
						else if ((y == topLeftY + 2))
						{
							if(x == topLeftX + 2) myTrainPower->addPart(x, y, { 120 });
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

			//����ö ��ġ
			{
				int vX = pX - 2 + 1 + 8;
				int vY = pY - 3 + 2 + 8;
				Vehicle* myTrain = new Vehicle(vX, vY, pZ - 1, 48);//���� ��ġ
				myTrainPower->rearTrain = myTrain;
				myTrain->isVehicle = true;
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
							if( x == topLeftX + 2)  myTrain->addPart(x, y, { 120 });
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
			World::ins()->getTile(pX - 5 + dx, pY - 5 + dy, pZ).floor = 292;
		}
	}

	new Prop(pX + 2, pY - 1, pZ, 297);//ǥ����


	//������ ��ġ
	World::ins()->getTile(pX + 2, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX + 2, pY - 3, pZ).setWall(114);
	World::ins()->getTile(pX + 2, pY - 2, pZ).setWall(114);
	World::ins()->getTile(pX + 4, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX + 4, pY - 3, pZ).setWall(114);

	//���� �� Ÿ��(����)
	World::ins()->getTile(pX - 3, pY + 0, pZ).floor = 225;
	World::ins()->getTile(pX - 4, pY + 0, pZ).floor = 226;
	World::ins()->getTile(pX - 3, pY + 1, pZ).floor = 225;
	World::ins()->getTile(pX - 4, pY + 1, pZ).floor = 226;
	World::ins()->getTile(pX - 4, pY + 2, pZ).floor = 225;

	int startX = -33;
	int startY = 36;
	for (int dy = 0; dy <= 30; dy++)
	{
		for (int dx = 0; dx <= 60; dx++)
		{
			if(dy <=2)World::ins()->getTile(startX + dx, startY + dy, pZ).floor = 381;
			else if(dy <=4) World::ins()->getTile(startX + dx, startY + dy, pZ).floor = 231;
			else World::ins()->getTile(startX + dx, startY + dy, pZ).floor = 232;
			
		}
	}

	//�ϴܿ���
	World::ins()->getTile(pX + 5, pY + 8, pZ).floor = 225;
	World::ins()->getTile(pX + 5, pY + 9, pZ).floor = 225;

	for (int dx = -3; dx <= 2; dx++)
	{
		World::ins()->getTile(pX + 5 + dx, pY + 10, pZ).floor = 225;
		World::ins()->getTile(pX + 5 + dx, pY + 11, pZ).floor = 225;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////�弳ġ�� �߰�////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	new Prop(pX + 1, pY - 2, pZ, 237);//���� ��ġ
	new Prop(pX + 1 - 1, pY - 2, pZ, 235);//���ɳ��� ��ġ
	new Prop(pX + 1, pY - 2 + 1, pZ, 235);//���ɳ��� ��ġ

	new Prop(pX + 3, pY - 2, pZ, 239);//���� ��ġ

	new Prop(pX + 5, pY - 1, pZ, 248);//���� ��ġ
	new Prop(pX + 5, pY - 1 + 1, pZ, 235);//���ɳ��� ��ġ
	new Prop(pX + 5 - 1, pY - 1, pZ, 235);//���ɳ��� ��ġ
	new Prop(pX + 5 - 1, pY - 1 + 1, pZ, 235);//���ɳ��� ��ġ

	new Prop(pX + 7, pY + 1, pZ, 237);//���� ��ġ
	new Prop(pX + 7 - 1, pY + 1, pZ, 235);//���ɳ��� ��ġ

	new Prop(pX + 4, pY - 5, pZ, 248);//���� ��ġ

	new Prop(pX + 9, pY - 4, pZ, 237);//���� ��ġ



	new Prop(pX, pY - 20, pZ, 237);//���ɳ��� ��ġ


	new Prop(pX + 3, pY + 3, pZ, 338);//��ö ��ġ
	new Prop(pX + 3 + 1, pY + 3, pZ, 338);//��ö ��ġ
	new Prop(pX + 3 + 2, pY + 3, pZ, 338);//��ö ��ġ
	new Prop(pX + 3 + 1, pY + 3 + 1, pZ, 338);//��ö ��ġ

	new Prop(pX + 10, pY + 11, pZ, 338);//��ö ��ġ
	new Prop(pX + 10, pY + 11 - 1, pZ, 338);//��ö ��ġ
	new Prop(pX + 10, pY + 11 - 2, pZ, 338);//��ö ��ġ

	new Prop(pX + 10 - 1, pY + 11, pZ, 338);//��ö ��ġ
	new Prop(pX + 10 - 1, pY + 11 - 1, pZ, 338);//��ö ��ġ



	//�ܵ�
	World::ins()->getTile(pX + -2, pY + 3, pZ).floor = 220;
	World::ins()->getTile(pX + -1, pY + 3, pZ).floor = 220;
	World::ins()->getTile(pX + 0, pY + 3, pZ).floor = 220;
	World::ins()->getTile(pX + 1, pY + 3, pZ).floor = 220;

	World::ins()->getTile(pX + -2, pY + 4, pZ).floor = 220;
	World::ins()->getTile(pX + -1, pY + 4, pZ).floor = 220;
	World::ins()->getTile(pX + 0, pY + 4, pZ).floor = 220;
	World::ins()->getTile(pX + 1, pY + 4, pZ).floor = 220;

	new Prop(pX + -2, pY + 3, pZ, 270);//�� ��ġ
	new Prop(pX + -1, pY + 3, pZ, 265);//�� ��ġ
	new Prop(pX + 0, pY + 3, pZ, 266);//�� ��ġ
	new Prop(pX + 1, pY + 3, pZ, 267);//�� ��ġ

	new Prop(pX + -2, pY + 4, pZ, 271);//�� ��ġ
	new Prop(pX + -1, pY + 4, pZ, 268);//�� ��ġ
	new Prop(pX + 0, pY + 4, pZ, 269);//�� ��ġ
	new Prop(pX + 1, pY + 4, pZ, 270);//�� ��ġ

	new Prop(pX, pY - 1, pZ, 118);//����� �� ��ġ

	//��Ÿ�� ��ġ
	new Prop(pX - 3, pY + 2, pZ, 206);
	new Prop(pX - 2, pY + 2, pZ, 206);
	new Prop(pX - 1, pY + 2, pZ, 206);
	new Prop(pX, pY + 2, pZ, 206);
	new Prop(pX + 1, pY + 2, pZ, 206);
	new Prop(pX + 2, pY + 2, pZ, 206);
	new Prop(pX + 3, pY + 2, pZ, 206);

	new Prop(pX - 3, pY + 3, pZ, 206);
	new Prop(pX - 3, pY + 4, pZ, 206);

	new Prop(pX + 2, pY + 3, pZ, 206);
	new Prop(pX + 2, pY + 4, pZ, 206);

	new Prop(pX - 3, pY + 5, pZ, 206);
	new Prop(pX - 2, pY + 5, pZ, 206);
	new Prop(pX - 1, pY + 5, pZ, 206);
	new Prop(pX, pY + 5, pZ, 206);
	new Prop(pX + 1, pY + 5, pZ, 206);
	new Prop(pX + 2, pY + 5, pZ, 206);

	//���� ��ġ
	new Prop(pX + 8, pY + 1, pZ, 143);
	new Prop(pX + 8, pY + 0, pZ, 143);
	new Prop(pX + 8, pY - 1, pZ, 143);
	new Prop(pX + 8, pY - 2, pZ, 143);
	new Prop(pX + 9, pY + 0, pZ, 143);

	//��� ��ġ
	new Prop(pX + 3, pY + 6, pZ, 144);
	new Prop(pX + 4, pY + 6, pZ, 144);
	new Prop(pX + 5, pY + 6, pZ, 144);
	new Prop(pX + 6, pY + 6, pZ, 144);
	new Prop(pX + 5, pY + 7, pZ, 144);

	//����

	new Prop(pX - 3, pY - 7, pZ, 213);
	new Prop(pX - 1, pY - 7, pZ, 214);
	new Prop(pX + 1, pY - 7, pZ, 216);
	new Prop(pX + 3, pY - 7, pZ, 217);
	new Prop(pX + 5, pY - 7, pZ, 218);
	new Prop(pX + 7, pY - 7, pZ, 219);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int vX = pY - 8;
	int vY = pY + 3;
	Vehicle* myCar = new Vehicle(vX, vY, pZ, 48);//���� ��ġ
	myCar->isVehicle = true;
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
	myCar->addPart(vX - 1, vY - 2, { 142,119,126 }); //����, ����
	myCar->addPart(vX, vY - 2, { 119 }); //����
	myCar->addPart(vX + 1, vY - 2, { 119 }); //����
	myCar->addPart(vX + 2, vY - 2, { 142,119,126 }); //����, ����
	//////////////////////////���߻�� 4Ÿ��////////////////////////////////////
	myCar->addPart(vX - 1, vY - 1, 121); //������
	myCar->addPart(vX, vY - 1, 121); //������
	myCar->addPart(vX + 1, vY - 1, 121); //������
	myCar->addPart(vX + 2, vY - 1, 121); //������
	////////////////////////////////������� 4Ÿ��///////////////////////////////
	myCar->addPart(vX - 1, vY, { 120 });//����
	myCar->addPart(vX, vY, { 122, 123, 99, 128 });//���,����
	myCar->addPart(vX + 1, vY, { 122, 123, 128 });//���,����
	myCar->addPart(vX + 2, vY, { 120 });//����
	//////////////////////////������� �Ʒ� ��� 4Ÿ��/////////////////////////////
	myCar->addPart(vX - 1, vY + 1, { 119 });//����
	myCar->addPart(vX, vY + 1, { 122, 128 });//���
	myCar->addPart(vX + 1, vY + 1, { 122, 128,129 });//���
	myCar->addPart(vX + 2, vY + 1, { 119 });//����
	///////////////////////////////����ڼ� 4Ÿ��/////////////////////
	myCar->addPart(vX - 1, vY + 2, { 120 });//����
	myCar->addPart(vX, vY + 2, { 122, 123, 128 });//���,����
	myCar->addPart(vX + 1, vY + 2, { 122, 123, 128 });//���,����
	myCar->addPart(vX + 2, vY + 2, { 120 });//����
	///////////////////////////////�����Ĺ� 4Ÿ��///////////////////////////
	myCar->addPart(vX - 1, vY + 3, { 119,127 });//����
	myCar->addPart(vX, vY + 3, { 124 });//Ʈ��ũ��
	myCar->addPart(vX + 1, vY + 3, { 124 });//Ʈ��ũ��
	myCar->addPart(vX + 2, vY + 3, { 119,127 });//����



	///////////////////�������///////////////////////////////////////////
	Vehicle* myMoto = new Vehicle(vX + 6, vY + 5, pZ, 48);//���� ��ġ
	myMoto->isVehicle = true;
	myMoto->extendPart(vX + 6, vY + 4, 48);
	myMoto->extendPart(vX + 6, vY + 6, 48);

	myMoto->addPart(vX + 6, vY + 4, { 102,134 });
	myMoto->addPart(vX + 6, vY + 5, 132);
	myMoto->addPart(vX + 6, vY + 6, 102);

	////////////////////������////////////////////////////////////////////
	Vehicle* myBike = new Vehicle(vX + 9, vY + 4, pZ, 48);//���� ��ġ
	myBike->isVehicle = true;
	myBike->extendPart(vX + 9, vY + 3, 48);
	myBike->extendPart(vX + 9, vY + 5, 48);

	myBike->addPart(vX + 9, vY + 3, { 102,133 });
	myBike->addPart(vX + 9, vY + 4, { 135, 132 });
	myBike->addPart(vX + 9, vY + 5, { 102,136 });


	////////////////////////////////////////////////////////////////////////////
	{
		int cx = 15;
		int cy = 0;
		Vehicle* myHeli = new Vehicle(cx, cy, pZ, 48);//���� ��ġ
		myHeli->isVehicle = true;
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

		//��� 3����
		myHeli->addPart(cx + 1, cy - 1, { 121 });
		myHeli->addPart(cx, cy - 1, { 121 });
		myHeli->addPart(cx - 1, cy - 1, { 121 });

		//�÷��̾� 3Ÿ��(������)
		myHeli->addPart(cx + 1, cy, { 120 });
		myHeli->addPart(cx, cy, { 122,123, 128 });
		myHeli->addPart(cx - 1, cy, { 120 });

		//��ĭ
		myHeli->addPart(cx + 1, cy + 1, { 119 });
		myHeli->addPart(cx, cy + 1, { 122, 128, 314});
		myHeli->addPart(cx - 1, cy + 1, { 119 });
		
		//���� �ݸ� 3ĭ��
		myHeli->addPart(cx + 1, cy + 2, { 119 });
		myHeli->addPart(cx, cy + 2, { 119 });
		myHeli->addPart(cx - 1, cy + 2, { 119 });

		//���� 3ĭ
		myHeli->addPart(cx, cy + 3, { 119 });
		myHeli->addPart(cx, cy + 4, { 119 });
		myHeli->addPart(cx, cy + 5, { 119, 315 });
	}

	//���� 3��
	Vehicle* cart1 = new Vehicle(10, 5, 0, 378);//���� ��ġ
	cart1->isVehicle = true;
	cart1->vehType = vehFlag::car;

	Vehicle* cart2 = new Vehicle(8, 5, 0, 379);//���� ��ġ
	cart2->isVehicle = true;
	cart2->vehType = vehFlag::car;

	Vehicle* cart3 = new Vehicle(6, 5, 0, 137);//���� ��ġ
	cart3->isVehicle = true;
	cart3->vehType = vehFlag::car;


	//new Vehicle(8, 5, 0, 379);//���� ��ġ
	//new Vehicle(6, 5, 0, 137);//���� ��ġ


	///////////////////////////////////////////////////////////////
	//new Vehicle(pX, pY + 2, pZ, 140);//��ں� ��ġ

	World::ins()->createSector(0, 0, 0);

	Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
	Player::ins()->updateMinimap();//�������� ������Ʈ���� �Ͼ�� ó������ ȭ�鿡 �ȱ׷����µ�?
};