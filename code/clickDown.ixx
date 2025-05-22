export module clickDown;

import std;
import util;
import globalVar;
import GUI;

export void clickDown()
{
	click = true;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = 0;
	dtClickStackStart = getMilliTimer();
	clickStartTime = getMilliTimer();
	deactHold = false;

	switch (option::inputMethod)
	{
		case input::mouse:
			clickDownPoint = { static_cast<int>(event.motion.x), static_cast<int>(event.motion.y) };
			break;
		case input::touch:
			clickDownPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
			break;
	}
	prevMouseX4Motion = event.button.x;
	prevMouseY4Motion = event.button.y;

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickDownGUI();
}