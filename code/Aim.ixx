#include <SDL.h>

export module Aim;

import std;
import util;
import GUI;
import textureVar;
import wrapVar;
import drawText;
import drawSprite;
import globalVar;
import wrapVar;
import checkCursor;
import drawWindow;
import Player;
import World;
import Entity;
import log;
import mouseGrid;
import ItemData;

export class Aim : public GUI
{
private:
	inline static Aim* ptr = nullptr;
	SDL_Rect aimBase;
	std::vector<std::array<int, 3>> aimTrailRev;
	Point3 aimCoord = { 0,0,0 };

	double aimAcc = 0;
	double fakeAimAcc = aimAcc;
	int aimStack = 0;
	atkType targetAtkType = atkType::shot;

	bool deactClickUp = false;
public:
	Aim() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeAim;


		auto pEquip = PlayerPtr->getEquipPtr();
		if (pEquip->itemInfo[0].checkFlag(itemFlag::CROSSBOW)) PlayerPtr->setSpriteIndex(charSprIndex::AIM_RIFLE);
		else if (pEquip->itemInfo[0].checkFlag(itemFlag::BOW)) PlayerPtr->setSpriteIndex(charSprIndex::AIM_RIFLE);

		

		int pX = PlayerX(), pY = PlayerY(), pZ = PlayerZ();
		
		if(PlayerPtr->entityInfo.sprFlip == false) aimCoord = { pX + 1, pY, pZ };
		else  aimCoord = { pX - 1, pY, pZ };

		Entity* nearTarget = nullptr;
		double hiDist = 99999;
		const int searchRange = 10;
		for (int dx = -searchRange; dx <= searchRange; dx++)
		{
			for (int dy = -searchRange; dy <= searchRange; dy++)
			{
				if (TileFov(pX + dx, pY + dy, pZ) == fovFlag::white)
				{
					if (TileEntity(pX + dx, pY + dy, pZ) != nullptr)
					{
						if (TileEntity(pX + dx, pY + dy, pZ) != PlayerPtr)
						{
							if (std::sqrt(dx * dx + dy * dy) < hiDist)
							{
								nearTarget = TileEntity(pX + dx, pY + dy, pZ);
								hiDist = std::sqrt(dx * dx + dy * dy);
							}
						}
					}
				}
			}
		}

