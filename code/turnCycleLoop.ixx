
#include <SDL.h>
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
import clickRight;
import gamepadBtnDown;
import gamepadBtnMotion;
import gamepadBtnUp;
import AI;
import turnWait;
import dirToXY;
import globalTime;
import log;

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

	//for (auto* ePtr : entityList)
	//{
	//	if (ePtr->entityInfo.fakeHP > ePtr->entityInfo.HP) { ePtr->entityInfo.fakeHP--; }

	//	if (ePtr->entityInfo.fakeHP != ePtr->entityInfo.HP)
	//	{
	//		if (ePtr->entityInfo.fakeHPAlpha > 30) { ePtr->entityInfo.fakeHPAlpha -= 30; }
	//		else { ePtr->entityInfo.fakeHPAlpha = 0; }
	//	}
	//	else { ePtr->entityInfo.fakeHPAlpha = 255; }
	//}

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

		//prt(col::cyan, L"[�� ������ 1] �÷��̾� �Է�\n");

		if (coTurnSkip)
		{
			prt(L"���� �Լ� �ڷ�ƾ �����\n");
			coTurnSkip = false;
			(*coFunc).run();
		}

		if (Player::ins()->getHasAStarDst() == false) isPlayerMoving = false;
	}

	if (Player::ins()->getHasAStarDst() == true) //�ڵ��̵� ����� Ȱ��ȭ�Ǿ��� ���
	{
		int dx = Player::ins()->getAStarDstX() - Player::ins()->getGridX();
		int dy = Player::ins()->getAStarDstY() - Player::ins()->getGridY();

		if (dx != 0 || dy != 0)
		{
			//�ֺ� 10ĭ���� �̵� ������ Ÿ�� �迭 ���
			std::set<std::array<int, 2>> walkableTile;
			for (int i = Player::ins()->getGridX() - 20; i <= Player::ins()->getGridX() + 20; i++)
			{
				for (int j = Player::ins()->getGridY() - 20; j <= Player::ins()->getGridY() + 20; j++)
				{
					if (World::ins()->getTile(i, j, Player::ins()->getGridZ()).walkable == true) walkableTile.insert({ i,j });
					//else prt(L"(%d,%d) Ÿ���� �̵� �Ұ����� Ÿ���̴�.\n",i,j);
				}
			}

			if (walkableTile.find({ Player::ins()->getAStarDstX(), Player::ins()->getAStarDstY() }) != walkableTile.end())//�������� �̵� ������ ���
			{
				aStarTrail = aStar(walkableTile, Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getAStarDstX(), Player::ins()->getAStarDstY());
				if (aStarTrail.size() >= 2)
				{
					Player::ins()->startMove(coord2Dir(aStarTrail[1].x - aStarTrail[0].x, aStarTrail[1].y - aStarTrail[0].y));
					aStarTrail.erase(aStarTrail.begin());
				}
				else
				{
					Player::ins()->deactAStarDst();
					isPlayerMoving = false;
				}
			}
			else
			{
				Player::ins()->deactAStarDst();
				isPlayerMoving = false;
			}
		}
		else
		{
			Player::ins()->deactAStarDst();
			isPlayerMoving = false;
		}

	}
	else
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_CONTROLLERBUTTONDOWN:
				if (inputType != input::gamepad)
				{
					updateLog(col2Str(col::white) + L"�����е� ���� �����Ͽ���.\n");
					inputType = input::gamepad;
				}
				gamepadBtnDown();
				break;
			case SDL_CONTROLLERBUTTONUP:
				if (inputType != input::gamepad)
				{
					updateLog(col2Str(col::white) + L"�����е� ���� �����Ͽ���.\n");
					inputType = input::gamepad;
				}
				gamepadBtnUp();
				break;
			case SDL_CONTROLLERAXISMOTION:
				gamepadBtnMotion();
				break;
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
				if (inputType != input::mouse)
				{
					updateLog(col2Str(col::white) + L"���콺 ���� �����Ͽ���.\n");
					inputType = input::mouse;
				}

				if (inputType == input::mouse) { clickDown(); }
				break;
			case SDL_FINGERDOWN:
				if (inputType == input::touch) { clickDown(); }
				break;
			case SDL_MOUSEMOTION:
				if (inputType != input::mouse)
				{
					updateLog(col2Str(col::white) + L"���콺 ���� �����Ͽ���.\n");
					inputType = input::mouse;
				}

				if (inputType == input::mouse) { clickMotion(); }
				break;
			case SDL_FINGERMOTION:
				if (inputType == input::touch && (std::abs(event.tfinger.dx) * cameraW > 5 || std::abs(event.tfinger.dy) * cameraH > 5))
				{
					clickMotion();
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (inputType != input::mouse)
				{
					updateLog(col2Str(col::white) + L"���콺 ���� �����Ͽ���.\n");
					inputType = input::mouse;
				}

				if (inputType == input::mouse) 
				{ 
					if (event.button.button == SDL_BUTTON_LEFT) clickUp();
					else if (event.button.button == SDL_BUTTON_RIGHT) clickRight();
				}
				break;
			case SDL_FINGERUP:
				if (inputType == input::touch) { clickUp(); }
				break;
			case SDL_MOUSEWHEEL:
				if (inputType != input::mouse)
				{
					updateLog(col2Str(col::white) + L"���콺 ���� �����Ͽ���.\n");
					inputType = input::mouse;
				}

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
					prt(L"�齺���̽� Ű �Էµ�\n");
					if (exInputEditing == false)
					{
						if (exInputCursor != 0) { exInputCursor--; }
						exInputText.erase(exInputCursor, 1);
					}
				}
				break;
			case SDL_TEXTINPUT: //�ؽ�Ʈ�� ������ �ԷµǾ��� ���� �̺�Ʈ(�ѱ��� �ϼ��Ǿ��� ��)
			{
				if (exInput == true)
				{
					prt(L"�ϼ� �̺�Ʈ �����\n");
					std::wstring singleChar = L"";
					singleChar += utf8Decoder(event.text.text[0], event.text.text[1], event.text.text[2], event.text.text[3]);
					exInputText.insert(exInputCursor, singleChar);
					exInputCursor++;
					exInputEditing = false;
					if (exInputText.size() > EX_INPUT_TEXT_MAX)
					{
						exInputText = exInputText.substr(0, EX_INPUT_TEXT_MAX);
						exInputCursor = EX_INPUT_TEXT_MAX;
					}
				}
				break;
			}
			case SDL_TEXTEDITING: //�ؽ�Ʈ�� �Է� �Ǿ��� ���� �̺�Ʈ(�̿ϼ��� �����, TEXTINPUT���� �� ū ����)
			{
				if (exInput == true)
				{
					prt(L"���� �̺�Ʈ �Էµ�\n");
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



			}
		}
	}

	return (getNanoTimer() - timeStampStart);
}

