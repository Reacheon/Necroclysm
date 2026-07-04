export module playerLevel;

import std;
import log;
import levelUpFX;

//플레이어 레벨/경험치/포인트 — 세션 글로벌 (세이브 미구현, 재시작 시 리셋)
//필요 경험치는 레벨당 100씩 증가: 1→2 = 100, 2→3 = 200, ...
export namespace playerLevel
{
	int level = 1;
	int exp = 0;        //현재 레벨에서 누적한 경험치
	int ap = 0;         //미분배 어빌리티 포인트 (레벨당 +1)
	int skillPoint = 0; //미사용 스킬 포인트 (레벨당 +10)

	int expToNext() { return level * 100; }

	//즉시 1레벨업 — 포인트 지급·연출·로그까지 한 번에 (F3/디버그에서도 직접 호출)
	void levelUp()
	{
		level++;
		ap += 1;
		skillPoint += 10;
		levelUpFX::trigger();
		updateLog(L"You reach Level " + std::to_wstring(level) + L". You feel new power flowing through you.");
	}

	void addExp(int amount)
	{
		if (amount <= 0) return;
		exp += amount;
		while (exp >= expToNext())
		{
			exp -= expToNext();
			levelUp();
		}
	}
}
