#include <SDL3/SDL.h>
export module Sleep;
import std;
import util;
import GUI;
import globalVar;
import constVar;
import wrapVar;
import textureVar;
import drawText;
import drawSprite;
import checkCursor;
import drawWindow;
import Msg;
import World;
import log;
import Player;
import turnWait;

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export class Sleep : public GUI
{
private:
	inline static Sleep* ptr = nullptr;
	SDL_Rect sleepBase;

	// 수면 관련 변수
	bool isTryingToSleep = true;  // 수면 시도 중
	bool isAsleep = false;        // 실제로 잠든 상태
	int sleepTime = 0;
	int sleepDuration = 480; // 8시간 (60분 * 8)
	bool negateMonster = false;

	// 추가: 피로도 회복 관련 변수
	float fatigueRecoveryPerMinute = 0.01f; // 분당 피로도 회복량
	float initialFatigue = 0.0f; // 수면 시작시 피로도

	// 수면 확률 계산
	float getSleepProbability()
	{
		float fatigueRatio = fatigue / (float)PLAYER_MAX_FATIGUE;

		// 피로도가 낮을수록 잠들 확률이 높아짐
		// 0% 피로도 = 9.5% 확률, 100% 피로도 = 0.01% 확률
		float probability = 0.095f - (fatigueRatio * 0.09f);
		return myMax(0.0001f, myMin(0.095f, probability));
	}

public:
	Sleep() : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;
		changeXY(cameraW / 2, cameraH / 2, true);
		tabType = tabFlag::closeWin;
		PlayerPtr->setSpriteIndex(charSprIndex::CRAWL);
		PlayerPtr->entityInfo.isEyesHalf = true;
		PlayerPtr->entityInfo.isEyesClose = false;


		// 수면 시도 시작
		CORO(executeSleep());
	}
	~Sleep()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
		PlayerPtr->entityInfo.walkMode = walkFlag::walk;
		PlayerPtr->setSpriteIndex(charSprIndex::WALK);
		PlayerPtr->entityInfo.isEyesHalf = false;
		PlayerPtr->entityInfo.isEyesClose = false;
	}
	static Sleep* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		sleepBase = { 0, 0, 650, 376 };
		if (center == false)
		{
			sleepBase.x += inputX;
			sleepBase.y += inputY;
		}
		else
		{
			sleepBase.x += inputX - sleepBase.w / 2;
			sleepBase.y += inputY - sleepBase.h / 2;
		}
		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - sleepBase.w / 2;
			y = inputY - sleepBase.h / 2;
		}
	}

	void drawGUI()
	{
		// 수면 시도 중이거나 실제로 수면 중일 때 상단 툴팁 표시
		if ((isTryingToSleep || isAsleep) && getStateDraw() == false)
		{
			SDL_Rect tooltipBox;
			if (isTryingToSleep)
			{
				// 수면 시도 중일 때는 작은 박스
				tooltipBox = { cameraW / 2 - 75, 50, 180, 46 };
			}
			else
			{
				// 수면 중일 때는 큰 박스 (게이지 표시용)
				tooltipBox = { cameraW / 2 - 75, 50, 180, 46 };
			}
			drawWindow(&tooltipBox);
			setZoom(1.0);

			// 로딩 애니메이션
			int markerIndex = 0;
			if (Msg::ins() == nullptr) markerIndex = (SDL_GetTicks() / 64) % 12;
			drawSprite(spr::loadingAnime, markerIndex, tooltipBox.x + tooltipBox.w / 2 - 78, tooltipBox.y + 6);

			if (isTryingToSleep)
			{
				// 수면 시도 중 텍스트
				int dotCount = (SDL_GetTicks() / 800) % 4;
				std::wstring sleepText = sysStr[214];
				for (int i = 0; i < dotCount; i++) sleepText += L".";
				setFontSize(12);
				renderTextCenter(sleepText.c_str(), tooltipBox.x + tooltipBox.w / 2 + 18, tooltipBox.y + 14);

				// 수면 확률 표시
				setFontSize(8);
				float probability = getSleepProbability();
				std::wstring probText = sysStr[229] + L": " + std::to_wstring((int)(probability * 100)) + L"%";
				renderTextCenter(probText, tooltipBox.x + tooltipBox.w / 2 + 18, tooltipBox.y + 30);
			}
			else if (isAsleep)
			{
				// 수면 중 텍스트 (점 애니메이션 추가)
				int dotCount = (SDL_GetTicks() / 800) % 4;
				std::wstring sleepText = sysStr[215];
				for (int i = 0; i < dotCount; i++) sleepText += L".";
				setFontSize(12);
				renderTextCenter(sleepText.c_str(), tooltipBox.x + 113, tooltipBox.y + 12);

				// 진행 바
				SDL_Rect sleepGauge = { tooltipBox.x + 51, tooltipBox.y + 23, 125, 7 };
				drawRect(sleepGauge, col::white);

				SDL_Rect sleepInGauge = { sleepGauge.x + 2, sleepGauge.y + 2, 121, 3 };
				sleepInGauge.w = 121 * ((float)sleepTime / (float)sleepDuration);
				drawFillRect(sleepInGauge, col::white);

				setFontSize(8);
				int remainingMinutes = sleepDuration - sleepTime;
				int hours = remainingMinutes / 60;
				int minutes = remainingMinutes % 60;

				std::wstring progressText = sysStr[216];
				progressText = replaceStr(progressText, L"(%hour)", std::to_wstring(hours));
				progressText = replaceStr(progressText, L"(%min)", std::to_wstring(minutes));
				progressText += L" ( ";
				progressText += std::to_wstring((int)(((float)sleepTime * 100.0 / (float)sleepDuration)));
				progressText += L"% )";

				renderTextCenter(progressText, sleepGauge.x + sleepGauge.w / 2, sleepGauge.y + sleepGauge.h + 7);

				int yOffsetSeq[8] = { 0,0,0,0,0,1,2,1 };
				int animIndex = (SDL_GetTicks() / 250) % 8;   // 250ms마다 한 스텝 → 1.2초에 한 주기
				int yOffset = yOffsetSeq[animIndex];
				setZoom(zoomScale);
				drawSpriteCenter(
					spr::thoughtBubble, 0,
					cameraW / 2 + 14.0f * zoomScale,
					cameraH / 2 - 7.0f * zoomScale - yOffset * zoomScale);
				setZoom(1.0);
			}
		}
	}

	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }
		if (checkCursor(&tab))
		{
			// 수면 시도 중에만 중단 가능
			if (isTryingToSleep)
			{
				updateLog(L"Sleep attempt cancelled.");
				coTurnSkip = false;
				close(aniFlag::null);
			}
			// 실제로 잠든 상태에서는 중단 불가능 (아무것도 하지 않음)
		}
	}

	Corouter executeSleep()
	{

		// 1단계: 수면 시도
		isTryingToSleep = true;
		isAsleep = false;
		deactDraw(); // 창을 숨기고 상단 툴팁만 표시

		while (isTryingToSleep)
		{

			// 주변 적 체크 (수면 시도 중에만)
			if (negateMonster == false)
			{
				for (int x = PlayerX() - 1; x <= PlayerX() + 1; x++)
				{
					for (int y = PlayerY() - 1; y <= PlayerY() + 1; y++)
					{
						if (!(x == PlayerX() && y == PlayerY()))
							if (TileFov(x, y, PlayerZ()) == fovFlag::white)
								if (TileEntity(x, y, PlayerZ()) != nullptr)
								{
									//경고, 주변에 적이 있습니다. 계속 수면을 시도하시겠습니까?, 네,아니오,무시
									new Msg(msgFlag::normal, sysStr[306],sysStr[323], { sysStr[36],sysStr[37],sysStr[311] });
									co_await std::suspend_always();
									if (coAnswer == sysStr[36]) goto sleepTryLoopEnd;
									else if (coAnswer == sysStr[311])
									{
										negateMonster = true;
										goto sleepTryLoopEnd;
									}
									else
									{
										updateLog(sysStr[324]);//수면 시도를 취소하였다.
										coTurnSkip = false;
										close(aniFlag::null);
										co_return;
									}
								}
					}
				}
			}

		sleepTryLoopEnd:

			turnWait(1.0);
			coTurnSkip = true;

			co_await std::suspend_always();

			// 수면 확률 체크
			float sleepChance = getSleepProbability();
			float roll = randomRange(0, 10000) / 10000.0f;

			if (roll < sleepChance)
			{
				updateLog(L"You fell asleep.");
				isTryingToSleep = false;
				isAsleep = true;
				PlayerPtr->entityInfo.isEyesClose = true; // 눈을 감음

				// 피로도 회복 관련 초기화
				initialFatigue = fatigue;
				float targetFatigue = PLAYER_MAX_FATIGUE; // 완전 회복 목표
				fatigueRecoveryPerMinute = (targetFatigue - initialFatigue) / sleepDuration;

				break;
			}
		}

		// 2단계: 실제 수면
		sleepTime = 0;

		while (sleepTime < sleepDuration && isAsleep)
		{

			turnWait(1.0);
			coTurnSkip = true;

			co_await std::suspend_always();

			sleepTime++;

			fatigue += FATIGUE_SPEED;
			fatigue += fatigueRecoveryPerMinute;
			if (fatigue > PLAYER_MAX_FATIGUE) fatigue = PLAYER_MAX_FATIGUE;

		}

		// 수면 완료
		if (isAsleep)
		{
			updateLog(sysStr[322]);//숙면을 취했습니다. 피로가 완전히 회복되었습니다. 
		}

		isAsleep = false;
		close(aniFlag::null);
	}

};