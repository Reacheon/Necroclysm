#include <SDL3/SDL.h>

module World;

import std;
import util;
import globalVar;
import constVar;
import ItemData;
import ItemPocket;
import ItemStack;
import Prop;
import Vehicle;
import Entity;
import Monster;
import log;
import globalTime;

unsigned __int16 TileFloor(int x, int y, int z) { return World::ins()->getTile(x, y, z).floor; }

unsigned __int16 TileFloor(Point3 coord) { return World::ins()->getTile(coord.x, coord.y, coord.z).floor; }

bool TileSnow(int x, int y, int z) { return World::ins()->getTile(x, y, z).hasSnow; }

unsigned __int16 TileWall(int x, int y, int z) { return World::ins()->getTile(x, y, z).wall; }

bool ExistWall(int x, int y, int z) { return (World::ins()->getTile(x, y, z).wall != 0); }

void setWall(Point3 coord, int val)
{
    World::ins()->getTile(coord.x, coord.y, coord.z).setWall(val);
}

void setFloor(Point3 coord, int val)
{
    World::ins()->getTile(coord.x, coord.y, coord.z).setFloor(val);
}

Entity* TileEntity(int x, int y, int z) { return (Entity*&)World::ins()->getTile(x, y, z).EntityPtr; }

void EntityPtrMove(Point3 startCoor, Point3 endCoor)
{
    // 1단계: 시작 좌표의 EntityPtr 존재 확인
    auto& startTile = World::ins()->getTile(startCoor);
    errorBox(startTile.EntityPtr == nullptr,
        L"EntityPtrMove: 시작 좌표에 EntityPtr이 없습니다. startCoor=(" +
        std::to_wstring(startCoor.x) + L"," + std::to_wstring(startCoor.y) + L"," + std::to_wstring(startCoor.z) + L")");

    // 2단계: 목적지 좌표의 EntityPtr이 비어있는지 확인
    auto& endTile = World::ins()->getTile(endCoor);
    errorBox(endTile.EntityPtr != nullptr,
        L"EntityPtrMove: 목적지에 이미 EntityPtr이 있습니다. endCoor=(" +
        std::to_wstring(endCoor.x) + L"," + std::to_wstring(endCoor.y) + L"," + std::to_wstring(endCoor.z) + L")");

    // 3단계: 이동 실행
    endTile.EntityPtr = std::move(startTile.EntityPtr);

    // 4단계: 이동 후 EntityPtr 존재 확인
    errorBox(endTile.EntityPtr == nullptr,
        L"EntityPtrMove: 이동 후 EntityPtr이 nullptr입니다. endCoor=(" +
        std::to_wstring(endCoor.x) + L"," + std::to_wstring(endCoor.y) + L"," + std::to_wstring(endCoor.z) + L")");

    // 5단계: setGrid 호출
    endTile.EntityPtr->setGrid(endCoor.x, endCoor.y, endCoor.z);
    endTile.EntityPtr->pullEquipLights();
}

void EntityPtrMove(std::unique_ptr<Entity> inputPtr, Point3 endCoor)
{
    World::ins()->getTile(endCoor).EntityPtr = std::move(inputPtr);
    World::ins()->getTile(endCoor).EntityPtr->setGrid(endCoor.x, endCoor.y, endCoor.z);
    World::ins()->getTile(endCoor).EntityPtr->pullEquipLights();
}

Prop* TileProp(int x, int y, int z) { return World::ins()->getTile(x, y, z).PropPtr.get(); }
Prop* TileProp(Point3 pt) { return World::ins()->getTile(pt.x, pt.y, pt.z).PropPtr.get(); }
Vehicle*& TileVehicle(int x, int y, int z) { return World::ins()->getTile(x, y, z).VehiclePtr; }
Vehicle*& TileVehicle(Point3 pt) { return World::ins()->getTile(pt.x, pt.y, pt.z).VehiclePtr; }

ItemStack* TileItemStack(int x, int y, int z) { return World::ins()->getTile(x, y, z).ItemStackPtr.get(); }
ItemStack* TileItemStack(Point3 pt) { return World::ins()->getTile(pt.x, pt.y, pt.z).ItemStackPtr.get(); }

fovFlag& TileFov(int x, int y, int z) { return static_cast<fovFlag&>(World::ins()->getTile(x, y, z).fov); }

void createMonster(Point3 inputCoor, int inputEntityCode)
{
    World::ins()->getTile(inputCoor).EntityPtr = std::make_unique<Monster>(inputEntityCode, inputCoor.x, inputCoor.y, inputCoor.z);
}

void createItemStack(Point3 inputCoor)
{
    World::ins()->getTile(inputCoor).ItemStackPtr = std::make_unique<ItemStack>(inputCoor);
    //addStack()은 ItemStack 생성자 내부의 setGrid()에서 자동 호출됨
}

