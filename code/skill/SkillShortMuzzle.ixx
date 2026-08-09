export module SkillShortMuzzle;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 짧은 주둥이 (fur색 반영)
export class SkillShortMuzzle : public SkillBehavior
{
public:
	SkillShortMuzzle()
	{
		id = L"MUT_SHORT_MUZZLE";
		name = sysStr[287];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_SHORT_MUZZLE";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 10;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
