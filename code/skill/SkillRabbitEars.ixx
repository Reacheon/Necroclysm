export module SkillRabbitEars;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 토끼귀 (fur색 반영)
export class SkillRabbitEars : public SkillBehavior
{
public:
	SkillRabbitEars()
	{
		name = L"Rabbit Ears";
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_RABBIT_EARS";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	int getSkillCode() const override { return 61; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
