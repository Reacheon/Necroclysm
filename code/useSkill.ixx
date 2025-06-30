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
	const int SKILL_MAX_RANGE = 30;
	currentUsingSkill = skillCode;
	switch (skillCode)
	{
	default:
		prt(L"[Entity:useSkill] 플레이어가 알 수 없는 스킬을 시전하였다.\n");
		break;
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
		new CoordSelect(CoordSelectFlag::FIRESTORM, L"화염폭풍을 시전할 위치를 입력해주세요.", coordList);
		co_await std::suspend_always();
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
		std::vector<std::array<int, 2>> coordList;
		for (int i = 0; i < 8; i++)
		{
			int dx , dy;
			dir2Coord(i, dx, dy);
			if (TileFov(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == fovFlag::white)
			{
				coordList.push_back({ PlayerX() + dx, PlayerY() + dy });
            }
		}
		new CoordSelect(CoordSelectFlag::SINGLE_TARGET_SKILL, L"구르기를 시전할 위치를 입력해주세요.", coordList);
		co_await std::suspend_always();
		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());
		PlayerPtr->setSkillTarget(targetX, targetY, targetZ);

		break;
	}
	}
	currentUsingSkill = -1;
}