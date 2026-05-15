export module renderUI;

import std;
import util;
import globalVar;
import constVar;
import GUI;

export std::int64_t renderUI()
{
	std::int64_t timeStampStart = getNanoTimer();

	for (int i = 0; i < GUI::getActiveGUIList().size(); i++) { GUI::getActiveGUIList()[i]->drawGUI(); }

	return (getNanoTimer() - timeStampStart);
}