﻿#include <SDL3/SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();


export module Equip;

import std;
import util;
import Player;
import drawText;
import globalVar;
import wrapVar;
import textureVar;
import ItemPocket;
import checkCursor;
import drawSprite;
import drawItemList;
import drawWindowArrow;
import GUI;
import log;
import Msg;
import actFuncSet;
import drawWindow;
import CoordSelect;
import Lst;
import Inventory;
import ItemData;
import ItemPocket;

export class Equip : public GUI
{
private:
	inline static Equip* ptr = nullptr;
	ItemPocket* equipPtr = PlayerPtr->getEquipPtr();

	int equipScroll = 0; //좌측 장비창의 스크롤
	int equipCursor = -1; //좌측 장비창의 커서
	const int equipScrollSize = 8;

	bool isTargetPocket = false; //우측의 포켓창이 조작 타겟일 경우

	SDL_Rect equipBase;
	SDL_Rect equipTitle;
	SDL_Rect equipItemRect[30];
	SDL_Rect equipItemSelectRect[30];
	SDL_Rect equipLabel;
	SDL_Rect equipLabelSelect;
	SDL_Rect equipLabelName;
	SDL_Rect equipLabelQuantity;
	SDL_Rect equipArea;
	SDL_Rect equipWindow;

	SDL_Rect topWindow;//상단에 표시되는 저항이나 방어 상성, 아이템의 설명


public:
	Equip() : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than equip instance was generated.");
		ptr = this;

		changeXY(0, (cameraH / 2) - 210, false);
		setAniSlipDir(4);

