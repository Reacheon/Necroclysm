﻿#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

export module turnCycleLoop;

import std;
import util;
import globalVar;
import wrapVar;
import Entity;
import World;
import Player;
import Monster;
import Vehicle;
import mouseInput;
import keyboardInput;
import gamepadInput;
import AI;
import turnWait;
import dirToXY;
import globalTime;
import log;
import GameOver;
import GUI;
import Sleep;

extern "C" 
{
	typedef struct 
	{
		Uint32 type;
		Uint32 reserved;
		Uint64 timestamp;
		SDL_TouchID touchID;
		float dTheta;
		float dDist;
		float x;
		float y;
		Uint16 numFingers;
		Uint16 padding;
	} Gesture_MultiGestureEvent;

#define GESTURE_MULTIGESTURE 0x802
}

static bool firstPlayerInput = true, firstPlayerAnime = true, firstMonsterAI = true, firstMonsterAnime = true;

__int64 playerInputTurn(), animationTurn(), entityAITurn(), propTurn();

export __int64 turnCycleLoop()
{
	__int64 timeStampStart = getNanoTimer();

	if (dtClickStack != -1)
	{
		switch (option::inputMethod)
		{
		case input::mouse:
			dtClickStack = SDL_GetTicks() - dtClickStackStart;
			break;
		case input::touch:
			dtClickStack = SDL_GetTicks() - dtClickStackStart;
			break;
		}
	}

	__int64 playerInputTime = 0, animationTime = 0, entityAITime = 0;
	if (turnCycle == turn::playerInput)
	{
		playerInputTime = playerInputTurn();
		//std::wprintf(L"[Turn:playerInput] %ls ns\n", decimalCutter(playerInputTime / 1000000.0, 5).c_str());
	}
	else if (turnCycle == turn::playerAnime || turnCycle == turn::monsterAnime)
	{
		animationTime = animationTurn();
		//std::wprintf(L"[Turn:animation] %ls ns\n", decimalCutter(animationTime / 1000000.0, 5).c_str());
	}
	else if (turnCycle == turn::monsterAI)
	{
		entityAITime = entityAITurn();
		//std::wprintf(L"[Turn:entityAI] %ls ns\n", decimalCutter(entityAITime / 1000000.0, 5).c_str());
	}
	else errorBox(L"Unknown turnCycle executed in turyCycleLoop.ixx");

	return (getNanoTimer() - timeStampStart);
};


void applyZoom(float newZoom, Point2 zoomCenter) 
{
	int worldX = cameraX + (zoomCenter.x - cameraW / 2) / zoomScale;
	int worldY = cameraY + (zoomCenter.y - cameraH / 2) / zoomScale;

	zoomScale = newZoom;

	cameraX = worldX - (zoomCenter.x - cameraW / 2) / zoomScale;
	cameraY = worldY - (zoomCenter.y - cameraH / 2) / zoomScale;

	prt(L"줌: %.0fx (위치: %d, %d)\n", zoomScale, zoomCenter.x, zoomCenter.y);
}

void handlePinchGesture(const SDL_Event& event) 
{
	if (!gestureInitialized) return;

	if (event.type == GESTURE_MULTIGESTURE)
	{
		const Gesture_MultiGestureEvent& mgesture = reinterpret_cast<const Gesture_MultiGestureEvent&>(event);

		if (mgesture.numFingers == 2)
		{
			if (!isPinchActive) 
			{
				isPinchActive = true;
				pinchAccumulator = 0.0f;
				pinchStartPos.x = static_cast<int>(mgesture.x * cameraW);
				pinchStartPos.y = static_cast<int>(mgesture.y * cameraH);
				deactClickUp = true;
				deactHold = true;
				clickStartTime = std::numeric_limits<__int64>::max();//clickHold 무효화 용도
				return;
			}

			pinchAccumulator += mgesture.dDist;
			const float threshold = 0.05f;
			if (pinchAccumulator > threshold) 
			{
				if (zoomScale < 5.0f) applyZoom(zoomScale + 1.0f, pinchStartPos);
				pinchAccumulator = 0.0f;

			}
			else if (pinchAccumulator < -threshold) 
			{
				if (zoomScale > 1.0f) applyZoom(zoomScale - 1.0f, pinchStartPos);
				pinchAccumulator = 0.0f;
			}
		}
	}
}

