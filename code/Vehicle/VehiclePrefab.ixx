module;
export module VehiclePrefab;

import std;
import util;
import constVar;
import VehiclePlan;

//재사용 차량 prefab — VehicleBuilder에 footprint(부품)를 채우는 자유함수들.
//  코어(leadItem/type/name)는 builder 생성 시점(addVehicle/VehicleBuilder 생성자)에 지정하고,
//  helper는 부품 레이아웃만 깐다. 좌표 인자 (x,y)는 anchor(=Lot-local) 좌표.

//SUV — 4x7 footprint. (metalFrame, car, "SUV")로 생성된 builder를 기대.
export void buildSUV(VehicleBuilder& v, int x, int y)
{
    const int vX = x;
    const int vY = y;

    ///////////////////////차량 기초 프레임//////////////////////////////////////
    v.extendPart(vX, vY - 1, itemID::metalFrame);
    v.extendPart(vX - 1, vY - 1, itemID::metalFrame);
    v.extendPart(vX + 1, vY - 1, itemID::metalFrame);
    v.extendPart(vX + 2, vY - 1, itemID::metalFrame);
    v.extendPart(vX - 1, vY - 2, itemID::metalFrame);
    v.extendPart(vX, vY - 2, itemID::metalFrame);
    v.extendPart(vX + 1, vY - 2, itemID::metalFrame);
    v.extendPart(vX + 2, vY - 2, itemID::metalFrame);
    v.extendPart(vX - 1, vY, itemID::metalFrame);
    v.extendPart(vX + 1, vY, itemID::metalFrame);
    v.extendPart(vX + 2, vY, itemID::metalFrame);
    v.extendPart(vX - 1, vY + 1, itemID::metalFrame);
    v.extendPart(vX, vY + 1, itemID::metalFrame);
    v.extendPart(vX + 1, vY + 1, itemID::metalFrame);
    v.extendPart(vX + 2, vY + 1, itemID::metalFrame);
    v.extendPart(vX - 1, vY + 2, itemID::metalFrame);
    v.extendPart(vX, vY + 2, itemID::metalFrame);
    v.extendPart(vX + 1, vY + 2, itemID::metalFrame);
    v.extendPart(vX + 2, vY + 2, itemID::metalFrame);
    v.extendPart(vX - 1, vY + 3, itemID::metalFrame);
    v.extendPart(vX, vY + 3, itemID::metalFrame);
    v.extendPart(vX + 1, vY + 3, itemID::metalFrame);
    v.extendPart(vX + 2, vY + 3, itemID::metalFrame);

    v.extendPart(vX - 1, vY - 3, itemID::steelBumper);
    v.extendPart(vX, vY - 3, itemID::steelBumper);
    v.extendPart(vX + 1, vY - 3, itemID::steelBumper);
    v.extendPart(vX + 2, vY - 3, itemID::steelBumper);
    //////////////////////////최상단 4타일////////////////////////////////////
    v.addPart(vX - 1, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
    v.addPart(vX, vY - 2, { itemID::vehicleWall });
    v.addPart(vX + 1, vY - 2, { itemID::vehicleWall });
    v.addPart(vX + 2, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
    //////////////////////////중상단 4타일////////////////////////////////////
    v.addPart(vX - 1, vY - 1, itemID::vehicleGlass);
    v.addPart(vX, vY - 1, { itemID::vehicleGlass, itemID::engineV2Gasoline });
    v.addPart(vX + 1, vY - 1, itemID::vehicleGlass);
    v.addPart(vX + 2, vY - 1, itemID::vehicleGlass);
    ////////////////////////////////운전석 4타일///////////////////////////
    v.addPart(vX - 1, vY, { itemID::vehicleDoor });
    v.addPart(vX, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleControl, itemID::vehicleRoof });
    v.addPart(vX + 1, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
    v.addPart(vX + 2, vY, { itemID::vehicleDoor });
    //////////////////////////운전석 아래 통로 4타일/////////////////////////////
    v.addPart(vX - 1, vY + 1, { itemID::vehicleWall });
    v.addPart(vX, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof });
    v.addPart(vX + 1, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof, itemID::vehicleTurret });
    v.addPart(vX + 2, vY + 1, { itemID::vehicleWall });
    ///////////////////////////////뒷자석 4타일/////////////////////
    v.addPart(vX - 1, vY + 2, { itemID::vehicleDoor, itemID::fuelTank10L });
    v.addCargo(vX - 1, vY + 2, itemID::fuelTank10L, itemID::gasoline, 900);
    v.addPart(vX, vY + 2, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
    v.addPart(vX + 1, vY + 2, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
    v.addPart(vX + 2, vY + 2, { itemID::vehicleDoor });
    ///////////////////////////////최후방 4타일///////////////////////////
    v.addPart(vX - 1, vY + 3, { itemID::vehicleWall, itemID::tailLight });
    v.addPart(vX, vY + 3, { itemID::trunkDoor });
    v.addPart(vX + 1, vY + 3, { itemID::trunkDoor });
    v.addPart(vX + 2, vY + 3, { itemID::vehicleWall, itemID::tailLight });

    v.setDir(dir16::dir2);
}
