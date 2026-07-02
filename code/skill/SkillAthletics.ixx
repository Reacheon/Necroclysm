export module SkillAthletics;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 체술: 수영/등반/달리기 등 신체 능력 전반의 숙련.
// Roll/Leap 등 체술 계열 액티브 스킬의 참조 스킬로 쓰여 실패율을 보정한다.
export class SkillAthletics : public SkillBehavior
{
public:
	SkillAthletics()
	{
		id = L"SKILL_ATHLETICS";
		name = L"Athletics";
		iconIndex = 13;
		descript = L"Overall physical conditioning. Improves swimming, climbing and running, and supports acrobatic skills such as Roll and Leap.";
		src = skillSrc::GENERAL;
		category = skillCategory::survival;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
