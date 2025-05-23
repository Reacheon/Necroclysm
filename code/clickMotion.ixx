export module clickMotion;

import std;
import globalVar;
import wrapVar;
import textureVar;
import Player;
import World;
import checkCursor;
import ItemPocket;
import ItemStack;
import GUI;

export void clickMotion()
{
	switch (option::inputMethod)
	{
		case input::mouse:
			dxClickStack += event.motion.xrel;
			dyClickStack += event.motion.yrel;
			//prt(L"(%d,%d)가 현재 상대적 이동값\n",dxClickStack,dyClickStack);
			break;
		case input::touch:
			dxClickStack += (int)(event.tfinger.dx * cameraW);
			dyClickStack += (int)(event.tfinger.dy * cameraH);
			//prt(L"(%d,%d)가 현재 상대적 이동값\n", dxClickStack, dyClickStack);
			break;
	}

	// 입력 방식에 따라 커서 이동 델타값 조정
	int dx, dy = 0;
	switch (option::inputMethod)
	{
		case input::mouse:// 마우스 조작
			dx = clickDownPoint.x - getMouseX();
			dy = clickDownPoint.y - getMouseY();
			break;
		case input::touch:// 터치 조작
			dx = clickDownPoint.x - (int)(event.tfinger.x * cameraW);
			dy = clickDownPoint.y - (int)(event.tfinger.y * cameraH);
			break;
	}

	GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1]->clickMotionGUI(dx,dy);

	if (click)
	{
		prevMouseX4Motion = getMouseX();
		prevMouseY4Motion = getMouseY();
	}
}