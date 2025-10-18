#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();
#include <SDL3/SDL.h>

export module ContextMenu;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import wrapVar;
import checkCursor;
import drawWindow;
import actFuncSet;
import Player;
import Loot;
import World;
import Vehicle;
import Prop;
import log;
import Lst;
import Maint;
import drawEpsilonText;
import ItemData;
import Sleep;
import turnWait;

export class ContextMenu : public GUI
{
private:
	inline static ContextMenu* ptr = nullptr;
	SDL_Rect contextMenuBase;
	SDL_Rect subContextMenuBase;
	int contextMenuCursor = -1;
	int contextMenuScroll = 0;
	std::vector<act> actOptions;
	std::array<SDL_Rect,30> optionRect;

public:
	ContextMenu(int inputMouseX, int inputMouseY, int inputGridX, int inputGridY, std::vector<act> inputOptions) : GUI(false)
	{
		actOptions = inputOptions;
		contextMenuTargetGrid = { inputGridX, inputGridY };
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(inputMouseX, inputMouseY, false);

		if (contextMenuTargetGrid.x > PlayerX()) PlayerPtr->setDirection(0);
		else if (contextMenuTargetGrid.x < PlayerX()) PlayerPtr->setDirection(4);

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~ContextMenu()
	{
		ptr = nullptr;
	}
	static ContextMenu* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		//94
		contextMenuBase = { 0, 0, 124, 6 + 22 * (int)actOptions.size() };
		subContextMenuBase = { 0,0,0,0 };

		

		if (center == false)
		{
			contextMenuBase.x += inputX;
			contextMenuBase.y += inputY;
		}
		else
		{
			contextMenuBase.x += inputX - contextMenuBase.w / 2;
			contextMenuBase.y += inputY - contextMenuBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - contextMenuBase.w / 2;
			y = inputY - contextMenuBase.h / 2;
		}

		for (int i = 0; i < 30; i++) optionRect[i] = { contextMenuBase.x + 4, contextMenuBase.y + 4 + 22 * i, contextMenuBase.w - 8, 20 };

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }





		if (getFoldRatio() == 1.0)
		{

			drawFillRect(contextMenuBase, col::black);
			drawRect(contextMenuBase, col::gray);

			//94 110
			for (int i = 0; i < actOptions.size(); i++)
			{
				//64,65
				std::wstring optionText;
				int iconIndex = 0;
				if (actOptions[i] == act::closeDoor)
				{
					optionText = sysStr[176];//닫기
					iconIndex = 64;
				}
				else if (actOptions[i] == act::inspect)
				{
					optionText = sysStr[177];//조사
					iconIndex = 65;
				}
				else if (actOptions[i] == act::unbox)
				{
					optionText = sysStr[178];//열기
					iconIndex = 66;
				}
				else if (actOptions[i] == act::pull)
				{
					optionText = sysStr[179];//당기기
					iconIndex = 67;
				}
				else if (actOptions[i] == act::climb)
				{
					optionText = sysStr[188];//등반
					iconIndex = 69;
				}
				else if (actOptions[i] == act::swim)
				{
					optionText = sysStr[186];//수영
					iconIndex = 70;
				}
				else if (actOptions[i] == act::ride)
				{
					optionText = sysStr[187];//탑승
					iconIndex = 71;
				}
				else if (actOptions[i] == act::vehicleRepair)
				{
					optionText = sysStr[203];//수리
					iconIndex = 28;
				}
				else if (actOptions[i] == act::vehicleDetach)
				{
					optionText = sysStr[204];//탈착
					iconIndex = 85;
				}
				else if (actOptions[i] == act::drawLiquid)
				{
					optionText = sysStr[206];//담기
					iconIndex = 93;
				}
				else if (actOptions[i] == act::sleep)
				{
					optionText = sysStr[211];//수면
					iconIndex = 98;
				}
				else if (actOptions[i] == act::drinkFloorWater)
				{
					optionText = sysStr[232];//마시기(바닥물마시기)
					iconIndex = 99;
				}
				else if (actOptions[i] == act::propCarry)
				{
					optionText = sysStr[327];//프롭 들기
					iconIndex = 100;
				}
				else if (actOptions[i] == act::propTurnOn)
				{
					optionText = sysStr[333];//켜기
					iconIndex = 102;
				}
				else if (actOptions[i] == act::propTurnOff)
				{
					optionText = sysStr[334];//끄기
					iconIndex = 101;
				}
				else if (actOptions[i] == act::connectPlusZ)
				{
					optionText = sysStr[340];//상층과 연결
					iconIndex = 101;
				}
				else if (actOptions[i] == act::connectMinusZ)
				{
					optionText = sysStr[341];//하층과 연결
					iconIndex = 101;
				}
				else optionText = L"???";

				if (checkCursor(&optionRect[i]))
				{
					if (click) drawFillRect(optionRect[i], lowCol::deepBlue);
					else  drawFillRect(optionRect[i], lowCol::blue);
				}
				else drawFillRect(optionRect[i], lowCol::black);
				setFontSize(16);
				renderTextCenter(optionText, optionRect[i].x + optionRect[i].w / 2 + 16, optionRect[i].y + optionRect[i].h / 2 + 1);
				drawSpriteCenter(spr::icon16, iconIndex, optionRect[i].x + 10, optionRect[i].y + 10);
			}
			
			// 좌측상단
			drawLine(contextMenuBase.x, contextMenuBase.y, contextMenuBase.x + 12, contextMenuBase.y, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y + 1, contextMenuBase.x + 12, contextMenuBase.y + 1, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y, contextMenuBase.x, contextMenuBase.y + 12, col::lightGray);
			drawLine(contextMenuBase.x + 1, contextMenuBase.y, contextMenuBase.x + 1, contextMenuBase.y + 12, col::lightGray);

			// 우측상단
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y + 1, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + 1, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + 12, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y, contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y + 12, col::lightGray);

