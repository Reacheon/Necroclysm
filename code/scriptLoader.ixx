#include <sol/sol.hpp>

export module scriptLoader;

import globalVar;
import Player;
import util;
import World;

export void scriptLoader()
{
    lua["playerX"] = []() -> int { return Player::ins()->getGridX(); };
    lua["playerY"] = []() -> int { return Player::ins()->getGridY(); };
    lua["playerZ"] = []() -> int { return Player::ins()->getGridZ(); };

    lua["isWalkableTile"] = [](int x, int y, int z)->bool { return World::ins()->getTile(x, y, z).walkable; };
    lua["coord2Dir"] = [](int dx, int dy)->int { return coord2Dir(dx, dy); };
    lua["noFogTile"] = [](int x, int y, int z)->bool { return World::ins()->getTile(x, y, z).fov == fovFlag::white; };
};