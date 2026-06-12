#include <SDL3/SDL.h>

import HUD;
import std;
import util;
import constVar;
import globalVar;
import World;
import Player;
import Vehicle;
import log;
import Dialogue;
import Loot;
import Prop;
import ItemData;
import GodPanel;
import actFuncSet;

void HUD::tileTouch(int touchX, int touchY) //일반 타일 터치
{
	const bool* state = SDL_GetKeyboardState(nullptr);
	bool wieldPickaxe = false;
	std::vector<ItemData>& equipInfo = PlayerEquip()->itemInfo;
	for (const ItemData& eqItem : equipInfo)
	{
		if (eqItem.equipState == equipHandFlag::both)
		{
			if (eqItem.itemCode == itemID::pickaxe) wieldPickaxe = true;
		}
	}
	int floorCode = TileFloor(touchX, touchY, PlayerZ());

	if (ctrlVeh == nullptr && currentUsingSkill.empty())//차량 조종 중이 아닐 경우
	{
		//화면에 있는 아이템 터치
		if (touchX == PlayerX() && touchY == PlayerY()) //자신 위치 터치
		{
			//우선순위: Vehicle 운전대 > Vehicle storage(pocketPtr) > ItemStack.
			//  같은 타일에 vehicle + itemStack 공존 시 vehicle이 먼저. vehicle 부품에
			//  운전대 있으면 운전 모드 토글, 없고 storage(쇼핑바스켓 등)만 있으면 Loot 오픈.
			Vehicle* belowVehicle = TileVehicle(touchX, touchY, PlayerZ());
			bool handled = false;

			if (belowVehicle != nullptr)
			{
				auto& partItems = belowVehicle->partInfo[{touchX, touchY, PlayerZ()}]->itemInfo;

				int controllerIdx = -1;
				int storageIdx = -1;
				for (int i = 0; i < (int)partItems.size(); i++)
				{
					int code = partItems[i].itemCode;
					if (code == itemID::vehicleControl
					 || code == itemID::helicopterController
					 || code == itemID::trainControl  //열차 조종장치(Train Control)
					 || code == itemID::minecartController)
					{
						controllerIdx = i;
						break;
					}
					if (storageIdx == -1 && partItems[i].pocketPtr != nullptr)
					{
						storageIdx = i;
					}
				}

				if (controllerIdx != -1)
				{
					int code = partItems[controllerIdx].itemCode;
					if (code == itemID::vehicleControl)
					{
						if (ctrlVeh == nullptr)
						{
							ctrlVeh = belowVehicle;
							barAct = actSet::vehicle;
							typeHUD = vehFlag::car;
							PlayerPtr->updateVision();
							PlayerPtr->updateMinimap();
						}
						else
						{
							ctrlVeh = nullptr;
							barAct = actSet::null();
							typeHUD = vehFlag::none;
							PlayerPtr->updateVision();
							PlayerPtr->updateMinimap();
						}
					}
					else if (code == itemID::helicopterController)
					{
						if (ctrlVeh == nullptr)
						{
							ctrlVeh = belowVehicle;
							barAct = actSet::helicopter;
							typeHUD = vehFlag::heli;
						}
						else
						{
							ctrlVeh = nullptr;
							barAct = actSet::null();
							typeHUD = vehFlag::none;
						}
					}
					else //313(Train Control) / itemID::minecartController - 둘 다 동일 train 모드
					{
						if (ctrlVeh == nullptr)
						{
							ctrlVeh = belowVehicle;
							barAct = actSet::train;
							typeHUD = vehFlag::minecart;
						}
						else
						{
							ctrlVeh = nullptr;
							barAct = actSet::null();
							typeHUD = vehFlag::none;
						}
					}
					handled = true;
				}
				else if (storageIdx != -1)
				{
					Point3 containerPos = { touchX, touchY, PlayerZ() };
					new Loot(partItems[storageIdx].pocketPtr.get(), &partItems[storageIdx], containerPos);
					click = false;
					handled = true;
				}
			}

			if (!handled && TileItemStack(touchX, touchY, PlayerZ()) != nullptr)
			{
				ItemStack* targetStack = TileItemStack(PlayerX(), PlayerY(), PlayerZ());
				new Loot(targetStack);
				click = false;
			}
			else if (!handled)
			{
				if (TileProp(touchX, touchY, PlayerZ()) != nullptr)
				{
					Prop* tgtProp = TileProp(touchX, touchY, PlayerZ());
					int tgtItemCode = tgtProp->leadItem.itemCode;
					if (tgtProp->leadItem.checkFlag(itemFlag::UPSTAIR))
					{
						if (TileFloor(PlayerX(), PlayerY(), PlayerZ() + 1) == itemID::none)
						{
							updateLog(L"There is no floor above these stairs.");
						}
						else
						{
							updateLog(L"You go up the stairs.");

							EntityPtrMove({ PlayerX(), PlayerY(), PlayerZ() }, { PlayerX(), PlayerY(), PlayerZ() + 1 });

							PlayerPtr->updateVision(PlayerInfo().eyeSight);
							PlayerPtr->updateMinimap();

							Prop* upProp = TileProp(touchX, touchY, PlayerZ());
							if (upProp == nullptr)
							{
								createProp({ PlayerX(), PlayerY(), PlayerZ() }, itemID::downwardStairs);//하강계단
							}
						}
					}
					else if (tgtProp->leadItem.checkFlag(itemFlag::DOWNSTAIR))
					{
						if (TileWall(PlayerX(), PlayerY(), PlayerZ() + 1) != itemID::none)
						{
							updateLog(L"The stairs going down are blocked by a wall.");
						}
						else
						{
							updateLog(L"You go down the stairs.");

							EntityPtrMove({ PlayerX(), PlayerY(), PlayerZ() }, { PlayerX(), PlayerY(), PlayerZ() - 1 });

							PlayerPtr->updateVision(PlayerInfo().eyeSight);
							PlayerPtr->updateMinimap();

							Prop* downProp = TileProp(touchX, touchY, PlayerZ());
							if (downProp == nullptr)
							{
								createProp({ PlayerX(), PlayerY(), PlayerZ() }, itemID::upwardStairs);//상승계단
							}
						}
					}
					else if (tgtProp->leadItem.itemCode == itemID::altarOfRehylion)
					{
						if (GodPanel::ins() == nullptr)
						{
							new GodPanel(godFlag::rehylion, { touchX, touchY, PlayerZ() });
							click = false;
						}
					}
				}
			}
		}
		else if ((std::abs(touchX - PlayerX()) <= 1 && std::abs(touchY - PlayerY()) <= 1) && isWalkable({ touchX, touchY, PlayerZ() }) == false)//1칸 이내(이동불가타일)
		{
			if (TileWall(touchX, touchY, PlayerZ()) != itemID::none) //곡괭이 벽 굴착 액션
			{
				auto ePtr = PlayerEquip();
				for (int i = 0; i < ePtr->itemInfo.size(); i++)
				{
					if (ePtr->itemInfo[i].itemCode == itemID::pickaxe)
					{
						if (ePtr->itemInfo[i].equipState == equipHandFlag::both)
						{
							PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
							addAniToPlayerTurn(PlayerPtr, aniFlag::miningWall);
							break;
						}
					}
				}
			}
			else if (TileVehicle(touchX, touchY, PlayerZ()) != nullptr)
			{
				ItemPocket* tgtPocket = TileVehicle(touchX, touchY, PlayerZ())->partInfo[{touchX, touchY, PlayerZ() }].get();
				for (int i = 0; i < tgtPocket->itemInfo.size(); i++)
				{
					if (tgtPocket->itemInfo[i].checkFlag(itemFlag::VPART_DOOR_CLOSE))
					{
						tgtPocket->itemInfo[i].eraseFlag(itemFlag::VPART_DOOR_CLOSE);
						tgtPocket->itemInfo[i].addFlag(itemFlag::VPART_DOOR_OPEN);

						tgtPocket->itemInfo[i].eraseFlag(itemFlag::VPART_NOT_WALKABLE);

						if (tgtPocket->itemInfo[i].checkFlag(itemFlag::PROP_GAS_OBSTACLE_ON))
						{
							tgtPocket->itemInfo[i].eraseFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
							tgtPocket->itemInfo[i].addFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
						}

						tgtPocket->itemInfo[i].vehSprIndex += 16;
						PlayerPtr->updateVision(PlayerInfo().eyeSight);
					}
				}
			}
			else if (TileProp(touchX, touchY, PlayerZ()) != nullptr)
			{
				Prop* tgtProp = TileProp(touchX, touchY, PlayerZ());
				int tgtItemCode = tgtProp->leadItem.itemCode;
				if (tgtProp->leadItem.checkFlag(itemFlag::DOOR_CLOSE))
				{
					if (tgtProp->leadItem.checkFlag(itemFlag::PROP_WALKABLE) == false)
					{
						tgtProp->leadItem.eraseFlag(itemFlag::DOOR_CLOSE);
						tgtProp->leadItem.addFlag(itemFlag::DOOR_OPEN);

						tgtProp->leadItem.addFlag(itemFlag::PROP_WALKABLE);
						tgtProp->leadItem.eraseFlag(itemFlag::PROP_BLOCKER);
						tgtProp->leadItem.extraSprIndexSingle++;

						if (tgtProp->leadItem.checkFlag(itemFlag::PROP_GAS_OBSTACLE_ON))
						{
							tgtProp->leadItem.eraseFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
							tgtProp->leadItem.addFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
						}

						PlayerPtr->updateVision(PlayerInfo().eyeSight);
					}
				}
				else if (tgtProp->leadItem.checkFlag(itemFlag::WINDOW))
				{
					//클릭 = 열기 진행: 커튼 닫힘이면 커튼부터, 그 다음 창문 유리. 깨진 창문은 무동작
					ItemData& w = tgtProp->leadItem;
					if (w.checkFlag(itemFlag::WINDOW_BROKEN) == false)
					{
						if (w.checkFlag(itemFlag::CURTAIN) && w.checkFlag(itemFlag::CURTAIN_OPEN) == false)
						{
							w.addFlag(itemFlag::CURTAIN_OPEN);
							w.eraseFlag(itemFlag::PROP_BLOCKER);
							PlayerPtr->updateVision(PlayerInfo().eyeSight);
						}
						else if (w.checkFlag(itemFlag::WINDOW_OPEN) == false)
						{
							w.addFlag(itemFlag::WINDOW_OPEN);
							w.addFlag(itemFlag::PROP_WALKABLE);
							PlayerPtr->updateVision(PlayerInfo().eyeSight);
						}
					}
				}
				else if (tgtProp->leadItem.checkFlag(itemFlag::TREE))
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::felling);
				}
				else if (tgtProp->leadItem.itemCode == itemID::leverRL || tgtProp->leadItem.itemCode == itemID::leverUD)
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::propTurnOnOff);
                }
				else if (tgtProp->leadItem.itemCode == itemID::valveRL || tgtProp->leadItem.itemCode == itemID::valveUD)
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::propTurnOnOff);
				}
				else if (tgtProp->leadItem.itemCode == itemID::tactSwitchRL || tgtProp->leadItem.itemCode == itemID::tactSwitchUD)
				{
					if (tgtProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
					{
						PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
						addAniToPlayerTurn(PlayerPtr, aniFlag::propTurnOnOff);
					}
				}
				else if (tgtProp->leadItem.itemCode == itemID::delayR || tgtProp->leadItem.itemCode == itemID::delayU || tgtProp->leadItem.itemCode == itemID::delayL || tgtProp->leadItem.itemCode == itemID::delayD)
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::changePropDelay);
				}
				else if (tgtProp->leadItem.itemCode == itemID::gasolineGeneratorR
					|| tgtProp->leadItem.itemCode == itemID::gasolineGeneratorT
					|| tgtProp->leadItem.itemCode == itemID::gasolineGeneratorL
					|| tgtProp->leadItem.itemCode == itemID::gasolineGeneratorB)
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::propTurnOnOff);
				}
				else if (tgtProp->leadItem.itemCode == itemID::mechanicalWinch)
				{
					//기계식 윈치: 툭 치면 인접 롤업도어 체인 개폐(propTurnOnOff 람다에서 처리)
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::propTurnOnOff);
				}
				else if (tgtProp->leadItem.itemCode == itemID::campfire && tgtProp->energyPercent > 0.0f)
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::propTurnOnOff);
				}
				else if (tgtProp->leadItem.itemCode == itemID::autodoc)
				{
					if (tgtProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON))
					{
						if (touchX > PlayerX()) PlayerPtr->setDirection(0);
						else if (touchX < PlayerX()) PlayerPtr->setDirection(4);
						Corouter::start(actFunc::useAutodoc(touchX, touchY, PlayerZ()));
						click = false;
					}
				}
				else if (tgtProp->leadItem.itemCode == itemID::altarOfRehylion)
				{
					if (touchX > PlayerX()) PlayerPtr->setDirection(0);
					else if (touchX < PlayerX()) PlayerPtr->setDirection(4);
					if (GodPanel::ins() == nullptr)
					{
						new GodPanel(godFlag::rehylion, { touchX, touchY, PlayerZ() });
						click = false;
					}
				}
				else if (tgtProp->leadItem.checkFlag(itemFlag::CROP) && tgtProp->plantGrowthPercent >= 100.0)
				{
					PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
					addAniToPlayerTurn(PlayerPtr, aniFlag::harvesting);
				}
				else if (tgtProp->leadItem.pocketPtr != nullptr) //컨테이너 클릭 시 루팅창 오픈
				{
					if (touchX > PlayerX()) PlayerPtr->setDirection(0);
					else if (touchX < PlayerX()) PlayerPtr->setDirection(4);
					Point3 containerPos = { touchX, touchY, PlayerZ() };
					new Loot(tgtProp->leadItem.pocketPtr.get(), &(tgtProp->leadItem), containerPos);
					click = false;
				}
			}
			else if (TileEntity(touchX, touchY, PlayerZ()) != nullptr && TileEntity(touchX, touchY, PlayerZ())->entityInfo.relation == relationFlag::friendly)
			{
				new Dialogue();

			}
			else
			{
				PlayerPtr->startMove(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
			}
		}
		else if ((std::abs(touchX - PlayerX()) <= 1 && std::abs(touchY - PlayerY()) <= 1)
			&& TileProp(touchX, touchY, PlayerZ()) != nullptr
			&& TileProp(touchX, touchY, PlayerZ())->leadItem.checkFlag(itemFlag::CROP)
			&& TileProp(touchX, touchY, PlayerZ())->plantGrowthPercent >= 100.0)
		{
			PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
			addAniToPlayerTurn(PlayerPtr, aniFlag::harvesting);
		}
		else
		{
			PlayerPtr->setAStarDst(touchX, touchY);
		}
	}
	else//차량을 조종하는 상태일 경우
	{
		if (touchX == PlayerX() && touchY == PlayerY())
		{
			ctrlVeh = nullptr;
			barAct = actSet::null();
		}
	}
}