import HUD;

#include <SDL3/SDL.h>

import std;
import util;
import globalVar;
import wrapVar;
import constVar;
import textureVar;
import Player;
import Sprite;
import World;
import drawSprite;
import drawText;
import globalTime;
import checkCursor;
import Vehicle;
import drawWindow;
import drawEpsilonText;
import ContextMenu;
import Maint;


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

void HUD::drawGUI()
{
	if (!drawHUD) return;

	const bool* state = SDL_GetKeyboardState(nullptr);
	Sprite* targetBtnSpr = nullptr;

	drawStatusEffects();

	drawStadium(letterbox.x, letterbox.y, letterbox.w, letterbox.h + 10, { 0,0,0 }, 150, 5);
	if (ctrlVeh != nullptr)
	{
		drawSpriteCenter(spr::vehicleHUD, 0, cameraW / 2, cameraH + 73 + y);
		if (ctrlVeh->isEngineOn)
		{
			drawSpriteCenter(spr::dashboard, 0, cameraW / 2, cameraH + 73 + y);
			setZoom(2.0);
			drawTextureCenter(texture::navimap, cameraW / 2, cameraH - 132 + y);
			setZoom(1.0);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (isAdvancedMode == false)
	{

		if (ctrlVeh == nullptr)
		{
			SDL_SetTextureBlendMode(texture::minimap, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(texture::minimap, 160);
			setZoom(4.0);
			drawTextureCenter(texture::minimap, 94, 94);
			setZoom(1.0);


			drawSprite(spr::minimapEdge, 1, 14, 14);
			Sprite* targetBtnSpr;
			if (option::inputMethod == input::gamepad)
			{
				if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_L1, 14 + 15, 14 + 15);
			}
			else if (option::inputMethod == input::mouse)
			{
				targetBtnSpr = spr::buttons;
				drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_M, 14 + 15, 14 + 15);
			}
		}


		int vShift = 396 * (ctrlVeh != nullptr);

		if (1)//플레이어 이름 그리기
		{
			setFontSize(10);
			renderText(L"Jackson, Practitioner of Elivilon ******", letterbox.x + 29 + vShift, letterbox.y + 5, lowCol::yellow);
		}

		setFontSize(10);


		{
			// 왼팔
			{
				//페이크 HP
				if (PlayerPtr->lArmFakeHP > PlayerPtr->lArmHP) { PlayerPtr->lArmFakeHP--; }
				else if (PlayerPtr->lArmFakeHP < PlayerPtr->lArmHP) PlayerPtr->lArmFakeHP = PlayerPtr->lArmHP;
				if (PlayerPtr->lArmFakeHP != PlayerPtr->lArmHP)
				{
					if (PlayerPtr->lArmFakeHPAlpha > 30) { PlayerPtr->lArmFakeHPAlpha -= 30; }
					else { PlayerPtr->lArmFakeHPAlpha = 0; }
				}
				else { PlayerPtr->lArmFakeHPAlpha = 255; }

				int pivotX = letterbox.x + 36 + vShift;
				int pivotY = letterbox.y + 22;
				renderTextCenter(L"왼팔", pivotX - 16, pivotY + 5, col::lightGray);
				drawSprite(spr::hpBlankGauge, pivotX, pivotY);



				// 페이크 HP
				float ratioFakeHP = myMax((float)0.0, (PlayerPtr->lArmFakeHP) / (float)(PART_MAX_HP));
				SDL_Rect fakeRect = { pivotX + 3, pivotY + 3, 38 * ratioFakeHP, 5 };
				drawFillRect(fakeRect, lowCol::white, PlayerPtr->lArmFakeHPAlpha);

				// 실제 HP
				float ratioHP = myMax((float)0.0, (float)(PlayerPtr->lArmHP) / (float)(PART_MAX_HP));
				SDL_Rect realRect = { pivotX + 3, pivotY + 3, 38 * ratioHP, 5 };
				if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
				drawFillRect(realRect, lowCol::green);
			}

			// 좌다리
			{
				//페이크 HP
				if (PlayerPtr->lLegFakeHP > PlayerPtr->lLegHP) { PlayerPtr->lLegFakeHP--; }
				else if (PlayerPtr->lLegFakeHP < PlayerPtr->lLegHP) PlayerPtr->lLegFakeHP = PlayerPtr->lLegHP;
				if (PlayerPtr->lLegFakeHP != PlayerPtr->lLegHP)
				{
					if (PlayerPtr->lLegFakeHPAlpha > 30) { PlayerPtr->lLegFakeHPAlpha -= 30; }
					else { PlayerPtr->lLegFakeHPAlpha = 0; }
				}
				else { PlayerPtr->lLegFakeHPAlpha = 255; }

				int pivotX = letterbox.x + 36 + vShift;
				int pivotY = letterbox.y + 22 + 15;
				renderTextCenter(L"좌다리", pivotX - 16, pivotY + 5, col::lightGray);
				drawSprite(spr::hpBlankGauge, pivotX, pivotY);

				// 페이크 HP
				float ratioFakeHP = myMax((float)0.0, (PlayerPtr->lLegFakeHP) / (float)(PART_MAX_HP));
				SDL_Rect fakeRect = { pivotX + 3, pivotY + 3, 38 * ratioFakeHP, 5 };
				drawFillRect(fakeRect, lowCol::white, PlayerPtr->lLegFakeHPAlpha);

				// 실제 HP
				float ratioHP = myMax((float)0.0, (float)(PlayerPtr->lLegHP) / (float)(PART_MAX_HP));
				SDL_Rect realRect = { pivotX + 3, pivotY + 3, 38 * ratioHP, 5 };
				if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
				drawFillRect(realRect, lowCol::green);
			}

			// 머리
			{
				//페이크 HP
				if (PlayerPtr->headFakeHP > PlayerPtr->headHP) { PlayerPtr->headFakeHP--; }
				else if (PlayerPtr->headFakeHP < PlayerPtr->headHP) PlayerPtr->headFakeHP = PlayerPtr->headHP;
				if (PlayerPtr->headFakeHP != PlayerPtr->headHP)
				{
					if (PlayerPtr->headFakeHPAlpha > 30) { PlayerPtr->headFakeHPAlpha -= 30; }
					else { PlayerPtr->headFakeHPAlpha = 0; }
				}
				else { PlayerPtr->headFakeHPAlpha = 255; }

				int pivotX = letterbox.x + 36 + 83 + vShift;
				int pivotY = letterbox.y + 22;
				renderTextCenter(L"머리", pivotX - 16, pivotY + 5, col::lightGray);
				drawSprite(spr::hpBlankGauge, pivotX, pivotY);

				// 페이크 HP
				float ratioFakeHP = myMax((float)0.0, (PlayerPtr->headFakeHP) / (float)(PART_MAX_HP));
				SDL_Rect fakeRect = { pivotX + 3, pivotY + 3, 38 * ratioFakeHP, 5 };
				drawFillRect(fakeRect, lowCol::white, PlayerPtr->headFakeHPAlpha);

				// 실제 HP
				float ratioHP = myMax((float)0.0, (float)(PlayerPtr->headHP) / (float)(PART_MAX_HP));
				SDL_Rect realRect = { pivotX + 3, pivotY + 3, 38 * ratioHP, 5 };
				if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
				drawFillRect(realRect, lowCol::green);
			}

			// 몸통 (기존 코드 유지)
			{
				int pivotX = letterbox.x + 36 + 83 + vShift;
				int pivotY = letterbox.y + 22 + 15;
				renderTextCenter(L"몸통", pivotX - 16, pivotY + 5, col::lightGray);
				drawSprite(spr::hpBlankGauge, pivotX, pivotY);

				// 페이크 HP
				float ratioFakeHP = myMax((float)0.0, (PlayerPtr->entityInfo.fakeHP) / (float)(PlayerPtr->entityInfo.maxHP));
				SDL_Rect fakeRect = { pivotX + 3, pivotY + 3, 38 * ratioFakeHP, 5 };
				drawFillRect(fakeRect, lowCol::white, PlayerPtr->entityInfo.fakeHPAlpha);

				// 실제 HP
				float ratioHP = myMax((float)0.0, (float)(PlayerPtr->entityInfo.HP) / (float)(PlayerPtr->entityInfo.maxHP));
				SDL_Rect realRect = { pivotX + 3, pivotY + 3, 38 * ratioHP, 5 };
				if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
				drawFillRect(realRect, lowCol::green);
			}

			// 오른팔
			{
				//페이크 HP
				if (PlayerPtr->rArmFakeHP > PlayerPtr->rArmHP) { PlayerPtr->rArmFakeHP--; }
				else if (PlayerPtr->rArmFakeHP < PlayerPtr->rArmHP) PlayerPtr->rArmFakeHP = PlayerPtr->rArmHP;
				if (PlayerPtr->rArmFakeHP != PlayerPtr->rArmHP)
				{
					if (PlayerPtr->rArmFakeHPAlpha > 30) { PlayerPtr->rArmFakeHPAlpha -= 30; }
					else { PlayerPtr->rArmFakeHPAlpha = 0; }
				}
				else { PlayerPtr->rArmFakeHPAlpha = 255; }

				int pivotX = letterbox.x + 36 + 83 * 2 + vShift;
				int pivotY = letterbox.y + 22;
				renderTextCenter(L"오른팔", pivotX - 16, pivotY + 5, col::lightGray);
				drawSprite(spr::hpBlankGauge, pivotX, pivotY);

				// 페이크 HP
				float ratioFakeHP = myMax((float)0.0, (PlayerPtr->rArmFakeHP) / (float)(PART_MAX_HP));
				SDL_Rect fakeRect = { pivotX + 3, pivotY + 3, 38 * ratioFakeHP, 5 };
				drawFillRect(fakeRect, lowCol::white, PlayerPtr->rArmFakeHPAlpha);

				// 실제 HP
				float ratioHP = myMax((float)0.0, (float)(PlayerPtr->rArmHP) / (float)(PART_MAX_HP));
				SDL_Rect realRect = { pivotX + 3, pivotY + 3, 38 * ratioHP, 5 };
				if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
				drawFillRect(realRect, lowCol::green);
			}

			// 우다리
			{
				//페이크 HP
				if (PlayerPtr->rLegFakeHP > PlayerPtr->rLegHP) { PlayerPtr->rLegFakeHP--; }
				else if (PlayerPtr->rLegFakeHP < PlayerPtr->rLegHP) PlayerPtr->rLegFakeHP = PlayerPtr->rLegHP;
				if (PlayerPtr->rLegFakeHP != PlayerPtr->rLegHP)
				{
					if (PlayerPtr->rLegFakeHPAlpha > 30) { PlayerPtr->rLegFakeHPAlpha -= 30; }
					else { PlayerPtr->rLegFakeHPAlpha = 0; }
				}
				else { PlayerPtr->rLegFakeHPAlpha = 255; }

				int pivotX = letterbox.x + 36 + 83 * 2 + vShift;
				int pivotY = letterbox.y + 22 + 15;
				renderTextCenter(L"우다리", pivotX - 16, pivotY + 5, col::lightGray);
				drawSprite(spr::hpBlankGauge, pivotX, pivotY);

				// 페이크 HP
				float ratioFakeHP = myMax((float)0.0, (PlayerPtr->rLegFakeHP) / (float)(PART_MAX_HP));
				SDL_Rect fakeRect = { pivotX + 3, pivotY + 3, 38 * ratioFakeHP, 5 };
				drawFillRect(fakeRect, lowCol::white, PlayerPtr->rLegFakeHPAlpha);

				// 실제 HP
				float ratioHP = myMax((float)0.0, (float)(PlayerPtr->rLegHP) / (float)(PART_MAX_HP));
				SDL_Rect realRect = { pivotX + 3, pivotY + 3, 38 * ratioHP, 5 };
				if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
				drawFillRect(realRect, lowCol::green);
			}
		}



		if (ctrlVeh == nullptr)
		{
			//스테미나
			{
				int pivotX = letterbox.x + 18 + 238;
				int pivotY = letterbox.y + 4;
				setFontSize(10);

				int pSTA = PlayerPtr->entityInfo.STA;
				int pSTAMax = PlayerPtr->entityInfo.maxSTA;


				if (fakeSTA > pSTA)
				{
					fakeSTA -= 1;
					if (alphaSTA > 10) alphaSTA -= 10;
					else alphaSTA = 0;
				}
				else if (fakeSTA < pSTA) fakeSTA++;
				else alphaSTA = 255;


				SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), 0, 0, 0);
				SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
				drawSprite(spr::staminaGaugeCircle, 0, pivotX, pivotY);

				if (fakeSTA > pSTA)
				{

					int sprIndexFake = (int)(90.0 - 90.0 * ((double)fakeSTA / (double)pSTAMax));
					SDL_SetTextureAlphaMod(spr::staminaGaugeCircle->getTexture(), alphaSTA);
					SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), lowCol::red.r, lowCol::red.g, lowCol::red.b);
					SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
					drawSprite(spr::staminaGaugeCircle, sprIndexFake, pivotX, pivotY);
					SDL_SetTextureAlphaMod(spr::staminaGaugeCircle->getTexture(), 255);


					int sprIndex = (int)(90.0 - 90.0 * ((double)pSTA / (double)pSTAMax));
					SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), lowCol::yellow.r, lowCol::yellow.g, lowCol::yellow.b);
					SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
					drawSprite(spr::staminaGaugeCircle, sprIndex, pivotX, pivotY);
				}
				else
				{
					int sprIndex = (int)(90.0 - 90.0 * ((double)fakeSTA / (double)pSTAMax));
					SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), lowCol::yellow.r, lowCol::yellow.g, lowCol::yellow.b);
					SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
					drawSprite(spr::staminaGaugeCircle, sprIndex, pivotX, pivotY);
				}


				setFontSize(10);
				renderTextCenter(L"STA", pivotX + 24, pivotY + 16);




				std::wstring STAStr = std::to_wstring(PlayerPtr->entityInfo.STA) + L"/" + std::to_wstring(PlayerPtr->entityInfo.maxSTA);
				setFontSize(8);
                renderTextOutlineCenter(STAStr, pivotX + 24, pivotY + 29);	
			}

			setFontSize(10);
			
			drawSpriteCenter(spr::icon13, 25, letterbox.x + 18 + 296 + 5, letterbox.y + 5 + 15 * 0 + 6);
			renderText(L"6,320", letterbox.x + 18 + 296 + 17, letterbox.y + 5 + 15 * 0, lowCol::yellow);

			renderText(L"SPEED", letterbox.x + 18 + 296, letterbox.y + 5 + 15 * 1, col::lightGray);
			renderText(L"120%", letterbox.x + 18 + 44 + 296, letterbox.y + 5 + 15 * 1, lowCol::green);
			renderText(L"STRESS", letterbox.x + 18 + 296, letterbox.y + 5 + 15 * 2, col::lightGray);
			renderText(L"79", letterbox.x + 18 + 44 + 296, letterbox.y + 5 + 15 * 2, lowCol::red);


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
			int pz = PlayerZ();
			World::ins()->changeToChunkCoord(PlayerX(), PlayerY(), cx, cy);
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

			renderTextCenter(L"15℃", letterbox.x + 18 + 374 + 36, letterbox.y + 16 + 24);

			drawSprite(spr::batteryGauge, 4, letterbox.x + 18 + 562, letterbox.y + 3);
			setFontSize(10);

			renderTextOutlineCenter(L"72%", letterbox.x + 18 + 562 + 16, letterbox.y + 3 + 24);
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

			if (option::inputMethod == input::gamepad)
			{
				if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_NORTH)) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_TRI, letterboxPopUpButton.x + 15, letterboxPopUpButton.y + 15);
			}
			else
			{
				if (y == 0) drawSpriteCenter(spr::windowArrow, 1, letterboxPopUpButton.x + 15, letterboxPopUpButton.y + 15);
				else drawSpriteCenter(spr::windowArrow, 3, letterboxPopUpButton.x + 15, letterboxPopUpButton.y + 15);
			}
		}

	}
	else
	{

	}


	if (getLastGUI() == ContextMenu::ins() || getLastGUI() == Maint::ins())
	{
		int tgtX = contextMenuTargetGrid.x;
		int tgtY = contextMenuTargetGrid.y;
		SDL_Rect dst;
		int tileSize = 16 * zoomScale;
		dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
		dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
		dst.w = tileSize;
		dst.h = tileSize;
		setZoom(zoomScale);
		Uint32 currentTime = SDL_GetTicks();
		const Uint32 pulsationPeriodMs = 1500;
		const Uint8 minAlpha = 100;
		const Uint8 maxAlpha = 255;
		double timeRatio = fmod((double)currentTime / pulsationPeriodMs, 1.0);
		double normalizedAlpha = (sin(timeRatio * 2.0 * 3.141592) + 1.0) / 2.0;
		Uint8 alpha = static_cast<Uint8>(minAlpha + normalizedAlpha * (maxAlpha - minAlpha));
		SDL_GetTicks();
		SDL_SetTextureAlphaMod(spr::gridMarker->getTexture(), alpha); //텍스쳐 투명도 설정
		SDL_SetTextureBlendMode(spr::gridMarker->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
		drawSpriteCenter
		(
			spr::gridMarker,
			0,
			dst.x + dst.w / 2,
			dst.y + dst.h / 2
		);
		SDL_SetTextureAlphaMod(spr::gridMarker->getTexture(), 255);
		setZoom(1.0);
	}


	drawBarAct();
	drawTab();
	drawQuickSlot();
	drawQuest();
	//drawHoverItemInfo();

}