void snapToNearestZoomLevel() {
	const float zoomLevels[] = { 1.0f,  2.0f, 3.0f, 4.0f, 5.0f };
	const int numLevels = sizeof(zoomLevels) / sizeof(zoomLevels[0]);

	float closestLevel = zoomLevels[0];
	float minDistance = std::abs(zoomScale - zoomLevels[0]);

	for (int i = 1; i < numLevels; i++) {
		float distance = std::abs(zoomScale - zoomLevels[i]);
		if (distance < minDistance) {
			minDistance = distance;
			closestLevel = zoomLevels[i];
		}
	}

	zoomScale = closestLevel;
	prt(L"줌 레벨 스냅: %.1f\n", zoomScale);
}


bool handleTouchEnd() {
	activeTouchCount--; // 터치 카운트 감소

	if (activeTouchCount < 0) activeTouchCount = 0; // 안전장치

	if (isPinchActive) {
		// 모든 터치가 끝났을 때만 핀치 종료
		if (activeTouchCount == 0) {
			isPinchActive = false;
			pinchAccumulator = 0.0f;
			deactClickUp = false;
			deactHold = false;
			prt(L"핀치 제스처 종료\n");
		}
		return true; // 핀치 상태였음
	}
	return false; // 핀치가 아니었음
}

