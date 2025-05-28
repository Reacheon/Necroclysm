#include <SDL3/SDL.h>

export module wrapVar;

import std;
import util;
import globalVar;
import Player;
import World;
import Entity;
import Monster;
import Prop;
import ItemStack;
import ItemPocket;

export inline int PlayerX() { return PlayerPtr->getGridX(); }
export inline int PlayerY() { return PlayerPtr->getGridY(); }
export inline int PlayerZ() { return PlayerPtr->getGridZ(); }

export inline const unsigned __int16 TileFloor(int x, int y, int z) { return World::ins()->getTile(x, y, z).floor; }

export inline const bool TileSnow(int x, int y, int z) { return World::ins()->getTile(x, y, z).hasSnow; }

export inline const unsigned __int16 TileWall(int x, int y, int z) { return World::ins()->getTile(x, y, z).wall;}

export inline const bool ExistWall(int x, int y, int z) { return (World::ins()->getTile(x, y, z).wall != 0); }

export inline void setWall(Point3 coord, int val)
{
    World::ins()->getTile(coord.x, coord.y, coord.z).setWall(val);
};

export inline void setFloor(Point3 coord, int val)
{
    World::ins()->getTile(coord.x, coord.y, coord.z).setFloor(val);
};

export inline Entity* TileEntity(int x, int y, int z) { return (Entity*&)World::ins()->getTile(x, y, z).EntityPtr; }
export inline void EntityPtrMove(Point3 startCoor, Point3 endCoor)
{
    World::ins()->getTile(endCoor).EntityPtr = std::move(World::ins()->getTile(startCoor).EntityPtr);
    World::ins()->getTile(endCoor).EntityPtr->setGrid(endCoor.x, endCoor.y, endCoor.z);
    World::ins()->getTile(endCoor).EntityPtr->pullEquipLights();
}
export inline void EntityPtrMove(std::unique_ptr<Entity> inputPtr, Point3 endCoor)
{
    World::ins()->getTile(endCoor).EntityPtr = std::move(inputPtr);
    World::ins()->getTile(endCoor).EntityPtr->setGrid(endCoor.x, endCoor.y, endCoor.z);
    World::ins()->getTile(endCoor).EntityPtr->pullEquipLights();
}
export inline Prop* TileProp(int x, int y, int z) { return World::ins()->getTile(x, y, z).PropPtr.get(); }
export inline Vehicle*& TileVehicle(int x, int y, int z) { return (Vehicle*&)World::ins()->getTile(x, y, z).VehiclePtr; }

export inline ItemStack* TileItemStack(int x, int y, int z) { return World::ins()->getTile(x, y, z).ItemStackPtr.get(); }
export inline ItemStack* TileItemStack(Point3 pt) { return World::ins()->getTile(pt.x, pt.y, pt.z).ItemStackPtr.get(); }

export inline fovFlag& TileFov(int x, int y, int z) { return (fovFlag&)World::ins()->getTile(x, y, z).fov; }

export inline void createMonster(Point3 inputCoor, int inputEntityCode)
{
    World::ins()->getTile(inputCoor).EntityPtr = std::make_unique<Monster>(inputEntityCode, inputCoor.x, inputCoor.y, inputCoor.z);
}

export inline void createItemStack(Point3 inputCoor)
{
    World::ins()->getTile(inputCoor).ItemStackPtr = std::make_unique<ItemStack>(inputCoor);
}

export inline void createItemStack(Point3 inputCoor, std::vector<std::pair<int, int>> inputItems)
{
    World::ins()->getTile(inputCoor).ItemStackPtr = std::make_unique<ItemStack>(inputCoor, inputItems);
}

export inline void destroyItemStack(Point3 inputCoor)
{
    World::ins()->getTile(inputCoor).ItemStackPtr.reset();
}


export inline void createProp(Point3 inputCoor, int inputItemCode) 
{ 
    World::ins()->getTile(inputCoor).PropPtr = std::make_unique<Prop>(inputCoor, inputItemCode);

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
export inline void createFlame(Point3 inputCoor, flameFlag inputFlag) { World::ins()->getTile(inputCoor).flamePtr = std::make_unique<Flame>(inputCoor, inputFlag); }

export inline void DestroyWall(int x, int y, int z) { World::ins()->getTile(x, y, z).destoryWall(); }

export inline bool isWalkable(Point3 coord)
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

    return true;
};

export inline bool isRayBlocker(Point3 coord)
{
    if (TileWall(coord.x, coord.y, coord.z) != 0 && itemDex[TileWall(coord.x, coord.y, coord.z)].checkFlag(itemFlag::TRANSPARENT_WALL) == false) return true;
    else if (TileProp(coord.x, coord.y, coord.z) != nullptr && TileProp(coord.x, coord.y, coord.z)->leadItem.checkFlag(itemFlag::PROP_BLOCKER) == true) return true;
    else return false;
};

export int updateQuiverSpr(ItemData& inputData)
{
    if (inputData.itemCode == itemRefCode::arrowQuiver || inputData.itemCode == itemRefCode::boltQuiver)
    {
        std::vector<ItemData>& pocketInfo = inputData.pocketPtr.get()->itemInfo;
        int num = inputData.pocketPtr.get()->getPocketNumber();
        if (num == 0) inputData.sprIndex = itemDex[inputData.itemCode].sprIndex;
        else if (num == 1) inputData.sprIndex = itemDex[inputData.itemCode].sprIndex + 1;
        else inputData.sprIndex = itemDex[inputData.itemCode].sprIndex + 2;

        return num;
    }
}

export void updateQuiverSpr(ItemPocket* inputPocket)
{
    for (int i = 0; i < inputPocket->itemInfo.size(); i++) updateQuiverSpr(inputPocket->itemInfo[i]);
}

//export float getMouseX()
//{
//    float mouseX;
//    SDL_GetMouseState(&mouseX, nullptr);
//    return mouseX;
//}
//
//export float getMouseY()
//{
//    float mouseY;
//    SDL_GetMouseState(nullptr, &mouseY);
//    return mouseY;
//}   



export float getMouseX()
{
    float px, py;
    SDL_GetMouseState(&px, &py);

    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);

    float scaleX = (float)cameraW / (float)winW;
    return px * scaleX;
}

export float getMouseY()
{
    float px, py;
    SDL_GetMouseState(&px, &py);

    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);

    float scaleY = (float)cameraH / (float)winH;
    return py * scaleY;
}