void HUD::drawTab()
{
	SDL_Color btnColor = { 0x00, 0x00, 0x00 };
	if (checkCursor(&tab))
	{
		if (click == false) { btnColor = lowCol::blue; }
		else { btnColor = lowCol::deepBlue; }
	}

	switch (tabType)
	{
	case tabFlag::autoAtk:
	{
		if (option::inputMethod == input::gamepad)
		{
			if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) drawSprite(spr::tab, 4, tab.x, tab.y - 2);
			else if (SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000) drawSprite(spr::tab, 2, tab.x, tab.y - 2);
			else drawSprite(spr::tab, 0, tab.x, tab.y - 2);
		}
		else
		{
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
			else drawSprite(spr::tab, 0, tab.x, tab.y - 2);
		}

		setFontSize(12);
		renderTextCenter(sysStr[1], tab.x + 60, tab.y + 94);
		renderTextCenter(sysStr[2], tab.x + 60, tab.y + 94 + 14);


		drawSpriteCenter(spr::icon48, 1, tab.x + 42, tab.y + 55);
		drawSpriteCenter(spr::icon48, 2, tab.x + 72, tab.y + 64);

		drawSpriteCenter(spr::icon48, 158, tab.x + 99, tab.y + 18);

		Sprite* targetBtnSpr;
		if (option::inputMethod == input::gamepad)
		{
			if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
			else { targetBtnSpr = spr::buttons; }
			drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_R1, tab.x + 117, tab.y - 4);
		}
		break;
	}
	case tabFlag::aim:
	{
		if (option::inputMethod == input::gamepad)
		{
			if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) drawSprite(spr::tab, 4, tab.x, tab.y - 2);
			else if(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000) drawSprite(spr::tab, 2, tab.x, tab.y - 2);
			else drawSprite(spr::tab, 0, tab.x, tab.y - 2);
		}
		else
		{
			if (checkCursor(&tabSmallBox))
			{
				if (click == false) { drawSprite(spr::tabBoxAim, 5, tab.x, tab.y - 2); }
				else { drawSprite(spr::tabBoxAim, 6, tab.x, tab.y - 2); }
			}
			else if (checkCursor(&tab))
			{
				drawLine(tab.x + 0, tab.y + 119, tab.x + 121, tab.y - 2, lowCol::red, 200);
				if ((getMouseX() - tab.x + getMouseY() - tab.y - 119) < 0)
				{
					if (click == false) { drawSprite(spr::tabBoxAim, 1, tab.x, tab.y - 2); }
					else { drawSprite(spr::tabBoxAim, 2, tab.x, tab.y - 2); }
				}
				else // 마우스 커서가 직선 위에 있거나(경계 포함) 아래에 있는 경우
				{
					if (click == false) { drawSprite(spr::tabBoxAim, 3, tab.x, tab.y - 2); }
					else { drawSprite(spr::tabBoxAim, 4, tab.x, tab.y - 2); }
				}
			}
			else drawSprite(spr::tabBoxAim, 0, tab.x, tab.y - 2);
		}

        



		setFontSize(12);
		renderTextCenter(sysStr[195], tab.x + 62, tab.y + 9);
		renderTextCenter(sysStr[207], tab.x + 104, tab.y + 52);
		//renderTextCenter(sysStr[195], tab.x + 60, tab.y + 92 + 7);



		setZoom(1.5);
		drawSpriteCenter(spr::icon48, 169, tab.x + 38, tab.y + 52);
		setZoom(1.0);
		drawSpriteCenter(spr::icon48, 170, tab.x + 80, tab.y + 85);

		drawSpriteCenter(spr::icon48, 158, tab.x + 100, tab.y + 21);



		Sprite* targetBtnSpr;
		if (option::inputMethod == input::gamepad)
		{
			if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
			else { targetBtnSpr = spr::buttons; }
			drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_R1, tab.x + 117, tab.y - 4);
		}
		break;
	}
	case tabFlag::closeWin:
		//창 닫기
		drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
		drawSpriteCenter(spr::icon48, 14, tab.x + 60, tab.y + 52);
		setFontSize(12);
		renderTextCenter(sysStr[14], tab.x + 60, tab.y + 92 + 7);
		break;
	case tabFlag::back:
		//뒤로가기
		drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
		drawSpriteCenter(spr::icon48, 31, tab.x + 60, tab.y + 52);
		setFontSize(12);
		renderTextCenter(sysStr[31], tab.x + 60, tab.y + 92 + 7);
		break;
	case tabFlag::confirm:
		//뒤로가기
		drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
		drawSpriteCenter(spr::icon48, 39, tab.x + 60, tab.y + 52);
		setFontSize(12);
		renderTextCenter(sysStr[91], tab.x + 60, tab.y + 92 + 7);
		break;
	default:
		errorBox(L"undefined tabFalg");
		break;
	}

	Sprite* targetBtnSpr;
	if (option::inputMethod == input::gamepad)
	{
		if (SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0) { targetBtnSpr = spr::buttonsPressed; }
		else { targetBtnSpr = spr::buttons; }
		drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_R2, tab.x + 4, tab.y + 4);
	}
}

