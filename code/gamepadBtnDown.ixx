export module gamepadBtnDown;

import std;
import globalVar;
import GUI;

export void gamepadBtnDown()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->gamepadBtnDown();
}