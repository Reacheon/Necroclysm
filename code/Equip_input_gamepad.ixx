#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

import Equip;

void Equip::gamepadBtnDown() { }
void Equip::gamepadBtnMotion() { }
void Equip::gamepadBtnUp() { }