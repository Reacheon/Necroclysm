export module clickHold;

import std;
import util;
import globalVar;
import GUI;

export void clickHold()
{
	click = false;
	prt(L"1초 이상 눌렀다.\n");
	clickStartTime = std::numeric_limits<__int64>::max();
	deactClickUp = true;

	switch (option::inputMethod)
	{
	case input::mouse:
		clickHoldPoint = { event.motion.x, event.motion.y };
		break;
	case input::touch:
		clickHoldPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
		break;
	}

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickHoldGUI();
}