__int64 playerInputTurn()
{
	__int64 timeStampStart = getNanoTimer();

	//턴 시작
	{
		if (hunger <= 0)
		{
			if (GUI::getLastGUI() == Sleep::ins())
			{
				coTurnSkip = false;
				delete Sleep::ins();
			}
			GameOver::create(L"극심한 영양실조로 사망했다.");
			PlayerPtr->deactAStarDst();
			aStarTrail.clear();
		}

		if (thirst <= 0)
		{
			if (GUI::getLastGUI() == Sleep::ins())
			{
				coTurnSkip = false;
				delete Sleep::ins();
			}
			GameOver::create(L"극심한 탈수 증세로 사망했다.");
			PlayerPtr->deactAStarDst();
			aStarTrail.clear();
		}

		if (fatigue <= 0)
		{
			GameOver::create(L"극심한 피로로 사망했다.");
			PlayerPtr->deactAStarDst();
			aStarTrail.clear();
		}
	}


	if (firstPlayerInput)
	{
		firstPlayerInput = false;
		firstPlayerAnime = true;
		firstMonsterAI = true;
		firstMonsterAnime = true;

		//prt(col::cyan, L"[턴 페이즈 1] 플레이어 입력\n");

		//플레이어턴에 이전에 사용된 큐들 지우는 코드, 활성화하면 버그는 줄어들지만 조작감이 나빠진다...
		//SDL_Event tempEvent;
		//while (SDL_PollEvent(&tempEvent))
		//{
		//	// 다른 턴에 쌓인 큐에 있는 모든 이벤트를 읽어서 버림
		//}

		if (coTurnSkip&&GameOver::ins()==nullptr)
		{
			//prt(L"메인 함수 코루틴 재실행\n");
			coTurnSkip = false;
			(*coFunc).run();
		}

		if (PlayerPtr->getHasAStarDst() == false) isPlayerMoving = false;
	}

	if (PlayerPtr->getHasAStarDst() == true) //자동이동 기능이 활성화되었을 경우
	{
		int dx = PlayerPtr->getAStarDstX() - PlayerX();
		int dy = PlayerPtr->getAStarDstY() - PlayerY();

		if (dx != 0 || dy != 0)
		{
			//주변 10칸으로 이동 가능한 타일 배열 계산
			std::unordered_set<Point2,Point2::Hash> walkableTile;
			for (int i = PlayerX() - 20; i <= PlayerX() + 20; i++)
			{
				for (int j = PlayerY() - 20; j <= PlayerY() + 20; j++)
				{
					if (isWalkable({ i, j, PlayerZ() })) walkableTile.insert({ i,j });
					//else prt(L"(%d,%d) 타일은 이동 불가능한 타일이다.\n",i,j);
				}
			}

			if (walkableTile.find({ PlayerPtr->getAStarDstX(), PlayerPtr->getAStarDstY() }) != walkableTile.end())//목적지가 이동 가능할 경우
			{
				aStarTrail = aStar(walkableTile, PlayerX(), PlayerY(), PlayerPtr->getAStarDstX(), PlayerPtr->getAStarDstY());
				if (aStarTrail.size() >= 2)
				{
					PlayerPtr->startMove(coord2Dir(aStarTrail[1].x - aStarTrail[0].x, aStarTrail[1].y - aStarTrail[0].y));
					aStarTrail.erase(aStarTrail.begin());
				}
				else
				{
					PlayerPtr->deactAStarDst();
					isPlayerMoving = false;
				}
			}
			else
			{
				PlayerPtr->deactAStarDst();
				isPlayerMoving = false;
			}
		}
		else
		{
			PlayerPtr->deactAStarDst();
			isPlayerMoving = false;
		}
	}
	else
	{
		bool first = true;
		while (SDL_PollEvent(&event))
		{
			if (first)
			{
				switch (event.type)
				{
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					if (option::inputMethod != input::gamepad)
					{
						updateLog(L"Switched to gamepad mode.");
						option::inputMethod = input::gamepad;
					}
					gamepadBtnDown();
					break;
				case SDL_EVENT_GAMEPAD_BUTTON_UP:
					if (option::inputMethod != input::gamepad)
					{
						updateLog(L"Switched to gamepad mode.");
						option::inputMethod = input::gamepad;
					}
					gamepadBtnUp();
					break;
				case SDL_EVENT_GAMEPAD_AXIS_MOTION:
					gamepadBtnMotion();
				case SDL_EVENT_WINDOW_RESIZED:
					SDL_SetRenderLogicalPresentation(renderer,
						434,
						244,
						SDL_LOGICAL_PRESENTATION_LETTERBOX);
					break;
				case SDL_EVENT_QUIT:
					//IMG_Quit();
					TTF_Quit();
					SDL_Quit();
					exit(0);
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (option::inputMethod == input::mouse) { clickDown(); }
					break;
				case SDL_EVENT_FINGER_DOWN:
					if (option::inputMethod == input::touch) 
					{
                        touchStartGrid = getAbsMouseGrid();
						activeTouchCount++;
						if (!isPinchActive) clickDown();
					}
					break;
				case SDL_EVENT_MOUSE_MOTION:
					if (option::inputMethod == input::mouse) { clickMotion(); }
					break;
				case SDL_EVENT_FINGER_MOTION:
					if (option::inputMethod == input::touch && !isPinchActive &&  // 핀치 중이 아닐 때만
						(std::abs(event.tfinger.dx) * cameraW > 5 || std::abs(event.tfinger.dy) * cameraH > 5))
					{
						clickMotion();
					}
					break;
				case SDL_EVENT_MOUSE_BUTTON_UP:
					if (option::inputMethod == input::mouse)
					{
						if (event.button.button == SDL_BUTTON_LEFT) clickUp();
						else if (event.button.button == SDL_BUTTON_RIGHT) clickRight();
					}
					break;
				case SDL_EVENT_FINGER_UP:
					if (option::inputMethod == input::touch)
					{
						bool wasPinchActive = handleTouchEnd(); // 핀치 종료 여부 저장
						if (!wasPinchActive) { // 핀치가 아니었을 때만 클릭 처리
							clickUp();
						}
					}
					break;
				case SDL_EVENT_MOUSE_WHEEL:
					mouseWheel();

					break;
				case SDL_EVENT_KEY_DOWN:
					if (exInput == true && event.key.key == UNI::BACKSPACE)
					{
						prt(L"백스페이스 키 입력됨\n");
						if (!exInputEditing)
						{
							if (exInputCursor != 0) { --exInputCursor; }
							exInputText.erase(exInputCursor, 1);
						}
					}
					else
					{
						keyboardBtnDown();
					}
					break;
				case SDL_EVENT_KEY_UP:
					keyboardBtnUp();
					break;
				case SDL_EVENT_TEXT_INPUT: //텍스트가 완전히 입력되었을 때의 이벤트(한글이 완성되었을 때)
				{
					if (exInput == true)
					{
						prt(L"완성 이벤트 실행됨\n");
						std::wstring singleChar = L"";
						singleChar += utf8Decoder(event.text.text[0], event.text.text[1], event.text.text[2], event.text.text[3]);
						if (exInputCursor > exInputText.size()) exInputCursor = exInputText.size();
						if (!singleChar.empty() && singleChar[0] != L'\0')
						{
							exInputText.insert(exInputCursor, singleChar);
							exInputCursor += singleChar.length();
						}
						exInputEditing = false;
						if (exInputText.size() > EX_INPUT_TEXT_MAX)
						{
							exInputText = exInputText.substr(0, EX_INPUT_TEXT_MAX);
							exInputCursor = EX_INPUT_TEXT_MAX;
						}
					}
					break;
				}
				case SDL_EVENT_TEXT_EDITING: //텍스트가 입력 되었을 때의 이벤트(미완성도 실행됨, TEXTINPUT보다 더 큰 개념)
				{
					if (exInput == true)
					{
						prt(L"편집 이벤트 입력됨\n");
						std::wstring singleChar = L"";
						singleChar += utf8Decoder(event.text.text[0], event.text.text[1], event.text.text[2], event.text.text[3]);
						exInputText.replace(exInputCursor, 1, singleChar);
						if (exInputText[exInputCursor] == 0) { exInputEditing = false; }
						else { exInputEditing = true; }
						if (exInputText.size() > EX_INPUT_TEXT_MAX)
						{
							exInputText = exInputText.substr(0, EX_INPUT_TEXT_MAX);
							exInputCursor = EX_INPUT_TEXT_MAX;
						}
					}
					break;
				}
				case GESTURE_MULTIGESTURE:
					if (option::inputMethod == input::touch) handlePinchGesture(event);
					break;



				}

				first = true;
			}
			
		}

	}

	return (getNanoTimer() - timeStampStart);
}

