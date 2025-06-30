export module gamepadInput;

import std;
import globalVar;
import GUI;

export void gamepadBtnDown()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->gamepadBtnDown();
}

export void gamepadBtnMotion()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->gamepadBtnMotion();
}

export void gamepadBtnUp()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->gamepadBtnUp();
}