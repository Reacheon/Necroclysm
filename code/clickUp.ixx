export module clickUp;

import std;
import checkCursor;
import globalVar;
import wrapVar;
import textureVar;
import Player;

import World;
import log;
import GUI;

export void clickUp()
{
	click = false;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = -1;
	clickStartTime = std::numeric_limits<__int64>::max();

	switch (option::inputMethod)
	{
		case input::mouse:
			//prt(L"마우스 Up 입력 (%d,%d)\n", event.motion.x, event.motion.y);
			clickUpPoint = { static_cast<int>(event.motion.x), static_cast<int>(event.motion.y) };
			break;
		case input::touch:
			//prt(L"터치 Up 입력 (%d,%d)\n", (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH));
			clickUpPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
			break;
	}

	if (deactClickUp == false) { GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickUpGUI(); }
	else { deactClickUp = false; }

	itemListColorLock = false;

	if (option::inputMethod == input::touch)
	{
		event.tfinger.x = 0;
		event.tfinger.y = 0;
	}
}