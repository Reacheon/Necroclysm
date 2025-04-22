#include <sol/sol.hpp>

export module scriptLoader;

import globalVar;
import wrapVar;
import Player;
import util;
import World;

export void scriptLoader()
{
    lua["playerX"] = []() -> int { return PlayerX(); };
    lua["playerY"] = []() -> int { return PlayerY(); };
    lua["playerZ"] = []() -> int { return PlayerZ(); };

    lua["isWalkableTile"] = [](int x, int y, int z)->bool { return isWalkable({ x,y,z }); };
    lua["coord2Dir"] = [](int dx, int dy)->int { return coord2Dir(dx, dy); };
    lua["noFogTile"] = [](int x, int y, int z)->bool { return TileFov(x, y, z) == fovFlag::white; };
};