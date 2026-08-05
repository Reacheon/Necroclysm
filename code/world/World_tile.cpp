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

int TileFloor(int x, int y, int z)
{
    TileData* t = World::ins()->tryGetTile(x, y, z);
    return t != nullptr ? t->floor : itemID::none;
}

int TileFloor(Point3 coord)
{
    TileData* t = World::ins()->tryGetTile(coord.x, coord.y, coord.z);
    return t != nullptr ? t->floor : itemID::none;
}

bool TileSnow(int x, int y, int z) { return World::ins()->getTile(x, y, z).hasSnow; }

int TileWall(int x, int y, int z) { return World::ins()->getTile(x, y, z).wall; }

bool ExistWall(int x, int y, int z) { return (World::ins()->getTile(x, y, z).wall != itemID::none); }

void setWall(Point3 coord, int val)
{
    World::ins()->getTile(coord.x, coord.y, coord.z).setWall(val);
}

void setFloor(Point3 coord, int val)
{
    World::ins()->getTile(coord.x, coord.y, coord.z).setFloor(val);
}

Entity* TileEntity(int x, int y, int z)
{
    // 줌아웃 hover 판정 등 로드 영역 경계 밖을 조회할 수 있는 read-only 경로가 호출하므로
    // getTile(.at() 예외) 대신 tryGetTile로 미로드 청크는 nullptr 처리 (TileFloor와 동일 패턴).
    TileData* t = World::ins()->tryGetTile(x, y, z);
    return t != nullptr ? t->EntityPtr.get() : nullptr;
}

void EntityPtrMove(Point3 startCoor, Point3 endCoor)
{
    // 1단계: 시작 좌표의 EntityPtr 존재 확인
    auto& startTile = World::ins()->getTile(startCoor);
    errorBox(startTile.EntityPtr == nullptr,
        L"EntityPtrMove: 시작 좌표에 EntityPtr이 없습니다. startCoor=(" +
        std::to_wstring(startCoor.x) + L"," + std::to_wstring(startCoor.y) + L"," + std::to_wstring(startCoor.z) + L")");

    // CDDA식 ramp 짝맞춤 + 양방향 전이.
    // 정방향: 진행 방향 2칸 앞 z±1에 floor + 2칸 앞이 ramp 아님 → z 전이
    // 역방향: 다리 위/하층에서 ramp 통과 시 다리/하층이 끝나면 자동 합류
    Point3 finalCoor = endCoor;
    {
        int stepDx = endCoor.x - startCoor.x;
        int stepDy = endCoor.y - startCoor.y;
        if (stepDx != 0 || stepDy != 0)
        {
            Prop* arrivedProp = TileProp(endCoor.x, endCoor.y, endCoor.z);
            Prop* belowProp = TileProp(endCoor.x, endCoor.y, endCoor.z - 1);
            Prop* aboveProp = TileProp(endCoor.x, endCoor.y, endCoor.z + 1);
            Prop* nextProp = TileProp(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z);
            bool nextIsRamp = nextProp != nullptr &&
                (nextProp->leadItem.checkFlag(itemFlag::RAMP_UP)
                    || nextProp->leadItem.checkFlag(itemFlag::RAMP_DOWN));
            int dz = 0;
            if (arrivedProp != nullptr && arrivedProp->leadItem.checkFlag(itemFlag::RAMP_UP)
                && TileFloor(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z + 1) != itemID::none
                && !nextIsRamp)
            {
                dz = 1;
            }
            else if (arrivedProp != nullptr && arrivedProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
                && TileFloor(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z - 1) != itemID::none
                && !nextIsRamp)
            {
                dz = -1;
            }
            // 역방향 하강: 위쪽 z 이동 중, 아래에 RAMP_UP, 진행 2칸 앞 다리 끝
            else if (belowProp != nullptr && belowProp->leadItem.checkFlag(itemFlag::RAMP_UP)
                && TileFloor(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z) == itemID::none
                && TileFloor(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z - 1) != itemID::none)
            {
                dz = -1;
            }
            // 역방향 상승: 아래쪽 z 이동 중, 위에 RAMP_DOWN, 진행 2칸 앞 하층 끝
            else if (aboveProp != nullptr && aboveProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
                && TileFloor(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z) == itemID::none
                && TileFloor(endCoor.x + stepDx, endCoor.y + stepDy, endCoor.z + 1) != itemID::none)
            {
                dz = 1;
            }
            if (dz != 0)
            {
                Point3 zCoor = { endCoor.x, endCoor.y, endCoor.z + dz };
                if (World::ins()->getTile(zCoor).EntityPtr == nullptr)
                {
                    finalCoor = zCoor;
                }
            }
        }
    }

    // 2단계: 최종 목적지의 EntityPtr이 비어있는지 확인
    auto& finalTile = World::ins()->getTile(finalCoor);
    errorBox(finalTile.EntityPtr != nullptr,
        L"EntityPtrMove: 목적지에 이미 EntityPtr이 있습니다. finalCoor=(" +
        std::to_wstring(finalCoor.x) + L"," + std::to_wstring(finalCoor.y) + L"," + std::to_wstring(finalCoor.z) + L")");

    // 3단계: 이동 실행
    finalTile.EntityPtr = std::move(startTile.EntityPtr);

    // 4단계: 이동 후 EntityPtr 존재 확인
    errorBox(finalTile.EntityPtr == nullptr,
        L"EntityPtrMove: 이동 후 EntityPtr이 nullptr입니다. finalCoor=(" +
        std::to_wstring(finalCoor.x) + L"," + std::to_wstring(finalCoor.y) + L"," + std::to_wstring(finalCoor.z) + L")");

    // 5단계: setGrid 호출
    finalTile.EntityPtr->setGrid(finalCoor.x, finalCoor.y, finalCoor.z);
    finalTile.EntityPtr->pullEquipLights();
}

