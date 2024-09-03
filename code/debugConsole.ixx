export module debugConsole;

import util;
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

export void debugConsole()
{
	int xp = Player::ins()->getGridX();
	int yp = Player::ins()->getGridY();
	int zp = Player::ins()->getGridZ();

	updateLog(L"#FFFFFF디버그 테스트 기능을 실행했다.");
	prt(L"////////////////////////////////////////\n");
	prt(L"[디버그 모드] 원하는 값을 입력해주세요.\n");
	prt(L"현재 플레이어의 좌표는 (%d,%d,%d)이다.\n", Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ());
	prt(L"1.광원 생성\n");
	prt(L"2.적 생성\n");
	prt(L"3.테스트 로그 갱신\n");
	prt(L"4.아이템 생성\n");
	prt(L"5.불 생성\n");
	prt(L"6.EntityPtr 콘솔 출력\n");
	prt(L"7.벽 생성\n");
	prt(L"8.로그 타이머 비활성화/활성화\n");
	prt(L"9.로그 클리어\n");
	prt(L"10.고정 텍스쳐 생성\n");
	prt(L"11.가스 생성\n");
	prt(L"12. 이큅 1번 제자리 드롭\n");
	prt(L"13. 달리기 토글\n");
	prt(L"14. 경험치 획득\n");
	prt(L"15. 탑승 중인 차량을 반시계로 22.5도 회전\n");
	prt(L"16. 탑승 중인 차량을 워프시키기\n");
	prt(L"17. 탑승 중인 차량을 연속적으로 이동시키기\n");
	prt(L"18. fov 콘솔 출력\n");
	prt(L"19. 시간 강제 설정\n");
	prt(L"20. 날짜 강제 설정\n");
	prt(L"21. 날씨 변경\n");
	prt(L"22. 말 생성\n");
	prt(L"23. 상태이상 추가\n");
	prt(L"////////////////////////////////////////\n");
	int select;
	std::cin >> select;
	switch (select)
	{
	default:
	{
		prt(L"잘못된 값을 입력하였습니다.\n");
		break;
	}
	case 1:// generateLight
	{
		int autoLight = false;
		prt(L"자동 조명을 생성하시겠습니까?(0 or 1)\n");
		std::cin >> autoLight;
		if (autoLight)
		{
			Light* light = new Light(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ(), 8, 255, { 0xd8,0x56,0x00 });
			Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
		}
		else
		{
			int lightColorR;
			int lightColorG;
			int lightColorB;
			int sight;
			int bright;
			prt(L"광원의 시야 범위를 입력해주세요(0~10).\n");
			std::cin >> sight;
			prt(L"광원의 밝기 입력해주세요(0~255).\n");
			std::cin >> bright;
			prt(L"광원의 R값을 입력해주세요(0~255).\n");
			std::cin >> lightColorR;
			prt(L"광원의 G값을 입력해주세요(0~255).\n");
			std::cin >> lightColorG;
			prt(L"광원의 B값을 입력해주세요(0~255).\n");
			std::cin >> lightColorB;

			Light* light = new Light(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ(), (Uint8)sight, (Uint8)bright, { (Uint8)lightColorR,(Uint8)lightColorG,(Uint8)lightColorB });
			Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
		}
		updateLog(L"#FFFFFF디버그 : 테스트 광원을 생성했다.");
		prt(L"[디버그]테스트 광원을 생성했다!\n");

		break;
	}
	case 2: // generateEntity
	{
		Player* ptr = Player::ins();
		new Monster(2, ptr->getGridX() + 1, ptr->getGridY(), ptr->getGridZ());
		prt(L"[디버그]새로운 엔티티를 만들었다!\n");
		break;
	}
	case 3: //로그 업데이트
	{
		updateLog(L"#FFFFFF디버그 : #FF0000Fire#2ECCFA얼음");
		prt(L"[디버그]테스트로그를 갱신했다!\n");
		break;
	}
	case 4: //테스트 아이템
	{
		updateLog(L"#FFFFFF디버그 : 테스트 아이템을 생성했다.");
		ItemStack* item = new ItemStack((Player::ins())->getGridX(), (Player::ins())->getGridY(), (Player::ins())->getGridZ());
		(item->getPocket())->addItemFromDex(0, 2);
		(item->getPocket())->addItemFromDex(1, 4);
		prt(L"[디버그]테스트 아이템을 생성했다!\n");
		break;
	}
	case 5: //맵핵
	{
		int inputX;
		int inputY;
		int fireSize;
		prt(L"[상대 좌표]\n");
		prt(L"생성할 불의 크기를 입력해주세요.(1~3)\n");
		std::cin >> fireSize;
		prt(L"생성할 화염의 x 좌표를 입력하세요.\n");
		std::cin >> inputX;
		prt(L"생성할 화염의 y 좌표를 입력하세요.\n");
		std::cin >> inputY;

		switch (fireSize)
		{
		case 1:
			new Flame(xp + inputX, yp + inputY, zp, flameFlag::SMALL);
			break;
		case 2:
			new Flame(xp + inputX, yp + inputY, zp, flameFlag::NORMAL);
			break;
		case 3:
			new Flame(xp + inputX, yp + inputY, zp, flameFlag::BIG);
			break;
		}
		updateLog(L"#FFFFFF화염을 생성했다.");
		prt(L"[디버그]화염을 생성했다.\n");
		break;
	}
	case 6: //entityPtr 출력
	{
		int xp = Player::ins()->getGridX();
		int yp = Player::ins()->getGridY();
		int zp = Player::ins()->getGridZ();
		for (int y = yp - 8; y <= yp + 8; y++)
		{
			for (int x = xp - 8; x <= xp + 8; x++)
			{
				if (World::ins()->getTile(x, y, zp).EntityPtr == nullptr) prt(L"□");
				else if (World::ins()->getTile(x, y, zp).EntityPtr == Player::ins()) prt(lowCol::yellow, L"@");
				else prt(lowCol::red, L"E");
			}
			prt(L"\n");
		}
		prt(L"[디버그] entityPtr 출력을 끝냈다!\n");
		break;
	}
	case 7: //create wall
	{
		int wallX;
		int wallY;
		int wallZ;
		prt(L"[상대 좌표]\n");
		prt(L"생성할 벽의 x 좌표를 입력하세요.\n");
		std::cin >> wallX;
		prt(L"생성할 벽의 y 좌표를 입력하세요.\n");
		std::cin >> wallY;
		prt(L"생성할 벽의 z 좌표를 입력하세요.\n");
		std::cin >> wallZ;
		prt(L"[디버그](%d,%d,%d) 위치에 벽을 생성했다!\n", xp + wallX, yp + wallY, zp + wallZ);
		World::ins()->getTile(xp + wallX, yp + wallY, zp + wallZ).wall = true;
		World::ins()->getTile(xp + wallX, yp + wallY, zp + wallZ).blocker = true;
		World::ins()->getTile(xp + wallX, yp + wallY, zp + wallZ).walkable = false;
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
		clearLog();
		break;
	}
	case 10: // generate Sticker
	{
		new Sticker(true, -80, -80, L"ABCDE", nullptr, { 255,222,255 }, L"501TEXT", false, 10);
		new Sticker(false, Player::ins()->getX(), Player::ins()->getY() - 16, spr::effectClaw1, 0, L"121DEPTHTEST", true);
		new Sticker(false, Player::ins()->getX(), Player::ins()->getY() - 16, spr::charsetHero, 0, L"122DEPTHTEST", true);
		//new Sticker(false, Player::ins()->getX(), Player::ins()->y - 16, spr::defaultStranger, 0, L"123DEPTHTEST", true);
		break;
	}
	case 11: // 가스 생성
	{
		int gasCode, gasVol;
		prt(L"[상대 좌표]\n");
		prt(L"생성할 가스의 아이템 코드를 입력해주세요.\n");
		std::cin >> gasCode;
		prt(L"생성할 가스의 부피를 입력해주세요.\n");
		std::cin >> gasVol;
		if (World::ins()->getTile(xp, yp, zp).checkGas(gasCode) == -1)
		{
			World::ins()->getTile(xp, yp, zp).gasVec.push_back({ gasCode, gasVol });
		}
		prt(col::white, L"%ls를 %d의 부피만큼 좌표 (%d,%d,%d)에 생성하였다!\n", itemDex[gasCode].name.c_str(), gasVol, xp, yp, zp);
		break;
	}
	case 12: // 이큅먼트 1번 아이템 제자리 드롭
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		ItemPocket* txPtr = new ItemPocket(storageType::temp);
		if (equipPtr->itemInfo.size() > 0)
		{
			equipPtr->transferItem(txPtr, 0, 1);
			Player::ins()->drop(txPtr);
		}
		break;
	}
	case 13:
	{
		break;
	}
	case 14:
	{
		int expVal;
		prt(L"추가할 경험치의 양을 입력해주세요.\n");
		std::cin >> expVal;
		Player::ins()->addTalentExp(expVal);
		break;
	}
	case 15://탑승 중인 차량 회전
	{
		Vehicle* myCar = (Vehicle*)(ctrlVeh);
		myCar->rotate(ACW(myCar->bodyDir));
		break;
	}
	case 16://탑승 중인 차량 순간이동
	{
		Vehicle* myCar = (Vehicle*)(ctrlVeh);
		int dx, dy;
		prt(L"이동할 거리 dx를 입력해주세요.\n");
		std::cin >> dx;
		prt(L"이동할 거리 dy를 입력해주세요.\n");
		std::cin >> dy;
		myCar->shift(dx, dy);
		break;
	}
	case 17://탑승 중인 차량 이동
	{
		Vehicle* myCar = (Vehicle*)(ctrlVeh);
		int dx, dy;
		prt(L"이동할 거리 dx를 입력해주세요.\n");
		std::cin >> dx;
		prt(L"이동할 거리 dy를 입력해주세요.\n");
		std::cin >> dy;
		myCar->rush(dx, dy);
		break;
	}
	case 18: //fov 출력
	{
		for (int y = yp - 8; y <= yp + 8; y++)
		{
			for (int x = xp - 8; x <= xp + 8; x++)
			{
				if (World::ins()->getTile(x, y, zp).fov == fovFlag::white) prt(L"○");
				else if (World::ins()->getTile(x, y, zp).fov == fovFlag::gray) prt(L"◎");
				else prt(L"●");
			}
			prt(L"\n");
		}
		prt(L"[디버그] entityPtr 출력을 끝냈다!\n");
		break;
	}
	case 19: //시간 강제 설정
	{
		int tgtHour, tgtMin;
		prt(L"[1/2] 바꿀 시간을 입력해주세요(0~23).\n");
		std::cin >> tgtHour;
		prt(L"[2/2] 바꿀 분을 입력해주세요(0~59)\n");
		std::cin >> tgtMin;
		setDebugTime(tgtHour, tgtMin);
		break;
	}
	case 20: //날짜 강제 설정
	{
		int tgtMonth, tgtDay;
		prt(L"[1/2] 바꿀 달을 입력해주세요(1~12).\n");
		std::cin >> tgtMonth;
		prt(L"[2/2] 바꿀 일을 입력해주세요.\n");
		std::cin >> tgtDay;
		setDebugDay(tgtMonth, tgtDay);
		break;
	}
	case 21: //날씨 변경
	{
		int cx, cy;
		World::ins()->changeToChunkCoord(Player::ins()->getGridX(), Player::ins()->getGridY(), cx, cy);

		int tgtWeather;
		prt(L" 바꿀 날씨를 선택해주세요.\n");
		prt(L" 1. 맑음\n");
		prt(L" 2. 흐림\n");
		prt(L" 3. 비\n");
		prt(L" 4. 천둥\n");
		prt(L" 5. 눈\n");
		std::cin >> tgtWeather;
		if (tgtWeather == 1) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::sunny);
		else if (tgtWeather == 2) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::cloudy);
		else if (tgtWeather == 3) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::rain);
		else if (tgtWeather == 4) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::storm);
		else if (tgtWeather == 5) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::snow);
		prt(L" 날씨를 성공적으로 변경했다! \n");
		break;
	}
	case 22: //말 생성
	{
		Player* ptr = Player::ins();
		new Monster(4, ptr->getGridX() + 1, ptr->getGridY(), ptr->getGridZ());
		prt(L"[디버그]새로운 엔티티를 만들었다!\n");
		break;
	}
	case 23: //상태이상 생성
	{
		int tgtEfctIndex = 0;
		int tgtEfctDur = 1;

		prt(L"[1/2] 추가할 상태이상의 인덱스를 입력해주세요.\n");
		std::cin >> tgtEfctIndex;

		prt(L"[2/2] 추가할 상태이상의 턴수를 입력해주세요.\n");
		std::cin >> tgtEfctDur;

		prt(L"상태이상을 성공적으로 추가하였다.\n");
		Player::ins()->entityInfo.statusEffects.push_back({ (statEfctFlag)tgtEfctIndex,tgtEfctDur });

		break;
	}
	}
}