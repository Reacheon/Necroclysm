#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module HUD;

import std;
import util;
import GUI;
import errorBox;
import globalVar;
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
import Talent;
import Mutation;
import Bionic;
import Craft;
import Light;
import Sticker;
import Monster;
import Lst;
import Aim;
import CoordSelect;
import Alchemy;
import turnWait;
import Vehicle;
import God;
import Prop;
import Map;
import globalTime;
import debugConsole;
import updateBarAct;
import CoordSelect;
import SkillData;
import ContextMenu;
import mouseGrid;


//HUD 객체는 멤버변수가 아니라 전역변수 사용하도록 만들 것
export class HUD : public GUI
{
private:
	inline static HUD* ptr = nullptr;
	bool isPopUp = false;
	SDL_Rect tabSmallBox = { tab.x + 78, tab.y - 2,44,44 };
	vehFlag typeHUD = vehFlag::none;
	bool executedHold = false;
	bool isAdvancedMode = false;
	Point2 advancedModeGrid;

	SDL_Rect quickSlotPopBtn;
	SDL_Rect quickSlotBtn[8];
	bool isQuickSlotPop = false; //화면우측의 퀵슬롯이 오른쪽으로 팝업되었는지를 나타내는 bool 변수
	float popUpDist = 360;
	int quickSlotDist = 0;

	bool disableClickUp4Motion = false;

	bool dpadUpPressed = false;
	bool dpadDownPressed = false;
	bool dpadLeftPressed = false;
	bool dpadRightPressed = false;
	int dpadDelay = 0;
	int barActCursorMoveDelay = 0;
	int rStickPressDelay = 0;
	int quickSlotCursor = -1;

	int delayR2 = 0;
public:
	SkillData* targetSkill; //GUI들이 가리키는 스킬
	HUD() : GUI(false)
	{
		prt(L"HUD instance was generated.\n");
		errorBox(ptr != nullptr, "More than one Loot instance was generated.");
		ptr = this;
		if (inputType != input::keyboard)
		{
			changeXY(0, 0, false);
		}
		else
		{
			changeXY(0, 82, false);
		}

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


		quickSlotRegion = { cameraW - 1 - 42 - quickSlotDist,cameraH / 2 - 177,180,358, };
		quickSlotPopBtn = { quickSlotRegion.x, quickSlotRegion.y, 43, 38 };
		for (int i = 0; i < 8; i++)
		{
			quickSlotBtn[i] = { quickSlotRegion.x, quickSlotRegion.y + 40 * (i + 1), 180,38 };
		}


		x = 0;
		y = inputY;
	}

	void drawGUI();
	void drawTab();
	void drawQuickSlot();
	void drawBarAct();

	void clickDownGUI();
	void clickMotionGUI(int dx, int dy);
	void clickUpGUI();
	void clickRightGUI();
	void clickHoldGUI();
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
		gamepadStep();
		mouseStep();

		if (GUI::getLastGUI() == this)
		{
			if (inputType == input::mouse || inputType == input::touch)
			{
				whiteMarkerCoord.x = getAbsMouseGrid().x;
				whiteMarkerCoord.y = getAbsMouseGrid().y;
				whiteMarkerCoord.z = Player::ins()->getGridZ();
			}
			else if (inputType == input::gamepad)
			{
				__int16 leftX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
				__int16 leftY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

				int tgtX = Player::ins()->getGridX();
				int tgtY = Player::ins()->getGridY();

				if (leftX > TOLERANCE_LSTICK) tgtX += 1;
				if (leftX < -TOLERANCE_LSTICK) tgtX -= 1;
				if (leftY > TOLERANCE_LSTICK) tgtY += 1;
				if (leftY < -TOLERANCE_LSTICK) tgtY -= 1;

				if (!(tgtX == Player::ins()->getGridX() && tgtY == Player::ins()->getGridY()))
				{
					whiteMarkerCoord.x = tgtX;
					whiteMarkerCoord.y = tgtY;
					whiteMarkerCoord.z = Player::ins()->getGridZ();
				}
				else whiteMarkerCoord.z = std::numeric_limits<int>::max();
			}
		}


