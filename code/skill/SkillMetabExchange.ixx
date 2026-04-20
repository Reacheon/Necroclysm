export module SkillMetabExchange;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;
import log;

// 토글 바이오닉: 대사 교환기 (몸통 부위)
// 허기를 소모하여 바이오닉 에너지를 충전한다.
export class SkillMetabExchange : public SkillBehavior
{
public:
	static constexpr double HUNGER_PER_TURN = 0.3;   // 턴당 허기 증가량
	static constexpr int ENERGY_PER_TURN = 15;        // 턴당 에너지 충전량
	static constexpr double HUNGER_THRESHOLD = 75.0;  // 자동 종료 허기 임계값 (%)

	SkillMetabExchange()
	{
		id = L"BION_METAB_EXCHANGE";
		name = L"Metab Exchange";
		iconIndex = 11;
		descript = L"Converts metabolic energy into bionic power. Increases hunger over time.";
		src = skillSrc::BIONIC;
		type = skillType::TOGGLE;
		skillRank = L"F";
		maxCooldown = 0.0f;
		energyPerAct = 0.0f;
	}

	// 허기 75% 이상이면 토글 ON 불가
	bool canUse(Entity* caster, const SkillData& data) const override
	{
		if (!data.toggle && hunger >= HUNGER_THRESHOLD)
		{
			updateLog(L"You are too hungry to activate Metab Exchange.");
			return false;
		}
		return true;
	}

	void onToggleOn(Entity* caster, const SkillData& data) override
	{
		updateLog(L"Metab Exchange activated. Your body begins converting nutrients into power.");
	}

	void onToggleOff(Entity* caster, const SkillData& data) override
	{
		updateLog(L"Metab Exchange deactivated.");
	}

	// 매 턴 호출: 허기 증가 + 에너지 충전, 허기 75% 이상이면 자동 종료
	void onTurnTick(Entity* caster, SkillData& data) override
	{
		if (!data.toggle) return;

		// 허기 임계값 초과 시 자동 종료
		if (hunger >= HUNGER_THRESHOLD)
		{
			data.toggle = false;
			updateLog(L"Metab Exchange automatically shuts down due to extreme hunger.");
			return;
		}

		hunger += HUNGER_PER_TURN;
		caster->entityInfo.energy += ENERGY_PER_TURN;
		if (caster->entityInfo.energy > caster->entityInfo.maxEnergy)
			caster->entityInfo.energy = caster->entityInfo.maxEnergy;
	}

	// execute는 토글 스킬이므로 사용되지 않지만 순수가상함수 구현 필요
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
