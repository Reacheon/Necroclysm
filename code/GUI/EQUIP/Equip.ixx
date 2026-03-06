module;
#include <SDL3/SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();


export module Equip;

import std;
import util;
import constVar;
import Player;
import drawText;
import globalVar;
import wrapFunc;
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
import CoordSelectCraft;
import Prop;
import ItemListPanel;
import barActCommon;


export class Equip : public GUI
{
private:
	inline static Equip* ptr = nullptr;
	ItemPocket* equipPtr = PlayerPtr->getEquipPtr();

	const int equipScrollSize = 8;

	bool isTargetPocket = false; //우측의 포켓창이 조작 타겟일 경우

	SDL_Rect equipBase;
	SDL_Rect equipTitle;
	SDL_Rect equipArea;

	SDL_Rect topWindow;//상단에 표시되는 저항이나 방어 상성, 아이템의 설명


public:
	ItemListPanel panel{ EQUIP_ITEM_MAX };

	Equip() : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than equip instance was generated.");
		ptr = this;

        int arrowEndX, arrowEndY, targetX, targetY;
		arrowEndX = cameraW / 2 - 8 * zoomScale + 16 * 0 * zoomScale;
		arrowEndY = cameraH / 2 + 16 * 0 * zoomScale;
		targetX = arrowEndX - 429;
		targetY = arrowEndY - 250;

		changeXY(targetX, targetY, false);
		setAniSlipDir(4);

		UIType = act::equip;

