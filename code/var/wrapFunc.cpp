#include <SDL3/SDL.h>

import wrapFunc;

import std;
import util;
import globalVar;
import constVar;
import ItemData;
import ItemPocket;
import ItemStack;
import World;
import Prop;
import Vehicle;
import Entity;
import Monster;
import Player;
import log;
import statusEffect;
import globalTime;

int PlayerX() { return PlayerPtr->getGridX(); }
int PlayerY() { return PlayerPtr->getGridY(); }
int PlayerZ() { return PlayerPtr->getGridZ(); }

const unsigned __int16 TileFloor(int x, int y, int z) { return World::ins()->getTile(x, y, z).floor; }

const unsigned __int16 TileFloor(Point3 coord) { return World::ins()->getTile(coord.x, coord.y, coord.z).floor; }

const bool TileSnow(int x, int y, int z) { return World::ins()->getTile(x, y, z).hasSnow; }

const unsigned __int16 TileWall(int x, int y, int z) { return World::ins()->getTile(x, y, z).wall; }

const bool ExistWall(int x, int y, int z) { return (World::ins()->getTile(x, y, z).wall != 0); }

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
Vehicle*& TileVehicle(int x, int y, int z) { return (Vehicle*&)World::ins()->getTile(x, y, z).VehiclePtr; }
Vehicle*& TileVehicle(Point3 pt) { return (Vehicle*&)World::ins()->getTile(pt.x, pt.y, pt.z).VehiclePtr; }

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
    Point2 cc = World::ins()->changeToChunkCoord(inputCoor.x, inputCoor.y);
    World::ins()->getChunk(cc.x, cc.y, inputCoor.z).addStack(World::ins()->getTile(inputCoor).ItemStackPtr.get());
}

void createItemStack(Point3 inputCoor, std::vector<std::pair<int, int>> inputItems)
{
    World::ins()->getTile(inputCoor).ItemStackPtr = std::make_unique<ItemStack>(inputCoor, inputItems);
    Point2 cc = World::ins()->changeToChunkCoord(inputCoor.x, inputCoor.y);
    World::ins()->getChunk(cc.x, cc.y, inputCoor.z).addStack(World::ins()->getTile(inputCoor).ItemStackPtr.get());
}

void destroyItemStack(Point3 inputCoor)
{
    Point2 cc = World::ins()->changeToChunkCoord(inputCoor.x, inputCoor.y);
    World::ins()->getChunk(cc.x, cc.y, inputCoor.z).eraseStack(World::ins()->getTile(inputCoor).ItemStackPtr.get());
    World::ins()->getTile(inputCoor).ItemStackPtr.reset();
}

void destroyProp(Point3 inputCoor)
{
    Point2 cc = World::ins()->changeToChunkCoord(inputCoor.x, inputCoor.y);
    World::ins()->getChunk(cc.x, cc.y, inputCoor.z).eraseProp(World::ins()->getTile(inputCoor).PropPtr.get());
    World::ins()->getTile(inputCoor).PropPtr.reset();
}