__int64 animationTurn()
{
	//AniUSet
	//Ani Ŭ������ ��ӹ��� Ŭ������ aniUSet�̶�� unordered set�� ID�� �߰��� ���
	//�������� �� ID�� runAnimation �޼ҵ带 �����Ŵ. ���� runAnimation�� true�� ��ȯ�� ���
	//aniUSet�� �ִ� �� ID�� ���������� ���� ��� �ִϸ��̼��� ��������� ��� turnCycle�� 2(Entity�� AI ����)���� �ѱ�
	__int64 timeStampStart = getNanoTimer();
	if (turnCycle == turn::playerAnime)
	{
		if (firstPlayerAnime)
		{
			firstPlayerInput = true;
			firstPlayerAnime = false;
			firstMonsterAI = true;
			firstMonsterAnime = true;

			//prt(col::cyan, L"[�� ������ 2] �÷��̾� �ִϸ��̼�\n");
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

			//prt(col::cyan, L"[�� ������ 4] ���� �ִϸ��̼�\n");
		}
	}

	auto endAnimeTurn = [=]() 
		{
		if (turnCycle == turn::playerAnime)
		{
			std::vector<Entity*> entityList = (World::ins())->getActiveEntityList();
			std::vector<Vehicle*> vehList = (World::ins())->getActiveVehicleList();
			addTimeTurn(timeGift);
			for (auto ePtr : entityList)
			{
				if (ePtr != (Player::ins())) ((Monster*)ePtr)->addTimeResource(timeGift);
			}
			for (auto vPtr : vehList) vPtr->addTimeResource(timeGift);
			timeGift = 0;
			turnCycle = turn::monsterAI;
		}
		else if (turnCycle == turn::monsterAnime) turnCycle = turn::monsterAI;
		};


	if (aniUSet.size() > 0)
	{
		//aniUSet�� �ִ� ��� �ν��Ͻ��� üũ�ؼ� �ִϸ��̼��� �����Ŵ. 
		for (auto it = aniUSet.begin(); it != aniUSet.end();)
		{
			//prt(L"aniUSet�� ������� %d�̴�.\n",aniUSet.size());
			bool loopBreak = false;
			if ((*it)->isDictator()) { loopBreak = true; }

			if ((*it)->runAnimation(false) == true)//�����ϴ� �ִϸ��̼��� �������� ���
			{
				aniUSet.erase(it++);//it�� ����Ű�� ��Ҹ� �����ϰ� ���������ڷ� �÷������� �ݺ��ڰ� ��ȿȭ�����ʵ��� ��
				if (aniUSet.size() == 0)
				{
					endAnimeTurn();
					//prt(L"runAnimation ����, �ɸ� �ð��� %ls�̴�.\n", decimalCutter((getNanoTimer()- timeStampStart) / 1000000.0, 5).c_str());
				}
			}
			else it++;

			if (loopBreak) break;
			else if (it != aniUSet.end())//���� Dictator�� �ƴ϶� ���� Ani�� Dictator�� �̸� ������, ������ Dictator���� �����븦 ���� 
			{
				if ((*it)->isDictator()) break;
			}

		}
	}
	//��� ������ �ִϸ��̼��� �ϳ��� ���� ���
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
		//prt(col::cyan, L"[�� ������ 3] ���� AI\n");
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
		if (ePtr != (Player::ins()))
		{
			if (((Monster*)ePtr)->runAI() == false) endMonsterTurn = false;
		}
	}

	if (endMonsterTurn == true) 
	{ 
		turnCycle = turn::playerInput; 

		//�ϻ���Ŭ ��� ����
		for (auto ePtr : entityList)
		{
			for (auto it = ePtr->entityInfo.statusEffects.begin(); it != ePtr->entityInfo.statusEffects.end();)
			{
				if (it->second > 0) it->second--;

				if (it->second == 0) it = ePtr->entityInfo.statusEffects.erase(it);
				else it++;
			}
		}
	}
	else { turnCycle = turn::monsterAnime; }


	return (getNanoTimer() - timeStampStart);
}