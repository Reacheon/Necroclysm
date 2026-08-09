export module SkillFloppyEars;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 축 늘어진 귀 (fur색 반영)
export class SkillFloppyEars : public SkillBehavior
{
public:
	SkillFloppyEars()
	{
		id = L"MUT_FLOPPY_EARS";
		name = sysStr[276];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_FLOPPY_EARS";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