void HUD::drawQuickSlot()
{
	int pivotX = quickSlotRegion.x;
	int pivotY = quickSlotRegion.y;

	//drawRect({ pivotX,pivotY,180,358, }, lowCol::skyBlue);


	//개별 스킬창 그리기 1번부터 8번까지
	for (int i = 0; i < 8; i++)
	{
		int pivotX = 185 + 48 * i;
		int pivotY = 0;

		SDL_Color btnCol = col::black;
		if (checkCursor(&quickSlotBtn[i]) && quickSlot[i].first != quickSlotFlag::NONE && getLastGUI() == this)
		{
			if (click) btnCol = lowCol::deepBlue;
			else btnCol = lowCol::blue;

			setFontSize(10);
			std::wstring skillName = L"";
			skillName = skillDex[quickSlot[i].second].name;

			renderTextOutlineCenter(skillName, 374, 57);
		}

		drawFillRect(SDL_Rect{ pivotX,pivotY,44,39 }, btnCol, 200);
		drawLine(pivotX + 1, pivotY + 39, pivotX + 42, pivotY + 39, btnCol, 200);
		drawLine(pivotX + 2, pivotY + 39 + 1, pivotX + 41, pivotY + 39 + 1, btnCol, 200);
		drawLine(pivotX + 3, pivotY + 39 + 2, pivotX + 40, pivotY + 39 + 2, btnCol, 200);
		drawLine(pivotX + 4, pivotY + 39 + 3, pivotX + 39, pivotY + 39 + 3, btnCol, 200);
		drawLine(pivotX + 5, pivotY + 39 + 4, pivotX + 38, pivotY + 39 + 4, btnCol, 200);
		drawLine(pivotX + 6, pivotY + 39 + 5, pivotX + 37, pivotY + 39 + 5, btnCol, 200);


		if (quickSlot[i].first == quickSlotFlag::NONE)
		{
			drawCross2(pivotX + 5, pivotY, 0, 5, 0, 5);
			drawCross2(pivotX + 5 + 32, pivotY, 0, 5, 5, 0);
			drawCross2(pivotX + 5, pivotY + 32, 5, 0, 0, 5);
			drawCross2(pivotX + 5 + 32, pivotY + 32, 5, 0, 5, 0);
			drawFillRect(SDL_Rect{ pivotX + 5, pivotY ,34,34 }, col::black, 180);
		}
		else
		{
			setZoom(2.0);
			std::wstring skillName = L"";
			drawSprite(spr::skillSet, skillDex[quickSlot[i].second].iconIndex, pivotX + 6, pivotY + 1);
			skillName = skillDex[quickSlot[i].second].name;
			setZoom(1.0);

			drawCross2(pivotX + 5, pivotY, 0, 5, 0, 5);
			drawCross2(pivotX + 5 + 32, pivotY, 0, 5, 5, 0);
			drawCross2(pivotX + 5, pivotY + 32, 5, 0, 0, 5);
			drawCross2(pivotX + 5 + 32, pivotY + 32, 5, 0, 5, 0);
		}
		setFontSize(10);
		renderText(std::to_wstring(i + 1), pivotX + 20, pivotY + 33);


	}

	if (dragQuickSlotTarget != -1)
	{
		if (quickSlot[dragQuickSlotTarget].first == quickSlotFlag::SKILL)
		{
			setZoom(2.0);
			SDL_SetTextureAlphaMod(spr::skillSet->getTexture(), 180); //텍스쳐 투명도 설정
			SDL_SetTextureBlendMode(spr::skillSet->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
			drawSpriteCenter(spr::skillSet, skillDex[quickSlot[dragQuickSlotTarget].second].iconIndex, getMouseX(), getMouseY());
			SDL_SetTextureAlphaMod(spr::skillSet->getTexture(), 255); //텍스쳐 투명도 설정
			setZoom(1.0);
		}
	}
}

void HUD::drawBarAct()
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


			if(ctrlVeh->isEngineOn) drawSpriteCenter(spr::vehicleHUDParts, 2 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트
			else  drawSpriteCenter(spr::vehicleHUDParts, 0 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트
			

			//기어(PRND) 그리기
			int gearSprIndex = 0;
			if (ctrlVeh->gearState == gearFlag::park) gearSprIndex = 0;
			else if (ctrlVeh->gearState == gearFlag::reverse) gearSprIndex = 1;
			else if (ctrlVeh->gearState == gearFlag::neutral) gearSprIndex = 2;
			else gearSprIndex = 3;
			drawSpriteCenter(spr::vehicleHUDParts, 5 + gearSprIndex, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 - 12);

			drawSpriteCenter(spr::vehicleHUDParts, 10 + (checkCursor(&barButton[5]) && click), barButton[5].x + barButton[5].w / 2, barButton[5].y + barButton[5].h / 2);
			drawSpriteCenter(spr::vehicleHUDParts, 12 + (checkCursor(&barButton[6]) && click), barButton[6].x + barButton[6].w / 2, barButton[6].y + barButton[6].h / 2);
		}
		else if (typeHUD == vehFlag::heli)
		{

			setZoom(1.5);
			//drawSpriteCenter(spr::collectiveLever, myHeli->collectiveState + 1, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2 - 16); //콜렉티브 레버
			setZoom(1);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
			drawSpriteCenter(spr::icon48, 88 + ctrlVeh->collectiveState, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2 - 8);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);

			drawSpriteCenter(spr::vehicleHUDParts, 16, barButton[1].x + barButton[1].w / 2, barButton[1].y + barButton[1].h / 2); //1턴 대기

			setZoom(1.5);
			//drawSpriteCenter(spr::cyclicLever, myHeli->cyclicState + 1, barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2 - 16); //사이클릭 레버
			setZoom(1);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
			drawSpriteCenter(spr::icon48, ctrlVeh->cyclicState + 1 + 110, barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2 - 8);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);


			drawSpriteCenter(spr::vehicleHUDParts, 0 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트

			setZoom(1.5);
			//drawSpriteCenter(spr::rpmLever, myHeli->rpmState, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 + 9); //RPM 레버
			setZoom(1);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
			drawSpriteCenter(spr::icon48, ctrlVeh->rpmState + 100, barButton[4].x + barButton[4].w / 2, barButton[4].y + barButton[4].h / 2 - 8);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);


			drawSpriteCenter(spr::tailPedalL, 0 + (checkCursor(&barButton[5]) && click), barButton[5].x + barButton[5].w / 2, barButton[5].y + barButton[5].h / 2); //좌회전 테일페달
			drawSpriteCenter(spr::tailPedalR, 0 + (checkCursor(&barButton[6]) && click), barButton[6].x + barButton[6].w / 2, barButton[6].y + barButton[6].h / 2); //우회전 테일페달
		}
		else if (typeHUD == vehFlag::minecart)
		{
			setZoom(1);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 160);
			drawSpriteCenter(spr::icon48, ctrlVeh->rpmState + 100, barButton[0].x + barButton[0].w / 2, barButton[0].y + barButton[0].h / 2 - 8);
			SDL_SetTextureAlphaMod(spr::icon48->getTexture(), 255);

			drawSpriteCenter(spr::vehicleHUDParts, 16, barButton[1].x + barButton[1].w / 2, barButton[1].y + barButton[1].h / 2); //1턴 대기

			drawSpriteCenter(spr::icon48, 92 + (checkCursor(&barButton[2]) && click), barButton[2].x + barButton[2].w / 2, barButton[2].y + barButton[2].h / 2 - 8);

			drawSpriteCenter(spr::vehicleHUDParts, 0 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트

			int gearSprIndex = 0;
			if (ctrlVeh->gearState == gearFlag::park) gearSprIndex = 0;
			else if (ctrlVeh->gearState == gearFlag::reverse) gearSprIndex = 1;
			else if (ctrlVeh->gearState == gearFlag::neutral) gearSprIndex = 2;
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
			drawRect(letterboxInButton, outlineColor);
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
		else if (barAct[i] == act::inventory) setBtnLayout(sysStr[5], 79);
		else if (barAct[i] == act::bionic) setBtnLayout(sysStr[6], 47);
		else if (barAct[i] == act::profic) setBtnLayout(sysStr[7], 7);
		else if (barAct[i] == act::skill) setBtnLayout(sysStr[197], 4);
		else if (barAct[i] == act::quest) setBtnLayout(sysStr[198], 52);
		else if (barAct[i] == act::runMode)
		{

			if (PlayerPtr->entityInfo.walkMode == walkFlag::walk) setBtnLayout(sysStr[8], 9);
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::run) setBtnLayout(sysStr[8], 10);
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::crouch) setBtnLayout(sysStr[8], 12);
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::crawl) setBtnLayout(sysStr[8], 11);
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::wade)
			{
				setBtnLayout(sysStr[8], 161);
				deactRect = true;
			}
			else if (PlayerPtr->entityInfo.walkMode == walkFlag::swim)
			{
				setBtnLayout(sysStr[8], 160);
				deactRect = true;
			}
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
		else if (barAct[i] == act::phone) setBtnLayout(sysStr[189], 150);
		else if (barAct[i] == act::message) setBtnLayout(sysStr[190], 151);
		else if (barAct[i] == act::camera) setBtnLayout(sysStr[191], 152);
		else if (barAct[i] == act::internet) setBtnLayout(sysStr[192], 154);
		else if (barAct[i] == act::settings) setBtnLayout(sysStr[193], 155);
		else if (barAct[i] == act::saveAndQuit) setBtnLayout(sysStr[194], 156);
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
		else if (barAct[i] == act::toggleOff) setBtnLayout(sysStr[196], 162);
		else if (barAct[i] == act::toggleOn) setBtnLayout(sysStr[196], 163);
		else if (barAct[i] == act::headlight)
		{
			if(ctrlVeh->headlightOn == false) setBtnLayout(sysStr[205], 165);
            else setBtnLayout(sysStr[205], 164);
		}
		else if (barAct[i] == act::drink) setBtnLayout(sysStr[210], 37);
		else setBtnLayout(L" ", 0);

		//48*48 심볼 아이콘 그리기
		drawSpriteCenter(spr::icon48, actIndex, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2) - 10);

		//하단 텍스트
		int fontSize = 12;
		setFontSize(fontSize);
		renderTextCenter(actName, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2) + 23);

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

