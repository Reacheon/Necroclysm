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
    timeGift = waitTime;
    turnCycle = turn::playerAnime;
}