		panel.pocket = equipPtr;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Equip()
	{
		prt(L"Equip : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		panel.cursor = -1;
		panel.scroll = 0;
		barAct = actSet::null;
	}
	static Equip* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		equipBase = { 0,0,404,506 };
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

		equipTitle = { equipBase.x + 124, equipBase.y, 156, 36 };
		equipArea = { equipBase.x + 12, equipBase.y + 36 + 67, 376, 37 * 8 };
		panel.initRects(equipBase.x, equipBase.y + 103);
		panel.label.w = equipBase.w - 24;

		topWindow = { 0, 0, 492, 168 };
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
		panel.handleWheel(equipBase);
	}
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		tabType = tabFlag::back;
	};


	void executeTab()
	{
		if (panel.cursor == -1) //아이템을 선택 중이지 않을 경우
		{
			close(aniFlag::winUnfoldClose);
		}
		else
		{
			//select 아이템이 하나라도 있을 경우 전부 제거
			panel.scroll = 0;
			panel.cursor = -1;
			panel.clearAllSelections();
			barAct = actSet::null;
		}
	}


	void executeOpen()
	{
		new Inventory(equipBase.x + equipBase.w - 1, equipBase.y, &equipPtr->itemInfo[panel.cursor]);
	}
	void updateBarAct()
	{
		if (equipPtr->itemInfo.size() > 0)
		{
			ItemData& targetItem = equipPtr->itemInfo[panel.cursor];
			barAct.clear();
			if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
			barAct.push_back(act::droping);//droping은 항상 추가
			barAct.push_back(act::throwing);//throwing도 항상 추가

			appendGunAmmoBarActs(targetItem);

			if (targetItem.pocketMaxVolume > 0)
			{
                ItemPocket* pocketPtr = targetItem.pocketPtr.get();
				if (pocketPtr->itemInfo.size() == 1)
				{
					if (pocketPtr->itemInfo[0].itemCode == itemID::water)
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

			//설치 추가

			if (targetItem.propInstallCode != 0)
			{
				if(targetItem.checkFlag(itemFlag::CAN_PLANT)) barAct.push_back(act::plant);
				else barAct.push_back(act::propInstall);
			}

			if (targetItem.checkFlag(itemFlag::SEED_FRUIT))
			{
				barAct.push_back(act::extractSeed);
			}

			if (targetItem.checkFlag(itemFlag::POWERED_BY_BATTERY))
			{
				if (targetItem.pocketPtr->itemInfo.size() >= 1) barAct.push_back(act::removeBattery);
				else barAct.push_back(act::insertBattery);
			}
		}
	}

	Corouter executeReload()//장전 : 타겟아이템(탄창이나 총)에 넣을 수 있는 탄환을 넣는다.
	{
		int targetEquipCursor = panel.cursor;
		std::vector<std::wstring> bulletList;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == panel.cursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[panel.cursor].pocketOnlyItem;
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
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[panel.cursor].pocketOnlyItem;
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
			updateLog(sysStr[96]);//이 아이템을 넣을만한 포켓이 없다.
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], bulletList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		if (coAnswer.empty() == true) co_return;
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
							updateLog(str3);
						}
						else
						{
							//(%magazine)를(을) (%gun)에 장전했다!
							std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(str2);
						}
					}
					else
					{
						//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
						std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", equipPtr->itemInfo[i].name);
						std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
						std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
						updateLog(str3);
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
									updateLog(str3);
								}
								else
								{
									//(%magazine)를(을) (%gun)에 장전했다!
									std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(str2);
								}
							}
							else
							{
								//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
								std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
								std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
								std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
								updateLog(str3);
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
		updateLog(replaceStr(sysStr[126], L"(%item)", equipPtr->itemInfo[panel.cursor].name));//(%item)를(을) 버렸다.
		equipPtr->transferItem(drop.get(), panel.cursor, 1);
		PlayerPtr->throwing(std::move(drop), PlayerX(), PlayerY());
		PlayerPtr->updateStatus();
	}



	Corouter executePropInstall()
	{
		deactDraw();
		std::vector<Point2> selectableTile;
		for (int dir = 0; dir < 8; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);
			if (TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
		}

		ItemData& tgtItem = equipPtr->itemInfo[panel.cursor];
		rangeSet.clear();
		for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });
		if (rangeSet.size() == 0)
		{
			updateLog(L"No valid space to install.");
			co_return;
		}

		int tgtItemCode = equipPtr->itemInfo[panel.cursor].itemCode;

		new CoordSelectCraft(tgtItem.propInstallCode, sysStr[299], selectableTile);//조합할 아이템을 설치할 위치를 선택해주세요.
		co_await std::suspend_always();
		rangeSet.clear();
		actDraw();

		if (coAnswer.empty() == false)
		{
			std::wstring targetStr = coAnswer;
			int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);
			int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);

			int targetItemCode = wtoi(targetStr.c_str());
			Point3 buildLocation = { targetX,targetY,PlayerZ() };

			createProp(buildLocation, targetItemCode);

			ItemData& propItem = TileProp(targetX, targetY, PlayerZ())->leadItem;

			//설치할 아이템의 내부에 있는 아이템을 전부 설치된 상태 아이템의 내부로 옮김
			if (propItem.pocketPtr != nullptr && tgtItem.pocketPtr != nullptr)
			{
                errorBox(propItem.pocketMaxVolume != tgtItem.pocketMaxVolume, L"PropInstall : propItem.pocketMaxVolume != tgtItem.pocketMaxVolume");
				for (int i = tgtItem.pocketPtr->itemInfo.size() - 1; i >= 0; i--)
				{
                    tgtItem.pocketPtr->transferItem(propItem.pocketPtr.get(), i, tgtItem.pocketPtr->itemInfo[i].number);
				}
			}


			for (int i = equipPtr->itemInfo.size()-1; i >= 0; i--)
			{
				if (equipPtr->itemInfo[i].itemCode == tgtItemCode)
				{
                    equipPtr->itemInfo.erase(equipPtr->itemInfo.begin() + i);
				}
			}
			PlayerPtr->updateStatus();

            updateLog(replaceStr(sysStr[329], L"(%item)", itemDex[targetItemCode].name));

			close(aniFlag::null);
		}
		else co_return;
	}
};
