export module dirToXY;

import std;

export void dir2Coord(int dir, int& dx, int& dy)
{
	switch (dir)
	{
		case 0:dx = 1; dy = 0; break;
		case 1:dx = 1; dy = -1; break;
		case 2:dx = 0; dy = -1; break;
		case 3:dx = -1; dy = -1; break;
		case 4:dx = -1; dy = 0; break;
		case 5:dx = -1; dy = 1; break;
		case 6:dx = 0; dy = 1; break;
		case 7:dx = 1; dy = 1; break;
		default: dx = 0; dy = 0; break;
	}
}


export int coord2Dir(int dx, int dy)
{
	if (dx == 1 && dy == 0) { return 0; }
	else if (dx == 1 && dy == -1) { return 1; }
	else if (dx == 0 && dy == -1) { return 2; }
	else if (dx == -1 && dy == -1) { return 3; }
	else if (dx == -1 && dy == 0) { return 4; }
	else if (dx == -1 && dy == 1) { return 5; }
	else if (dx == 0 && dy == 1) { return 6; }
	else if (dx == 1 && dy == 1) { return 7; }
	else { return -1; }
}

export int del2Dir(float delX, float delY)
{
	if (delX == 0)
	{
		if (delY > 0) return 2;
		else if (delY < 0) return 6;
		else
		{
			std::wprintf(L"del2Dir에서 잘못된 기울기 값이 계산되었다. 원점을 좌표로 입력했다.\n");
			std::abort();
		}
	}

	float m = (-delY) / delX;

	if (delX >= 0)
	{
		if (-0.5 <= m && m < 0.5) return 0;
		else if (0.5 <= m && m < 2) return 1;
		else if (-2 <= m && m < -0.5) return 7;
		else if (m < -2) return 6;
		else if (m >= 2) return 2;
		else
		{
			std::wprintf(L"del2Dir에서 잘못된 기울기 값이 계산되었다.\n");
			std::abort();
		}
	}
	else
	{
		if (-0.5 <= m && m < 0.5) return 4;
		else if (0.5 <= m && m < 2) return 5;
		else if (-2 <= m && m < -0.5) return 3;
		else if (m < -2) return 2;
		else if (m >= 2) return 6;
		else
		{
			std::wprintf(L"del2Dir에서 잘못된 기울기 값이 계산되었다.\n");
			std::abort();
		}
	}
}