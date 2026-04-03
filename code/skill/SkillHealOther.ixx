export module SkillHealOther;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;
import Player;
import CoordSelect;
import log;
import GodService;
import turnWait;

export class SkillHealOther : public SkillBehavior
{
	static constexpr int PIETY_COST_MIN = 2;
	static constexpr int PIETY_COST_MAX = 3;
	static constexpr int HEAL_AMOUNT = 20;
	static constexpr int UNDEAD_DAMAGE = 20;

public:
	SkillHealOther()
	{
		name = L"Heal Other";
		iconIndex = 6;
		descript = L"Heal target's HP. Deals damage to undead instead.";
		src = skillSrc::MAGIC;
		type = skillType::ACTIVE;
		maxCooldown = 0.0f;
		reqProfic = { proficFlag::invocations };
		skillRank = L"E";
	}

	int getSkillCode() const override { return 41; }

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
		int cx = caster->getGridX();
		int cy = caster->getGridY();
		int cz = caster->getGridZ();

		rangeSet.clear();
		for (int dy = -1; dy <= 1; dy++)
		{
			for (int dx = -1; dx <= 1; dx++)
			{
				if (dx == 0 && dy == 0) continue;
				if (TileEntity(cx + dx, cy + dy, cz) != nullptr)
				{
					rangeSet.insert({ cx + dx, cy + dy });
				}
			}
		}

		if (rangeSet.empty())
		{
			updateLog(L"No target nearby.");
			currentUsingSkill = -1;
			co_return;
		}

		new CoordSelect(CoordSelectFlag::SINGLE_TARGET_SKILL, L"Select target to heal");
		co_await std::suspend_always();
		rangeSet.clear();
		if (coAnswer.empty()) { currentUsingSkill = -1; co_return; }

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		Entity* target = TileEntity(targetX, targetY, targetZ);
		if (target == nullptr) { currentUsingSkill = -1; co_return; }

		int cost = randomRange(PIETY_COST_MIN, PIETY_COST_MAX);
		GodService::changePiety(-cost);

		if (target->entityInfo.creature == creatureType::undead)
		{
			target->takeDamage(UNDEAD_DAMAGE, dmgFlag::fire);
			updateLog(L"The holy light burns the undead!");
		}
		else
		{
			int maxHP = target->entityInfo.maxHP;
			target->entityInfo.HP = myMin(maxHP, target->entityInfo.HP + HEAL_AMOUNT);
			updateLog(L"You channel healing light into the target.");
		}

		turnWait(1.0);
		currentUsingSkill = -1;
		co_return;
	}
};
