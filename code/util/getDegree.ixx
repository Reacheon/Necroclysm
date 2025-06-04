export module getDegree;

import std;

export float getDegree(int x1, int y1, int x2, int y2)
{
	int xd = x2 - x1;
	int yd = y2 - y1;
	float degree = std::atan2(yd, xd) * 180 / 3.141592;
	if (degree > 0){degree = 360 - degree;}
	else{degree *= -1;}
	return degree;
}

export int getIntDegree(int x1, int y1, int x2, int y2)
{
	//prt(L"\033[0;31m(%d,%d)에서 (%d,%d) 사이의 각도는 %f입니다.\033[0m\n",x1,y1,x2,y2, getDegree(x1, y1, x2, y2));
	if (getDegree(x1, y1, x2, y2) < 22.5 || 337.5 <= getDegree(x1, y1, x2, y2)) { return 0; }
	else if (22.5 <= getDegree(x1, y1, x2, y2)&& getDegree(x1, y1, x2, y2) < 67.5) { return 1; }
	else if (67.5 <= getDegree(x1, y1, x2, y2) && getDegree(x1, y1, x2, y2) < 112.5) { return 2; }
	else if (112.5 <= getDegree(x1, y1, x2, y2) && getDegree(x1, y1, x2, y2) < 157.5) { return 3; }
	else if (157.5 <= getDegree(x1, y1, x2, y2) && getDegree(x1, y1, x2, y2) < 202.5) { return 4; }
	else if (202.5 <= getDegree(x1, y1, x2, y2) && getDegree(x1, y1, x2, y2) < 247.5) { return 5; }
	else if (247.5 <= getDegree(x1, y1, x2, y2) && getDegree(x1, y1, x2, y2) < 292.5) { return 6; }
	else if (292.5 <= getDegree(x1, y1, x2, y2) && getDegree(x1, y1, x2, y2) < 337.5) { return 7; }
	else { return -1; }
}