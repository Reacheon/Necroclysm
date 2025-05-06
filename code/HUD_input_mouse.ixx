﻿#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

#include <SDL.h>

import HUD;

import std;
import util;
import checkCursor;
import globalVar;
import wrapVar;
import Player;
import World;
import Vehicle;
import log;
import Prop;
import ContextMenu;
import mouseGrid;
import Entity;
import Aim;
import useSkill;


void HUD::clickDownGUI()
{
	executedHold = false;

	for (int i = 0; i < QUICK_SLOT_MAX; i++)
	{
		if (checkCursor(&quickSlotBtn[i]))
		{
			if (quickSlot[i].first != quickSlotFlag::NONE)
			{
				dragQuickSlotTarget = i;
			}
		}
	}
}
void HUD::clickMotionGUI(int dx, int dy)
{
	if (click == true && (event.button.button == SDL_BUTTON_MIDDLE||option::inputMethod == input::touch) )
	{
		const int maxDist = 160;
		int prevCameraX = cameraX, prevCameraY = cameraY;
		cameraFix = false;
		cameraX -= ((event.motion.x - prevMouseX4Motion) / 2);
		cameraY -= ((event.motion.y - prevMouseY4Motion) / 2);
		disableClickUp4Motion = true;

		if (std::abs(PlayerPtr->getX() - cameraX) > maxDist) cameraX = prevCameraX;
		if (std::abs(PlayerPtr->getY() - cameraY) > maxDist) cameraY = prevCameraY;
	}
}
void HUD::clickUpGUI()
{
	if (disableClickUp4Motion && (event.button.button == SDL_BUTTON_MIDDLE || option::inputMethod == input::touch))
	{
		disableClickUp4Motion = false;
		return;
	}

	if (executedHold) return;
	

	if (checkCursor(&letterboxPopUpButton) == true)//팝업 기능
	{
		if (y == 0) { executePopUp(); }
		else { executePopDown(); }
	}
	else if (checkCursor(&tabSmallBox) == true)
	{
		//플레이어의 시야에 있는 모든 객체들을 체크
		std::array<int, 2> nearCoord = { 0,0 };//상대좌표
		int playerX = PlayerX();
		int playerY = PlayerY();
		int playerZ = PlayerZ();
		for (int i = playerX - DARK_VISION_RADIUS; i <= playerX + DARK_VISION_RADIUS; i++)
		{
			for (int j = playerY - DARK_VISION_RADIUS; j <= playerY + DARK_VISION_RADIUS; j++)
			{
				if (TileFov(i, j, playerZ) == fovFlag::white)
				{
					//없는 타일이거나 플레이어 개체는 제외함
					if (TileEntity(i, j, playerZ) != nullptr && TileEntity(i, j, playerZ) != PlayerPtr)
					{
						if (std::sqrt(pow(i - playerX, 2) + pow(j - playerY, 2)) < std::sqrt(pow(nearCoord[0], 2) + pow(nearCoord[1], 2)) || (nearCoord[0] == 0 && nearCoord[1] == 0))//갱신
						{
							nearCoord = { i - playerX, j - playerY };
						}
					}
				}
			}
		}

		if (nearCoord[0] == 0 && nearCoord[1] == 0)//찾지 못했을 경우
		{
			updateLog(col2Str(col::white) + sysStr[105]);
		}
		else//찾았을 경우
		{
			new Aim();
		}
	}
	else if (checkCursor(&tab) == true) executeTab();
	else if (checkCursor(&quickSlotPopBtn)) quickSlotToggle();
	else if (checkCursor(&letterbox))
	{
		for (int i = 0; i < barAct.size(); i++) // 하단 UI 터치 이벤트
		{
			if (checkCursor(&barButton[i]))
			{
				clickLetterboxBtn(barAct[i]);
			}
		}
	}
	else if (checkCursor(&quickSlotRegion))//퀵슬롯 이벤트 터치
	{
		for (int i = 0; i < 8; i++)
		{
			if (checkCursor(&quickSlotBtn[i]))
			{
				prt(L"%d번 스킬 슬롯을 눌렀다!\n", i + 1);
				CORO(useSkill(quickSlot[i].second));
			}
		}
	}
	else//타일터치
	{
		if (dragQuickSlotTarget == -1)
		{
			cameraFix = true;
			//터치한 좌표를 얻어내는 부분
			int cameraGridX, cameraGridY;
			if (cameraX >= 0) cameraGridX = cameraX / 16;
			else cameraGridX = -1 + cameraX / 16;
			if (cameraY >= 0) cameraGridY = cameraY / 16;
			else cameraGridY = -1 + cameraY / 16;

			int camDelX = cameraX - (16 * cameraGridX + 8);
			int camDelY = cameraY - (16 * cameraGridY + 8);

			int revX, revY, revGridX, revGridY;
			if (option::inputMethod == input::touch)
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

			//상대좌표를 절대좌표로 변환
			clickTile.x = cameraGridX + revGridX;
			clickTile.y = cameraGridY + revGridY;
			prt(L"[HUD] 절대좌표 (%d,%d) 타일을 터치했다.\n", clickTile.x, clickTile.y);
			tileTouch(clickTile.x, clickTile.y);
		}
	}

	if (dragQuickSlotTarget != -1)
	{
		if (checkCursor(&quickSlotRegion) == false)
		{
			quickSlot[dragQuickSlotTarget].first = quickSlotFlag::NONE;
			quickSlot[dragQuickSlotTarget].second = -1;
		}
		else
		{
			for (int i = 0; i < QUICK_SLOT_MAX; i++)
			{
				if (checkCursor(&quickSlotBtn[i]))
				{
					quickSlotFlag prevFlag = quickSlot[dragQuickSlotTarget].first;
					int prevIndex = quickSlot[dragQuickSlotTarget].second;

					for (int j = 0; j < QUICK_SLOT_MAX; j++)
					{
						if (quickSlot[j].first == prevFlag && quickSlot[j].second == prevIndex)
						{
							quickSlot[j].first = quickSlotFlag::NONE;
							quickSlot[j].second = -1;
						}
					}

					quickSlot[i].first = prevFlag;
					quickSlot[i].second = prevIndex;
					break;
				}
			}
		}
	}

	dragQuickSlotTarget = -1;
}

