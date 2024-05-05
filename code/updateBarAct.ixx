export module updateBarAct;

import util;
import constVar;
import globalVar;
import World;
import Install;

export void updateNearbyBarAct(int px, int py, int pz)
{
 	if (barAct[0] == act::status)
	{
		barAct = actSet::null;
		
		//플레이어 타일
		if (World::ins()->getTile(px, py, pz).InstallPtr != nullptr)
		{

		}

		//주변 타일
		for (int dir = 0; dir <= 7; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);

			if (World::ins()->getTile(px + dx, py + dy, pz).InstallPtr != nullptr)
			{
				Install* instlPtr = (Install*)World::ins()->getTile(px + dx, py + dy, pz).InstallPtr;
				if (instlPtr->leadItem.checkFlag(itemFlag::DOOR_OPEN))
				{
					if (std::find(barAct.begin(),barAct.end(),act::closeDoor) == barAct.end())
					{
						barAct.push_back(act::closeDoor);
					}
				}
			}
		}
	}
}