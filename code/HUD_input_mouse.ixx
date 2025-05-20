#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

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
import ItemData;
import ItemPocket;


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
			std::vector<ItemData>& equipInfo = PlayerPtr->getEquipPtr()->itemInfo;
			for (int i = 0; i < equipInfo.size(); i++)
			{
				if (equipInfo[i].equipState != equipHandFlag::none)
				{
					if (equipInfo[i].checkFlag(itemFlag::BOW))
					{
						if (equipInfo[i].pocketPtr.get()->getPocketNumber() > 0) new Aim();
						else
						{
							for (int j = 0; j < equipInfo.size(); j++)
							{
								if(equipInfo[j].itemCode == itemRefCode::arrowQuiver)
								{
									if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
									{
										equipInfo[j].pocketPtr.get()->transferItem(equipInfo[i].pocketPtr.get(), 0,1);
										updateLog(col2Str(col::white) + L"당신은 화살을 시위에 걸었다.");
										new Aim();
										break;
									}
                                }
								if(j== equipInfo.size()-1) updateLog(col2Str(col::white) + L"현재 가지고 있는 화살이 없다.");
							}
						}
					}
					else if (equipInfo[i].checkFlag(itemFlag::CROSSBOW))
					{
						if(equipInfo[i].pocketPtr.get()->getPocketNumber() > 0) new Aim();
						else
						{
							for (int j = 0; j < equipInfo.size(); j++)
							{
								if (equipInfo[j].itemCode == itemRefCode::boltQuiver)
								{
									if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
									{
										equipInfo[j].pocketPtr.get()->transferItem(equipInfo[i].pocketPtr.get(), 0, 1);
										updateLog(col2Str(col::white) + L"당신은 석궁에 볼트를 장전했다.");
										new Aim();
										break;
									}
								}
								if (j == equipInfo.size() - 1) updateLog(col2Str(col::white) + L"현재 가지고 있는 볼트가 없다.");
							}
						}
					}
					else if (equipInfo[i].checkFlag(itemFlag::GUN))
					{
						if (equipInfo[i].pocketOnlyItem.empty())
						{
							updateLog(col2Str(col::white) + L"이 총은 탄 정보를 찾을 수 없습니다.");
							break;
						}

						unsigned short onlyCode = equipInfo[i].pocketOnlyItem[0];
						/* ① 리볼버·산탄총처럼 ‘직장전식’  ------------------------------------ */
						if (itemDex[onlyCode].checkFlag(itemFlag::AMMO))
						{
							if (getBulletNumber(equipInfo[i]) > 0) new Aim();
							else updateLog(col2Str(col::white) + L"현재 장전된 탄이 없습니다.");

						}
						/* ② 탄창식( MAGAZINE )  ------------------------------------------------ */
						else if (itemDex[onlyCode].checkFlag(itemFlag::MAGAZINE))
						{
							ItemPocket* gunPocket = equipInfo[i].pocketPtr.get();

							if (gunPocket && !gunPocket->itemInfo.empty())
							{
								ItemData& magazine = gunPocket->itemInfo[0];

								if (getBulletNumber(magazine) > 0)
								{
									new Aim();
								}
								else updateLog(col2Str(col::white) + L"탄창에 탄이 없습니다.");
							}
							else updateLog(col2Str(col::white) + L"총에 탄창이 장착돼 있지 않습니다.");
						}
						/* ③ 그밖의 예외적인 경우(확장성을 위해) ------------------------------- */
                        else errorBox(L"이 총은 탄 정보를 찾을 수 없습니다.");
						break;
					}
				}
			}
			
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