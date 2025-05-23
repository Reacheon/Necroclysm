export module clickHold;

import std;
import util;
import globalVar;
import GUI;
import wrapVar;

export void clickHold()
{
	click = false;
	prt(L"1초 이상 눌렀다.\n");
	clickStartTime = std::numeric_limits<__int64>::max();
	deactClickUp = true;

	switch (option::inputMethod)
	{
	case input::mouse:
		clickHoldPoint = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
		break;
	case input::touch:
		clickHoldPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
		break;
	}

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickHoldGUI();
}