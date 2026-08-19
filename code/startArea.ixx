module;
#define SNOW(x, y, z) (World::ins()->getTile(x, y, z).hasSnow = true)

export module startArea;

import std;
import util;
import constVar;
import globalVar;
import HUD;
import Player;
import Item;
import ItemStack;
import World;
import Vehicle;
import Prop;
import Monster;
import statusEffect;

export void startArea()
{
	new HUD();

	World::ins()->getTile(0, 0, 0).EntityPtr = std::make_unique<Player>(0, 0, 0);
	PlayerPtr = (Player*)TileEntity(0, 0, 0);
	PlayerPtr->setGrid(0, 0, 0);
	PlayerPtr->setDstGrid(0, 0);

	//  startArea가 그 위에 home base 영역 props/items를 배치.)

	PlayerInfo().statusEffectVec.push_back({ statusEffectFlag::hungry, -1 });
	PlayerInfo().statusEffectVec.push_back({ statusEffectFlag::dehydrated, -1 });
	PlayerInfo().statusEffectVec.push_back({ statusEffectFlag::tired, -1 });



	createItemStack({ 2, 1, 0 }, {
{ itemID::backpack, 1},  { itemID::pipeSpear, 1}, { itemID::policeShield, 10}, { itemID::ne555, 4}, { itemID::none, 1},
{ itemID::tripleSword, 1}, { itemID::assaultRifle, 1}, { itemID::rifleMagazine, 1}, { itemID::rifleDrumMagazine, 1}, { itemID::nato556Round, 1}, { itemID::nato556Tracer, 1},
{ itemID::nato556AP, 1}, { itemID::rifleBayonet, 1}, { itemID::revolver, 1}, { itemID::magnum357Round, 8}, { itemID::fermentedEthanol, 1}, { itemID::ethanol, 1000},
{ itemID::benzene, 1000},{itemID::gasoline, 1000}, { itemID::distiller, 1},{ itemID::crusaderHelmet,1}, { itemID::longsword,2}, { itemID::kiteShield,1},
{itemID::tshirt, 1}, {itemID::bra, 1}, {itemID::panties, 1},{ itemID::bread,99}
		}
	);

	createItemStack({ 12, 17, 0 }, {
		{ itemID::battery, 1},{ itemID::batteryPack, 1},
		{ itemID::nipper, 1}, { itemID::plier, 1}, { itemID::multimeter, 1}, { itemID::electricSwitch, 10}, { itemID::fuseCartridge, 4}, { itemID::transistor, 1},
		{ itemID::relay, 1}, { itemID::inductor, 1}, { itemID::capacitor, 1}, { itemID::supercapacitor, 1}, { itemID::resistor, 1}
		}
	);




	//의약품 상자
	{
		createItemStack({ 0, -19, 0 }, { { itemID::firstAidKit,1 } });
		ItemPocket* aidKitInside = TileItemStack({ 0, -19, 0 })->getPocket()->itemInfo[0].pocketPtr.get();
		aidKitInside->addItemFromDex({ { itemID::smallTube, 1 }, { itemID::tube, 1 }, { itemID::splint, 4 }, { itemID::bandage, 10 }, { itemID::medicineBottle, 1 }, { itemID::adhesiveBandage, 20 } });
		std::vector<ItemData>& targetItemInfo = aidKitInside->itemInfo;
		for (int i = 0; i < targetItemInfo.size(); i++)
		{
			if (targetItemInfo[i].itemCode == itemID::smallTube)
			{
				targetItemInfo[i].pocketPtr->addItemFromDex(itemID::healingOintment, 9);
			}
			else if (targetItemInfo[i].itemCode == itemID::tube)
			{
				targetItemInfo[i].pocketPtr->addItemFromDex(itemID::toothpaste, 100);
			}
			else if (targetItemInfo[i].itemCode == itemID::medicineBottle)
			{
				targetItemInfo[i].pocketPtr->addItemFromDex(itemID::povidoneIodine, 30);
			}
		}
	}




	createItemStack({ -5, 2, 0 }, { { itemID::plasticBottle, 1},{ itemID::largePlasticBottle, 1},{ itemID::gasCan, 1},{ itemID::milkJug, 1},{ itemID::glassBottle, 1} });//페트병
	

	createItemStack({ 2, 8, 0 }, { {itemID::arrowQuiver,1} });//화살통
	createItemStack({ 2, 9, 0 }, { {itemID::boltQuiver,1} });//볼트통

	//활과 석궁
	createItemStack({ 3, 8, 0 }, { { itemID::bow,1} });
	createItemStack({ 4, 8, 0 }, { { itemID::arrow,30} });
	createItemStack({ 3, 9, 0 }, { { itemID::crossbow,1} });
	createItemStack({ 4, 9, 0 }, { { itemID::bolt,30} });

	createMonster({ 8,8,0 }, 5);//허수아비

	createItemStack({ 7, -4, 0 }, { { itemID::fellingAxe,1} }); //벌목도끼

	createItemStack({ -5, 1, 0 }, { { itemID::fishingRod,1} }); //낚시대

	createItemStack({ -3, -4, -1 }, { {itemID::pickaxe,1} });//곡괭이
	createItemStack({ -2, -4, -1 }, { { itemID::minerHelmet,1} });//광부헬멧
	createItemStack({ -1, -4, -1 }, { { itemID::shovel,1} });//삽



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////▼아이템 레시피 추가////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (int i = 1; i < itemDex.size(); i++)
	{
		if (itemDex[i].name != L"?")
		{
			if (itemDex[i].checkFlag(itemFlag::CANCRAFT) && itemDex[i].getSprIndex() != 1)
			{
				ItemPocket::unlockRecipes.get()->addRecipe(i);
			}
		}
	}

	//타일 세팅
	{
		for (int dx = -30; dx <= 30; dx++)
		{
			for (int dy = -30; dy <= 30; dy++)
			{
				setFloor({ dx,dy,0 }, itemID::dirt);
			}
		}
		for (int dx = -30; dx <= 30; dx++)
		{
			for (int dy = -30; dy <= -2; dy++)
			{
				setFloor({ dx,dy,0 }, itemID::grass);
			}
		}


		//집 바닥 타일
		for (int dx = 0; dx < 5; dx++)
		{
			for (int dy = 0; dy < 4; dy++)
			{
				setFloor({ -5 + dx, -5 + dy, 0 }, itemID::tileFloor);
			}
		}
		createProp({ 2, -1, 0 }, itemID::woodenSign);//표지판
		//유리벽 설치
		setWall({ 2,-4,0 }, itemID::glassWall);
		setWall({ 2,-3,0 }, itemID::glassWall);
		setWall({ 2,-2,0 }, itemID::glassWall);
		setWall({ 4,-4,0 }, itemID::glassWall);
		setWall({ 4,-3,0 }, itemID::glassWall);
		//얕은 물 타일(연못)
		setFloor({ -3,0,0 }, itemID::shallowFreshWater);
		setFloor({ -4,0,0 }, itemID::deepFreshWater);
		setFloor({ -3,1,0 }, itemID::shallowFreshWater);
		setFloor({ -4,1,0 }, itemID::deepFreshWater);
		setFloor({ -4,2,0 }, itemID::shallowFreshWater);
		int startX = -33;
		int startY = 36;
		for (int dy = 0; dy <= 30; dy++)
		{
			for (int dx = 0; dx <= 60; dx++)
			{
				itemID::sandFloor;
				if (dy <= 1) setFloor({ startX + dx, startY + dy, 0 }, itemID::shallowSeaWater);//얕은해수
				else if (dy <= 5) setFloor({ startX + dx, startY + dy, 0 }, itemID::sandFloor);//모래
				else if (dy <= 7) setFloor({ startX + dx, startY + dy, 0 }, itemID::shallowSeaWater);//얕은해수
				else setFloor({ startX + dx, startY + dy, 0 }, itemID::deepSeaWater);//깊은해수
			}
		}

		setFloor({ -7,41,0 }, itemID::shallowSeaWater);
		setFloor({ 0,38,0 }, itemID::shallowSeaWater);
		setFloor({ 1,38,0 }, itemID::shallowSeaWater);
		setFloor({ 1,36,0 }, itemID::deepSeaWater);
		setFloor({ 4,41,0 }, itemID::shallowSeaWater);
		setFloor({ 5,41,0 }, itemID::shallowSeaWater);
		setFloor({ 6,41,0 }, itemID::shallowSeaWater);
		setFloor({ 5,43,0 }, itemID::deepSeaWater);
		setFloor({ 6,43,0 }, itemID::deepSeaWater);
		//하단연못
		setFloor({ 5,8,0 }, itemID::shallowFreshWater);
		setFloor({ 5,9,0 }, itemID::shallowFreshWater);
		for (int dx = -3; dx <= 2; dx++)
		{
			setFloor({ 5 + dx,10,0 }, itemID::shallowFreshWater);
			setFloor({ 5 + dx,11,0 }, itemID::shallowFreshWater);
		}

		//오솔길
		setFloor({ -3,-1,0 }, itemID::trail);
		setFloor({ -2,-1,0 }, itemID::trail);
		setFloor({ -2,0,0 }, itemID::trail);
		setFloor({ -1,0,0 }, itemID::trail);
		setFloor({ 0,0,0 }, itemID::trail);
		setFloor({ 1,0,0 }, itemID::trail);
		setFloor({ 2,0,0 }, itemID::trail);
		setFloor({ 2,1,0 }, itemID::trail);
		setFloor({ 3,1,0 }, itemID::trail);
		setFloor({ 4,1,0 }, itemID::trail);
		setFloor({ 4,2,0 }, itemID::trail);
		for (int i = 0; i < 9; i++) setFloor({ 5 + i,2,0 }, itemID::trail);
	}

	createProp({ 10, -8, 0 }, itemID::refrigerator);//냉장고 설치
	ItemPocket* refri = TileProp(10, -8, 0)->leadItem.pocketPtr.get();
	refri->addItemFromDex({ { itemID::revolver, 1 },{ itemID::assaultRifle, 1 },{ itemID::shotgun, 1 },{ itemID::pistol, 1 },{ itemID::smg, 1 },{ itemID::sniperRifle, 1 } });
	createProp({ 9, -8, 0 }, itemID::ammoBox);//탄통 설치
	TileProp(9, -8, 0)->leadItem.pocketPtr->addItemFromDex({ { itemID::magnum357Round,99},{ itemID::nato556Round,99},{ itemID::nato556Tracer,99},{ itemID::nato556AP,99} ,{ itemID::slugRound,99},{ itemID::buckshot,99},{ itemID::dragonsBreath,99},{ itemID::parabellum9mm,99} ,{ itemID::magnum357Rubber,99 },{ itemID::magnum357Blank,99 },{ itemID::magnum357Shotshell,99 },{ itemID::magnum357AP,99 },{ itemID::magnum357HotLoad,99 },{ itemID::nato556Blank,99 },{ itemID::bmg50Ball,30 },{ itemID::bmg50Tracer,30 },{ itemID::bmg50AP,30 },{ itemID::rifleMagazine,1 },{ itemID::rifleDrumMagazine,1 },{ itemID::magazine9mm,1 },{ itemID::magazine9mmExtended,1 },{ itemID::sniperBmg50Magazine,1 } });

	//나무벽 설치
	//집 하단 5타일
	setWall({ -1,-2,0 }, itemID::wasteContainerWall);
	setWall({ -2,-2,0 }, itemID::wasteContainerWall);
	createProp({ -3, -2, 0 }, itemID::woodenDoorH);//집 하단 벽 가운데 수직문
	setWall({ -4,-2,0 }, itemID::wasteContainerWall);
	setWall({ -5,-2,0 }, itemID::wasteContainerWall);
	//집 우측 4타일
	setWall({ -1,-3,0 }, itemID::wasteContainerWall);
	createProp({ -1,-4,0 }, itemID::window);//유리벽 → 창문(프롭). 벽은 깔지 않음
	setWall({ -1,-5,0 }, itemID::wasteContainerWall);
	setWall({ -1,-6,0 }, itemID::wasteContainerWall);
	//집 좌측 4타일
	setWall({ -5,-3,0 }, itemID::wasteContainerWall);
	createProp({ -5,-4,0 }, itemID::window);//유리벽 → 창문(프롭). 벽은 깔지 않음
	setWall({ -5,-5,0 }, itemID::wasteContainerWall);
	setWall({ -5,-6,0 }, itemID::wasteContainerWall);
	//잡 상단 중앙 3타일
	setWall({ -2,-6,0 }, itemID::wasteContainerWall);
	setWall({ -3,-6,0 }, itemID::wasteContainerWall);
	setWall({ -4,-6,0 }, itemID::wasteContainerWall);

	createProp({ -4, -5, 0 }, itemID::bookshelf);//책장
	createProp({ -2, -5, 0 }, itemID::bed);//침대

	createProp({ -4, -3, 0 }, itemID::upwardStairs);//상승계단
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			setFloor({ -3 + dx, -4 + dy, 1 }, itemID::tileFloor);
		}
	}

	createProp({ -2, -3, 0 }, itemID::downwardStairs);//하강계단


	//철조망
	for (int i = 0; i < 17; i++)//상단
	{
		setWall({ -5 + i, -9, 0 }, itemID::wireFence);
	}

	for (int i = 0; i < 20; i++)//우측
	{
		if (-8 + i != 1 && -8 + i != 2 && -8 + i != 3)
		{
			setWall({ 11, -8 + i, 0 }, itemID::wireFence);
		}
	}

	for (int i = 0; i < 17; i++)//상단
	{
		setWall({ 11 - i, 12, 0 }, itemID::wireFence);
	}

	//철조망 우측 입구 전통등 2개
	createProp({ 12, 0, 0 }, itemID::bollardLight);//볼라드등
	createProp({ 12, 4, 0 }, itemID::bollardLight);//볼라드등


	//철조망 아래 선로

	createProp({ -2, 15, 0 }, itemID::railBR);
	for (int i = 0; i < 11; i++)  createProp({ -1 + i, 15, 0 }, itemID::railRL);
	createProp({ 10, 15, 0 }, itemID::railSwitchRTurnLEnter);
	for (int i = 0; i < 7; i++)  createProp({ 10, 16 + i, 0 }, itemID::railTB);
	createProp({ 10, 23, 0 }, itemID::railTL);
	for (int i = 0; i < 6; i++)  createProp({ 9 - i, 23, 0 }, itemID::railRL);
	createProp({ 3, 23, 0 }, itemID::railTR);
	for (int i = 0; i < 3; i++) createProp({ 3, 22 - i, 0 }, itemID::railTB);
	createProp({ 3, 19, 0 }, itemID::railBL);
	for (int i = 0; i < 4; i++) createProp({ 2 - i, 19, 0 }, itemID::railRL);
	createProp({ -2, 19, 0 }, itemID::railTR);
	for (int i = 0; i < 3; i++) createProp({ -2, 18 - i, 0 }, itemID::railTB);

	for (int i = 0; i < 5; i++)  createProp({ 11 + i, 15, 0 }, itemID::railRL);

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
				setWall({ dx, dy, -1 }, itemID::stoneWall);
			}
		}

		createProp({ - 2, - 5, -1 }, itemID::traditionalLamp);//전통등
		{
			int cx = -1;
			int cy = -1;

			for (int i = 1; i < 9; i++) DestroyWall(cx, cy + i, -1);
			createProp({ cx, cy + 1, -1 }, itemID::woodenDoorH);//Y축 통로용 수직문

			for (int i = 1; i < 9; i++) DestroyWall(cx + i, cy, -1);
			createProp({ cx + 1, cy, -1 }, itemID::woodenDoorV);//X축 통로용 수평문

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
			//	createProp(6, 39 - i, -1, itemID::wideRailVLeft);
			//	createProp(7, 39 - i, -1, itemID::wideRailVMid);
			//	createProp(8, 39 - i, -1, itemID::wideRailVRight);
			//	cursorY--;
			//}

			for (int i = 0; i <= 78; i++)
			{
				createProp({ 6, 39 - i, -1 }, itemID::wideRailVLeft);
				createProp({ 7, 39 - i, -1 }, itemID::wideRailVMid);
				createProp({ 8, 39 - i, -1 }, itemID::wideRailVRight);
				cursorY--;
			}


			//지하철 설치
			int vX = 7;
			int vY = -1;
			Vehicle* myTrainPower = World::ins()->createVehicle(vX, vY, -1, itemID::metalFrame);//차량 설치
			{
				myTrainPower->name = L"동력차";
				myTrainPower->vehType = vehFlag::train;
				myTrainPower->isPowerTrain = true;

				///////////////////////차량 기초 프레임//////////////////////////////////////
				myTrainPower->extendPart(vX, vY - 1, itemID::metalFrame);
				myTrainPower->extendPart(vX - 1, vY - 1, itemID::metalFrame);
				myTrainPower->extendPart(vX + 1, vY - 1, itemID::metalFrame);
				myTrainPower->extendPart(vX + 2, vY - 1, itemID::metalFrame);
				myTrainPower->extendPart(vX - 2, vY - 1, itemID::metalFrame);

				myTrainPower->extendPart(vX - 1, vY - 2, itemID::metalFrame);
				myTrainPower->extendPart(vX, vY - 2, itemID::metalFrame);
				myTrainPower->extendPart(vX + 1, vY - 2, itemID::metalFrame);
				myTrainPower->extendPart(vX + 2, vY - 2, itemID::metalFrame);
				myTrainPower->extendPart(vX - 2, vY - 2, itemID::metalFrame);

				myTrainPower->extendPart(vX - 1, vY - 3, itemID::metalFrame);
				myTrainPower->extendPart(vX, vY - 3, itemID::metalFrame);
				myTrainPower->extendPart(vX + 1, vY - 3, itemID::metalFrame);
				myTrainPower->extendPart(vX + 2, vY - 3, itemID::metalFrame);
				myTrainPower->extendPart(vX - 2, vY - 3, itemID::metalFrame);

				myTrainPower->extendPart(vX - 1, vY, itemID::metalFrame);
				myTrainPower->extendPart(vX + 1, vY, itemID::metalFrame);
				myTrainPower->extendPart(vX + 2, vY, itemID::metalFrame);
				myTrainPower->extendPart(vX - 2, vY, itemID::metalFrame);

				for (int i = 1; i < 5; i++)
				{
					myTrainPower->extendPart(vX - 1, vY + i, itemID::metalFrame);
					myTrainPower->extendPart(vX, vY + i, itemID::metalFrame);
					myTrainPower->extendPart(vX + 1, vY + i, itemID::metalFrame);
					myTrainPower->extendPart(vX + 2, vY + i, itemID::metalFrame);
					myTrainPower->extendPart(vX - 2, vY + i, itemID::metalFrame);
				}

				int topLeftX = vX - 2;
				int topLeftY = vY - 3;

				for (int x = topLeftX; x <= topLeftX + 4; x++)
				{
					for (int y = topLeftY; y <= topLeftY + 7; y++)
					{
						if ((x == topLeftX || x == topLeftX + 4) || (y == topLeftY || y == topLeftY + 7))
						{
							if (x == topLeftX + 2 && y == topLeftY + 7)  myTrainPower->addPart(x, y, { itemID::vehicleDoor });
							else if (y == topLeftY + 4)  myTrainPower->addPart(x, y, { itemID::vehicleDoor });
							else if (y == topLeftY) myTrainPower->addPart(x, y, { itemID::vehicleGlass });
							else myTrainPower->addPart(x, y, { itemID::vehicleWall });
						}
						else if ((y == topLeftY + 2))
						{
							if (x == topLeftX + 2) myTrainPower->addPart(x, y, { itemID::vehicleDoor });
							else myTrainPower->addPart(x, y, { itemID::vehicleWall });
						}
						else
						{
							myTrainPower->addPart(x, y, { itemID::vehiclePassage });
						}
					}
				}

				myTrainPower->addPart(topLeftX + 2, topLeftY + 1, { itemID::trainControl });


				myTrainPower->addPart(topLeftX + 1, topLeftY + 3, { itemID::vehicleSeat });
				myTrainPower->addPart(topLeftX + 1, topLeftY + 5, { itemID::vehicleSeat });
				myTrainPower->addPart(topLeftX + 1, topLeftY + 6, { itemID::vehicleSeat });

				myTrainPower->addPart(topLeftX + 3, topLeftY + 3, { itemID::vehicleSeat });
				myTrainPower->addPart(topLeftX + 3, topLeftY + 5, { itemID::vehicleSeat });
				myTrainPower->addPart(topLeftX + 3, topLeftY + 6, { itemID::vehicleSeat });
			}

			//지하철(화물칸) 설치

			Vehicle* myTrain = World::ins()->createVehicle(7, 7, -1, itemID::metalFrame);//차량 설치

			{
				int vX = 7;
				int vY = 7;
				myTrain->vehType = vehFlag::train;

				///////////////////////차량 기초 프레임//////////////////////////////////////
				myTrain->extendPart(vX, vY - 1, itemID::metalFrame);
				myTrain->extendPart(vX - 1, vY - 1, itemID::metalFrame);
				myTrain->extendPart(vX + 1, vY - 1, itemID::metalFrame);
				myTrain->extendPart(vX + 2, vY - 1, itemID::metalFrame);
				myTrain->extendPart(vX - 2, vY - 1, itemID::metalFrame);

				myTrain->extendPart(vX - 1, vY - 2, itemID::metalFrame);
				myTrain->extendPart(vX, vY - 2, itemID::metalFrame);
				myTrain->extendPart(vX + 1, vY - 2, itemID::metalFrame);
				myTrain->extendPart(vX + 2, vY - 2, itemID::metalFrame);
				myTrain->extendPart(vX - 2, vY - 2, itemID::metalFrame);

				myTrain->extendPart(vX - 1, vY - 3, itemID::metalFrame);
				myTrain->extendPart(vX, vY - 3, itemID::metalFrame);
				myTrain->extendPart(vX + 1, vY - 3, itemID::metalFrame);
				myTrain->extendPart(vX + 2, vY - 3, itemID::metalFrame);
				myTrain->extendPart(vX - 2, vY - 3, itemID::metalFrame);

				myTrain->extendPart(vX - 1, vY, itemID::metalFrame);
				myTrain->extendPart(vX + 1, vY, itemID::metalFrame);
				myTrain->extendPart(vX + 2, vY, itemID::metalFrame);
				myTrain->extendPart(vX - 2, vY, itemID::metalFrame);

				for (int i = 1; i < 5; i++)
				{
					myTrain->extendPart(vX - 1, vY + i, itemID::metalFrame);
					myTrain->extendPart(vX, vY + i, itemID::metalFrame);
					myTrain->extendPart(vX + 1, vY + i, itemID::metalFrame);
					myTrain->extendPart(vX + 2, vY + i, itemID::metalFrame);
					myTrain->extendPart(vX - 2, vY + i, itemID::metalFrame);
				}

				int topLeftX = vX - 2;
				int topLeftY = vY - 3;

				for (int x = topLeftX; x <= topLeftX + 4; x++)
				{
					for (int y = topLeftY; y <= topLeftY + 7; y++)
					{
						if ((x == topLeftX || x == topLeftX + 4) || (y == topLeftY || y == topLeftY + 7))
						{
							if (x == topLeftX + 2)  myTrain->addPart(x, y, { itemID::vehicleDoor });
							else if (y == topLeftY + 4)  myTrain->addPart(x, y, { itemID::vehicleDoor });
							else myTrain->addPart(x, y, { itemID::vehicleWall });
						}
						else
						{
							myTrain->addPart(x, y, { itemID::vehiclePassage });
						}
					}
				}

				myTrain->addPart(topLeftX + 1, topLeftY + 1, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 1, topLeftY + 2, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 1, topLeftY + 3, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 1, topLeftY + 5, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 1, topLeftY + 6, { itemID::vehicleSeat });

				myTrain->addPart(topLeftX + 3, topLeftY + 1, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 3, topLeftY + 2, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 3, topLeftY + 3, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 3, topLeftY + 5, { itemID::vehicleSeat });
				myTrain->addPart(topLeftX + 3, topLeftY + 6, { itemID::vehicleSeat });
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

		struct Range { int from, to; };

		auto destroyRect = [](Range x, Range y) {
			for (int xi = x.from; xi <= x.to; xi++)
				for (int yi = y.from; yi <= y.to; yi++)
					DestroyWall(xi, yi, -1);
			};
		auto destroyHLine = [](Range x, int y) {
			for (int xi = x.from; xi <= x.to; xi++) DestroyWall(xi, y, -1);
			};
		auto destroyVLine = [](int x, Range y) {
			for (int yi = y.from; yi <= y.to; yi++) DestroyWall(x, yi, -1);
			};

		// 중앙 큰 공간
		destroyRect({ -15, -8 }, { -5, -1 });
		destroyHLine({ -14, -7 }, 0);
		destroyHLine({ -14, -12 }, 1);

		// 상단 확장
		destroyRect({ -13, -6 }, { -6, -5 });
		destroyHLine({ -10, -5 }, -7);
		destroyRect({ -9, -4 }, { -9, -8 });
		destroyHLine({ -6, -3 }, -10);

		// 우측 하단 돌출부
		DestroyWall(-7, -1, -1);
		DestroyWall(-7, -2, -1);
		DestroyWall(-6, -2, -1);
		destroyHLine({ -7, -4 }, -3);

		// 좌측 통로
		destroyVLine(-16, { -4, -2 });

		destroyHLine({ -7, -5 }, -4);

		createProp({ -4, -3, -1 }, itemID::woodenDoorV);//destroyHLine 통로용 수평문

		//담수호
		{
			auto setDeep = [](int x, int y) { setFloor({ x, y, -1 }, itemID::deepFreshWater); };
			auto setShallow = [](int x, int y) { setFloor({ x, y, -1 }, itemID::shallowFreshWater); };

			auto floorRect = [](int code, Range x, Range y) {
				for (int xi = x.from; xi <= x.to; xi++)
					for (int yi = y.from; yi <= y.to; yi++)
						setFloor({ xi, yi, -1 }, code);
				};
			auto floorHLine = [](int code, Range x, int y) {
				for (int xi = x.from; xi <= x.to; xi++) setFloor({ xi, y, -1 }, code);
				};
			auto floorVLine = [](int code, int x, Range y) {
				for (int yi = y.from; yi <= y.to; yi++) setFloor({ x, yi, -1 }, code);
				};

			floorVLine(itemID::deepFreshWater, -16, { -4,-2 });
			floorVLine(itemID::deepFreshWater, -15, { -5,-1 });
			floorVLine(itemID::deepFreshWater, -14, { -5,1 });
			floorRect(itemID::shallowFreshWater, { -13,-12 }, { -6,1 });
			floorVLine(itemID::shallowFreshWater, -11, { -6,-3 });
			floorVLine(itemID::deepFreshWater, -13, { -6,-2 });
			floorVLine(itemID::shallowFreshWater, -10, { -5,-4 });
			floorVLine(itemID::deepFreshWater, -12, { -6,-4 });
		}


	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////▼농경지대 관련////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	createProp({ -3, -10, 0 }, itemID::verticalElbowRB);//지상으로 나오는 파이프
	createItemStack({ -4, -12, 0 },
		{ {itemID::hoe,1},
		{ itemID::scythe,1},
		{ itemID::rice,20},
		{ itemID::wheat,20},
		{ itemID::potato,20 },
		{ itemID::orange,20 },
		{ itemID::strawHat,1 },
		{ itemID::wateringCan,1},
		{ itemID::tomato, 15},
		{ itemID::watermelon,10},
		{ itemID::carrotSeed,5 },
		{ itemID::carrot,3 },
		{ itemID::cabbage,3 },
		{ itemID::cabbageSeed,12 },
		{ itemID::rawChicken,3 },
		{ itemID::cacaoFruit,3 },
		{ itemID::butter,3 },
		{ itemID::egg,10 },
		{ itemID::scallion, 10},
		{ itemID::onion, 10},
		{ itemID::garlic, 10}
		});








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


	createProp({ 1, -3, 0 }, itemID::spruceTree);//나무 설치
	createProp({ 3, -2, 0 }, itemID::ginkgoTree);//나무 설치
	createProp({ 5, -1, 0 }, itemID::lemonTree);//나무 설치
	createProp({ 0, -5, 0 }, itemID::pineTree);//나무 설치
	createProp({ 7, 1, 0 }, itemID::cherryTree);//나무 설치
	createProp({ 4, -5, 0 }, itemID::peachTree);//나무 설치
	createProp({ 9, -4, 0 }, itemID::cherryTree);//나무 설치
	createProp({ 10, -1, 0 }, itemID::appleTree);//사과나무 설치
	createProp({ -2, 39, 0 }, itemID::palmTree);//야자나무 설치
	createProp({ 0, -20, 0 }, itemID::cherryTree);//나무 설치
	createProp({ -4, 5, 0 }, itemID::orangeTree);//사과나무 설치
	createProp({ 3, 3, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 4, 3, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 5, 3, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 4, 4, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 10, 11, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 10, 10, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 10, 9, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 9, 11, 0 }, itemID::scrapMetalPile);//고철 설치
	createProp({ 9, 10, 0 }, itemID::scrapMetalPile);//고철 설치
	//잔디

	setFloor({ -2, 3, 0 }, itemID::grass);
	setFloor({ -1, 3, 0 }, itemID::grass);
	setFloor({ 0, 3, 0 }, itemID::grass);
	setFloor({ 1, 3, 0 }, itemID::grass);
	setFloor({ -2, 4, 0 }, itemID::grass);
	setFloor({ -1, 4, 0 }, itemID::grass);
	setFloor({ 0, 4, 0 }, itemID::grass);
	setFloor({ 1, 4, 0 }, itemID::grass);
	createProp({ -2, 3, 0 }, itemID::lily);//꽃 설치
	createProp({ -1, 3, 0 }, itemID::chrysanthemum);//꽃 설치
	createProp({ 0, 3, 0 }, itemID::tulip);//꽃 설치
	createProp({ 1, 3, 0 }, itemID::forsythia);//꽃 설치
	createProp({ -2, 4, 0 }, itemID::lavender);//꽃 설치
	createProp({ -1, 4, 0 }, itemID::freesia);//꽃 설치
	createProp({ 0, 4, 0 }, itemID::azalea);//꽃 설치
	createProp({ 1, 4, 0 }, itemID::lily);//꽃 설치
	createProp({ 6, -4, 0 }, itemID::lily);//꽃 설치
	createProp({ 0, -1, 0 }, itemID::traditionalLamp);//전통 등 설치
	createProp({ 4, 0, 0 }, itemID::traditionalLamp);//볼라드 등 설치

	//울타리 설치
	createProp({ -3, 2, 0 }, itemID::woodenFence);
	createProp({ -2, 2, 0 }, itemID::woodenFence);
	createProp({ -1, 2, 0 }, itemID::woodenFence);
	createProp({ 0, 2, 0 }, itemID::woodenFence);
	createProp({ 1, 2, 0 }, itemID::woodenFence);
	createProp({ 2, 2, 0 }, itemID::woodenFence);
	createProp({ 3, 2, 0 }, itemID::woodenFence);
	createProp({ -3, 3, 0 }, itemID::woodenFence);
	createProp({ -3, 4, 0 }, itemID::woodenFence);
	createProp({ 2, 3, 0 }, itemID::woodenFence);
	createProp({ 2, 4, 0 }, itemID::woodenFence);
	createProp({ -3, 5, 0 }, itemID::woodenFence);
	createProp({ -2, 5, 0 }, itemID::woodenFence);
	createProp({ -1, 5, 0 }, itemID::woodenFence);
	createProp({ 0, 5, 0 }, itemID::woodenFence);
	createProp({ 1, 5, 0 }, itemID::woodenFence);
	createProp({ 2, 5, 0 }, itemID::woodenFence);



	//배관 설치
	createProp({ 3, 6, 0 }, itemID::pipe);
	createProp({ 4, 6, 0 }, itemID::pipe);
	createProp({ 5, 6, 0 }, itemID::pipe);
	createProp({ 6, 6, 0 }, itemID::pipe);
	createProp({ 5, 7, 0 }, itemID::pipe);
	createProp({ 5, 8, 0 }, itemID::intakePipeU);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	int vX = -9;
	int vY = +3;
	{
		Vehicle* myCar = World::ins()->createVehicle(vX, vY, 0, itemID::metalFrame);
		myCar->name = L"SUV";
		myCar->vehType = vehFlag::car;

		///////////////////////차량 기초 프레임//////////////////////////////////////
		myCar->extendPart(vX, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY + 3, itemID::metalFrame);
		myCar->extendPart(vX, vY + 3, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY + 3, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY + 3, itemID::metalFrame);

		myCar->extendPart(vX - 1, vY - 3, itemID::steelBumper);
		myCar->extendPart(vX, vY - 3, itemID::steelBumper);
		myCar->extendPart(vX + 1, vY - 3, itemID::steelBumper);
		myCar->extendPart(vX + 2, vY - 3, itemID::steelBumper);
		//////////////////////////최상단 4타일////////////////////////////////////
		myCar->addPart(vX - 1, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
		myCar->addPart(vX, vY - 2, { itemID::vehicleWall });
		myCar->addPart(vX + 1, vY - 2, { itemID::vehicleWall });
		myCar->addPart(vX + 2, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
		//////////////////////////중상단 4타일////////////////////////////////////
		myCar->addPart(vX - 1, vY - 1, itemID::vehicleGlass);
		myCar->addPart(vX, vY - 1, { itemID::vehicleGlass, itemID::engineV2Gasoline });
		myCar->addPart(vX + 1, vY - 1, itemID::vehicleGlass);
		myCar->addPart(vX + 2, vY - 1, itemID::vehicleGlass);
		////////////////////////////////운전석 4타일///////////////////////////
		myCar->addPart(vX - 1, vY, { itemID::vehicleDoor });
		myCar->addPart(vX, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleControl, itemID::vehicleRoof });
		myCar->addPart(vX + 1, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
		myCar->addPart(vX + 2, vY, { itemID::vehicleDoor });
		//////////////////////////운전석 아래 통로 4타일/////////////////////////////
		myCar->addPart(vX - 1, vY + 1, { itemID::vehicleWall });
		myCar->addPart(vX, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof });
		myCar->addPart(vX + 1, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof, itemID::vehicleTurret });
		myCar->addPart(vX + 2, vY + 1, { itemID::vehicleWall });
		///////////////////////////////뒷자석 4타일/////////////////////
		myCar->addPart(vX - 1, vY + 2, { itemID::vehicleDoor, itemID::fuelTank10L });
		{
			ItemPocket* partPocket = myCar->partInfo[{vX - 1, vY + 2, myCar->getGridZ()}].get();
			for (int i = 0; i < partPocket->itemInfo.size(); i++)
			{
				if (partPocket->itemInfo[i].itemCode == itemID::fuelTank10L)
				{
					partPocket->itemInfo[i].pocketPtr->addItemFromDex(itemID::gasoline, 900);
				}
			}
		}
		myCar->addPart(vX, vY + 2, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
		myCar->addPart(vX + 1, vY + 2, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
		myCar->addPart(vX + 2, vY + 2, { itemID::vehicleDoor });
		///////////////////////////////최후방 4타일///////////////////////////
		myCar->addPart(vX - 1, vY + 3, { itemID::vehicleWall, itemID::tailLight });
		myCar->addPart(vX, vY + 3, { itemID::trunkDoor });
		myCar->addPart(vX + 1, vY + 3, { itemID::trunkDoor });
		myCar->addPart(vX + 2, vY + 3, { itemID::vehicleWall, itemID::tailLight });
	}



	///////////////////오토바이///////////////////////////////////////////
	Vehicle* myMoto = World::ins()->createVehicle(vX + 6, vY + 5, 0, itemID::metalFrame);
	myMoto->extendPart(vX + 6, vY + 4, itemID::metalFrame);
	myMoto->extendPart(vX + 6, vY + 6, itemID::metalFrame);

	myMoto->addPart(vX + 6, vY + 4, { itemID::tire,itemID::motorcycleWindshield });
	myMoto->addPart(vX + 6, vY + 5, itemID::bicycleSaddle);
	myMoto->addPart(vX + 6, vY + 6, itemID::tire);

	////////////////////자전거////////////////////////////////////////////
	Vehicle* myBike = World::ins()->createVehicle(vX + 9, vY + 4, 0, itemID::metalFrame);
	myBike->extendPart(vX + 9, vY + 3, itemID::metalFrame);
	myBike->extendPart(vX + 9, vY + 5, itemID::metalFrame);

	myBike->addPart(vX + 9, vY + 3, { itemID::tire,itemID::bicycleHandlebar });
	myBike->addPart(vX + 9, vY + 4, { itemID::bicyclePedal, itemID::bicycleSaddle });
	myBike->addPart(vX + 9, vY + 5, { itemID::tire,itemID::shoppingBasket });


	////////////////////////////////////////////////////////////////////////////
	{
		int cx = 15;
		int cy = 0;
		Vehicle* myHeli = World::ins()->createVehicle(cx, cy, 0, itemID::metalFrame);
		myHeli->name = L"헬기";
		myHeli->vehType = vehFlag::heli;
		myHeli->addPart(cx, cy, { itemID::helicopterController });

		myHeli->extendPart(cx, cy - 1, itemID::metalFrame);
		myHeli->extendPart(cx, cy + 1, itemID::metalFrame);
		myHeli->extendPart(cx, cy + 2, itemID::metalFrame);
		myHeli->extendPart(cx, cy + 3, itemID::metalFrame);
		myHeli->extendPart(cx, cy + 4, itemID::metalFrame);
		myHeli->extendPart(cx, cy + 5, itemID::metalFrame);

		myHeli->extendPart(cx + 1, cy - 1, itemID::metalFrame);
		myHeli->extendPart(cx + 1, cy, itemID::metalFrame);
		myHeli->extendPart(cx + 1, cy + 1, itemID::metalFrame);
		myHeli->extendPart(cx + 1, cy + 2, itemID::metalFrame);

		myHeli->extendPart(cx - 1, cy - 1, itemID::metalFrame);
		myHeli->extendPart(cx - 1, cy, itemID::metalFrame);
		myHeli->extendPart(cx - 1, cy + 1, itemID::metalFrame);
		myHeli->extendPart(cx - 1, cy + 2, itemID::metalFrame);

		myHeli->addPart(cx + 1, cy - 1, { itemID::vehicleGlass });
		myHeli->addPart(cx, cy - 1, { itemID::vehicleGlass });
		myHeli->addPart(cx - 1, cy - 1, { itemID::vehicleGlass });

		myHeli->addPart(cx + 1, cy, { itemID::vehicleDoor });
		myHeli->addPart(cx, cy, { itemID::vehiclePassage,itemID::vehicleSeat, itemID::vehicleRoof });
		myHeli->addPart(cx - 1, cy, { itemID::vehicleDoor });

		myHeli->addPart(cx + 1, cy + 1, { itemID::vehicleWall });
		myHeli->addPart(cx, cy + 1, { itemID::vehiclePassage, itemID::vehicleRoof, itemID::helicopterRotor });
		myHeli->addPart(cx - 1, cy + 1, { itemID::vehicleWall });

		myHeli->addPart(cx + 1, cy + 2, { itemID::vehicleWall });
		myHeli->addPart(cx, cy + 2, { itemID::vehicleWall });
		myHeli->addPart(cx - 1, cy + 2, { itemID::vehicleWall });

		myHeli->addPart(cx, cy + 3, { itemID::vehicleWall });
		myHeli->addPart(cx, cy + 4, { itemID::vehicleWall });
		myHeli->addPart(cx, cy + 5, { itemID::vehicleWall, itemID::tailRotor });
	}

	//수레 3종
	Vehicle* cart1 = World::ins()->createVehicle(10, 5, 0, itemID::woodenCart);
	cart1->vehType = vehFlag::car;

	Vehicle* cart2 = World::ins()->createVehicle(8, 5, 0, itemID::foldingWagon);
	cart2->vehType = vehFlag::car;

	Vehicle* cart3 = World::ins()->createVehicle(6, 5, 0, itemID::shoppingCart);
	cart3->vehType = vehFlag::car;

	Vehicle* cart4 = World::ins()->createVehicle(7, -5, 0, itemID::woodenCart);
	cart4->vehType = vehFlag::car;

	//광차
	Vehicle* minecart1 = World::ins()->createVehicle(3, 15, 0, itemID::minecart);
	minecart1->vehType = vehFlag::minecart;
	minecart1->addPart(3, 15, { itemID::minecartController });
	minecart1->bodyDir = dir16::dir0;
	minecart1->isPowerCart = true;

	Vehicle* minecart2 = World::ins()->createVehicle(2, 15, 0, itemID::minecart);
	minecart2->vehType = vehFlag::minecart;
	minecart2->bodyDir = dir16::dir0;

	Vehicle* minecart3 = World::ins()->createVehicle(1, 15, 0, itemID::minecart);
	minecart3->vehType = vehFlag::minecart;
	minecart3->bodyDir = dir16::dir0;

	minecart1->rearCart = minecart2;
	minecart2->rearCart = minecart3;



	///////////////////////////////////////////////////////////////


//타일 세팅 - 섬 모양
//타일 세팅 - 섬 모양
//타일 세팅 - 섬 모양
//타일 세팅 - 섬 모양
	{
		// 더 넓은 범위로 타일 설정
		for (int dx = -60; dx <= 60; dx++)
		{
			for (int dy = -60; dy <= 60; dy++)
			{
				float distance = std::sqrt(dx * dx + dy * dy);

				// 여러 노이즈를 조합해서 더 자연스러운 해안선 만들기
				float noise1 = std::sin(dx * 0.08f) * std::cos(dy * 0.12f) * 4.0f;
				float noise2 = std::sin(dx * 0.15f + dy * 0.1f) * 2.5f;
				float noise3 = std::cos(dx * 0.05f) * std::sin(dy * 0.07f) * 3.0f;
				float totalNoise = noise1 + noise2 + noise3;

				// 타원형 기본 모양 (가로세로 비율 조정)
				float ellipseX = dx / 1.4f;
				float ellipseY = dy / 1.1f;
				float ellipseDistance = std::sqrt(ellipseX * ellipseX + ellipseY * ellipseY);

				// 최종 거리 계산
				float finalDistance = ellipseDistance + totalNoise;

				if (finalDistance <= 30) // 중심부 - 땅/잔디
				{
					if (finalDistance <= 20)
					{
						setFloor({ dx, dy, 0 }, itemID::grass);
					}
					else
					{
						setFloor({ dx, dy, 0 }, itemID::dirt);
					}
				}
				else if (finalDistance <= 38) // 해변 - 모래
				{
					setFloor({ dx, dy, 0 }, itemID::sandFloor);
				}
				else if (finalDistance <= 41) // 얕은 바다 (범위 축소: 38~41, 약 3타일)
				{
					setFloor({ dx, dy, 0 }, itemID::shallowSeaWater);
				}
				else // 깊은 바다
				{
					setFloor({ dx, dy, 0 }, itemID::deepSeaWater);
				}
			}
		}

		// 철조망 내부 지역을 예전처럼 흙으로 다시 설정
		for (int dx = -30; dx <= 30; dx++)
		{
			for (int dy = -30; dy <= 30; dy++)
			{
				// 철조망 내부 범위 (대략적으로)
				if (dx >= -5 && dx <= 11 && dy >= -9 && dy <= 12)
				{
					setFloor({ dx, dy, 0 }, itemID::dirt);
				}
			}
		}

		// 철조망 내부에서 북쪽 부분만 잔디로 (예전처럼)
		for (int dx = -30; dx <= 30; dx++)
		{
			for (int dy = -30; dy <= -2; dy++)
			{
				// 철조망 내부의 북쪽 부분
				if (dx >= -5 && dx <= 11 && dy >= -9 && dy <= -2)
				{
					setFloor({ dx, dy, 0 }, itemID::grass);
				}
			}
		}

		// 수직 도로
		// 기본 4차선 도로
		// 너비 16칸 (중앙선은 dx=-14|-13 이음새에 반쪽 타일로 폭 1타일 형성)
		for (int dx = -6; dx >= -21; dx--)
		{
			for (int dy = -60; dy <= 29; dy++)
			{
				// 횡단보도 위치는 검은 아스팔트로 미리 깔아 노란 중앙선과 섞이지 않게 함
				bool inCrosswalkBand = (dy >= -43 && dy <= -38) || (dy >= -21 && dy <= -16);
				bool dashOn = ((((dy + 24) % 6) + 6) % 6 < 3);
				if (dx == -14 && !inCrosswalkBand && dashOn) setFloor({ dx, dy, 0 }, itemID::yellowAsphaltRightHalf);
				else if (dx == -13 && !inCrosswalkBand && dashOn) setFloor({ dx, dy, 0 }, itemID::yellowAsphaltLeftHalf);
				else setFloor({ dx, dy, 0 }, itemID::blackAsphalt);
			}
		}

		// 수평 도로
		// 기본 4차선 도로
		// 너비 16칸 (중앙선은 dy=-30|-29 이음새에 반쪽 타일로 폭 1타일 형성)
		for (int dy = -37; dy <= -22; dy++)
		{
			for (int dx = -32; dx <= 32; dx++)
			{
				bool inIntersection = (dx >= -21 && dx <= -6);
				// 횡단보도 위치는 검은 아스팔트로 미리 깔아 노란 중앙선과 섞이지 않게 함
				bool inCrosswalkBand = (dx >= -27 && dx <= -22) || (dx >= -5 && dx <= 0);
				bool dashOn = false;
				if (!inIntersection && !inCrosswalkBand)
				{
					if (dx < -21)     dashOn = (((-19 - dx) % 6 + 6) % 6) < 3;
					else dashOn = (((dx + 8) % 6 + 6) % 6) < 3;
				}
				if (dashOn && dy == -30) setFloor({ dx, dy, 0 }, itemID::yellowAsphaltBottomHalf);
				else if (dashOn && dy == -29) setFloor({ dx, dy, 0 }, itemID::yellowAsphaltTopHalf);
				else setFloor({ dx, dy, 0 }, itemID::blackAsphalt);
			}
		}

		{
			//남쪽 횡단보도 (우측보행: 안쪽 띠 서쪽방향, 바깥쪽 띠 동쪽방향)
			//   16타일 도로 = 화살표 1 + stripe 7쌍(14) + lone 1 타일
			setFloor({ -6, -19, 0 }, itemID::whiteAsphaltArrowL);
			setFloor({ -6, -21, 0 }, itemID::whiteAsphaltArrowL);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -8 - 2 * i, -19, 0 }, itemID::whiteAsphalt);
				setFloor({ -8 - 2 * i, -20, 0 }, itemID::whiteAsphalt);
				setFloor({ -8 - 2 * i, -21, 0 }, itemID::whiteAsphalt);
			}



			setFloor({ -21, -16, 0 }, itemID::whiteAsphaltArrowR);
			setFloor({ -21, -18, 0 }, itemID::whiteAsphaltArrowR);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -19 + 2*i, -16, 0 }, itemID::whiteAsphalt);
				setFloor({ -19 + 2 * i, -17, 0 }, itemID::whiteAsphalt);
				setFloor({ -19 + 2 * i, -18, 0 }, itemID::whiteAsphalt);
			}


			//북쪽 횡단보도 (도로가 1타일 북쪽 확장됨에 따라 dy도 1북상)
			setFloor({ -21, -38, 0 }, itemID::whiteAsphaltArrowR);
			setFloor({ -21, -40, 0 }, itemID::whiteAsphaltArrowR);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -19 + 2 * i, -38, 0 }, itemID::whiteAsphalt);
				setFloor({ -19 + 2 * i, -39, 0 }, itemID::whiteAsphalt);
				setFloor({ -19 + 2 * i, -40, 0 }, itemID::whiteAsphalt);
			}

			setFloor({ -6, -41, 0 }, itemID::whiteAsphaltArrowL);
			setFloor({ -6, -43, 0 }, itemID::whiteAsphaltArrowL);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -8 - 2 * i, -41, 0 }, itemID::whiteAsphalt);
				setFloor({ -8 - 2 * i, -42, 0 }, itemID::whiteAsphalt);
				setFloor({ -8 - 2 * i, -43, 0 }, itemID::whiteAsphalt);
			}

			//동쪽 횡단보도 (도로 북쪽 끝 dy=-37로 확장됨에 따라 화살표 위치 1북상)
			setFloor({ -5, -37, 0 }, itemID::whiteAsphaltArrowD);
			setFloor({ -3, -37, 0 }, itemID::whiteAsphaltArrowD);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -5, -35 + 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -4, -35 + 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -3, -35 + 2 * i, 0 }, itemID::whiteAsphalt);
			}

			setFloor({ -2, -22, 0 }, itemID::whiteAsphaltArrowU);
			setFloor({ 0, -22, 0 }, itemID::whiteAsphaltArrowU);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -2, -24 - 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -1, -24 - 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ 0, -24 - 2 * i, 0 }, itemID::whiteAsphalt);
			}

			//서쪽 횡단보도 (우측보행: 안쪽 띠 북쪽방향, 바깥쪽 띠 남쪽방향. 도로 서쪽 확장으로 dx 1서쪽 이동)
			setFloor({ -22, -22, 0 }, itemID::whiteAsphaltArrowU);
			setFloor({ -24, -22, 0 }, itemID::whiteAsphaltArrowU);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -22, -24 - 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -23, -24 - 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -24, -24 - 2 * i, 0 }, itemID::whiteAsphalt);
			}

			setFloor({ -25, -37, 0 }, itemID::whiteAsphaltArrowD);
			setFloor({ -27, -37, 0 }, itemID::whiteAsphaltArrowD);

			for (int i = 0; i < 7; i++)
			{
				setFloor({ -25, -35 + 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -26, -35 + 2 * i, 0 }, itemID::whiteAsphalt);
				setFloor({ -27, -35 + 2 * i, 0 }, itemID::whiteAsphalt);
			}

			// 1. [대각선 횡단보도] 좌하단에서 중앙으로 (교차로 NW 모서리 (-1,-1) 이동)
			for (int i = 0; i < 5; i++)
			{
				int pivotX = -21 + i;
				int pivotY = -34 + i;
				setFloor({ pivotX, pivotY, 0 }, itemID::whiteAsphaltTopQuarter);
				setFloor({ pivotX, pivotY - 1, 0 }, itemID::whiteAsphaltDiagLR);
				setFloor({ pivotX + 1, pivotY - 1, 0 }, itemID::whiteAsphaltDiagUL);
				setFloor({ pivotX + 1, pivotY - 2, 0 }, itemID::whiteAsphaltDiagLR);
				setFloor({ pivotX + 2, pivotY - 2, 0 }, itemID::whiteAsphaltDiagUL);
				setFloor({ pivotX + 2, pivotY - 3, 0 }, itemID::whiteAsphaltDiagLR);
				setFloor({ pivotX + 3, pivotY - 3, 0 }, itemID::whiteAsphaltLeftQuarter);
			}

			// 2. [대각선 횡단보도] 우상단에서 중앙으로 (SE 모서리 변동 없음)
			for (int i = 0; i < 5; i++)
			{
				int pivotX = -6 - i;
				int pivotY = -25 - i;

				setFloor({ pivotX, pivotY, 0 }, itemID::whiteAsphaltBottomQuarter);
				setFloor({ pivotX, pivotY + 1, 0 }, itemID::whiteAsphaltDiagUL);
				setFloor({ pivotX - 1, pivotY + 1, 0 }, itemID::whiteAsphaltDiagLR);
				setFloor({ pivotX - 1, pivotY + 2, 0 }, itemID::whiteAsphaltDiagUL);
				setFloor({ pivotX - 2, pivotY + 2, 0 }, itemID::whiteAsphaltDiagLR);
				setFloor({ pivotX - 2, pivotY + 3, 0 }, itemID::whiteAsphaltDiagUL);
				setFloor({ pivotX - 3, pivotY + 3, 0 }, itemID::whiteAsphaltRightQuarter);
			}

			// 3. [대각선 횡단보도] 좌상단에서 중앙으로 (SW 모서리 (-1,0) 이동)
			for (int i = 0; i < 5; i++)
			{
				int pivotX = -21 + i;
				int pivotY = -25 - i;
				setFloor({ pivotX, pivotY, 0 }, itemID::whiteAsphaltBottomQuarter);
				setFloor({ pivotX, pivotY + 1, 0 }, itemID::whiteAsphaltDiagUR);
				setFloor({ pivotX + 1, pivotY + 1, 0 }, itemID::whiteAsphaltDiagLL);
				setFloor({ pivotX + 1, pivotY + 2, 0 }, itemID::whiteAsphaltDiagUR);
				setFloor({ pivotX + 2, pivotY + 2, 0 }, itemID::whiteAsphaltDiagLL);
				setFloor({ pivotX + 2, pivotY + 3, 0 }, itemID::whiteAsphaltDiagUR);
				setFloor({ pivotX + 3, pivotY + 3, 0 }, itemID::whiteAsphaltLeftQuarter);
			}

			// 4. [대각선 횡단보도] 우하단에서 중앙으로 (NE 모서리 (0,-1) 이동)
			for (int i = 0; i < 5; i++)
			{
				int pivotX = -6 - i;
				int pivotY = -34 + i;

				setFloor({ pivotX, pivotY, 0 }, itemID::whiteAsphaltTopQuarter);
				setFloor({ pivotX, pivotY - 1, 0 }, itemID::whiteAsphaltDiagLL);
				setFloor({ pivotX - 1, pivotY - 1, 0 }, itemID::whiteAsphaltDiagUR);
				setFloor({ pivotX - 1, pivotY - 2, 0 }, itemID::whiteAsphaltDiagLL);
				setFloor({ pivotX - 2, pivotY - 2, 0 }, itemID::whiteAsphaltDiagUR);
				setFloor({ pivotX - 2, pivotY - 3, 0 }, itemID::whiteAsphaltDiagLL);
				setFloor({ pivotX - 3, pivotY - 3, 0 }, itemID::whiteAsphaltRightQuarter);
			}


		}

		// 섬 반경 18 ≥ 다리 폭 24/2 → 다리 끝 24타일 전체가 섬 안에 들어옴
		for (int dx = -33; dx <= 7; dx++)
		{
			for (int dy = 53; dy <= 93; dy++)
			{
				float ldx = dx + 13.0f;
				float ldy = dy - 73.0f;
				float dist = std::sqrt(ldx * ldx + ldy * ldy);
				if (dist <= 18)
				{
					if (dist <= 13) setFloor({ dx, dy, 0 }, itemID::grass);
					else setFloor({ dx, dy, 0 }, itemID::dirt);
				}
			}
		}

		// 도로 16타일 + 양쪽 paver 인도 4타일(가드레일 1 + 보행 3) = 폭 24타일
		for (int dy = 29; dy <= 60; dy++)
		{
			for (int dx = -25; dx <= -2; dx++)
			{
				if (dx <= -22 || dx >= -5) setFloor({ dx, dy, 1 }, itemID::paver);
				else setFloor({ dx, dy, 1 }, itemID::blackAsphalt);
			}
		}

		for (int dx = -25; dx <= -2; dx++)
		{
			createProp({ dx, 29, 0 }, itemID::rampUp);   // 도로 → 다리
			createProp({ dx, 29, 1 }, itemID::rampDown); // 다리 → 도로
			createProp({ dx, 60, 1 }, itemID::rampDown); // 다리 → 섬
			createProp({ dx, 60, 0 }, itemID::rampUp);   // 섬 → 다리
		}

		// 다리 가장자리 가드레일 (paver 바깥쪽 위, ramp 위치는 비워서 차량 진입 가능)
		for (int dy = 29; dy <= 60; dy++)
		{
			setWall({ -25, dy, 1 }, itemID::guardrail);
			setWall({ -2, dy, 1 }, itemID::guardrail);
		}

		for (int dx = -25; dx <= -2; dx++)
		{
			setWall({ dx, 30, 0 }, itemID::pillarWall); // 북쪽 ramp 뒤
			setWall({ dx, 59, 0 }, itemID::pillarWall); // 남쪽 ramp 뒤
		}

		// 양쪽 paver만 깔아서 가운데(진입 차선 / 섬 가장자리)는 그대로 보이게
		for (int dy = 25; dy <= 29; dy++)
		{
			setFloor({ -25, dy, 0 }, itemID::paver);
			setFloor({ -24, dy, 0 }, itemID::paver);
			setFloor({ -23, dy, 0 }, itemID::paver);
			setFloor({ -22, dy, 0 }, itemID::paver);
			setFloor({ -5, dy, 0 }, itemID::paver);
			setFloor({ -4, dy, 0 }, itemID::paver);
			setFloor({ -3, dy, 0 }, itemID::paver);
			setFloor({ -2, dy, 0 }, itemID::paver);
		}
		// 출구측은 섬 위라 가운데도 blackAsphalt로 채워줘야 다리 모양 (진입측은 기존 도로가 가운데 채움)
		for (int dy = 60; dy <= 64; dy++)
		{
			for (int dx = -25; dx <= -2; dx++)
			{
				if (dx <= -22 || dx >= -5) setFloor({ dx, dy, 0 }, itemID::paver);
				else setFloor({ dx, dy, 0 }, itemID::blackAsphalt);
			}
		}
		for (int dy = 25; dy <= 29; dy++)
		{
			setWall({ -25, dy, 0 }, itemID::guardrail);
			setWall({ -2, dy, 0 }, itemID::guardrail);
		}
		for (int dy = 60; dy <= 64; dy++)
		{
			setWall({ -25, dy, 0 }, itemID::guardrail);
			setWall({ -2, dy, 0 }, itemID::guardrail);
		}




		// 집 바닥 타일 (기존 유지)
		for (int dx = 0; dx < 5; dx++)
		{
			for (int dy = 0; dy < 4; dy++)
			{
				setFloor({ -5 + dx, -5 + dy, 0 }, itemID::tileFloor);
			}
		}

		// 레일 구간을 확실히 땅으로 설정 (더 부드럽게)
		for (int x = -5; x <= 20; x++)
		{
			for (int y = 12; y <= 26; y++)
			{
				float distFromRail = std::abs(y - 19) + std::abs(x - 7.5f) * 0.5f;
				if (distFromRail <= 8)
				{
					setFloor({ x, y, 0 }, itemID::dirt);
				}
			}
		}

		// 기존 연못 유지
		setFloor({ -3, 0, 0 }, itemID::shallowFreshWater);
		setFloor({ -4, 0, 0 }, itemID::deepFreshWater);
		setFloor({ -3, 1, 0 }, itemID::shallowFreshWater);
		setFloor({ -4, 1, 0 }, itemID::deepFreshWater);
		setFloor({ -4, 2, 0 }, itemID::shallowFreshWater);

		// 하단연못 유지
		setFloor({ 5, 8, 0 }, itemID::shallowFreshWater);
		setFloor({ 5, 9, 0 }, itemID::shallowFreshWater);
		for (int dx = -3; dx <= 2; dx++)
		{
			setFloor({ 5 + dx, 10, 0 }, itemID::shallowFreshWater);
			setFloor({ 5 + dx, 11, 0 }, itemID::shallowFreshWater);
		}

		// 오솔길 유지
		setFloor({ -3, -1, 0 }, itemID::trail);
		setFloor({ -2, -1, 0 }, itemID::trail);
		setFloor({ -2, 0, 0 }, itemID::trail);
		setFloor({ -1, 0, 0 }, itemID::trail);
		setFloor({ 0, 0, 0 }, itemID::trail);
		setFloor({ 1, 0, 0 }, itemID::trail);
		setFloor({ 2, 0, 0 }, itemID::trail);
		setFloor({ 2, 1, 0 }, itemID::trail);
		setFloor({ 3, 1, 0 }, itemID::trail);
		setFloor({ 4, 1, 0 }, itemID::trail);
		setFloor({ 4, 2, 0 }, itemID::trail);
		for (int i = 0; i < 9; i++) setFloor({ 5 + i, 2, 0 }, itemID::trail);
	}

	//상단 연못 및 농업펌프
	setFloor({ -1,-16,0 }, itemID::shallowFreshWater);
	setFloor({ -1,-15,0 }, itemID::deepFreshWater);
	setFloor({ -1,-14,0 }, itemID::deepFreshWater);
	setFloor({ -1,-13,0 }, itemID::shallowFreshWater);

	setFloor({ -2,-15,0 }, itemID::shallowFreshWater);
	setFloor({ -2,-14,0 }, itemID::shallowFreshWater);

	setFloor({ 0,-15,0 }, itemID::shallowFreshWater);
	setFloor({ 0,-14,0 }, itemID::shallowFreshWater);

	createProp({ 0,-14,0 }, itemID::intakePipeR);
	createProp({ 1,-14,0 }, itemID::pumpR);
	createProp({ 2,-14,0 }, itemID::valveRL);
	createProp({ 3,-14,0 }, itemID::pipe);

	createProp({ 1,-16,0 }, itemID::gasolineGeneratorB);
	createProp({ 1,-15,0 }, itemID::copperCable);

	TileProp({ 1,-16,0 })->leadItem.pocketPtr->addItemFromDex(itemID::gasoline, 1000);


	//요리도구
	createItemStack({ -3, -12, 0 }, { {itemID::cookingPot,1},{itemID::fryingPan,1} });
	createItemStack({ -2, -12, 0 }, { {itemID::woodenPlate,1},{itemID::ceramicPlate,1} });

		
	//테크 도구들
	createProp({ 15,-8,0 }, itemID::autodoc);

	createProp({ 13,-10,0 }, itemID::gasolineGeneratorB);
	TileProp({ 13,-10,0 })->leadItem.pocketPtr->addItemFromDex(itemID::gasoline, 1000);
	createProp({ 13,-9,0 }, itemID::leverUD);
	createProp({ 13,-8,0 }, itemID::copperCable);
	createProp({ 14,-8,0 }, itemID::copperCable);



	createItemStack({ 16, -8, 0 }, {
		{itemID::cbm_nervedrive,1},
		{itemID::cbm_powerStorage,10},
		{itemID::cbm_metabExchange,1},
		{itemID::mutagen,20},
		{itemID::dyeAmpule,5}});




	PlayerPtr->updateVision(PlayerInfo().eyeSight);

};