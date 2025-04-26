export module wrapVar;

import std;
import util;
import Player;
import World;
import Entity;
import Prop;
import ItemStack;

export inline int PlayerX() { return Player::ins()->getGridX(); }
export inline int PlayerY() { return Player::ins()->getGridY(); }
export inline int PlayerZ() { return Player::ins()->getGridZ(); }

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

export inline Entity*& TileEntity(int x, int y, int z) { return (Entity*&)World::ins()->getTile(x, y, z).EntityPtr; }
export inline Prop* TileProp(int x, int y, int z) { return World::ins()->getTile(x, y, z).PropPtr.get(); }
export inline Vehicle*& TileVehicle(int x, int y, int z) { return (Vehicle*&)World::ins()->getTile(x, y, z).VehiclePtr; }
export inline ItemStack*& TileItemStack(int x, int y, int z) { return (ItemStack*&)World::ins()->getTile(x, y, z).ItemStackPtr; }
export inline fovFlag& TileFov(int x, int y, int z) { return (fovFlag&)World::ins()->getTile(x, y, z).fov; }

export inline void createProp(Point3 inputCoor, int inputItemCode) 
{ 
    World::ins()->getTile(inputCoor).PropPtr = std::make_unique<Prop>(inputCoor, inputItemCode);

    World::ins()->getTile(inputCoor).PropPtr->updateSprIndex();

    //주변 타일을 분석해 extraIndex 설정
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
    else return true;
};

export inline bool isRayBlocker(Point3 coord)
{
    if (TileWall(coord.x, coord.y, coord.z) != 0 && itemDex[TileWall(coord.x, coord.y, coord.z)].checkFlag(itemFlag::TRANSPARENT_WALL) == false) return true;
    else if (TileProp(coord.x, coord.y, coord.z) != nullptr && TileProp(coord.x, coord.y, coord.z)->leadItem.checkFlag(itemFlag::PROP_BLOCKER) == true) return true;
    else return false;
};








