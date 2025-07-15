export module aStar;

import std;
import Point;
import hasher;
import prt;
import nanoTimer;
import timeKeeper;
import dirToXY;

export std::vector<Point2> aStar(std::unordered_set<Point2, Point2::Hash> walkableTile, int playerX, int playerY, int dstX, int dstY)
{
	//prt(L"////////////////////////////aStar 알고리즘 실행됨/////////////////////////////\n");
	//prt(L"이 엔티티의 현재 위치는 (%d,%d)이고 목적지는 (%d,%d)이다\n",playerX,playerY, dstX, dstY);

	Point2 playerPos(playerX, playerY);
	Point2 dstPos(dstX, dstY);

	//에러 : 목표가 바로 옆에 있는데도 이동 명령을 내림
	if (dstPos == playerPos) return {};
	else if (std::abs(playerX - dstX) <= 1 && std::abs(playerY - dstY) <= 1)
	{
		std::vector<Point2> trail;
		trail.push_back(playerPos);
		trail.push_back(dstPos);
		return trail;
	}

	walkableTile.insert(playerPos);

	std::unordered_map<Point2, int, Point2::Hash> valG;
	std::unordered_map<Point2, int, Point2::Hash> valH;
	std::unordered_map<Point2, int, Point2::Hash> valF;
	std::unordered_map<Point2, int, Point2::Hash> valDir;

	std::unordered_set<Point2, Point2::Hash> openSet;
	std::unordered_set<Point2, Point2::Hash> closeSet;

	//시작점을 오픈리스트에 넣음
	Point2 pivot = dstPos;
	bool pathFind = false;
	valG[pivot] = 0;

	std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>* openQueue = new std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>;
	std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>* closeQueue = new std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>;

	std::array<int, 3> tempPair;
	tempPair = { 1, dstX, dstY };
	openQueue->push(tempPair);
	openSet.insert(dstPos);

	int dx, dy, dg, ddir;
	int stack = 0;
	//오픈리스트가 하나라도 남아있으면
	while (openQueue->empty() == false && pathFind == false)
	{
		stack++;
		tempPair = openQueue->top();
		pivot.set(tempPair[1], tempPair[2]);

		//오픈리스트에서 피벗 위치를 닫힌 리스트에 넣고 오픈리스트에서 제거함
		closeQueue->push(tempPair);//클로즈큐에 추가
		closeSet.insert(pivot);
		openQueue->pop();//오픈큐에서 제거
		openSet.erase(pivot);

		//피벗 위치에서 인접한 8 사각형이 열린 리스트에 없으면 추가함
		//이미 열린 리스트면 G값을 보고 더 낮으면 갱신함
		for (int m = 0; m < 8; m++)
		{
			switch (m)
			{
			case 0: dx = 1; dy = 0; dg = 10; ddir = 4; break;
			case 1: dx = 1; dy = -1; dg = 14; ddir = 5; break;
			case 2: dx = 0; dy = -1; dg = 10; ddir = 6; break;
			case 3: dx = -1; dy = -1; dg = 14; ddir = 7; break;
			case 4: dx = -1; dy = 0; dg = 10; ddir = 0; break;
			case 5: dx = -1; dy = 1; dg = 14; ddir = 1; break;
			case 6: dx = 0; dy = 1; dg = 10; ddir = 2; break;
			case 7: dx = 1; dy = 1; dg = 14; ddir = 3; break;
			}

			Point2 nextPos(pivot.x + dx, pivot.y + dy);

			if (walkableTile.find(nextPos) != walkableTile.end())//해당 방향의 사각형이 오픈리스트에 있거나 현재 오픈리스트가 0개인지 체크함
			{
				//prt(L"aStar : (%d,%d) 타일은 이동가능 타일이다.\n", nextPos.x, nextPos.y);
				if (closeSet.find(nextPos) == closeSet.end())
				{
					if (openSet.find(nextPos) != openSet.end())//이미 열린 리스트면 현재 위치에서 G값을 계산하고 원래 G값보다 낮으면 갱신함
					{
						//prt(L"aStar : (%d,%d) 타일은 이미 오픈리스트이다.\n", nextPos.x, nextPos.y);
						if (valG[pivot] + dg < valG[nextPos])
						{
							//prt(L"aStar : (%d,%d) 타일이 갱신되었다.\n", nextPos.x, nextPos.y);
							valG.erase(valG.find(nextPos));
							valH.erase(valH.find(nextPos));
							valF.erase(valF.find(nextPos));
							valDir.erase(valDir.find(nextPos));

							valDir[nextPos] = ddir;
							//prt(L"aStar : (%d,%d) 타일의 방향은 %d이다.\n", nextPos.x, nextPos.y, valDir[nextPos]);
							valH[nextPos] = 10 * (std::abs(nextPos.x - playerX) + std::abs(nextPos.y - playerY));
							valG[nextPos] = valG[pivot] + dg;
							valF[nextPos] = valG[nextPos] + valH[nextPos];
						}
						else
						{
							//prt(L"aStar : (%d,%d) 타일의 갱신에 실패하였다.\n", nextPos.x, nextPos.y);
						}
					}
					else//열린 리스트에도 없고 닫힌 리스트에도 없으면 새로 오픈리스트에 넣고 FGH를 새로 추가함
					{
						//prt(L"aStar : (%d,%d) 타일이 새로운 오픈리스트에 추가되었다.\n", nextPos.x, nextPos.y);

						//열린 리스트에 피벗X+1와 피벗Y를 추가하고 추가한 노드에 FGH를 추가함
						valDir[nextPos] = ddir;
						valH[nextPos] = 10 * (std::abs(nextPos.x - playerX) + std::abs(nextPos.y - playerY));
						valG[nextPos] = valG[pivot] + dg;
						valF[nextPos] = valG[nextPos] + valH[nextPos];
						//prt(L"aStar : (%d,%d) 타일의 방향은 %d이다.\n", nextPos.x, nextPos.y, valDir[nextPos]);

						tempPair = { valF[nextPos], nextPos.x, nextPos.y };
						openQueue->push(tempPair);
						openSet.insert(nextPos);

						if (nextPos == playerPos)//목적지가 열린리스트에 추가되었으면
						{
							pathFind = true;
							break;//반복문 탈출
						}
					}
				}

			}
			else
			{
				//prt(L"aStar : (%d,%d) 타일은 이동불가 타일이다.\n", nextPos.x, nextPos.y);
			}
		}
	}

	delete openQueue;
	delete closeQueue;

	if (pathFind == true)
	{
		//prt(L"%d개의 오픈리스트를 연산에 이용했다.\n", stack);
		//prt(L"현재 위치에서 %d 방향으로 이동하기 시작한다.\n", valDir[playerPos]);
		//prt(L"////////////////////////////aStar 알고리즘 종료됨/////////////////////////////\n");
		std::vector<Point2> trail;
		Point2 current = playerPos;
		while (true)
		{
			trail.push_back(current);
			if (current == dstPos) break;

			int dx = 0, dy = 0;
			dir2Coord(valDir[current], dx, dy);
			current.x += dx;
			current.y += dy;
		}

		return trail;
	}
	else
	{
		//prt(L"[디버그] 길찾기 실패\n");
		//prt(L"////////////////////////////aStar 알고리즘 종료됨/////////////////////////////\n");
		return {};
	}
}