		//현재 수련 중인 재능이 없을 경우 강제로 재능 창을 열음
		if (Talent::ins() == nullptr)
		{
			for (int i = 0; i < TALENT_SIZE; i++)
			{
				if (Player::ins()->getTalentFocus(i) > 0) { break; }

				if (i == TALENT_SIZE - 1)
				{
					new Talent();
					Talent::ins()->setWarningIndex(1);
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
		case act::talent:
			updateLog(L"#FFFFFF재능 창을 열었다.");
			new Talent();
			break;
		case act::mutation:
			updateLog(L"#FFFFFF돌연변이 창을 열었다.");
			new Mutation();
			break;
		case act::bionic:
			updateLog(L"#FFFFFF바이오닉 창을 열었다.");
			new Bionic();
			break;
		case act::runMode:
			if (Player::ins()->getSpriteInfimum() == sprInf::walk) Player::ins()->setSpriteInfimum(sprInf::run);
			else if (Player::ins()->getSpriteInfimum() == sprInf::run) Player::ins()->setSpriteInfimum(sprInf::sit);
			else if (Player::ins()->getSpriteInfimum() == sprInf::sit) Player::ins()->setSpriteInfimum(sprInf::crawl);
			else if (Player::ins()->getSpriteInfimum() == sprInf::crawl)Player::ins()->setSpriteInfimum(sprInf::walk);
			Player::ins()->setSpriteIndex(Player::ins()->getSpriteInfimum());
			popDownWhenEnd = false;
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
			if (myCar->gearState == gearFlag::park) myCar->gearState = gearFlag::reverse;
			else if (myCar->gearState == gearFlag::reverse) myCar->gearState = gearFlag::neutral;
			else if (myCar->gearState == gearFlag::neutral) myCar->gearState = gearFlag::drive;
			else myCar->gearState = gearFlag::park;
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
			myCar->trainMoveCounter = 0;
			myCar->trainSpdVal = 0;

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

			if (myCar->vehType == vehFlag::train)
			{
				dir16 startDir = dir16::dir2;
				int vx = myCar->getGridX();
				int vy = myCar->getGridY();
				int vz = myCar->getGridZ();
				Prop* tgtProp = (Prop*)World::ins()->getTile(vx, vy, vz).PropPtr;
				if (tgtProp != nullptr)
				{
					if (myCar->gearState == gearFlag::drive)//주행기어
					{
						if (myCar->bodyDir == dir16::dir0)//동쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir0), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir2), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir6), 7.0);
						}
						else if (myCar->bodyDir == dir16::dir2)//북쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir2), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir4), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir0), 7.0);
						}
						else if (myCar->bodyDir == dir16::dir4)//서쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir4), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir2), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir6), 7.0);
						}
						else if (myCar->bodyDir == dir16::dir6)//남쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir6), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir4), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir0), 7.0);
						}
					}
					else if (myCar->gearState == gearFlag::reverse)
					{
						if (myCar->bodyDir == dir16::dir0)//동쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir4), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir2), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir6), 7.0);
						}
						else if (myCar->bodyDir == dir16::dir2)//북쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir6), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir4), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir0), 7.0);
						}
						else if (myCar->bodyDir == dir16::dir4)//서쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir0), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir2), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir6), 7.0);
						}
						else if (myCar->bodyDir == dir16::dir6)//남쪽 방향 열차
						{
							if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir2), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir4), 7.0);
							else if (tgtProp->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) myCar->accVec = scalarMultiple(dir16ToVec(dir16::dir0), 7.0);
						}
					}
				}
			}

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
			CORO(closeDoor(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()));
			break;
		}
		default:
			updateLog(L"#FFFFFF알수없는 레터박스 버튼이 눌렸다.");
		}

		if (popDownWhenEnd == true)
		{
			if (y != 0) executePopDown();
		}
	}
	void tileTouch(int touchX, int touchY) //일반 타일 터치
	{
		if (ctrlVeh == nullptr)//차량 조종 중이 아닐 경우
		{
			//화면에 있는 아이템 터치
			if (touchX == Player::ins()->getGridX() && touchY == Player::ins()->getGridY())
			{
				if (World::ins()->getItemPos(touchX, touchY, Player::ins()->getGridZ()) != nullptr)
				{
					prt(L"루팅창 오픈 함수 실행\n");
					new Loot(World::ins()->getItemPos(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ())->getPocket(), nullptr);
					click = false;
				}
				else if (World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).VehiclePtr != nullptr)
				{
					Vehicle* belowVehicle = (Vehicle*)World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).VehiclePtr;
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
								typeHUD = vehFlag::train;
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
					if (World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).PropPtr != nullptr)
					{
						Prop* tgtProp = ((Prop*)(World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).PropPtr));
						int tgtItemCode = tgtProp->leadItem.itemCode;
						if (tgtProp->leadItem.checkFlag(itemFlag::UPSTAIR))
						{
							if (World::ins()->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ() + 1).floor == 0)
							{
								updateLog(L"#FFFFFF이 계단의 위층에 바닥이 존재하지 않는다.");
							}
							else
							{
								updateLog(L"#FFFFFF계단을 올라갔다.");
								(World::ins())->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).EntityPtr = nullptr;
								Player::ins()->setGrid(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ() + 1);
								(World::ins())->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).EntityPtr = Player::ins();
								Player::ins()->updateVision(Player::ins()->getEyeSight());
								Player::ins()->updateMinimap();

								Prop* upProp = ((Prop*)(World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).PropPtr));
								if (upProp == nullptr)
								{
									new Prop(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ(), 299);//하강계단
								}
							}
						}
						else if (tgtProp->leadItem.checkFlag(itemFlag::DOWNSTAIR))
						{
							if (World::ins()->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ() + 1).wall != 0)
							{
								updateLog(L"#FFFFFF내려가는 계단이 벽으로 막혀있다.");
							}
							else
							{
								updateLog(L"#FFFFFF계단을 내려갔다.");
								(World::ins())->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).EntityPtr = nullptr;
								Player::ins()->setGrid(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ() - 1);
								(World::ins())->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).EntityPtr = Player::ins();
								Player::ins()->updateVision(Player::ins()->getEyeSight());
								Player::ins()->updateMinimap();

								Prop* downProp = ((Prop*)(World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).PropPtr));
								if (downProp == nullptr)
								{
									new Prop(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ(), 298);//상승계단
								}
							}
						}
					}
				}
			}
			else if ((std::abs(touchX - Player::ins()->getGridX()) <= 1 && std::abs(touchY - Player::ins()->getGridY()) <= 1) && World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).walkable == false)//1칸 이내
			{
				if (World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).PropPtr != nullptr)
				{
					Prop* tgtProp = ((Prop*)(World::ins()->getTile(touchX, touchY, Player::ins()->getGridZ()).PropPtr));
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

							tgtProp->updateTile();
							Player::ins()->updateVision(Player::ins()->getEyeSight());
							updateNearbyBarAct(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ());
						}
					}
					else if (tgtProp->leadItem.checkFlag(itemFlag::UPSTAIR))
					{
						if (World::ins()->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ() + 1).floor == 0)
						{
							updateLog(L"#FFFFFF이 계단의 위층에 바닥이 존재하지 않는다.");
						}
						else
						{
							updateLog(L"#FFFFFF계단을 올라갔다.");
							Player::ins()->addGridZ(1);
						}
					}
					else if (tgtProp->leadItem.checkFlag(itemFlag::DOWNSTAIR))
					{
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
				else
				{
					Player::ins()->startMove(coord2Dir(touchX - Player::ins()->getGridX(), touchY - Player::ins()->getGridY()));
				}
			}
			else
			{
				Player::ins()->setAStarDst(touchX, touchY);
			}
		}
		else//차량을 조종하는 상태일 경우
		{
			if (touchX == Player::ins()->getGridX() && touchY == Player::ins()->getGridY())
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
			if (World::ins()->getTile(cx + dx, cy + dy, cz).PropPtr != nullptr)
			{
				Prop* instlPtr = (Prop*)World::ins()->getTile(cx + dx, cy + dy, cz).PropPtr;
				if (instlPtr->leadItem.checkFlag(itemFlag::DOOR_OPEN))
				{
					doorNumber++;
					doorList.push_back({ cx + dx, cy + dy });
				}
			}
		}

		auto closeDoorCoord = [=](int inputX, int inputY, int inputZ)
			{
				Prop* tgtProp = (Prop*)World::ins()->getTile(inputX, inputY, inputZ).PropPtr;
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
				tgtProp->updateTile();
				Player::ins()->updateVision(Player::ins()->getEyeSight());
				updateNearbyBarAct(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ());
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


			Prop* tgtProp = (Prop*)World::ins()->getTile(targetX, targetY, cz).PropPtr;
			tgtProp->leadItem.eraseFlag(itemFlag::DOOR_OPEN);
			tgtProp->leadItem.addFlag(itemFlag::DOOR_CLOSE);

			tgtProp->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
			tgtProp->leadItem.addFlag(itemFlag::PROP_BLOCKER);
			tgtProp->leadItem.extraSprIndexSingle--;
			tgtProp->updateTile();
			Player::ins()->updateVision(Player::ins()->getEyeSight());
			updateNearbyBarAct(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ());
		}
	};

	Corouter useSkill(SkillData dat)
	{
		const int SKILL_MAX_RANGE = 30;
		switch (dat.skillCode)
		{
		default:
			prt(L"[Entity:useSkill] 플레이어가 알 수 없는 스킬을 시전하였다.\n", this);
			break;
		case 0:
			break;
		case 1:
			break;
		case 30://화염폭풍
		{
			std::vector<std::array<int, 2>> coordList;
			for (int tgtY = -SKILL_MAX_RANGE; tgtY <= SKILL_MAX_RANGE; tgtY++)
			{
				for (int tgtX = -SKILL_MAX_RANGE; tgtX <= SKILL_MAX_RANGE; tgtX++)
				{
					if (World::ins()->getTile(Player::ins()->getGridX() + tgtX, Player::ins()->getGridY() + tgtY, Player::ins()->getGridZ()).fov == fovFlag::white)
					{
						coordList.push_back({ Player::ins()->getGridX() + tgtX, Player::ins()->getGridY() + tgtY });

					}
				}
			}
			new CoordSelect(CoordSelectFlag::FIRESTORM, L"화염폭풍을 시전할 위치를 입력해주세요.", coordList);
			co_await std::suspend_always();
			std::wstring targetStr = coAnswer;
			int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);
			int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
			targetStr.erase(0, targetStr.find(L",") + 1);
			int targetZ = wtoi(targetStr.c_str());

			Player::ins()->setSkillTarget(targetX, targetY, targetZ);
			addAniUSetPlayer(Player::ins(), aniFlag::fireStorm);
			break;
		}
		}
	}

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
			if (World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).EntityPtr != nullptr)
			{
				Player::ins()->startAtk(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ());
				turnWait(1.0);
				break;
			}
		}
	}

	void openContextMenu(Point2 targetGrid)
	{
		std::vector<act> inputOptions;
		//문닫기 추가
		if (World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).PropPtr != nullptr)
		{
			Prop* instlPtr = (Prop*)World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).PropPtr;
			if (instlPtr->leadItem.checkFlag(itemFlag::DOOR_OPEN))
			{
				inputOptions.push_back(act::closeDoor);
			}
		}

		//열기 추가
		if (World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).VehiclePtr != nullptr)
		{
			Vehicle* vPtr = (Vehicle*)World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).VehiclePtr;

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
		if (World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).VehiclePtr != nullptr)
		{
			inputOptions.push_back(act::pull);
		}

		inputOptions.push_back(act::inspect);

		Point2 windowCoord;
		if (inputType == input::mouse || inputType == input::touch)
		{
			windowCoord = { getMouseXY().x, getMouseXY().y };
		}
		else if (inputType == input::gamepad)
		{
			SDL_Rect dst;
			dst.x = cameraW / 2 + zoomScale * ((16 * targetGrid.x + 8) - cameraX) - ((16 * zoomScale) / 2) + 16 * zoomScale;
			dst.y = cameraH / 2 + zoomScale * ((16 * targetGrid.y + 8) - cameraY) - ((16 * zoomScale) / 2) + 16 * zoomScale;
			windowCoord = { dst.x, dst.y };
		}

		//수영하기 추가
		if (itemDex[World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).floor].checkFlag(itemFlag::WATER_DEEP))
		{
			inputOptions.push_back(act::swim);
		}

		//등반 추가
		if (itemDex[World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).wall].checkFlag(itemFlag::CAN_CLIMB))
		{
			inputOptions.push_back(act::climb);
		}

		//승마 추가
		if (World::ins()->getTile(targetGrid.x, targetGrid.y, Player::ins()->getGridZ()).EntityPtr != nullptr)
		{
			inputOptions.push_back(act::ride);
		}

		new ContextMenu(windowCoord.x, windowCoord.y, targetGrid.x, targetGrid.y, inputOptions);
	}
};