export module initCoordTransform;

import std;
import util;
import globalVar;
import constVar;

export void initCoordTransform()
{
    coordTransform.clear();

    auto xShiftLeft = [=](dir16 dir, int y) {
        for (int x = -maxVehicleSize / 2; x <= maxVehicleSize / 2 - 1; x++) coordTransform[dir][{x, y}] = coordTransform[dir][{x + 1, y}];
        coordTransform[dir][{maxVehicleSize / 2, y}][0]++;
        };

    auto xShiftRight = [=](dir16 dir, int y) {
        for (int x = maxVehicleSize / 2; x >= -maxVehicleSize / 2 + 1; x--) coordTransform[dir][{x, y}] = coordTransform[dir][{x - 1, y}];
        coordTransform[dir][{-maxVehicleSize / 2, y}][0]--;
        };

    auto yShiftUp = [=](dir16 dir, int x) {
        for (int y = -maxVehicleSize / 2; y < maxVehicleSize / 2 - 1; y++) coordTransform[dir][{x, y}] = coordTransform[dir][{x, y + 1}];
        coordTransform[dir][{x, maxVehicleSize / 2}][1]++;
        };

    auto yShiftDown = [=](dir16 dir, int x) {
        for (int y = maxVehicleSize / 2; y >= -maxVehicleSize / 2 + 1; y--) coordTransform[dir][{x, y}] = coordTransform[dir][{x, y - 1}];
        coordTransform[dir][{x, -maxVehicleSize / 2}][1]--;
        };

    //★방향 0도
    for (int x = -maxVehicleSize / 2; x <= maxVehicleSize / 2; x++)
    {
        for (int y = -maxVehicleSize / 2; y <= maxVehicleSize / 2; y++)
        {
            coordTransform[dir16::dir0][{x, y}] = { x,y };
        }
    }

    //★방향 90도
    for (int x = -maxVehicleSize / 2; x <= maxVehicleSize / 2; x++)
    {
        for (int y = -maxVehicleSize / 2; y <= maxVehicleSize / 2; y++)
        {
            coordTransform[dir16::dir2][{x, y}] = { -y,x };
        }
    }

    //★방향 180도
    for (int x = -maxVehicleSize / 2; x <= maxVehicleSize / 2; x++)
    {
        for (int y = -maxVehicleSize / 2; y <= maxVehicleSize / 2; y++)
        {
            coordTransform[dir16::dir4][{x, y}] = { -x,-y };
        }
    }

    //★방향 270도
    for (int x = -maxVehicleSize / 2; x <= maxVehicleSize / 2; x++)
    {
        for (int y = -maxVehicleSize / 2; y <= maxVehicleSize / 2; y++)
        {
            coordTransform[dir16::dir6][{x, y}] = { y,-x };
        }
    }

    //★방향 27.5도
    coordTransform[dir16::dir0_5] = coordTransform[dir16::dir0];
    int repeat = 1;
    for (int x = 1; x <= maxVehicleSize / 2; x++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftUp(dir16::dir0_5, x);
        repeat++;
    }
    repeat = 1;
    for (int x = -1; x >= -maxVehicleSize / 2; x--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftDown(dir16::dir0_5, x);
        repeat++;
    }

    //★방향 45도
    coordTransform[dir16::dir1] = coordTransform[dir16::dir2];
    for (int i = 0; i <= maxVehicleSize / 2 - 1; i++)
    {
        for (int y = -1 - i; y >= -maxVehicleSize / 2; y--) xShiftRight(dir16::dir1, y);

        for (int y = +1 + i; y <= maxVehicleSize / 2; y++) xShiftLeft(dir16::dir1, y);
    }

    //★방향 67.5도
    coordTransform[dir16::dir1_5] = coordTransform[dir16::dir2];
    repeat = 1;
    for (int y = 1; y <= maxVehicleSize / 2; y++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftLeft(dir16::dir1_5, y);
        repeat++;
    }
    repeat = 1;
    for (int y = -1; y >= -maxVehicleSize / 2; y--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftRight(dir16::dir1_5, y);
        repeat++;
    }

    //방향 112.5도
    coordTransform[dir16::dir2_5] = coordTransform[dir16::dir2];
    repeat = 1;
    for (int y = 1; y <= maxVehicleSize / 2; y++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftRight(dir16::dir2_5, y);
        repeat++;
    }
    repeat = 1;
    for (int y = -1; y >= -maxVehicleSize / 2; y--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftLeft(dir16::dir2_5, y);
        repeat++;
    }

    //방향 135도
    coordTransform[dir16::dir3] = coordTransform[dir16::dir2];
    for (int i = 0; i <= maxVehicleSize / 2 - 1; i++)
    {
        for (int y = -1 - i; y >= -maxVehicleSize / 2; y--) xShiftLeft(dir16::dir3, y);

        for (int y = +1 + i; y <= maxVehicleSize / 2; y++) xShiftRight(dir16::dir3, y);
    }

    //방향 157.5도
    coordTransform[dir16::dir3_5] = coordTransform[dir16::dir4];
    repeat = 1;
    for (int x = 1; x <= maxVehicleSize / 2; x++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftDown(dir16::dir3_5, x);
        repeat++;
    }
    repeat = 1;
    for (int x = -1; x >= -maxVehicleSize / 2; x--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftUp(dir16::dir3_5, x);
        repeat++;
    }

    //방향 202.5도
    coordTransform[dir16::dir4_5] = coordTransform[dir16::dir4];
    repeat = 1;
    for (int x = 1; x <= maxVehicleSize / 2; x++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftUp(dir16::dir4_5, x);
        repeat++;
    }
    repeat = 1;
    for (int x = -1; x >= -maxVehicleSize / 2; x--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftDown(dir16::dir4_5, x);
        repeat++;
    }

    //방향 225도
    coordTransform[dir16::dir5] = coordTransform[dir16::dir6];
    for (int i = 0; i <= maxVehicleSize / 2 - 1; i++)
    {
        for (int y = -1 - i; y >= -maxVehicleSize / 2; y--) xShiftRight(dir16::dir5, y);

        for (int y = +1 + i; y <= maxVehicleSize / 2; y++) xShiftLeft(dir16::dir5, y);
    }

    //방향 247.5도
    coordTransform[dir16::dir5_5] = coordTransform[dir16::dir6];
    repeat = 1;
    for (int y = -1; y >= -maxVehicleSize / 2; y--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftRight(dir16::dir5_5, y);
        repeat++;
    }
    repeat = 1;
    for (int y = 1; y <= maxVehicleSize / 2; y++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftLeft(dir16::dir5_5, y);
        repeat++;
    }

    //방향 292.5도
    coordTransform[dir16::dir6_5] = coordTransform[dir16::dir6];
    repeat = 1;
    for (int y = -1; y >= -maxVehicleSize / 2; y--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftLeft(dir16::dir6_5, y);
        repeat++;
    }
    repeat = 1;
    for (int y = 1; y <= maxVehicleSize / 2; y++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) xShiftRight(dir16::dir6_5, y);
        repeat++;
    }

    //방향 315도
    coordTransform[dir16::dir7] = coordTransform[dir16::dir6];
    for (int i = 0; i <= maxVehicleSize / 2 - 1; i++)
    {
        for (int y = -1 - i; y >= -maxVehicleSize / 2; y--) xShiftLeft(dir16::dir7, y);

        for (int y = +1 + i; y <= maxVehicleSize / 2; y++) xShiftRight(dir16::dir7, y);
    }

    //방향 337.5도
    coordTransform[dir16::dir7_5] = coordTransform[dir16::dir0];
    repeat = 1;
    for (int x = 1; x <= maxVehicleSize / 2; x++)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftDown(dir16::dir7_5, x);
        repeat++;
    }
    repeat = 1;
    for (int x = -1; x >= -maxVehicleSize / 2; x--)
    {
        for (int i = 0; i < (repeat + 1) / 2; i++) yShiftUp(dir16::dir7_5, x);
        repeat++;
    }

    bool doPrt = false;;
    if (doPrt)
    {
        bool indicateCoord = false;
        auto prtCoordTransform = [=](dir16 input)
            {
                prt(L"\n[initCoordTransform] ▼반시계방향으로 %ls만큼 회전▼\n", dir16ToString(input).c_str());

                for (int y = -maxVehicleSize / 2; y <= maxVehicleSize / 2; y++)
                {
                    for (int x = -maxVehicleSize / 2; x <= maxVehicleSize / 2; x++)
                    {

                        if (std::abs(coordTransform[input][{x, y}][1]) <= 2 && coordTransform[input][{x, y}][0] <= 14 && coordTransform[input][{x, y}][0] >= -4) std::printf("\033[38;5;214m");
                        if (coordTransform[input][{x, y}][0] == 0 && coordTransform[input][{x, y}][1] == 0) std::printf("\033[31m");
                        if (indicateCoord)
                        {
                            std::cout << "("
                                << std::setw(3) << std::setfill(' ') << coordTransform[input][{x, y}][0] << ","
                                << std::setw(3) << std::setfill(' ') << coordTransform[input][{x, y}][1]
                                << ")";
                        }
                        else
                        {
                            std::printf("■");
                        }
                        if (coordTransform[input][{x, y}][0] == 0 && coordTransform[input][{x, y}][1] == 0) std::printf("\033[0m");
                        if (std::abs(coordTransform[input][{x, y}][1]) <= 2 && coordTransform[input][{x, y}][0] <= 14 && coordTransform[input][{x, y}][0] >= -4) std::printf("\033[0m");
                        if (x == maxVehicleSize / 2) std::cout << "\n";
                    }
                }
            };


        prtCoordTransform(dir16::dir0);
        prtCoordTransform(dir16::dir0_5);
        prtCoordTransform(dir16::dir1);
        prtCoordTransform(dir16::dir1_5);
        prtCoordTransform(dir16::dir2);
        prtCoordTransform(dir16::dir2_5);
        prtCoordTransform(dir16::dir3);
        prtCoordTransform(dir16::dir3_5);
        prtCoordTransform(dir16::dir4);
        prtCoordTransform(dir16::dir4_5);
        prtCoordTransform(dir16::dir5);
        prtCoordTransform(dir16::dir5_5);
        prtCoordTransform(dir16::dir6);
        prtCoordTransform(dir16::dir6_5);
        prtCoordTransform(dir16::dir7);
        prtCoordTransform(dir16::dir7_5);
    }
}