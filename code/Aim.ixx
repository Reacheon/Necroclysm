#include <SDL.h>

export module Aim;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import Player;
import World;
import Entity;
import log;
import mouseGrid;

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
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//�޼��� �ڽ� ������
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeAim;
		Player::ins()->setSpriteIndex(charSprIndex::AIM_RIFLE);

		int pX = Player::ins()->getGridX(), pY = Player::ins()->getGridY(), pZ = Player::ins()->getGridZ();
		
		if(Player::ins()->entityInfo.sprFlip == false) aimCoord = { pX + 1, pY, pZ };
		else  aimCoord = { pX - 1, pY, pZ };

		Entity* nearTarget = nullptr;
		double hiDist = 99999;
		const int searchRange = 10;
		for (int dx = -searchRange; dx <= searchRange; dx++)
		{
			for (int dy = -searchRange; dy <= searchRange; dy++)
			{
				if (World::ins()->getTile(pX + dx, pY + dy, pZ).fov == fovFlag::white)
				{
					if (World::ins()->getTile(pX + dx, pY + dy, pZ).EntityPtr != nullptr)
					{
						if (World::ins()->getTile(pX + dx, pY + dy, pZ).EntityPtr != Player::ins())
						{
							if (std::sqrt(dx * dx + dy * dy) < hiDist)
							{
								nearTarget = (Entity*)World::ins()->getTile(pX + dx, pY + dy, pZ).EntityPtr;
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
		Player::ins()->setSpriteIndex(charSprIndex::WALK);
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
			SDL_SetTextureAlphaMod(spr::aimMarkerWhite->getTexture(), 150); //�ؽ��� ���� ����
			drawSpriteCenter
			(
				spr::aimMarkerWhite,
				0,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			SDL_SetTextureAlphaMod(spr::aimMarkerWhite->getTexture(), 255); //�ؽ��� ���� ����
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
		drawTextCenter(L"#FFFFFF����� ��ġ�� �������ּ���.", cameraW / 2, 52);
	}

	void changeAimTarget(int tgtX, int tgtY)
	{
		aimCoord.x = tgtX;
		aimCoord.y = tgtY;

		if (World::ins()->getTile(aimCoord.x, aimCoord.y, aimCoord.z).EntityPtr != nullptr) aimAcc = randomRangeFloat(0.5, 0.8);
		else aimAcc = 0;
		fakeAimAcc = aimAcc;
		aimStack = 0;

		if (aimCoord.x < Player::ins()->getGridX()) Player::ins()->setDirection(4);
		else if (aimCoord.x > Player::ins()->getGridX()) Player::ins()->setDirection(0);

		std::vector<std::array<int, 2>> lineCoord;
		makeLine(lineCoord, aimCoord.x - Player::ins()->getGridX(), aimCoord.y - Player::ins()->getGridY());
		aimTrailRev.clear();
		for (int i = 0; i < lineCoord.size(); i++)
		{
			if (lineCoord[i][0] != 0 || lineCoord[i][1] != 0)
			{
				aimTrailRev.push_back({ Player::ins()->getGridX() + lineCoord[i][0],Player::ins()->getGridY() + lineCoord[i][1],Player::ins()->getGridZ() });
			}
		}
	}
	void aimAddAcc()
	{
		if (aimAcc != 0)
		{
			aimAcc += randomRangeFloat(0.03, 0.07);
			if (aimAcc > 0.999) aimAcc = 0.999;
			Player::ins()->setFlashType(1);
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
			Player::ins()->getFlashRGBA(targetR, targetG, targetB, targetAlpha);
			if (Player::ins()->getFlashType() == 1)
			{
				if (targetAlpha > 17) { targetAlpha -= 17; }
				else { targetAlpha = 0; }
				Player::ins()->setFlashRGBA(targetR, targetG, targetB, targetAlpha);
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



		if (inputType == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 1000)
			{
				prt(L"���� ����Ǿ���.\n");
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
		Entity* victimEntity = (Entity*)World::ins()->getTile(aimCoord.x, aimCoord.y, aimCoord.z).EntityPtr;
		int targetX = aimCoord.x;
		int targetY = aimCoord.y;
		int targetZ = aimCoord.z;
		int weaponRange = 10;

		Player::ins()->aimWeaponLeft();

		if (victimEntity == nullptr)
		{
			return;
		}
		//�Ǽ� ��Ÿ� ��Ż
		else if (Player::ins()->getAimWeaponIndex() == -1)
		{
			if (1 < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)))
			{
				return;
			}

			Player::ins()->startAtk(targetX, targetY, targetZ);
			turnWait(1.0);
			Player::ins()->initAimStack();

		}
		//���� ��Ÿ� ��Ż
		else
		{
			if (targetAtkType == atkType::throwing) //������
			{
				if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY)) ) { return; }

				/*if (targetX != Player::ins()->getGridX() || targetY != Player::ins()->getGridY())
				{*/

				//�ڱ� �ڽſ��� ������ ��쵵 ����ؾ� �ǳ�?
				ItemPocket* drop = new ItemPocket(storageType::null);
				Player::ins()->getEquipPtr()->transferItem(drop, Player::ins()->getAimWeaponIndex(), 1);
				Player::ins()->throwing(drop, targetX, targetY);
				Player::ins()->updateStatus();
				updateLog(L"#FFFFFF�������� ������.");
				//}
				//else
				//{
				//	ItemPocket* drop = new ItemPocket(storageType::null);
				//	Player::ins()->getEquipPtr()->transferItem(drop, Player::ins()->getAimWeaponIndex(), 1);
				//	Player::ins()->drop(drop);
				//	Player::ins()->updateStatus();
				//	updateLog(L"#FFFFFF�������� ���ȴ�.");
				//}

				Player::ins()->startAtk(targetX, targetY, targetZ, aniFlag::throwing);
				turnWait(1.0);
				Player::ins()->initAimStack();
				Player::ins()->setNextAtkType(targetAtkType);
			}
			else if (targetAtkType == atkType::shot) //���
			{
				ItemData tmpAimWeapon = Player::ins()->getEquipPtr()->itemInfo[Player::ins()->getAimWeaponIndex()];
				if (getBulletNumber(tmpAimWeapon) > 0) //����ε� �Ѿ��� ���� ���
				{
					if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY))) { return; }
				}
				else return;

				//��ź�� ��
				if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
				{
					popTopBullet(((ItemPocket*)tmpAimWeapon.pocketPtr));
				}
				//źâ�� ��
				else if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					popTopBullet((ItemPocket*)((ItemPocket*)tmpAimWeapon.pocketPtr)->itemInfo[0].pocketPtr);
				}

				Player::ins()->startAtk(targetX, targetY, targetZ, aniFlag::shotSingle);
				turnWait(1.0);
				Player::ins()->initAimStack();
				aimAcc = 0.6;
				Player::ins()->setNextAtkType(targetAtkType);
			}
			else//��������
			{
				if (weaponRange < myMax(abs(Player::ins()->getGridX() - targetX), abs(Player::ins()->getGridY() - targetY))) { return; }

				Player::ins()->startAtk(targetX, targetY, targetZ);
				turnWait(1.0);
				Player::ins()->initAimStack();
				Player::ins()->setNextAtkType(targetAtkType);
			}
		}
	}
};