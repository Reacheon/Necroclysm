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
		id = L"BION_POWER_STORAGE";
		name = sysStr[273];
		iconIndex = 12;
		descript = L"";
		src = skillSrc::BIONIC;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;
	}

	// TODO: 효과 구현
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
