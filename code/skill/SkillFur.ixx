export module SkillFur;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 전신 털 (fur색 반영)
export class SkillFur : public SkillBehavior
{
public:
	SkillFur()
	{
		id = L"MUT_FUR";
		name = sysStr[277];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::underEyes;
		mutSprBaseName = L"MUT_FUR";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 10;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
