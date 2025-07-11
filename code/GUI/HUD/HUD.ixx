﻿#include <SDL3/SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module HUD;

import std;
import util;
import GUI;
import globalVar;
import wrapVar;
import constVar;
import textureVar;
import Loot;
import Sprite;
import Player;
import World;
import log;
import Equip;
import Profic;
import Craft;
import Light;
import Sticker;
import Lst;
import CoordSelect;
import Alchemy;
import turnWait;
import Vehicle;
import Prop;
import Map;
import globalTime;
import debugConsole;
import CoordSelect;
import SkillData;
import ContextMenu;
import ItemPocket;
import ItemStack;
import Dialogue;
import Skill;
import Quest;
import Sleep;
import Aim;

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

	int fakeHunger = hunger;
	int fakeThirst = thirst;
	int fakeFatigue = fatigue;

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
	void drawQuest();

	void clickDownGUI();
	void clickMotionGUI(int dx, int dy);
	void clickUpGUI();
	void clickRightGUI();
	void clickHoldGUI();
	void keyUpGUI();

	void mouseWheel() 
	{
		bool partSelect = TileEntity(getAbsMouseGrid().x, getAbsMouseGrid().y, PlayerZ()) != nullptr 
			&& (std::abs(getAbsMouseGrid().x-PlayerX()) == 1 || std::abs(getAbsMouseGrid().y - PlayerY()) == 1);
			

		if (partSelect==false)
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
		else
		{

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

	void step();

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
			break;
		case act::bionic:
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
			if (ctrlVeh->wheelDir == ACW(ctrlVeh->bodyDir)) ctrlVeh->wheelDir = ACW2(ctrlVeh->bodyDir);
			else if (ctrlVeh->wheelDir == ctrlVeh->bodyDir) ctrlVeh->wheelDir = ACW(ctrlVeh->bodyDir);
			else if (ctrlVeh->wheelDir == CW(ctrlVeh->bodyDir)) ctrlVeh->wheelDir = ctrlVeh->bodyDir;
			else if (ctrlVeh->wheelDir == CW2(ctrlVeh->bodyDir)) ctrlVeh->wheelDir = CW(ctrlVeh->bodyDir);
			else
			{
				ctrlVeh->wheelDir = ACW2(ctrlVeh->bodyDir);
			}
			turnWait(1.0);
			ctrlVeh->updateSpr();
			break;
		}
		case act::turnRight:
		{
			if (ctrlVeh->wheelDir == ACW2(ctrlVeh->bodyDir)) ctrlVeh->wheelDir = ACW(ctrlVeh->bodyDir);
			else if (ctrlVeh->wheelDir == ACW(ctrlVeh->bodyDir)) ctrlVeh->wheelDir = ctrlVeh->bodyDir;
			else if (ctrlVeh->wheelDir == ctrlVeh->bodyDir) ctrlVeh->wheelDir = CW(ctrlVeh->bodyDir);
			else if (ctrlVeh->wheelDir == CW(ctrlVeh->bodyDir)) ctrlVeh->wheelDir = CW2(ctrlVeh->bodyDir);
			else
			{
				ctrlVeh->wheelDir = CW2(ctrlVeh->bodyDir);
			}
			turnWait(1.0);
			ctrlVeh->updateSpr();
			break;
		}
		case act::wait:
		{
			turnWait(1.0);
			break;
		}
		case act::startEngine:
		{
			ctrlVeh->isEngineOn = true;
			auto it = std::find(barAct.begin(), barAct.end(), act::startEngine);
			if (it != barAct.end()) *it = act::stopEngine;
			break;
		}
		case act::stopEngine:
		{
			ctrlVeh->isEngineOn = false;
			auto it = std::find(barAct.begin(), barAct.end(), act::stopEngine);
			if (it != barAct.end()) *it = act::startEngine;
			break;
		}
		case act::shiftGear:
		{
			if (ctrlVeh->vehType == vehFlag::car)
			{
				if (ctrlVeh->gearState == gearFlag::park) ctrlVeh->gearState = gearFlag::reverse;
				else if (ctrlVeh->gearState == gearFlag::reverse) ctrlVeh->gearState = gearFlag::neutral;
				else if (ctrlVeh->gearState == gearFlag::neutral) ctrlVeh->gearState = gearFlag::drive;
				else ctrlVeh->gearState = gearFlag::park;
			}
			else
			{
				if (ctrlVeh->gearState == gearFlag::park) ctrlVeh->gearState = gearFlag::drive;
				else if (ctrlVeh->gearState == gearFlag::reverse) ctrlVeh->gearState = gearFlag::drive;
				else if (ctrlVeh->gearState == gearFlag::neutral) ctrlVeh->gearState = gearFlag::drive;
				else ctrlVeh->gearState = gearFlag::reverse;
			}
			turnWait(1.0);
			break;
		}
		case act::accel:
		{
			if (ctrlVeh->isEngineOn)
			{
				if (ctrlVeh->gearState == gearFlag::drive) ctrlVeh->accVec = scalarMultiple(dir16ToVec(ctrlVeh->wheelDir), 7.0);
				else if (ctrlVeh->gearState == gearFlag::reverse) ctrlVeh->accVec = scalarMultiple(dir16ToVec(reverse(ctrlVeh->wheelDir)), 7.0);
				turnWait(1.0);
			}
			else updateLog(L"차량에 시동이 걸리지 않은 상태다.");
			break;
		}
		case act::brake:
		{
			ctrlVeh->rpmState = 0;
			ctrlVeh->singleRailMoveCounter = 0;
			ctrlVeh->singleRailSpdVal = 0;

			if (ctrlVeh->spdVec.getLength() != 0)
			{
				ctrlVeh->accVec = getZeroVec();
				ctrlVeh->turnOnBrake = true;
				float delSpd = 1.0;
				float massCoeff = 1.0;
				float frictionCoeff = 10.0;
				delSpd = frictionCoeff / massCoeff;
				Vec3 brakeNormDirVec = scalarMultiple(ctrlVeh->spdVec, -1).getNormDirVec();
				Vec3 brakeVec = scalarMultiple(brakeNormDirVec, delSpd);

				if (ctrlVeh->spdVec.getLength() < brakeVec.getLength())
				{
					ctrlVeh->spdVec = getZeroVec();
				}
				else
				{
					ctrlVeh->spdVec.addVec(brakeVec);
				}
			}
			turnWait(1.0);
			break;
		}
		case act::god:
			updateLog(L"#FFFFFF신앙 창을 열었다.");
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
			ctrlVeh->collectiveState++;
			if (ctrlVeh->collectiveState >= 2) ctrlVeh->collectiveState = -1;

			if (ctrlVeh->collectiveState == 1) ctrlVeh->spdVec.compZ = 1;
			else if (ctrlVeh->collectiveState == -1) ctrlVeh->spdVec.compZ = -1;
			else ctrlVeh->spdVec.compZ = 0;

			break;
		}
		case act::cyclicLever:
		{
			ctrlVeh->cyclicState++;
			if (ctrlVeh->cyclicState >= 8) ctrlVeh->cyclicState = -1;
			break;
		}
		case act::rpmLever:
		{
			ctrlVeh->rpmState++;
			if (ctrlVeh->rpmState >= 7) ctrlVeh->rpmState = 0;
			break;
		}
		case act::tailRotorPedalL:
		{
			ctrlVeh->rotate(ACW(ctrlVeh->bodyDir));
			break;
		}
		case act::tailRotorPedalR:
		{
			ctrlVeh->rotate(CW(ctrlVeh->bodyDir));
			break;
		}
		case act::closeDoor:
		{
			CORO(closeDoor(PlayerX(), PlayerY(), PlayerZ()));
			break;
		}
		case act::headlight:
		{
			if (ctrlVeh->headlightOn)
			{
				ctrlVeh->headlightOn = false;

				for (auto it = ctrlVeh->partInfo.begin(); it != ctrlVeh->partInfo.end(); it++)
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
				ctrlVeh->headlightOn = true;

				for(auto it = ctrlVeh->partInfo.begin(); it != ctrlVeh->partInfo.end(); it++)
				{
					for (int i = 0; i < it->second->itemInfo.size(); i++)
					{
						if (it->second->itemInfo[i].checkFlag(itemFlag::HEADLIGHT))
						{
							if (it->second->itemInfo[i].lightPtr == nullptr)
							{
								it->second->itemInfo[i].lightPtr = std::make_unique<Light>(it->first[0], it->first[1], ctrlVeh->getGridZ(), 12, 120, col::white, ctrlVeh->bodyDir);
							}
						}
					}
                }
			}

			PlayerPtr->updateVision();
			PlayerPtr->updateMinimap();
			break;
		}
		case act::sleep:
		{
			new Sleep();
			break;
		}
		default:
			updateLog(L"#FFFFFF알수없는 레터박스 버튼이 눌렸다.");
			break;
		}

		if (popDownWhenEnd == true && ctrlVeh == nullptr)
		{
			if (y != 0) executePopDown();
		}
	}
	void tileTouch(int touchX, int touchY); //일반 타일 터치

	void findAndOpenAim()
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
								if (equipInfo[j].itemCode == itemRefCode::arrowQuiver)
								{
									if (equipInfo[j].pocketPtr.get()->getPocketNumber() > 0)
									{
										equipInfo[j].pocketPtr.get()->transferItem(equipInfo[i].pocketPtr.get(), 0, 1);
										updateLog(col2Str(col::white) + L"당신은 화살을 시위에 걸었다.");
										new Aim();
										break;
									}
								}
								if (j == equipInfo.size() - 1) updateLog(col2Str(col::white) + L"현재 가지고 있는 화살이 없다.");
							}
						}
					}
					else if (equipInfo[i].checkFlag(itemFlag::CROSSBOW))
					{
						if (equipInfo[i].pocketPtr.get()->getPocketNumber() > 0) new Aim();
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
	};

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
			windowCoord = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
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
		if (TileEntity(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr && TileEntity(targetGrid.x, targetGrid.y, PlayerZ())->entityInfo.entityCode == entityRefCode::horse)
		{
			inputOptions.push_back(act::ride);
		}

		//수면 추가
		if (TileEntity(targetGrid.x, targetGrid.y, PlayerZ()) == PlayerPtr)
		{
			inputOptions.push_back(act::sleep);
		}

		if (TileVehicle(targetGrid.x, targetGrid.y, PlayerZ()) != nullptr)
		{
			Vehicle* vPtr = TileVehicle(targetGrid.x, targetGrid.y, PlayerZ());
			std::vector<ItemData>& tgtPocket = vPtr->partInfo[{targetGrid.x, targetGrid.y}]->itemInfo;
			if (tgtPocket.size()>0)
			{
				inputOptions.push_back(act::vehicleRepair);
				inputOptions.push_back(act::vehicleDetach);
			}

			for(int i=0; i<tgtPocket.size(); i++)
			{
				if (tgtPocket[i].checkFlag(itemFlag::VPART_DOOR_OPEN))
				{
					inputOptions.push_back(act::closeDoor);
					break;
				}
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

			inputOptions.push_back(act::drinkFloorWater);

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