export module gamepadBtnMotion;

import std;
import globalVar;
import GUI;

export void gamepadBtnMotion()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->gamepadBtnMotion();
}