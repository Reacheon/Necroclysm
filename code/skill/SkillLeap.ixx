export module SkillLeap;

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

export class SkillLeap : public SkillBehavior
{
public:
	SkillLeap()
	{
		name = L"Leap";
		iconIndex = 2;
		descript = L"Jump to any tile within 2 spaces. Consumes twice as much time as normal movement. ";
		src = skillSrc::GENERAL;
		type = skillType::ACTIVE;
		reqStat = L"DEX";
		reqProfic = { 1 };
	}

	int getSkillCode() const override { return 33; }

	bool canUse(Entity* caster, const SkillData& data) const override
	{
		int cx = caster->getGridX();
		int cy = caster->getGridY();
		int cz = caster->getGridZ();
		if (itemDex[TileFloor(cx, cy, cz)].checkFlag(itemFlag::WATER_SHALLOW) || itemDex[TileFloor(cx, cy, cz)].checkFlag(itemFlag::WATER_DEEP))
		{
			updateLog(L"You cannot leap in water.");
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
		for (int dx = -2; dx <= 2; dx++)
		{
			for (int dy = -2; dy <= 2; dy++)
			{
				if (dx == 0 && dy == 0) continue;
				if (TileFov(cx + dx, cy + dy, cz) == fovFlag::white)
				{
					if (isWalkable({ cx + dx, cy + dy, cz }))
					{
						rangeSet.insert({ cx + dx, cy + dy });
					}
				}
			}
		}

		new CoordSelect(CoordSelectFlag::SINGLE_TARGET_SKILL, sysStr[320]);
		co_await std::suspend_always();
		rangeSet.clear();
		if (coAnswer.empty()) { currentUsingSkill = -1; co_return; }

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());
		caster->setSkillTarget(targetX, targetY, targetZ);

		int prevGridX = caster->getGridX();
		int prevGridY = caster->getGridY();
		int dstGridX = caster->getSkillTarget().x;
		int dstGridY = caster->getSkillTarget().y;
		int dGridX = dstGridX - prevGridX;
		int dGridY = dstGridY - prevGridY;

		if (dGridX > 0) caster->setDirection(0);
		else if (dGridX < 0) caster->setDirection(4);

		caster->entityInfo.gridMoveSpd = 1.0;
		EntityPtrMove({ prevGridX, prevGridY, caster->getGridZ() }, { dstGridX, dstGridY, caster->getGridZ() });
		caster->setFakeX(-16 * dGridX);
		caster->setFakeY(-16 * dGridY);

		if (caster == static_cast<Entity*>(PlayerPtr))
		{
			cameraFix = false;
			cameraX = caster->getX() + caster->getIntegerFakeX();
			cameraY = caster->getY() + caster->getIntegerFakeY();
		}

		addAniToPlayerTurn(caster, aniFlag::leap);
		currentUsingSkill = -1;
	}
};