		//barAct = actSet::null;
		tabType = tabFlag::closeWin;
		UIType = act::equip;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);
	}
	~Equip()
	{
		prt(L"Equip : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		equipCursor = -1;
		equipScroll = 0;
		barAct = actSet::null;
		tabType = tabFlag::autoAtk;
	}
	static Equip* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		equipBase = { 0,0,335,420 };
		if (center == false)
		{
			equipBase.x += inputX;
			equipBase.y += inputY;
		}
		else
		{
			equipBase.x += inputX - equipBase.w / 2;
			equipBase.y += inputY - equipBase.h / 2;
		}

		equipTitle = { equipBase.x + 103, equipBase.y, 130, 30 };
		equipWindow = { equipBase.x, equipBase.y + 30, 335, 380 };

		equipLabel = { equipWindow.x + 10, equipWindow.y + 26, equipWindow.w - 20 , 26 };
		equipLabelSelect = { equipLabel.x, equipLabel.y, 62 , 26 };
		equipLabelName = { equipLabel.x + equipLabelSelect.w, equipLabel.y, 182 , 26 };
		equipLabelQuantity = { equipLabel.x + equipLabelName.w + equipLabelSelect.w, equipLabel.y, 71 , 26 };

		equipArea = { equipWindow.x + 10, equipWindow.y + 56,312, 42 * 8 - 6 };
		for (int i = 0; i < EQUIP_ITEM_MAX; i++)
		{
			equipItemRect[i] = { equipArea.x + 42, equipArea.y + 32 * i, 270, 26 };
			equipItemSelectRect[i] = { equipArea.x, equipArea.y + 32 * i, 36, 26 };
		}


		topWindow = { 0, 0, 410,140 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - equipBase.w / 2;
			y = inputY - equipBase.h / 2;
		}
	}
	void drawGUI();
	void clickUpGUI();
	void clickMotionGUI(int dx, int dy);
	void clickDownGUI();
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() 
	{
		if (checkCursor(&equipBase))
		{
			if (event.wheel.y > 0)
			{
				if(equipScroll>0) equipScroll -= 1;
			}
			else if (event.wheel.y < 0)
			{
				if(equipScroll + EQUIP_ITEM_MAX < equipPtr->itemInfo.size()) equipScroll += 1;
			}
		}
	}
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step() {};


	void executeTab()
	{
		if (equipCursor == -1) //아이템을 선택 중이지 않을 경우
		{
			close(aniFlag::winSlipClose);
		}
		else
		{
			//select 아이템이 하나라도 있을 경우 전부 제거
			equipScroll = 0;
			equipCursor = -1;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++) { equipPtr->itemInfo[i].lootSelect = 0; }
			barAct = actSet::null;
			tabType = tabFlag::closeWin;
		}
	}


	void executeOpen()
	{
		new Inventory(334, (cameraH / 2) - 210, &equipPtr->itemInfo[equipCursor]);
	}
	void updateBarAct()
	{
		if (equipPtr->itemInfo.size() > 0)
		{
			ItemData& targetItem = equipPtr->itemInfo[equipCursor];
			barAct.clear();
			if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
			if (targetItem.pocketOnlyItem.size() > 0) { barAct.push_back(act::reload); }//전용 아이템 있을 경우 reload 추가
			barAct.push_back(act::droping);//droping은 항상 추가
			barAct.push_back(act::throwing);//throwing도 항상 추가

			//업데이트할 아이템이 총일 경우
			if (targetItem.checkFlag(itemFlag::GUN))
			{
				//전용 아이템이 탄창일 경우(일반 소총)
				if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					ItemPocket* gunPtr = targetItem.pocketPtr.get();

					if (gunPtr->itemInfo.size() == 0)
					{
						barAct.push_back(act::reloadMagazine);
					}
					else
					{
						barAct.push_back(act::unloadMagazine);
					}
				}
				//전용 아이템이 탄일 경우(리볼버류)
				else if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
				{
					ItemPocket* gunPtr = targetItem.pocketPtr.get();
					//탄환 분리
					if (gunPtr->itemInfo.size() > 0)
					{
						barAct.push_back(act::unloadBulletFromGun);
					}

					//탄환 장전
					int bulletNumber = 0;
					for (int i = 0; i < gunPtr->itemInfo.size(); i++)
					{
						bulletNumber += gunPtr->itemInfo[i].number;
					}

					if (bulletNumber < targetItem.pocketMaxNumber)
					{
						barAct.push_back(act::reloadBulletToGun);
					}
				}
			}
			//업데이트할 아이템이 탄창일 경우
			else if (targetItem.checkFlag(itemFlag::MAGAZINE))
			{
				if (targetItem.itemCode != itemRefCode::arrowQuiver && targetItem.itemCode != itemRefCode::boltQuiver) barAct.push_back(act::reloadMagazine);

				//탄창 장전
				ItemPocket* magazinePtr = targetItem.pocketPtr.get();
				if (magazinePtr->itemInfo.size() > 0)
				{
					barAct.push_back(act::unloadBulletFromMagazine);
				}

				//총알 장전
				int bulletNumber = 0;
				for (int i = 0; i < magazinePtr->itemInfo.size(); i++)
				{
					bulletNumber += magazinePtr->itemInfo[i].number;
				}

				if (bulletNumber < targetItem.pocketMaxNumber)
				{
					barAct.push_back(act::reloadBulletToMagazine);
				}
			}
			//업데이트할 아이템이 탄환일 경우
			else if (targetItem.checkFlag(itemFlag::AMMO))
			{
				barAct.push_back(act::reloadBulletToGun);
			}
			
			if (targetItem.pocketMaxVolume > 0)
			{
                ItemPocket* pocketPtr = targetItem.pocketPtr.get();	
				if (pocketPtr->itemInfo.size() == 1)
				{
					if (pocketPtr->itemInfo[0].itemCode == itemRefCode::water)
					{

					}
				}
			}


			if (targetItem.checkFlag(itemFlag::CAN_EAT))
			{
                barAct.push_back(act::eat);
			}

			if (targetItem.pocketMaxVolume > 0)
			{
				ItemPocket* pocketPtr = targetItem.pocketPtr.get();
				// 포켓 내부에 CAN_DRINK 플래그가 있는 아이템이 있는지 확인
				for (int i = 0; i < pocketPtr->itemInfo.size(); i++)
				{
					if (pocketPtr->itemInfo[i].checkFlag(itemFlag::CAN_DRINK))
					{
						barAct.push_back(act::drink);
						break; // 하나라도 찾으면 중단
					}
				}
			}

			if (targetItem.checkFlag(itemFlag::TOGGLE_ON)) barAct.push_back(act::toggleOff);
			else if (targetItem.checkFlag(itemFlag::TOGGLE_OFF)) barAct.push_back(act::toggleOn);

			//쏟기 추가
			if (targetItem.pocketMaxVolume > 0 && targetItem.pocketPtr.get()->itemInfo.size() > 0)
			{
				barAct.push_back(act::dump);
			}
		}
	}

	Corouter executeReload()//장전 : 타겟아이템(탄창이나 총)에 넣을 수 있는 탄환을 넣는다.
	{
		int targetEquipCursor = equipCursor;
		std::vector<std::wstring> bulletList;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == equipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				bulletList.push_back(equipPtr->itemInfo[i].name);
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = equipPtr->itemInfo[i].pocketPtr.get();
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						bulletList.push_back(pocketPtr->itemInfo[pocketItr].name);
						break;
					}
				}
			}
		}

		if (bulletList.size() == 0) //넣을만한 포켓을 찾지 못했을 경우
		{
			//이 아이템을 넣을만한 포켓이 없다.
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], bulletList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == targetEquipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(1층)
				{
					if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
					{
						if (equipPtr->itemInfo[i].checkFlag(itemFlag::AMMO))
						{
							//(%bullet) (%number)개를(을) (%gun)에 장전했다!

							std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
							std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str3);
						}
						else
						{
							//(%magazine)를(을) (%gun)에 장전했다!
							std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str2);
						}
					}
					else
					{
						//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
						std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", equipPtr->itemInfo[i].name);
						std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
						std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
						updateLog(col2Str(col::white) + str3);
					}

					equipPtr->transferItem
					(
						equipPtr->itemInfo[targetEquipCursor].pocketPtr.get(),
						i,
						equipPtr->itemInfo[i].number
					);
					co_return;
				}
				counter++;
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = equipPtr->itemInfo[i].pocketPtr.get();
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(2층)
						{
							if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
							{
								if (pocketPtr->itemInfo[pocketItr].checkFlag(itemFlag::AMMO))
								{
									//(%bullet) (%number)개를(을) (%gun)에 장전했다!

									std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
									std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str3);
								}
								else
								{
									//(%magazine)를(을) (%gun)에 장전했다!
									std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str2);
								}
							}
							else
							{
								//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
								std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
								std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
								std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
								updateLog(col2Str(col::white) + str3);
							}


							pocketPtr->transferItem
							(
								equipPtr->itemInfo[targetEquipCursor].pocketPtr.get(),
								pocketItr,
								pocketPtr->itemInfo[pocketItr].number
							);
							co_return;
						}
						counter++;
						break;
					}
				}
			}
		}
	}

	void executeDroping()
	{
		std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
		equipPtr->transferItem(drop.get(), equipCursor, 1);
		PlayerPtr->throwing(std::move(drop), PlayerX(), PlayerY());
		PlayerPtr->updateStatus();

		updateLog(col2Str(col::white) + sysStr[126]);
	}

	Corouter executeThrowing(ItemPocket* inputPocket, int inputIndex)//던지기
	{
		
		new CoordSelect(sysStr[131]);
		deactDraw();
		rangeRay = true;

		co_await std::suspend_always();

		if (coAnswer.empty())
		{
			rangeRay = false;
			actDraw();
			tabType = tabFlag::closeWin;
			co_return;
		}

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		if (targetX == PlayerX() && targetY == PlayerY() && targetZ == PlayerZ());
		else PlayerPtr->setDirection(getIntDegree(PlayerX(), PlayerY(), targetX, targetY));

		prt(L"executeThrowing에서 사용한 좌표의 값은 (%d,%d,%d)이다.\n", targetX, targetY, targetZ);

		std::unique_ptr<ItemPocket> throwing = std::make_unique<ItemPocket>(storageType::null);
		inputPocket->transferItem(throwing.get(), inputIndex, 1);
		PlayerPtr->throwing(std::move(throwing), targetX, targetY);
		PlayerPtr->updateStatus();

		updateLog(L"#FFFFFF아이템을 던졌다.");
		close(aniFlag::null);
		rangeRay = false;
	}

};
