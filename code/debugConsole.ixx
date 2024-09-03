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

	updateLog(L"#FFFFFF����� �׽�Ʈ ����� �����ߴ�.");
	prt(L"////////////////////////////////////////\n");
	prt(L"[����� ���] ���ϴ� ���� �Է����ּ���.\n");
	prt(L"���� �÷��̾��� ��ǥ�� (%d,%d,%d)�̴�.\n", Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ());
	prt(L"1.���� ����\n");
	prt(L"2.�� ����\n");
	prt(L"3.�׽�Ʈ �α� ����\n");
	prt(L"4.������ ����\n");
	prt(L"5.�� ����\n");
	prt(L"6.EntityPtr �ܼ� ���\n");
	prt(L"7.�� ����\n");
	prt(L"8.�α� Ÿ�̸� ��Ȱ��ȭ/Ȱ��ȭ\n");
	prt(L"9.�α� Ŭ����\n");
	prt(L"10.���� �ؽ��� ����\n");
	prt(L"11.���� ����\n");
	prt(L"12. ��Ţ 1�� ���ڸ� ���\n");
	prt(L"13. �޸��� ���\n");
	prt(L"14. ����ġ ȹ��\n");
	prt(L"15. ž�� ���� ������ �ݽð�� 22.5�� ȸ��\n");
	prt(L"16. ž�� ���� ������ ������Ű��\n");
	prt(L"17. ž�� ���� ������ ���������� �̵���Ű��\n");
	prt(L"18. fov �ܼ� ���\n");
	prt(L"19. �ð� ���� ����\n");
	prt(L"20. ��¥ ���� ����\n");
	prt(L"21. ���� ����\n");
	prt(L"22. �� ����\n");
	prt(L"23. �����̻� �߰�\n");
	prt(L"////////////////////////////////////////\n");
	int select;
	std::cin >> select;
	switch (select)
	{
	default:
	{
		prt(L"�߸��� ���� �Է��Ͽ����ϴ�.\n");
		break;
	}
	case 1:// generateLight
	{
		int autoLight = false;
		prt(L"�ڵ� ������ �����Ͻðڽ��ϱ�?(0 or 1)\n");
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
			prt(L"������ �þ� ������ �Է����ּ���(0~10).\n");
			std::cin >> sight;
			prt(L"������ ��� �Է����ּ���(0~255).\n");
			std::cin >> bright;
			prt(L"������ R���� �Է����ּ���(0~255).\n");
			std::cin >> lightColorR;
			prt(L"������ G���� �Է����ּ���(0~255).\n");
			std::cin >> lightColorG;
			prt(L"������ B���� �Է����ּ���(0~255).\n");
			std::cin >> lightColorB;

			Light* light = new Light(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ(), (Uint8)sight, (Uint8)bright, { (Uint8)lightColorR,(Uint8)lightColorG,(Uint8)lightColorB });
			Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
		}
		updateLog(L"#FFFFFF����� : �׽�Ʈ ������ �����ߴ�.");
		prt(L"[�����]�׽�Ʈ ������ �����ߴ�!\n");

		break;
	}
	case 2: // generateEntity
	{
		Player* ptr = Player::ins();
		new Monster(2, ptr->getGridX() + 1, ptr->getGridY(), ptr->getGridZ());
		prt(L"[�����]���ο� ��ƼƼ�� �������!\n");
		break;
	}
	case 3: //�α� ������Ʈ
	{
		updateLog(L"#FFFFFF����� : #FF0000Fire#2ECCFA����");
		prt(L"[�����]�׽�Ʈ�α׸� �����ߴ�!\n");
		break;
	}
	case 4: //�׽�Ʈ ������
	{
		updateLog(L"#FFFFFF����� : �׽�Ʈ �������� �����ߴ�.");
		ItemStack* item = new ItemStack((Player::ins())->getGridX(), (Player::ins())->getGridY(), (Player::ins())->getGridZ());
		(item->getPocket())->addItemFromDex(0, 2);
		(item->getPocket())->addItemFromDex(1, 4);
		prt(L"[�����]�׽�Ʈ �������� �����ߴ�!\n");
		break;
	}
	case 5: //����
	{
		int inputX;
		int inputY;
		int fireSize;
		prt(L"[��� ��ǥ]\n");
		prt(L"������ ���� ũ�⸦ �Է����ּ���.(1~3)\n");
		std::cin >> fireSize;
		prt(L"������ ȭ���� x ��ǥ�� �Է��ϼ���.\n");
		std::cin >> inputX;
		prt(L"������ ȭ���� y ��ǥ�� �Է��ϼ���.\n");
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
		updateLog(L"#FFFFFFȭ���� �����ߴ�.");
		prt(L"[�����]ȭ���� �����ߴ�.\n");
		break;
	}
	case 6: //entityPtr ���
	{
		int xp = Player::ins()->getGridX();
		int yp = Player::ins()->getGridY();
		int zp = Player::ins()->getGridZ();
		for (int y = yp - 8; y <= yp + 8; y++)
		{
			for (int x = xp - 8; x <= xp + 8; x++)
			{
				if (World::ins()->getTile(x, y, zp).EntityPtr == nullptr) prt(L"��");
				else if (World::ins()->getTile(x, y, zp).EntityPtr == Player::ins()) prt(lowCol::yellow, L"@");
				else prt(lowCol::red, L"E");
			}
			prt(L"\n");
		}
		prt(L"[�����] entityPtr ����� ���´�!\n");
		break;
	}
	case 7: //create wall
	{
		int wallX;
		int wallY;
		int wallZ;
		prt(L"[��� ��ǥ]\n");
		prt(L"������ ���� x ��ǥ�� �Է��ϼ���.\n");
		std::cin >> wallX;
		prt(L"������ ���� y ��ǥ�� �Է��ϼ���.\n");
		std::cin >> wallY;
		prt(L"������ ���� z ��ǥ�� �Է��ϼ���.\n");
		std::cin >> wallZ;
		prt(L"[�����](%d,%d,%d) ��ġ�� ���� �����ߴ�!\n", xp + wallX, yp + wallY, zp + wallZ);
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
	case 11: // ���� ����
	{
		int gasCode, gasVol;
		prt(L"[��� ��ǥ]\n");
		prt(L"������ ������ ������ �ڵ带 �Է����ּ���.\n");
		std::cin >> gasCode;
		prt(L"������ ������ ���Ǹ� �Է����ּ���.\n");
		std::cin >> gasVol;
		if (World::ins()->getTile(xp, yp, zp).checkGas(gasCode) == -1)
		{
			World::ins()->getTile(xp, yp, zp).gasVec.push_back({ gasCode, gasVol });
		}
		prt(col::white, L"%ls�� %d�� ���Ǹ�ŭ ��ǥ (%d,%d,%d)�� �����Ͽ���!\n", itemDex[gasCode].name.c_str(), gasVol, xp, yp, zp);
		break;
	}
	case 12: // ��Ţ��Ʈ 1�� ������ ���ڸ� ���
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
		prt(L"�߰��� ����ġ�� ���� �Է����ּ���.\n");
		std::cin >> expVal;
		Player::ins()->addTalentExp(expVal);
		break;
	}
	case 15://ž�� ���� ���� ȸ��
	{
		Vehicle* myCar = (Vehicle*)(ctrlVeh);
		myCar->rotate(ACW(myCar->bodyDir));
		break;
	}
	case 16://ž�� ���� ���� �����̵�
	{
		Vehicle* myCar = (Vehicle*)(ctrlVeh);
		int dx, dy;
		prt(L"�̵��� �Ÿ� dx�� �Է����ּ���.\n");
		std::cin >> dx;
		prt(L"�̵��� �Ÿ� dy�� �Է����ּ���.\n");
		std::cin >> dy;
		myCar->shift(dx, dy);
		break;
	}
	case 17://ž�� ���� ���� �̵�
	{
		Vehicle* myCar = (Vehicle*)(ctrlVeh);
		int dx, dy;
		prt(L"�̵��� �Ÿ� dx�� �Է����ּ���.\n");
		std::cin >> dx;
		prt(L"�̵��� �Ÿ� dy�� �Է����ּ���.\n");
		std::cin >> dy;
		myCar->rush(dx, dy);
		break;
	}
	case 18: //fov ���
	{
		for (int y = yp - 8; y <= yp + 8; y++)
		{
			for (int x = xp - 8; x <= xp + 8; x++)
			{
				if (World::ins()->getTile(x, y, zp).fov == fovFlag::white) prt(L"��");
				else if (World::ins()->getTile(x, y, zp).fov == fovFlag::gray) prt(L"��");
				else prt(L"��");
			}
			prt(L"\n");
		}
		prt(L"[�����] entityPtr ����� ���´�!\n");
		break;
	}
	case 19: //�ð� ���� ����
	{
		int tgtHour, tgtMin;
		prt(L"[1/2] �ٲ� �ð��� �Է����ּ���(0~23).\n");
		std::cin >> tgtHour;
		prt(L"[2/2] �ٲ� ���� �Է����ּ���(0~59)\n");
		std::cin >> tgtMin;
		setDebugTime(tgtHour, tgtMin);
		break;
	}
	case 20: //��¥ ���� ����
	{
		int tgtMonth, tgtDay;
		prt(L"[1/2] �ٲ� ���� �Է����ּ���(1~12).\n");
		std::cin >> tgtMonth;
		prt(L"[2/2] �ٲ� ���� �Է����ּ���.\n");
		std::cin >> tgtDay;
		setDebugDay(tgtMonth, tgtDay);
		break;
	}
	case 21: //���� ����
	{
		int cx, cy;
		World::ins()->changeToChunkCoord(Player::ins()->getGridX(), Player::ins()->getGridY(), cx, cy);

		int tgtWeather;
		prt(L" �ٲ� ������ �������ּ���.\n");
		prt(L" 1. ����\n");
		prt(L" 2. �帲\n");
		prt(L" 3. ��\n");
		prt(L" 4. õ��\n");
		prt(L" 5. ��\n");
		std::cin >> tgtWeather;
		if (tgtWeather == 1) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::sunny);
		else if (tgtWeather == 2) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::cloudy);
		else if (tgtWeather == 3) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::rain);
		else if (tgtWeather == 4) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::storm);
		else if (tgtWeather == 5) World::ins()->setChunkWeather(cx, cy, zp, weatherFlag::snow);
		prt(L" ������ ���������� �����ߴ�! \n");
		break;
	}
	case 22: //�� ����
	{
		Player* ptr = Player::ins();
		new Monster(4, ptr->getGridX() + 1, ptr->getGridY(), ptr->getGridZ());
		prt(L"[�����]���ο� ��ƼƼ�� �������!\n");
		break;
	}
	case 23: //�����̻� ����
	{
		int tgtEfctIndex = 0;
		int tgtEfctDur = 1;

		prt(L"[1/2] �߰��� �����̻��� �ε����� �Է����ּ���.\n");
		std::cin >> tgtEfctIndex;

		prt(L"[2/2] �߰��� �����̻��� �ϼ��� �Է����ּ���.\n");
		std::cin >> tgtEfctDur;

		prt(L"�����̻��� ���������� �߰��Ͽ���.\n");
		Player::ins()->entityInfo.statusEffects.push_back({ (statEfctFlag)tgtEfctIndex,tgtEfctDur });

		break;
	}
	}
}