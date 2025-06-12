

export module turnWait;

import std;
import util;
import globalVar;
import wrapVar;
import constVar;
import Player;
import World;
import TileData;
import Prop;
import GameOver;

//globalVar에 전방선언됨

//@brief 플레이어의 턴일 때 입력한 수치만큼 턴을 기다립니다.
//@param waitTime 기다린 턴(분), 예로 1.5를 입력하면 1.5턴(분)
export void turnWait(float waitTime)
{
    //240708기준 2ms 정도의 실행속도

    //prt(L"[대기] %f분을 대기했다.\n", waitTime);
    const int GAS_UPDATE_RANGE = 29;
    std::unordered_map<std::array<int, 2>, std::vector<gasData>> tempGas;
    int px = PlayerX();
    int py = PlayerY();
    int pz = PlayerZ();

    static std::unordered_map<int, double> gasStack;

    //가스 복사
    for (int tgtX = px - GAS_UPDATE_RANGE; tgtX <= px + GAS_UPDATE_RANGE; tgtX++)
    {
        for (int tgtY = py - GAS_UPDATE_RANGE; tgtY <= py + GAS_UPDATE_RANGE; tgtY++)
        {
            TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, pz);
            if (thisTile->gasVec.size() > 0) tempGas[{tgtX, tgtY}] = thisTile->gasVec;
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
                        Prop* pPtr = thisTile->PropPtr.get();
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

    PlayerPtr->entityInfo.STA += 2;
    if (PlayerPtr->entityInfo.STA > PlayerPtr->entityInfo.maxSTA) PlayerPtr->entityInfo.STA = PlayerPtr->entityInfo.maxSTA;

    hunger -= static_cast<int>(waitTime * 1.2);
    thirst -= static_cast<int>(waitTime * 1.5);
    fatigue -= static_cast<int>(waitTime);

    timeGift = waitTime;
    turnCycle = turn::playerAnime;
}