export module SkillRatEars;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 쥐 귀 (fur색 반영)
export class SkillRatEars : public SkillBehavior
{
public:
	SkillRatEars()
	{
		id = L"MUT_RAT_EARS";
		name = sysStr[284];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_RAT_EARS";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
