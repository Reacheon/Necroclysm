export module SkillBushyTail;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 복슬복슬 꼬리 (여우/너구리/다람쥐 등, fur색 반영)
export class SkillBushyTail : public SkillBehavior
{
public:
	SkillBushyTail()
	{
		id = L"MUT_BUSHY_TAIL";
		name = sysStr[274];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::underEyes;
		mutSprBaseName = L"MUT_BUSHY_TAIL";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
