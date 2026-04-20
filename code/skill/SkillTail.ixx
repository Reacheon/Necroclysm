export module SkillTail;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 꼬리 (fur색 반영)
export class SkillTail : public SkillBehavior
{
public:
	SkillTail()
	{
		id = L"MUT_TAIL";
		name = L"Tail";
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::underEyes;
		mutSprBaseName = L"MUT_TAIL";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
