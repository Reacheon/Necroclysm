#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

import std;
import util;
import constVar;
import globalVar;
import displayLoader;
import fontLoader;
import textureLoader;
import dataLoader;
import textureVar;
import renderUI;
import renderTile;
import renderSticker;
import renderLog;
import renderWeather;
import renderFPS;
import Player;
import stepEvent;
import log;
import turnCycleLoop;
import startSetting;
import initCoordTransform;

int main(int argc, char** argv)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	std::locale::global(std::locale("korean"));
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);
	Mix_OpenAudio(22050, AUDIO_S16, 2, 4096);
	displayLoader();//실행시킨 디바이스의 해상도에 따라 게임의 해상도를 조정
	textureLoader(); //프로그램에 사용될 텍스쳐들을 image 폴더에서 로드
	fontLoader(); //프로그램에 사용될 폰트들을 언어에 맞게 로드
	dataLoader();
	initLog();
	initNanoTimer();
	initCoordTransform();
	startSetting();//스타트 세팅

	//int currentWidth, currentHeight;
	//SDL_GetWindowSize(window, &currentWidth, &currentHeight); // 현재 윈도우 크기를 가져옵니다.
	//// 윈도우 크기를 2배로 설정합니다.
	//SDL_SetWindowSize(window, currentWidth * 0.5, currentHeight * 0.5);
	//SDL_RenderSetScale(renderer, 0.5f, 0.5f);

	int fpsTimeStack = 0;
	int fpsFrame = 0;
	int fps = 60;
	while (!quit)
	{
		__int64 loopStart = getNanoTimer();
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
			cameraX = (Player::ins())->getX();
			cameraY = (Player::ins())->getY();
		}

		//▼화면 렌더링 관련 코드는 이  아래에 적혀야 함▼
		dur::renderTile = renderTile();
		renderWeather();
		dur::renderSticker = renderSticker(cameraX, cameraY);
		dur::renderUI = renderUI();
		dur::renderLog = renderLog(renderer);
		__int64 loopEnd = getNanoTimer();

		const int constDelay = 16000000;
		__int64 delayTime = constDelay - (loopEnd - loopStart);
		if (delayTime >= constDelay) delayTime = constDelay; // 만약 루프 시간이 음수(오류)가 나왔을 경우
		else if (delayTime < 0) delayTime = 0;
		SDL_Delay(delayTime/1000000);//FPS60일 때 16, 루프 시간이 길어질 경우 그 시간을 측정해서 슬립 시간을 줄여줌 최대 16ms
		//renderFPS(getNanoTimer() - loopStart);
		SDL_RenderPresent(renderer);
	}


	Mix_CloseAudio();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	_CrtDumpMemoryLeaks();

	return 0;
};