void HUD::drawStatusEffects()
{
	std::vector<std::pair<statEfctFlag, int>>& myEfcts = PlayerPtr->entityInfo.statusEffects;
	for (int i = 0; i < myEfcts.size(); i++)
	{
		int pivotX = 5;
		int pivotY = 250 + 20*i;
		std::wstring statEfctName = L"";
		int statEfctIcon = 0;
        SDL_Color textColor = col::white;

		switch (myEfcts[i].first)
		{
		case statEfctFlag::confusion:
			statEfctName = L"혼란";
			statEfctIcon = 1;
			break;
		case statEfctFlag::bleeding:
			statEfctName = L"출혈";
			statEfctIcon = 2;
			break;
		case statEfctFlag::hungry:
			if (hunger < 14400)
			{
				statEfctName = L"영양실조";
				textColor = lowCol::red;
			}
			else if (hunger < 14400 * 2)
			{
				statEfctName = L"매우 배고픔";
				textColor = lowCol::orange;
			}
			else if (hunger < 14400 * 3)
			{
				statEfctName = L"배고픔";
				textColor = lowCol::yellow;
			}
			statEfctIcon = 3;
			break;
		case statEfctFlag::dehydration:
			if(thirst < 7200)
			{
				statEfctName = L"탈수증";
				textColor = lowCol::red;
			}
			else if (thirst < 7200 * 2)
			{
				statEfctName = L"매우 목마름";
				textColor = lowCol::orange;
			}
			else if (thirst < 7200 * 3)
			{
				statEfctName = L"목마름";
				textColor = lowCol::yellow;
            }	

			statEfctIcon = 4;
			break;
		case statEfctFlag::blindness:
			statEfctName = L"실명";
			statEfctIcon = 15;
			break;
		}

		setFontSize(10);

		setZoom(1.0);
		drawSprite(spr::statusIcon, statEfctIcon, pivotX, pivotY);
		setZoom(1.0);

		
		int textWidth = queryTextWidth(statEfctName)+15;

		drawFillRect(SDL_Rect{ pivotX + 16, pivotY, textWidth, 16 }, col::black, 85);
		int lineStartX = pivotX + textWidth + 16;
		for (int i = 0; i < 8; i++)
		{
			drawLine(lineStartX + i, pivotY + 1 + i, lineStartX + i, pivotY + 15, col::black, 85);
		}


		renderTextOutline(statEfctName, pivotX + 19, pivotY + 1, lowCol::red);

		if (myEfcts[i].second > 0)
		{
			//setZoom(0.7);
			//int seg1 = myEfcts[i].second / 100;
			//int seg2 = (myEfcts[i].second % 100) / 10;
			//int seg3 = myEfcts[i].second % 10;

			int xCorrection = 0;
			if (myEfcts[i].second > 999) xCorrection = -8;
			else if (myEfcts[i].second > 99) xCorrection = -4;
			else if (myEfcts[i].second < 10) xCorrection = +4;


			drawEplsionText(std::to_wstring(myEfcts[i].second), lineStartX + xCorrection, pivotY + 10, col::white);
			
			//if (seg1 > 0) drawSprite(spr::segment, myEfcts[i].second / 100, pivotX + 97, pivotY + 20);
			//if (seg2 > 0 || seg1 > 0) drawSprite(spr::segment, (myEfcts[i].second % 100)/10, pivotX + 97 + 11, pivotY + 20);
			//if (seg3 > 0 || seg2 > 0 || seg3 > 0) drawSprite(spr::segment, myEfcts[i].second % 10, pivotX + 97 + 22, pivotY + 20);
			//setZoom(1.0);
		}
		else
		{
			//if (myEfcts[i].first == statEfctFlag::bleeding)
			//{
			//	//게이지
			//	drawRect(pivotX + 37, pivotY + 20, 61, 11, col::gray);
			//	drawFillRect(pivotX + 37 + 2, pivotY + 20 + 2, 61 - 4 - 20, 11 - 4, lowCol::red);
			//}
		}
	}
}

