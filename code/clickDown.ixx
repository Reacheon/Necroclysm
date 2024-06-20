#include <SDL.h>

export module clickDown;

import std;
import globalVar;
import GUI;

export void clickDown()
{
	click = true;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = 0;
	dtClickStackStart = SDL_GetTicks();
	switch (inputType)
	{
		case input::mouse:
			clickDownPoint = { event.motion.x, event.motion.y };
			break;
		case input::touch:
			clickDownPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
			break;
	}
	prevMouseX4Motion = event.button.x;
	prevMouseY4Motion = event.button.y;

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickDownGUI();
}