void HUD::mouseStep()
{
	//홀드 이벤트
	if (dtClickStack >= 1000) //1초간 누르고 있을 경우
	{
		if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false && checkCursor(&tabSmallBox) == false)
		{
			//터치한 좌표를 얻어내는 부분
			// prt(L"1초 이상 눌렀다.\n");
			int revX, revY, revGridX, revGridY;
			revX = clickDownPoint.x - (cameraW / 2);
			//revY = clickDownPoint.y - (cameraH / 2);
			//revX += sgn(revX) * (8 * zoomScale);
			//revGridX = revX / (16 * zoomScale);
			//revY += sgn(revY) * (8 * zoomScale);
			//revGridY = revY / (16 * zoomScale);
			//dtClickStack = -1;
			//executedHold = true;
			////advancedTileTouch(PlayerX() + revGridX, PlayerY() + revGridY);
		}
	}
}

void HUD::clickRightGUI()
{
	updateLog(L"#FFFFFF[HUD] Right click event triggered.");

	if (checkCursor(&quickSlotRegion) == true)
	{
		for (int i = 0; i < QUICK_SLOT_MAX; i++)
		{
			if (checkCursor(&quickSlotBtn[i]))
			{
				quickSlot[i].first = quickSlotFlag::NONE;
				quickSlot[i].second = -1;
			}
		}
	}
	else
	{
		/*if(option::inputMethod==input::mouse) */openContextMenu(getAbsMouseGrid());
	}

	
}
void HUD::clickHoldGUI()
{
	if (option::inputMethod == input::touch)
	{
		updateLog(L"#FFFFFF[HUD] Touch hold event triggered.");
		openContextMenu(getAbsMouseGrid());
	}
}