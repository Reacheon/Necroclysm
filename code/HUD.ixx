#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module HUD;

import std;
import util;
import GUI;
import errorBox;
import globalVar;
import wrapVar;
import constVar;
import textureVar;
import drawText;
import drawSprite;
import checkCursor;
import Loot;
import Sprite;
import Player;
import World;
import log;
import Equip;
import Profic;
import Mutation;
import Bionic;
import Craft;
import Light;
import Sticker;
import Monster;
import Lst;
import CoordSelect;
import Alchemy;
import turnWait;
import Vehicle;
import God;
import Prop;
import Map;
import globalTime;
import debugConsole;
import CoordSelect;
import SkillData;
import ContextMenu;
import mouseGrid;
import ItemPocket;
import ItemStack;
import Dialogue;
import Skill;
import Quest;

//HUD 객체는 멤버변수가 아니라 전역변수 사용하도록 만들 것
export class HUD : public GUI
{
private:
	inline static HUD* ptr = nullptr;
	bool isPopUp = false;
	vehFlag typeHUD = vehFlag::none;
	bool executedHold = false;
	bool isAdvancedMode = false;
	Point2 advancedModeGrid;

	SDL_Rect quickSlotPopBtn;
	
	bool isQuickSlotPop = false; //화면우측의 퀵슬롯이 오른쪽으로 팝업되었는지를 나타내는 bool 변수
	float popUpDist = 360;
	int quickSlotDist = 0;

	bool disableClickUp4Motion = false;

	bool dpadUpPressed = false;
	bool dpadDownPressed = false;
	bool dpadLeftPressed = false;
	bool dpadRightPressed = false;

	int barActCursorMoveDelay = 0;
	int rStickPressDelay = 0;
	int quickSlotCursor = -1;

	int fakeSTA = 0;
	int alphaSTA = 255;


	int delayR2 = 0;

	int dragQuickSlotTarget = -1;//HUD에서 스킬창 옮길 때 용도(Skill GUI랑 기능 동일)

public:
	SkillData* targetSkill; //GUI들이 가리키는 스킬
	HUD() : GUI(false)
	{
		prt(L"HUD instance was generated.\n");
		errorBox(ptr != nullptr, L"More than one Loot instance was generated.");
		ptr = this;
		changeXY(0, 0, false);
	}
	~HUD()
	{
		prt(L"[Error] HUD instance was destroyed.\n");
	}
	static HUD* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		/*
		* HUD는 다른 GUI 객체와 다르게 inputY에 의해서만 값이 적용받음
		* 팝업 용도이며 즉 center와 inputX의 값은 바꿔도 아무 영향 없음
		* inputY는 상대위치임 -10이면 평소 위치에서 10 픽셀만큼 위로 올라간 것과 같음
		*/

		letterbox = { 0, 0, 630, 140 };
		letterbox.x = (cameraW - letterbox.w) / 2;
		letterbox.y = cameraH - letterbox.h + 6 + inputY;
		letterbox.h = cameraH - letterbox.y + 6;

		for (int i = 0; i < 35; i++)
		{
			barButton[i] = { cameraW / 2 - 300 + (88 * (i % 7)), cameraH - 80 + inputY + (88 * (i / 7)), 72,72 };
			letterboxInButton[i] = { barButton[i].x + 3,  barButton[i].y + 3, 72 - 6, 72 - 6 };
		}
		letterboxPopUpButton = { letterbox.x + letterbox.w - 42 + 3, letterbox.y - 36 + 3,29,29 };
		//탭 버튼은 changeXY의 영향을 받지 않음
		tab = { cameraW - 128, 22, 120, 120 };
		tabSmallBox = { tab.x + 78, tab.y - 2,44,44 };


		quickSlotRegion = { 185, 0,380,45, };
		for (int i = 0; i < 8; i++) quickSlotBtn[i] = { 185 + 48 * i, 0, 44,45 };


