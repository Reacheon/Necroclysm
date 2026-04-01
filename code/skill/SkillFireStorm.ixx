export module SkillFireStorm;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;
import CoordSelect;

export class SkillFireStorm : public SkillBehavior
{
public:
	SkillFireStorm()
	{
		name = L"Firestorm";
		iconIndex = 3;
		descript = L"스킬 설명";
		src = skillSrc::MAGIC;
		type = skillType::ACTIVE;
		reqStat = L"INT";
		skillRank = L"F";
	}

	int getSkillCode() const override { return 30; }

	Corouter execute(Entity* caster, SkillData& data) override
	{
		const int SKILL_MAX_RANGE = 30;

		std::vector<Point2> coordList;
		auto [cx, cy, cz] = caster->getGrid();
		for (int tgtY = -SKILL_MAX_RANGE; tgtY <= SKILL_MAX_RANGE; tgtY++)
		{
			for (int tgtX = -SKILL_MAX_RANGE; tgtX <= SKILL_MAX_RANGE; tgtX++)
			{
				if (TileFov(cx + tgtX, cy + tgtY, cz) == fovFlag::white)
				{
					coordList.push_back({ cx + tgtX, cy + tgtY });
				}
			}
		}
		new CoordSelect(CoordSelectFlag::FIRESTORM, sysStr[321], coordList);
		co_await std::suspend_always();
		if (coAnswer.empty()) { currentUsingSkill = -1; co_return; }

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		caster->setSkillTarget(targetX, targetY, targetZ);
		addAniToPlayerTurn(caster, aniFlag::fireStorm);
		currentUsingSkill = -1;
	}
};
