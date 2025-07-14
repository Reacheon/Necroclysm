export module useSkill;

import std;
import util;
import checkCursor;
import globalVar;
import wrapVar;
import Player;
import World;
import Vehicle;
import log;
import Prop;
import ContextMenu;
import Entity;
import Aim;
import CoordSelect;

export Corouter useSkill(int skillCode)
{
	PlayerPtr->deactAStarDst();
	if (turnCycle != turn::playerInput) co_return;

	const int SKILL_MAX_RANGE = 30;
	currentUsingSkill = skillCode;
	switch (skillCode)
	{
	default:
	{
		std::wstring errorMsg = replaceStr(L"Player used an unknown skill: %d", L"%d", std::to_wstring(skillCode));
		errorBox(errorMsg);
		break;
	}
	case 0:
		break;
	case 1:
		break;
	case 30://화염폭풍
	{
		std::vector<std::array<int, 2>> coordList;
		for (int tgtY = -SKILL_MAX_RANGE; tgtY <= SKILL_MAX_RANGE; tgtY++)
		{
			for (int tgtX = -SKILL_MAX_RANGE; tgtX <= SKILL_MAX_RANGE; tgtX++)
			{
				if (TileFov(PlayerX() + tgtX, PlayerY() + tgtY, PlayerZ()) == fovFlag::white)
				{
					coordList.push_back({ PlayerX() + tgtX, PlayerY() + tgtY });

				}
			}
		}
		new CoordSelect(CoordSelectFlag::FIRESTORM, sysStr[321], coordList);
		co_await std::suspend_always();
		if (coAnswer.empty()) co_return;
		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		PlayerPtr->setSkillTarget(targetX, targetY, targetZ);
		addAniUSetPlayer(PlayerPtr, aniFlag::fireStorm);
		break;
	}
	case 32://구르기
	{
		if (itemDex[TileFloor(PlayerX(), PlayerY(), PlayerZ())].checkFlag(itemFlag::WATER_SHALLOW) || itemDex[TileFloor(PlayerX(), PlayerY(), PlayerZ())].checkFlag(itemFlag::WATER_DEEP))
		{
			updateLog(L"You cannot roll in water.");
			currentUsingSkill = -1;
			co_return;
		}

		rangeSet.clear();
		for (int i = 0; i < 8; i++)
		{
			int dx , dy;
			dir2Coord(i, dx, dy);
			if (TileFov(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == fovFlag::white)
			{
				if (isWalkable({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }))
				{
					rangeSet.insert({ PlayerX() + dx, PlayerY() + dy });
				}
			}
		}
		new CoordSelect(CoordSelectFlag::SINGLE_TARGET_SKILL, sysStr[319]);//구를 타일을 선택해주세요.
		co_await std::suspend_always();
		rangeSet.clear();
		if (coAnswer.empty()) co_return;

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());
		PlayerPtr->setSkillTarget(targetX, targetY, targetZ);

		int prevGridX = PlayerPtr->getGridX();
		int prevGridY = PlayerPtr->getGridY();
		int dstGridX = PlayerPtr->getSkillTarget().x;
		int dstGridY = PlayerPtr->getSkillTarget().y;
		int dGridX = dstGridX - prevGridX;
		int dGridY = dstGridY - prevGridY;

		if (dGridX > 0) PlayerPtr->setDirection(0);
		else if (dGridX < 0) PlayerPtr->setDirection(4);

		PlayerPtr->entityInfo.gridMoveSpd = 1.0;
		EntityPtrMove({ prevGridX,prevGridY, PlayerPtr->getGridZ() }, { dstGridX, dstGridY, PlayerPtr->getGridZ() });
		PlayerPtr->setFakeX(-16 * dGridX);
		PlayerPtr->setFakeY(-16 * dGridY);

		cameraFix = false;
		cameraX = PlayerPtr->getX() + PlayerPtr->getIntegerFakeX();
		cameraY = PlayerPtr->getY() + PlayerPtr->getIntegerFakeY();

		addAniUSetPlayer(PlayerPtr, aniFlag::roll);
		break;
	}

	case 33://점프
	{
		if (itemDex[TileFloor(PlayerX(), PlayerY(), PlayerZ())].checkFlag(itemFlag::WATER_SHALLOW) || itemDex[TileFloor(PlayerX(), PlayerY(), PlayerZ())].checkFlag(itemFlag::WATER_DEEP))
		{
			updateLog(L"You cannot leap in water.");
			currentUsingSkill = -1;
			co_return;
		}

		rangeSet.clear();
		for (int dx = -2; dx <= 2; dx++)
		{
			for (int dy = -2; dy <= 2; dy++)
			{
				if (dx == 0 && dy == 0) continue;
				if (TileFov(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == fovFlag::white)
				{
					if (isWalkable({ PlayerX() + dx, PlayerY() + dy, PlayerZ() }))
					{
						rangeSet.insert({ PlayerX() + dx, PlayerY() + dy });
					}
				}
			}
		}

		new CoordSelect(CoordSelectFlag::SINGLE_TARGET_SKILL, sysStr[320]);//도약할 타일을 선택해주세요.
		co_await std::suspend_always();
		rangeSet.clear();
		if (coAnswer.empty()) co_return;

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());
		PlayerPtr->setSkillTarget(targetX, targetY, targetZ);

		int prevGridX = PlayerPtr->getGridX();
		int prevGridY = PlayerPtr->getGridY();
		int dstGridX = PlayerPtr->getSkillTarget().x;
		int dstGridY = PlayerPtr->getSkillTarget().y;
		int dGridX = dstGridX - prevGridX;
		int dGridY = dstGridY - prevGridY;

		if (dGridX > 0) PlayerPtr->setDirection(0);
		else if (dGridX < 0) PlayerPtr->setDirection(4);

		PlayerPtr->entityInfo.gridMoveSpd = 1.0;
		EntityPtrMove({ prevGridX,prevGridY, PlayerPtr->getGridZ() }, { dstGridX, dstGridY, PlayerPtr->getGridZ() });
		PlayerPtr->setFakeX(-16 * dGridX);
		PlayerPtr->setFakeY(-16 * dGridY);

		cameraFix = false;
		cameraX = PlayerPtr->getX() + PlayerPtr->getIntegerFakeX();
		cameraY = PlayerPtr->getY() + PlayerPtr->getIntegerFakeY();

		addAniUSetPlayer(PlayerPtr, aniFlag::leap);
		break;
	}
	}
	currentUsingSkill = -1;
}