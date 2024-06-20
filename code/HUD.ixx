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
import drawPrimitive;
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

namespace segmentIndex
{
	int zero = 0;
	int one = 1;
	int two = 2;
	int three = 3;
	int four = 4;
	int five = 5;
	int six = 6;
	int seven = 7;
	int eight = 8;
	int nine = 9;
	int dash = 10;
	int colon = 11;
	int pm = 12;
	int am = 13;
	float popUpDist = 360;

};

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
		tab = { 20, 20, 120, 120 };


		quickSlotRegion =  { cameraW - 1 - 42 - quickSlotDist,cameraH / 2 - 177,180,358, } ;
		quickSlotPopBtn = { quickSlotRegion.x, quickSlotRegion.y, 43, 38 };
		for (int i = 0; i < 8; i++)
		{
			quickSlotBtn[i] = { quickSlotRegion.x, quickSlotRegion.y + 40 * (i + 1), 180,38 };
		}


		x = 0;
		y = inputY;
	}
	void drawGUI()
	{
		const Uint8* state = SDL_GetKeyboardState(NULL);
		Sprite* targetBtnSpr = nullptr;
		
		//SDL_SetRenderDrawColor(renderer, 0x87, 0xCE, 0xEB, 200);
		//SDL_RenderDrawPoint(renderer, cameraW / 2, cameraH / 2);

		drawStadium(letterbox.x, letterbox.y, letterbox.w, letterbox.h + 10, { 0,0,0 }, 150, 5);
		if (ctrlVeh != nullptr) drawSpriteCenter(spr::vehicleHUD, 0, cameraW / 2, cameraH + 73);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if (isAdvancedMode == false)
		{
			SDL_SetTextureBlendMode(texture::minimap, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(texture::minimap, 160);
			setZoom(4.0);
			
			
			drawTextureCenter(texture::minimap, cameraW - 94, 94);
			setZoom(1.0);

			int vShift = 384 * (ctrlVeh != nullptr);

			if (1)//플레이어 이름 그리기
			{
				setFontSize(12);
				SDL_SetRenderDrawColor(renderer, lowCol::yellow.r, lowCol::yellow.g, lowCol::yellow.b, 0xff);
				drawText(L"Jackson, Practitioner of Elivilon ******", letterbox.x + 18 + vShift, letterbox.y + 1);
			}

			if (1)//플레이어 HP바 그리기
			{
				SDL_Rect hpRect = { letterbox.x + 18 + vShift,letterbox.y + 20, 216,4 };
				__int16 currentHP = Player::ins()->entityInfo.HP;
				__int16	fakeHP = Player::ins()->entityInfo.fakeHP;
				__int16 maxHP = Player::ins()->entityInfo.maxHP;

				drawStadium(hpRect.x, hpRect.y, hpRect.w, hpRect.h, col::black, 255, 1);

				float ratioFakeHP = myMax((float)0.0, (float)(fakeHP) / (float)(maxHP));
				drawStadium(hpRect.x, hpRect.y, hpRect.w * ratioFakeHP, hpRect.h, col::white, Player::ins()->entityInfo.fakeHPAlpha, 1);

				float ratioHP = myMax((float)0.0, (float)(currentHP) / (float)(maxHP));
				drawStadium(hpRect.x, hpRect.y, hpRect.w * ratioHP, hpRect.h, { 0x75, 0xf0, 0x3f }, 255, 1);

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

			if (1)//플레이어 부위별 HP 그리기
			{
				setFontSize(12);
				drawText(col2Str(col::lightGray) + L"LARM", letterbox.x + 18 + +vShift + 80 * 0, letterbox.y + 9 + 15 * 1);
				drawText(col2Str(lowCol::green) + L"|||||", letterbox.x + 18 + +vShift + 38 + 80 * 0, letterbox.y + 9 + 15 * 1);
				drawText(col2Str(col::lightGray) + L"LLEG", letterbox.x + 18 + +vShift + 80 * 0, letterbox.y + 8 + 15 * 2);
				drawText(col2Str(lowCol::green) + L"|||||", letterbox.x + 18 + +vShift + 38 + 80 * 0, letterbox.y + 8 + 15 * 2);

				drawText(col2Str(col::lightGray) + L"HEAD", letterbox.x + 18 + +vShift + 80 * 1, letterbox.y + 9 + 15 * 1);
				drawText(col2Str(lowCol::green) + L"|||||", letterbox.x + 18 + +vShift + 38 + 80 * 1, letterbox.y + 9 + 15 * 1);
				drawText(col2Str(col::lightGray) + L"TORSO", letterbox.x + 18 + +vShift + 80 * 1, letterbox.y + 8 + 15 * 2);
				drawText(col2Str(lowCol::green) + L"|||||", letterbox.x + 18 + +vShift + 38 + 80 * 1, letterbox.y + 8 + 15 * 2);

				drawText(col2Str(col::lightGray) + L"RARM", letterbox.x + 18 + +vShift + 80 * 2, letterbox.y + 9 + 15 * 1);
				drawText(col2Str(lowCol::green) + L"|||||", letterbox.x + 18 + +vShift + 38 + 80 * 2, letterbox.y + 9 + 15 * 1);
				drawText(col2Str(col::lightGray) + L"RLEG", letterbox.x + 18 + +vShift + 80 * 2, letterbox.y + 8 + 15 * 2);
				drawText(col2Str(lowCol::green) + L"|||||", letterbox.x + 18 + +vShift + 38 + 80 * 2, letterbox.y + 8 + 15 * 2);
			}

			if (ctrlVeh == nullptr)
			{
				setFontSize(10);
				drawSprite(spr::staminaGauge, 6, letterbox.x + 18 + 238, letterbox.y + 4);
				drawTextCenter(col2Str(lowCol::yellow) + L"STA", letterbox.x + 18 + 238 + 24, letterbox.y + 4 + 16);
				drawTextCenter(col2Str(lowCol::green) + L"72%", letterbox.x + 18 + 238 + 24, letterbox.y + 4 + 14 + 15);

				setFontSize(12);
				drawText(col2Str(col::lightGray) + L"SPEED", letterbox.x + 18 + 296, letterbox.y + 3 + 15 * 1);
				drawText(col2Str(lowCol::green) + L"120%", letterbox.x + 18 + 44 + 296, letterbox.y + 3 + 15 * 1);
				drawText(col2Str(col::lightGray) + L"MENTAL", letterbox.x + 18 + 296, letterbox.y + 3 + 15 * 2);
				drawText(col2Str(lowCol::green) + L"XD", letterbox.x + 18 + 44 + 296, letterbox.y + 3 + 15 * 2);

				//시간 표시 기능
				{
					int timeIndex;
					// PM or AM
					if (getHour() >= 12) { timeIndex = segmentIndex::pm; }
					else { timeIndex = segmentIndex::am; }
					drawSprite(spr::segment, timeIndex, letterbox.x + 18 + 446 + 14 * 0, letterbox.y + 10);
					//xx시
					drawSprite(spr::segment, getHour() / 10, letterbox.x + 18 + 446 + 14 * 1, letterbox.y + 10);
					drawSprite(spr::segment, getHour() % 10, letterbox.x + 18 + 446 + 14 * 2, letterbox.y + 10);
					//:
					drawSprite(spr::segment, segmentIndex::colon, letterbox.x + 18 + 446 + 14 * 3, letterbox.y + 10);
					//xx분
					drawSprite(spr::segment, getMin() / 10, letterbox.x + 18 + 446 + 14 * 4, letterbox.y + 10);
					drawSprite(spr::segment, getMin() % 10, letterbox.x + 18 + 446 + 14 * 5, letterbox.y + 10);
					//xx초
					setZoom(0.7);
					drawSprite(spr::segment, getSec() / 10, letterbox.x + 18 + 446 + 14 * 6, letterbox.y + 15);
					drawSprite(spr::segment, getSec() % 10, letterbox.x + 18 + 446 + 14 * 6 + 10, letterbox.y + 15);

					//xxxx년
					drawSprite(spr::segment, getYear() / 1000, letterbox.x + 18 + 460 + 10 * -1, letterbox.y + 7 + 25);
					drawSprite(spr::segment, (getYear() % 1000) / 100, letterbox.x + 18 + 460 + 10 * 0, letterbox.y + 7 + 25);
					drawSprite(spr::segment, ((getYear() % 100) / 10), letterbox.x + 18 + 460 + 10 * 1, letterbox.y + 7 + 25);
					drawSprite(spr::segment, getYear() % 10, letterbox.x + 18 + 460 + 10 * 2, letterbox.y + 7 + 25);
					//:
					drawSprite(spr::segment, segmentIndex::dash, letterbox.x + 18 + 460 + 10 * 3, letterbox.y + 7 + 25);
					//xx월
					drawSprite(spr::segment, getMonth() / 10, letterbox.x + 18 + 460 + 10 * 4, letterbox.y + 7 + 25);
					drawSprite(spr::segment, getMonth() % 10, letterbox.x + 18 + 460 + 10 * 5, letterbox.y + 7 + 25);
					//:
					drawSprite(spr::segment, segmentIndex::dash, letterbox.x + 18 + 460 + 10 * 6, letterbox.y + 7 + 25);
					//xx일
					drawSprite(spr::segment, getDay() / 10, letterbox.x + 18 + 460 + 10 * 7, letterbox.y + 7 + 25);
					drawSprite(spr::segment, getDay() % 10, letterbox.x + 18 + 460 + 10 * 8, letterbox.y + 7 + 25);
					setZoom(1.0);
				}

				drawSprite(spr::ecliptic, 2, letterbox.x + 18 + 374, letterbox.y);

				int cx, cy;
				int pz = Player::ins()->getGridZ();
				World::ins()->changeToChunkCoord(Player::ins()->getGridX(), Player::ins()->getGridY(), cx, cy);
				if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::sunny)
				{
					static int index = 0;
					int sprSize = 6;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index == sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolSunny, index, letterbox.x + 426, letterbox.y + 19);
				}
				else if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::cloudy)
				{
					static int index = 0;
					int sprSize = 9;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolCloudy, index, letterbox.x + 426, letterbox.y + 19);
				}
				else if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::rain)
				{
					static int index = 0;
					int sprSize = 3;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolRain, index, letterbox.x + 426, letterbox.y + 19);
				}
				else if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::storm)
				{
					static int index = 0;
					int sprSize = 3;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolStorm, index, letterbox.x + 426, letterbox.y + 19);
				}
				else if (World::ins()->getChunkWeather(cx, cy, pz) == weatherFlag::snow)
				{
					static int index = 0;
					int sprSize = 15;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolSnow, index, letterbox.x + 426, letterbox.y + 19);
				}

				SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
				drawTextCenter(L"15℃", letterbox.x + 18 + 374 + 36, letterbox.y + 16 + 22);

				drawSprite(spr::batteryGauge, 4, letterbox.x + 18 + 562, letterbox.y + 3);
				setFontSize(10);

				drawTextCenter(col2Str(lowCol::black) + L"72%", letterbox.x + 18 + 562 + 16 + 1, letterbox.y + 3 + 24);
				drawTextCenter(col2Str(lowCol::black) + L"72%", letterbox.x + 18 + 562 + 16, letterbox.y + 3 + 24 - 1);
				drawTextCenter(col2Str(lowCol::black) + L"72%", letterbox.x + 18 + 562 + 16 - 1, letterbox.y + 3 + 24);
				drawTextCenter(col2Str(lowCol::black) + L"72%", letterbox.x + 18 + 562 + 16, letterbox.y + 3 + 24 + 1);
				drawTextCenter(col2Str(lowCol::white) + L"72%", letterbox.x + 18 + 562 + 16, letterbox.y + 3 + 24);
			}

			//팝업 버튼
			drawSprite(spr::menuPopUp, 0, letterbox.x + letterbox.w - 42, letterbox.y - 36);
			{
				SDL_Color popUpBtnColor;
				if (checkCursor(&letterboxPopUpButton))
				{
					if (click == true) { popUpBtnColor = lowCol::deepBlue; }
					else { popUpBtnColor = lowCol::blue; }
				}
				else if (state[SDL_SCANCODE_RETURN]) { popUpBtnColor = lowCol::blue; }
				else { popUpBtnColor = lowCol::black; }

				drawStadium(letterboxPopUpButton.x, letterboxPopUpButton.y, letterboxPopUpButton.w, letterboxPopUpButton.h, popUpBtnColor, 200, 5);

				if (inputType != input::keyboard)
				{
					if (y == 0)
					{
						drawSpriteCenter(spr::windowArrow, 1, letterboxPopUpButton.x + 15, letterboxPopUpButton.y + 15);
					}
					else
					{
						drawSpriteCenter(spr::windowArrow, 3, letterboxPopUpButton.x + 15, letterboxPopUpButton.y + 15);
					}
				}
				else
				{
					if (state[SDL_SCANCODE_RETURN]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_Enter, letterboxPopUpButton.x + 15, letterboxPopUpButton.y + 15);
				}
			}

		}
		else
		{

		}

		drawBarAct();
		drawTab();
		drawQuickSlot();

		//디버깅용 좌측상단 하늘색 점
		SDL_SetRenderDrawColor(renderer, 35, 246, 204, 255);
		SDL_RenderDrawPoint(renderer, 0, 0);
	}

	void clickDownGUI()
	{
		executedHold = false;
	}
	void clickMotionGUI(int dx, int dy)
	{
		if (click && dtClickStack > 200)//200ms 이상 누르면 마우스를 움직여 카메라를 움직일 수 있음
		{
			const int maxDist = 160;
			int prevCameraX = cameraX, prevCameraY = cameraY;
			cameraFix = false;
			cameraX -= ((event.motion.x - prevMouseX4Motion) / 2);
			cameraY -= ((event.motion.y - prevMouseY4Motion) / 2);
			disableClickUp4Motion = true;

			if (std::abs(Player::ins()->getX() - cameraX) > maxDist) cameraX = prevCameraX;
			if (std::abs(Player::ins()->getY() - cameraY) > maxDist) cameraY = prevCameraY;
		}
	}
	void clickUpGUI()
	{
		if (disableClickUp4Motion)
		{
			disableClickUp4Motion = false;
			return;
		}
		if (executedHold) return;

		if (checkCursor(&letterboxPopUpButton) == true)//팝업 기능
		{
			if (y == 0) { executePopUp(); }
			else { executePopDown(); }
		}
		else if (checkCursor(&tabSmallBox) == true)
		{
			//플레이어의 시야에 있는 모든 객체들을 체크
			std::array<int, 2> nearCoord = { 0,0 };//상대좌표
			int playerX = Player::ins()->getGridX();
			int playerY = Player::ins()->getGridY();
			int playerZ = Player::ins()->getGridZ();
			for (int i = playerX - userVisionHalfW; i <= playerX + userVisionHalfW; i++)
			{
				for (int j = playerY - userVisionHalfH; j <= playerY + userVisionHalfH; j++)
				{
					if (World::ins()->getTile(i, j, playerZ).fov == fovFlag::white)
					{
						//없는 타일이거나 플레이어 개체는 제외함
						if (World::ins()->getTile(i, j, playerZ).EntityPtr != nullptr && World::ins()->getTile(i, j, playerZ).EntityPtr != Player::ins())
						{
							if (sqrt(pow(i - playerX, 2) + pow(j - playerY, 2)) < sqrt(pow(nearCoord[0], 2) + pow(nearCoord[1], 2)) || (nearCoord[0] == 0 && nearCoord[1] == 0))//갱신
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
				new Aim(playerX + nearCoord[0], playerY + nearCoord[1], playerZ);
			}
		}
		else if (checkCursor(&tab) == true)
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
		else if (checkCursor(&quickSlotPopBtn))
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
		else if (checkCursor(&letterbox))
		{
			for (int i = 0; i < barAct.size(); i++) // 하단 UI 터치 이벤트
			{
				if (checkCursor(&barButton[i]))
				{
					clickLetterboxBtn(barAct[i]);

					if (y != 0)
					{
						deactInput();
						addAniUSetPlayer(this, aniFlag::popDownLetterbox);
					}
				}
			}
		}
		else if (checkCursor(&quickSlotRegion))//퀵슬롯 이벤트 터치
		{
			for (int i = 0; i < 8; i++)
			{
				if (checkCursor(&quickSlotBtn[i]))
				{
					prt(L"%d번 스킬 슬롯을 눌렀다!\n", i + 1);

					if (skillDex[quickSlot[i].second].src == skillSrc::BIONIC)
					{
						int index = Player::ins()->searchBionicCode(quickSlot[i].second);
						CORO(useSkill(Player::ins()->getBionicList()[index]));
					}
					else if (skillDex[quickSlot[i].second].src == skillSrc::MUTATION)
					{
						int index = Player::ins()->searchMutationCode(quickSlot[i].second);
						CORO(useSkill(Player::ins()->getMutationList()[index]));
					}
					else if (skillDex[quickSlot[i].second].src == skillSrc::MARTIAL_ART)
					{
						int index = Player::ins()->searchMartialArtCode(quickSlot[i].second);
						CORO(useSkill(Player::ins()->getMartialArtList()[index]));
					}
					else if (skillDex[quickSlot[i].second].src == skillSrc::DIVINE_POWER)
					{
						int index = Player::ins()->searchDivinePowerCode(quickSlot[i].second);
						CORO(useSkill(Player::ins()->getDivinePowerList()[index]));
					}
					else if (skillDex[quickSlot[i].second].src == skillSrc::MAGIC)
					{
						int index = Player::ins()->searchMagicCode(quickSlot[i].second);
						CORO(useSkill(Player::ins()->getMagicList()[index]));
					}
				}
			}
		}
		else//타일터치
		{
			cameraFix = true;
			//터치한 좌표를 얻어내는 부분
			int cameraGridX, cameraGridY;
			if (cameraX >= 0) cameraGridX = cameraX / 16;
			else cameraGridX = -1 + cameraX / 16;
			if (cameraY >= 0) cameraGridY = cameraY / 16;
			else cameraGridY = -1 + cameraY / 16;

			int camDelX = cameraX - (16 * cameraGridX + 8);
			int camDelY = cameraY - (16 * cameraGridY + 8);

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
			revX += sgn(revX) * (8 * zoomScale) + camDelX;
			revGridX = revX / (16 * zoomScale);
			revY += sgn(revY) * (8 * zoomScale) + camDelY;
			revGridY = revY / (16 * zoomScale);

			//상대좌표를 절대좌표로 변환
			clickTile.x = cameraGridX + revGridX;
			clickTile.y = cameraGridY + revGridY;
			prt(L"[HUD] 절대좌표 (%d,%d) 타일을 터치했다.\n", clickTile.x, clickTile.y);
			tileTouch(clickTile.x, clickTile.y);
		}
	}
	void step()
	{
		//현재 수련 중인 재능이 없을 경우 강제로 재능 창을 열음
		if (Talent::ins() == nullptr)
		{
			for (int i = 0; i < talentSize; i++)
			{
				if (Player::ins()->getTalentFocus(i) > 0) { break; }

				if (i == talentSize - 1)
				{
					new Talent();
					Talent::ins()->setWarningIndex(1);
				}
			}
		}

		//홀드 이벤트
		if (dtClickStack >= 1000) //1초간 누르고 있을 경우
		{
			if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false && checkCursor(&tabSmallBox) == false)
			{
				//터치한 좌표를 얻어내는 부분

				prt(L"1초 이상 눌렀다.\n");
				int revX, revY, revGridX, revGridY;
				revX = clickDownPoint.x - (cameraW / 2);
				//revY = clickDownPoint.y - (cameraH / 2);
				//revX += sgn(revX) * (8 * zoomScale);
				//revGridX = revX / (16 * zoomScale);
				//revY += sgn(revY) * (8 * zoomScale);
				//revGridY = revY / (16 * zoomScale);
				//dtClickStack = -1;
				//executedHold = true;
				////advancedTileTouch(Player::ins()->getGridX() + revGridX, Player::ins()->getGridY() + revGridY);
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

	void executePopUp()
	{
		isPopUp = true;
		popUpDist = ((barAct.size() - 1) / 7) * 89;
		deactInput();
		addAniUSetPlayer(this, aniFlag::popUpLetterbox);
	}
	void executePopDown()
	{
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
			CORO(closeDoor(Player::ins()->getGridX(),Player::ins()->getGridY(),Player::ins()->getGridZ()));
			break;
		}
		default:
			updateLog(L"#FFFFFF알수없는 레터박스 버튼이 눌렸다.");
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
					new Loot(touchX, touchY);
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
							tgtProp->updateTile();
							Player::ins()->updateVision(Player::ins()->getEyeSight());
							updateNearbyBarAct(Player::ins()->getGridX(), Player::ins()->getGridY(),Player::ins()->getGridZ());
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
				else Player::ins()->setAStarDst(touchX, touchY);
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

	void drawTab()
	{
		SDL_Color btnColor = { 0x00, 0x00, 0x00 };
		if (checkCursor(&tab))
		{
			if (click == false) { btnColor = lowCol::blue; }
			else { btnColor = lowCol::deepBlue; }
		}
\
		switch (tabType)
		{
		case tabFlag::autoAtk:

			if (checkCursor(&tabSmallBox))
			{
				if (click == false) { drawSprite(spr::tab, 3, tab.x, tab.y - 2); }
				else { drawSprite(spr::tab, 4, tab.x, tab.y - 2); }
			}
			else if (checkCursor(&tab))
			{
				if (click == false) { drawSprite(spr::tab, 1, tab.x, tab.y - 2); }
				else { drawSprite(spr::tab, 2, tab.x, tab.y - 2); }
			}
			else
			{
				drawSprite(spr::tab, 0, tab.x, tab.y - 2);
			}

			//drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
			//drawSpriteCenter(spr::icon48, 2, tab.x + 40, tab.y + 39);
			//drawSpriteCenter(spr::icon48, 1, tab.x + 80, tab.y + 45);
			setFontSize(13);
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(sysStr[1], tab.x + 60, tab.y + 92);
			drawTextCenter(sysStr[2], tab.x + 60, tab.y + 92 + 14);
			break;
		case tabFlag::aim:
			drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
			setZoom(1.5);
			drawSpriteCenter(spr::icon48, 42, tab.x + 60, tab.y + 52);
			setZoom(1.0);
			setFontSize(13);
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(sysStr[104], tab.x + 60, tab.y + 92 + 7);
			break;
		case tabFlag::closeWin:
			//창 닫기
			drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
			drawSpriteCenter(spr::icon48, 14, tab.x + 60, tab.y + 52);
			setFontSize(13);
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(sysStr[14], tab.x + 60, tab.y + 92 + 7);
			break;
		case tabFlag::back:
			//뒤로가기
			drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
			drawSpriteCenter(spr::icon48, 31, tab.x + 60, tab.y + 52);
			setFontSize(13);
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(sysStr[31], tab.x + 60, tab.y + 92 + 7);
			break;
		case tabFlag::confirm:
			//뒤로가기
			drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
			drawSpriteCenter(spr::icon48, 39, tab.x + 60, tab.y + 52);
			setFontSize(13);
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(sysStr[91], tab.x + 60, tab.y + 92 + 7);
			break;
		default:
			errorBox("undefined tabFalg");
			break;
		}
	}
	
	void drawQuickSlot()
	{
		int pivotX = quickSlotRegion.x;
		int pivotY = quickSlotRegion.y;
		
		//drawRect({ pivotX,pivotY,180,358, }, lowCol::skyBlue);

		int sprIndex = 0;
		if (isQuickSlotPop)
		{
			if (checkCursor(&quickSlotPopBtn))
			{
				if (click == false) sprIndex = 1;
				else sprIndex = 2;
			}
			else sprIndex = 0;
		}
		else
		{
			if (checkCursor(&quickSlotPopBtn))
			{
				if (click == false) sprIndex = 4;
				else sprIndex = 5;
			}
			else sprIndex = 3;
		}

		drawSprite(spr::topQuickSlotBtn, sprIndex, pivotX, pivotY);
		drawFillRect({ pivotX + 43,pivotY,137,38 }, col::black, 200);

		//개별 스킬창 그리기 1번부터 8번까지
		for (int i = 0; i < 8; i++)
		{
			if(isQuickSlotPop == true && checkCursor(&quickSlotBtn[i]) && quickSlot[i].first != quickSlotFlag::NONE)
			{
				if (click) drawSprite(spr::singleQuickSlot, 2, quickSlotBtn[i].x, quickSlotBtn[i].y);
				else drawSprite(spr::singleQuickSlot, 1, quickSlotBtn[i].x, quickSlotBtn[i].y);
			}
			else drawSprite(spr::singleQuickSlot, 0, quickSlotBtn[i].x, quickSlotBtn[i].y);

			if (quickSlot[i].first == quickSlotFlag::NONE)
			{
				drawCross2(quickSlotBtn[i].x + 6, quickSlotBtn[i].y + 2, 0, 5, 0, 5);
				drawCross2(quickSlotBtn[i].x + 6 + 32, quickSlotBtn[i].y + 2, 0, 5, 5, 0);
				drawCross2(quickSlotBtn[i].x + 6, quickSlotBtn[i].y + 2 + 32, 5, 0, 0, 5);
				drawCross2(quickSlotBtn[i].x + 6 + 32, quickSlotBtn[i].y + 2 + 32, 5, 0, 5, 0);
				drawFillRect({ quickSlotBtn[i].x + 6, quickSlotBtn[i].y + 2 ,34,34 }, col::black, 180);
			}
			else
			{
				setZoom(2.0);
				std::wstring skillName = L"";
				if (skillDex[quickSlot[i].second].src == skillSrc::BIONIC)
				{
					int index = Player::ins()->searchBionicCode(quickSlot[i].second);
					if (index != -1) drawSprite(spr::skillSet, Player::ins()->getBionicList()[index].iconIndex, quickSlotBtn[i].x + 7, quickSlotBtn[i].y + 3);
					skillName = Player::ins()->getBionicList()[index].name;
				}
				else if (skillDex[quickSlot[i].second].src == skillSrc::MUTATION)
				{
					int index = Player::ins()->searchMutationCode(quickSlot[i].second);
					if (index != -1) drawSprite(spr::skillSet, Player::ins()->getMutationList()[index].iconIndex, quickSlotBtn[i].x + 7, quickSlotBtn[i].y + 3);
					skillName = Player::ins()->getMutationList()[index].name;
				}
				else if (skillDex[quickSlot[i].second].src == skillSrc::MARTIAL_ART)
				{
					int index = Player::ins()->searchMartialArtCode(quickSlot[i].second);
					if (index != -1) drawSprite(spr::skillSet, Player::ins()->getMartialArtList()[index].iconIndex, quickSlotBtn[i].x + 7, quickSlotBtn[i].y + 3);
					skillName = Player::ins()->getMartialArtList()[index].name;
				}
				else if (skillDex[quickSlot[i].second].src == skillSrc::DIVINE_POWER)
				{
					int index = Player::ins()->searchDivinePowerCode(quickSlot[i].second);
					if (index != -1) drawSprite(spr::skillSet, Player::ins()->getDivinePowerList()[index].iconIndex, quickSlotBtn[i].x + 7, quickSlotBtn[i].y + 3);
					skillName = Player::ins()->getDivinePowerList()[index].name;
				}
				else if (skillDex[quickSlot[i].second].src == skillSrc::MAGIC)
				{
					int index = Player::ins()->searchMagicCode(quickSlot[i].second);
					if (index != -1) drawSprite(spr::skillSet, Player::ins()->getMagicList()[index].iconIndex, quickSlotBtn[i].x + 7, quickSlotBtn[i].y + 3);
					skillName = Player::ins()->getMagicList()[index].name;
				}
				setZoom(1.0);

				setFontSize(14);
				drawText(L"#FFFFFF" + skillName, quickSlotBtn[i].x + 43, quickSlotBtn[i].y + 5);
				
				//setFontSize(10);
				//drawText(L"#FFFF00-32 MP" + skillName, quickSlotBtn[i].x + 120, quickSlotBtn[i].y + 24);

				drawCross2(quickSlotBtn[i].x + 6, quickSlotBtn[i].y + 2, 0, 5, 0, 5);
				drawCross2(quickSlotBtn[i].x + 6 + 32, quickSlotBtn[i].y + 2, 0, 5, 5, 0);
				drawCross2(quickSlotBtn[i].x + 6, quickSlotBtn[i].y + 2 + 32, 5, 0, 0, 5);
				drawCross2(quickSlotBtn[i].x + 6 + 32, quickSlotBtn[i].y + 2 + 32, 5, 0, 5, 0);
			}
		}
	}

	void drawBarAct()
	{
		if (ctrlVeh != nullptr)
		{
			if (typeHUD == vehFlag::car)
			{
				//핸들
				int wheelIndex = 0;
				SDL_SetTextureAlphaMod(spr::vehicleHUDSteeringWheel->getTexture(), 140);
				SDL_SetTextureBlendMode(spr::vehicleHUDSteeringWheel->getTexture(), SDL_BLENDMODE_BLEND);
				if (click)
				{
					if (checkCursor(&barButton[0])) wheelIndex = 1;
					else if (checkCursor(&barButton[1])) wheelIndex = 3;
					else if (checkCursor(&barButton[2])) wheelIndex = 2;
				}
				drawSpriteCenter(spr::vehicleHUDSteeringWheel, wheelIndex, barButton[1].x + (barButton[1].w / 2), barButton[1].y + (barButton[1].h / 2));

				drawSpriteCenter(spr::vehicleHUDParts, 15, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2); //좌회전 마크
				drawSpriteCenter(spr::vehicleHUDParts, 16, barButton[1].x + barButton[1].w / 2, barButton[1].y + barButton[1].h / 2); //1턴 대기
				drawSpriteCenter(spr::vehicleHUDParts, 17, barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2); //우회전 마크

				drawSpriteCenter(spr::vehicleHUDParts, 0 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트

				//기어(PRND) 그리기
				int gearSprIndex = 0;
				if (((Vehicle*)ctrlVeh)->gearState == gearFlag::park) gearSprIndex = 0;
				else if (((Vehicle*)ctrlVeh)->gearState == gearFlag::reverse) gearSprIndex = 1;
				else if (((Vehicle*)ctrlVeh)->gearState == gearFlag::neutral) gearSprIndex = 2;
				else gearSprIndex = 3;
				drawSpriteCenter(spr::vehicleHUDParts, 5 + gearSprIndex, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 - 12);

				drawSpriteCenter(spr::vehicleHUDParts, 10 + (checkCursor(&barButton[5]) && click), barButton[5].x + barButton[5].w / 2, barButton[5].y + barButton[5].h / 2);
				drawSpriteCenter(spr::vehicleHUDParts, 12 + (checkCursor(&barButton[6]) && click), barButton[6].x + barButton[6].w / 2, barButton[6].y + barButton[6].h / 2);
			}
			else if(typeHUD == vehFlag::heli)
			{
				Vehicle* myHeli = ((Vehicle*)ctrlVeh);

				setZoom(1.5);
				//drawSpriteCenter(spr::collectiveLever, myHeli->collectiveState + 1, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2 - 16); //콜렉티브 레버
				setZoom(1);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
				drawSpriteCenter(spr::icon48, 88 + myHeli->collectiveState, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2 - 8);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);

				drawSpriteCenter(spr::vehicleHUDParts, 16, barButton[1].x + barButton[1].w / 2, barButton[1].y + barButton[1].h / 2); //1턴 대기

				setZoom(1.5);
				//drawSpriteCenter(spr::cyclicLever, myHeli->cyclicState + 1, barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2 - 16); //사이클릭 레버
				setZoom(1);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
				drawSpriteCenter(spr::icon48, myHeli->cyclicState + 1 + 110, barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2 - 8);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);


				drawSpriteCenter(spr::vehicleHUDParts, 0 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트

				setZoom(1.5);
				//drawSpriteCenter(spr::rpmLever, myHeli->rpmState, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 + 9); //RPM 레버
				setZoom(1);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
				drawSpriteCenter(spr::icon48, myHeli->rpmState + 100, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 - 8);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);


				drawSpriteCenter(spr::tailPedalL, 0 + (checkCursor(&barButton[5]) && click), barButton[5].x + barButton[5].w / 2, barButton[5].y + barButton[5].h / 2); //좌회전 테일페달
				drawSpriteCenter(spr::tailPedalR, 0 + (checkCursor(&barButton[6]) && click), barButton[6].x + barButton[6].w / 2, barButton[6].y + barButton[6].h / 2); //우회전 테일페달
			}
			else if (typeHUD == vehFlag::train)
			{
				// std::vector<act> train = { act::rpmLever, act::wait, act::brake, act::startEngine, act::blank,act::blank,act::blank };

				Vehicle* myTrain = ((Vehicle*)ctrlVeh);

				setZoom(1);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
				drawSpriteCenter(spr::icon48, myTrain->rpmState + 100, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2 - 8);
				SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);

				drawSpriteCenter(spr::vehicleHUDParts, 16, barButton[1].x + barButton[1].w / 2, barButton[1].y + barButton[1].h / 2); //1턴 대기

				drawSpriteCenter(spr::icon48, 92 + (checkCursor(&barButton[2]) && click), barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2 - 8);

				drawSpriteCenter(spr::vehicleHUDParts, 0 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트

				int gearSprIndex = 0;
				if (((Vehicle*)ctrlVeh)->gearState == gearFlag::park) gearSprIndex = 0;
				else if (((Vehicle*)ctrlVeh)->gearState == gearFlag::reverse) gearSprIndex = 1;
				else if (((Vehicle*)ctrlVeh)->gearState == gearFlag::neutral) gearSprIndex = 2;
				else gearSprIndex = 3;
				drawSpriteCenter(spr::vehicleHUDParts, 5 + gearSprIndex, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 - 12);

				drawSpriteCenter(spr::tailPedalL, 0 + (checkCursor(&barButton[5]) && click), barButton[5].x + barButton[5].w / 2, barButton[5].y + barButton[5].h / 2); //좌회전 테일페달(미배정)
				drawSpriteCenter(spr::tailPedalR, 0 + (checkCursor(&barButton[6]) && click), barButton[6].x + barButton[6].w / 2, barButton[6].y + barButton[6].h / 2); //우회전 테일페달(미배정)
			}
		}

		//하단 레터박스 7버튼 그리기
		for (int i = 0; i < barAct.size(); i++)
		{
			if (ctrlVeh == nullptr)//차량 모드가 아닐 경우
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };

				if (checkCursor(&barButton[i]) || barActCursor == i)
				{
					if (click == false)
					{
						btnColor = lowCol::blue;
					}
					else
					{
						btnColor = lowCol::deepBlue;
					}
					outlineColor = { 0xa6, 0xa6, 0xa6 };
				}

				drawStadium(barButton[i].x, barButton[i].y, 72, 72, btnColor, 200, 5);
				SDL_Rect letterboxInButton = { barButton[i].x + 3,  barButton[i].y + 3, 72 - 6, 72 - 6 };
				SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, 255);
				SDL_RenderDrawRect(renderer, &letterboxInButton);
			}
			else if (i <= 2)//왼쪽부터 3개의 버튼의 경우(좌회전, 1턴대기, 우회전)
			{
				//버튼을 투명하게 그림
			}
			else if (i <= 7) //그 외의 4개의 버튼
			{
			}

			std::wstring actName = L" ";
			int actIndex = 0;
			bool deactRect = false; // true일 경우 회색으로 변함
			auto setBtnLayout = [&actName, &actIndex](std::wstring inputString, int inputActIndex)
				{
					actName = inputString;
					actIndex = inputActIndex;
				};

			if (barAct[i] == act::status) setBtnLayout(sysStr[3], 33);
			else if (barAct[i] == act::mutation) setBtnLayout(sysStr[54], 34);
			else if (barAct[i] == act::ability) setBtnLayout(sysStr[4], 4);
			else if (barAct[i] == act::inventory) setBtnLayout(sysStr[5], 50);
			else if (barAct[i] == act::bionic) setBtnLayout(sysStr[6], 47);
			else if (barAct[i] == act::talent) setBtnLayout(sysStr[7], 7);
			else if (barAct[i] == act::runMode)
			{
				if (Player::ins()->getSpriteInfimum() == sprInf::walk) setBtnLayout(sysStr[8], 9);
				else if (Player::ins()->getSpriteInfimum() == sprInf::run) setBtnLayout(sysStr[8], 10);
				else if (Player::ins()->getSpriteInfimum() == sprInf::sit) setBtnLayout(sysStr[8], 12);
				else if (Player::ins()->getSpriteInfimum() == sprInf::crawl) setBtnLayout(sysStr[8], 11);
			}
			else if (barAct[i] == act::identify) setBtnLayout(sysStr[135], 52);
			else if (barAct[i] == act::vehicle) setBtnLayout(sysStr[128], 48);
			else if (barAct[i] == act::alchemy) setBtnLayout(sysStr[134], 51);
			else if (barAct[i] == act::armor) setBtnLayout(sysStr[129], 16);
			else if (barAct[i] == act::cooking) setBtnLayout(sysStr[130], 49);
			else if (barAct[i] == act::menu) setBtnLayout(sysStr[9], 13);
			else if (barAct[i] == act::loot) setBtnLayout(sysStr[10], 0);
			else if (barAct[i] == act::pick) setBtnLayout(sysStr[11], 17);
			else if (barAct[i] == act::wield) setBtnLayout(sysStr[12], 15);
			else if (barAct[i] == act::equip) setBtnLayout(sysStr[13], 16);
			else if (barAct[i] == act::eat) setBtnLayout(sysStr[30], 25);
			else if (barAct[i] == act::pickSelect) setBtnLayout(sysStr[25], 26);
			else if (barAct[i] == act::selectAll) setBtnLayout(sysStr[26], 27);
			else if (barAct[i] == act::searching) setBtnLayout(sysStr[27], 28);
			else if (barAct[i] == act::select) setBtnLayout(sysStr[29], 30);
			else if (barAct[i] == act::droping) setBtnLayout(sysStr[52], 18);
			else if (barAct[i] == act::throwing) setBtnLayout(sysStr[53], 19);
			else if (barAct[i] == act::craft) setBtnLayout(sysStr[75], 21);
			else if (barAct[i] == act::construct) setBtnLayout(sysStr[73], 20);
			else if (barAct[i] == act::open) setBtnLayout(sysStr[88], 38);
			else if (barAct[i] == act::test) setBtnLayout(sysStr[92], 60);
			else if (barAct[i] == act::insert) setBtnLayout(sysStr[11], 17);
			else if (barAct[i] == act::reload) setBtnLayout(sysStr[100], 40);
			else if (barAct[i] == act::reloadBulletToMagazine) setBtnLayout(sysStr[113], 41);
			else if (barAct[i] == act::unloadBulletFromMagazine) setBtnLayout(sysStr[114], 44);
			else if (barAct[i] == act::reloadMagazine) setBtnLayout(sysStr[115], 40);
			else if (barAct[i] == act::unloadMagazine) setBtnLayout(sysStr[116], 43);
			else if (barAct[i] == act::reloadBulletToGun) setBtnLayout(sysStr[113], 45);
			else if (barAct[i] == act::unloadBulletFromGun) setBtnLayout(sysStr[114], 46);
			else if (barAct[i] == act::turnLeft) setBtnLayout(sysStr[141], 0);
			else if (barAct[i] == act::wait) setBtnLayout(sysStr[142], 0);
			else if (barAct[i] == act::turnRight) setBtnLayout(sysStr[143], 0);
			else if (barAct[i] == act::startEngine) setBtnLayout(sysStr[144], 0);
			else if (barAct[i] == act::stopEngine) setBtnLayout(sysStr[145], 0);
			else if (barAct[i] == act::shiftGear) setBtnLayout(sysStr[146], 0);
			else if (barAct[i] == act::accel) setBtnLayout(sysStr[147], 0);
			else if (barAct[i] == act::brake) setBtnLayout(sysStr[148], 0);
			else if (barAct[i] == act::god) setBtnLayout(sysStr[149], 61);
			else if (barAct[i] == act::map) setBtnLayout(sysStr[150], 73);
			else if (barAct[i] == act::collectiveLever) setBtnLayout(sysStr[151], 0);
			else if (barAct[i] == act::cyclicLever) setBtnLayout(sysStr[152], 0);
			else if (barAct[i] == act::rpmLever) setBtnLayout(sysStr[153], 0);
			else if (barAct[i] == act::tailRotorPedalL) setBtnLayout(sysStr[154], 0);
			else if (barAct[i] == act::tailRotorPedalR) setBtnLayout(sysStr[155], 0);
			else if (barAct[i] == act::closeDoor) setBtnLayout(sysStr[156], 91);
			else if (barAct[i] == act::skillActive)
			{
				errorBox(targetSkill == nullptr, L"HUD의 targetSkill이 nullptr이다.\n");
				setBtnLayout(sysStr[157], 140);

			}
			else if (barAct[i] == act::skillToggle)
			{
				errorBox(targetSkill == nullptr, L"HUD의 targetSkill이 nullptr이다.\n");
				if (targetSkill->toggle == false)
				{
					setBtnLayout(sysStr[158], 141);
				}
				else
				{
					setBtnLayout(sysStr[159], 142);
				}
			}
			else if (barAct[i] == act::quickSlot)
			{
				errorBox(targetSkill == nullptr, L"HUD의 targetSkill이 nullptr이다.\n");
				if (targetSkill->isQuickSlot == false)
				{
					setBtnLayout(sysStr[160], 146);
				}
				else
				{
					setBtnLayout(sysStr[161], 147);
				}
			}
			else setBtnLayout(L" ", 0);

			//48*48 심볼 아이콘 그리기
			drawSpriteCenter(spr::icon48, actIndex, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2) - 10);

			//하단 텍스트
			int fontSize = 13;
			setFontSize(fontSize);
			while (queryTextWidth(actName, true) > barButton[0].w)
			{
				fontSize--;
				setFontSize(fontSize);
			}
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			drawTextCenter(actName, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2) + 22);

			if (checkCursor(&barButton[i]) || barActCursor == i)
			{
				int markerIndex = 0;
				if (timer::timer600 % 30 < 5) { markerIndex = 0; }
				else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
				else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
				else if (timer::timer600 % 30 < 20) { markerIndex = 1; }
				else { markerIndex = 0; }
				if (ctrlVeh == nullptr)
				{
					drawSpriteCenter(spr::letterboxBtnMarker, markerIndex, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2));
				}
				else
				{
					drawSpriteCenter(spr::vehicleActCursor, markerIndex + 1, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2));
				}
			}
			else if (ctrlVeh != nullptr)
			{
				drawSpriteCenter(spr::vehicleActCursor, 0, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2));
			}

			if (deactRect == true) drawStadium(barButton[i].x, barButton[i].y, 72, 72, { 0,0,0 }, 120, 5);
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

	bool runAnimation(bool shutdown) override
	{
		const int acc = 20;
		const int initSpeed = 4;
		if (getAniType() == aniFlag::popUpLetterbox)
		{
			//initDist가 정해져있어야 한다.
			//역으로 initDist에서 가속을 구할 수 있어야 함
			static std::array<float, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static float initDist = 0;
			float dstDist = popUpDist;
			if (inputType == input::keyboard) { dstDist += noActHeightHUD; }

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
				if (inputType == input::keyboard) { changeXY(0, -dstDist + noActHeightHUD, false); }
				else { changeXY(0, -dstDist, false); }
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
			if (inputType == input::keyboard) { dstDist += noActHeightHUD; }

			if (inputType == input::keyboard) { dstDist = -y + noActHeightHUD; }
			else { dstDist = -y; }

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
				if (inputType == input::keyboard) { changeXY(0, noActHeightHUD, false); }
				else { changeXY(0, 0, false); }
				resetTimer();
				actInput();
				setAniType(aniFlag::null);
				return true;
				break;
			}
		}
		else if (getAniType() == aniFlag::popUpSingleLetterbox)//키보드에서만 발생하는 1칸 팝업
		{
			//initDist가 정해져있어야 한다.
			//역으로 initDist에서 가속을 구할 수 있어야 함
			static std::array<float, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static float initDist = 0;
			float dstDist = noActHeightHUD;

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
						coordList.push_back({ tgtX,tgtY });

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
};