		x = 0;
		y = inputY;
	}

	void drawGUI();
	void drawTab();
	void drawQuickSlot();
	void drawBarAct();
	void drawStatusEffects();
	void drawHoverItemInfo();
	void drawHoverEntityName();

	void clickDownGUI();
	void clickMotionGUI(int dx, int dy);
	void clickUpGUI();
	void clickRightGUI();
	void clickHoldGUI();
	void mouseWheel() 
	{
		//줌인 줌아웃
		if (event.wheel.y > 0)
		{
			zoomScale += 1;
			if (zoomScale > 5.0) zoomScale = 5;
		}
		else if (event.wheel.y < 0)
		{
			zoomScale -= 1;
			if (zoomScale < 1.0) zoomScale = 1;
		}
	}
	void mouseStep();

	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void gamepadStep();

	bool runAnimation(bool shutdown) override
	{
		//prt(L"HUD의 runAnimation이 실행되었다.\n");
		const int acc = 20;
		const int initSpeed = 4;
		if (getAniType() == aniFlag::popUpLetterbox)
		{
			//initDist가 정해져있어야 한다.
			//역으로 initDist에서 가속을 구할 수 있어야 함
			static std::array<float, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static float initDist = 0;
			float dstDist = popUpDist;

			addTimer();
			switch (getTimer())
			{
			case 1:
				initDist = 0;
				for (int i = 9; i >= 0; i--)
				{
					spd[9 - i] = acc * i + initSpeed;
					initDist += spd[9 - i];
				}
				//정규화
				for (int i = 0; i <= 9; i++)
				{
					spd[i] *= (dstDist / initDist);
				}
				break;
			default:
				changeXY(0, y - floor(spd[getTimer() - 2]), false);
				break;
			case 12:
				changeXY(0, -dstDist, false);
				resetTimer();
				actInput();
				setAniType(aniFlag::null);
				return true;
				break;
			}
		}
		else if (getAniType() == aniFlag::popDownLetterbox)
		{
			//initDist가 정해져있어야 한다.
			//역으로 initDist에서 가속을 구할 수 있어야 함
			static std::array<float, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static float initDist = 0;
			float dstDist = popUpDist;
			dstDist = -y;

			addTimer();
			switch (getTimer())
			{
			case 1:
				initDist = 0;
				for (int i = 0; i <= 9; i++)
				{
					spd[i] = acc * i + initSpeed;
					initDist += spd[i];
				}
				//정규화
				for (int i = 0; i <= 9; i++)
				{
					spd[i] *= (dstDist / initDist);
				}
				break;
			default:
				changeXY(0, y + floor(spd[getTimer() - 2]), false);
				break;
			case 12:
				changeXY(0, 0, false);
				resetTimer();
				actInput();
				setAniType(aniFlag::null);
				return true;
				break;
			}
		}
		else if (getAniType() == aniFlag::quickSlotPopLeft)
		{
			static std::array<float, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static float initDist = 0;
			float dstDist = 137;

			addTimer();
			switch (getTimer())
			{
			case 1:
				quickSlotDist = 0;
				initDist = 0;
				for (int i = 9; i >= 0; i--)
				{
					spd[9 - i] = acc * i + initSpeed;
					initDist += spd[9 - i];
				}
				//정규화
				for (int i = 0; i <= 9; i++)
				{
					spd[i] *= (dstDist / initDist);
				}
				break;
			default:
				quickSlotDist += floor(spd[getTimer() - 2]);
				changeXY(x, y, false);
				break;
			case 12:
				quickSlotDist = dstDist;
				changeXY(x, y, false);
				resetTimer();
				actInput();
				setAniType(aniFlag::null);
				return true;
				break;
			}
		}
		else if (getAniType() == aniFlag::quickSlotPopRight)
		{
			static std::array<float, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static float initDist = 0;
			float dstDist = 137;

			addTimer();
			switch (getTimer())
			{
			case 1:
				quickSlotDist = dstDist;
				initDist = 0;
				for (int i = 9; i >= 0; i--)
				{
					spd[9 - i] = acc * i + initSpeed;
					initDist += spd[9 - i];
				}
				//정규화
				for (int i = 0; i <= 9; i++)
				{
					spd[i] *= (dstDist / initDist);
				}
				break;
			default:
				quickSlotDist -= floor(spd[getTimer() - 2]);
				changeXY(x, y, false);
				break;
			case 12:
				quickSlotDist = 0;
				changeXY(x, y, false);
				resetTimer();
				actInput();
				setAniType(aniFlag::null);
				return true;
				break;
			}
		}

		return false;
	}

	void step()
	{
		if (GUI::getLastGUI() == this)
		{
			gamepadStep();
			mouseStep();
		}

		//현재 수련 중인 재능이 없을 경우 강제로 재능 창을 열음
		if (Profic::ins() == nullptr)
		{
			for (int i = 0; i < TALENT_SIZE; i++)
			{
				if (PlayerPtr->entityInfo.proficFocus[i] > 0) { break; }

				if (i == TALENT_SIZE - 1)
				{
					new Profic();
					Profic::ins()->setWarningIndex(1);
				}
			}
		}

		if (doPopUpSingleHUD == true)
		{
			executePopUpSingle();
			doPopUpSingleHUD = false;
		}
		else if (doPopDownHUD == true)
		{
			executePopDown();
			doPopDownHUD = false;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////

	void executePopUp()
	{
		isPopUp = true;
		popUpDist = ((barAct.size() - 1) / 7) * 89;
		deactInput();
		addAniUSetPlayer(this, aniFlag::popUpLetterbox);
	}
	void executePopDown()
	{
		barActCursor = -1;
		isPopUp = false;
		deactInput();
		addAniUSetPlayer(this, aniFlag::popDownLetterbox);
	}
	void executePopUpSingle()
	{
		isPopUp = true;
		deactInput();
		addAniUSetPlayer(this, aniFlag::popUpSingleLetterbox);
	}

	void clickLetterboxBtn(act inputAct)
	{
		bool popDownWhenEnd = true;
		switch (inputAct)
		{
		case act::inventory:
			new Equip();
			break;
		case act::profic:
			updateLog(L"#FFFFFF재능 창을 열었다.");
			new Profic();
			break;
		case act::mutation:
			updateLog(L"#FFFFFF돌연변이 창을 열었다.");
			new Mutation();
			break;
		case act::bionic:
			updateLog(L"#FFFFFF바이오닉 창을 열었다.");
			new Bionic();
			break;
		case act::quest:
			new Quest();
			break;
		case act::runMode:
			if (PlayerPtr->entityInfo.walkMode == walkFlag::walk) PlayerPtr->entityInfo.walkMode = walkFlag::run;
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::run) PlayerPtr->entityInfo.walkMode = walkFlag::crouch;
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::crouch) PlayerPtr->entityInfo.walkMode = walkFlag::crawl;
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::crawl) PlayerPtr->entityInfo.walkMode = walkFlag::walk;
			popDownWhenEnd = false;
			break;
		case act::skill:
			new Skill();
			break;
		case act::craft:
			updateLog(L"#FFFFFF조합 창을 열었다.");
			new Craft();
			break;
		case act::construct:
			updateLog(L"#FFFFFF삭제된 GUI이다.");
			break;
		case act::alchemy:
			updateLog(L"#FFFFFF연금술 창을 열었다.");
			new Alchemy();
			break;
		case act::turnLeft:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			if (myCar->wheelDir == ACW(myCar->bodyDir)) myCar->wheelDir = ACW2(myCar->bodyDir);
			else if (myCar->wheelDir == myCar->bodyDir) myCar->wheelDir = ACW(myCar->bodyDir);
			else if (myCar->wheelDir == CW(myCar->bodyDir)) myCar->wheelDir = myCar->bodyDir;
			else if (myCar->wheelDir == CW2(myCar->bodyDir)) myCar->wheelDir = CW(myCar->bodyDir);
			else
			{
				myCar->wheelDir = ACW2(myCar->bodyDir);
			}
			turnWait(1.0);
			myCar->updateSpr();
			break;
		}
		case act::turnRight:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			if (myCar->wheelDir == ACW2(myCar->bodyDir)) myCar->wheelDir = ACW(myCar->bodyDir);
			else if (myCar->wheelDir == ACW(myCar->bodyDir)) myCar->wheelDir = myCar->bodyDir;
			else if (myCar->wheelDir == myCar->bodyDir) myCar->wheelDir = CW(myCar->bodyDir);
			else if (myCar->wheelDir == CW(myCar->bodyDir)) myCar->wheelDir = CW2(myCar->bodyDir);
			else
			{
				myCar->wheelDir = CW2(myCar->bodyDir);
			}
			turnWait(1.0);
			myCar->updateSpr();
			break;
		}
		case act::wait:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			turnWait(1.0);
			break;
		}
		case act::startEngine:
			break;
		case act::stopEngine:
			break;
		case act::shiftGear:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			if (myCar->vehType == vehFlag::car)
			{
				if (myCar->gearState == gearFlag::park) myCar->gearState = gearFlag::reverse;
				else if (myCar->gearState == gearFlag::reverse) myCar->gearState = gearFlag::neutral;
				else if (myCar->gearState == gearFlag::neutral) myCar->gearState = gearFlag::drive;
				else myCar->gearState = gearFlag::park;
			}
			else
			{
				if (myCar->gearState == gearFlag::park) myCar->gearState = gearFlag::drive;
				else if (myCar->gearState == gearFlag::reverse) myCar->gearState = gearFlag::drive;
				else if (myCar->gearState == gearFlag::neutral) myCar->gearState = gearFlag::drive;
				else myCar->gearState = gearFlag::reverse;
			}
			turnWait(1.0);
			break;
		}
		case act::accel:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			if (myCar->gearState == gearFlag::drive) myCar->accVec = scalarMultiple(dir16ToVec(myCar->wheelDir), 7.0);
			else if (myCar->gearState == gearFlag::reverse) myCar->accVec = scalarMultiple(dir16ToVec(reverse(myCar->wheelDir)), 7.0);
			turnWait(1.0);
			break;
		}
		case act::brake:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			myCar->rpmState = 0;
			myCar->singleRailMoveCounter = 0;
			myCar->singleRailSpdVal = 0;

			if (myCar->spdVec.getLength() != 0)
			{
				myCar->accVec = getZeroVec();
				myCar->turnOnBrake = true;
				float delSpd = 1.0;
				float massCoeff = 1.0;
				float frictionCoeff = 10.0;
				delSpd = frictionCoeff / massCoeff;
				Vec3 brakeNormDirVec = scalarMultiple(myCar->spdVec, -1).getNormDirVec();
				Vec3 brakeVec = scalarMultiple(brakeNormDirVec, delSpd);

				if (myCar->spdVec.getLength() < brakeVec.getLength())
				{
					myCar->spdVec = getZeroVec();
				}
				else
				{
					myCar->spdVec.addVec(brakeVec);
				}
			}
			turnWait(1.0);
			break;
		}
		case act::god:
			updateLog(L"#FFFFFF신앙 창을 열었다.");
			new God(playerGod);
			break;
		case act::map:
			updateLog(L"#FFFFFF지도 창을 열었다.");
			new Map();
			break;
		case act::test:
			debugConsole();
			break;
		case act::collectiveLever:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			myCar->collectiveState++;
			if (myCar->collectiveState >= 2) myCar->collectiveState = -1;

			if (myCar->collectiveState == 1) myCar->spdVec.compZ = 1;
			else if (myCar->collectiveState == -1) myCar->spdVec.compZ = -1;
			else myCar->spdVec.compZ = 0;

			break;
		}
		case act::cyclicLever:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			myCar->cyclicState++;
			if (myCar->cyclicState >= 8) myCar->cyclicState = -1;
			break;
		}
		case act::rpmLever:
		{
			Vehicle* myCar = (Vehicle*)ctrlVeh;
			myCar->rpmState++;
			if (myCar->rpmState >= 7) myCar->rpmState = 0;
			break;
		}
		case act::tailRotorPedalL:
		{
			Vehicle* myHeli = (Vehicle*)(ctrlVeh);
			myHeli->rotate(ACW(myHeli->bodyDir));
			break;
		}
		case act::tailRotorPedalR:
		{
			Vehicle* myHeli = (Vehicle*)(ctrlVeh);
			myHeli->rotate(CW(myHeli->bodyDir));
			break;
		}
		case act::closeDoor:
		{
			CORO(closeDoor(PlayerX(), PlayerY(), PlayerZ()));
			break;
		}
		case act::headlight:
		{
			Vehicle* myCar = (Vehicle*)(ctrlVeh);
			if (myCar->headlightOn)
			{
				myCar->headlightOn = false;

				for (auto it = myCar->partInfo.begin(); it != myCar->partInfo.end(); it++)
				{
					for (int i = 0; i < it->second->itemInfo.size(); i++)
					{
						if (it->second->itemInfo[i].checkFlag(itemFlag::HEADLIGHT))
						{
							if (it->second->itemInfo[i].lightPtr != nullptr)
							{
								it->second->itemInfo[i].lightPtr.reset();
							}
						}
					}
				}

			}
			else
			{
				myCar->headlightOn = true;

				for(auto it = myCar->partInfo.begin(); it != myCar->partInfo.end(); it++)
				{
					for (int i = 0; i < it->second->itemInfo.size(); i++)
					{
						if (it->second->itemInfo[i].checkFlag(itemFlag::HEADLIGHT))
						{
							if (it->second->itemInfo[i].lightPtr == nullptr)
							{
								it->second->itemInfo[i].lightPtr = std::make_unique<Light>(it->first[0], it->first[1], myCar->getGridZ(), 12, 120, col::white, myCar->bodyDir);
							}
						}
					}
                }
			}

			PlayerPtr->updateVision();
			break;
		}
		default:
			updateLog(L"#FFFFFF알수없는 레터박스 버튼이 눌렸다.");
		}

		if (popDownWhenEnd == true && ctrlVeh == nullptr)
		{
			if (y != 0) executePopDown();
		}
	}
	void tileTouch(int touchX, int touchY) //일반 타일 터치
	{
		if (ctrlVeh == nullptr)//차량 조종 중이 아닐 경우
		{
			//화면에 있는 아이템 터치
			if (touchX == PlayerX() && touchY == PlayerY()) //자신 위치 터치
			{

				if (TileItemStack(touchX, touchY, PlayerZ()) != nullptr)
				{
					prt(L"루팅창 오픈 함수 실행\n");
					ItemStack* targetStack = TileItemStack(PlayerX(), PlayerY(), PlayerZ());
					new Loot(targetStack);
					click = false;
				}
				else if (TileVehicle(touchX, touchY, PlayerZ()) != nullptr)
				{
					Vehicle* belowVehicle = TileVehicle(touchX, touchY, PlayerZ());
					bool findController = false;
					prt(L"below prop의 사이즈는 %d이다.\n", belowVehicle->partInfo[{touchX, touchY}]->itemInfo.size());
					for (int i = 0; i < belowVehicle->partInfo[{touchX, touchY}]->itemInfo.size(); i++)
					{
						if (belowVehicle->partInfo[{touchX, touchY}]->itemInfo[i].itemCode == 99)//차량 조종장치
						{
							if (ctrlVeh == nullptr)
							{
								ctrlVeh = belowVehicle;
								barAct = actSet::vehicle;
								typeHUD = vehFlag::car;
							}
							else
							{
								ctrlVeh = nullptr;
								barAct = actSet::null;
								typeHUD = vehFlag::none;
							}
						}
						else if (belowVehicle->partInfo[{touchX, touchY}]->itemInfo[i].itemCode == 311)//헬기 조종장치
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
								barAct = actSet::null;
								typeHUD = vehFlag::none;;
							}
						}
						else if (belowVehicle->partInfo[{touchX, touchY}]->itemInfo[i].itemCode == 313)//열차 조종장치
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
								barAct = actSet::null;
								typeHUD = vehFlag::none;;
							}
						}
						else if (belowVehicle->partInfo[{touchX, touchY}]->itemInfo[i].itemCode == itemRefCode::minecartController) //열차 조종장치
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
								barAct = actSet::null;
								typeHUD = vehFlag::none;;
							}
						}
					}
				}
				else
				{
					if (TileProp(touchX, touchY, PlayerZ()) != nullptr)
					{
						Prop* tgtProp = TileProp(touchX, touchY, PlayerZ());
						int tgtItemCode = tgtProp->leadItem.itemCode;
						if (tgtProp->leadItem.checkFlag(itemFlag::UPSTAIR))
						{
							if (TileFloor(PlayerX(), PlayerY(), PlayerZ() + 1) == 0)
							{
								updateLog(L"#FFFFFF이 계단의 위층에 바닥이 존재하지 않는다.");
							}
							else
							{
								updateLog(L"#FFFFFF계단을 올라갔다.");

								EntityPtrMove({ PlayerX(), PlayerY(), PlayerZ() }, { PlayerX(), PlayerY(), PlayerZ() + 1 });

								PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
								PlayerPtr->updateMinimap();

								Prop* upProp = TileProp(touchX, touchY, PlayerZ());
								if (upProp == nullptr)
								{
									createProp({ PlayerX(), PlayerY(), PlayerZ() }, 299);//하강계단
								}
							}
						}
						else if (tgtProp->leadItem.checkFlag(itemFlag::DOWNSTAIR))
						{
							if (TileWall(PlayerX(), PlayerY(), PlayerZ() + 1) != 0)
							{
								updateLog(L"#FFFFFF내려가는 계단이 벽으로 막혀있다.");
							}
							else
							{
								updateLog(L"#FFFFFF계단을 내려갔다.");

								EntityPtrMove({ PlayerX(), PlayerY(), PlayerZ() }, { PlayerX(), PlayerY(), PlayerZ() - 1 });

								PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
								PlayerPtr->updateMinimap();

								Prop* downProp = TileProp(touchX, touchY, PlayerZ());
								if (downProp == nullptr)
								{
									createProp({ PlayerX(), PlayerY(), PlayerZ() }, 298);//상승계단
								}
							}
						}
					}
				}
			}
			else if ((std::abs(touchX - PlayerX()) <= 1 && std::abs(touchY - PlayerY()) <= 1) && isWalkable({ touchX, touchY, PlayerZ() }) == false)//1칸 이내
			{
				if (TileWall(touchX, touchY, PlayerZ()) != 0)
				{
					auto ePtr = PlayerPtr->getEquipPtr();
					for (int i = 0; i < ePtr->itemInfo.size(); i++)
					{
						if (ePtr->itemInfo[i].itemCode == itemRefCode::pickaxe)
						{
							if (ePtr->itemInfo[i].equipState == equipHandFlag::both)
							{
								PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
								addAniUSetPlayer(PlayerPtr, aniFlag::miningWall);
								break;
							}
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

							PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
						}
					}
					else if (tgtProp->leadItem.checkFlag(itemFlag::UPSTAIR))
					{
						if (TileFloor(PlayerX(), PlayerY(), PlayerZ() + 1) == 0)
						{
							updateLog(L"#FFFFFF이 계단의 위층에 바닥이 존재하지 않는다.");
						}
						else
						{
							updateLog(L"#FFFFFF계단을 올라갔다.");
							PlayerPtr->addGridZ(1);
						}
					}
					else if (tgtProp->leadItem.checkFlag(itemFlag::DOWNSTAIR))
					{
					}
					else if (tgtProp->leadItem.checkFlag(itemFlag::TREE))
					{
						PlayerPtr->setDirection(coord2Dir(touchX - PlayerX(), touchY - PlayerY()));
						addAniUSetPlayer(PlayerPtr, aniFlag::felling);
					}
					else if (tgtItemCode == 213 || tgtItemCode == 218)//불교 제단
					{
						new God(godFlag::buddha);
					}
					else if (tgtItemCode == 214)//천공로
					{
						new God(godFlag::yudi);
					}
					else if (tgtItemCode == 216)//십자가
					{
						new God(godFlag::jesus);
					}
					else if (tgtItemCode == 217)//토리이
					{
						new God(godFlag::amaterasu);
					}
					else if (tgtItemCode == 219)//석판
					{
						new God(godFlag::ra);
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
				barAct = actSet::null;
			}
		}
	}
	Corouter closeDoor(int cx, int cy, int cz)
	{
		int doorNumber = 0;
		std::vector<std::array<int, 2>> doorList;

		for (int dir = 0; dir <= 7; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);
			if (TileProp(cx + dx, cy + dy, cz) != nullptr)
			{
				Prop* instlPtr = TileProp(cx + dx, cy + dy, cz);
				if (instlPtr->leadItem.checkFlag(itemFlag::DOOR_OPEN))
				{
					doorNumber++;
					doorList.push_back({ cx + dx, cy + dy });
				}
			}
		}

		auto closeDoorCoord = [=](int inputX, int inputY, int inputZ)
			{
				Prop* tgtProp = TileProp(inputX, inputY, inputZ);
				tgtProp->leadItem.eraseFlag(itemFlag::DOOR_OPEN);
				tgtProp->leadItem.addFlag(itemFlag::DOOR_CLOSE);

				if (tgtProp->leadItem.checkFlag(itemFlag::PROP_GAS_OBSTACLE_OFF))
				{
					tgtProp->leadItem.eraseFlag(itemFlag::PROP_GAS_OBSTACLE_OFF);
					tgtProp->leadItem.addFlag(itemFlag::PROP_GAS_OBSTACLE_ON);
				}

				tgtProp->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
				tgtProp->leadItem.addFlag(itemFlag::PROP_BLOCKER);
				tgtProp->leadItem.extraSprIndexSingle--;
				PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
			};

		if (doorNumber == 1)
		{
			closeDoorCoord(doorList[0][0], doorList[0][1], cz);
		}
		else //문이 2개 이상일 경우
		{
			new CoordSelect(L"닫을 문을 선택해주세요.", doorList);
			co_await std::suspend_always();

			std::wstring targetStr = coAnswer;
			int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);
			int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);


			Prop* tgtProp = TileProp(targetX, targetY, cz);
			tgtProp->leadItem.eraseFlag(itemFlag::DOOR_OPEN);
			tgtProp->leadItem.addFlag(itemFlag::DOOR_CLOSE);

			tgtProp->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
			tgtProp->leadItem.addFlag(itemFlag::PROP_BLOCKER);
			tgtProp->leadItem.extraSprIndexSingle--;
			PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
		}
	};

	void quickSlotToggle()
	{
		if (isPopUp == false)
		{
			if (isQuickSlotPop)
			{
				isQuickSlotPop = false;
				deactInput();
				addAniUSetPlayer(this, aniFlag::quickSlotPopRight);
			}
			else
			{
				isQuickSlotPop = true;
				deactInput();
				addAniUSetPlayer(this, aniFlag::quickSlotPopLeft);
			}
		}
	}

	void executeTab()
	{
		int dx, dy = 0;
		for (int dir = 0; dir < 8; dir++)
		{
			dir2Coord(dir, dx, dy);
			if (TileEntity(PlayerX() + dx, PlayerY() + dy, PlayerZ()) != nullptr)
			{
				PlayerPtr->startAtk(PlayerX() + dx, PlayerY() + dy, PlayerZ());
				turnWait(1.0);
				break;
			}
		}
	}

	void openContextMenu(Point2 targetGrid)
	{
		std::vector<act> inputOptions;
		//문닫기 추가
		if (TileProp(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			Prop* instlPtr = TileProp(targetGrid.x, targetGrid.y, PlayerZ());
			if (instlPtr->leadItem.checkFlag(itemFlag::DOOR_OPEN))
			{
				inputOptions.push_back(act::closeDoor);
			}
		}

		//열기 추가
		if (TileVehicle(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			Vehicle* vPtr = TileVehicle(targetGrid.x, targetGrid.y, PlayerZ());

			for (int i = 0; i < vPtr->partInfo[{targetGrid.x, targetGrid.y}]->itemInfo.size(); i++)
			{
				if (vPtr->partInfo[{targetGrid.x, targetGrid.y}]->itemInfo[i].checkFlag(itemFlag::POCKET))
				{
					inputOptions.push_back(act::unbox);
					break;
				}
			}
		}

		//당기기 추가
		if (TileVehicle(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			inputOptions.push_back(act::pull);
		}

		inputOptions.push_back(act::inspect);

		Point2 windowCoord;
		if (option::inputMethod == input::mouse || option::inputMethod == input::touch)
		{
			windowCoord = { getMouseXY().x, getMouseXY().y };
		}
		else if (option::inputMethod == input::gamepad)
		{
			SDL_Rect dst;
			dst.x = cameraW / 2 + zoomScale * ((16 * targetGrid.x + 8) - cameraX) - ((16 * zoomScale) / 2) + 16 * zoomScale;
			dst.y = cameraH / 2 + zoomScale * ((16 * targetGrid.y + 8) - cameraY) - ((16 * zoomScale) / 2) + 16 * zoomScale;
			windowCoord = { dst.x, dst.y };
		}

		//수영하기 추가
		if (itemDex[TileFloor(targetGrid.x, targetGrid.y, PlayerZ())].checkFlag(itemFlag::WATER_DEEP))
		{
			inputOptions.push_back(act::swim);
		}

		//등반 추가
		if (itemDex[TileWall(targetGrid.x, targetGrid.y, PlayerZ())].checkFlag(itemFlag::CAN_CLIMB))
		{
			inputOptions.push_back(act::climb);
		}

		//승마 추가
		if (TileEntity(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			inputOptions.push_back(act::ride);
		}

		if (TileVehicle(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			Vehicle* vPtr = TileVehicle(targetGrid.x, targetGrid.y, PlayerZ());
			if (vPtr->partInfo[{targetGrid.x, targetGrid.y}]->itemInfo.size()>0)
			{
				inputOptions.push_back(act::vehicleRepair);
				inputOptions.push_back(act::vehicleDetach);
			}
		}

		if (TileProp(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			Prop* vPtr = TileProp(targetGrid.x, targetGrid.y, PlayerZ());
			if (vPtr->leadItem.pocketPtr != nullptr)
			{
				inputOptions.push_back(act::unbox);
			}
		}

		if (TileFloor(targetGrid.x, targetGrid.y, PlayerZ()) == itemRefCode::shallowFreshWater ||
			TileFloor(targetGrid.x, targetGrid.y, PlayerZ()) == itemRefCode::deepFreshWater ||
			TileFloor(targetGrid.x, targetGrid.y, PlayerZ()) == itemRefCode::shallowSeaWater ||
			TileFloor(targetGrid.x, targetGrid.y, PlayerZ()) == itemRefCode::deepSeaWater)
		{
			for (int i = 0; i < PlayerPtr->getEquipPtr()->itemInfo.size(); i++)
			{
				if (PlayerPtr->getEquipPtr()->itemInfo[i].checkFlag(itemFlag::CONTAINER_LIQ))
				{
					inputOptions.push_back(act::drawLiquid);
					break;
                }

				if (PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr != nullptr && 
					PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo.size()>0)
				{
					for (int j = 0; j < PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo.size(); j++)
					{
						if (PlayerPtr->getEquipPtr()->itemInfo[i].pocketPtr.get()->itemInfo[j].checkFlag(itemFlag::CONTAINER_LIQ))
						{
							inputOptions.push_back(act::drawLiquid);
							i = PlayerPtr->getEquipPtr()->itemInfo.size();//이중루프 탈출
							break;
                        }
					}
				}
			}
		}

		new ContextMenu(windowCoord.x, windowCoord.y, targetGrid.x, targetGrid.y, inputOptions);
	}
};