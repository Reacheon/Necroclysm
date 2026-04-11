export module SkillNervedrive;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;
import log;

// 토글 바이오닉: 신경 가속 (머리 부위)
// 작동 중 모든 행동의 턴소모가 0이 된다. 매턴 500kJ 소모.
export class SkillNervedrive : public SkillBehavior
{
public:
	static constexpr int ENERGY_PER_TURN = 500; // 턴당 에너지 소모량 (kJ)

	SkillNervedrive()
	{
		name = L"Nervedrive";
		iconIndex = 10;
		descript = L"Accelerates neural processing to extreme speeds. All actions cost 0 turns while active. Drains 500 kJ per turn.";
		src = skillSrc::BIONIC;
		type = skillType::TOGGLE;
		skillRank = L"F";
		maxCooldown = 0.0f;
		energyPerAct = 0.0f;
		energyPerTurn = 500.0f;
	}

	int getSkillCode() const override { return 50; }

	// 에너지가 500kJ 미만이면 토글 ON 불가
	bool canUse(Entity* caster, const SkillData& data) const override
	{
		if (!data.toggle && caster->entityInfo.energy < ENERGY_PER_TURN)
		{
			updateLog(L"Not enough bionic energy to activate Nervedrive.");
			return false;
		}
		return true;
	}

	void onToggleOn(Entity* caster, const SkillData& data) override
	{
		nervedriveOn = true;
		updateLog(L"Nervedrive activated. Your perception of time slows to a crawl.");
	}

	void onToggleOff(Entity* caster, const SkillData& data) override
	{
		nervedriveOn = false;
		updateLog(L"Nervedrive deactivated.");
	}

	// 매 턴 호출: 에너지 차감 후 다음 턴 에너지 부족하면 자동 종료
	void onTurnTick(Entity* caster, SkillData& data) override
	{
		if (!data.toggle) return;

		caster->entityInfo.energy -= ENERGY_PER_TURN;
		if (caster->entityInfo.energy < 0) caster->entityInfo.energy = 0;

		// 다음 턴에 쓸 에너지가 부족하면 종료
		if (caster->entityInfo.energy < ENERGY_PER_TURN)
		{
			data.toggle = false;
			nervedriveOn = false;
			updateLog(L"Nervedrive shuts down due to insufficient energy.");
		}
	}

	// execute는 토글 스킬이므로 사용되지 않지만 순수가상함수 구현 필요
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
