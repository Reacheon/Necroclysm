export module Flame;

import util;
import Coord;
import TileData;
import constVar;
import globalVar;
import World;
import Light;

export struct Flame
{
    int gridX, gridY, gridZ;
    flameFlag flameType = flameFlag::NONE;
    Light* lightPtr = nullptr;
    unsigned __int8 sprInfimum = 0;
    unsigned __int8 sprRandomStart = 0;

    Flame(int inputGridX, int inputGridY, int inputGridZ, flameFlag inputFlame)
    {
        TileData* thisTile = &World::ins()->getTile(inputGridX, inputGridY, inputGridZ);
        if (thisTile->flamePtr != nullptr)
        {
            prt(L"[makeFlag] 이미 (%d,%d,%d)에는 화염이 존재한다.\n", inputGridX, inputGridY, inputGridZ);
            errorBox(L"중복된 위치에 Flame이 생성되었다.\n");
        }
        else
        {
            sprRandomStart = randomRange(0, 4);
            gridX = inputGridX;
            gridY = inputGridY;
            gridZ = inputGridZ;

            thisTile->flamePtr = this;
            switch (inputFlame)
            {
            default:
            case flameFlag::SMALL:
                lightPtr = new Light(inputGridX, inputGridY, inputGridZ, 2, 30, lowCol::orange);
                sprInfimum = 15;
                break;
            case flameFlag::NORMAL:
                lightPtr = new Light(inputGridX, inputGridY, inputGridZ, 3, 50, lowCol::orange);
                sprInfimum = 10;
                break;
            case flameFlag::BIG:
                lightPtr = new Light(inputGridX, inputGridY, inputGridZ, 4, 70, lowCol::orange);
                sprInfimum = 5;
                break;
            }
        }
    }

    ~Flame()
    {
        TileData* thisTile = &World::ins()->getTile(gridX, gridY, gridZ);
        thisTile->flamePtr = nullptr;
        delete (Light*)(lightPtr);
    }
};
