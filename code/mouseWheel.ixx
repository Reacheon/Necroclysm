export module mouseWheel;

import std;
import util;
import globalVar;
import GUI;

export void mouseWheel()
{
	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->mouseWheel();
}


