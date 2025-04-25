import Flame;
import util;
import Coord;
import TileData;
import constVar;
import globalVar;
import World;
import Light;

Flame::Flame(Point3 inputCoor, flameFlag inputFlame)
{
    TileData* thisTile = &World::ins()->getTile(inputCoor.x, inputCoor.y, inputCoor.z);
    if (thisTile->flamePtr != nullptr)
    {
        prt(L"[makeFlag] 이미 (%d,%d,%d)에는 화염이 존재한다.\n", inputCoor.x, inputCoor.y, inputCoor.z);
        errorBox(L"중복된 위치에 Flame이 생성되었다.\n");
    }
    else
    {
        sprRandomStart = randomRange(0, 4);
        gridX = inputCoor.x;
        gridY = inputCoor.y;
        gridZ = inputCoor.z;

        switch (inputFlame)
        {
        default:
        case flameFlag::SMALL:
            lightPtr = new Light(inputCoor.x, inputCoor.y, inputCoor.z, 2, 30, lowCol::orange);
            sprInfimum = 15;
            break;
        case flameFlag::NORMAL:
            lightPtr = new Light(inputCoor.x, inputCoor.y, inputCoor.z, 3, 50, lowCol::orange);
            sprInfimum = 10;
            break;
        case flameFlag::BIG:
            lightPtr = new Light(inputCoor.x, inputCoor.y, inputCoor.z, 4, 70, lowCol::orange);
            sprInfimum = 5;
            break;
        }
    }
}

Flame::~Flame()
{
    TileData* thisTile = &World::ins()->getTile(gridX, gridY, gridZ);
    thisTile->flamePtr = nullptr;
    delete (Light*)(lightPtr);
}