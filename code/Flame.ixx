export module Flame;

import util;
import constVar;
import Light;

export struct Flame
{
    int gridX, gridY, gridZ;
    flameFlag flameType = flameFlag::NONE;
    std::unique_ptr<Light> flameLightPtr;
    unsigned __int8 sprInfimum = 0;
    unsigned __int8 sprRandomStart = 0;

    Flame(Point3 inputCoor, flameFlag inputFlame);
    ~Flame();
};
