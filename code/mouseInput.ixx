export module mouseInput;

import std;
import util;
import globalVar;
import wrapVar;
import GUI;

export void clickDown()
{
	click = true;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = 0;
	dtClickStackStart = getMilliTimer();
	clickStartTime = getMilliTimer();
	deactHold = false;

	if (option::inputMethod == input::mouse) clickDownPoint = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
	else if (option::inputMethod == input::touch) clickDownPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };

	prevMouseX4Motion = getMouseX();
	prevMouseY4Motion = getMouseY();

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickDownGUI();
}

export void clickHold()
{
	click = false;
	prt(L"1초 이상 눌렀다.\n");
	clickStartTime = std::numeric_limits<__int64>::max();
	deactClickUp = true;

	if (option::inputMethod == input::mouse) clickHoldPoint = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
	else if (option::inputMethod == input::touch) clickHoldPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickHoldGUI();
}


export void clickMotion()
{
	int dx, dy = 0;
	if (option::inputMethod == input::mouse)
	{
		dxClickStack += event.motion.xrel;
		dyClickStack += event.motion.yrel;
		dx = clickDownPoint.x - getMouseX();
		dy = clickDownPoint.y - getMouseY();
	}
	else if (option::inputMethod == input::touch)
	{
		dxClickStack += (int)(event.tfinger.dx * cameraW);
		dyClickStack += (int)(event.tfinger.dy * cameraH);
		dx = clickDownPoint.x - (int)(event.tfinger.x * cameraW);
		dy = clickDownPoint.y - (int)(event.tfinger.y * cameraH);
	}

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickMotionGUI(dx, dy);

	if (click)
	{
		prevMouseX4Motion = getMouseX();
		prevMouseY4Motion = getMouseY();
	}
}

export void clickRight()
{
	click = false;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = -1;


	switch (option::inputMethod)
	{
	case input::mouse:
		prt(L"마우스 클릭라이트 Up 입력 (%d,%d)\n", static_cast<int>(getMouseX()), static_cast<int>(getMouseY()));
		clickUpPoint = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
		break;
	case input::touch:
		prt(L"터치 클릭라이트 Up 입력 (%d,%d)\n", (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH));
		clickUpPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };
		break;
	}

	if (deactClickUp == false) { GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickRightGUI(); }
	else { deactClickUp = false; }

	itemListColorLock = false;

	if (option::inputMethod == input::touch)
	{
		event.tfinger.x = 0;
		event.tfinger.y = 0;
	}
}

export void clickUp()
{
	click = false;
	dxClickStack = 0;
	dyClickStack = 0;
	dtClickStack = -1;
	clickStartTime = std::numeric_limits<__int64>::max();

	if (option::inputMethod == input::mouse) clickUpPoint = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
	else if (option::inputMethod == input::touch) clickUpPoint = { (int)(event.tfinger.x * cameraW), (int)(event.tfinger.y * cameraH) };

	if (deactClickUp == false) { GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickUpGUI(); }
	else { deactClickUp = false; }

	itemListColorLock = false;

	if (option::inputMethod == input::touch)
	{
		event.tfinger.x = 0;
		event.tfinger.y = 0;
	}
}

export void mouseWheel()
{
	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->mouseWheel();
}
