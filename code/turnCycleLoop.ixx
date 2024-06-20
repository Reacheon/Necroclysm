#include <SDL.h>;
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

export module turnCycleLoop;

import std;
import util;
import globalVar;
import Entity;
import World;
import Player;
import Monster;
import Vehicle;
import clickUp;
import clickDown;
import clickMotion;
import AI;
import turnWait;
import dirToXY;
import globalTime;


static bool firstPlayerInput = true, firstPlayerAnime = true, firstMonsterAI = true, firstMonsterAnime = true;

__int64 playerInputTurn(), animationTurn(), entityAITurn();

export __int64 turnCycleLoop()
{
	__int64 timeStampStart = getNanoTimer();

	if (dtClickStack != -1)
	{
		switch (inputType)
		{
			case input::mouse:
				dtClickStack = SDL_GetTicks() - dtClickStackStart;
				break;
			case input::touch:
				dtClickStack = SDL_GetTicks() - dtClickStackStart;
				break;
		}
	}

	std::vector<Entity*> entityList = (World::ins())->getEntityList();
	for (int i = 0; i < entityList.size(); i++)
	{
		Entity* address = (Entity*)entityList[i];
		if (address->entityInfo.fakeHP > address->entityInfo.HP) { address->entityInfo.fakeHP--; }


		if (address->entityInfo.fakeHP != address->entityInfo.HP)
		{
			if (address->entityInfo.fakeHPAlpha > 30) { address->entityInfo.fakeHPAlpha -= 30; }
			else { address->entityInfo.fakeHPAlpha = 0; }
		}
		else { address->entityInfo.fakeHPAlpha = 255; }
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
	else errorBox("Unknown turnCycle executed in turyCycleLoop.ixx");

	return (getNanoTimer() - timeStampStart);
};

__int64 playerInputTurn()
{
	__int64 timeStampStart = getNanoTimer();
	if (firstPlayerInput)
	{
		firstPlayerInput = false;
		firstPlayerAnime = true;
		firstMonsterAI = true;
		firstMonsterAnime = true;

		//prt(col::cyan, L"[턴 페이즈 1] 플레이어 입력\n");

		if (coTurnSkip)
		{
			prt(printRed);
			prt(L"메인 함수 코루틴 재실행\n");
			prt(printReset);
			coTurnSkip = false;
			(*coFunc).run();

		}
	}



	if (Player::ins()->getHasAStarDst() == true) //자동이동 기능이 활성화되었을 경우
	{
		int dx = Player::ins()->getAStarDstX() - Player::ins()->getGridX();
		int dy = Player::ins()->getAStarDstY() - Player::ins()->getGridY();

		if (abs(dx) <= 1 && abs(dy) <= 1)
		{
			Player::ins()->startMove(coord2Dir(dx, dy));
			Player::ins()->deactAStarDst();
		}
		else
		{
			//주변 10칸으로 이동 가능한 타일 배열 계산
			std::set<std::array<int, 2>> walkableTile;
			for (int i = Player::ins()->getGridX() - 20; i <= Player::ins()->getGridX() + 20; i++)
			{
				for (int j = Player::ins()->getGridY() - 20; j <= Player::ins()->getGridY() + 20; j++)
				{
					if (World::ins()->getTile(i, j, Player::ins()->getGridZ()).walkable == true) walkableTile.insert({ i,j });
					//else prt(L"(%d,%d) 타일은 이동 불가능한 타일이다.\n",i,j);
				}
			}

			int finalDir = -1;
			if (walkableTile.find({ Player::ins()->getAStarDstX(), Player::ins()->getAStarDstY() }) != walkableTile.end())//목적지가 이동 가능할 경우
			{
				finalDir = aStar(walkableTile, Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getAStarDstX(), Player::ins()->getAStarDstY());
				Player::ins()->startMove((finalDir + 4) % 8);
			}
			else Player::ins()->deactAStarDst();
		}

	}
	else
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					SDL_RenderSetLogicalSize(renderer, 434, 244);
					break;
				}
				break;
			case SDL_QUIT:
				Mix_CloseAudio();
				IMG_Quit();
				TTF_Quit();
				SDL_Quit();
				exit(0);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (inputType == input::mouse) { clickDown(); }
				break;
			case SDL_FINGERDOWN:
				if (inputType == input::touch) { clickDown(); }
				break;
			case SDL_MOUSEMOTION:
				if (inputType == input::mouse) { clickMotion(); }
				break;
			case SDL_FINGERMOTION:
				if (inputType == input::touch && (std::abs(event.tfinger.dx) * cameraW > 5 || std::abs(event.tfinger.dy) * cameraH > 5))
				{
					clickMotion();
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (inputType == input::mouse) { clickUp(); }
				break;
			case SDL_FINGERUP:
				if (inputType == input::touch) { clickUp(); }
				break;
			case SDL_MOUSEWHEEL:
				if (event.wheel.y > 0) 
				{
					zoomScale += 1;
					if (zoomScale > 5.0) zoomScale = 5;
				}
				else if (event.wheel.y < 0) 
				{
					zoomScale -= 1;
					if (zoomScale < 1.0) zoomScale = 1;
				}
				break;
			case SDL_KEYDOWN:
				if (exInput == true && event.key.keysym.sym == UNI::BACKSPACE)
				{
					prt(L"백스페이스 키 입력됨\n");
					if (exInputEditing == false)
					{
						if (exInputCursor != 0) { exInputCursor--; }
						exInputText.erase(exInputCursor, 1);
					}
				}
				break;
			case SDL_TEXTINPUT: //텍스트가 완전히 입력되었을 때의 이벤트(한글이 완성되었을 때)
			{
				if (exInput == true)
				{
					prt(L"완성 이벤트 실행됨\n");
					std::wstring singleChar = L"";
					singleChar += utf8Decoder(event.text.text[0], event.text.text[1], event.text.text[2], event.text.text[3]);
					exInputText.insert(exInputCursor, singleChar);
					exInputCursor++;
					exInputEditing = false;
					if (exInputText.size() > exInputTextMax)
					{
						exInputText = exInputText.substr(0, exInputTextMax);
						exInputCursor = exInputTextMax;
					}
				}
				break;
			}
			case SDL_TEXTEDITING: //텍스트가 입력 되었을 때의 이벤트(미완성도 실행됨, TEXTINPUT보다 더 큰 개념)
			{
				if (exInput == true)
				{
					prt(L"편집 이벤트 입력됨\n");
					std::wstring singleChar = L"";
					singleChar += utf8Decoder(event.text.text[0], event.text.text[1], event.text.text[2], event.text.text[3]);
					exInputText.replace(exInputCursor, 1, singleChar);
					if (exInputText[exInputCursor] == 0) { exInputEditing = false; }
					else { exInputEditing = true; }
					if (exInputText.size() > exInputTextMax)
					{
						exInputText = exInputText.substr(0, exInputTextMax);
						exInputCursor = exInputTextMax;
					}
				}
				break;
			}
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

	auto distributeTime = []() {
		addTimeTurn(timeGift);
		for (int i = 0; i < (World::ins())->getEntityList().size(); i++)
		{
			if ((World::ins())->getEntityList()[i] != (Player::ins()))
			{
				((Monster*)(World::ins())->getEntityList()[i])->addTimeResource(timeGift);
			}
		}
		for (int i = 0; i < (World::ins())->getVehicleList().size(); i++)
		{
			((Vehicle*)(World::ins())->getVehicleList()[i])->addTimeResource(timeGift);
		}
		timeGift = 0;
		};

	auto endAnimeTurn = [=]() {
		if (turnCycle == turn::playerAnime)
		{
			distributeTime();
			turnCycle = turn::monsterAI;
		}
		else if (turnCycle == turn::monsterAnime) turnCycle = turn::monsterAI;
		};

	if (aniUSet.size() > 0)
	{
		//aniUSet에 있는 모든 인스턴스를 체크해서 애니메이션을 실행시킴. 
		for (auto it = aniUSet.begin(); it != aniUSet.end();)
		{
			bool loopBreak = false;
			if (((Ani*)(*it))->isDictator()) { loopBreak = true; }

			if (((Ani*)(*it))->runAnimation(false) == true)//종료하는 애니메이션을 실행했을 경우
			{
				aniUSet.erase(it++);//it가 가리키는 요소를 제거하고 후위연산자로 늘려버려서 반복자가 무효화되지않도록 함
				if (aniUSet.size() == 0) endAnimeTurn();
			}
			else
			{
				it++;
			}

			if (loopBreak) break;
			else if (it != aniUSet.end())//설령 Dictator가 아니라도 다음 Ani가 Dictator면 미리 종료함, 다음은 Dictator만의 독무대를 위해 
			{
				if (((Ani*)(*it))->isDictator())
				{
					break;
				}
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

	std::vector<Vehicle*> vehList = (World::ins())->getVehicleList();
	for (int i = 0; i < vehList.size(); i++)
	{
		if (((Vehicle*)vehList[i])->runAI() == false) endMonsterTurn = false;
	}

	std::vector<Entity*> entityList = (World::ins())->getEntityList();
	for (int i = 0; i < entityList.size(); i++)
	{
		if (entityList[i] != (Player::ins()))
		{
			if (((Monster*)entityList[i])->runAI() == false) endMonsterTurn = false;
		}
	}

	if (endMonsterTurn == true) { turnCycle = turn::playerInput; }
	else { turnCycle = turn::monsterAnime; }

	return (getNanoTimer() - timeStampStart);
}