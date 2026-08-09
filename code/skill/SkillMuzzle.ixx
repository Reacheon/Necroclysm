export module SkillMuzzle;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 주둥이 (fur색 반영)
export class SkillMuzzle : public SkillBehavior
{
public:
	SkillMuzzle()
	{
		id = L"MUT_MUZZLE";
		name = sysStr[280];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_MUZZLE";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 10;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
