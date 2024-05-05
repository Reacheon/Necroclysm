export module aStar;

import std;
import hasher;
import prt;

export int aStar(std::set<std::array<int, 2>> walkableTile, int playerX, int playerY, int dstX, int dstY)//(x2,y2)는 A*을 계산할 객체의 위치고 (x1,y1)은 도착지, 성공하면 최단거리로의 방향을 반환, 아니면 -1 반환
{
	//prt(L"////////////////////////////aStar 알고리즘 실행됨/////////////////////////////\n");
	//prt(L"이 엔티티의 현재 위치는 (%d,%d)이고 목적지는 (%d,%d)이다\n",playerX,playerY, dstX, dstY);

	//에러 : 목표가 바로 옆에 있는데도 이동 명령을 내림
	if (dstX == playerX && dstY == playerY) return -1;
	else if (std::abs(playerX - dstX) <= 1 && std::abs(playerY - dstY) <= 1)
	{
		if (dstX - playerX == 1 && dstY - playerY == 0) return 4;
		else if (dstX - playerX == 1 && dstY - playerY == -1) return 5;
		else if (dstX - playerX == 0 && dstY - playerY == -1) return 6;
		else if (dstX - playerX == -1 && dstY - playerY == -1) return 7;
		else if (dstX - playerX == -1 && dstY - playerY == 0) return 0;
		else if (dstX - playerX == -1 && dstY - playerY == 1) return 1;
		else if (dstX - playerX == 0 && dstY - playerY == 1) return 2;
		else return 3;
	}

	walkableTile.insert({ playerX,playerY });

	std::unordered_map<std::array<int, 2>, int, decltype(arrayHasher2)> valG;
	std::unordered_map<std::array<int, 2>, int, decltype(arrayHasher2)> valH;
	std::unordered_map<std::array<int, 2>, int, decltype(arrayHasher2)> valF;
	std::unordered_map<std::array<int, 2>, int, decltype(arrayHasher2)> valDir;

	std::unordered_set<std::array<int, 2>, decltype(arrayHasher2)> openSet;
	std::unordered_set<std::array<int, 2>, decltype(arrayHasher2)> closeSet;

	//시작점을 오픈리스트에 넣음
	int pivotX = dstX;
	int pivotY = dstY;
	bool pathFind = false;
	valG[{pivotX, pivotY}] = 0;

	std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>* openQueue = new std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>;
	std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>* closeQueue = new std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, std::greater<std::array<int, 3>>>;

	std::array<int, 3> tempPair;
	tempPair = { 1, dstX, dstY };
	openQueue->push(tempPair);
	openSet.insert({ dstX, dstY });

	int dx, dy, dg, ddir;
	int stack = 0;
	//오픈리스트가 하나라도 남아있으면
	while (openQueue->empty() == false && pathFind == false)
	{
		stack++;
		tempPair = openQueue->top();
		pivotX = tempPair[1];
		pivotY = tempPair[2];

		//오픈리스트에서 피벗 위치를 닫힌 리스트에 넣고 오픈리스트에서 제거함
		closeQueue->push(tempPair);//클로즈큐에 추가
		closeSet.insert({ pivotX,pivotY });
		openQueue->pop();//오픈큐에서 제거
		openSet.erase({ pivotX,pivotY });

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


			if (walkableTile.find({ pivotX + dx, pivotY + dy }) != walkableTile.end())//해당 방향의 사각형이 오픈리스트에 있거나 현재 오픈리스트가 0개인지 체크함
			{
				//prt(L"aStar : (%d,%d) 타일은 이동가능 타일이다.\n", pivotX + dx, pivotY + dy);
				if (closeSet.find({ pivotX + dx, pivotY + dy }) == closeSet.end())
				{
					if (openSet.find({ pivotX + dx, pivotY + dy }) != openSet.end())//이미 열린 리스트면 현재 위치에서 G값을 계산하고 원래 G값보다 낮으면 갱신함
					{
						//prt(L"aStar : (%d,%d) 타일은 이미 오픈리스트이다.\n", pivotX + dx, pivotY + dy);
						if (valG[{pivotX, pivotY}] + dg < valG[{pivotX + dx, pivotY + dy}])
						{
							//prt(L"aStar : (%d,%d) 타일이 갱신되었다.\n", pivotX + dx, pivotY + dy);
							valG.erase(valG.find({ pivotX + dx, pivotY + dy }));
							valH.erase(valH.find({ pivotX + dx, pivotY + dy }));
							valF.erase(valF.find({ pivotX + dx, pivotY + dy }));
							valDir.erase(valDir.find({ pivotX + dx, pivotY + dy }));

							valDir[{ pivotX + dx, pivotY + dy }] = ddir;
							//prt(L"aStar : (%d,%d) 타일의 방향은 %d이다.\n", pivotX + dx, pivotY + dy, (valDir[{ pivotX + dx, pivotY + dy }]));
							valH[{ pivotX + dx, pivotY + dy }] = 10 * (std::abs((pivotX + dx) - playerX) + std::abs((pivotY + dy) - playerY));
							valG[{ pivotX + dx, pivotY + dy }] = valG[{pivotX, pivotY}] + dg;
							valF[{ pivotX + dx, pivotY + dy }] = valG[{pivotX + dx, pivotY + dy}] + valH[{pivotX + dx, pivotY + dy}];
						}
						else
						{
							//prt(L"aStar : (%d,%d) 타일의 갱신에 실패하였다.\n", pivotX + dx, pivotY + dy);
						}
					}
					else//열린 리스트에도 없고 닫힌 리스트에도 없으면 새로 오픈리스트에 넣고 FGH를 새로 추가함
					{
						//prt(L"aStar : (%d,%d) 타일이 새로운 오픈리스트에 추가되었다.\n", pivotX + dx, pivotY + dy);
						int toX = pivotX + dx;
						int toY = pivotY + dy;

						//열린 리스트에 피벗X+1와 피벗Y를 추가하고 추가한 노드에 FGH를 추가함
						valDir[{ pivotX + dx, pivotY + dy }] = ddir;
						valH[{ pivotX + dx, pivotY + dy }] = 10 * (std::abs((toX)-playerX) + std::abs((toY)-playerY));
						valG[{ pivotX + dx, pivotY + dy }] = valG[{pivotX, pivotY}] + dg;
						valF[{ pivotX + dx, pivotY + dy }] = valG[{pivotX + dx, pivotY + dy}] + valH[{pivotX + dx, pivotY + dy}];
						//prt(L"aStar : (%d,%d) 타일의 방향은 %d이다.\n", pivotX + dx, pivotY + dy, (valDir[{ pivotX + dx, pivotY + dy }]));

						tempPair = { valF[{ toX, toY }], toX, toY };
						openQueue->push(tempPair);
						openSet.insert({ toX,toY });

						if (toX == playerX && toY == playerY)//목적지가 열린리스트에 추가되었으면
						{
							pathFind = true;
							break;//반복문 탈출
						}
					}
				}

			}
			else
			{
				//prt(L"aStar : (%d,%d) 타일은 이동불가 타일이다.\n", pivotX + dx, pivotY + dy);
			}
		}
	}

	delete openQueue;
	delete closeQueue;

	if (pathFind == true)
	{
		//prt(L"%d개의 오픈리스트를 연산에 이용했다.\n", stack);
		//prt(L"현재 위치에서 %d 방향으로 이동하기 시작한다.\n", valDir[{ playerX, playerY }]);
		//prt(L"////////////////////////////aStar 알고리즘 종료됨/////////////////////////////\n");
		
		return (valDir[{ playerX, playerY }]+4)%8;
	}
	else
	{
		//prt(L"[디버그] 길찾기 실패\n");
		//prt(L"////////////////////////////aStar 알고리즘 종료됨/////////////////////////////\n");
		return -1;
	}
}