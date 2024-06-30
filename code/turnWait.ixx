export module turnWait;

import std;
import util;
import globalVar;
import constVar;
import Player;
import World;
import TileData;
import Prop;

//globalVar�� ���漱���

//@brief �÷��̾��� ���� �� �Է��� ��ġ��ŭ ���� ��ٸ��ϴ�.
//@param waitTime ��ٸ� ��(��), ���� 1.5�� �Է��ϸ� 1.5��(��)
export void turnWait(float waitTime)
{
    //prt(L"[���] %f���� ����ߴ�.\n", waitTime);

    const int GAS_UPDATE_RANGE = 29;
    std::unordered_map<std::array<int, 2>, std::vector<gasData>> tempGas;
    int px = Player::ins()->getGridX();
    int py = Player::ins()->getGridY();
    int pz = Player::ins()->getGridZ();

    static std::unordered_map<int, double> gasStack;

    //���� ����
    for (int tgtX = px - GAS_UPDATE_RANGE; tgtX <= px + GAS_UPDATE_RANGE; tgtX++)
    {
        for (int tgtY = py - GAS_UPDATE_RANGE; tgtY <= py + GAS_UPDATE_RANGE; tgtY++)
        {
            TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, pz);
            if(thisTile->gasVec.size()>0) tempGas[{tgtX, tgtY}] = thisTile->gasVec;
            thisTile->gasVec.clear();
        }
    }

    std::unordered_set<int> allowGasList;

    for (auto it = tempGas.begin(); it != tempGas.end(); it++)
    {
        for (int i = 0; i < it->second.size(); i++)
        {
            int tgtX = it->first[0];
            int tgtY = it->first[1];
            int tgtGasCode = it->second[i].gasCode;

            if (allowGasList.find(tgtGasCode) == allowGasList.end())
            {
                if (gasStack.find(tgtGasCode) == gasStack.end()) gasStack[tgtGasCode] = 0;

                gasStack[tgtGasCode] += 18.0 / (double)itemDex[tgtGasCode].molecularWeight;
                
                if (gasStack[tgtGasCode] > 1.0)
                {
                    gasStack[tgtGasCode] = 0;
                    allowGasList.insert(tgtGasCode);
                }
            }

            if (allowGasList.find(tgtGasCode) != allowGasList.end())
            {
                int totalVal = it->second[i].gasVol;

                std::vector<Point2> emptyTileDelVec;
                int openTileNumber = 1;
                emptyTileDelVec.push_back({ 0,0 });
                for (int dir = 0; dir < 8; dir++)
                {
                    int dx, dy;
                    dir2Coord(dir, dx, dy);
                    TileData* thisTile = &World::ins()->getTile(tgtX + dx, tgtY + dy, pz);
                    if (thisTile->wall == 0 || itemDex[thisTile->wall].checkFlag(itemFlag::WALL_GAS_PERMEABLE))
                    {
                        Prop* pPtr = (Prop*)(thisTile->PropPtr);
                        if (pPtr == nullptr || pPtr->leadItem.checkFlag(itemFlag::PROP_GAS_OBSTACLE_ON) == false)
                        {
                            openTileNumber++;
                            emptyTileDelVec.push_back({ dx,dy });
                        }
                    }
                }

                int divVal = it->second[i].gasVol / openTileNumber;
                if (divVal != 0)
                {
                    for (auto elem : emptyTileDelVec)
                    {
                        int dx = elem.x;
                        int dy = elem.y;
                        TileData* thisTile = &World::ins()->getTile(tgtX + dx, tgtY + dy, pz);
                        int findIndex = thisTile->checkGas(tgtGasCode);
                        if (findIndex == -1) thisTile->gasVec.push_back({ tgtGasCode,divVal });
                        else thisTile->gasVec[findIndex].gasVol += divVal;
                    }
                }
            }
            else
            {
                int totalVal = it->second[i].gasVol;
                TileData* thisTile = &World::ins()->getTile(tgtX + 0, tgtY + 0, pz);
                int findIndex = thisTile->checkGas(tgtGasCode);
                if (findIndex == -1) thisTile->gasVec.push_back({ tgtGasCode,totalVal });
                else thisTile->gasVec[findIndex].gasVol += totalVal;
            }
        }
    }
    

    timeGift = waitTime;
    turnCycle = turn::playerAnime;
}