void EntityPtrMove(std::unique_ptr<Entity> inputPtr, Point3 endCoor)
{
    World::ins()->getTile(endCoor).EntityPtr = std::move(inputPtr);
    World::ins()->getTile(endCoor).EntityPtr->setGrid(endCoor.x, endCoor.y, endCoor.z);
    World::ins()->getTile(endCoor).EntityPtr->pullEquipLights();
}

Prop* TileProp(int x, int y, int z)
{
    TileData* t = World::ins()->tryGetTile(x, y, z);
    return t != nullptr ? t->PropPtr.get() : nullptr;
}
Prop* TileProp(Point3 pt)
{
    TileData* t = World::ins()->tryGetTile(pt.x, pt.y, pt.z);
    return t != nullptr ? t->PropPtr.get() : nullptr;
}
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

    //철거로 연결이 끊긴 주변 설치물의 extraIndex(연결) 갱신
    int dx = 0;
    int dy = 0;
    for (int i = 0; i < 8; i++)
    {
        dir2Coord(i, dx, dy);
        Prop* targetProp = TileProp(inputCoor.x + dx, inputCoor.y + dy, inputCoor.z);
        if (targetProp != nullptr) targetProp->updateSprIndex();
    }
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
    // 미로드 청크 타일은 보행 불가로 간주. 아래 TileVehicle 등 getTile().at()이 던지는
    // std::out_of_range 방지 — 월드 로드 직후 청크 스트리밍 전, 자동이동 A*가 플레이어
    // 주변 박스(±20)를 훑으며 미로드 타일에 닿을 수 있다. 미탐사 void는 경로가 될 수 없으므로 false가 맞다.
    if (World::ins()->tryGetTile(coord.x, coord.y, coord.z) == nullptr) return false;
    if (TileWall(coord.x, coord.y, coord.z) != itemID::none) return false;
    else if (TileProp(coord.x, coord.y, coord.z) != nullptr && TileProp(coord.x, coord.y, coord.z)->leadItem.checkFlag(itemFlag::PROP_WALKABLE) == false) return false;
    else if (TileEntity(coord.x, coord.y, coord.z) != nullptr) return false;
    else if (TileVehicle(coord.x, coord.y, coord.z) != nullptr)
    {
        // partInfo와 TileVehicle 등록 불일치 케이스 방어 (transition 중에 발생 가능)
        auto& vehPartInfo = TileVehicle(coord.x, coord.y, coord.z)->partInfo;
        auto it = vehPartInfo.find({ coord.x, coord.y, coord.z });
        if (it != vehPartInfo.end() && it->second != nullptr)
        {
            ItemPocket* targetPocket = it->second.get();
            for (int i = 0; i < targetPocket->itemInfo.size(); i++)
            {
                if (targetPocket->itemInfo[i].checkFlag(itemFlag::VPART_NOT_WALKABLE)) return false;
            }
        }
    }
    else if (TileFloor(coord.x, coord.y, coord.z) == itemID::none) return false; //바닥이 없는 경우

    return true;
}

bool isRayBlocker(Point3 coord)
{
    if (TileWall(coord.x, coord.y, coord.z) != itemID::none && itemDex[TileWall(coord.x, coord.y, coord.z)].checkFlag(itemFlag::TRANSPARENT_WALL) == false) return true;
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
