﻿#define SNOW(x, y, z) (World::ins()->getTile(x, y, z).hasSnow = true)
#include <sol/sol.hpp>

export module startArea;

import globalVar;
import wrapVar;
import HUD;
import Player;
import ItemData;
import ItemPocket;
import ItemStack;
import World;
import Vehicle;
import Prop;
import Monster;

export void startArea()
{
	new HUD();

	World::ins()->getTile(0, 0, 0).EntityPtr = std::make_unique<Player>(0, 0, 0);
	PlayerPtr = (Player*)TileEntity(0, 0, 0);
	PlayerPtr->setGrid(0, 0, 0);
	PlayerPtr->setDstGrid(0, 0);


	PlayerPtr->updateWalkable(PlayerX() + 1, PlayerY());
	PlayerPtr->updateWalkable(PlayerX() + 1, PlayerY() - 1);
	PlayerPtr->updateWalkable(PlayerX(), PlayerY() - 1);
	PlayerPtr->updateWalkable(PlayerX() - 1, PlayerY() - 1);
	PlayerPtr->updateWalkable(PlayerX() - 1, PlayerY());
	PlayerPtr->updateWalkable(PlayerX() - 1, PlayerY() + 1);
	PlayerPtr->updateWalkable(PlayerX(), PlayerY() + 1);
	PlayerPtr->updateWalkable(PlayerX() + 1, PlayerY() + 1);

	//테스트 아이템
	int pX = PlayerX();
	int pY = PlayerY();
	int pZ = PlayerZ();


	createItemStack({ 2, 1, 0 }, {
		{2, 1}, {0, 5}, {23, 1}, {24, 10}, {1, 4}, {0, 1},
		{3, 1}, {12, 1}, {13, 1}, {14, 1}, {15, 1}, {16, 1},
		{17, 1}, {18, 1}, {4, 1}, {5, 8}, {88, 1}, {89, 1000},
		{91, 1000}, {82, 1},{389,1}, {386,2}, {387,1}
		}
	);


	createItemStack({ -5, 2, 0 }, {
		{373, 1}
		}
	);

	createItemStack({ 2, 8, 0 }, { {408,1} });//화살통
	createItemStack({ 2, 9, 0 }, { {409,1} });//볼트통

	//활과 석궁
	createItemStack({ 3, 8, 0 }, { {383,1} });
	createItemStack({ 4, 8, 0 }, { {385,30} });
	createItemStack({3, 9, 0 }, { {382,1} });
	createItemStack({4, 9, 0 }, { {384,30} });

	createItemStack({7, -4, 0 }, { {391,1} }); //벌목도끼

	createItemStack({-5, 1, 0 }, { {394,1} }); //낚시대

	createItemStack({-3, -4, -1 }, { {388,1} });//곡괭이
	createItemStack({-2, -4, -1 }, { {393,1} });//광부헬멧
	createItemStack({-1, -4, -1 }, { {395,1} });//삽



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////▼아이템 레시피 추가////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	availableRecipe = std::make_unique<ItemPocket>(storageType::recipe);
	for (int i = 1; i <= 212; i++)
	{
		if (itemDex[i].name != L"?")
		{
			if (itemDex[i].checkFlag(itemFlag::CANCRAFT))
			{
				availableRecipe.get()->addRecipe(i);
			}
		}
	}

	//타일 세팅
	for (int dx = -30; dx <= 30; dx++)
	{
		for (int dy = -30; dy <= 30; dy++)
		{
			setFloor({ dx,dy,0 }, itemRefCode::dirt);
		}
	}
	for (int dx = -30; dx <= 30; dx++)
	{
		for (int dy = -30; dy <= -2; dy++)
		{
			setFloor({ dx,dy,0 }, itemRefCode::grass);
		}
	}
	for (int dx = -6; dx >= -14; dx--)
	{
		for (int dy = -30; dy <= 30; dy++)
		{
			if (dx == -10 && ((dy + 30) % 6 < 3)) setFloor({ dx,dy,0 }, itemRefCode::yellowAsphalt); //노랑아스팔트
			else setFloor({ dx,dy,0 }, itemRefCode::blackAsphalt); //검정아스팔트
		}
	}

	//오솔길
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


	createProp({ 10, -8, 0 }, 96);//냉장고 설치
	ItemPocket* refri = TileProp(10, -8, 0)->leadItem.pocketPtr.get();
	refri->addItemFromDex({ { 4, 1 },{ 12, 1 },{ 410, 1 },{ 414, 1 },{ 415, 1 },{ 417, 1 } });
	createProp({ 9, -8, 0 }, 427);//탄통 설치
	TileProp(9, -8, 0)->leadItem.pocketPtr->addItemFromDex({ { 5,99},{ 15,99},{ 16,99},{ 17,99} ,{ 411,99},{ 412,99},{ 413,99},{ 416,99} ,{ 418,99 },{ 419,99 },{ 420,99 },{ 421,99 },{ 422,99 },{ 423,99 },{ 424,30 },{ 425,30 },{ 426,30 },{ 13,1 },{ 14,1 },{ 428,1 },{ 429,1 },{ 430,1 } });

	//나무벽 설치
	//집 하단 5타일
	setWall({ -1,-2,0 }, 375);
	setWall({ -2,-2,0 }, 375);
	createProp({ -3, -2, 0 }, 291);//나무문 설치
	setWall({ -4,-2,0 }, 375);
	setWall({ -5,-2,0 }, 375);
	//집 우측 4타일
	setWall({ -1,-3,0 }, 375);
	setWall({ -1,-4,0 }, 114);
	setWall({ -1,-5,0 }, 375);
	setWall({ -1,-6,0 }, 375);
	//집 좌측 4타일
	setWall({ -5,-3,0 }, 375);
	setWall({ -5,-4,0 }, 114);
	setWall({ -5,-5,0 }, 375);
	setWall({ -5,-6,0 }, 375);
	//잡 상단 중앙 3타일
	setWall({ -2,-6,0 }, 375);
	setWall({ -3,-6,0 }, 375);
	setWall({ -4,-6,0 }, 375);

	createProp({ -4, -5, 0 }, 295);//책장
	createProp({ -2, -5, 0 }, 294);//침대

	createProp({ -4, -3, 0 }, 298);//상승계단
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			setFloor({ -3 + dx, -4 + dy, 1 }, 292);
		}
	}

	createProp({ -2, -3, 0 }, 299);//하강계단


	//철조망
	for (int i = 0; i < 17; i++)//상단
	{
		setWall({ -5 + i, -9, 0 }, 376);
	}

	for (int i = 0; i < 20; i++)//우측
	{
		if (-8 + i != 1 && -8 + i != 2 && -8 + i != 3)
		{
			setWall({ 11, -8 + i, 0 }, 376);
		}
	}

	for (int i = 0; i < 17; i++)//상단
	{
		setWall({ 11 - i, 12, 0 }, 376);
	}

	//철조망 우측 입구 전통등 2개
	createProp({ 12, 0, 0 }, 118);//볼라드등
	createProp({ 12, 4, 0 }, 118);//볼라드등


	//철조망 아래 선로

	createProp({ -2, 15, 0 }, itemRefCode::railBR);
	for (int i = 0; i < 11; i++)  createProp({ -1 + i, 15, 0 }, itemRefCode::railRL);
	createProp({ 10, 15, 0 }, itemRefCode::railSwitchWS);
	for (int i = 0; i < 7; i++)  createProp({ 10, 16 + i, 0 }, itemRefCode::railTB);
	createProp({ 10, 23, 0 }, itemRefCode::railTL);
	for (int i = 0; i < 6; i++)  createProp({ 9 - i, 23, 0 }, itemRefCode::railRL);
	createProp({ 3, 23, 0 }, itemRefCode::railTR);
	for (int i = 0; i < 3; i++) createProp({ 3, 22 - i, 0 }, itemRefCode::railTB);
	createProp({ 3, 19, 0 }, itemRefCode::railBL);
	for (int i = 0; i < 4; i++) createProp({ 2 - i, 19, 0 }, itemRefCode::railRL);
	createProp({ -2, 19, 0 }, itemRefCode::railTR);
	for (int i = 0; i < 3; i++) createProp({ -2, 18 - i, 0 }, itemRefCode::railTB);

	for (int i = 0; i < 5; i++)  createProp({ 11 + i, 15, 0 }, itemRefCode::railRL);

	//지하
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

		createProp({ pX - 2, pY - 5, -1 }, 211);//전통등
		{
			int cx = -1;
			int cy = -1;

			for (int i = 1; i < 9; i++) DestroyWall(cx, cy + i, -1);
			createProp({ cx, cy + 1, -1 }, 291);//나무문 설치

			for (int i = 1; i < 9; i++) DestroyWall(cx + i, cy, -1);
			createProp({ cx + 1, cy, -1 }, 291);
			Prop* door2 = TileProp(cx + 1, cy, -1);//나무문 설치
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
			//	createProp(6, 39 - i, -1, itemRefCode::wideRailVLeft);
			//	createProp(7, 39 - i, -1, itemRefCode::wideRailVMid);
			//	createProp(8, 39 - i, -1, itemRefCode::wideRailVRight);
			//	cursorY--;
			//}

			for (int i = 0; i <= 78; i++)
			{
				createProp({ 6, 39 - i, -1 }, itemRefCode::wideRailVLeft);
				createProp({ 7, 39 - i, -1 }, itemRefCode::wideRailVMid);
				createProp({ 8, 39 - i, -1 }, itemRefCode::wideRailVRight);
				cursorY--;
			}


			//지하철 설치
			int vX = 7;
			int vY = -1;
			Vehicle* myTrainPower = new Vehicle(vX, vY, -1, 48);//차량 설치
			{
				myTrainPower->name = L"동력차";
				myTrainPower->vehType = vehFlag::train;
				myTrainPower->isPowerTrain = true;

				///////////////////////차량 기초 프레임//////////////////////////////////////
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

			//지하철(화물칸) 설치

			Vehicle* myTrain = new Vehicle(7, 7, -1, 48);//차량 설치

			{
				int vX = 7;
				int vY = 7;
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
			//	createProp(endX + 3, targetY, pZ - 1, 303);//나무문 설치
			//}

			//createProp(endX + 3, endY - 20, pZ - 1, 317);//나무문 설치

			//for (int targetY = endY - 21; targetY >= endY - 50; targetY--)
			//{
			//	createProp(endX + 3, targetY, pZ - 1, 303);//나무문 설치
			//}

			//for (int targetX = endX + 1; targetX >= endX - 30; targetX--)
			//{
			//	createProp(targetX, endY - 20, pZ - 1, 303);//나무문 설치
			//}
		}
	}

	//집 바닥 타일
	for (int dx = 0; dx < 5; dx++)
	{
		for (int dy = 0; dy < 4; dy++)
		{
			setFloor({ -5 + dx, -5 + dy, 0 }, 292);
		}
	}
	createProp({ 2, -1, 0 }, 297);//표지판
	//유리벽 설치
	setWall({ 2,-4,0 }, 114);
	setWall({ 2,-3,0 }, 114);
	setWall({ 2,-2,0 }, 114);
	setWall({ 4,-4,0 }, 114);
	setWall({ 4,-3,0 }, 114);
	//얕은 물 타일(연못)
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
			if (dy <= 1) setFloor({ startX + dx, startY + dy, 0 }, 231);//얕은해수
			else if (dy <= 5) setFloor({ startX + dx, startY + dy, 0 }, 381);//모래
			else if (dy <= 7) setFloor({ startX + dx, startY + dy, 0 }, 231);//얕은해수
			else setFloor({ startX + dx, startY + dy, 0 }, 232);//깊은해수
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
	//하단연못
	setFloor({ 5,8,0 }, 225);
	setFloor({ 5,9,0 }, 225);
	for (int dx = -3; dx <= 2; dx++)
	{
		setFloor({ 5 + dx,10,0 }, 225);
		setFloor({ 5 + dx,11,0 }, 225);
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////▼설치물 추가////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//createProp({pX + 1 - 1, pY - 2, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 1, pY - 2 + 1, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 6, pY - 5, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 7, pY - 3, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 7, pY - 4, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 5, pY - 1 + 1, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 5 - 1, pY - 1, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 5 - 1, pY - 1 + 1, pZ}, 234);//벚꽃낙엽 설치
	//createProp({pX + 7 - 1, pY + 1, pZ}, 234);//벚꽃낙엽 설치


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


	createProp({ 1, -3, 0 }, 117);//나무 설치

	createProp({ pX + 3, pY - 2, 0 }, 239);//나무 설치

	createProp({ pX + 5, pY - 1, 0 }, 247);//나무 설치

	createProp({ 0, -5, 0 }, 238);//나무 설치




	createProp({ pX + 7, pY + 1, 0 }, 237);//나무 설치


	createProp({ pX + 4, pY - 5, 0 }, 248);//나무 설치

	createProp({ pX + 9, pY - 4, 0 }, 237);//나무 설치
	createProp({ pX + 10, pY - 1, 0 }, 244);//사과나무 설치

	createProp({ pX - 2, pY + 39, 0 }, 242);//야자나무 설치


	createProp({ pX, pY - 20, 0 }, 237);

	createProp({ -4, 5, 0 }, 245);//사과나무 설치


	createProp({ pX + 3, pY + 3, 0 }, 338);//고철 설치
	createProp({ pX + 3 + 1, pY + 3, 0 }, 338);//고철 설치
	createProp({ pX + 3 + 2, pY + 3, 0 }, 338);//고철 설치
	createProp({ pX + 3 + 1, pY + 3 + 1, 0 }, 338);//고철 설치

	createProp({ pX + 10, pY + 11, 0 }, 338);//고철 설치
	createProp({ pX + 10, pY + 11 - 1, 0 }, 338);//고철 설치
	createProp({ pX + 10, pY + 11 - 2, 0 }, 338);//고철 설치

	createProp({ pX + 10 - 1, pY + 11, 0 }, 338);//고철 설치
	createProp({ pX + 10 - 1, pY + 11 - 1, 0 }, 338);//고철 설치



	//잔디
	setFloor({ pX - 2, pY + 3, 0 }, 220);
	setFloor({ pX - 1, pY + 3, 0 }, 220);
	setFloor({ pX, pY + 3, 0 }, 220);
	setFloor({ pX + 1, pY + 3, 0 }, 220);

	setFloor({ pX - 2, pY + 4, 0 }, 220);
	setFloor({ pX - 1, pY + 4, 0 }, 220);
	setFloor({ pX, pY + 4, 0 }, 220);
	setFloor({ pX + 1, pY + 4, 0 }, 220);

	createProp({ pX - 2, pY + 3, 0 }, 270);//꽃 설치
	createProp({pX - 1, pY + 3, 0 }, 265);//꽃 설치
	createProp({pX, pY + 3, 0 }, 266);//꽃 설치
	createProp({pX + 1, pY + 3, 0 }, 267);//꽃 설치

	createProp({pX - 2, pY + 4, 0 }, 271);//꽃 설치
	createProp({pX - 1, pY + 4, 0 }, 268);//꽃 설치
	createProp({pX, pY + 4, 0 }, 269);//꽃 설치
	createProp({pX + 1, pY + 4, 0 }, 270);//꽃 설치


	createProp({pX + 6, pY - 4, 0 }, 270);//꽃 설치

	createProp({0, -1, 0 }, 211);//전통 등 설치
	createProp({4, 0, 0 }, 211);//볼라드 등 설치

	//울타리 설치
	createProp({pX - 3, pY + 2, 0 }, 206);
	createProp({pX - 2, pY + 2, 0 }, 206);
	createProp({pX - 1, pY + 2, 0 }, 206);
	createProp({pX, pY + 2, 0 }, 206);
	createProp({pX + 1, pY + 2, 0 }, 206);
	createProp({pX + 2, pY + 2, 0 }, 206);
	createProp({pX + 3, pY + 2, 0 }, 206);

	createProp({pX - 3, pY + 3, 0 }, 206);
	createProp({pX - 3, pY + 4, 0 }, 206);

	createProp({pX + 2, pY + 3, 0 }, 206);
	createProp({pX + 2, pY + 4, 0 }, 206);

	createProp({pX - 3, pY + 5, 0 }, 206);
	createProp({pX - 2, pY + 5, 0 }, 206);
	createProp({pX - 1, pY + 5, 0 }, 206);
	createProp({pX, pY + 5, 0 }, 206);
	createProp({pX + 1, pY + 5, 0 }, 206);
	createProp({pX + 2, pY + 5, 0 }, 206);

	//전선 설치
	createProp({pX + 8, pY + 1, 0 }, 143);
	createProp({pX + 8, pY, 0 }, 143);
	createProp({pX + 8, pY - 1, 0 }, 143);
	createProp({pX + 8, pY - 2, 0 }, 143);
	createProp({pX + 9, pY, 0 }, 143);

	//배관 설치
	createProp({pX + 3, pY + 6, 0 }, 144);
	createProp({pX + 4, pY + 6, 0 }, 144);
	createProp({pX + 5, pY + 6, 0 }, 144);
	createProp({pX + 6, pY + 6, 0 }, 144);
	createProp({pX + 5, pY + 7, 0 }, 144);

	//종교


	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int vX = pY - 8;
	int vY = pY + 3;
	Vehicle* myCar = new Vehicle(vX, vY, 0, 48);//차량 설치
	myCar->name = L"SUV";
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
	myCar->addPart(vX - 1, vY - 2, { 142,119,126 });
	myCar->addPart(vX, vY - 2, { 119 });
	myCar->addPart(vX + 1, vY - 2, { 119 });
	myCar->addPart(vX + 2, vY - 2, { 142,119,126 });
	//////////////////////////▼중상단 4타일////////////////////////////////////
	myCar->addPart(vX - 1, vY - 1, 121);
	myCar->addPart(vX, vY - 1, {121,100});
	myCar->addPart(vX + 1, vY - 1, 121);
	myCar->addPart(vX + 2, vY - 1, 121);
	////////////////////////////////▼운전석 4타일///////////////////////////////
	myCar->addPart(vX - 1, vY, { 120 });
	myCar->addPart(vX, vY, { 122, 123, 99, 128 });
	myCar->addPart(vX + 1, vY, { 122, 123, 128 });
	myCar->addPart(vX + 2, vY, { 120 });
	//////////////////////////▼운전석 아래 통로 4타일/////////////////////////////
	myCar->addPart(vX - 1, vY + 1, { 119 });
	myCar->addPart(vX, vY + 1, { 122, 128 });
	myCar->addPart(vX + 1, vY + 1, { 122, 128,129 });
	myCar->addPart(vX + 2, vY + 1, { 119 });
	///////////////////////////////▼뒷자석 4타일/////////////////////
	myCar->addPart(vX - 1, vY + 2, { 120, 101 });
	{
		ItemPocket* partPocket = myCar->partInfo[{vX - 1, vY + 2}].get();
		for (int i = 0; i < partPocket->itemInfo.size(); i++)
		{
			if (partPocket->itemInfo[i].itemCode == 101)
			{
				partPocket->itemInfo[i].pocketPtr->addItemFromDex(itemRefCode::gasoline, 90);
			}
		}
	}
	myCar->addPart(vX, vY + 2, { 122, 123, 128 });
	myCar->addPart(vX + 1, vY + 2, { 122, 123, 128 });
	myCar->addPart(vX + 2, vY + 2, { 120 });
	///////////////////////////////▼최후방 4타일///////////////////////////
	myCar->addPart(vX - 1, vY + 3, { 119,127 });
	myCar->addPart(vX, vY + 3, { 124 });
	myCar->addPart(vX + 1, vY + 3, { 124 });
	myCar->addPart(vX + 2, vY + 3, { 119,127 });



	///////////////////오토바이///////////////////////////////////////////
	Vehicle* myMoto = new Vehicle(vX + 6, vY + 5, 0, 48);
	myMoto->extendPart(vX + 6, vY + 4, 48);
	myMoto->extendPart(vX + 6, vY + 6, 48);

	myMoto->addPart(vX + 6, vY + 4, { 102,134 });
	myMoto->addPart(vX + 6, vY + 5, 132);
	myMoto->addPart(vX + 6, vY + 6, 102);

	////////////////////자전거////////////////////////////////////////////
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
		myHeli->name = L"헬기";
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

	//수레 3종
	Vehicle* cart1 = new Vehicle(10, 5, 0, 378);
	cart1->vehType = vehFlag::car;

	Vehicle* cart2 = new Vehicle(8, 5, 0, 379);
	cart2->vehType = vehFlag::car;

	Vehicle* cart3 = new Vehicle(6, 5, 0, 137);
	cart3->vehType = vehFlag::car;

	Vehicle* cart4 = new Vehicle(7, -5, 0, 378);
	cart4->vehType = vehFlag::car;

	//광차
	Vehicle* minecart1 = new Vehicle(3, 15, 0, 405);
	minecart1->vehType = vehFlag::minecart;
	minecart1->addPart(3, 15, { itemRefCode::minecartController });
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

	PlayerPtr->updateVision(PlayerPtr->eyeSight);
	
};