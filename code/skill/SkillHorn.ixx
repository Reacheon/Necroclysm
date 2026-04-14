export module SkillHorn;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 뿔 (fur와 독립된 hornColor 사용, 기본 BROWN)
export class SkillHorn : public SkillBehavior
{
public:
	SkillHorn()
	{
		name = L"Horn";
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_HORN";
		mutColorSrc = mutColorSource::horn;
		mutDrawPriority = 30; //귀(20)보다 위
	}

	int getSkillCode() const override { return 62; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