void createItemStack(Point3 inputCoor, std::vector<std::pair<int, int>> inputItems)
{
    World::ins()->getTile(inputCoor).ItemStackPtr = std::make_unique<ItemStack>(inputCoor, inputItems);
    //addStack()은 ItemStack 생성자 내부의 setGrid()에서 자동 호출됨
}

void destroyItemStack(Point3 inputCoor)
{
    //eraseStack()은 ItemStack 소멸자에서 자동 호출됨
    World::ins()->getTile(inputCoor).ItemStackPtr.reset();
}

void destroyProp(Point3 inputCoor)
{
    //eraseProp()은 Prop 소멸자에서 자동 호출됨
    World::ins()->getTile(inputCoor).PropPtr.reset();
}

void createProp(Point3 inputCoor, int inputItemCode)
{
    World::ins()->getTile(inputCoor).PropPtr = std::make_unique<Prop>(inputCoor, inputItemCode);
    //addProp()은 Prop 생성자 내부의 setGrid()에서 자동 호출됨

    World::ins()->getTile(inputCoor).PropPtr->updateSprIndex();

    //주변 타일을 분석해 extraIndex(연결) 설정
    int dx = 0;
    int dy = 0;
    for (int i = 0; i < 8; i++)
    {
        dir2Coord(i, dx, dy);
        Prop* targetProp = TileProp(inputCoor.x + dx, inputCoor.y + dy, inputCoor.z);
        if (targetProp != nullptr) targetProp->updateSprIndex();
    }
}

void createFlame(Point3 inputCoor, flameFlag inputFlag) { World::ins()->getTile(inputCoor).flamePtr = std::make_unique<Flame>(inputCoor, inputFlag); }

void DestroyWall(int x, int y, int z) { World::ins()->getTile(x, y, z).destoryWall(); }

bool isWalkable(Point3 coord)
{
    if (TileWall(coord.x, coord.y, coord.z) != 0) return false;
    else if (TileProp(coord.x, coord.y, coord.z) != nullptr && TileProp(coord.x, coord.y, coord.z)->leadItem.checkFlag(itemFlag::PROP_WALKABLE) == false) return false;
    else if (TileEntity(coord.x, coord.y, coord.z) != nullptr) return false;
    else if (TileVehicle(coord.x, coord.y, coord.z) != nullptr)
    {
        ItemPocket* targetPocket = TileVehicle(coord.x, coord.y, coord.z)->partInfo[{coord.x, coord.y}].get();
        for (int i = 0; i < targetPocket->itemInfo.size(); i++)
        {
            if (targetPocket->itemInfo[i].checkFlag(itemFlag::VPART_NOT_WALKABLE)) return false;
        }
    }
    else if (TileFloor(coord.x, coord.y, coord.z) == 0) return false; //바닥이 없는 경우

    return true;
}

bool isRayBlocker(Point3 coord)
{
    if (TileWall(coord.x, coord.y, coord.z) != 0 && itemDex[TileWall(coord.x, coord.y, coord.z)].checkFlag(itemFlag::TRANSPARENT_WALL) == false) return true;
    else if (TileProp(coord.x, coord.y, coord.z) != nullptr && TileProp(coord.x, coord.y, coord.z)->leadItem.checkFlag(itemFlag::PROP_BLOCKER) == true) return true;
    else return false;
}

void addItemToTile(Point3 coord, int itemCode, int number)
{
    if (TileItemStack(coord.x, coord.y, coord.z) == nullptr) createItemStack(coord); //그 자리에 템 없는 경우
    TileItemStack(coord)->getPocket()->addItemFromDex(itemCode, number);
}
void addItemToTile(Point3 coord, ItemPocket* inputPokcet)
{
    if (TileItemStack(coord.x, coord.y, coord.z) == nullptr) createItemStack(coord); //그 자리에 템 없는 경우
    ItemStack* targetStack = TileItemStack(coord);
    while (inputPokcet->itemInfo.size() > 0)
    {
        int itemCount = inputPokcet->itemInfo[0].number;
        inputPokcet->transferItem(targetStack->getPocket(), 0, itemCount);
    }
}

bool isWetTile(Point3 coord)
{
    if (World::ins()->getTile(coord).lastWetTurn == -1) return false;
    else if (getElapsedTurn() - World::ins()->getTile(coord).lastWetTurn < 1440) return true;
    else return false;
}

void updateWetTile(Point3 coord)
{
    World::ins()->getTile(coord).lastWetTurn = getElapsedTurn();
}

void resetWetTile(Point3 coord)
{
    World::ins()->getTile(coord).lastWetTurn = -1;
}