void createProp(Point3 inputCoor, int inputItemCode)
{
    World::ins()->getTile(inputCoor).PropPtr = std::make_unique<Prop>(inputCoor, inputItemCode);

    Point2 cc = World::ins()->changeToChunkCoord(inputCoor.x, inputCoor.y);
    World::ins()->getChunk(cc.x, cc.y, inputCoor.z).addProp(World::ins()->getTile(inputCoor).PropPtr.get());

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

float getMouseX()
{
    float px, py;
    SDL_GetMouseState(&px, &py);
    float renderX, renderY;
    SDL_RenderCoordinatesFromWindow(renderer, px, py, &renderX, &renderY);
    return renderX;
}

float getMouseY()
{
    float px, py;
    SDL_GetMouseState(&px, &py);
    float renderX, renderY;
    SDL_RenderCoordinatesFromWindow(renderer, px, py, &renderX, &renderY);
    return renderY;
}

Point2 getAbsMouseGrid()
{
    int cameraGridX, cameraGridY;
    if (cameraX >= 0) cameraGridX = cameraX / 16;
    else cameraGridX = -1 + cameraX / 16;
    if (cameraY >= 0) cameraGridY = cameraY / 16;
    else cameraGridY = -1 + cameraY / 16;

    int camDelX = cameraX - (16 * cameraGridX + 8);
    int camDelY = cameraY - (16 * cameraGridY + 8);

    int revX, revY, revGridX, revGridY;
    if (option::inputMethod == input::touch)
    {
        errorBox(activeTouchCount == 0, L"getAbsMouseGrid: 터치 입력 모드인데 활성 터치가 없습니다. 쓰레기 값 출력 중...");
        revX = static_cast<int>(event.tfinger.x * cameraW) - (cameraW / 2);
        revY = static_cast<int>(event.tfinger.y * cameraH) - (cameraH / 2);
    }
    else
    {
        revX = static_cast<int>(getMouseX()) - (cameraW / 2);
        revY = static_cast<int>(getMouseY()) - (cameraH / 2);
    }
    revX += sgn(revX) * (8 * zoomScale) + camDelX;
    revGridX = revX / (16 * zoomScale);
    revY += sgn(revY) * (8 * zoomScale) + camDelY;
    revGridY = revY / (16 * zoomScale);

    return { cameraGridX + revGridX, cameraGridY + revGridY };
}

int getVolume(const ItemData& inputData)
{
    int baseVolume = inputData.originalVolume;

    if (inputData.checkFlag(itemFlag::CONTAINER_FLEX) && inputData.pocketPtr != nullptr)
    {
        for (const auto& item : inputData.pocketPtr->itemInfo)
        {
            baseVolume += getVolume(item) * item.number;
        }
    }

    return baseVolume;
}

void sortVolumeDescend(std::vector<ItemData>& inputInfo, int startIndex, int endIndex)
{
    std::sort(inputInfo.begin() + startIndex, inputInfo.begin() + endIndex + 1,
        [](ItemData& a, ItemData& b)
        {
            return (getVolume(a) > getVolume(b));
        }
    );
}
void sortVolumeDescend(std::vector<ItemData>& inputInfo) { sortVolumeDescend(inputInfo, 0, inputInfo.size() - 1); }

void sortVolumeAscend(std::vector<ItemData>& inputInfo, int startIndex, int endIndex)
{
    std::sort(inputInfo.begin() + startIndex, inputInfo.begin() + endIndex + 1,
        [](ItemData& a, ItemData& b)
        {
            return (getVolume(a) < getVolume(b));
        }
    );
}
void sortVolumeAscend(std::vector<ItemData>& inputInfo) { sortVolumeAscend(inputInfo, 0, inputInfo.size() - 1); }

bool checkStatusEffect(std::vector<statusEffect>& inputStatus, statusEffectFlag inputFlag)
{
    for (const auto& effect : inputStatus)
    {
        if (effect.effectType == inputFlag) return true;
    }
    return false;
}

void eraseStatusEffect(std::vector<statusEffect>& inputStatus, statusEffectFlag inputFlag)
{
    for (auto it = inputStatus.begin(); it != inputStatus.end();)
    {
        if (it->effectType == inputFlag)
        {
            it = inputStatus.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

int getItemSprIndex(ItemData& inputData)
{
    if ((inputData.itemCode == itemID::arrowQuiver || inputData.itemCode == itemID::boltQuiver) && inputData.pocketPtr != nullptr)
    {
        std::vector<ItemData>& pocketInfo = inputData.pocketPtr.get()->itemInfo;

        int num = inputData.pocketPtr.get()->getPocketNumber();
        if (num == 0) return itemDex[inputData.itemCode].itemSprIndex;
        else if (num == 1) return itemDex[inputData.itemCode].itemSprIndex + 1;
        else return itemDex[inputData.itemCode].itemSprIndex + 2;
    }
    else if (inputData.checkFlag(itemFlag::CONTAINER_LIQ) && inputData.checkFlag(itemFlag::CONTAINER_TRANSPARENT) && inputData.pocketPtr != nullptr)//투명 액체 용기
    {
        std::vector<ItemData>& pocketInfo = inputData.pocketPtr.get()->itemInfo;

        if (pocketInfo.size() > 0)
        {
            if (pocketInfo[0].checkFlag(itemFlag::LIQ_COL_RED))  return inputData.itemSprIndex + 2;
            else if (pocketInfo[0].checkFlag(itemFlag::LIQ_COL_BLUE)) return inputData.itemSprIndex + 3;
            else if (pocketInfo[0].checkFlag(itemFlag::LIQ_COL_YELLOW)) return inputData.itemSprIndex + 4;
            else if (pocketInfo[0].checkFlag(itemFlag::LIQ_COL_WHITE)) return inputData.itemSprIndex + 5;
            else if (pocketInfo[0].checkFlag(itemFlag::LIQ_COL_GRAY)) return inputData.itemSprIndex + 6;
            else if (pocketInfo[0].checkFlag(itemFlag::LIQ_COL_BLACK)) return inputData.itemSprIndex + 7;
            else return inputData.itemSprIndex + 8;
        }
        else return inputData.itemSprIndex;
    }
    else if (inputData.checkFlag(itemFlag::CONTAINER_LIQ) && inputData.checkFlag(itemFlag::CONTAINER_TRANSLUCENT) && inputData.pocketPtr != nullptr)//반투명   
    {
        std::vector<ItemData>& pocketInfo = inputData.pocketPtr.get()->itemInfo;

        if (pocketInfo.size() > 0)
        {
            return inputData.itemSprIndex + 1;
        }
        else return inputData.itemSprIndex;
    }
    else return inputData.itemSprIndex;
}

void changePlayerWalkMode(walkFlag inputMode)
{
    auto& pStatus = PlayerPtr->entityInfo.statusEffectVec;

    //pStatus에서 run, crouch, crawl 모두 제거
    pStatus.erase(std::remove_if(pStatus.begin(), pStatus.end(),
        [](statusEffect& effect)
        {
            return effect.effectType == statusEffectFlag::run ||
                effect.effectType == statusEffectFlag::crouch ||
                effect.effectType == statusEffectFlag::crawl;
        }),
        pStatus.end());

    if (inputMode == walkFlag::run) pStatus.push_back({ statusEffectFlag::run, -1 });
    else if (inputMode == walkFlag::crouch) pStatus.push_back({ statusEffectFlag::crouch, -1 });
    else if (inputMode == walkFlag::crawl) pStatus.push_back({ statusEffectFlag::crawl, -1 });

    PlayerPtr->entityInfo.walkMode = inputMode;
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

int fluidTypeToCode(fluidType inputType)
{
    switch (inputType)
    {
    default:
        return 0;
    case fluidType::WATER:
        return itemID::water;
    }
}

const bool isWetTile(Point3 coord)
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