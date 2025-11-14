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


namespace tabSprFlag
{
	constexpr int ONE_BTN = 0;
	constexpr int ONE_BTN_HOVER = 1;
	constexpr int ONE_BTN_CLICK = 2;
	constexpr int APPLE_BTN = 3;
	constexpr int APPLE_BTN_HOVER = 4;
	constexpr int APPLE_BTN_CLICK = 5;
	constexpr int TWO_BTN = 6;
	constexpr int TWO_BTN_TWO_HOVER = 7;
	constexpr int TWO_BTN_TWO_CLICK = 8;
	constexpr int TWO_BTN_UP_HOVER = 9;
	constexpr int TWO_BTN_UP_CLICK = 10;
	constexpr int TWO_BTN_DOWN_HOVER = 11;
	constexpr int TWO_BTN_DOWN_CLICK = 12;
}

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




	drawStadium(letterbox.x, letterbox.y, letterbox.w, letterbox.h, { 0,0,0 }, 150, 5);
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
			setZoom(6.0);
			drawTexture(texture::minimap, 13, 13);
			setZoom(1.0);

			drawSprite(spr::minimapEdge, 0, 14, 14);

			setZoom(1.5);
			Point2 minimapBtn = { 36,36 };
			Sprite* targetBtnSpr;
			if (option::inputMethod == input::gamepad)
			{
				if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_L1, minimapBtn.x, minimapBtn.y);
			}
			else if (option::inputMethod == input::mouse)
			{
				targetBtnSpr = spr::buttons;
				drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_M, minimapBtn.x, minimapBtn.y);
			}
			setZoom(1.0);

		}


		int vShift = 396 * (ctrlVeh != nullptr);
		int LETTERBOX_Y_OFFSET = 5; // 하단 박스 요소들의 Y 오프셋

		if (1)//플레이어 이름 그리기
		{
			setFont(fontType::mainFontSemiBold);
			setFontSize(22);
			drawText(L"Jackson, Practitioner of Elivilon ******", letterbox.x + 14 + vShift, letterbox.y +1 + LETTERBOX_Y_OFFSET, lowCol::yellow);
			setFont(fontType::mainFont);
		}

		auto drawMainGaugeFill = [](int x, int y, double ratio, SDL_Color color, Uint8 alpha = 255)
			{
				SDL_SetRenderTarget(renderer, texture::mainGaugeWhiteShadow);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);
				SDL_SetTextureBlendMode(texture::mainGaugeWhiteShadow, SDL_BLENDMODE_BLEND);

				SDL_Rect clipRect = { 0, 0, static_cast<int>(170 * ratio), 16 };
				SDL_SetRenderClipRect(renderer, &clipRect);

				drawSprite(spr::mainGauge, 0, 0, 0);

				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
				SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
				SDL_FRect rect = { 0, 0, static_cast<float>(170 * ratio), 16.0f };
				SDL_RenderFillRect(renderer, &rect);

				SDL_SetRenderClipRect(renderer, nullptr);

				SDL_SetRenderTarget(renderer, nullptr);
				SDL_SetTextureAlphaMod(texture::mainGaugeWhiteShadow, alpha);
				drawTexture(texture::mainGaugeWhiteShadow, x, y);
				SDL_SetTextureAlphaMod(texture::mainGaugeWhiteShadow, 255);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			};

		const int GAUGE_Y_OFFSET = -4;

		{
			int stressPivotX = letterbox.x + 13;
			int stressPivotY = letterbox.y + 38;
			drawSprite(spr::icon28, 1, stressPivotX, stressPivotY);
			int stressGaugePivotX = stressPivotX + 34;
			int stressGaugePivotY = stressPivotY + 6 + GAUGE_Y_OFFSET;
			drawSprite(spr::mainGauge, 0, stressGaugePivotX, stressGaugePivotY);
			drawMainGaugeFill(stressGaugePivotX, stressGaugePivotY, 0.2, col::red);
			setFont(fontType::mainFontSemiBold);
			setFontSize(16);
			drawTextOutline(L"92 / 100", stressGaugePivotX + 98, stressGaugePivotY + 9, lowCol::white);
			setFont(fontType::mainFont);
		}

		{
			int pSTA = PlayerPtr->entityInfo.STA;
			int pSTAMax = PlayerPtr->entityInfo.maxSTA;
			if (fakeSTA > pSTA)
			{
				fakeSTA -= 1;
				if (alphaSTA > 10) alphaSTA -= 10;
				else alphaSTA = 0;
			}
			else if (fakeSTA < pSTA) fakeSTA++;
			else alphaSTA = 150;


			int staminaPivotX = letterbox.x + 13 + 224;
			int staminaPivotY = letterbox.y + 38;
			drawSprite(spr::icon28, 3, staminaPivotX, staminaPivotY);

			int staminaGaugePivotX = staminaPivotX + 34;
            int staminaGaugePivotY = staminaPivotY + 6 + GAUGE_Y_OFFSET;
			drawSprite(spr::mainGauge, 0, staminaGaugePivotX, staminaGaugePivotY);
			drawMainGaugeFill(staminaGaugePivotX, staminaGaugePivotY, (double)fakeSTA / (double)pSTAMax, lowCol::white, alphaSTA);
			drawMainGaugeFill(staminaGaugePivotX, staminaGaugePivotY, (double)pSTA / (double)pSTAMax, lowCol::yellow);

			setFont(fontType::mainFontSemiBold);
			setFontSize(16);
			drawTextOutline(std::to_wstring(pSTA) + L" / " + std::to_wstring(pSTAMax), staminaGaugePivotX + 98, staminaGaugePivotY + 9, lowCol::white);
			setFont(fontType::mainFont);
		}

  //      drawSprite(spr::mainGauge, 0, staminaPivotX + 34, staminaPivotY + 6);
		//for (int i = 0; i < 170; i++)
		//{
		//	if (i == 0) drawLine(staminaPivotX + 34, staminaPivotY + 6 + 2, staminaPivotX + 34, staminaPivotY + 6 + 2 + 11);
		//	else if (i == 1) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6 + 1, staminaPivotX + 34 + i, staminaPivotY + 6 + 1 + 13);
		//	else if (i == 68) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 14);
		//	else if (i == 69) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 13);
		//	else if (i == 70) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 12);
		//	else if (i == 71)  drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 11);
		//	else if (i==72) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 10);
		//	else if (i == 73) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 9);
		//	else if (i == 169)  drawLine(staminaPivotX + 34 + i, staminaPivotY + 6 + 1, staminaPivotX + 34 + i, staminaPivotY + 6 + 1 + 5);
		//	else if ( i >= 74) drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 8);
		//	else drawLine(staminaPivotX + 34 + i, staminaPivotY + 6, staminaPivotX + 34 + i, staminaPivotY + 6 + 15);
		//}



		//setFont(fontType::pixel);
		//setFontSize(11);

		//setFont(fontType::pixel);
		//setFontSize(11);

		//{
		//	// HP 게이지 그리기 람다 함수
		//	auto drawBodyPartHP = [&](
		//		int& fakeHP,
		//		int& realHP,
		//		unsigned char& fakeHPAlpha,
		//		int maxHP,
		//		int pivotX,
		//		int pivotY,
		//		const std::wstring& partName) -> void
		//		{
		//			// 페이크 HP 업데이트
		//			if (fakeHP > realHP) { fakeHP--; }
		//			else if (fakeHP < realHP) fakeHP = realHP;
		//			if (fakeHP != realHP)
		//			{
		//				if (fakeHPAlpha > 30) { fakeHPAlpha -= 30; }
		//				else { fakeHPAlpha = 0; }
		//			}
		//			else { fakeHPAlpha = 255; }
		//			int gaugeX = pivotX + 2;
		//			drawTextCenter(partName, pivotX - 16, pivotY + 5, col::lightGray);
		//			// 페이크 HP
		//			float ratioFakeHP = myMax(0.0f, static_cast<float>(fakeHP) / static_cast<float>(maxHP));
		//			SDL_Rect fakeRect = { gaugeX + 3, pivotY + 3, static_cast<int>(38 * ratioFakeHP), 5 };
		//			drawFillRect(fakeRect, lowCol::white, fakeHPAlpha);
		//			// 실제 HP
		//			float ratioHP = myMax(0.0f, static_cast<float>(realHP) / static_cast<float>(maxHP));
		//			SDL_Rect realRect = { gaugeX + 3, pivotY + 3, static_cast<int>(38 * ratioHP), 5 };
		//			if (ratioHP > 0 && realRect.w == 0) { realRect.w = 1; }
		//			drawFillRect(realRect, lowCol::green);
		//		};

		//	int X_OFFSET = 0;
		//	int Y_OFFSET = 3;
		//	int X_DIST = 83;
		//	int Y_DIST = 16;

		//	// 왼팔
		//	drawBodyPartHP(PlayerPtr->lArmFakeHP, PlayerPtr->lArmHP, PlayerPtr->lArmFakeHPAlpha,
		//		PART_MAX_HP,
		//		letterbox.x + 36 + vShift + X_OFFSET, letterbox.y + 22 + Y_OFFSET + LETTERBOX_Y_OFFSET, sysStr[108]);
		//	// 좌다리
		//	drawBodyPartHP(PlayerPtr->lLegFakeHP, PlayerPtr->lLegHP, PlayerPtr->lLegFakeHPAlpha,
		//		PART_MAX_HP,
		//		letterbox.x + 36 + vShift + X_OFFSET, letterbox.y + 22 + Y_OFFSET + Y_DIST + LETTERBOX_Y_OFFSET, sysStr[110]);
		//	// 머리
		//	drawBodyPartHP(PlayerPtr->headFakeHP, PlayerPtr->headHP, PlayerPtr->headFakeHPAlpha,
		//		PART_MAX_HP,
		//		letterbox.x + 36 + X_DIST + vShift + X_OFFSET, letterbox.y + 22 + Y_OFFSET + LETTERBOX_Y_OFFSET, sysStr[107]);
		//	// 몸통
		//	drawBodyPartHP(PlayerPtr->entityInfo.fakeHP, PlayerPtr->entityInfo.HP, PlayerPtr->entityInfo.fakeHPAlpha,
		//		PlayerPtr->entityInfo.maxHP,
		//		letterbox.x + 36 + X_DIST + vShift + X_OFFSET, letterbox.y + 22 + Y_OFFSET + Y_DIST + LETTERBOX_Y_OFFSET, sysStr[106]);
		//	// 오른팔
		//	drawBodyPartHP(PlayerPtr->rArmFakeHP, PlayerPtr->rArmHP, PlayerPtr->rArmFakeHPAlpha,
		//		PART_MAX_HP,
		//		letterbox.x + 36 + X_DIST * 2 + vShift + X_OFFSET, letterbox.y + 22 + Y_OFFSET + LETTERBOX_Y_OFFSET, sysStr[109]);
		//	// 우다리
		//	drawBodyPartHP(PlayerPtr->rLegFakeHP, PlayerPtr->rLegHP, PlayerPtr->rLegFakeHPAlpha,
		//		PART_MAX_HP,
		//		letterbox.x + 36 + X_DIST * 2 + vShift + X_OFFSET, letterbox.y + 22 + Y_OFFSET + Y_DIST + LETTERBOX_Y_OFFSET, sysStr[111]);
		//}


		//if (ctrlVeh == nullptr)
		//{
			////스테미나
			//{
			//	int pivotX = letterbox.x + 18 + 238;
			//	int pivotY = letterbox.y + 4 + LETTERBOX_Y_OFFSET;
			//	setFontSize(10);

			//	int pSTA = PlayerPtr->entityInfo.STA;
			//	int pSTAMax = PlayerPtr->entityInfo.maxSTA;


			//	if (fakeSTA > pSTA)
			//	{
			//		fakeSTA -= 1;
			//		if (alphaSTA > 10) alphaSTA -= 10;
			//		else alphaSTA = 0;
			//	}
			//	else if (fakeSTA < pSTA) fakeSTA++;
			//	else alphaSTA = 255;


			//	SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), 0, 0, 0);
			//	SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
			//	drawSprite(spr::staminaGaugeCircle, 0, pivotX, pivotY);

			//	if (fakeSTA > pSTA)
			//	{

			//		int sprIndexFake = (int)(90.0 - 90.0 * ((double)fakeSTA / (double)pSTAMax));
			//		SDL_SetTextureAlphaMod(spr::staminaGaugeCircle->getTexture(), alphaSTA);
			//		SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), lowCol::red.r, lowCol::red.g, lowCol::red.b);
			//		SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
			//		drawSprite(spr::staminaGaugeCircle, sprIndexFake, pivotX, pivotY);
			//		SDL_SetTextureAlphaMod(spr::staminaGaugeCircle->getTexture(), 255);


			//		int sprIndex = (int)(90.0 - 90.0 * ((double)pSTA / (double)pSTAMax));
			//		SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), lowCol::yellow.r, lowCol::yellow.g, lowCol::yellow.b);
			//		SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
			//		drawSprite(spr::staminaGaugeCircle, sprIndex, pivotX, pivotY);
			//	}
			//	else
			//	{
			//		int sprIndex = (int)(90.0 - 90.0 * ((double)fakeSTA / (double)pSTAMax));
			//		SDL_SetTextureColorMod(spr::staminaGaugeCircle->getTexture(), lowCol::yellow.r, lowCol::yellow.g, lowCol::yellow.b);
			//		SDL_SetTextureBlendMode(spr::staminaGaugeCircle->getTexture(), SDL_BLENDMODE_BLEND);
			//		drawSprite(spr::staminaGaugeCircle, sprIndex, pivotX, pivotY);
			//	}


			//	setFont(fontType::mainFont);
			//	setFontSize(12);
			//	drawTextCenter(L"STA", pivotX + 24, pivotY + 16);
			//	setFont(fontType::pixel);
			//	setFontSize(11);
			//	std::wstring STAStr = std::to_wstring(PlayerPtr->entityInfo.STA) + L"/" + std::to_wstring(PlayerPtr->entityInfo.maxSTA);
			//	drawTextOutlineCenter(STAStr, pivotX + 24, pivotY + 29);
			//}

		//	setFont(fontType::pixel);
		//	setFontSize(12);
		//	drawSpriteCenter(spr::icon13, 25, letterbox.x + 18 + 296 + 5, letterbox.y + 5 + 15 * 0 + 6 + LETTERBOX_Y_OFFSET);
		//	drawText(L"6320", letterbox.x + 18 + 296 + 17, letterbox.y + 5 + 15 * 0 + LETTERBOX_Y_OFFSET, lowCol::yellow);

		//	setFont(fontType::mainFont);
		//	setFontSize(11);
		//	drawText(L"SPEED", letterbox.x + 18 + 296, letterbox.y + 2 + 18 * 1 + LETTERBOX_Y_OFFSET, col::lightGray);
		//	drawText(L"MENTAL", letterbox.x + 18 + 296, letterbox.y + 2 + 18 * 2 + LETTERBOX_Y_OFFSET, col::lightGray);

		//	setFont(fontType::pixel);
		//	setFontSize(12);
		//	drawText(L"120%", letterbox.x + 18 + 44 + 296, letterbox.y + 5 + 18 * 1 + LETTERBOX_Y_OFFSET, lowCol::green);
		//	drawText(L"39%", letterbox.x + 18 + 50 + 296, letterbox.y + 5 + 18 * 2 + LETTERBOX_Y_OFFSET, lowCol::red);


		//	//시간 표시 기능
		//	{
		//		int timeIndex;
		//		// PM or AM
		//		if (getHour() >= 12) { timeIndex = segmentIndex::pm; }
		//		else { timeIndex = segmentIndex::am; }
		//		drawSprite(spr::segment, timeIndex, letterbox.x + 18 + 446 + 14 * 0, letterbox.y + 10 + LETTERBOX_Y_OFFSET);
		//		//xx시
		//		drawSprite(spr::segment, getHour() / 10, letterbox.x + 18 + 446 + 14 * 1, letterbox.y + 10 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, getHour() % 10, letterbox.x + 18 + 446 + 14 * 2, letterbox.y + 10 + LETTERBOX_Y_OFFSET);
		//		//:
		//		drawSprite(spr::segment, segmentIndex::colon, letterbox.x + 18 + 446 + 14 * 3, letterbox.y + 10 + LETTERBOX_Y_OFFSET);
		//		//xx분
		//		drawSprite(spr::segment, getMin() / 10, letterbox.x + 18 + 446 + 14 * 4, letterbox.y + 10 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, getMin() % 10, letterbox.x + 18 + 446 + 14 * 5, letterbox.y + 10 + LETTERBOX_Y_OFFSET);
		//		//xx초
		//		setZoom(0.7);
		//		drawSprite(spr::segment, getSec() / 10, letterbox.x + 18 + 446 + 14 * 6, letterbox.y + 15 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, getSec() % 10, letterbox.x + 18 + 446 + 14 * 6 + 10, letterbox.y + 15 + LETTERBOX_Y_OFFSET);

		//		//xxxx년
		//		drawSprite(spr::segment, getYear() / 1000, letterbox.x + 18 + 460 + 10 * -1, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, (getYear() % 1000) / 100, letterbox.x + 18 + 460 + 10 * 0, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, ((getYear() % 100) / 10), letterbox.x + 18 + 460 + 10 * 1, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, getYear() % 10, letterbox.x + 18 + 460 + 10 * 2, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		//:
		//		drawSprite(spr::segment, segmentIndex::dash, letterbox.x + 18 + 460 + 10 * 3, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		//xx월
		//		drawSprite(spr::segment, getMonth() / 10, letterbox.x + 18 + 460 + 10 * 4, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, getMonth() % 10, letterbox.x + 18 + 460 + 10 * 5, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		//:
		//		drawSprite(spr::segment, segmentIndex::dash, letterbox.x + 18 + 460 + 10 * 6, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		//xx일
		//		drawSprite(spr::segment, getDay() / 10, letterbox.x + 18 + 460 + 10 * 7, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		drawSprite(spr::segment, getDay() % 10, letterbox.x + 18 + 460 + 10 * 8, letterbox.y + 7 + 25 + LETTERBOX_Y_OFFSET);
		//		setZoom(1.0);
		//	}





			{
				int cx, cy;
				int pz = PlayerZ();
				World::ins()->changeToChunkCoord(PlayerX(), PlayerY(), cx, cy);
				weatherFlag currentWeather = World::ins()->getChunkWeather(cx, cy, pz);
				
				int pivotX = letterbox.x + 458;
				int pivotY = letterbox.y + 4 + LETTERBOX_Y_OFFSET;

				// 하늘 배경 그리기 (기존 코드)
				int eclipticIndex = 0;
				if (getHour() >= 7 && getHour() < 17)
				{
					eclipticIndex = 2; // 낮 하늘
				}
				else if (getHour() >= 17 && getHour() < 18)
				{
					eclipticIndex = 3; // 노을
				}
				else if (getHour() >= 6 && getHour() < 7)
				{
					eclipticIndex = 1; // 새벽
				}
				else
				{
					eclipticIndex = 4; // 밤
				}

				if (getHour() >= 6 && getHour() < 18)
				{
					if (currentWeather == weatherFlag::cloudy || currentWeather == weatherFlag::rain ||
						currentWeather == weatherFlag::snow || currentWeather == weatherFlag::storm)
					{
						eclipticIndex = 5; // 회색 하늘
					}
				}

				// 이클립스 배경 이미지 그리기
				drawSprite(spr::ecliptic, eclipticIndex, pivotX, pivotY);


				if (currentWeather == weatherFlag::sunny)
				{
					if (getHour() >= 6 && getHour() < 18)
					{
						static int index = 0;
						int sprSize = 6;
						if (timer::timer600 % 12 == 0)
						{
							index++;
							if (index == sprSize) index = 0;
						}
						drawSpriteCenter(spr::symbolSunny, index, pivotX + 38, pivotY + 27);
					}
					else
					{
						static int moonBrightnessTimer = 0;
						moonBrightnessTimer++;

						float breatheCycle = 240.0f;
						float brightness = (sin(moonBrightnessTimer * 2.0f * 3.141592 / breatheCycle) + 1.0f) * 0.5f;

						Uint8 alpha = static_cast<Uint8>(128 + brightness * 127);


						int moonPhaseIndex = calculateMoonPhase();
						SDL_SetTextureAlphaMod(spr::symbolMoon->getTexture(), alpha);
						SDL_SetTextureBlendMode(spr::symbolMoon->getTexture(), SDL_BLENDMODE_BLEND);

						drawSpriteCenter(spr::symbolMoon, moonPhaseIndex, pivotX + 38, pivotY + 27);
						SDL_SetTextureAlphaMod(spr::symbolMoon->getTexture(), 255);
					}
				}
				else if (currentWeather == weatherFlag::cloudy)
				{
					static int index = 0;
					int sprSize = 26;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolCloudy, index, pivotX + 38, pivotY + 27);
				}
				else if (currentWeather == weatherFlag::rain)
				{
					static int index = 0;
					int sprSize = 8;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolRain, index, pivotX + 38, pivotY + 27);
				}
				else if (currentWeather == weatherFlag::storm)
				{
					static int index = 0;
					int sprSize = 8;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolStorm, index, pivotX + 38, pivotY + 27);
				}
				else if (currentWeather == weatherFlag::snow)
				{
					static int index = 0;
					int sprSize = 15;
					if (timer::timer600 % 12 == 0)
					{
						index++;
						if (index >= sprSize) index = 0;
					}
					drawSpriteCenter(spr::symbolSnow, index, pivotX + 38, pivotY + 27);
				}



					setFontSize(16);
					drawTextCenter(L"15℃", pivotX  + 38, pivotY + 52);
			}








		//	drawSprite(spr::batteryGauge, 4, letterbox.x + 18 + 562, letterbox.y + 3 + LETTERBOX_Y_OFFSET);
		//	
		//	setFontSize(11);
		//	drawTextOutlineCenter(L"72%", letterbox.x + 18 + 562 + 16, letterbox.y + 3 + 24 + LETTERBOX_Y_OFFSET);
		//}

		//팝업 버튼
		drawSprite(spr::menuPopUp, 0, letterbox.x + letterbox.w - 42, letterbox.y - 36);
		{
			SDL_Color popUpBtnColor = lowCol::black;
			if (getLastGUI() == this)
			{
				if (checkCursor(&letterboxPopUpButton))
				{
					if (click == true) { popUpBtnColor = lowCol::deepBlue; }
					else { popUpBtnColor = lowCol::blue; }
				}
				else if (state[SDL_SCANCODE_RETURN]) { popUpBtnColor = lowCol::blue; }
				else { popUpBtnColor = lowCol::black; }
			}

			drawStadium(letterboxPopUpButton.x, letterboxPopUpButton.y, letterboxPopUpButton.w, letterboxPopUpButton.h, popUpBtnColor, 200, 5);

			if (option::inputMethod == input::gamepad)
			{
				if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_NORTH) && getLastGUI() == this) { targetBtnSpr = spr::buttonsPressed; }
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
			if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) drawSprite(spr::tab, tabSprFlag::ONE_BTN_CLICK, tab.x, tab.y - 2);
			else if (SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000) drawSprite(spr::tab, tabSprFlag::ONE_BTN_HOVER, tab.x, tab.y - 2);
			else drawSprite(spr::tab, tabSprFlag::ONE_BTN, tab.x, tab.y - 2);
		}
		else
		{
			if (checkCursor(&tabSmallBox))
			{
				if (click == false)
				{
					drawSprite(spr::tab, tabSprFlag::APPLE_BTN, tab.x, tab.y - 2);
					drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::blue, 255, 3);
				}
				else
				{
					drawSprite(spr::tab, tabSprFlag::APPLE_BTN, tab.x, tab.y - 2);
					drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::deepBlue, 255, 3);
				}
			}
			else if (checkCursor(&tab))
			{
				if (click == false)
				{
					drawSprite(spr::tab, tabSprFlag::APPLE_BTN_HOVER, tab.x, tab.y - 2);
					drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::black, 255, 3);
				}
				else
				{
					drawSprite(spr::tab, tabSprFlag::APPLE_BTN_CLICK, tab.x, tab.y - 2);
					drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::black, 255, 3);
				}
			}
			else
			{
				drawSprite(spr::tab, tabSprFlag::APPLE_BTN, tab.x, tab.y - 2);
				drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::black, 255, 3);
			}
		}
		if (option::language == L"Korean") setFontSize(12);
		else  setFontSize(10);
		setZoom(1.5);
		drawSpriteCenter(spr::icon48, 1, tab.x + 42 + 25, tab.y + 55 + 12);
		drawSpriteCenter(spr::icon48, 2, tab.x + 72 + 40, tab.y + 64 + 29);
		setZoom(1.0);
		setZoom(1.5);
		drawSpriteCenter(spr::icon48, 158, tabSmallBox.x + tabSmallBox.w / 2, tabSmallBox.y + tabSmallBox.h / 2);
		setZoom(1.0);
		setFontSize(14);
		drawTextCenter(sysStr[1], tab.x + tab.w / 2, tab.y + tab.h - 36);
		drawTextCenter(sysStr[2], tab.x + tab.w / 2, tab.y + tab.h - 36 + 16);
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
			if (SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) drawSprite(spr::tab, tabSprFlag::ONE_BTN_CLICK, tab.x, tab.y - 2);
			else if (SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000) drawSprite(spr::tab, tabSprFlag::ONE_BTN_HOVER, tab.x, tab.y - 2);
			else drawSprite(spr::tab, tabSprFlag::ONE_BTN, tab.x, tab.y - 2);
		}
		else
		{
			drawLine(tab.x + 0, tab.y + 178, tab.x + 178, tab.y + 0, lowCol::red, 200);
			if (checkCursor(&tabSmallBox))
			{
				drawSprite(spr::tab, tabSprFlag::TWO_BTN, tab.x, tab.y - 2);
				if (click == false) drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::blue, 255, 3);
				else drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::deepBlue, 255, 3);
			}
			else if (checkCursor(&tab))
			{

				drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::black, 255, 3);
				if ((getMouseX() - tab.x) + (getMouseY() - tab.y) < 178)
				{
					if (click == false) { drawSprite(spr::tab, tabSprFlag::TWO_BTN_UP_HOVER, tab.x, tab.y - 2); }
					else { drawSprite(spr::tab, tabSprFlag::TWO_BTN_UP_CLICK, tab.x, tab.y - 2); }
				}
				else // 마우스 커서가 직선 위에 있거나(경계 포함) 아래에 있는 경우
				{
					if (click == false) { drawSprite(spr::tab, tabSprFlag::TWO_BTN_DOWN_HOVER, tab.x, tab.y - 2); }
					else { drawSprite(spr::tab, tabSprFlag::TWO_BTN_DOWN_CLICK, tab.x, tab.y - 2); }
				}
			}
			else
			{
				drawSprite(spr::tab, tabSprFlag::TWO_BTN, tab.x, tab.y - 2);
				drawStadium(tab.x + 116, tab.y - 6, 68, 68, lowCol::black, 255, 3);
			}
		}




		setFontSize(12);
		drawTextCenter(sysStr[195], tab.x + 62, tab.y + 9);
		drawTextCenter(sysStr[207], tab.x + 104, tab.y + 52);
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
		setZoom(1.5);
		drawSpriteCenter(spr::icon48, 17, tab.x + 90, tab.y + 78);
		setZoom(1.0);
		setFontSize(22);
		drawTextCenter(sysStr[14], tab.x + 90, tab.y + 150);
		break;
	case tabFlag::back:
		//뒤로가기
		drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
		setZoom(1.5);
		drawSpriteCenter(spr::icon48, 182, tab.x + 90, tab.y + 78);
		setZoom(1.0);
		setFontSize(22);
		drawTextCenter(sysStr[31], tab.x + 90, tab.y + 150);
		break;
	case tabFlag::confirm:
		//뒤로가기
		drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
		setZoom(1.5);
		drawSpriteCenter(spr::icon48, 39, tab.x + 90, tab.y + 78);
		setZoom(1.0);
		setFontSize(22);
		drawTextCenter(sysStr[91], tab.x + 90, tab.y + 150);
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
	const bool* state = SDL_GetKeyboardState(nullptr);
	int pivotX = quickSlotRegion.x;
	int pivotY = quickSlotRegion.y;
	const SDL_Scancode quickSlotKeys[8] = { SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8 };

	bool currentSkillFoundInQuickSlot = false;

	for (int i = 0; i < 8; i++)
	{
		int pivotX = 185 + 90 + 72 * i;
		int pivotY = 0;
		SDL_Color btnCol = col::black;
		bool keyPressed = state[quickSlotKeys[i]];
		bool showSkillName = (checkCursor(&quickSlotBtn[i]) && getLastGUI() == this) || keyPressed;
		bool deact = false; //현재 사용이 불가능한 스킬

		if (quickSlot[i].first == quickSlotFlag::SKILL && (quickSlot[i].second == skillRefCode::roll || quickSlot[i].second == skillRefCode::leap))
		{
			if (itemDex[TileFloor(PlayerX(), PlayerY(), PlayerZ())].checkFlag(itemFlag::WATER_SHALLOW) || itemDex[TileFloor(PlayerX(), PlayerY(), PlayerZ())].checkFlag(itemFlag::WATER_DEEP))
			{
				deact = true;
			}
		}

		if (currentUsingSkill == -1)
		{
			if (checkCursor(&quickSlotBtn[i]) && quickSlot[i].first != quickSlotFlag::NONE && getLastGUI() == this)
			{
				if (click) btnCol = lowCol::deepBlue;
				else btnCol = lowCol::blue;
			}
			else if (keyPressed) btnCol = lowCol::deepBlue;
		}
		else if (quickSlot[i].first != quickSlotFlag::NONE && currentUsingSkill == quickSlot[i].second)
		{
			btnCol = lowCol::blue;
			showSkillName = true;
			currentSkillFoundInQuickSlot = true;
		}


		if (deact) btnCol = col::black;

		//스킬 이름 표시
		if (showSkillName && quickSlot[i].first != quickSlotFlag::NONE && deact==false)
		{
			setFont(fontType::pixel);
			setFontSize(12);
			std::wstring skillName = skillDex[quickSlot[i].second].name;
			drawTextOutlineCenter(skillName, quickSlotRegion.x + quickSlotRegion.w / 2, quickSlotRegion.h + 15);
			
		}

		SDL_Rect skillRect = SDL_Rect{ pivotX,pivotY,62,58 };
		drawFillRect(skillRect, btnCol, 200);
		drawLine(pivotX + 1, pivotY + skillRect.h, pivotX + skillRect.w - 2, pivotY + skillRect.h, btnCol, 200);
		drawLine(pivotX + 2, pivotY + skillRect.h + 1, pivotX + skillRect.w - 3, pivotY + skillRect.h + 1, btnCol, 200);
		drawLine(pivotX + 3, pivotY + skillRect.h + 2, pivotX + skillRect.w - 4, pivotY + skillRect.h + 2, btnCol, 200);
		drawLine(pivotX + 4, pivotY + skillRect.h + 3, pivotX + skillRect.w - 5, pivotY + skillRect.h + 3, btnCol, 200);
		drawLine(pivotX + 5, pivotY + skillRect.h + 4, pivotX + skillRect.w - 6, pivotY + skillRect.h + 4, btnCol, 200);
		drawLine(pivotX + 6, pivotY + skillRect.h + 5, pivotX + skillRect.w - 7, pivotY + skillRect.h + 5, btnCol, 200);

		if (quickSlot[i].first == quickSlotFlag::NONE)
		{

			drawFillRect(SDL_Rect{ pivotX + 6, pivotY + 1 ,48,48 }, col::black, 180);

			drawCross2(pivotX + 5, pivotY, 0, 7, 0, 7,col::gray);
			drawCross2(pivotX + 5 + 48, pivotY, 0, 7, 7, 0, col::gray);
			drawCross2(pivotX + 5, pivotY + 48, 7, 0, 0, 7, col::gray);
			drawCross2(pivotX + 5 + 48, pivotY + 48, 7, 0, 7, 0, col::gray);
		}
		else
		{
			setZoom(3.0);
			drawSprite(spr::skillSet, skillDex[quickSlot[i].second].iconIndex, pivotX + 6, pivotY + 1);
			setZoom(1.0);
			drawCross2(pivotX + 5, pivotY, 0, 7, 0, 7);
			drawCross2(pivotX + 5 + 48, pivotY, 0, 7, 7, 0);
			drawCross2(pivotX + 5, pivotY + 48, 7, 0, 0, 7);
			drawCross2(pivotX + 5 + 48, pivotY + 48, 7, 0, 7, 0);
		}
		setFont(fontType::pixel);
		setFontSize(12);
		drawTextCenter(std::to_wstring(i + 1), pivotX + skillRect.w/2, pivotY + 58);

		if (deact)
		{
			drawFillRect(SDL_Rect{ pivotX,pivotY,44,39 }, col::black, 120);
			drawLine(pivotX + 1, pivotY + 39, pivotX + 42, pivotY + 39, col::black, 120);
			drawLine(pivotX + 2, pivotY + 39 + 1, pivotX + 41, pivotY + 39 + 1, col::black, 120);
			drawLine(pivotX + 3, pivotY + 39 + 2, pivotX + 40, pivotY + 39 + 2, col::black, 120);
			drawLine(pivotX + 4, pivotY + 39 + 3, pivotX + 39, pivotY + 39 + 3, col::black, 120);
			drawLine(pivotX + 5, pivotY + 39 + 4, pivotX + 38, pivotY + 39 + 4, col::black, 120);
			drawLine(pivotX + 6, pivotY + 39 + 5, pivotX + 37, pivotY + 39 + 5, col::black, 120);
			drawLine(pivotX + 6, pivotY + 39 + 5, pivotX + 37, pivotY + 39 + 5, col::black, 120);
		}
		
	}

	if (dragQuickSlotTarget != -1)
	{
		if (quickSlot[dragQuickSlotTarget].first == quickSlotFlag::SKILL)
		{
			setZoom(2.0);
			SDL_SetTextureAlphaMod(spr::skillSet->getTexture(), 180);
			SDL_SetTextureBlendMode(spr::skillSet->getTexture(), SDL_BLENDMODE_BLEND);
			drawSpriteCenter(spr::skillSet, skillDex[quickSlot[dragQuickSlotTarget].second].iconIndex, getMouseX(), getMouseY());
			SDL_SetTextureAlphaMod(spr::skillSet->getTexture(), 255);
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


			if (ctrlVeh->isEngineOn) drawSpriteCenter(spr::vehicleHUDParts, 2 + (checkCursor(&barButton[3]) && click), barButton[3].x + barButton[3].w / 2, barButton[3].y + barButton[3].h / 2); //엔진 스타트
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

			drawStadium(barButton[i].x, barButton[i].y, barButton[i].w, barButton[i].h, btnColor, 200, 5);
			drawRect(letterboxInButton[i], outlineColor);
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
		else if (barAct[i] == act::equipment) setBtnLayout(sysStr[332], 79);
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
		else if (barAct[i] == act::sleep) setBtnLayout(sysStr[211], 172);
		else if (barAct[i] == act::saveAndQuit) setBtnLayout(sysStr[194], 156);
		else if (barAct[i] == act::propInstall) setBtnLayout(sysStr[328], 177);
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
			if (ctrlVeh->headlightOn == false) setBtnLayout(sysStr[205], 165);
			else setBtnLayout(sysStr[205], 164);
		}
		else if (barAct[i] == act::drink) setBtnLayout(sysStr[210], 37);
		else if (barAct[i] == act::dump ) setBtnLayout(sysStr[296], 174);
		else setBtnLayout(L" ", 0);

		//48*48 심볼 아이콘 그리기
		setZoom(1.5);
		drawSpriteCenter(spr::icon48, actIndex, barButton[i].x + (barButton[i].w / 2), barButton[i].y + (barButton[i].h / 2) - 10);
		setZoom(1.0);

		//하단 텍스트

		setFont(fontType::mainFont);
		setFontSize(16);
		drawTextCenter(actName, barButton[i].x + (barButton[i].w / 2), barButton[i].y + barButton[i].h - 14);

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


	//setFont(fontType::pixel);
	//setFontSize(12);
	//drawTextDirect(L"abcdefghijklmnopqrstuvwxyz 다람쥐 헌 쳇바퀴에 타고파", 350, 350);
}

