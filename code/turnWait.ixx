export module turnWait;

import std;
import util;
import globalVar;
import constVar;

//@brief 플레이어의 턴일 때 입력한 수치만큼 턴을 기다립니다.
//@param waitTime 기다린 턴(분), 예로 1.5를 입력하면 1.5턴(분)
export void turnWait(float waitTime)
{
    //prt(L"[대기] %f분을 대기했다.\n", waitTime);
    int px = Player::ins()->getGridX();
    int py = Player::ins()->getGridY();
    int pz = Player::ins()->getGridZ();

    const int GAS_UPDATE_RANGE = 29;
    std::array<std::array<int, GAS_UPDATE_RANGE>, GAS_UPDATE_RANGE> newGasVol;
    for (auto& row : newGasVol) row.fill(0);

    for (int x = px - GAS_UPDATE_RANGE; x <= px + GAS_UPDATE_RANGE; x++)
    {
        for (int y = py - GAS_UPDATE_RANGE; y <= py + GAS_UPDATE_RANGE; y++)
        {
            TileData* thisTile = &World::ins()->getTile(px, py, pz);
            if(thisTile->
        }
    }


    timeGift = waitTime;
    turnCycle = turn::playerAnime;
}