export module clickRight;

import std;
import util;
import checkCursor;
import globalVar;
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

	switch (inputType)
	{
	case input::mouse:
		prt(L"마우스 클릭라이트 Up 입력 (%d,%d)\n", event.motion.x, event.motion.y);
		clickUpPoint = { event.motion.x, event.motion.y };
		break;
	case input::touch:
		prt(L"터치 클릭라이트 Up 입력 (%d,%d)\n", (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH));
		clickUpPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
		break;
	}

	if (deactClickUp == false) { GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickRightGUI(); }
	else { deactClickUp = false; }

	itemListColorLock = false;

	if (inputType == input::touch)
	{
		event.tfinger.x = 0;
		event.tfinger.y = 0;
	}
}