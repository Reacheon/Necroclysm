#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

#include <SDL.h>

import HUD;

import std;
import util;
import checkCursor;
import globalVar;
import Player;
import World;
import Vehicle;
import log;
import Prop;
import ContextMenu;
import mouseGrid;
import Entity;
import Aim;


void HUD::clickDownGUI()
{
	executedHold = false;
}
void HUD::clickMotionGUI(int dx, int dy)
{
	if (click == true && (event.button.button == SDL_BUTTON_MIDDLE||inputType == input::touch) )
	{
		const int maxDist = 160;
		int prevCameraX = cameraX, prevCameraY = cameraY;
		cameraFix = false;
		cameraX -= ((event.motion.x - prevMouseX4Motion) / 2);
		cameraY -= ((event.motion.y - prevMouseY4Motion) / 2);
		disableClickUp4Motion = true;

		if (std::abs(Player::ins()->getX() - cameraX) > maxDist) cameraX = prevCameraX;
		if (std::abs(Player::ins()->getY() - cameraY) > maxDist) cameraY = prevCameraY;
	}
}
void HUD::clickUpGUI()
{
	if (disableClickUp4Motion && (event.button.button == SDL_BUTTON_MIDDLE || inputType == input::touch))
	{
		disableClickUp4Motion = false;
		return;
	}

	if (executedHold) return;

	if (checkCursor(&letterboxPopUpButton) == true)//�˾� ���
	{
		if (y == 0) { executePopUp(); }
		else { executePopDown(); }
	}
	else if (checkCursor(&tabSmallBox) == true)
	{
		//�÷��̾��� �þ߿� �ִ� ��� ��ü���� üũ
		std::array<int, 2> nearCoord = { 0,0 };//�����ǥ
		int playerX = Player::ins()->getGridX();
		int playerY = Player::ins()->getGridY();
		int playerZ = Player::ins()->getGridZ();
		for (int i = playerX - DARK_VISION_HALF_W; i <= playerX + DARK_VISION_HALF_W; i++)
		{
			for (int j = playerY - DARK_VISION_HALF_H; j <= playerY + DARK_VISION_HALF_H; j++)
			{
				if (World::ins()->getTile(i, j, playerZ).fov == fovFlag::white)
				{
					//���� Ÿ���̰ų� �÷��̾� ��ü�� ������
					if (World::ins()->getTile(i, j, playerZ).EntityPtr != nullptr && World::ins()->getTile(i, j, playerZ).EntityPtr != Player::ins())
					{
						if (std::sqrt(pow(i - playerX, 2) + pow(j - playerY, 2)) < std::sqrt(pow(nearCoord[0], 2) + pow(nearCoord[1], 2)) || (nearCoord[0] == 0 && nearCoord[1] == 0))//����
						{
							nearCoord = { i - playerX, j - playerY };
						}
					}
				}
			}
		}

		if (nearCoord[0] == 0 && nearCoord[1] == 0)//ã�� ������ ���
		{
			updateLog(col2Str(col::white) + sysStr[105]);
		}
		else//ã���� ���
		{
			new Aim();
		}
	}
	else if (checkCursor(&tab) == true) executeTab();
	else if (checkCursor(&quickSlotPopBtn)) quickSlotToggle();
	else if (checkCursor(&letterbox))
	{
		for (int i = 0; i < barAct.size(); i++) // �ϴ� UI ��ġ �̺�Ʈ
		{
			if (checkCursor(&barButton[i]))
			{
				clickLetterboxBtn(barAct[i]);
			}
		}
	}
	else if (checkCursor(&quickSlotRegion))//������ �̺�Ʈ ��ġ
	{
		for (int i = 0; i < 8; i++)
		{
			if (checkCursor(&quickSlotBtn[i]))
			{
				prt(L"%d�� ��ų ������ ������!\n", i + 1);
				CORO(useSkill(quickSlot[i].second));
			}
		}
	}
	else//Ÿ����ġ
	{
		cameraFix = true;
		//��ġ�� ��ǥ�� ���� �κ�
		int cameraGridX, cameraGridY;
		if (cameraX >= 0) cameraGridX = cameraX / 16;
		else cameraGridX = -1 + cameraX / 16;
		if (cameraY >= 0) cameraGridY = cameraY / 16;
		else cameraGridY = -1 + cameraY / 16;

		int camDelX = cameraX - (16 * cameraGridX + 8);
		int camDelY = cameraY - (16 * cameraGridY + 8);

		int revX, revY, revGridX, revGridY;
		if (inputType == input::touch)
		{
			revX = event.tfinger.x * cameraW - (cameraW / 2);
			revY = event.tfinger.y * cameraH - (cameraH / 2);
		}
		else
		{
			revX = event.motion.x - (cameraW / 2);
			revY = event.motion.y - (cameraH / 2);
		}
		revX += sgn(revX) * (8 * zoomScale) + camDelX;
		revGridX = revX / (16 * zoomScale);
		revY += sgn(revY) * (8 * zoomScale) + camDelY;
		revGridY = revY / (16 * zoomScale);

		//�����ǥ�� ������ǥ�� ��ȯ
		clickTile.x = cameraGridX + revGridX;
		clickTile.y = cameraGridY + revGridY;
		prt(L"[HUD] ������ǥ (%d,%d) Ÿ���� ��ġ�ߴ�.\n", clickTile.x, clickTile.y);
		tileTouch(clickTile.x, clickTile.y);
	}
}

void HUD::mouseStep()
{
	//Ȧ�� �̺�Ʈ
	if (dtClickStack >= 1000) //1�ʰ� ������ ���� ���
	{
		if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false && checkCursor(&tabSmallBox) == false)
		{
			//��ġ�� ��ǥ�� ���� �κ�
			// prt(L"1�� �̻� ������.\n");
			int revX, revY, revGridX, revGridY;
			revX = clickDownPoint.x - (cameraW / 2);
			//revY = clickDownPoint.y - (cameraH / 2);
			//revX += sgn(revX) * (8 * zoomScale);
			//revGridX = revX / (16 * zoomScale);
			//revY += sgn(revY) * (8 * zoomScale);
			//revGridY = revY / (16 * zoomScale);
			//dtClickStack = -1;
			//executedHold = true;
			////advancedTileTouch(Player::ins()->getGridX() + revGridX, Player::ins()->getGridY() + revGridY);
		}
	}
}

void HUD::clickRightGUI()
{
	updateLog(L"#FFFFFF[HUD] Right click event triggered.");
	/*if(inputType==input::mouse) */openContextMenu(getAbsMouseGrid());
}
void HUD::clickHoldGUI()
{
	if (inputType == input::touch)
	{
		updateLog(L"#FFFFFF[HUD] Touch hold event triggered.");
		openContextMenu(getAbsMouseGrid());
	}
}