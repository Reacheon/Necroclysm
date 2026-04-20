export module SkillRabbitTail;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 토끼꼬리 (fur색 반영)
export class SkillRabbitTail : public SkillBehavior
{
public:
	SkillRabbitTail()
	{
		id = L"MUT_RABBIT_TAIL";
		name = L"Rabbit Tail";
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::underEyes;
		mutSprBaseName = L"MUT_RABBIT_TAIL";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
