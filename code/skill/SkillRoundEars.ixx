export module SkillRoundEars;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 둥근 귀 (너구리/곰/판다 등)
// fur색 적용. 색상별 PNG가 없으면 MUT_ROUND_EARS.png로 폴백.
export class SkillRoundEars : public SkillBehavior
{
public:
	SkillRoundEars()
	{
		id = L"MUT_ROUND_EARS";
		name = sysStr[286];
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_ROUND_EARS";
		mutColorSrc = mutColorSource::fur;
		mutDrawPriority = 20;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}
};
