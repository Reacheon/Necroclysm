module;
#include "lua/luaState.h"
#include <SDL3/sdl.h>

export module debugConsole;

import util;
import constVar;
import globalVar;
import globalTime;
import textureVar;
import log;
import World;
import Player;
import Monster;
import Vehicle;
import Sticker;
import Light;
import Flame;
import ItemStack;
import GameOver;
import SkillRegistry;
import Lst;
import paletteLoader;
import Teleport;
import levelUpFX;
import TitleScreen;
import CharSelectScreen;
import displayLoader;
import GUI;
import WorldData;

export void debugConsole()
{
	int xp = PlayerX();
	int yp = PlayerY();
	int zp = PlayerZ();

	dbgPrt(L"////////////////////////////////////////\n");
	dbgPrt(L"[디버그 모드] 원하는 값을 입력해주세요.\n");
	dbgPrt(L"현재 플레이어의 좌표는 (%d,%d,%d)이다.\n", PlayerX(), PlayerY(), PlayerZ());
	dbgPrt(L"2.적 생성\n");
	dbgPrt(L"3.테스트 로그 갱신\n");
	dbgPrt(L"4.아이템 생성\n");
	dbgPrt(L"5.불 생성\n");
	dbgPrt(L"6.EntityPtr 콘솔 출력\n");
	dbgPrt(L"7.벽 생성\n");
	dbgPrt(L"8.로그 타이머 비활성화/활성화\n");
	dbgPrt(L"9.콘솔 입력 로그 출력\n");
	dbgPrt(L"10.고정 텍스쳐 생성\n");
	dbgPrt(L"11.가스 생성\n");
	dbgPrt(L"12. 이큅 1번 제자리 드롭\n");
	dbgPrt(L"13. 달리기 토글\n");
	dbgPrt(L"14. 레벨업 이펙트 재생\n");
	dbgPrt(L"15. 탑승 중인 차량을 반시계로 22.5도 회전\n");
	dbgPrt(L"16. 탑승 중인 차량을 워프시키기\n");
	dbgPrt(L"17. 탑승 중인 차량을 연속적으로 이동시키기\n");
	dbgPrt(L"18. fov 콘솔 출력\n");
	dbgPrt(L"19. 시간 강제 설정\n");
	dbgPrt(L"20. 날짜 강제 설정\n");
	dbgPrt(L"21. 날씨 변경\n");
	dbgPrt(L"22. 말 생성\n");
	dbgPrt(L"23. 상태이상 추가\n");
	dbgPrt(L"24. 플레이어 텔레포트\n");
	dbgPrt(L"25. 청크 라인 표시\n");
	dbgPrt(L"26. 청크 덮어쓰기\n");
	dbgPrt(L"27. Lua 스크립트 실행\n");
	dbgPrt(L"28. 게임오버\n");
	dbgPrt(L"30. 스킬 추가\n");
	dbgPrt(L"31. 테스트 Lst 띄우기\n");
	dbgPrt(L"32. 플레이어 헤어스타일 변경\n");
	dbgPrt(L"33. 플레이어 눈 색상 변경\n");
	dbgPrt(L"34. 플레이어 피부색 변경\n");
	dbgPrt(L"35. 플레이어 성별 변경\n");
	dbgPrt(L"37. SUV 소환\n");
	dbgPrt(L"41. 경험치 추가\n");
	dbgPrt(L"42. 타이틀 화면으로\n");
	dbgPrt(L"43. 캐릭터 선택 화면\n");
	dbgPrt(L"44. 해상도 변경\n");
	dbgPrt(L"45. 모든 UI 숨기기 토글\n");
	dbgPrt(L"46. 월드 생성\n");

	dbgPrt(L"99. 콘솔 클리어\n");
	dbgPrt(L"////////////////////////////////////////\n");
	int select;
	std::cin >> select;
	switch (select)
	{
	default:
	{
		dbgPrt(L"잘못된 값을 입력하였습니다.\n");
		break;
	}
	case 2: // generateEntity
	{
		Player* ptr = PlayerPtr;
		int inputEntityCode;
		dbgPrt(L"생성할 몬스터의 코드를 입력해주세요.\n");
		std::cin >> inputEntityCode;
		createMonster({ ptr->getGridX() + 1, ptr->getGridY(), ptr->getGridZ() }, inputEntityCode);
		dbgPrt(L"[디버그]새로운 엔티티를 만들었다!\n");
		break;
	}
	case 3: //로그 업데이트
	{
		updateLog(L"#FFFFFFDEBUG : #FF0000Fire#2ECCFAICE");
		dbgPrt(L"[디버그]테스트로그를 갱신했다!\n");
		break;
	}
	case 4: //테스트 아이템
	{
		createItemStack({ PlayerX(), PlayerY(), PlayerZ() });
		ItemPocket* itemPtr = TileItemStack(PlayerX(), PlayerY(), PlayerZ())->getPocket();
		itemPtr->addItemFromDex(itemID::test, 2);
		itemPtr->addItemFromDex(itemID::ne555, 4);
		dbgPrt(L"[디버그]테스트 아이템을 생성했다!\n");
		break;
	}
	case 5: //맵핵
	{
		int inputX;
		int inputY;
		int fireSize;
		dbgPrt(L"[상대 좌표]\n");
		dbgPrt(L"생성할 불의 크기를 입력해주세요.(1~3)\n");
		std::cin >> fireSize;
		dbgPrt(L"생성할 화염의 x 좌표를 입력하세요.\n");
		std::cin >> inputX;
		dbgPrt(L"생성할 화염의 y 좌표를 입력하세요.\n");
		std::cin >> inputY;

		switch (fireSize)
		{
		case 1:
			createFlame({ xp + inputX,yp + inputY,zp }, flameFlag::SMALL);
			break;
		case 2:
			createFlame({ xp + inputX,yp + inputY,zp }, flameFlag::NORMAL);
			break;
		case 3:
			createFlame({ xp + inputX,yp + inputY,zp }, flameFlag::BIG);
			break;
		}
		dbgPrt(L"[디버그]화염을 생성했다.\n");
		break;
	}
	case 6: //entityPtr 출력
	{
		int xp = PlayerX();
		int yp = PlayerY();
		int zp = PlayerZ();
		for (int y = yp - 8; y <= yp + 8; y++)
		{
			for (int x = xp - 8; x <= xp + 8; x++)
			{
				if (TileEntity(x, y, zp) == nullptr) dbgPrt(L"□");
				else if (TileEntity(x, y, zp) == PlayerPtr) dbgPrt(lowCol::yellow, L"@");
				else dbgPrt(lowCol::red, L"E");
			}
			dbgPrt(L"\n");
		}
		dbgPrt(L"[디버그] entityPtr 출력을 끝냈다!\n");
		break;
	}
	case 7: //create wall
	{
		int wallX;
		int wallY;
		int wallZ;
		dbgPrt(L"[상대 좌표]\n");
		dbgPrt(L"생성할 벽의 x 좌표를 입력하세요.\n");
		std::cin >> wallX;
		dbgPrt(L"생성할 벽의 y 좌표를 입력하세요.\n");
		std::cin >> wallY;
		dbgPrt(L"생성할 벽의 z 좌표를 입력하세요.\n");
		std::cin >> wallZ;
		dbgPrt(L"[디버그](%d,%d,%d) 위치에 벽을 생성했다!\n", xp + wallX, yp + wallY, zp + wallZ);
		setWall({ xp + wallX, yp + wallY, zp + wallZ }, itemID::ne555);
		break;
	}
	case 8:
	{
		if (stopLog == false)
		{
			stopLog = true;
		}
		else
		{
			stopLog = false;
		}
		break;
	}
	case 9:
	{
		std::wstring input;
		dbgPrt(L"로그에 출력할 문자열을 입력해주세요.\n");
		std::wcin.ignore();
		std::getline(std::wcin, input);
		updateLog(input);
		dbgPrt(L"[디버그] 로그를 출력했다!\n");
		break;
	}
	case 10: // generate Sticker
	{
		new Sticker(true, -80, -80, L"ABCDE", nullptr, { 255,222,255 }, L"501TEXT", false, 10);
		new Sticker(false, PlayerPtr->getX(), PlayerPtr->getY() - 16, spr::effectClaw1, 0, L"121DEPTHTEST", true);
		new Sticker(false, PlayerPtr->getX(), PlayerPtr->getY() - 16, spr::charsetHero, 0, L"122DEPTHTEST", true);
		//new Sticker(false, PlayerPtr->getX(), PlayerPtr->y - 16, spr::defaultStranger, 0, L"123DEPTHTEST", true);
		break;
	}
	case 11: // 가스 생성
	{
		int gasCode, gasVol;
		dbgPrt(L"[상대 좌표]\n");
		dbgPrt(L"생성할 가스의 아이템 코드를 입력해주세요.\n");
		std::cin >> gasCode;
		dbgPrt(L"생성할 가스의 부피를 입력해주세요.\n");
		std::cin >> gasVol;
		if (World::ins()->getTile(xp, yp, zp).checkGas(gasCode) == -1)
		{
			World::ins()->getTile(xp, yp, zp).gasVec.push_back({ gasCode, gasVol });
		}
		dbgPrt(col::white, L"%ls를 %d의 부피만큼 좌표 (%d,%d,%d)에 생성하였다!\n", itemDex[gasCode].name.c_str(), gasVol, xp, yp, zp);
		break;
	}
	case 12: // 이큅먼트 1번 아이템 제자리 드롭
	{
		ItemPocket* equipPtr = PlayerEquip();
		std::unique_ptr<ItemPocket> txPtr = std::make_unique<ItemPocket>(storageType::temp);
		if (equipPtr->itemInfo.size() > 0)
		{
			equipPtr->transferItem(txPtr.get(), 0, 1);
			PlayerPtr->drop(txPtr.get());
		}
		break;
	}
	case 13:
	{
		break;
	}
	case 14://레벨업 연출 재생
	{
		levelUpFX::trigger();
		dbgPrt(L"[디버그] 레벨업 연출 재생!\n");
		break;
	}
	case 15://탑승 중인 차량 회전
	{
		ctrlVeh->rotate(ACW(ctrlVeh->bodyDir));
		break;
	}
	case 16://탑승 중인 차량 순간이동
	{
		int dx, dy;
		dbgPrt(L"이동할 거리 dx를 입력해주세요.\n");
		std::cin >> dx;
		dbgPrt(L"이동할 거리 dy를 입력해주세요.\n");
		std::cin >> dy;
		ctrlVeh->shift(dx, dy);
		break;
	}
	case 17://탑승 중인 차량 이동
	{
		int dx, dy;
		dbgPrt(L"이동할 거리 dx를 입력해주세요.\n");
		std::cin >> dx;
		dbgPrt(L"이동할 거리 dy를 입력해주세요.\n");
		std::cin >> dy;
		ctrlVeh->rush(dx, dy);
		break;
	}
	case 18: //fov 출력
	{
		for (int y = yp - 8; y <= yp + 8; y++)
		{
			for (int x = xp - 8; x <= xp + 8; x++)
			{
				if (TileFov(x, y, zp) == fovFlag::white) dbgPrt(L"○");
				else if (TileFov(x, y, zp) == fovFlag::gray) dbgPrt(L"◎");
				else dbgPrt(L"●");
			}
			dbgPrt(L"\n");
		}
		dbgPrt(L"[디버그] entityPtr 출력을 끝냈다!\n");
		break;
	}
	case 19: //시간 강제 설정
	{
		int tgtHour, tgtMin;
		dbgPrt(L"[1/2] 바꿀 시간을 입력해주세요(0~23).\n");
		std::cin >> tgtHour;
		dbgPrt(L"[2/2] 바꿀 분을 입력해주세요(0~59)\n");
		std::cin >> tgtMin;
		setDebugTime(tgtHour, tgtMin);
		break;
	}
	case 20: //날짜 강제 설정
	{
		int tgtMonth, tgtDay;
		dbgPrt(L"[1/2] 바꿀 달을 입력해주세요(1~12).\n");
		std::cin >> tgtMonth;
		dbgPrt(L"[2/2] 바꿀 일을 입력해주세요.\n");
		std::cin >> tgtDay;
		setDebugDay(tgtMonth, tgtDay);
		break;
	}
	case 21: //날씨 변경
	{
		int cx, cy;
		World::ins()->changeToChunkCoord(PlayerX(), PlayerY(), cx, cy);

		int tgtWeather;
		dbgPrt(L" 바꿀 날씨를 선택해주세요.\n");
		dbgPrt(L" 1. 맑음\n");
		dbgPrt(L" 2. 흐림\n");
		dbgPrt(L" 3. 비\n");
		dbgPrt(L" 4. 천둥\n");
		dbgPrt(L" 5. 눈\n");
		std::cin >> tgtWeather;
		if (tgtWeather == 1) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::sunny);
		else if (tgtWeather == 2) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::cloudy);
		else if (tgtWeather == 3) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::rain);
		else if (tgtWeather == 4) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::storm);
		else if (tgtWeather == 5) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::snow);
		dbgPrt(L" 날씨를 성공적으로 변경했다! \n");
		break;
	}
	case 22: //말 생성
	{
		Player* ptr = PlayerPtr;
		createMonster({ ptr->getGridX() + 1, ptr->getGridY(), ptr->getGridZ() }, 4);
		dbgPrt(L"[디버그]새로운 엔티티를 만들었다!\n");
		break;
	}
	case 23: //상태이상 생성
	{
		int tgtEfctIndex = 0;
		float tgtEfctDur = 1.0f;

		dbgPrt(L"[1/2] 추가할 상태이상의 인덱스를 입력해주세요.\n");
		std::cin >> tgtEfctIndex;

		dbgPrt(L"[2/2] 추가할 상태이상의 턴수를 입력해주세요.\n");
		std::cin >> tgtEfctDur;

		dbgPrt(L"상태이상을 성공적으로 추가하였다.\n");
		PlayerInfo().statusEffectVec.push_back({ (statusEffectFlag)tgtEfctIndex,tgtEfctDur });

		break;
	}
	case 24:
	{
		// 프리셋 정의 — 새 도시 추가 시 여기에 한 줄 추가하면 자동으로 메뉴에 노출됨
		struct TeleportPreset
		{
			const wchar_t* name;
			int x;
			int y;
			int z;
		};
		const std::array<TeleportPreset, 3> presets = { {
			{ L"SEOUL",       365772, -108156, 0 },
			{ L"YELLOW_SEA",  364580, -108156, 0 },
			{ L"SEOUL_RIVER", 365471, -108054, 0 },
		} };

		int tgtGridX = 0, tgtGridY = 0, tgtGridZ = 0;

		dbgPrt(L"텔레포트 모드를 선택해주세요.\n");
		dbgPrt(L"1. 수동 좌표 입력\n");
		dbgPrt(L"2. 프리셋 위치\n");
		int teleMode;
		std::cin >> teleMode;

		if (teleMode == 2)
		{
			dbgPrt(L"프리셋 번호를 선택해주세요.\n");
			for (int i = 0; i < (int)presets.size(); i++)
			{
				dbgPrt(L"%d. %ls (%d, %d, %d)\n", i + 1, presets[i].name, presets[i].x, presets[i].y, presets[i].z);
			}
			int psel;
			std::cin >> psel;
			if (psel < 1 || psel >(int)presets.size())
			{
				dbgPrt(L"잘못된 값을 입력하였습니다.\n");
				break;
			}
			tgtGridX = presets[psel - 1].x;
			tgtGridY = presets[psel - 1].y;
			tgtGridZ = presets[psel - 1].z;
		}
		else if (teleMode == 1)
		{
			dbgPrt(L"텔레포트할 위치의 gridX 좌표를 입력해주세요.\n");
			std::cin >> tgtGridX;

			dbgPrt(L"텔레포트할 위치의 gridY 좌표를 입력해주세요.\n");
			std::cin >> tgtGridY;

			dbgPrt(L"텔레포트할 위치의 gridZ 좌표를 입력해주세요.\n");
			std::cin >> tgtGridZ;
		}
		else
		{
			dbgPrt(L"잘못된 값을 입력하였습니다.\n");
			break;
		}

		// Teleport 모듈 통합 함수 — 패치·섹터·청크 동기 ensure + 로딩 화면 + 이동.
		teleportPlayer(Point3{ tgtGridX, tgtGridY, tgtGridZ });
		break;
	}
	case 25://청크라인 그리기
	{
		if(debug::chunkLineDraw ==false) debug::chunkLineDraw = true;
		else  debug::chunkLineDraw = false;
		break;
	}
	case 26://청크 덮어쓰기
	{
		int chunkX = 0;
		int chunkY = 0;
		int chunkZ = 0;
		int chunkInput = 0;

		dbgPrt(L"chunkX를 입력해주세요.\n");
		std::cin >> chunkX;

		dbgPrt(L"chunkY를 입력해주세요.\n");
		std::cin >> chunkY;

		dbgPrt(L"chunkZ를 입력해주세요.\n");
		std::cin >> chunkZ;


		dbgPrt(L"바꿀 청크를 입력해주세요.\n");
		dbgPrt(L"1.바다\n");
		dbgPrt(L"2.흙\n");
		dbgPrt(L"3.도로\n");
		dbgPrt(L"4.편의점\n");
		dbgPrt(L"5.아파트\n");
		std::cin >> chunkInput;

		if (chunkInput == 1) World::ins()->chunkOverwrite(chunkX, chunkY, chunkZ, chunkFlag::seawater);
		if (chunkInput == 2) World::ins()->chunkOverwrite(chunkX, chunkY, chunkZ, chunkFlag::dirt);

		break;
	}
	case 27://Lua 스크립트 실행
	{
		lua["cppValue"] = 10;
		sol::protected_function_result result = lua.script_file("exampleScript.lua");
		if (result.valid()) dbgPrt(L"루아스크립트가 성공적으로 실행되었다.\n");
		else
		{
			sol::error err = result;
			std::wcerr << "Error: " << err.what() << std::endl;
		}
		break;
	}
	case 28://게임오버
	{
		GameOver::create(L"테스트 사망 문구입니다.");
		break;
	}
	case 30://스킬 추가
	{
		dbgPrt(L"현재 보유 스킬 목록:\n");
		std::unordered_set<std::wstring> ownedIds;
		for (auto& sd : PlayerInfo().skillList)
		{
			auto* bhv = SkillRegistry::get(sd.skillId);
			dbgPrt(L"  - %ls: %ls (Lv%d)\n", sd.skillId.c_str(), bhv ? bhv->name.c_str() : L"(미등록)", sd.skillLevel);
			ownedIds.insert(sd.skillId);
		}

		// 등록된 모든 스킬을 src별로 그룹화해 번호와 함께 나열. 사용자는 번호로 선택.
		dbgPrt(L"\n========== 등록된 모든 스킬 ==========\n");
		std::vector<std::wstring> allIds = SkillRegistry::getAllIds();
		const std::array<std::pair<skillSrc, const wchar_t*>, 5> srcOrder = { {
			{ skillSrc::GENERAL,  L"GENERAL"  },
			{ skillSrc::BIONIC,   L"BIONIC"   },
			{ skillSrc::MUTATION, L"MUTATION" },
			{ skillSrc::MAGIC,    L"MAGIC"    },
			{ skillSrc::GOD,      L"GOD"      },
		} };
		// 번호 -> id 매핑을 만들어 입력 편의 제공. 번호는 전체 목록 순서대로 1-base.
		std::vector<std::wstring> indexedIds;
		int menuIdx = 1;
		for (const auto& [src, label] : srcOrder)
		{
			bool headerPrinted = false;
			for (const std::wstring& id : allIds)
			{
				auto* bhv = SkillRegistry::get(id);
				if (bhv == nullptr || bhv->src != src) continue;
				if (headerPrinted == false)
				{
					dbgPrt(L"[%ls]\n", label);
					headerPrinted = true;
				}
				const wchar_t* ownedTag = ownedIds.count(id) ? L" [OWNED]" : L"";
				dbgPrt(L"  %d) %ls (%ls)%ls\n", menuIdx, bhv->name.c_str(), id.c_str(), ownedTag);
				indexedIds.push_back(id);
				menuIdx++;
			}
		}
		dbgPrt(L"=====================================\n");

		dbgPrt(L"추가할 스킬 번호를 입력해주세요. (-1: 취소)\n");
		int menuChoice;
		std::cin >> menuChoice;
		if (menuChoice == -1) break;
		if (menuChoice < 1 || menuChoice > (int)indexedIds.size())
		{
			dbgPrt(L"[에러] 번호 %d는 범위를 벗어났습니다.\n", menuChoice);
			break;
		}

		const std::wstring& pickedId = indexedIds[menuChoice - 1];
		auto* bhv = SkillRegistry::get(pickedId);
		if (!bhv)
		{
			dbgPrt(L"[에러] 스킬 %ls는 SkillRegistry에 등록되지 않았습니다.\n", pickedId.c_str());
			break;
		}

		// 이미 보유 중인지 확인
		for (auto& sd : PlayerInfo().skillList)
		{
			if (sd.skillId == pickedId)
			{
				dbgPrt(L"[에러] 이미 보유 중인 스킬입니다: %ls\n", bhv->name.c_str());
				goto debugSkillEnd;
			}
		}

		{
			SkillData newSD;
			newSD.skillId = pickedId;
			newSD.isLearned = true;
			newSD.skillRank = bhv->skillRank; //시작 랭크 (바이오닉은 부품 등급)
			PlayerInfo().skillList.push_back(newSD);
			dbgPrt(L"[디버그] 스킬 추가 완료: %ls (%ls, src=%d)\n",
				bhv->name.c_str(), pickedId.c_str(), (int)bhv->src);

			// 추가된 스킬을 인게임 로그에도 노출 (디버그 테스트 시 무엇이 들어갔는지 확인용).
			// src에 따라 라벨을 바꾼다: MUTATION="mutation", BIONIC="bionic", 그 외="skill".
			std::wstring srcLabel;
			switch (bhv->src)
			{
			case skillSrc::MUTATION: srcLabel = L"mutation"; break;
			case skillSrc::BIONIC:   srcLabel = L"bionic";   break;
			default:                 srcLabel = L"skill";    break;
			}
			updateLog(L"[DEBUG] 새로운 스킬을 추가했다.");
		}
		debugSkillEnd:
		break;
	}
	case 31://테스트 Lst 띄우기
	{
		int optionCount;
		dbgPrt(L"생성할 옵션의 개수를 입력해주세요.\n");
		std::cin >> optionCount;
		if (optionCount <= 0) { dbgPrt(L"1 이상의 값을 입력해주세요.\n"); break; }

		std::wstring lstText;
		dbgPrt(L"안내 문자열을 입력해주세요. (Lst 상단에 표시됨)\n");
		std::wcin.ignore();
		std::getline(std::wcin, lstText);

		std::vector<std::wstring> options;
		for (int i = 0; i < optionCount; i++)
			options.push_back(L"Option " + std::to_wstring(i + 1));

		new Lst(L"Test List", lstText, options, false);
		dbgPrt(L"[디버그] 옵션 %d개의 테스트 Lst를 띄웠다!\n", optionCount);
		break;
	}
	case 32://플레이어 헤어스타일 변경
	{
		// image/charset/body/hair/의 PNG stem을 그대로 스타일명으로 사용.
		// 빈 문자열(0번)을 선택하면 대머리로 설정.
		std::vector<std::wstring> styles;
		styles.push_back(L""); // 0: 헤어 없음
		for (const auto& entry : std::filesystem::directory_iterator("image/charset/body/hair"))
		{
			if (entry.is_regular_file() == false) continue;
			if (entry.path().extension() != ".png") continue;
			styles.push_back(entry.path().stem().wstring());
		}

		dbgPrt(L"현재 헤어: %ls (색: %ls)\n",
			PlayerPtr->entityInfo.hairStyle.empty() ? L"(없음)" : PlayerPtr->entityInfo.hairStyle.c_str(),
			PlayerPtr->entityInfo.hairColor.c_str());
		dbgPrt(L"변경할 헤어스타일 번호를 선택해주세요.\n");
		for (int i = 0; i < (int)styles.size(); i++)
		{
			dbgPrt(L"%d. %ls\n", i, styles[i].empty() ? L"(없음)" : styles[i].c_str());
		}

		int sel;
		std::cin >> sel;
		if (sel < 0 || sel >= (int)styles.size())
		{
			dbgPrt(L"잘못된 값입니다.\n");
			break;
		}

		PlayerPtr->entityInfo.hairStyle = styles[sel];
		dbgPrt(L"[디버그] 헤어스타일을 %ls로 변경했다.\n",
			styles[sel].empty() ? L"(없음)" : styles[sel].c_str());
		break;
	}
	case 33://플레이어 눈 색상 변경
	{
		// palette/eyes.tsv의 헤더(BLUE/RED/SKY/...)를 그대로 선택지로 노출.
		// 새 색상은 TSV 컬럼 추가만으로 자동 반영됨 (하드코딩 없음).
		PaletteTable pal = loadPaletteTable("palette/eyes.tsv");
		if (pal.colorNames.empty())
		{
			dbgPrt(L"[디버그] palette/eyes.tsv 로드 실패.\n");
			break;
		}

		dbgPrt(L"현재 눈 색상: %ls\n",
			PlayerPtr->entityInfo.eyeColor.empty() ? L"(없음)" : PlayerPtr->entityInfo.eyeColor.c_str());
		dbgPrt(L"변경할 눈 색상 번호를 선택해주세요.\n");
		for (int i = 0; i < (int)pal.colorNames.size(); i++)
		{
			dbgPrt(L"%d. %ls\n", i, pal.colorNames[i].c_str());
		}

		int sel;
		std::cin >> sel;
		if (sel < 0 || sel >= (int)pal.colorNames.size())
		{
			dbgPrt(L"잘못된 값입니다.\n");
			break;
		}

		PlayerPtr->entityInfo.eyeColor = pal.colorNames[sel];
		dbgPrt(L"[디버그] 눈 색상을 %ls로 변경했다.\n", pal.colorNames[sel].c_str());
		break;
	}
	case 34://플레이어 피부색 변경
	{
		// palette/skin.tsv의 헤더(LIGHT/FAIR/TAN/...)를 그대로 선택지로 노출.
		// 새 색상은 TSV 컬럼 추가만으로 자동 반영됨 (하드코딩 없음).
		PaletteTable pal = loadPaletteTable("palette/skin.tsv");
		if (pal.colorNames.empty())
		{
			dbgPrt(L"[디버그] palette/skin.tsv 로드 실패.\n");
			break;
		}

		dbgPrt(L"현재 피부색: %ls\n",
			PlayerPtr->entityInfo.skinColor.empty() ? L"(없음)" : PlayerPtr->entityInfo.skinColor.c_str());
		dbgPrt(L"변경할 피부색 번호를 선택해주세요.\n");
		for (int i = 0; i < (int)pal.colorNames.size(); i++)
		{
			dbgPrt(L"%d. %ls\n", i, pal.colorNames[i].c_str());
		}

		int sel;
		std::cin >> sel;
		if (sel < 0 || sel >= (int)pal.colorNames.size())
		{
			dbgPrt(L"잘못된 값입니다.\n");
			break;
		}

		PlayerPtr->entityInfo.skinColor = pal.colorNames[sel];
		dbgPrt(L"[디버그] 피부색을 %ls로 변경했다.\n", pal.colorNames[sel].c_str());
		break;
	}
	case 35://플레이어 성별 변경
	{
		// image/charset/body/skin/SKIN_<gender>.png 의 stem에서 "SKIN_" 접두 제거한 부분을 선택지로 사용.
		// 새 신체 타입은 SKIN_<NAME>.png 한 장 추가만으로 자동 반영됨 (하드코딩 없음).
		std::vector<std::wstring> genders;
		for (const auto& entry : std::filesystem::directory_iterator("image/charset/body/skin"))
		{
			if (entry.is_regular_file() == false) continue;
			if (entry.path().extension() != ".png") continue;
			std::wstring stem = entry.path().stem().wstring();
			const std::wstring prefix = L"SKIN_";
			if (stem.starts_with(prefix) == false) continue;
			genders.push_back(stem.substr(prefix.size()));
		}

		if (genders.empty())
		{
			dbgPrt(L"[디버그] image/charset/body/skin 에서 SKIN_*.png를 찾지 못했다.\n");
			break;
		}

		dbgPrt(L"현재 성별: %ls\n", PlayerPtr->entityInfo.gender.c_str());
		dbgPrt(L"변경할 성별 번호를 선택해주세요.\n");
		for (int i = 0; i < (int)genders.size(); i++)
		{
			dbgPrt(L"%d. %ls\n", i, genders[i].c_str());
		}

		int sel;
		std::cin >> sel;
		if (sel < 0 || sel >= (int)genders.size())
		{
			dbgPrt(L"잘못된 값입니다.\n");
			break;
		}

		PlayerPtr->entityInfo.gender = genders[sel];
		dbgPrt(L"[디버그] 성별을 %ls로 변경했다.\n", genders[sel].c_str());
		break;
	}
	case 37://SUV 소환
	{
		// SUV는 [vX-1..vX+2] × [vY-3..vY+3] (4×7=28 타일)를 차지한다.
		// 플레이어 우측으로 3타일 떨어진 지점부터 점유하도록 vX = xp+4 로 잡는다.
		int vX = xp + 4;
		int vY = yp;
		int vZ = zp;

		bool blocked = false;
		for (int tx = vX - 1; tx <= vX + 2 && blocked == false; tx++)
		{
			for (int ty = vY - 3; ty <= vY + 3 && blocked == false; ty++)
			{
				if (TileVehicle(tx, ty, vZ) != nullptr)
				{
					dbgPrt(L"[디버그] (%d,%d,%d) 위치에 이미 차량이 있어 SUV를 소환할 수 없다.\n", tx, ty, vZ);
					blocked = true;
				}
			}
		}
		if (blocked) break;

		Vehicle* myCar = World::ins()->createVehicle(vX, vY, vZ, itemID::metalFrame);
		myCar->name = L"SUV";
		myCar->vehType = vehFlag::car;

		///////////////////////차량 기초 프레임//////////////////////////////////////
		myCar->extendPart(vX, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY - 1, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY - 2, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY + 1, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY + 2, itemID::metalFrame);
		myCar->extendPart(vX - 1, vY + 3, itemID::metalFrame);
		myCar->extendPart(vX, vY + 3, itemID::metalFrame);
		myCar->extendPart(vX + 1, vY + 3, itemID::metalFrame);
		myCar->extendPart(vX + 2, vY + 3, itemID::metalFrame);

		myCar->extendPart(vX - 1, vY - 3, itemID::steelBumper);
		myCar->extendPart(vX, vY - 3, itemID::steelBumper);
		myCar->extendPart(vX + 1, vY - 3, itemID::steelBumper);
		myCar->extendPart(vX + 2, vY - 3, itemID::steelBumper);
		//////////////////////////▼최상단 4타일////////////////////////////////////
		myCar->addPart(vX - 1, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
		myCar->addPart(vX, vY - 2, { itemID::vehicleWall });
		myCar->addPart(vX + 1, vY - 2, { itemID::vehicleWall });
		myCar->addPart(vX + 2, vY - 2, { itemID::steerableTire, itemID::vehicleWall, itemID::headlight });
		//////////////////////////▼중상단 4타일////////////////////////////////////
		myCar->addPart(vX - 1, vY - 1, itemID::vehicleGlass);
		myCar->addPart(vX, vY - 1, { itemID::vehicleGlass, itemID::engineV2Gasoline });
		myCar->addPart(vX + 1, vY - 1, itemID::vehicleGlass);
		myCar->addPart(vX + 2, vY - 1, itemID::vehicleGlass);
		////////////////////////////////▼운전석 4타일///////////////////////////////
		myCar->addPart(vX - 1, vY, { itemID::vehicleDoor });
		myCar->addPart(vX, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleControl, itemID::vehicleRoof });
		myCar->addPart(vX + 1, vY, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
		myCar->addPart(vX + 2, vY, { itemID::vehicleDoor });
		//////////////////////////▼운전석 아래 통로 4타일/////////////////////////////
		myCar->addPart(vX - 1, vY + 1, { itemID::vehicleWall });
		myCar->addPart(vX, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof });
		myCar->addPart(vX + 1, vY + 1, { itemID::vehiclePassage, itemID::vehicleRoof, itemID::vehicleTurret });
		myCar->addPart(vX + 2, vY + 1, { itemID::vehicleWall });
		///////////////////////////////▼뒷자석 4타일/////////////////////
		myCar->addPart(vX - 1, vY + 2, { itemID::vehicleDoor, itemID::fuelTank10L });
		{
			ItemPocket* partPocket = myCar->partInfo[{vX - 1, vY + 2, myCar->getGridZ()}].get();
			for (int i = 0; i < partPocket->itemInfo.size(); i++)
			{
				if (partPocket->itemInfo[i].itemCode == itemID::fuelTank10L)
				{
					partPocket->itemInfo[i].pocketPtr->addItemFromDex(itemID::gasoline, 900);
				}
			}
		}
		myCar->addPart(vX, vY + 2, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
		myCar->addPart(vX + 1, vY + 2, { itemID::vehiclePassage, itemID::vehicleSeat, itemID::vehicleRoof });
		myCar->addPart(vX + 2, vY + 2, { itemID::vehicleDoor });
		///////////////////////////////▼최후방 4타일///////////////////////////
		myCar->addPart(vX - 1, vY + 3, { itemID::vehicleWall, itemID::tailLight });
		myCar->addPart(vX, vY + 3, { itemID::trunkDoor });
		myCar->addPart(vX + 1, vY + 3, { itemID::trunkDoor });
		myCar->addPart(vX + 2, vY + 3, { itemID::vehicleWall, itemID::tailLight });

		dbgPrt(L"[디버그] SUV를 (%d,%d,%d) 위치에 소환했다.\n", vX, vY, vZ);
		break;
	}
	case 41://경험치 추가
	{
		dbgPrt(L"현재 레벨: %d, 경험치: %d/%d, AP: %d, 스킬포인트: %d\n",
			PlayerPtr->level, PlayerPtr->exp, PlayerPtr->expToNext(), PlayerPtr->ap, PlayerPtr->skillPoint);
		dbgPrt(L"추가할 경험치를 입력해주세요.\n");
		int expInput;
		std::cin >> expInput;
		PlayerPtr->addExp(expInput);
		dbgPrt(L"[디버그] 경험치를 %d만큼 추가했다. 현재 레벨: %d, 경험치: %d/%d, AP: %d, 스킬포인트: %d\n",
			expInput, PlayerPtr->level, PlayerPtr->exp, PlayerPtr->expToNext(), PlayerPtr->ap, PlayerPtr->skillPoint);
		break;
	}
	case 42://타이틀 화면으로
	{
		new TitleScreen();
		break;
	}
	case 43://캐릭터 선택 화면
	{
		new CharSelectScreen();
		break;
	}
	case 44://런타임 해상도 변경
	{
		int newW, newH;
		dbgPrt(L"현재 카메라 해상도: %dx%d\n", cameraW, cameraH);
		dbgPrt(L"기준은 1080x1080 - 가로가 길면 W만, 세로가 길면 H만 1080보다 크게.\n");
		dbgPrt(L"새 가로(W) 해상도를 입력해주세요.\n");
		std::cin >> newW;
		dbgPrt(L"새 세로(H) 해상도를 입력해주세요.\n");
		std::cin >> newH;
		applyResolution(newW, newH);
		// 레터박스·탭 등 changeXY가 캐시한 절대 좌표를 새 cameraW/H로 재계산.
		// 현재 x/y를 그대로 되넘기는 건 GUI 베이스 애니메이션과 동일한 재배치 패턴 —
		// HUD는 y가 상대값이라 팝업 상태도 유지된다. (HUD 직접 import는 순환이라 불가)
		for (GUI* g : GUI::activeGUIList) g->changeXY(g->x, g->y, false);
		break;
	}
	case 45://모든 UI 숨기기 토글 - renderUI(HUD 포함 전체 GUI)/renderLog 스킵. 스크린샷용
	{
		debug::hideAllUI = !debug::hideAllUI;
		if (debug::hideAllUI) dbgPrt(L"[디버그] 모든 UI를 숨겼다. (다시 45번 입력 시 복원)\n");
		else dbgPrt(L"[디버그] 모든 UI를 다시 표시한다.\n");
		break;
	}
	case 46://월드 생성
	{
		currentWorld.reset();
		SDL_DestroyTexture(texture::worldmap);
		texture::worldmap = nullptr;
		static std::uint64_t attempt = 0;
		attempt++;
		currentWorld = std::make_unique<WorldData>(getSeed() ^ (attempt * 0x9E3779B97F4A7C15ULL));
		dbgPrt(L"[디버그] 월드 생성이 완료되었다.\n");
	}
	case 99://콘솔 출력 초기화
	{
		system("cls");
		break;
	}

	}
}