#include <SDL.h>


export module Aim;

import std;
import util;
import GUI;
import errorBox;
import globalVar;
import drawText;
import checkCursor;
import drawSprite;
import textureVar;
import Entity;
import Player;
import Sprite;
import World;
import log;
import ItemData;
import Monster;
import ItemPocket;
import turnWait;

export class Aim : public GUI
{
private:
	inline static Aim* ptr = nullptr;
	SDL_Rect aimBase;
	SDL_Rect aimWindow;
	SDL_Rect aimTitle;

	int tmpPivotX = 0;
	int tmpPivotY = 0;

	int targetX = 0;
	int targetY = 0;
	int targetZ = 0;
	atkType targetAtkType = atkType::bash;//0:관통, 1: 베기, 2: 타격, 3:사격, 4:투척
	int targetPart = 0;

	SDL_Rect targetPartRect;
	SDL_Rect atkBtn, aimBtn, modeBtn;
	SDL_Rect pierceBtn, cutBtn, bashBtn, shotBtn, throwBtn;

	SDL_Rect zombieTorsoBox, zombieHeadBox, zombieLArmBox, zombieRArmBox, zombieLLegBox, zombieRLegBox;
	SDL_Point zombieTorsoPoint, zombieHeadPoint, zombieLArmPoint, zombieRArmPoint, zombieLLegPoint, zombieRLegPoint;

	SDL_Rect humanTorsoBox, humanHeadBox, humanLArmBox, humanRArmBox, humanLLegBox, humanRLegBox;
	SDL_Point humanTorsoPoint, humanHeadPoint, humanLArmPoint, humanRArmPoint, humanLLegPoint, humanRLegPoint;

	SDL_Rect equipSlotBehind, equipSlotFront;

	Entity* victimEntity = nullptr;

	bool existMode = false; //현재 가리키는 아이템에 모드(3점사나 연발)가 존재하는지를 나타내는 변수
	bool inSightRange = false; //현재 이 객체가 플레이어의 시야 내에 있는지 체크
	unsigned __int8 weaponRange = 0;
public:
	Aim(int inputX, int inputY, int inputZ) : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one Aim instance was generated.");
		ptr = this;

		victimEntity = (Entity*)World::ins()->getTile(inputX, inputY, inputZ).EntityPtr;
		cameraFix = false;
		cameraX = victimEntity->getX();
		cameraY = victimEntity->getY();

		targetX = inputX;
		targetY = inputY;
		targetZ = inputZ;

		//메세지 박스 렌더링
		changeXY(cameraW / 2 + 210, cameraH / 2 - 26, true);

		tabType = tabFlag::closeWin;

