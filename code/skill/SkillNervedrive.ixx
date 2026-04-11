export module SkillNervedrive;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 액티브 바이오닉: 신경 가속 (머리 부위)
export class SkillNervedrive : public SkillBehavior
{
public:
	SkillNervedrive()
	{
		name = L"Nervedrive";
		iconIndex = 10;
		descript = L"";
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
