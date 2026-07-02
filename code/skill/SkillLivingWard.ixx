export module SkillLivingWard;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

// 패시브: 신앙도(Piety)에 비례하여 언데드로부터 받는 피해 감소
export class SkillLivingWard : public SkillBehavior
{
public:
	SkillLivingWard()
	{
		id = L"SKILL_LIVING_WARD";
		name = L"Living Ward";
		iconIndex = 9;
		descript = L"A divine ward that reduces damage taken from undead, scaling with your piety.";
		src = skillSrc::MAGIC;
		type = skillType::PASSIVE;
		maxCooldown = 0.0f;
		skillRank = L"F";
	}

	// 패시브 스킬이므로 execute는 빈 코루틴
	Corouter execute(Entity* caster, SkillData& data) override
	{
		currentUsingSkill.clear();
		co_return;
	}

	// TODO: 언데드 피해 감소 로직 구현
	// void onTurnTick(Entity* caster, const SkillData& data) override { }
	// void modifyStats(Entity* caster, const SkillData& data) override { }
};
