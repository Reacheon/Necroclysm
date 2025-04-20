export module Ani;

import util;
import constVar;

export class Ani
{
private:
	int timer = 0;
	aniFlag aniType = aniFlag::null;
	bool dictator = false; //독재변수 : true일 경우 턴사이클에서 혼자만 애니메이션 재생됨(동시 재생 불가)
	int priority = 1; //독재프롭 우선도 4, 일반 프롭 우선도 3, 독재 엔티티 우선도 2, 일반 엔티티 우선도 1
public:
	virtual bool runAnimation(bool shutdown)
	{
		return false;
	}
	int getTimer() { return timer; }
	void addTimer() { timer++; }
	void resetTimer() { timer = 0; }
	void setAniType(aniFlag enumVal){aniType = enumVal;}
	aniFlag getAniType() { return aniType; }
	void setAniPriority(int inputVal) { priority = inputVal; }
	int getAniPriority() { return priority; }
	void iAmDictator() 
	{ 
		if (dictator == false)
		{
			dictator = true;
			priority += 1;
		}
		else errorBox(L"이미 독재 타입의 애니인데 독재타입으로 변화하려 하였다 in iAmDictaotr()");
	}
	void iAmNotDictator() 
	{ 
		if (dictator == true)
		{
			dictator = false;
			priority -= 1;
		}
		else errorBox(L"이미 독재 타입의 애니가 아닌데 독재타입을 버리려 하였다 in iAmNotDictaotr()");
	}
	bool isDictator() { return dictator; }
};
