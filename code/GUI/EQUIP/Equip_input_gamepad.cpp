#include <SDL3/SDL.h>
import Equip;
import globalVar;

void Equip::gamepadBtnDown() 
{
	switch (event.gbutton.button)
	{
	case SDL_GAMEPAD_BUTTON_EAST:
		executeTab();
		break;
	}
}
void Equip::gamepadBtnMotion() { }
void Equip::gamepadBtnUp() { }