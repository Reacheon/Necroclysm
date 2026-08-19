#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

import std;
import util;
import constVar;
import globalVar;
import displayLoader;
import fontLoader;
import textureLoader;
import dataLoader;
import scriptLoader;
import textureVar;
import renderUI;
import renderTile;
import renderSticker;
import renderLog;
import renderWeather;
import renderFPS;
import Player;
import World;
import stepEvent;
import turnCycleLoop;
import startArea;
import initCoordTransform;
import nervedriveFilter;
#define SDL_GESTURE_IMPLEMENTATION 1
#include "SDL_gesture.h"

int main(int argc, char** argv)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//"korean"(Windows 전용) 대신 ""(시스템 기본)을 사용하여 크로스플랫폼 호환:
	//  Windows 한국어: ko_KR/CP949, Linux/macOS: LC_ALL 환경변수 기준.
	try { std::locale::global(std::locale("")); } catch (...) { /*최소환경 폴백*/ }
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
	TTF_Init();

	int numThreads = std::thread::hardware_concurrency();
	if (numThreads == 0) numThreads = 4;
	dbgPrt(L"이 컴퓨터의 스레드 숫자는 %d개이다.\n", numThreads);
	threadPoolPtr = std::make_unique<ThreadPool>(numThreads);
	SDL_AddGamepadMappingsFromFile("gamecontrollerdb.txt");

	initRandomEngine();//게임 전체에서 공용으로 사용할 단일 시드 확정
	initCircle();
	displayLoader();//실행시킨 디바이스의 해상도에 따라 게임의 해상도를 조정
	textureLoader(); //프로그램에 사용될 텍스쳐들을 image 폴더에서 로드
	fontLoader(); //프로그램에 사용될 폰트들을 언어에 맞게 로드
	dataLoader();
	initNanoTimer();
	initCoordTransform();
	startArea();//스타트 세팅

	if (Gesture_Init() == 0)
	{
		gestureInitialized = true;
		dbgPrt(L"SDL_gesture 초기화 성공\n");
	}
	else 
	{
		gestureInitialized = false;
		dbgPrt(L"SDL_gesture 초기화 실패: %s\n", SDL_GetError());
	}

	scriptLoader();

	//int currentWidth, currentHeight;
	//SDL_GetWindowSize(window, &currentWidth, &currentHeight); // 현재 윈도우 크기를 가져옵니다.
	//// 윈도우 크기를 2배로 설정합니다.
	//SDL_SetWindowSize(window, currentWidth * 0.5, currentHeight * 0.5);
	//SDL_SetRenderScale(renderer, 0.5f, 0.5f);
	int fpsTimeStack = 0;
	int fpsFrame = 0;
	int fps = 60;
	bool hasInitMinimap = false; //실행 초기에 미니맵이 생성되지않는 버그를 고치기위해(startArea에 추가하면 왜 안되는거지?)
	while (!quit)
	{
		std::int64_t loopStart = getNanoTimer();

		if (frameTarget != nullptr) SDL_SetRenderTarget(renderer, frameTarget);//이번 프레임의 모든 그리기를 논리 해상도 렌더타겟으로

		//■Timer 변수
		if (timer::cursorHightlight < 23) { timer::cursorHightlight++; }
		else { timer::cursorHightlight = 0; }
		if (timer::timer600 != 599) { timer::timer600++; }
		else { timer::timer600 = 0; }

		dur::turnCycle = turnCycleLoop();
		dur::stepEvent = stepEvent();
		//▲이동 관련 코드는 전부 이 위에 적혀야 함. 아니면 화면이 흔들림▲

		if (cameraFix == true)
		{
			cameraX = (PlayerPtr)->getX();
			cameraY = (PlayerPtr)->getY();
		}

		//▼화면 렌더링 관련 코드는 이  아래에 적혀야 함▼
		// nervedriveOn이면 월드 렌더를 오프스크린 RT로 우회 → 초록 틴트 후 블릿 → 플레이어 덧그림.
		// 비활성일 때는 아무 효과 없이 평소처럼 진행됨.
		const bool nervedriveFilterActive = nervedriveFilter::beginWorldPass();
		dur::renderTile = renderTile();
		dur::renderWeather = renderWeather();
		dur::renderSticker = renderSticker(cameraX, cameraY);
		if (nervedriveFilterActive)
		{
			nervedriveFilter::endWorldPassAndBlit();
			PlayerPtr->drawSelf();
		}
		dur::renderUI = renderUI();
		dur::renderLog = renderLog(renderer);
		dur::totalDelay = getNanoTimer() - loopStart;
		const int constDelay = 16000000;
		std::int64_t delayTime = constDelay - dur::totalDelay;
		if (delayTime >= constDelay) delayTime = constDelay; // 만약 루프 시간이 음수(오류)가 나왔을 경우
		else if (delayTime < 0)
		{
			//dbgPrt(L"루프가 16ms를 넘는 과부하가 발생했다.");
			//if (turnCycle == turn::playerInput) dbgPrt(L"현재 턴은 playerInput이다.	");
			//else if (turnCycle == turn::playerAnime) dbgPrt(L"현재 턴은 playerAnime이다.	");
			//else if (turnCycle == turn::monsterAI) dbgPrt(L"현재 턴은 monsterAI이다.	");
			//else dbgPrt(L"현재 턴은 monsterAnime이다.	");
			//dbgPrt(L"turn : %ls ms, ", decimalCutter(dur::turnCycle / 1000000.0, 5).c_str());
			//dbgPrt(L"tile : %ls ms, ", decimalCutter(dur::renderTile / 1000000.0, 5).c_str());
			//dbgPrt(L"UI : %ls ms\n", decimalCutter(dur::renderUI / 1000000.0, 5).c_str());
			delayTime = 0;
		}
		SDL_Delay(delayTime/1000000);//FPS60일 때 16, 루프 시간이 길어질 경우 그 시간을 측정해서 슬립 시간을 줄여줌 최대 16ms
		//renderFPS(getNanoTimer() - loopStart);
		//렌더타겟을 창 크기로 단일 블릿
		if (frameTarget != nullptr)
		{
			SDL_SetRenderTarget(renderer, nullptr);
			const SDL_FRect dst = { 0.0f, 0.0f, (float)cameraW, (float)cameraH };
			SDL_RenderTexture(renderer, frameTarget, nullptr, &dst);
		}
		SDL_RenderPresent(renderer);

		if (hasInitMinimap == false && PlayerPtr != nullptr)
		{
			PlayerPtr->updateMinimap();
			hasInitMinimap = true;
		}
	}


	threadPoolPtr.reset();
	if (gestureInitialized) 
	{
		Gesture_Quit();
		gestureInitialized = false;
	}
	TTF_Quit();
	SDL_Quit();
	//_CrtDumpMemoryLeaks();

	return 0;
};