void HUD::drawHoverItemInfo()
{
	if (getLastGUI() == ContextMenu::ins() || getLastGUI() == this)
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
				if(vehPtr->vehType == vehFlag::heli) drawSpriteCenter(spr::icon16, 89, pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 7);
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
					drawSpriteCenter(spr::itemset, tgtPart.sprIndex, pivotX + 24, newPivotY + 12 + 17 * i);

					//아이템 이름
					renderText(tgtPart.name, pivotX + 35, newPivotY + 6 + 17 * i);

					//연료량
					drawRect(pivotX + 135, newPivotY + 7 + 17 * i, 53, 11, col::white);
					drawFillRect(pivotX + 135 + 2, newPivotY + 7 + 2 + 17 * i, 45, 7, lowCol::orange);
					drawEplsionText(L"32.7/30.0 L", pivotX + 135 + 3, newPivotY + 7 + 3 + 17 * i, col::white);
				}
			}
			else if (stackPtr != nullptr && false)
			{
				int pivotX = cameraW - 200;
				int pivotY = 148;

				drawFillRect(pivotX, pivotY, 192, 17, col::black, 200);
				drawRect(pivotX, pivotY, 192, 17, col::lightGray, 255);
				setFontSize(10);
				std::wstring titleName = itemDex[TileFloor(tgtGrid.x, tgtGrid.y, PlayerZ())].name;
				renderTextCenter(titleName, pivotX + 96, pivotY + 9);
				drawSpriteCenter(spr::itemset, itemDex[TileFloor(tgtGrid.x, tgtGrid.y, PlayerZ())].sprIndex, pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 10);


				int newPivotY = pivotY + 16;


				int stackSize = stackPtr->getPocket()->itemInfo.size();

				drawFillRect(pivotX, newPivotY, 192, 25 + 17 * (stackSize - 1), col::black, 200);
				drawRect(pivotX, newPivotY, 192, 25 + 17 * (stackSize - 1), col::lightGray, 255);


				for (int i = 0; i < stackSize; i++)
				{
					ItemData& tgtPart = stackPtr->getPocket()->itemInfo[i];
					//내구도
					drawRect(pivotX + 6, newPivotY + 6 + 17 * i, 6, 13, col::white);
					drawFillRect(pivotX + 8, newPivotY + 8 + 17 * i, 2, 9, lowCol::green);

					//아이템 아이콘
					drawSpriteCenter(spr::itemset, tgtPart.sprIndex, pivotX + 24, newPivotY + 12 + 17 * i);

					//아이템 이름
					renderText(tgtPart.name, pivotX + 35, newPivotY + 6 + 17 * i);

					//연료량
					//drawRect(pivotX + 135, newPivotY + 7 + 17 * i, 53, 11, col::white);
					//drawFillRect(pivotX + 135 + 2, newPivotY + 7 + 2 + 17 * i, 45, 7, lowCol::orange);
					//drawEplsionText(L"32.7/30.0 L", pivotX + 135 + 3, newPivotY + 7 + 3 + 17 * i, col::white);
				}
			}

		}
	}
}

void HUD::drawQuest()
{
	setFontSize(16);
	renderText(L"죽음을 거스르는 희망", 8, 182);

	drawLine(8, 200, 118, 200);
	for (int i = 0; i < 60; i++)
	{
		drawPoint(119 + i, 200, col::white, 255 - 4 * i);
	}


	drawRect({ 9, 206,9,9 }, col::white);
	setFontSize(10);
	int elapsedDay = getElapsedDays();
	std::wstring questStr = L"100일 동안 생존  (";
	questStr += std::to_wstring(elapsedDay);
	questStr += L"/100)";
	renderText(questStr, 21, 204);
}