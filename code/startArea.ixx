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

	//테스트 아이템
	int pX = PlayerX();
	int pY = PlayerY();
	int pZ = PlayerZ();

	new Monster(6, 5, 0, 0);//NPC

	new ItemStack(pX + 2, pY + 1, pZ, {
		{2, 1}, {0, 5}, {23, 1}, {24, 10}, {1, 4}, {0, 1},
		{3, 1}, {12, 1}, {13, 1}, {14, 1}, {15, 1}, {16, 1},
		{17, 1}, {18, 1}, {4, 1}, {5, 8}, {88, 1}, {89, 1000},
		{91, 1000}, {82, 1},{389,1}
		}
	);
	
	//활과 석궁
	new ItemStack(3, 8, 0, { {383,1} });
	new ItemStack(4, 8, 0, { {385,30} });
	new ItemStack(3, 9, 0, { {382,1}});
	new ItemStack(4, 9, 0, { {384,30} });

	new Monster(5, 8, 8, 0);//허수아비

	new ItemStack(-1, 7, -1, { {388,1} });






	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////▼아이템 레시피 추가////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	availableRecipe = new ItemPocket(storageType::recipe);
	for (int i = 1; i <= 212; i++) ((ItemPocket*)availableRecipe)->addRecipe(i);

	//타일 세팅
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
			if (dx == -10 && ((dy + 30) % 6 < 3)) World::ins()->getTile(pX + dx, pY + dy, pZ).floor = 377;//노랑아스팔트
			else World::ins()->getTile(pX + dx, pY + dy, pZ).floor = 296;//검정아스팔트
		}
	}

	//오솔길
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

	//나무벽 설치
	//집 하단 5타일
	World::ins()->getTile(pX - 1, pY - 2, pZ).setWall(375);
	World::ins()->getTile(pX - 2, pY - 2, pZ).setWall(375);
	new Prop(pX - 3, pY - 2, pZ, 291);//나무문 설치
	World::ins()->getTile(pX - 4, pY - 2, pZ).setWall(375);
	World::ins()->getTile(pX - 5, pY - 2, pZ).setWall(375);

	//집 우측 4타일
	World::ins()->getTile(pX - 1, pY - 3, pZ).setWall(375);
	World::ins()->getTile(pX - 1, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX - 1, pY - 5, pZ).setWall(375);
	World::ins()->getTile(pX - 1, pY - 6, pZ).setWall(375);

	//집 좌측 4타일
	World::ins()->getTile(pX - 5, pY - 3, pZ).setWall(375);
	World::ins()->getTile(pX - 5, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX - 5, pY - 5, pZ).setWall(375);
	World::ins()->getTile(pX - 5, pY - 6, pZ).setWall(375);

	//잡 상단 중앙 3타일
	World::ins()->getTile(pX - 2, pY - 6, pZ).setWall(375);
	World::ins()->getTile(pX - 3, pY - 6, pZ).setWall(375);
	World::ins()->getTile(pX - 4, pY - 6, pZ).setWall(375);

	new Prop(pX - 4, pY - 5, pZ, 295);//책장
	new Prop(pX - 2, pY - 5, pZ, 294);//침대

	new Prop(pX - 4, pY - 3, pZ, 298);//상승계단
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			World::ins()->getTile(pX - 3 + dx, pY - 4 + dy, pZ + 1).floor = 292;
		}
	}

	new Prop(pX - 2, pY - 3, pZ, 299);//하강계단


	//철조망
	for (int i = 0; i < 17; i++)//상단
	{
		World::ins()->getTile(pX - 5 + i, pY - 9, pZ).setWall(376);
	}

	for (int i = 0; i < 20; i++)//우측
	{
		if (pY - 8 + i != 1 && pY - 8 + i != 2 && pY - 8 + i != 3)
		{
			World::ins()->getTile(pX + 11, pY - 8 + i, pZ).setWall(376);
		}
	}

	for (int i = 0; i < 17; i++)//상단
	{
		World::ins()->getTile(pX + 11 - i, pY + 12, pZ).setWall(376);
	}

	//철조망 우측 입구 전통등 2개
	new Prop(pX + 12, pY + 0, pZ + 0, 118);//볼라드등
	new Prop(pX + 12, pY + 4, pZ + 0, 118);//볼라드등


	//지하
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -2; dy <= 2; dy++)
			{

				World::ins()->getTile(pX - 2 + dx, pY - 3 + dy, pZ - 1).destoryWall();
			}
		}

		new Prop(pX - 2, pY - 5, pZ - 1, 211);//전통등
		{
			int cx = pX - 2 + 1;
			int cy = pY - 3 + 2;

			for (int i = 1; i < 9; i++) World::ins()->getTile(cx, cy + i, pZ - 1).destoryWall();
			new Prop(cx, cy + 1, pZ - 1, 291);//나무문 설치

			for (int i = 1; i < 9; i++) World::ins()->getTile(cx + i, cy, pZ - 1).destoryWall();
			Prop* door2 = new Prop(cx + 1, cy, pZ - 1, 291);//나무문 설치
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

			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railSwitchWS);//전환기

			int secondCursorX = cursorX+1;
			int secondCursorY = cursorY;
			for (int i = 0; i < 22; i++)
			{
				new Prop(secondCursorX, secondCursorY, pZ - 1, itemVIPCode::railRL);//나무문 설치
				secondCursorX++;
			}

			cursorY++;

			for (int i = 0; i < 26; i++)
			{
				new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTB);//나무문 설치
				cursorY++;
			}

			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTL);//나무문 설치
			cursorX--;

			for (int i = 0; i < 11; i++)
			{
				new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railRL);//나무문 설치
				cursorX--;
			}
			new Prop(cursorX, cursorY, pZ - 1, itemVIPCode::railTR);//나무문 설치


			//지하철 설치
			int vX = pX - 2 + 1 + 8;
			int vY = pY - 3 + 2;
			Vehicle* myTrainPower = new Vehicle(vX, vY, pZ - 1, 48);//차량 설치
			{

				myTrainPower->vehType = vehFlag::train;
				myTrainPower->isVehicle = true;

				///////////////////////차량 기초 프레임//////////////////////////////////////
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

			//지하철 설치
			{
				int vX = pX - 2 + 1 + 8;
				int vY = pY - 3 + 2 + 8;
				Vehicle* myTrain = new Vehicle(vX, vY, pZ - 1, 48);//차량 설치
				myTrainPower->rearTrain = myTrain;
				myTrain->isVehicle = true;
				myTrain->vehType = vehFlag::train;

				///////////////////////차량 기초 프레임//////////////////////////////////////
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
			//	new Prop(endX + 3, targetY, pZ - 1, 303);//나무문 설치
			//}

			//new Prop(endX + 3, endY - 20, pZ - 1, 317);//나무문 설치

			//for (int targetY = endY - 21; targetY >= endY - 50; targetY--)
			//{
			//	new Prop(endX + 3, targetY, pZ - 1, 303);//나무문 설치
			//}

			//for (int targetX = endX + 1; targetX >= endX - 30; targetX--)
			//{
			//	new Prop(targetX, endY - 20, pZ - 1, 303);//나무문 설치
			//}

		}
	}

	//집 바닥 타일
	for (int dx = 0; dx < 5; dx++)
	{
		for (int dy = 0; dy < 4; dy++)
		{
			World::ins()->getTile(pX - 5 + dx, pY - 5 + dy, pZ).floor = 292;
		}
	}

	new Prop(pX + 2, pY - 1, pZ, 297);//표지판


	//유리벽 설치
	World::ins()->getTile(pX + 2, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX + 2, pY - 3, pZ).setWall(114);
	World::ins()->getTile(pX + 2, pY - 2, pZ).setWall(114);
	World::ins()->getTile(pX + 4, pY - 4, pZ).setWall(114);
	World::ins()->getTile(pX + 4, pY - 3, pZ).setWall(114);

	//얕은 물 타일(연못)
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

	//하단연못
	World::ins()->getTile(pX + 5, pY + 8, pZ).floor = 225;
	World::ins()->getTile(pX + 5, pY + 9, pZ).floor = 225;

	for (int dx = -3; dx <= 2; dx++)
	{
		World::ins()->getTile(pX + 5 + dx, pY + 10, pZ).floor = 225;
		World::ins()->getTile(pX + 5 + dx, pY + 11, pZ).floor = 225;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////▼설치물 추가////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




	//new Prop(pX + 1 - 1, pY - 2, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 1, pY - 2 + 1, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 6, pY - 5, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 7, pY - 3, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 7, pY - 4, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 5, pY - 1 + 1, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 5 - 1, pY - 1, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 5 - 1, pY - 1 + 1, pZ, 234);//벚꽃낙엽 설치
	//new Prop(pX + 7 - 1, pY + 1, pZ, 234);//벚꽃낙엽 설치


	for (int x = 0; x <= 10; x++) {
		for (int y = -8; y <= -2; y++) {
			SNOW(x, y, pZ);
		}
	}

	for (int x = 4; x <= 10; x++) SNOW(x, -1, pZ);

	for (int x = 6; x <= 10; x++) SNOW(x, 0, pZ);

	SNOW(3, 4, pZ);
	SNOW(3, 5, pZ);
	SNOW(4, 5, pZ);

	for (int x = 5; x <= 13; x++) {
		for (int y = 4; y <= 7; y++) {
			SNOW(x, y, pZ);
		}
	}

	for (int x = -5; x <= 1; x++) {
		for (int y = 4; y <= 11; y++) {
			SNOW(x, y, pZ);
		}
	}

	for (int x = 2; x <= 3; x++) {
		for (int y = 6; y <= 8; y++) {
			SNOW(x, y, pZ);
		}
	}

	for (int x = -2; x <= 1; x++) {
		for (int y = 2; y <= 3; y++) {
			SNOW(x, y, pZ);
		}
	}

	SNOW(-3, 3, pZ);

	for (int x = 11; x <= 13; x++) {
		for (int y = -8; y <= -1; y++) {
			SNOW(x, y, pZ);
		}
	}

	for (int x = -6; x <= -1; x++) {
		for (int y = -9; y <= -7; y++) {
			SNOW(x, y, pZ);
		}
	}

	SNOW(6, 3, pZ);
	SNOW(9, 3, pZ);
	SNOW(10, 3, pZ);
	SNOW(-5, 3, pZ);
	SNOW(7, 1, pZ);
	SNOW(-5, 2, pZ);

	for (int y = -7; y <= -3; y++) SNOW(14, y, pZ);


	new Prop(1, -3, 0, 117);//나무 설치

	new Prop(pX + 3, pY - 2, pZ, 239);//나무 설치

	new Prop(pX + 5, pY - 1, pZ, 247);//나무 설치

	new Prop(0, -5, pZ, 238);//나무 설치




	new Prop(pX + 7, pY + 1, pZ, 237);//나무 설치


	new Prop(pX + 4, pY - 5, pZ, 248);//나무 설치

	new Prop(pX + 9, pY - 4, pZ, 237);//나무 설치
	new Prop(pX + 10, pY - 1, pZ, 244);//사과나무 설치

	new Prop(pX - 2, pY + 36, pZ, 242);//야자나무 설치


	new Prop(pX, pY - 20, pZ, 237);

	new Prop(-4, 5, 0, 245);//사과나무 설치


	new Prop(pX + 3, pY + 3, pZ, 338);//고철 설치
	new Prop(pX + 3 + 1, pY + 3, pZ, 338);//고철 설치
	new Prop(pX + 3 + 2, pY + 3, pZ, 338);//고철 설치
	new Prop(pX + 3 + 1, pY + 3 + 1, pZ, 338);//고철 설치

	new Prop(pX + 10, pY + 11, pZ, 338);//고철 설치
	new Prop(pX + 10, pY + 11 - 1, pZ, 338);//고철 설치
	new Prop(pX + 10, pY + 11 - 2, pZ, 338);//고철 설치

	new Prop(pX + 10 - 1, pY + 11, pZ, 338);//고철 설치
	new Prop(pX + 10 - 1, pY + 11 - 1, pZ, 338);//고철 설치



	//잔디
	World::ins()->getTile(pX + -2, pY + 3, pZ).floor = 220;
	World::ins()->getTile(pX + -1, pY + 3, pZ).floor = 220;
	World::ins()->getTile(pX + 0, pY + 3, pZ).floor = 220;
	World::ins()->getTile(pX + 1, pY + 3, pZ).floor = 220;

	World::ins()->getTile(pX + -2, pY + 4, pZ).floor = 220;
	World::ins()->getTile(pX + -1, pY + 4, pZ).floor = 220;
	World::ins()->getTile(pX + 0, pY + 4, pZ).floor = 220;
	World::ins()->getTile(pX + 1, pY + 4, pZ).floor = 220;

	new Prop(pX + -2, pY + 3, pZ, 270);//꽃 설치
	new Prop(pX + -1, pY + 3, pZ, 265);//꽃 설치
	new Prop(pX + 0, pY + 3, pZ, 266);//꽃 설치
	new Prop(pX + 1, pY + 3, pZ, 267);//꽃 설치

	new Prop(pX + -2, pY + 4, pZ, 271);//꽃 설치
	new Prop(pX + -1, pY + 4, pZ, 268);//꽃 설치
	new Prop(pX + 0, pY + 4, pZ, 269);//꽃 설치
	new Prop(pX + 1, pY + 4, pZ, 270);//꽃 설치


	new Prop(pX + 6, pY - 4, pZ, 270);//꽃 설치

	new Prop(0, -1, 0, 211);//전통 등 설치
	new Prop(4, 0, 0, 211);//볼라드 등 설치

	//울타리 설치
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

	//전선 설치
	new Prop(pX + 8, pY + 1, pZ, 143);
	new Prop(pX + 8, pY + 0, pZ, 143);
	new Prop(pX + 8, pY - 1, pZ, 143);
	new Prop(pX + 8, pY - 2, pZ, 143);
	new Prop(pX + 9, pY + 0, pZ, 143);

	//배관 설치
	new Prop(pX + 3, pY + 6, pZ, 144);
	new Prop(pX + 4, pY + 6, pZ, 144);
	new Prop(pX + 5, pY + 6, pZ, 144);
	new Prop(pX + 6, pY + 6, pZ, 144);
	new Prop(pX + 5, pY + 7, pZ, 144);

	//종교

	new Prop(pX - 3, pY - 7, pZ, 213);
	new Prop(pX - 1, pY - 7, pZ, 214);
	new Prop(pX + 1, pY - 7, pZ, 216);
	new Prop(pX + 3, pY - 7, pZ, 217);
	new Prop(pX + 5, pY - 7, pZ, 218);
	new Prop(pX + 7, pY - 7, pZ, 219);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int vX = pY - 8;
	int vY = pY + 3;
	Vehicle* myCar = new Vehicle(vX, vY, pZ, 48);//차량 설치
	myCar->isVehicle = true;
	myCar->vehType = vehFlag::car;

	///////////////////////차량 기초 프레임//////////////////////////////////////
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
	//////////////////////////▼최상단 4타일////////////////////////////////////
	myCar->addPart(vX - 1, vY - 2, { 142,119,126 }); //바퀴, 차벽
	myCar->addPart(vX, vY - 2, { 119 }); //차벽
	myCar->addPart(vX + 1, vY - 2, { 119 }); //차벽
	myCar->addPart(vX + 2, vY - 2, { 142,119,126 }); //바퀴, 차벽
	//////////////////////////▼중상단 4타일////////////////////////////////////
	myCar->addPart(vX - 1, vY - 1, 121); //차유리
	myCar->addPart(vX, vY - 1, 121); //차유리
	myCar->addPart(vX + 1, vY - 1, 121); //차유리
	myCar->addPart(vX + 2, vY - 1, 121); //차유리
	////////////////////////////////▼운전석 4타일///////////////////////////////
	myCar->addPart(vX - 1, vY, { 120 });//차문
	myCar->addPart(vX, vY, { 122, 123, 99, 128 });//통로,의자
	myCar->addPart(vX + 1, vY, { 122, 123, 128 });//통로,의자
	myCar->addPart(vX + 2, vY, { 120 });//차문
	//////////////////////////▼운전석 아래 통로 4타일/////////////////////////////
	myCar->addPart(vX - 1, vY + 1, { 119 });//차벽
	myCar->addPart(vX, vY + 1, { 122, 128 });//통로
	myCar->addPart(vX + 1, vY + 1, { 122, 128,129 });//통로
	myCar->addPart(vX + 2, vY + 1, { 119 });//차벽
	///////////////////////////////▼뒷자석 4타일/////////////////////
	myCar->addPart(vX - 1, vY + 2, { 120 });//차문
	myCar->addPart(vX, vY + 2, { 122, 123, 128 });//통로,의자
	myCar->addPart(vX + 1, vY + 2, { 122, 123, 128 });//통로,의자
	myCar->addPart(vX + 2, vY + 2, { 120 });//차문
	///////////////////////////////▼최후방 4타일///////////////////////////
	myCar->addPart(vX - 1, vY + 3, { 119,127 });//차벽
	myCar->addPart(vX, vY + 3, { 124 });//트렁크문
	myCar->addPart(vX + 1, vY + 3, { 124 });//트렁크문
	myCar->addPart(vX + 2, vY + 3, { 119,127 });//차벽



	///////////////////오토바이///////////////////////////////////////////
	Vehicle* myMoto = new Vehicle(vX + 6, vY + 5, pZ, 48);//차량 설치
	myMoto->isVehicle = true;
	myMoto->extendPart(vX + 6, vY + 4, 48);
	myMoto->extendPart(vX + 6, vY + 6, 48);

	myMoto->addPart(vX + 6, vY + 4, { 102,134 });
	myMoto->addPart(vX + 6, vY + 5, 132);
	myMoto->addPart(vX + 6, vY + 6, 102);

	////////////////////자전거////////////////////////////////////////////
	Vehicle* myBike = new Vehicle(vX + 9, vY + 4, pZ, 48);//차량 설치
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
		Vehicle* myHeli = new Vehicle(cx, cy, pZ, 48);//차량 설치
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

		//상단 3유리
		myHeli->addPart(cx + 1, cy - 1, { 121 });
		myHeli->addPart(cx, cy - 1, { 121 });
		myHeli->addPart(cx - 1, cy - 1, { 121 });

		//플레이어 3타일(조종석)
		myHeli->addPart(cx + 1, cy, { 120 });
		myHeli->addPart(cx, cy, { 122,123, 128 });
		myHeli->addPart(cx - 1, cy, { 120 });

		//짐칸
		myHeli->addPart(cx + 1, cy + 1, { 119 });
		myHeli->addPart(cx, cy + 1, { 122, 128, 314});
		myHeli->addPart(cx - 1, cy + 1, { 119 });
		
		//꼬리 격리 3칸벽
		myHeli->addPart(cx + 1, cy + 2, { 119 });
		myHeli->addPart(cx, cy + 2, { 119 });
		myHeli->addPart(cx - 1, cy + 2, { 119 });

		//꼬리 3칸
		myHeli->addPart(cx, cy + 3, { 119 });
		myHeli->addPart(cx, cy + 4, { 119 });
		myHeli->addPart(cx, cy + 5, { 119, 315 });
	}

	//수레 3종
	Vehicle* cart1 = new Vehicle(10, 5, 0, 378);//차량 설치
	cart1->isVehicle = true;
	cart1->vehType = vehFlag::car;

	Vehicle* cart2 = new Vehicle(8, 5, 0, 379);//차량 설치
	cart2->isVehicle = true;
	cart2->vehType = vehFlag::car;

	Vehicle* cart3 = new Vehicle(6, 5, 0, 137);//차량 설치
	cart3->isVehicle = true;
	cart3->vehType = vehFlag::car;


	//new Vehicle(8, 5, 0, 379);//차량 설치
	//new Vehicle(6, 5, 0, 137);//차량 설치


	///////////////////////////////////////////////////////////////
	//new Vehicle(pX, pY + 2, pZ, 140);//모닥불 설치

	World::ins()->createSector(0, 0, 0);

	Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
	
};