export module SkillInfravision;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 패시브 돌연변이: 적외선 시각 (머리 부위)
export class SkillInfravision : public SkillBehavior
{
public:
	SkillInfravision()
	{
		id = L"MUT_INFRAVISION";
		name = sysStr[279];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;
	}

	// 패시브 스킬이므로 execute는 빈 코루틴
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}

	// TODO: 적외선 시각 효과 구현
	// void onTurnTick(Entity* caster, const SkillData& data) override { }
	// void modifyStats(Entity* caster, const SkillData& data) override { }
};
