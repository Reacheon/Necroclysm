export module stepEvent;

import globalVar;
import constVar;
import util;
import Damage;
import Corpse;
import GUI;
import clickHold;

//GUI의 스텝이벤트를 실행시키는 함수

export __int64 stepEvent()
{
    __int64 timeStampStart = getNanoTimer();

    if (dxClickStack > TOLERANCE_HOLD_DEL_XY || dyClickStack > TOLERANCE_HOLD_DEL_XY) deactHold = true;

    if (getMilliTimer() - clickStartTime > 1000)
    {
        if (deactHold == false)
        {
            clickHold();
        }
    }
	//데미지 객체 스탭 이벤트 실행
	for (int i = 0; i < Damage::list.size(); i++){Damage::list[i]->step();}

    //시체 객체 스텝 이벤트 실행
	for (int i = 0; i < Corpse::list.size(); i++){Corpse::list[i]->step();}

    //GUI 객체 스텝 이벤트 실행
    for (int i = 0; i < GUI::getActiveGUIList().size(); i++){GUI::getActiveGUIList()[i]->step();}

    return (getNanoTimer() - timeStampStart);
}