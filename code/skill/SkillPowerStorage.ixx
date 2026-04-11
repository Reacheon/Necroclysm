export module SkillPowerStorage;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 패시브 바이오닉: 내장 전력 저장소 (몸통 부위)
export class SkillPowerStorage : public SkillBehavior
{
public:
	SkillPowerStorage()
	{
		name = L"Power Storage";
		iconIndex = 12;
		descript = L"";
		src = skillSrc::BIONIC;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;
	}

	int getSkillCode() const override { return 52; }

	// TODO: 효과 구현
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
