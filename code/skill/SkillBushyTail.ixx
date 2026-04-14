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
		name = L"Bushy Tail";
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

	int getSkillCode() const override { return 64; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