void HUD::drawStatusEffects()
{
	std::vector<statusEffect>& myEfcts = PlayerPtr->entityInfo.statusEffectVec;

	// 페이크값 업데이트 람다 함수
	auto updateFakeValue = [](int& fakeValue, int realValue) {
		int diff = std::abs(fakeValue - realValue);
		if (diff == 0) return;

		int speed = 1;
		if (diff > 100) speed = 8;
		else if (diff > 50) speed = 4;
		else if (diff > 20) speed = 2;

		if (fakeValue < realValue) fakeValue += std::min(speed, realValue - fakeValue);
		else if (fakeValue > realValue) fakeValue -= std::min(speed, fakeValue - realValue);
		};

	// 페이크값들 업데이트
	updateFakeValue(fakeHunger, hunger);
	updateFakeValue(fakeThirst, thirst);
	updateFakeValue(fakeFatigue, fatigue);

	for (int i = 0; i < myEfcts.size(); i++)
	{
		int pivotX = 15;
		int pivotY = 370 + 45 * i;
		std::wstring statEfctName = L"";
		int statEfctIcon = 0;
		SDL_Color textColor = col::white;
		int textOffsetY = -3;

		switch (myEfcts[i].effectType)
		{
		case statusEffectFlag::confused:
			statEfctName = L"혼란";
			statEfctIcon = 1;
			break;

		case statusEffectFlag::bleeding:
			statEfctName = L"출혈";
			statEfctIcon = 2;
			textColor = lowCol::red;
			break;

		case statusEffectFlag::hungry:
			if (fakeHunger < PLAYER_STARVE_CALORIE)
			{
				statEfctName = sysStr[220]; // 영양실조 (Starving)
				textColor = lowCol::red;
			}
			else if (fakeHunger < PLAYER_VERY_HUNGRY_CALORIE)
			{
				statEfctName = sysStr[219]; // 굶주림 (Famished)
				textColor = lowCol::orange;
			}
			else if (fakeHunger < PLAYER_HUNGRY_CALORIE)
			{
				statEfctName = sysStr[218]; // 배고픔 (Hungry)
				textColor = lowCol::yellow;
			}
			else
			{
				statEfctName = sysStr[217]; // 배부름 (Full)
				textColor = col::green;
			}
			statEfctIcon = 3;
			break;

		case statusEffectFlag::dehydrated:
			if (fakeThirst < PLAYER_DEHYDRATION_HYDRATION)
			{
				statEfctName = sysStr[224]; // 탈수 상태 (Dehydrated)
				textColor = lowCol::red;
			}
			else if (fakeThirst < PLAYER_VERY_THIRSTY_HYDRATION)
			{
				statEfctName = sysStr[223]; // 심한 갈증 (Parched)
				textColor = lowCol::orange;
			}
			else if (fakeThirst < PLAYER_THIRSTY_HYDRATION)
			{
				statEfctName = sysStr[222]; // 목마름 (Thirsty)
				textColor = lowCol::yellow;
			}
			else
			{
				statEfctName = sysStr[221]; // 해갈 (Hydrated)
				textColor = col::green;
			}
			statEfctIcon = 4;
			break;

		case statusEffectFlag::blind:
			statEfctName = L"실명";
			statEfctIcon = 15;
			break;

		case statusEffectFlag::tired:
			statEfctIcon = 11;
			if (fakeFatigue < PLAYER_EXHAUSTED_FATIGUE)
			{
				statEfctName = sysStr[228]; // 탈진 상태 (Exhausted)
				textColor = lowCol::red;
			}
			else if (fakeFatigue < PLAYER_VERY_TIRED_FATIGUE)
			{
				statEfctName = sysStr[227]; // 심한 피로 (Weary)
				textColor = lowCol::orange;
			}
			else if (fakeFatigue < PLAYER_TIRED_FATIGUE)
			{
				statEfctName = sysStr[226]; // 피곤함 (Tired)
				textColor = lowCol::yellow;
			}
			else
			{
				statEfctName = sysStr[225]; // 개운함 (Refreshed)
				textColor = col::green;
				statEfctIcon = 58;
			}
			break;
		}

		setFontSize(15);

		setZoom(2.0); // 2.25→2.0 (픽셀 퍼펙트)
		drawSprite(spr::statusIcon, statEfctIcon, pivotX, pivotY);
		setZoom(1.0);

		setFont(fontType::mainFont);
		setFontSize(21);
		int textWidth = queryTextWidth(statEfctName) + 45;

		drawFillRect(SDL_Rect{ pivotX + 32, pivotY, textWidth, 32 }, col::black, 140);
		int lineStartX = pivotX + textWidth + 32;
		for (int i = 0; i < 18; i++)
		{
			drawLine(lineStartX + i, pivotY + 2 + i, lineStartX + i, pivotY + 31, col::black, 140);
			textWidth++;
		}

		drawTextOutline(statEfctName, pivotX + 45, pivotY + 6 + textOffsetY, textColor);

		int intDuration = std::ceil(myEfcts[i].duration);

		if (intDuration > 0)
		{
			setFont(fontType::pixel);
			setFontSize(17);
			int textWidth = queryTextWidth(std::to_wstring(intDuration));
			drawText(std::to_wstring(intDuration), lineStartX + 11 - textWidth, pivotY + 17, col::white);
		}

		if (myEfcts[i].effectType == statusEffectFlag::hungry)
		{
			float gaugeRatio = 0.0f;
			SDL_Color gaugeCol = col::white;

			if (fakeHunger < PLAYER_STARVE_CALORIE)
			{
				gaugeCol = lowCol::red;
				gaugeRatio = fakeHunger / static_cast<float>(PLAYER_STARVE_CALORIE);
			}
			else if (fakeHunger < PLAYER_VERY_HUNGRY_CALORIE)
			{
				gaugeCol = lowCol::orange;
				gaugeRatio = (fakeHunger - PLAYER_STARVE_CALORIE) / static_cast<float>(PLAYER_VERY_HUNGRY_CALORIE - PLAYER_STARVE_CALORIE);
			}
			else if (fakeHunger < PLAYER_HUNGRY_CALORIE)
			{
				gaugeCol = lowCol::yellow;
				gaugeRatio = (fakeHunger - PLAYER_VERY_HUNGRY_CALORIE) / static_cast<float>(PLAYER_HUNGRY_CALORIE - PLAYER_VERY_HUNGRY_CALORIE);
			}
			else
			{
				gaugeCol = col::green;
				gaugeRatio = (fakeHunger - PLAYER_HUNGRY_CALORIE) / static_cast<float>(PLAYER_MAX_CALORIE - PLAYER_HUNGRY_CALORIE);
			}

			int sprIndex = static_cast<int>((1.0f - gaugeRatio) * 53);
			sprIndex = std::max(0, std::min(53, sprIndex));

			SDL_SetTextureColorMod(spr::statusEffectGaugeCircle->getTexture(), gaugeCol.r, gaugeCol.g, gaugeCol.b);
			drawSprite(spr::statusEffectGaugeCircle, sprIndex, pivotX + textWidth - 9, pivotY + 2);
			SDL_SetTextureColorMod(spr::statusEffectGaugeCircle->getTexture(), 255, 255, 255);
		}
		else if (myEfcts[i].effectType == statusEffectFlag::dehydrated)
		{
			float gaugeRatio = 0.0f;
			SDL_Color gaugeCol = col::white;

			if (fakeThirst < PLAYER_DEHYDRATION_HYDRATION)
			{
				gaugeCol = lowCol::red;
				gaugeRatio = fakeThirst / static_cast<float>(PLAYER_DEHYDRATION_HYDRATION);
			}
			else if (fakeThirst < PLAYER_VERY_THIRSTY_HYDRATION)
			{
				gaugeCol = lowCol::orange;
				gaugeRatio = (fakeThirst - PLAYER_DEHYDRATION_HYDRATION) / static_cast<float>(PLAYER_VERY_THIRSTY_HYDRATION - PLAYER_DEHYDRATION_HYDRATION);
			}
			else if (fakeThirst < PLAYER_THIRSTY_HYDRATION)
			{
				gaugeCol = lowCol::yellow;
				gaugeRatio = (fakeThirst - PLAYER_VERY_THIRSTY_HYDRATION) / static_cast<float>(PLAYER_THIRSTY_HYDRATION - PLAYER_VERY_THIRSTY_HYDRATION);
			}
			else
			{
				gaugeCol = lowCol::skyBlue;
				gaugeRatio = (fakeThirst - PLAYER_THIRSTY_HYDRATION) / static_cast<float>(PLAYER_MAX_HYDRATION - PLAYER_THIRSTY_HYDRATION);
			}

			int sprIndex = static_cast<int>((1.0f - gaugeRatio) * 53);
			sprIndex = std::max(0, std::min(53, sprIndex));

			SDL_SetTextureColorMod(spr::statusEffectGaugeCircle->getTexture(), gaugeCol.r, gaugeCol.g, gaugeCol.b);
			drawSprite(spr::statusEffectGaugeCircle, sprIndex, pivotX + textWidth - 9, pivotY + 2);
			SDL_SetTextureColorMod(spr::statusEffectGaugeCircle->getTexture(), 255, 255, 255);
		}
		else if (myEfcts[i].effectType == statusEffectFlag::tired)
		{
			float gaugeRatio = 0.0f;
			SDL_Color gaugeCol = col::white;

			if (fakeFatigue < PLAYER_EXHAUSTED_FATIGUE)
			{
				gaugeCol = lowCol::red;
				gaugeRatio = fakeFatigue / static_cast<float>(PLAYER_EXHAUSTED_FATIGUE);
			}
			else if (fakeFatigue < PLAYER_VERY_TIRED_FATIGUE)
			{
				gaugeCol = lowCol::orange;
				gaugeRatio = (fakeFatigue - PLAYER_EXHAUSTED_FATIGUE) / static_cast<float>(PLAYER_VERY_TIRED_FATIGUE - PLAYER_EXHAUSTED_FATIGUE);
			}
			else if (fakeFatigue < PLAYER_TIRED_FATIGUE)
			{
				gaugeCol = lowCol::yellow;
				gaugeRatio = (fakeFatigue - PLAYER_VERY_TIRED_FATIGUE) / static_cast<float>(PLAYER_TIRED_FATIGUE - PLAYER_VERY_TIRED_FATIGUE);
			}
			else
			{
				gaugeCol = lowCol::green;
				gaugeRatio = (fakeFatigue - PLAYER_TIRED_FATIGUE) / static_cast<float>(PLAYER_MAX_FATIGUE - PLAYER_TIRED_FATIGUE);
			}

			int sprIndex = static_cast<int>((1.0f - gaugeRatio) * 73);
			sprIndex = std::max(0, std::min(73, sprIndex));

			SDL_SetTextureColorMod(spr::statusEffectGaugeCircle->getTexture(), gaugeCol.r, gaugeCol.g, gaugeCol.b);
			drawSprite(spr::statusEffectGaugeCircle, sprIndex, pivotX + textWidth - 9, pivotY + 2);
			SDL_SetTextureColorMod(spr::statusEffectGaugeCircle->getTexture(), 255, 255, 255);
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
				drawTextCenter(titleName, pivotX + 96, pivotY + 9);
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
					drawText(tgtPart.name, pivotX + 35, newPivotY + 6 + 17 * i);

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
				drawTextCenter(titleName, pivotX + 96, pivotY + 9);
				drawSpriteCenter(spr::itemset, getItemSprIndex(itemDex[TileFloor(tgtGrid.x, tgtGrid.y, PlayerZ())]), pivotX + 96 - queryTextWidth(titleName) / 2.0 - 11, pivotY + 10);


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
					drawSpriteCenter(spr::itemset, getItemSprIndex(tgtPart), pivotX + 24, newPivotY + 12 + 17 * i);

					//아이템 이름
					drawText(tgtPart.name, pivotX + 35, newPivotY + 6 + 17 * i);

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
	setFont(fontType::mainFontExtraBold);
	int pivotX = 14;
	int pivotY = 272;
	setFontSize(24);
	drawText(sysStr[212], pivotX, pivotY);
	drawLine(pivotX, pivotY + 36, pivotX + 110, pivotY + 36);
	for (int i = 0; i < 120; i++)
	{
		drawPoint(pivotX + 111 + i, pivotY + 36, col::white, 255 - 2 * i);
	}
	setFont(fontType::mainFont);
	drawRect({ pivotX + 1, pivotY + 49, 14, 14 }, col::white);
	setFontSize(16);
	int elapsedDay = getElapsedDays();
	std::wstring questStr = sysStr[213] + L"  (";
	questStr += std::to_wstring(elapsedDay);
	questStr += L"/100)";
	drawText(questStr, pivotX + 20, pivotY + 46);
}