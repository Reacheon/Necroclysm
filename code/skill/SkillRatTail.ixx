export module SkillRatTail;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 쥐꼬리 (fur색과 무관, 항상 핑크)
export class SkillRatTail : public SkillBehavior
{
public:
	SkillRatTail()
	{
		name = L"Rat Tail";
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::underEyes;
		mutSprBaseName = L"MUT_RAT_TAIL";
		mutColorSrc = mutColorSource::none; //항상 핑크
		mutDrawPriority = 20;
	}

	int getSkillCode() const override { return 55; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
