#include <SDL3/SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

import Craft;
import globalVar;

void Craft::gamepadBtnDown() 
{
	switch (event.gbutton.button)
	{
	case SDL_GAMEPAD_BUTTON_EAST:
		executeTab();
		break;
	}
}
void Craft::gamepadBtnMotion() {}
void Craft::gamepadBtnUp() {}