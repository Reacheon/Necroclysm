export module turnWait;

import std;
import util;
import globalVar;
import constVar;

//@brief �÷��̾��� ���� �� �Է��� ��ġ��ŭ ���� ��ٸ��ϴ�.
//@param waitTime ��ٸ� ��(��), ���� 1.5�� �Է��ϸ� 1.5��(��)
export void turnWait(float waitTime)
{
    //prt(L"[���] %f���� ����ߴ�.\n", waitTime);
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