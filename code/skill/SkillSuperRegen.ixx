export module SkillSuperRegen;

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

export class SkillSuperRegen : public SkillBehavior
{
	static constexpr int PIETY_COST = 20;
	static constexpr float REGEN_DURATION = 50.0f;

public:
	SkillSuperRegen()
	{
		name = L"Super Regen";
		iconIndex = 4;
		descript = L"Grant super regeneration and status immunity for a period of time.";
		src = skillSrc::MAGIC;
		type = skillType::ACTIVE;
		maxCooldown = 0.0f;
	}

	int getSkillCode() const override { return 44; }

	bool canUse(Entity* caster, const SkillData& data) const override
	{
		if (godPiety < PIETY_COST)
		{
			updateLog(L"Not enough piety.");
			return false;
		}
		return true;
	}

	Corouter execute(Entity* caster, SkillData& data) override
	{
		GodService::changePiety(-PIETY_COST);

		auto& statusVec = caster->entityInfo.statusEffectVec;

		eraseStatusEffect(statusVec, statusEffectFlag::superRegen);
		statusVec.push_back({ statusEffectFlag::superRegen, REGEN_DURATION });

		eraseStatusEffect(statusVec, statusEffectFlag::immuneStatus);
		statusVec.push_back({ statusEffectFlag::immuneStatus, REGEN_DURATION });

		updateLog(L"Divine energy surges through your body!");
		turnWait(1.0);
		currentUsingSkill = -1;
		co_return;
	}
};
