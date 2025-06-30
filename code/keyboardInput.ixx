export module keyboardInput;

import std;
import globalVar;
import GUI;

export void keyboardBtnDown()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->keyDownGUI();
}

export void keyboardBtnUp()
{
    GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->keyUpGUI();
}