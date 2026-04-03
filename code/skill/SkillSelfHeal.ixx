export module SkillSelfHeal;

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
import turnWait;

export class SkillSelfHeal : public SkillBehavior
{
	static constexpr int PIETY_COST_MIN = 4;
	static constexpr int PIETY_COST_MAX = 5;
	static constexpr int HEAL_AMOUNT = 10;

public:
	SkillSelfHeal()
	{
		name = L"Self Heal";
		iconIndex = 5;
		descript = L"Instantly recover a small amount of HP for all body parts.";
		src = skillSrc::MAGIC;
		type = skillType::ACTIVE;
		maxCooldown = 0.0f;
		reqProfic = { proficFlag::invocations };
		skillRank = L"F";
	}

	int getSkillCode() const override { return 40; }

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

		int maxHP = caster->entityInfo.maxHP;

		if (caster == static_cast<Entity*>(PlayerPtr))
		{
			PlayerPtr->headHP = myMin(maxHP, PlayerPtr->headHP + HEAL_AMOUNT);
			PlayerPtr->lArmHP = myMin(maxHP, PlayerPtr->lArmHP + HEAL_AMOUNT);
			PlayerPtr->rArmHP = myMin(maxHP, PlayerPtr->rArmHP + HEAL_AMOUNT);
			PlayerPtr->lLegHP = myMin(maxHP, PlayerPtr->lLegHP + HEAL_AMOUNT);
			PlayerPtr->rLegHP = myMin(maxHP, PlayerPtr->rLegHP + HEAL_AMOUNT);
		}
		caster->entityInfo.HP = myMin(maxHP, caster->entityInfo.HP + HEAL_AMOUNT);

		updateLog(L"You feel a warm light healing your wounds.");
		turnWait(1.0);
		currentUsingSkill = -1;
		co_return;
	}
};
