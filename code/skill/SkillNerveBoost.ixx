export module SkillNerveBoost;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 액티브 바이오닉: 신경 부스터 (머리 부위)
export class SkillNerveBoost : public SkillBehavior
{
public:
	SkillNerveBoost()
	{
		name = L"Nerve Boost";
		iconIndex = 116;
		descript = L"Activates an implanted neural booster, temporarily enhancing reaction speed.";
		src = skillSrc::BIONIC;
		type = skillType::ACTIVE;
		skillRank = L"F";
		maxCooldown = 30.0f;
		energyPerAct = 20.0f;
	}

	int getSkillCode() const override { return 50; }

	// TODO: 효과 구현
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
