export module SkillFocusHeal;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;
import Player;
import Lst;
import log;
import GodService;
import turnWait;

export class SkillFocusHeal : public SkillBehavior
{
	static constexpr int PIETY_COST_MIN = 9;
	static constexpr int PIETY_COST_MAX = 10;
	static constexpr int HEAL_AMOUNT = 50;

public:
	SkillFocusHeal()
	{
		id = L"SKILL_FOCUS_HEAL";
		name = L"Focused Heal";
		iconIndex = 7;
		descript = L"Greatly heal a specific body part of your choice.";
		src = skillSrc::MAGIC;
		type = skillType::ACTIVE;
		maxCooldown = 0.0f;
		reqProfic = { proficFlag::invocations };
		skillRank = L"C";
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
		new Lst(L"Focused Heal", L"Select body part to heal", {
			L"Head",
			L"Left Arm",
			L"Right Arm",
			L"Left Leg",
			L"Right Leg",
			L"Torso"
		});
		co_await std::suspend_always();
		if (coAnswer.empty()) { currentUsingSkill.clear(); co_return; }

		int selected = wtoi(coAnswer.c_str());
		int cost = randomRange(PIETY_COST_MIN, PIETY_COST_MAX);
		GodService::changePiety(-cost);

		int maxHP = caster->entityInfo.maxHP;

		if (caster == static_cast<Entity*>(PlayerPtr))
		{
			switch (selected)
			{
			case 0: PlayerPtr->headHP = myMin(maxHP, PlayerPtr->headHP + HEAL_AMOUNT); break;
			case 1: PlayerPtr->lArmHP = myMin(maxHP, PlayerPtr->lArmHP + HEAL_AMOUNT); break;
			case 2: PlayerPtr->rArmHP = myMin(maxHP, PlayerPtr->rArmHP + HEAL_AMOUNT); break;
			case 3: PlayerPtr->lLegHP = myMin(maxHP, PlayerPtr->lLegHP + HEAL_AMOUNT); break;
			case 4: PlayerPtr->rLegHP = myMin(maxHP, PlayerPtr->rLegHP + HEAL_AMOUNT); break;
			case 5: caster->entityInfo.HP = myMin(maxHP, caster->entityInfo.HP + HEAL_AMOUNT); break;
			}
		}

		updateLog(L"Concentrated healing light mends your body.");
		turnWait(1.0);
		currentUsingSkill.clear();
		co_return;
	}
};