			// 좌측하단
			drawLine(contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + 12, contextMenuBase.y + contextMenuBase.h - 1, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 2, contextMenuBase.x + 12, contextMenuBase.y + contextMenuBase.h - 2, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);
			drawLine(contextMenuBase.x + 1, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + 1, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);

			// 우측하단
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 1, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y + contextMenuBase.h - 2, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 2, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);


			drawFillRect(subContextMenuBase, col::black);



			{
				int mouseX = getAbsMouseGrid().x;
				int mouseY = getAbsMouseGrid().y;

				int tileW = (float)cameraW / (16.0 * zoomScale);
				int tileH = (float)cameraH / (16.0 * zoomScale);

				Point2 tgtGrid;

				//컨텍스트메뉴가 열려있으면 거기로 고정
				if (ContextMenu::ins() != nullptr)  tgtGrid = { contextMenuTargetGrid.x,contextMenuTargetGrid.y };
				else tgtGrid = { mouseX,mouseY };

				if (tgtGrid.x > PlayerX() - tileW / 2 - 1 && tgtGrid.x < PlayerX() + tileW / 2 + 1 && tgtGrid.y > PlayerY() - tileH / 2 - 1 && tgtGrid.y < PlayerY() + tileH / 2 + 1)
				{
					Vehicle* vehPtr = TileVehicle(tgtGrid.x, tgtGrid.y, PlayerZ());
					ItemStack* stackPtr = TileItemStack(tgtGrid.x, tgtGrid.y, PlayerZ());
					if (vehPtr != nullptr)
					{
						int pivotX = cameraW - 200;
						int pivotY = 148;

						drawFillRect(pivotX, pivotY, 192, 17, col::black, 200);
						drawRect(pivotX, pivotY, 192, 17, col::lightGray, 255);
						setFontSize(10);
						std::wstring titleName = vehPtr->name;
						renderTextCenter(titleName, pivotX + 96, pivotY + 9);
						if (vehPtr->vehType == vehFlag::heli) drawSpriteCenter(spr::icon16, 89, pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 7);
						else if (vehPtr->vehType == vehFlag::train) drawSpriteCenter(spr::icon16, 90, pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 7);
						else if (vehPtr->vehType == vehFlag::minecart) drawSpriteCenter(spr::icon16, 92, pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 7);
						else drawSpriteCenter(spr::icon16, 88, pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 7);



						int newPivotY = pivotY + 16;


						int vehSize = vehPtr->partInfo[{tgtGrid.x, tgtGrid.y}]->itemInfo.size();
						drawFillRect(pivotX, newPivotY, 192, 25 + 17 * (vehSize - 1), col::black, 200);
						drawRect(pivotX, newPivotY, 192, 25 + 17 * (vehSize - 1), col::lightGray, 255);


						for (int i = 0; i < vehSize; i++)
						{
							ItemData& tgtPart = vehPtr->partInfo[{tgtGrid.x, tgtGrid.y}]->itemInfo[vehSize - 1 - i];
							//내구도
							drawRect(pivotX + 6, newPivotY + 6 + 17 * i, 6, 13, col::white);
							drawFillRect(pivotX + 8, newPivotY + 8 + 17 * i, 2, 9, lowCol::green);

							//아이템 아이콘
							drawSpriteCenter(spr::itemset, getItemSprIndex(tgtPart), pivotX + 24, newPivotY + 12 + 17 * i);

							//아이템 이름
                            renderText(tgtPart.name, pivotX + 35, newPivotY + 6 + 17 * i, col::white);


							if (tgtPart.pocketPtr != nullptr) //Inventory에도 같은 코드가 존재
							{

								ItemPocket* pkPtr = tgtPart.pocketPtr.get();
								if (tgtPart.pocketMaxVolume > 0)
								{
									SDL_Rect volumeGaugeRect = { pivotX + 135, newPivotY + 7 + 17 * i, 53, 11 };
									drawRect(volumeGaugeRect, col::white);
									//drawFillRect(pivotX + 135 + 2, newPivotY + 7 + 2 + 17 * i, 45, 7, lowCol::orange);

									int currentVolume = 0;
									for (int i = 0; i < pkPtr->itemInfo.size(); i++)
									{
										currentVolume += getVolume(pkPtr->itemInfo[i]) * (pkPtr->itemInfo[i].number);
									}
									float volumeRatio = (float)currentVolume / (float)tgtPart.pocketMaxVolume;
									SDL_Color gaugeCol = lowCol::green;
									if (volumeRatio > 0.6) gaugeCol = lowCol::yellow;
									else if (volumeRatio > 0.9) gaugeCol = lowCol::red;
									drawFillRect(SDL_Rect{ volumeGaugeRect.x + 2,volumeGaugeRect.y + 2,static_cast<int>(49.0 * volumeRatio),6 }, gaugeCol);

									std::wstring currentVolumeStr = decimalCutter((float)currentVolume / 1000.0, 1);
									std::wstring maxVolumeStr = decimalCutter((float)tgtPart.pocketMaxVolume / 1000.0, 1) + L"L";
									drawEplsionText(currentVolumeStr + L"/" + maxVolumeStr, pivotX + 135 + 3, newPivotY + 7 + 3 + 17 * i, col::white);

								}
								else if (tgtPart.pocketMaxNumber > 0)
								{

								}
							}



						}
					}
				}
			}
		}
		else
		{
			SDL_Rect vRect = contextMenuBase;
			int type = 1;
			switch (type)
			{
			case 0:
				vRect.w = vRect.w * getFoldRatio();
				vRect.h = vRect.h * getFoldRatio();
				break;
			case 1:
				vRect.x = vRect.x + vRect.w * (1 - getFoldRatio()) / 2;
				vRect.w = vRect.w * getFoldRatio();
				break;
			}
			drawWindow(&vRect);
		}
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else if(checkCursor(&contextMenuBase))
		{
			for (int i = 0; i < actOptions.size(); i++)
			{
				if (checkCursor(&optionRect[i]))
				{
					executeContextAct(actOptions[i]);
				}

			}

			close(aniFlag::null);
		}
		else close(aniFlag::winUnfoldClose);
	}
	void step() 
	{
		tabType = tabFlag::back;
		contextMenuBase.h = 6 + 22 * (int)actOptions.size();
		//bool openSub = false;
		//for (int i = 0; i < actOptions.size(); i++)
		//{
		//	if (actOptions[i] == act::inspect)
		//	{
		//		SDL_Rect optionRect = { contextMenuBase.x + 4, contextMenuBase.y + 4 + 22 * i, 90, 20 };
		//		if (checkCursor(&optionRect) || checkCursor(&subContextMenuBase))
		//		{
		//			subContextMenuBase = { contextMenuBase.x + contextMenuBase.w, contextMenuBase.y + 4 + 22 * i - 1, contextMenuBase.w,100 };
		//			openSub = true;
		//		}
		//	}
		//}

		//if(openSub==false) subContextMenuBase = { -1, -1, -1,-1 };
	}

	void executeContextAct(act inputAct)
	{
		if (inputAct == act::closeDoor)
		{
			if(TileVehicle(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()) != nullptr)
			{
				actFunc::closeVDoor(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			}
			else if(TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()) != nullptr)
			{
				actFunc::closeDoor(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			}
		}
		else if (inputAct == act::unbox)
		{
			Vehicle* vPtr = TileVehicle(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
            Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());


			if (pPtr != nullptr && pPtr->leadItem.pocketPtr != nullptr)
			{
				new Loot(pPtr->leadItem.pocketPtr.get(), &(pPtr->leadItem));
			}
			else if (vPtr != nullptr)
			{
				Vehicle* vPtr = TileVehicle(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
				for (int i = 0; i < vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo.size(); i++)
				{
					if (vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo[i].checkFlag(itemFlag::POCKET))
					{
						new Loot(vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo[i].pocketPtr.get(), &(vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo[i]));
						break;
					}
				}
			}
		}
		else if (inputAct == act::pull)
		{
			if (PlayerPtr->pulledCart == nullptr)
			{
				PlayerPtr->pulledCart = TileVehicle(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			}
			else
			{
				PlayerPtr->pulledCart = nullptr;
			}
		}
		else if (inputAct == act::ride)
		{
			PlayerPtr->ridingEntity = std::move(World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).EntityPtr);
			PlayerPtr->ridingType = ridingFlag::horse;
		}
		else if (inputAct == act::vehicleRepair)
		{
			Vehicle* vPtr = TileVehicle(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			std::vector<ItemData>& vParts = vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo;
			std::vector<std::wstring> partNames;
			for (int i = 0; i < vParts.size(); i++)
			{
				partNames.push_back(vParts[i].name);
			}

			new Maint(L"수리", L"수리할 부품을 선택해주세요.", {contextMenuTargetGrid.x,contextMenuTargetGrid.y,PlayerZ()},maintFlag::repair);
		}
		else if (inputAct == act::vehicleDetach)
		{
			Vehicle* vPtr = TileVehicle(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			std::vector<ItemData>& vParts = vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo;
			std::vector<std::wstring> partNames;
			for (int i = 0; i < vParts.size(); i++)
			{
				partNames.push_back(vParts[i].name);
				partNames.push_back(vParts[i].name);
				partNames.push_back(vParts[i].name);
			}

			new Maint(L"탈착", L"차량에서 분리할 부품을 선택해주세요.", { contextMenuTargetGrid.x,contextMenuTargetGrid.y,PlayerZ() },maintFlag::detach);
		}
		else if (inputAct == act::drawLiquid)
		{
			ItemPocket* targetBottle = nullptr;
			int maxVolume = 0;
			for (int i = 0; i < PlayerPtr->getEquipPtr()->itemInfo.size(); i++)
			{
				if (PlayerPtr->getEquipPtr()->itemInfo[i].checkFlag(itemFlag::CONTAINER_LIQ))
				{
					targetBottle = PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get();
					maxVolume = PlayerPtr->getEquipPtr()->itemInfo[i].pocketMaxVolume;
					break;
				}

				if (PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr != nullptr &&
					PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo.size() > 0)
				{
					for (int j = 0; j < PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo.size(); j++)
					{
						if (PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo[j].checkFlag(itemFlag::CONTAINER_LIQ))
						{
							targetBottle = PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo[j].pocketPtr.get();
							maxVolume = PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo[j].pocketMaxVolume;
							i = PlayerPtr->getEquipPtr()->itemInfo.size();//이중루프 탈출
							break;
						}
					}
				}
			}

			if (targetBottle != nullptr)
			{
				targetBottle->itemInfo.clear();
				targetBottle->itemInfo.push_back(std::move(cloneFromItemDex(itemDex[itemRefCode::water], maxVolume)));
				updateLog(L"You fill the bottle with water.");
				
			}
		}
		else if (inputAct == act::sleep)
		{
			new Sleep();
		}
		else if (inputAct == act::drinkFloorWater)
		{
			int floorCode = TileFloor(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			if (floorCode == itemRefCode::shallowFreshWater || floorCode == itemRefCode::deepFreshWater)
			{
				thirst = PLAYER_MAX_HYDRATION;
			}
			else if (floorCode == itemRefCode::shallowSeaWater || floorCode == itemRefCode::deepSeaWater)
			{
				thirst -= 100;
			}
		}
		else if (inputAct == act::propCarry)
		{
			if (TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()) != nullptr)
			{
                ItemData& propItem = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ())->leadItem;
				std::unique_ptr<ItemPocket> tempPocket = std::make_unique<ItemPocket>(storageType::temp);
                ItemPocket* tempPocketPtr = tempPocket.get();
				tempPocket->addItemFromDex(propItem.propUninstallCode, 1);
				

				//프롭 내부에 있는 아이템을 전부 미설치 상태 아이템의 내부로 옮김
                if (propItem.pocketPtr != nullptr && tempPocket->itemInfo[0].pocketPtr != nullptr)
				{
                    errorBox(propItem.pocketMaxVolume != tempPocket->itemInfo[0].pocketMaxVolume, L"Prop uninstall pocket volume is different from itemdex.");
					for (int i = propItem.pocketPtr->itemInfo.size() - 1; i >= 0; i--)
					{
						propItem.pocketPtr->transferItem(tempPocket->itemInfo[0].pocketPtr.get(), i, propItem.pocketPtr->itemInfo[i].number);
					}
				}

                CORO(actFunc::executeWield(tempPocketPtr, 0));//8월 30일 여기서부터... 아직 들기가 되지않음
				destroyProp({ contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ() });
			}
		}
		else if (inputAct == act::propTurnOn)
		{
			if(TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()) != nullptr)
			{
                Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
				if (pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorR ||
					pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorT ||
					pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorL ||
					pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorB)
				{
					if (pPtr->leadItem.pocketPtr->itemInfo.size() > 0)
					{
						if (pPtr->leadItem.pocketPtr->itemInfo[0].itemCode == itemRefCode::gasoline)
						{
							pPtr->propTurnOn();
							updateLog(sysStr[337]);//가솔린 발전기에 시동을 걸었다.
						}
						else updateLog(sysStr[335]);//이 발전기를 켜기 위해서는 내부에 가솔린이 들어있어야 한다. 
					}
					else updateLog(sysStr[335]);//이 발전기를 켜기 위해서는 내부에 가솔린이 들어있어야 한다. 
					turnWait(1.0);
				}
				else
				{
					Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
					pPtr->propTurnOn();
					turnWait(1.0);
				}
            }


		}
		else if (inputAct == act::propTurnOff)
		{
			if (TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()) != nullptr)
			{
				Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
				if (pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorR ||
					pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorT ||
					pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorL ||
					pPtr->leadItem.itemCode == itemRefCode::gasolineGeneratorB)
				{
					pPtr->propTurnOff();
					updateLog(sysStr[339]);//발전기를 껐다.
					turnWait(1.0);
				}
				else
				{
					Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
					pPtr->propTurnOff();
					turnWait(1.0);
				}
			}


        }
		else if (inputAct == act::connectPlusZ)
		{
			Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			pPtr->leadItem.addFlag(itemFlag::CABLE_Z_ASCEND);

			Prop* abovePropPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ() + 1);
			abovePropPtr->leadItem.addFlag(itemFlag::CABLE_Z_DESCEND);
		}
		else if (inputAct == act::connectMinusZ)
		{
			Prop* pPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			pPtr->leadItem.addFlag(itemFlag::CABLE_Z_DESCEND);

			Prop* belowPropPtr = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ() - 1);
			belowPropPtr->leadItem.addFlag(itemFlag::CABLE_Z_ASCEND);
		}
	}
};