export module SkillPurify;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;
import Player;
import log;
import GodService;
import statusEffect;
import turnWait;

export class SkillPurify : public SkillBehavior
{
	static constexpr int PIETY_COST_MIN = 2;
	static constexpr int PIETY_COST_MAX = 3;
	static constexpr float IMMUNE_DURATION = 5.0f;

public:
	SkillPurify()
	{
		id = L"SKILL_PURIFY";
		name = L"Purify";
		iconIndex = 8;
		descript = L"Cure all negative status effects and grant status immunity for a short time.";
		src = skillSrc::MAGIC;
		type = skillType::ACTIVE;
		maxCooldown = 0.0f;
		skillRank = L"F";
	}

	bool canUse(Entity* caster, const SkillData& data) const override
	{
		if (godPiety < PIETY_COST_MIN)
		{
			updateLog(L"Not enough piety.");
			return false;
		}
		return true;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		int cost = randomRange(PIETY_COST_MIN, PIETY_COST_MAX);
		GodService::changePiety(-cost);

		auto& statusVec = caster->entityInfo.statusEffectVec;
		eraseStatusEffect(statusVec, statusEffectFlag::confused);
		eraseStatusEffect(statusVec, statusEffectFlag::bleeding);
		eraseStatusEffect(statusVec, statusEffectFlag::blind);

		eraseStatusEffect(statusVec, statusEffectFlag::immuneStatus);
		statusVec.push_back({ statusEffectFlag::immuneStatus, IMMUNE_DURATION });

		updateLog(L"A purifying light cleanses your body.");
		turnWait(1.0);
		currentUsingSkill.clear();
		co_return;
	}
};
