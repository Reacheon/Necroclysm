export module SkillEyeMask;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 외형 돌연변이: 눈가 검은 마스크 (너구리/판다/오소리 등)
// 눈부심 방지 효과. 색상 고정(항상 검정).
export class SkillEyeMask : public SkillBehavior
{
public:
	SkillEyeMask()
	{
		name = L"Eye Mask";
		iconIndex = 117;
		descript = L"";
		src = skillSrc::MUTATION;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;

		mutLayer = mutDrawLayer::aboveEquip;
		mutSprBaseName = L"MUT_EYE_MASK";
		mutColorSrc = mutColorSource::none;
		mutDrawPriority = 15; //주둥이(10) 위, 귀(20) 아래
	}

	int getSkillCode() const override { return 65; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill = -1;
		co_return;
	}
};
