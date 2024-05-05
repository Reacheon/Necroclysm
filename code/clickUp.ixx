export module clickUp;

import std;
import checkCursor;
import globalVar;
import textureVar;
import Player;
import World;
import drawStadium;
import log;
import GUI;

export void clickUp()
{
	click = false;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = -1;

	switch (inputType)
	{
		case input::mouse:
			//prt(L"마우스 Up 입력 (%d,%d)\n", event.motion.x, event.motion.y);
			clickUpPoint = { event.motion.x, event.motion.y };
			break;
		case input::touch:
			//prt(L"터치 Up 입력 (%d,%d)\n", (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH));
			clickUpPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
			break;
	}


	if (deactClickUp == false) { GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickUpGUI(); }
	else { deactClickUp = false; }

	cursorMotionLock = false;

	if (inputType == input::touch)
	{
		event.tfinger.x = 0;
		event.tfinger.y = 0;
	}
}