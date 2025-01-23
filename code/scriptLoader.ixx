#include <sol/sol.hpp>

export module scriptLoader;

import globalVar;
import Player;


export void scriptLoader()
{
    lua["playerX"] = []() -> int { return Player::ins()->getGridX(); };
    lua["playerY"] = []() -> int { return Player::ins()->getGridY(); };
    lua["playerZ"] = []() -> int { return Player::ins()->getGridZ(); };
};