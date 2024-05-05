export module dirToXY;

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