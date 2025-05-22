export module clickRight;

import std;
import util;
import checkCursor;
import globalVar;
import wrapVar;
import textureVar;
import Player;
import World;
import log;
import GUI;

export void clickRight()
{
	click = false;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = -1;

	switch (option::inputMethod)
	{
	case input::mouse:
		prt(L"마우스 클릭라이트 Up 입력 (%d,%d)\n", static_cast<int>(event.motion.x), static_cast<int>(event.motion.y));
		clickUpPoint = { static_cast<int>(event.motion.x), static_cast<int>(event.motion.y) };
		break;
	case input::touch:
		prt(L"터치 클릭라이트 Up 입력 (%d,%d)\n", (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH));
		clickUpPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
		break;
	}

	if (deactClickUp == false) { GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickRightGUI(); }
	else { deactClickUp = false; }

	itemListColorLock = false;

	if (option::inputMethod == input::touch)
	{
		event.tfinger.x = 0;
		event.tfinger.y = 0;
	}
}