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
import mouseGrid;
import Entity;
import Aim;
import CoordSelect;

export Corouter useSkill(int skillCode)
{
	const int SKILL_MAX_RANGE = 30;
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

		Player::ins()->setSkillTarget(targetX, targetY, targetZ);
		addAniUSetPlayer(Player::ins(), aniFlag::fireStorm);
		break;
	}
	}
}