__int64 animationTurn()
{
	//AniUSet
	//Ani 클래스를 상속받은 클래스는 aniUSet이라는 unordered set에 ID가 추가될 경우
	//루프마다 그 ID의 runAnimation 메소드를 실행시킴. 만약 runAnimation이 true를 반환할 경우
	//aniUSet에 있는 그 ID를 지워버리고 만약 모든 애니메이션을 실행시켰을 경우 turnCycle을 2(Entity의 AI 연산)으로 넘김
	__int64 timeStampStart = getNanoTimer();
	if (turnCycle == turn::playerAnime)
	{
		if (firstPlayerAnime)
		{
			firstPlayerInput = true;
			firstPlayerAnime = false;
			firstMonsterAI = true;
			firstMonsterAnime = true;

			//prt(col::cyan, L"[턴 페이즈 2] 플레이어 애니메이션\n");
		}
	}
	else if (turnCycle == turn::monsterAnime)
	{
		if (firstMonsterAnime)
		{
			firstPlayerInput = true;
			firstPlayerAnime = true;
			firstMonsterAI = true;
			firstMonsterAnime = false;

			//prt(col::cyan, L"[턴 페이즈 4] 몬스터 애니메이션\n");
		}
	}

	auto endAnimeTurn = [=]()
		{
			if (turnCycle == turn::playerAnime)
			{
				std::vector<Entity*> entityList = (World::ins())->getActiveEntityList();
				std::vector<Vehicle*> vehList = (World::ins())->getActiveVehicleList();

				int startHour = getHour();
				int startMin = getMin();
				int startSec = getSec();

				addTimeTurn(timeGift);

				int endHour = getHour();
				int endMin = getMin();
				int endSec = getSec();

				int minutesPassed = 0;

				int startTotalSeconds = startHour * 3600 + startMin * 60 + startSec;
				int endTotalSeconds = endHour * 3600 + endMin * 60 + endSec;

				if (endTotalSeconds < startTotalSeconds) endTotalSeconds += 24 * 3600;

				if (startSec == 0) minutesPassed = (endTotalSeconds - startTotalSeconds) / 60;
				else 
				{
					int remainingTime = endTotalSeconds - startTotalSeconds;
					if (remainingTime >= (60 - startSec)) minutesPassed = 1 + (remainingTime - (60 - startSec)) / 60;
				}
				for (int i = 0; i < minutesPassed; i++) propTurn();

				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				for (auto ePtr : entityList)
				{
					if (ePtr != (PlayerPtr)) ((Monster*)ePtr)->addTurnResource(timeGift);
				}
				for (auto vPtr : vehList) vPtr->addTurnResource(timeGift);
				timeGift = 0;
				turnCycle = turn::monsterAI;
			}
			else if (turnCycle == turn::monsterAnime) turnCycle = turn::monsterAI;
		};


	if (coTurnSkip)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_KEY_DOWN && (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_TAB))
			{

				// 즉시 취소 처리
				//coTurnSkip = false;
				std::wprintf(L"코루틴이 즉시 취소되었습니다.\n");
			}
		}
	}

	int aniSize = aniUSet.size();
	if (aniUSet.size() > 0)
	{
		//aniUSet에 있는 모든 인스턴스를 체크해서 애니메이션을 실행시킴. 
		for (auto it = aniUSet.begin(); it != aniUSet.end();)
		{
			//prt(L"aniUSet의 사이즈는 %d이다.\n",aniUSet.size());
			bool loopBreak = false;
			if ((*it)->isDictator()) { loopBreak = true; }

			if ((*it)->runAnimation(false) == true)//종료하는 애니메이션을 실행했을 경우
			{
				aniUSet.erase(it++);//it가 가리키는 요소를 제거하고 후위연산자로 늘려버려서 반복자가 무효화되지않도록 함
				if (aniUSet.size() == 0)
				{
					endAnimeTurn();
					//prt(L"runAnimation 종료, 걸린 시간은 %ls이다.\n", decimalCutter((getNanoTimer()- timeStampStart) / 1000000.0, 5).c_str());
				}
			}
			else it++;

			if (loopBreak) break;
			else if (it != aniUSet.end())//설령 Dictator가 아니라도 다음 Ani가 Dictator면 미리 종료함, 다음은 Dictator만의 독무대를 위해 
			{
				if ((*it)->isDictator()) break;
			}

		}
	}
	//사용 가능한 애니메이션이 하나도 없는 경우
	else endAnimeTurn();

	return (getNanoTimer() - timeStampStart);
}

