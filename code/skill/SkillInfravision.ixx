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
		name = L"Infravision";
		iconIndex = 117;
		descript = L"Mutated eyes grant the ability to perceive heat signatures, revealing warm-blooded creatures in darkness.";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		bodyPart = humanPartFlag::head;
		maxCooldown = 0.0f;
	}

	int getSkillCode() const override { return 51; }

	// 패시브 스킬이므로 execute는 빈 코루틴
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}

	// TODO: 적외선 시각 효과 구현
	// void onTurnTick(Entity* caster, const SkillData& data) override { }
	// void modifyStats(Entity* caster, const SkillData& data) override { }
};
