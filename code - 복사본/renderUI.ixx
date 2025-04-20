export module renderUI;

import std;
import util;
import globalVar;
import constVar;
import GUI;

export __int64 renderUI()
{
	__int64 timeStampStart = getNanoTimer();

	for (int i = 0; i < GUI::getActiveGUIList().size(); i++) { GUI::getActiveGUIList()[i]->drawGUI(); }

	return (getNanoTimer() - timeStampStart);
}