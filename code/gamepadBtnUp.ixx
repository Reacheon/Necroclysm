export module gamepadBtnUp;

import std;
import globalVar;
import GUI;

export void gamepadBtnUp()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->gamepadBtnUp();
}