		//생성될 때의 기본 저격 부위 설정
		if (victimEntity == nullptr) {}
		else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::zombie)
		{
			targetPartRect = zombieTorsoBox;
			targetPart = partType::torso;
		}
		else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::human)
		{
			targetPartRect = humanTorsoBox;
			targetPart = partType::torso;
		}

		//생성될 때 아이템이 있는 쪽을 우선해서 수정
		for (int i = 0; i < Player::ins()->getEquipPtr()->itemInfo.size(); i++)
		{
			if (Player::ins()->getEquipPtr()->itemInfo[i].equipState == equip::right)
			{
				break;
			}

			if (i == Player::ins()->getEquipPtr()->itemInfo.size() - 1)
			{
				Player::ins()->aimWeaponLeft();
			}
		}

		//생성될 때 PCBST 자동 설정(장비 중인 아이템으로부터)

		if (Player::ins()->getAimWeaponIndex() == -1) //맨손일 경우
		{
			targetAtkType = atkType::bash;
		}
		else
		{
			if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].pierce > 0) { targetAtkType = atkType::pierce; }
			else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].cut > 0) { targetAtkType = atkType::cut; }
			else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].bash > 0) { targetAtkType = atkType::bash; }
			else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].gunDmg > 0.0) { targetAtkType = atkType::shot; }
			else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].throwAtk > 0) { targetAtkType = atkType::throwing; }
			else { errorBox("No atkMode unavailiable in Aim.ixx"); }
		}


		deactInput();
		deactDraw();

		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Aim()
	{
		cameraFix = true;
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Aim* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		aimBase = { 0, 0, 260, 416 };
		aimTitle = { 130 - 65, 0, 130, 30 };
		aimWindow = { 0, 36, 260, 380 };

		if (center == false)
		{
			aimBase.x += inputX;
			aimBase.y += inputY;
			aimTitle.x += inputX;
			aimTitle.y += inputY;
			aimWindow.x += inputX;
			aimWindow.y += inputY;
		}
		else
		{
			aimBase.x += inputX - aimBase.w / 2;
			aimBase.y += inputY - aimBase.h / 2;
			aimTitle.x += inputX - aimBase.w / 2;
			aimTitle.y += inputY - aimBase.h / 2;
			aimWindow.x += inputX - aimBase.w / 2;
			aimWindow.y += inputY - aimBase.h / 2;
		}

		tmpPivotX = aimWindow.x + aimWindow.w / 2;
		tmpPivotY = aimWindow.y + aimWindow.h / 2 + 8;

		atkBtn = { aimWindow.x + aimWindow.w / 2 - 96 - 27 * existMode , aimWindow.y + 287, 192,35 };
		aimBtn = { aimWindow.x + aimWindow.w / 2 - 96 - 27 * existMode , aimWindow.y + 287 + 38, 192,35 };

		modeBtn = { aimWindow.x + 202, aimWindow.y + 287, 54, 73 };

		pierceBtn = { aimWindow.x + 4 + 50 * 0,aimWindow.y + 68 + 20,48,36 };
		cutBtn = { aimWindow.x + 4 + 50 * 1,aimWindow.y + 68 + 20,48,36 };
		bashBtn = { aimWindow.x + 4 + 50 * 2,aimWindow.y + 68 + 20,48,36 };
		shotBtn = { aimWindow.x + 4 + 50 * 3,aimWindow.y + 68 + 20,48,36 };
		throwBtn = { aimWindow.x + 4 + 50 * 4,aimWindow.y + 68 + 20,48,36 };

		zombieTorsoBox = { tmpPivotX + 17,tmpPivotY - 56, 64,32 };
		zombieHeadBox = { tmpPivotX - 95,tmpPivotY - 67, 64,32 };
		zombieLArmBox = { tmpPivotX + 35,tmpPivotY - 15, 64,32 };
		zombieRArmBox = { tmpPivotX - 108,tmpPivotY - 13, 64,32 };
		zombieLLegBox = { tmpPivotX + 35, tmpPivotY + 28 , 64,32 };
		zombieRLegBox = { tmpPivotX - 89, tmpPivotY + 29, 64,32 };

		zombieTorsoPoint = { tmpPivotX - 6 , tmpPivotY - 22 };
		zombieHeadPoint = { tmpPivotX - 12 , tmpPivotY - 50 };
		zombieLArmPoint = { tmpPivotX + 18, tmpPivotY - 6 };
		zombieRArmPoint = { tmpPivotX - 29, tmpPivotY - 6 };
		zombieLLegPoint = { tmpPivotX + 13, tmpPivotY + 30 };
		zombieRLegPoint = { tmpPivotX - 9, tmpPivotY + 34 };


		humanTorsoBox = { tmpPivotX + 27,tmpPivotY - 51, 64,32 };
		humanHeadBox = { tmpPivotX - 85,tmpPivotY - 59, 64,32 };
		humanLArmBox = { tmpPivotX + 35,tmpPivotY - 5, 64,32 };
		humanRArmBox = { tmpPivotX - 91,tmpPivotY - 9, 64,32 };
		humanLLegBox = { tmpPivotX + 22, tmpPivotY + 31 , 64,32 };
		humanRLegBox = { tmpPivotX - 85, tmpPivotY + 31, 64,32 };

		humanTorsoPoint = { tmpPivotX + 1, tmpPivotY - 23 };
		humanHeadPoint = { tmpPivotX , tmpPivotY - 50 };
		humanLArmPoint = { tmpPivotX + 21, tmpPivotY - 2 };
		humanRArmPoint = { tmpPivotX - 14, tmpPivotY + 4 };
		humanLLegPoint = { tmpPivotX + 8, tmpPivotY + 40 };
		humanRLegPoint = { tmpPivotX - 5 , tmpPivotY + 40 };

		equipSlotBehind = { aimWindow.x + 12, aimWindow.y + 12, 48, 48 };
		equipSlotFront = { aimWindow.x + 18, aimWindow.y + 18, 48, 48 };
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		const Uint8* state = SDL_GetKeyboardState(NULL);
		Sprite* targetBtnSpr = nullptr;

		//타겟 엔티티 갱신
		victimEntity = (Entity*)World::ins()->getTile(targetX,targetY,targetZ).EntityPtr;
		std::vector<ItemData> playerEquipInfo = Player::ins()->getEquipPtr()->itemInfo;

		{
			//최상단 title 조준공격
			setFontSize(16);
			std::wstring titleStr = sysStr[104];
			aimTitle.w = queryTextWidth(titleStr, false) + 72;
			aimTitle.x = aimWindow.x + aimWindow.w / 2 - aimTitle.w / 2;
			drawStadium(aimTitle.x, aimTitle.y, aimTitle.w, aimTitle.h, { 0,0,0 }, 230, 5);
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			drawTextCenter(titleStr, aimTitle.x + (aimTitle.w / 2), aimTitle.y + 13);
			setZoom(1.5);
			drawSpriteCenter(spr::icon16, 8, aimTitle.x + (aimTitle.w / 2) - queryTextWidth(titleStr, true) / 2 - 20, aimTitle.y + aimTitle.h / 2);
			setZoom(1.0);
		}

		//장착한 아이템 그리기

		drawStadium(aimWindow.x, aimWindow.y, aimWindow.w, aimWindow.h, { 0,0,0 }, 150, 5);

		setZoom(zoomScale);
		drawSpriteCenter
		(
			spr::aimMarker,
			0,
			cameraW / 2,
			cameraH / 2
		);
		setZoom(1.0);


		{
			SDL_Color slotColor = col::black;
			if (checkCursor(&equipSlotFront) || checkCursor(&equipSlotBehind))
			{
				if (click == true) { slotColor = lowCol::deepBlue; }
				else { slotColor = lowCol::blue; }
			}

			{
				//뒤에 나올 아이템 그리기
				int behindSprIndex = 0;
				int targetHand = 0;
				std::wstring behindStr = L"";

				if (Player::ins()->getAimHand() == equip::left)
				{
					targetHand = equip::left;
					behindStr = L"[L] ";
				}
				else
				{
					targetHand = equip::right;
					behindStr = L"[R] ";
				}

				for (int i = 0; i < playerEquipInfo.size(); i++)
				{
					if (playerEquipInfo[i].equipState == targetHand)
					{
						behindStr += playerEquipInfo[i].name;
						behindSprIndex = playerEquipInfo[i].sprIndex;
						break;
					}
					else if (playerEquipInfo[i].equipState == equip::both)
					{
						behindStr = L"[B]";
						behindStr += playerEquipInfo[i].name;
						behindSprIndex = playerEquipInfo[i].sprIndex;
						break;
					}
					if (i == playerEquipInfo.size() - 1) { behindStr += L"맨손"; }
				}
				if (playerEquipInfo.size() == 0) { behindStr += L"맨손"; }
				SDL_Rect inEquipSlotBehind = { aimWindow.x + 12 + 2, aimWindow.y + 12 + 2, 48 - 4, 48 - 4 };
				drawStadium(equipSlotBehind.x, equipSlotBehind.y, equipSlotBehind.w, equipSlotBehind.h, slotColor, 200, 5);
				drawRect(inEquipSlotBehind, lowCol::white);
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, behindSprIndex, equipSlotBehind.x + equipSlotBehind.w / 2, equipSlotBehind.y + equipSlotBehind.h / 2);
				setZoom(1.0);

				setFontSize(12);
				drawText(col2Str(col::black) + behindStr, aimWindow.x + 74 + 1, aimWindow.y + 6);

				drawText(col2Str(col::black) + behindStr, aimWindow.x + 74 - 1, aimWindow.y + 6);
				drawText(col2Str(col::black) + behindStr, aimWindow.x + 74, aimWindow.y + 6 + 1);
				drawText(col2Str(col::black) + behindStr, aimWindow.x + 74, aimWindow.y + 6 - 1);
				drawText(col2Str(col::gray) + behindStr, aimWindow.x + 74, aimWindow.y + 6);
			}


			{
				//앞에 나올 아이템 그리기
				int frontSprIndex = 0;
				int targetHand = 0;
				std::wstring frontStr = L"";
				if (Player::ins()->getAimHand() == equip::left)
				{
					targetHand = equip::left;
					frontStr = L"[L] ";
				}
				else
				{
					targetHand = equip::right;
					frontStr = L"[R] ";
				}

				if (Player::ins()->getAimWeaponIndex() == -1)
				{
					frontStr += L"맨손";
					drawSprite(spr::aimLRChange, 0, aimWindow.x + 3, aimWindow.y + 4);
				}
				else
				{
					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].equipState == equip::both) { frontStr = L"[B] "; }

					frontStr += playerEquipInfo[Player::ins()->getAimWeaponIndex()].name;

					if (((ItemPocket*)playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketPtr)->itemInfo.size() > 0)
					{
						//리볼버
						if (((ItemPocket*)playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketPtr)->itemInfo[0].checkFlag(itemFlag::AMMO))
						{
							int bulletNumber = 0;
							ItemPocket* gunInside = (ItemPocket*)playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketPtr;
							for (int i = 0; i < gunInside->itemInfo.size(); i++)
							{
								bulletNumber += gunInside->itemInfo[i].number;
							}

							frontStr += L" (";
							frontStr += std::to_wstring(bulletNumber);
							frontStr += L"/";
							frontStr += std::to_wstring(playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketMaxNumber);
							frontStr += L")";
						}
						else if (((ItemPocket*)playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketPtr)->itemInfo[0].checkFlag(itemFlag::MAGAZINE))//탄창
						{
							int bulletNumber = 0;
							ItemPocket* magazineInside = (ItemPocket*)(((ItemPocket*)playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketPtr)->itemInfo[0].pocketPtr);
							for (int i = 0; i < magazineInside->itemInfo.size(); i++)
							{
								bulletNumber += magazineInside->itemInfo[i].number;
							}
							frontStr += L" (";
							frontStr += std::to_wstring(bulletNumber);
							frontStr += L"/";
							frontStr += std::to_wstring(((ItemPocket*)playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketPtr)->itemInfo[0].pocketMaxNumber);
							frontStr += L")";
						}
					}
					else
					{
						//리볼버 종류 또는 탄창
						if (itemDex[playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
						{
							frontStr += L" (";
							frontStr += L"0";
							frontStr += L"/";
							frontStr += std::to_wstring(playerEquipInfo[Player::ins()->getAimWeaponIndex()].pocketMaxNumber);
							frontStr += L")";
						}
					}

					frontSprIndex = playerEquipInfo[Player::ins()->getAimWeaponIndex()].sprIndex;

					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].equipState == targetHand) { drawSprite(spr::aimLRChange, 0, aimWindow.x + 3, aimWindow.y + 4); }
				}

				SDL_Rect inEquipSlotFront = { aimWindow.x + 18 + 2, aimWindow.y + 18 + 2, 48 - 4, 48 - 4 };
				drawStadium(equipSlotFront.x, equipSlotFront.y, equipSlotFront.w, equipSlotFront.h, slotColor, 200, 5);
				drawRect(inEquipSlotFront, col::white, 170);
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, frontSprIndex, equipSlotFront.x + equipSlotFront.w / 2, equipSlotFront.y + equipSlotFront.h / 2);
				setZoom(1.0);

				setFontSize(12);

				drawText(col2Str(col::black) + frontStr, aimWindow.x + 74 + 4 + 1, aimWindow.y + 6 + 4);
				drawText(col2Str(col::black) + frontStr, aimWindow.x + 74 + 4 - 1, aimWindow.y + 6 + 4);
				drawText(col2Str(col::black) + frontStr, aimWindow.x + 74 + 4, aimWindow.y + 6 + 4 + 1);
				drawText(col2Str(col::black) + frontStr, aimWindow.x + 74 + 4, aimWindow.y + 6 + 4 - 1);
				drawText(col2Str(col::white) + frontStr, aimWindow.x + 74 + 4, aimWindow.y + 6 + 4);
			}
		}

		//drawStadium(modeBtn.x,modeBtn.y,modeBtn.w,modeBtn.h,color::black,200,5);

		//setFontSize(10);
		//drawTextCenter(col2Str(color::white) + L"2-handed", modeBtn.x + modeBtn.w/2, modeBtn.y + modeBtn.h/2 - 1);

		setFontSize(10);

		int minDmg, maxDmg;
		float baseAcc;
		float atkSpd;
		int playerStr = Player::ins()->entityInfo.statStr;
		int playerDex = Player::ins()->entityInfo.statDex;

		if (Player::ins()->getAimWeaponIndex() != -1)
		{
			ItemData tmpAimWeapon = Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()];
			switch (targetAtkType)
			{
				case atkType::pierce://관통
					maxDmg = calcMelee::maxDmg(tmpAimWeapon.pierce, playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					minDmg = calcMelee::minDmg(tmpAimWeapon.pierce, tmpAimWeapon.meleeBalance, playerDex, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					atkSpd = calcMelee::atkSpd(tmpAimWeapon.meleeAtkSpd, playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					weaponRange = tmpAimWeapon.meleeRange;
					baseAcc = tmpAimWeapon.meleeAtkAcc;
					break;
				case atkType::cut://절단
					maxDmg = calcMelee::maxDmg(tmpAimWeapon.cut, playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					minDmg = calcMelee::minDmg(tmpAimWeapon.cut, tmpAimWeapon.meleeBalance, playerDex, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					atkSpd = calcMelee::atkSpd(tmpAimWeapon.meleeAtkSpd, playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					weaponRange = tmpAimWeapon.meleeRange;
					baseAcc = tmpAimWeapon.meleeAtkAcc;
					break;
				case atkType::bash://타격
					maxDmg = calcMelee::maxDmg(tmpAimWeapon.bash, playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					minDmg = calcMelee::minDmg(tmpAimWeapon.bash, tmpAimWeapon.meleeBalance, playerDex, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					atkSpd = calcMelee::atkSpd(tmpAimWeapon.meleeAtkSpd, playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::piercingWeapon));
					weaponRange = tmpAimWeapon.meleeRange;
					baseAcc = tmpAimWeapon.meleeAtkAcc;
					break;
				case atkType::shot://사격
					if (getBulletNumber(tmpAimWeapon) > 0)
					{
						int bulletAtk = myMax(getTopBulletData(tmpAimWeapon).bulletPierce, getTopBulletData(tmpAimWeapon).bulletCut, getTopBulletData(tmpAimWeapon).bulletBash);
						maxDmg = calcShot::maxDmg(bulletAtk, tmpAimWeapon.gunDmg);
						minDmg = calcShot::minDmg(bulletAtk, tmpAimWeapon.gunBalance);
						atkSpd = calcShot::atkSpd(tmpAimWeapon.gunShotSpd);
						weaponRange = getTopBulletData(tmpAimWeapon).bulletRange + tmpAimWeapon.gunRange;
						baseAcc = tmpAimWeapon.gunAccInit;
					}
					else
					{
						minDmg = 0;
						maxDmg = 0;
						atkSpd = 0;
						weaponRange = 0;
						baseAcc = 0;
					}
					break;
				case atkType::throwing://투척
					maxDmg = calcThrow::maxDmg(tmpAimWeapon.throwAtk, playerStr, Player::ins()->getTalentLevel(talentFlag::throwing));
					minDmg = calcThrow::minDmg(tmpAimWeapon.throwAtk, tmpAimWeapon.throwBalance, playerDex, Player::ins()->getTalentLevel(talentFlag::throwing));
					atkSpd = 1.0;
					weaponRange = tmpAimWeapon.throwRange;
					baseAcc = tmpAimWeapon.throwAtkAcc;
					break;
			}
		}
		else
		{
			minDmg = calcUnarmed::minDmg(playerDex, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::unarmedCombat));
			maxDmg = calcUnarmed::maxDmg(playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::unarmedCombat));
			atkSpd = calcUnarmed::atkSpd(playerStr, Player::ins()->getTalentLevel(talentFlag::fighting), Player::ins()->getTalentLevel(talentFlag::unarmedCombat));
			weaponRange = 1;
			baseAcc = 0.9;
		}


		drawText(col2Str(col::white) + L"데미지 : " + std::to_wstring(minDmg) + L"~" + std::to_wstring(maxDmg) + L"(P)", aimWindow.x + 74 + 4, aimWindow.y + 32);
		drawText(col2Str(col::white) + L"공격속도 : " + decimalCutter(atkSpd, 2), aimWindow.x + 74 + 4, aimWindow.y + 32 + 14);


		drawText(col2Str(col::white) + L"명중률 : " + decimalCutter(baseAcc, 2), aimWindow.x + 74 + 4 + 90, aimWindow.y + 32);
		drawText(col2Str(col::white) + L"사거리 : " + std::to_wstring(weaponRange) + L"칸", aimWindow.x + 74 + 4 + 90, aimWindow.y + 32 + 14);

		drawText(col2Str(col::white) + L"탄종 : 소총 철갑탄", aimWindow.x + 74 + 4, aimWindow.y + 32 + 28);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			{//관통 버튼
				SDL_Color btnColor = col::black;
				if (targetAtkType == atkType::pierce) { btnColor = lowCol::blue; }
				if (checkCursor(&pierceBtn))
				{
					if (click == true) { btnColor = lowCol::deepBlue; }
					else { btnColor = lowCol::blue; }
				}
				drawStadium(pierceBtn.x, pierceBtn.y, pierceBtn.w, pierceBtn.h, btnColor, 200, 5);
				drawTextCenter(col2Str(col::white) + L"PIERCE", aimWindow.x + 28, pierceBtn.y + 26);
				drawSpriteCenter(spr::icon16, 18, pierceBtn.x + pierceBtn.w / 2, pierceBtn.y + 10);
				if (targetAtkType == atkType::pierce) { drawSpriteCenter(spr::aimAtkTypeMarker, 0, pierceBtn.x + pierceBtn.w / 2, pierceBtn.y + pierceBtn.h / 2); }

				if (Player::ins()->getAimWeaponIndex() != -1)
				{
					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].pierce == 0)
					{
						drawStadium(pierceBtn.x, pierceBtn.y, pierceBtn.w, pierceBtn.h, col::black, 150, 5);
					}
				}
				else
				{
					drawStadium(pierceBtn.x, pierceBtn.y, pierceBtn.w, pierceBtn.h, col::black, 150, 5);
				}
			}

			{//베기 버튼
				SDL_Color btnColor = col::black;
				if (targetAtkType == atkType::cut) { btnColor = lowCol::blue; }
				if (checkCursor(&cutBtn))
				{
					if (click == true) { btnColor = lowCol::deepBlue; }
					else { btnColor = lowCol::blue; }
				}
				drawStadium(cutBtn.x, cutBtn.y, cutBtn.w, cutBtn.h, btnColor, 200, 5);
				drawTextCenter(col2Str(col::white) + L"CUT", aimWindow.x + 28 + 50 * 1, cutBtn.y + 26);
				drawSpriteCenter(spr::icon16, 19, cutBtn.x + cutBtn.w / 2, cutBtn.y + 10);
				if (targetAtkType == atkType::cut) { drawSpriteCenter(spr::aimAtkTypeMarker, 0, cutBtn.x + cutBtn.w / 2, cutBtn.y + cutBtn.h / 2); }

				if (Player::ins()->getAimWeaponIndex() != -1)
				{
					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].cut == 0)
					{
						drawStadium(cutBtn.x, cutBtn.y, cutBtn.w, cutBtn.h, col::black, 150, 5);
					}
				}
				else
				{
					drawStadium(cutBtn.x, cutBtn.y, cutBtn.w, cutBtn.h, col::black, 150, 5);
				}
			}

			{//타격 버튼
				SDL_Color btnColor = col::black;
				if (targetAtkType == atkType::bash) { btnColor = lowCol::blue; }
				if (checkCursor(&bashBtn))
				{
					if (click == true) { btnColor = lowCol::deepBlue; }
					else { btnColor = lowCol::blue; }
				}
				drawStadium(bashBtn.x, bashBtn.y, bashBtn.w, bashBtn.h, btnColor, 200, 5);
				drawTextCenter(col2Str(col::white) + L"BASH", aimWindow.x + 28 + 50 * 2, bashBtn.y + 26);
				drawSpriteCenter(spr::icon16, 20, bashBtn.x + bashBtn.w / 2, bashBtn.y + 10);
				if (targetAtkType == atkType::bash) { drawSpriteCenter(spr::aimAtkTypeMarker, 0, bashBtn.x + bashBtn.w / 2, bashBtn.y + bashBtn.h / 2); }

				if (Player::ins()->getAimWeaponIndex() != -1)
				{
					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].bash == 0)
					{
						drawStadium(bashBtn.x, bashBtn.y, bashBtn.w, bashBtn.h, col::black, 150, 5);
					}
				}
			}

			{//사격 버튼
				SDL_Color btnColor = col::black;
				if (targetAtkType == atkType::shot) { btnColor = lowCol::blue; }
				if (checkCursor(&shotBtn))
				{
					if (click == true) { btnColor = lowCol::deepBlue; }
					else { btnColor = lowCol::blue; }
				}
				drawStadium(shotBtn.x, shotBtn.y, shotBtn.w, shotBtn.h, btnColor, 200, 5);
				drawTextCenter(col2Str(col::white) + L"SHOT", aimWindow.x + 28 + 50 * 3, shotBtn.y + 26);
				drawSpriteCenter(spr::icon16, 21, shotBtn.x + shotBtn.w / 2, shotBtn.y + 10);
				if (targetAtkType == atkType::shot) { drawSpriteCenter(spr::aimAtkTypeMarker, 0, shotBtn.x + shotBtn.w / 2, shotBtn.y + shotBtn.h / 2); }

				if (Player::ins()->getAimWeaponIndex() != -1)
				{
					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].gunDmg == 0.0)
					{
						drawStadium(shotBtn.x, shotBtn.y, shotBtn.w, shotBtn.h, col::black, 150, 5);
					}
				}
				else
				{
					drawStadium(shotBtn.x, shotBtn.y, shotBtn.w, shotBtn.h, col::black, 150, 5);
				}
			}

			{//투척 버튼
				SDL_Color btnColor = col::black;
				if (targetAtkType == atkType::throwing) { btnColor = lowCol::blue; }
				if (checkCursor(&throwBtn))
				{
					if (click == true) { btnColor = lowCol::deepBlue; }
					else { btnColor = lowCol::blue; }
				}
				drawStadium(throwBtn.x, throwBtn.y, throwBtn.w, throwBtn.h, btnColor, 200, 5);
				drawTextCenter(col2Str(col::white) + L"THROW", aimWindow.x + 28 + 50 * 4, throwBtn.y + 26);
				drawSpriteCenter(spr::icon16, 22, throwBtn.x + throwBtn.w / 2, throwBtn.y + 10);
				if (targetAtkType == atkType::throwing) { drawSpriteCenter(spr::aimAtkTypeMarker, 0, throwBtn.x + throwBtn.w / 2, throwBtn.y + throwBtn.h / 2); }

				if (Player::ins()->getAimWeaponIndex() != -1)
				{
					if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].throwAtk == 0)//단분자 칼날같이 분리가 불가능한 장비
					{
						drawStadium(throwBtn.x, throwBtn.y, throwBtn.w, throwBtn.h, col::black, 150, 5);
					}
				}
				else
				{
					drawStadium(throwBtn.x, throwBtn.y, throwBtn.w, throwBtn.h, col::black, 150, 5);
				}
			}
		}

		//바디템플릿 그리기

		SDL_Rect partRect;
		SDL_Point partPoint;
		std::wstring partName;
		int sprIndex;
		int targetPartType = partType::body;
		float aimAcc = 0;
		float mainAimAcc = 0;
		if (victimEntity == nullptr)//해당 위치에 엔티티가 존재하지 않을 경우
		{
			drawTextCenter(col2Str(col::white) + sysStr[112], tmpPivotX, tmpPivotY);
		}
		else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::zombie)//해당 엔티티가 좀비 템플릿일 경우
		{
			//#96fd46
			for (int i = 0; i < 6; i++)
			{
				switch (i)
				{
					case 0:
						targetPartType = partType::torso;
						partRect = zombieTorsoBox;
						partName = sysStr[106];
						partPoint = zombieTorsoPoint;
						sprIndex = 1;
						break;
					case 1:
						targetPartType = partType::head;
						partRect = zombieHeadBox;
						partName = sysStr[107];
						partPoint = zombieHeadPoint;
						sprIndex = 2;
						break;
					case 2:
						targetPartType = partType::larm;
						partRect = zombieLArmBox;
						partName = sysStr[108];
						partPoint = zombieLArmPoint;
						sprIndex = 3;
						break;
					case 3:
						targetPartType = partType::rarm;
						partRect = zombieRArmBox;
						partName = sysStr[109];
						partPoint = zombieRArmPoint;
						sprIndex = 4;
						break;
					case 4:
						targetPartType = partType::lleg;
						partRect = zombieLLegBox;
						partName = sysStr[110];
						partPoint = zombieLLegPoint;
						sprIndex = 5;
						break;
					case 5:
						targetPartType = partType::rleg;
						partRect = zombieRLegBox;
						partName = sysStr[111];
						partPoint = zombieRLegPoint;
						sprIndex = 6;
						break;
				}

				////////////////해당 부위의 명중률 계산////////////////////
				if (targetPartRect.x == partRect.x && targetPartRect.y == partRect.y)
				{
					aimAcc = Player::ins()->getAimAcc(victimEntity, targetPartType, true);
					mainAimAcc = aimAcc;
				}
				else
				{
					aimAcc = Player::ins()->getAimAcc(victimEntity, targetPartType, false);
				}
				/////////////////////////////////////////////////////////
				drawSinglePart(partRect, sprIndex, partPoint, partName, targetPartType, spr::bodyTmpZombie, aimAcc);
			}
		}
		else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::human)//해당 엔티티가 휴먼 템플릿일 경우
		{

			for (int i = 0; i < 6; i++)
			{
				switch (i)
				{
					case 0:
						targetPartType = partType::torso;
						partRect = humanTorsoBox;
						partName = sysStr[106];
						partPoint = humanTorsoPoint;
						sprIndex = 1;
						break;
					case 1:
						targetPartType = partType::head;
						partRect = humanHeadBox;
						partName = sysStr[107];
						partPoint = humanHeadPoint;
						sprIndex = 2;
						break;
					case 2:
						targetPartType = partType::larm;
						partRect = humanLArmBox;
						partName = sysStr[108];
						partPoint = humanLArmPoint;
						sprIndex = 3;
						break;
					case 3:
						targetPartType = partType::rarm;
						partRect = humanRArmBox;
						partName = sysStr[109];
						partPoint = humanRArmPoint;
						sprIndex = 4;
						break;
					case 4:
						targetPartType = partType::lleg;
						partRect = humanLLegBox;
						partName = sysStr[110];
						partPoint = humanLLegPoint;
						sprIndex = 5;
						break;
					case 5:
						targetPartType = partType::rleg;
						partRect = humanRLegBox;
						partName = sysStr[111];
						partPoint = humanRLegPoint;
						sprIndex = 6;
						break;
				}
				////////////////해당 부위의 명중률 계산////////////////////
				if (targetPartRect.x == partRect.x && targetPartRect.y == partRect.y)
				{
					aimAcc = Player::ins()->getAimAcc(victimEntity, targetPartType, true);
					mainAimAcc = aimAcc;
				}
				else
				{
					aimAcc = Player::ins()->getAimAcc(victimEntity, targetPartType, false);
				}
				/////////////////////////////////////////////////////////
				drawSinglePart(partRect, sprIndex, partPoint, partName, targetPartType, spr::bodyTmpHuman, aimAcc);
			}
		}


		//템플릿 피벗
		drawPoint(tmpPivotX, tmpPivotY, lowCol::red);

		if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
		{
			if (victimEntity != nullptr)
			{
				drawStadium(tmpPivotX - aimWindow.w / 2, tmpPivotY - 66, aimWindow.w, 132, col::black, 150, 5);
				drawSpriteCenter(spr::floatWarning, 0, tmpPivotX, tmpPivotY);
				setFontSize(12);
				drawTextCenter(col2Str(lowCol::white) + L"OUT OF RANGE", tmpPivotX, tmpPivotY);
			}
		}

		if (victimEntity == nullptr) {}
		else//엔티티 HP바 그리기
		{

			setFontSize(10);
			drawTextCenter(col2Str(lowCol::black) + L"Lord Ereshkigal", tmpPivotX + 1, tmpPivotY + 69);
			drawTextCenter(col2Str(lowCol::black) + L"Lord Ereshkigal", tmpPivotX - 1, tmpPivotY + 69);
			drawTextCenter(col2Str(lowCol::black) + L"Lord Ereshkigal", tmpPivotX, tmpPivotY + 69 + 1);
			drawTextCenter(col2Str(lowCol::black) + L"Lord Ereshkigal", tmpPivotX, tmpPivotY + 69 - 1);
			drawTextCenter(col2Str(lowCol::white) + L"Lord Ereshkigal", tmpPivotX, tmpPivotY + 69);

			SDL_Rect hpRect = { tmpPivotX - 108, tmpPivotY + 80, 216,4 };
			__int16 currentHP = victimEntity->entityInfo.HP;
			__int16	fakeHP = victimEntity->entityInfo.fakeHP;
			__int16 maxHP = victimEntity->entityInfo.maxHP;

			drawStadium(hpRect.x, hpRect.y, hpRect.w, hpRect.h, col::black, 255, 1);

			float ratioFakeHP = myMax((float)0.0, (float)(fakeHP) / (float)(maxHP));
			drawStadium(hpRect.x, hpRect.y, hpRect.w * ratioFakeHP, hpRect.h, col::white, Player::ins()->entityInfo.fakeHPAlpha, 1);

			float ratioHP = myMax((float)0.0, (float)(currentHP) / (float)(maxHP));
			drawStadium(hpRect.x, hpRect.y, hpRect.w * ratioHP, hpRect.h, lowCol::red, 255, 1);

			std::wstring strHP = L"HP ";
			std::wstring slash = L"/";
			strHP += std::to_wstring(currentHP) + slash + std::to_wstring(maxHP);

			setFontSize(8);
			drawTextCenter(col2Str(lowCol::black) + strHP, hpRect.x + (hpRect.w / 2) + 1, hpRect.y + (hpRect.h / 2));
			drawTextCenter(col2Str(lowCol::black) + strHP, hpRect.x + (hpRect.w / 2), hpRect.y + (hpRect.h / 2) - 1);
			drawTextCenter(col2Str(lowCol::black) + strHP, hpRect.x + (hpRect.w / 2) - 1, hpRect.y + (hpRect.h / 2));
			drawTextCenter(col2Str(lowCol::black) + strHP, hpRect.x + (hpRect.w / 2), hpRect.y + (hpRect.h / 2) + 1);
			drawTextCenter(col2Str(lowCol::white) + strHP, hpRect.x + (hpRect.w / 2), hpRect.y + (hpRect.h / 2));
		}


		//1. 정밀 공격 버튼
		{
			SDL_Color btnColor = col::black;
			if (checkCursor(&atkBtn))
			{
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
			}

			SDL_Rect inAtkBtn = { atkBtn.x + 3, atkBtn.y + 3, atkBtn.w - 6, atkBtn.h - 6 };
			drawStadium(atkBtn.x, atkBtn.y, atkBtn.w, atkBtn.h, btnColor, 200, 5);
			drawRect(inAtkBtn, col::white, 130);

			setFontSize(10);
			drawText(col2Str(col::white) + L"몸통 사격 (" + std::to_wstring((int)(mainAimAcc * 100)) + L"%," + std::to_wstring(Player::ins()->getAimStack()) + L"차지)", atkBtn.x + 31, atkBtn.y + 11);
			setFontSize(7);
			drawText(col2Str(col::white) + L"1.1 turn", atkBtn.x + atkBtn.w - 6 - queryTextWidth(L"1.1 turn", true), atkBtn.y + 4);

			setZoom(2.0);
			drawSpriteCenter(spr::icon16, 23, atkBtn.x + 14, atkBtn.y + atkBtn.h / 2);
			setZoom(1.0);

			//타겟좌표에 엔티티가 없으면
			if (victimEntity == nullptr)
			{
				drawStadium(atkBtn.x, atkBtn.y, atkBtn.w, atkBtn.h, btnColor, 100, 5);
			}
			//맨손 사거리 이탈
			else if (Player::ins()->getAimWeaponIndex() == -1)
			{
				if (1 < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
				{
					drawStadium(atkBtn.x, atkBtn.y, atkBtn.w, atkBtn.h, btnColor, 100, 5);
				}
			}
			//무기 사거리 이탈
			else
			{
				if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
				{
					drawStadium(atkBtn.x, atkBtn.y, atkBtn.w, atkBtn.h, btnColor, 100, 5);
				}
			}
		}

		//2. 조준 버튼
		{
			SDL_Color btnColor = col::black;
			if (checkCursor(&aimBtn))
			{
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
			}

			SDL_Rect inAimBtn = { aimBtn.x + 3, aimBtn.y + 3, aimBtn.w - 6, aimBtn.h - 6 };
			drawStadium(aimBtn.x, aimBtn.y, aimBtn.w, aimBtn.h, btnColor, 200, 5);
			drawRect(inAimBtn, col::white, 130);
			setFontSize(10);
			drawText(col2Str(col::white) + L"해당 부위를 정조준", aimBtn.x + 31, aimBtn.y + 11);
			setFontSize(7);
			drawText(col2Str(col::white) + L"0.5 turn", aimBtn.x + aimBtn.w - 6 - queryTextWidth(L"0.5 turn", true), aimBtn.y + 4);

			setZoom(2.0);
			drawSpriteCenter(spr::icon16, 27, aimBtn.x + 14, aimBtn.y + aimBtn.h / 2);
			setZoom(1.0);

			if (victimEntity == nullptr)
			{
				drawStadium(aimBtn.x, aimBtn.y, aimBtn.w, aimBtn.h, btnColor, 100, 5);
			}
			//맨손 사거리 이탈
			else if (Player::ins()->getAimWeaponIndex() == -1)
			{
				if (1 < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
				{
					drawStadium(aimBtn.x, aimBtn.y, aimBtn.w, aimBtn.h, btnColor, 100, 5);
				}
			}
			//무기 사거리 이탈
			else
			{
				if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].meleeRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
				{
					drawStadium(aimBtn.x, aimBtn.y, aimBtn.w, aimBtn.h, btnColor, 100, 5);
				}
			}
		}

		//3. 모드체인지 버튼
		if (existMode == true && Player::ins()->getAimWeaponIndex() >= 0)
		{
			SDL_Color btnColor = col::black;
			if (checkCursor(&modeBtn))
			{
				if (click == true) { btnColor = lowCol::deepBlue; }
				else { btnColor = lowCol::blue; }
			}

			SDL_Rect inModeBtn = { modeBtn.x + 3, modeBtn.y + 3, modeBtn.w - 6, modeBtn.h - 6 };
			drawStadium(modeBtn.x, modeBtn.y, modeBtn.w, modeBtn.h, btnColor, 200, 5);
			drawRect(inModeBtn, col::white, 130);

			if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeTemplate == 1)
			{
				drawLine(modeBtn.x + modeBtn.w / 2 - 16, modeBtn.y + 3, modeBtn.x + modeBtn.w / 2 + 16, modeBtn.y + 3, btnColor);
				setFontSize(8);
				drawTextCenter(col2Str(col::white) + L"MODE", modeBtn.x + modeBtn.w / 2, modeBtn.y + 6);

				setFontSize(10);
				SDL_Color modeCol;
				if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeCurrent == weaponMode::safeMode) { modeCol = col::white; }
				else { modeCol = col::gray; }
				drawTextCenter(col2Str(modeCol) + L"안전", modeBtn.x + modeBtn.w / 2, modeBtn.y + 21);
				if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeCurrent == weaponMode::semiAutoMode) { modeCol = col::white; }
				else { modeCol = col::gray; }
				drawTextCenter(col2Str(modeCol) + L"단발", modeBtn.x + modeBtn.w / 2, modeBtn.y + 21 + 12 * 1);
				if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeCurrent == weaponMode::burstMode) { modeCol = col::white; }
				else { modeCol = col::gray; }
				drawTextCenter(col2Str(modeCol) + L"점사", modeBtn.x + modeBtn.w / 2, modeBtn.y + 21 + 12 * 2);
				if (playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeCurrent == weaponMode::autoMode) { modeCol = col::white; }
				else { modeCol = col::gray; }
				drawTextCenter(col2Str(modeCol) + L"연발", modeBtn.x + modeBtn.w / 2, modeBtn.y + 21 + 12 * 3);
			}
		}

		setFontSize(10);
		drawText(col2Str(col::white) + L"ㅁ공격타입 변경  ㅁ좌우무기 변경 ㅁ타격부위 변경", aimWindow.x + 10, aimWindow.y + aimWindow.h - 15);
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
	}
	void clickMotionGUI(int dx, int dy)
	{
	}
	void clickDownGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
		}
		else if (checkCursor(&aimWindow))
		{
			if (checkCursor(&equipSlotFront) || checkCursor(&equipSlotBehind))
			{
				if (Player::ins()->getAimHand() == equip::right)
				{
					Player::ins()->aimWeaponLeft();
				}
				else
				{
					Player::ins()->aimWeaponRight();
				}

				if (Player::ins()->getAimWeaponIndex() == -1)
				{
					targetAtkType = atkType::bash;
				}
				else
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].pierce > 0) { targetAtkType = atkType::pierce; }
					else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].cut > 0) { targetAtkType = atkType::cut; }
					else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].bash > 0) { targetAtkType = atkType::bash; }
					else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].gunDmg > 0.0) { targetAtkType = atkType::shot; }
					else if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].throwAtk > 0) { targetAtkType = atkType::throwing; }
					else { errorBox("No atkMode unavailiable in Aim.ixx"); }
				}
			}

			if (Player::ins()->getAimWeaponIndex() != -1)//무기를 들고 있을 때만 공격의 타입을 바꿀 수 있음
			{
				if (checkCursor(&pierceBtn))
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].pierce > 0)
					{
						targetAtkType = atkType::pierce;
					}
				}
				else if (checkCursor(&cutBtn))
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].cut > 0)
					{
						targetAtkType = atkType::cut;
					}
				}
				else if (checkCursor(&bashBtn))
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].bash > 0)
					{
						targetAtkType = atkType::bash;
					}
				}
				else if (checkCursor(&shotBtn))
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].gunDmg > 0.0)
					{
						targetAtkType = atkType::shot;
					}
				}
				else if (checkCursor(&throwBtn))
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].throwAtk > 0)
					{
						targetAtkType = atkType::throwing;
					}
				}
			}

			if (checkCursor(&atkBtn))
			{
				if (victimEntity == nullptr)
				{
					return;
				}
				//맨손 사거리 이탈
				else if (Player::ins()->getAimWeaponIndex() == -1)
				{
					if (1 < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
					{
						return;
					}

					Player::ins()->startAtk(targetX, targetY, targetZ, targetPart);
					turnWait(1.0);
					Player::ins()->initAimStack();

				}
				//무기 사거리 이탈
				else
				{
					if (targetAtkType == atkType::throwing) //던지기
					{
						if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY))) { return; }

						/*if (targetX != Player::ins()->getGridX() || targetY != Player::ins()->getGridY())
						{*/

						//자기 자신에게 던지는 경우도 고려해야 되나?
						ItemPocket* drop = new ItemPocket(storageType::null);
						Player::ins()->getEquipPtr()->transferItem(drop, Player::ins()->getAimWeaponIndex(), 1);
						Player::ins()->throwing(drop, targetX, targetY);
						Player::ins()->updateStatus();
						updateLog(L"#FFFFFF아이템을 던졌다.");
						//}
						//else
						//{
						//	ItemPocket* drop = new ItemPocket(storageType::null);
						//	Player::ins()->getEquipPtr()->transferItem(drop, Player::ins()->getAimWeaponIndex(), 1);
						//	Player::ins()->drop(drop);
						//	Player::ins()->updateStatus();
						//	updateLog(L"#FFFFFF아이템을 버렸다.");
						//}

						Player::ins()->startAtk(targetX, targetY, targetZ, targetPart, aniFlag::throwing);
						turnWait(1.0);
						Player::ins()->initAimStack();
						Player::ins()->setNextAtkType(targetAtkType);
					}
					else if (targetAtkType == atkType::shot) //사격
					{
						ItemData tmpAimWeapon = Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()];
						if (getBulletNumber(tmpAimWeapon) > 0)
						{
							if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY))) { return; }
						}
						else//사격인데 총알이 없을 경우
						{
							return;
						}

						ItemPocket* drop = new ItemPocket(storageType::null);
						drop->addItemFromDex(25, 1);
						Player::ins()->throwing(drop, targetX, targetY);
						Player::ins()->updateStatus();

						//직탄식 총
						if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
						{
							popTopBullet(((ItemPocket*)tmpAimWeapon.pocketPtr));
						}
						//탄창식 총
						else if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
						{
							popTopBullet((ItemPocket*)((ItemPocket*)tmpAimWeapon.pocketPtr)->itemInfo[0].pocketPtr);
						}

						Player::ins()->startAtk(targetX, targetY, targetZ, targetPart, aniFlag::shotSingle);
						turnWait(1.0);
						Player::ins()->initAimStack();
						Player::ins()->setNextAtkType(targetAtkType);
					}
					else//근접공격
					{
						if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY))) { return; }

						Player::ins()->startAtk(targetX, targetY, targetZ, targetPart);
						turnWait(1.0);
						Player::ins()->initAimStack();
						Player::ins()->setNextAtkType(targetAtkType);
					}
				}
			}
			else if (checkCursor(&aimBtn))
			{
				if (victimEntity == nullptr)
				{
					return;
				}
				//맨손 사거리 이탈
				else if (Player::ins()->getAimWeaponIndex() == -1)
				{
					if (1 < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
					{
						return;
					}
				}
				//무기 사거리 이탈
				else
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].meleeRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
					{
						return;
					}
				}

				Player::ins()->addAimStack();
				turnWait(1.0);
			}
			else if (checkCursor(&modeBtn))
			{
				if (Player::ins()->getAimWeaponIndex() >= 0)
				{
					if (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].modeTemplate == 1)
					{
						switch (Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].modeCurrent)
						{
							case weaponMode::safeMode:
								Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].modeCurrent = weaponMode::semiAutoMode;
								break;
							case weaponMode::semiAutoMode:
								Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].modeCurrent = weaponMode::burstMode;
								break;
							case weaponMode::burstMode:
								Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].modeCurrent = weaponMode::autoMode;
								break;
							case weaponMode::autoMode:
								Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()].modeCurrent = weaponMode::safeMode;
								break;
						}
					}
				}
			}

			if (victimEntity == nullptr)
			{
			}
			else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::zombie)//좀비 템플릿
			{
				if (checkCursor(&zombieTorsoBox))
				{
					targetPartRect = zombieTorsoBox;
					targetPart = partType::torso;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&zombieHeadBox))
				{
					targetPartRect = zombieHeadBox;
					targetPart = partType::head;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&zombieLArmBox))
				{
					targetPartRect = zombieLArmBox;
					targetPart = partType::larm;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&zombieRArmBox))
				{
					targetPartRect = zombieRArmBox;
					targetPart = partType::rarm;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&zombieLLegBox))
				{
					targetPartRect = zombieLLegBox;
					targetPart = partType::lleg;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&zombieRLegBox))
				{
					targetPartRect = zombieRLegBox;
					targetPart = partType::rleg;
					Player::ins()->initAimStack();
				}
			}
			else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::human)//휴먼 템플릿
			{
				if (checkCursor(&humanTorsoBox))
				{
					targetPartRect = humanTorsoBox;
					targetPart = partType::torso;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&humanHeadBox))
				{
					targetPartRect = humanHeadBox;
					targetPart = partType::head;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&humanLArmBox))
				{
					targetPartRect = humanLArmBox;
					targetPart = partType::larm;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&humanRArmBox))
				{
					targetPartRect = humanRArmBox;
					targetPart = partType::rarm;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&humanLLegBox))
				{
					targetPartRect = humanLLegBox;
					targetPart = partType::lleg;
					Player::ins()->initAimStack();
				}
				else if (checkCursor(&humanRLegBox))
				{
					targetPartRect = humanRLegBox;
					targetPart = partType::rleg;
					Player::ins()->initAimStack();
				}
			}
		}
		else if (checkCursor(&aimTitle))
		{
		}
		else
		{
			if (inputType != input::keyboard)//클릭한 타일의 좌표를 얻어대는 함수
			{
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
				revX += sgn(revX) * (8 * zoomScale);
				revGridX = revX / (16 * zoomScale);
				revY += sgn(revY) * (8 * zoomScale);
				revGridY = revY / (16 * zoomScale);

				//상대좌표를 절대좌표로 변환
				clickTile.x = targetX + revGridX;
				clickTile.y = targetY + revGridY;
			}

			prt(L"절대좌표 (%d,%d) 타일을 터치했다.\n", clickTile.x, clickTile.y);

			if (clickTile.x <= Player::ins()->getGridX() + DARK_VISION_HALF_W && clickTile.x >= Player::ins()->getGridX() - DARK_VISION_HALF_W)
			{
				if (clickTile.y <= Player::ins()->getGridY() + DARK_VISION_HALF_H && clickTile.y >= Player::ins()->getGridY() - DARK_VISION_HALF_H)
				{
					cameraX += 16 * (clickTile.x - targetX);
					cameraY += 16 * (clickTile.y - targetY);
					targetX = clickTile.x;
					targetY = clickTile.y;
					victimEntity = (Entity*)World::ins()->getTile(targetX, targetY, targetZ).EntityPtr;
					if (victimEntity == nullptr) {}
					else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::zombie)
					{
						targetPartRect = zombieTorsoBox;
						targetPart = partType::torso;
					}
					else if (victimEntity->entityInfo.bodyTemplate == bodyTemplateFlag::human)
					{
						targetPartRect = humanTorsoBox;
						targetPart = partType::torso;
					}
				}
			}
		}
	}
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		if (victimEntity != nullptr)
		{
			//만약 목표 엔티티가 움직였으면 해당 엔티티를 추적해서 좌표를 수정함
			if (World::ins()->getTile(targetX, targetY, targetZ).EntityPtr != victimEntity)
			{
				//플레이어의 시야에 있는 모든 객체들을 체크
				std::array<int, 2> nearCoord = { 0,0 };//상대좌표
				int playerX = Player::ins()->getGridX();
				int playerY = Player::ins()->getGridY();
				int playerZ = Player::ins()->getGridZ();
				for (int i = playerX - DARK_VISION_HALF_W; i <= playerX + DARK_VISION_HALF_W; i++)
				{
					for (int j = playerY - DARK_VISION_HALF_H; j <= playerY + DARK_VISION_HALF_H; j++)
					{
						if (World::ins()->getTile(i, j, playerZ).fov == fovFlag::white)
						{
							//없는 타일이거나 플레이어 개체는 제외함
							if (World::ins()->getTile(i, j, playerZ).EntityPtr == victimEntity)
							{
								nearCoord = { i - playerX, j - playerY };
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
					int originX = targetX;
					int originY = targetY;
					targetX = playerX + nearCoord[0];
					targetY = playerY + nearCoord[1];
					cameraX += 16 * (targetX - originX);
					cameraY += 16 * (targetY - originY);
				}
			}
		}

		if (Player::ins()->getAimWeaponIndex() >= 0)
		{
			std::vector<ItemData> playerEquipInfo = Player::ins()->getEquipPtr()->itemInfo;
			if (existMode == false && playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeTemplate > 0)
			{
				existMode = true;
				changeXY(cameraW / 2 + 210, cameraH / 2 - 26, true);
			}
			else if (existMode == true && playerEquipInfo[Player::ins()->getAimWeaponIndex()].modeTemplate == 0)
			{
				existMode = false;
				changeXY(cameraW / 2 + 210, cameraH / 2 - 26, true);
			}
		}
		else
		{
			if (existMode == true)
			{
				existMode = false;
				changeXY(cameraW / 2 + 210, cameraH / 2 - 26, true);
			}
		}
	}

	void drawSinglePart(SDL_Rect partRect, int sprIndex, SDL_Point partPoint, std::wstring partName, int targetPartType, Sprite* inputTmpSpr, float acc)
	{
		int markerIndex = 0;
		if (timer::timer600 % 30 < 5) { markerIndex = 0; }
		else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
		else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
		else if (timer::timer600 % 30 < 20) { markerIndex = 1; }
		else { markerIndex = 0; }

		int dCol = 0;
		if (timer::timer600 % 30 < 4) { dCol = 0; }
		else if (timer::timer600 % 30 < 8) { dCol = 30; }
		else if (timer::timer600 % 30 < 12) { dCol = 60; }
		else if (timer::timer600 % 30 < 16) { dCol = 90; }
		else if (timer::timer600 % 30 < 20) { dCol = 60; }
		else if (timer::timer600 % 30 < 24) { dCol = 30; }
		else { dCol = 0; }

		int currentHP, maxHP, gaugeSprIndex;
		SDL_Color partColor = { 0xff, 0xff, 0xff };
		for (int i = 0; i < victimEntity->entityInfo.parts.size(); i++)
		{
			if (victimEntity->entityInfo.parts[i][partsFlag::index] == targetPartType)
			{
				currentHP = victimEntity->entityInfo.parts[i][partsFlag::hp];
				maxHP = victimEntity->entityInfo.parts[i][partsFlag::maxHP];
				float ratioHP = (float)currentHP / (float)maxHP; // 0~1.0
				gaugeSprIndex = floor(15 - 15.0 * ratioHP);
				break;
			}

			errorBox(i == victimEntity->entityInfo.parts.size() - 1, "cannot find input partIndex");
		}

		if (gaugeSprIndex < 3) { partColor = lowCol::green; }
		else if (gaugeSprIndex < 7) { partColor = lowCol::yellow; }
		else if (gaugeSprIndex < 11) { partColor = lowCol::orange; }
		else { partColor = lowCol::red; }

		SDL_Color partColorDark = { myMax(0,partColor.r - dCol),myMax(0,partColor.g - dCol),myMax(0,partColor.b - dCol) };

		//조준 중인 부위일 경우 
		if (targetPartRect.x == partRect.x && targetPartRect.y == partRect.y) { SDL_SetTextureColorMod(inputTmpSpr->getTexture(), partColorDark.r, partColorDark.g, partColorDark.b); }
		else { SDL_SetTextureColorMod(inputTmpSpr->getTexture(), partColor.r, partColor.g, partColor.b); }

		drawSpriteCenter(inputTmpSpr, sprIndex, tmpPivotX, tmpPivotY);

		if (targetPartRect.x == partRect.x && targetPartRect.y == partRect.y)//조준 중인 부위일 경우
		{
			drawSpriteCenter(spr::aimMarkerTmp, markerIndex, partPoint.x, partPoint.y);
		}

		SDL_Color btnColor = col::black;

		if (targetPartRect.x == partRect.x && targetPartRect.y == partRect.y) { btnColor = lowCol::blue; }

		if (checkCursor(&partRect))
		{
			if (click == true) { btnColor = lowCol::deepBlue; }
			else { btnColor = lowCol::blue; }
		}

		drawFillRect(partRect, btnColor);
		drawRect(partRect, lowCol::green);
		setFontSize(8);
		drawText(col2Str(lowCol::white) + partName, partRect.x + 3, partRect.y + 3);
		setFontSize(10);
		drawText(col2Str(lowCol::white) + std::to_wstring((int)(acc * 100)) + L"%", partRect.x + 3 + 38, partRect.y + 3);
		drawSprite(spr::partsSlotGauge, gaugeSprIndex, partRect.x + 1, partRect.y + 17);
		setFontSize(7);
		drawText(col2Str(lowCol::white) + std::to_wstring(currentHP) + L"/" + std::to_wstring(maxHP), partRect.x + 12, partRect.y + 17 + 5);
	}
};