__int64 entityAITurn()
{
	__int64 timeStampStart = getNanoTimer();
	if (firstMonsterAI)
	{
		firstPlayerInput = true;
		firstPlayerAnime = true;
		firstMonsterAI = false;
		firstMonsterAnime = true;
		//prt(col::cyan, L"[턴 페이즈 3] 몬스터 AI\n");
	}

	bool endMonsterTurn = true;

	std::vector<Entity*> entityList = (World::ins())->getActiveEntityList();
	std::vector<Vehicle*> vehList = (World::ins())->getActiveVehicleList();

	for (auto vPtr : vehList)
	{
		if (vPtr->runAI() == false) endMonsterTurn = false;
	}

	for (auto ePtr : entityList)
	{
		if (ePtr != (PlayerPtr))
		{
			if (((Monster*)ePtr)->runAI() == false) endMonsterTurn = false;
		}
	}

	if (endMonsterTurn == true)
	{
		turnCycle = turn::playerInput;

		//플레이어 스테미나 회복


		//턴사이클 모두 종료
		for (auto ePtr : entityList)
		{
			for (auto it = ePtr->entityInfo.statusEffectVec.begin(); it != ePtr->entityInfo.statusEffectVec.end();)
			{
				if (it->duration > 0) it->duration--;

				if (it->duration == 0) it = ePtr->entityInfo.statusEffectVec.erase(it);
				else it++;
			}
		}
	}
	else { turnCycle = turn::monsterAnime; }


	return (getNanoTimer() - timeStampStart);
}

__int64 propTurn()
{
    //updateLog(L"Prop turn started.");
	return 0;
}