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
		name = L"Short Muzzle";
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

	int getSkillCode() const override { return 57; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
