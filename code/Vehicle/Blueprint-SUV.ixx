module;
export module Blueprint:SUV;

import std;
import :base;
import constVar;
import util;
import Vehicle;
import ItemPocket;
import ItemData;

//SUV — startArea에서 추출한 표준 차량. 4×7 footprint.
//
//   build() 인자명을 myCar로 둔 이유: 원본 코드의 변수명을 그대로 두면 라인 단위
//   diff를 최소화할 수 있어 향후 부품 조정도 추적이 쉽다.

export class SUVBlueprint final : public Blueprint
{
public:
    int leadItem() const override { return itemID::metalFrame; }
    vehFlag type() const override { return vehFlag::car; }
    std::wstring name() const override { return L"SUV"; }

    void build(Vehicle* myCar, Point3 anchor) const override
    {
        const int vX = anchor.x;
        const int vY = anchor.y;

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
        //////////////////////////▼최상단 4타일////////////////////////////////////
        myCar->addPart(vX - 1, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
        myCar->addPart(vX, vY - 2, { itemID::vehicleWall });
        myCar->addPart(vX + 1, vY - 2, { itemID::vehicleWall });
        myCar->addPart(vX + 2, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
        //////////////////////////▼중상단 4타일////////////////////////////////////
        myCar->addPart(vX - 1, vY - 1, itemID::vehicleGlass);
        myCar->addPart(vX, vY - 1, { itemID::vehicleGlass, itemID::engineV2Gasoline });
        myCar->addPart(vX + 1, vY - 1, itemID::vehicleGlass);
        myCar->addPart(vX + 2, vY - 1, itemID::vehicleGlass);
        ////////////////////////////////▼운전석 4타일///////////////////////////////
        myCar->addPart(vX - 1, vY, { itemID::vehicleDoor });
        myCar->addPart(vX, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleControl, itemID::vehicleRoof });
        myCar->addPart(vX + 1, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
        myCar->addPart(vX + 2, vY, { itemID::vehicleDoor });
        //////////////////////////▼운전석 아래 통로 4타일/////////////////////////////
        myCar->addPart(vX - 1, vY + 1, { itemID::vehicleWall });
        myCar->addPart(vX, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof });
        myCar->addPart(vX + 1, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof, itemID::vehicleTurret });
        myCar->addPart(vX + 2, vY + 1, { itemID::vehicleWall });
        ///////////////////////////////▼뒷자석 4타일/////////////////////
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
        ///////////////////////////////▼최후방 4타일///////////////////////////
        myCar->addPart(vX - 1, vY + 3, { itemID::vehicleWall, itemID::tailLight });
        myCar->addPart(vX, vY + 3, { itemID::trunkDoor });
        myCar->addPart(vX + 1, vY + 3, { itemID::trunkDoor });
        myCar->addPart(vX + 2, vY + 3, { itemID::vehicleWall, itemID::tailLight });
    }
};

export inline const SUVBlueprint suvBlueprint;