		if (nearTarget != nullptr)
		{
			changeAimTarget(nearTarget->getGridX(), nearTarget->getGridY());
		}
	}
	~Aim()
	{
		PlayerPtr->setSpriteIndex(charSprIndex::WALK);
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Aim* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		aimBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			aimBase.x += inputX;
			aimBase.y += inputY;
		}
		else
		{
			aimBase.x += inputX - aimBase.w / 2;
			aimBase.y += inputY - aimBase.h / 2;
		}



		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - aimBase.w / 2;
			y = inputY - aimBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		SDL_Rect dst;

		setFontSize(13);


		if (aimAcc != 0 && turnCycle == turn::playerInput)
		{
			std::wstring accStr = decimalCutter(fakeAimAcc * 100.0, 1);
			accStr += L"%";
			//if(aimStack>0) accStr += L" (" + std::to_wstring(aimStack) + L")";

			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2 + 2, cameraH / 2 - 20 * zoomScale);
			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2 - 2, cameraH / 2 - 20 * zoomScale);
			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2, cameraH / 2 + 2 - 20 * zoomScale);
			drawTextCenter(col2Str(col::black) + accStr, cameraW / 2, cameraH / 2 - 2 - 20 * zoomScale);

			drawTextCenter(col2Str(col::white) + accStr, cameraW / 2, cameraH / 2 - 20 * zoomScale);
		}


		if(checkCursor(&tab)==false && checkCursor(&letterbox) == false && checkCursor(&quickSlotRegion) == false)
		{
			double tileSize = 16 * zoomScale;
			int tgtX = getAbsMouseGrid().x;
			int tgtY = getAbsMouseGrid().y;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = tileSize;
			dst.h = tileSize;



			setZoom(zoomScale);
			SDL_SetTextureAlphaMod(spr::aimMarkerWhite->getTexture(), 150); //텍스쳐 투명도 설정
			drawSpriteCenter
			(
				spr::aimMarkerWhite,
				0,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			SDL_SetTextureAlphaMod(spr::aimMarkerWhite->getTexture(), 255); //텍스쳐 투명도 설정
			setZoom(1.0);
		}

		{
			int tgtX = aimCoord.x;
			int tgtY = aimCoord.y;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = 16.0 * zoomScale;
			dst.h = 16.0 * zoomScale;
			setZoom(zoomScale);


			int sprIndex = 0;
			if (SDL_GetTicks() % 800 < 200) sprIndex = 0;
			else if (SDL_GetTicks() % 800 < 400) sprIndex = 1;
			else if (SDL_GetTicks() % 800 < 600) sprIndex = 2;
			else sprIndex = 1;

			drawSpriteCenter
			(
				spr::aimMarkerTmp,
				sprIndex,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			setZoom(1.0);
		}

		for (int i = 0; i < aimTrailRev.size(); i++)
		{
			int tgtX = aimTrailRev[i][0];
			int tgtY = aimTrailRev[i][1];
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = 16.0 * zoomScale;
			dst.h = 16.0 * zoomScale;
			setZoom(zoomScale);
			drawSpriteCenter
			(
				spr::trail,
				1,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			setZoom(1.0);
		}


		drawSpriteCenter(spr::floatLog, 0, cameraW / 2, 52);
		drawTextCenter(L"#FFFFFF사격할 위치를 선택해주세요.", cameraW / 2, 52);
	}

	void changeAimTarget(int tgtX, int tgtY)
	{
		aimCoord.x = tgtX;
		aimCoord.y = tgtY;

		if (TileEntity(aimCoord.x, aimCoord.y, aimCoord.z) != nullptr) aimAcc = randomRangeFloat(0.5, 0.8);
		else aimAcc = 0;
		fakeAimAcc = aimAcc;
		aimStack = 0;

		if (aimCoord.x < PlayerX()) PlayerPtr->setDirection(4);
		else if (aimCoord.x > PlayerX()) PlayerPtr->setDirection(0);

		std::vector<std::array<int, 2>> lineCoord;
		makeLine(lineCoord, aimCoord.x - PlayerX(), aimCoord.y - PlayerY());
		aimTrailRev.clear();
		for (int i = 0; i < lineCoord.size(); i++)
		{
			if (lineCoord[i][0] != 0 || lineCoord[i][1] != 0)
			{
				aimTrailRev.push_back({ PlayerX() + lineCoord[i][0],PlayerY() + lineCoord[i][1],PlayerZ() });
			}
		}
	}
	void aimAddAcc()
	{
		if (aimAcc != 0)
		{
			aimAcc += randomRangeFloat(0.03, 0.07);
			if (aimAcc > 0.999) aimAcc = 0.999;
			PlayerPtr->setFlashType(1);
			aimStack++;
		}
	}

	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tabSmallBox))
		{
			aimAddAcc();
		}
		else if (checkCursor(&tab))
		{
			executeTab();
		}
	}
	void clickMotionGUI(int dx, int dy) {}
	void clickDownGUI() 
	{
		if (checkCursor(&letterbox) || checkCursor(&quickSlotRegion))
		{
			close(aniFlag::null);
		}
		else if (checkCursor(&tab) == false)
		{
			changeAimTarget(getAbsMouseGrid().x, getAbsMouseGrid().y);
		}
		
	}
	void clickRightGUI() {}
	void clickHoldGUI() {}
	void mouseWheel() {}
	void gamepadBtnDown() 
	{
		switch (event.cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			aimAddAcc();
			break;
		case SDL_CONTROLLER_BUTTON_B:
			close(aniFlag::null);
			break;

		}
	}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}
	void step() 
	{
		{
			Uint8 targetR, targetG, targetB, targetAlpha;
			PlayerPtr->getFlashRGBA(targetR, targetG, targetB, targetAlpha);
			if (PlayerPtr->getFlashType() == 1)
			{
				if (targetAlpha > 17) { targetAlpha -= 17; }
				else 
				{ 
					targetAlpha = 0; 
					PlayerPtr->setFlashType(0);
				}
				PlayerPtr->setFlashRGBA(targetR, targetG, targetB, targetAlpha);
			}
		}

		if (fabs(aimAcc - fakeAimAcc) > 0.002)
		{
			if (aimAcc > fakeAimAcc)
			{
				fakeAimAcc += 0.004;
			}
			else if (aimAcc < fakeAimAcc)
			{
				fakeAimAcc -= 0.004;
			}
		}



		if (option::inputMethod == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 1000)
			{
				prt(L"탭이 실행되었다.\n");
				executeTab();
				delayR2 = 20;
			}
			else delayR2--;


			if (dpadDelay <= 0)
			{
				dpadDelay = 6;
				int dir = -1;
				SDL_PollEvent(&event);
				bool dpadUpPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
				bool dpadDownPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				bool dpadLeftPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				bool dpadRightPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

				if (dpadUpPressed && dpadLeftPressed) dir = 3;
				else if (dpadUpPressed && dpadRightPressed) dir = 1;
				else if (dpadDownPressed && dpadLeftPressed) dir = 5;
				else if (dpadDownPressed && dpadRightPressed) dir = 7;
				else if (dpadUpPressed) dir = 2;
				else if (dpadDownPressed) dir = 6;
				else if (dpadLeftPressed) dir = 4;
				else if (dpadRightPressed) dir = 0;

				if (dir != -1)
				{
					int dx, dy;
					dir2Coord(dir, dx, dy);
					changeAimTarget(aimCoord.x + dx, aimCoord.y + dy);
				}
			}
			else dpadDelay--;
		}
	}

	

	void executeTab()
	{
		Entity* victimEntity = TileEntity(aimCoord.x, aimCoord.y, aimCoord.z);
		int targetX = aimCoord.x;
		int targetY = aimCoord.y;
		int targetZ = aimCoord.z;
		int weaponRange = 10;

		PlayerPtr->aimWeaponLeft();

		if (victimEntity == nullptr)
		{
			return;
		}
		//맨손 사거리 이탈
		else if (PlayerPtr->getAimWeaponIndex() == -1)
		{
			if (1 < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY)))
			{
				return;
			}

			PlayerPtr->startAtk(targetX, targetY, targetZ);
			turnWait(1.0);
			PlayerPtr->initAimStack();

		}
		//무기 사거리 이탈
		else
		{
			if (targetAtkType == atkType::throwing) //던지기
			{
				if (weaponRange < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY)) ) { return; }

				//자기 자신에게 던지는 경우도 고려해야 되나?
				std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
				PlayerPtr->getEquipPtr()->transferItem(drop.get(), PlayerPtr->getAimWeaponIndex(), 1);
				PlayerPtr->throwing(drop.get(), targetX, targetY);
				PlayerPtr->updateStatus();
				PlayerPtr->updateCustomSpriteHuman();
				updateLog(L"#FFFFFF아이템을 던졌다.");


				PlayerPtr->startAtk(targetX, targetY, targetZ, aniFlag::throwing);
				turnWait(1.0);
				PlayerPtr->initAimStack();
				PlayerPtr->setNextAtkType(targetAtkType);
			}
			else if (targetAtkType == atkType::shot) //사격
			{
				ItemData& tmpAimWeapon = PlayerPtr->getEquipPtr()->itemInfo[PlayerPtr->getAimWeaponIndex()];
				if (getBulletNumber(tmpAimWeapon) > 0) //사격인데 총알이 없을 경우
				{
					if (weaponRange < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY))) { return; }
				}
				else return;

				//직탄식 총
				if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
				{
					popTopBullet(tmpAimWeapon.pocketPtr.get());
				}
				//탄창식 총
				else if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					popTopBullet(tmpAimWeapon.pocketPtr.get()->itemInfo[0].pocketPtr.get());
				}

				auto pEquip = PlayerPtr->getEquipPtr();
				if (pEquip->itemInfo[0].checkFlag(itemFlag::BOW)) PlayerPtr->setSpriteIndex(charSprIndex::WALK);

				PlayerPtr->startAtk(targetX, targetY, targetZ, aniFlag::shotSingle);
				turnWait(1.0);
				PlayerPtr->initAimStack();
				aimAcc = 0.6;
				PlayerPtr->setNextAtkType(targetAtkType);
			}
			else//근접공격
			{
				if (weaponRange < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY))) { return; }

				PlayerPtr->startAtk(targetX, targetY, targetZ);
				turnWait(1.0);
				PlayerPtr->initAimStack();
				PlayerPtr->setNextAtkType(targetAtkType);
			}
		}
	}
};