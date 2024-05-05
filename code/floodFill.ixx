export module floodFill;

import std;

struct pairHash
{
	template <class T1, class T2>
	std::size_t operator () (std::pair<T1, T2> const& pair) const
	{
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};


//@biref floodFill을 실행해 이동 가능한 타일을 반환합니다.
//@return 플루드필, 해당 타일에 도착했을 경우 참 반환
export int floodFill(std::unordered_set<std::pair<int,int>, pairHash> walkableTile, int startGridX, int startGridY, int limit, int endGridX, int endGridY)
{
	//walkableTile이 자료구조에 없으면 걸을 수 없는 취급
	std::unordered_set<std::pair<int, int>, pairHash> floodColor;

	floodColor.insert({ startGridX,startGridY });
	int floodNumber = 1; //색칠된 타일의 수
	std::queue<int> qX;
	std::queue<int> qY;

	bool hasEndPoint;
	if (endGridX != -1 || endGridY != -1) { hasEndPoint = false; }
	else { hasEndPoint = true; }

	//시작 지점을 큐에 넣음
	qX.push(startGridX);
	qY.push(startGridY);
	int floodX, floodY = -1;
	int dx,dy = 0;
	while (qX.empty() == false) //큐가 없어질 때까지 계속
	{
		//큐의 앞을 빼내어 타겟으로 잡음
		floodX = qX.front();
		qX.pop();
		floodY = qY.front();
		qY.pop();

		//타겟 타일 주변 8칸 조사
		for (int i = 0; i < 8; i++)
		{
			switch (i)
			{
				case 0: dx = 1; dy = 0; break;
				case 1: dx = 1; dy = -1; break;
				case 2: dx = 0; dy = -1; break;
				case 3: dx = -1; dy = -1; break;
				case 4: dx = -1; dy = 0; break;
				case 5: dx = -1; dy = 1; break;
				case 6: dx = 0; dy = 1; break;
				case 7: dx = 1; dy = 1; break;
			}

			if (walkableTile.find({ floodX + dx, floodY + dy }) != walkableTile.end())
			{
				if (floodColor.find({ floodX + dx, floodY + dy }) != floodColor.end())
				{
					if (hasEndPoint == true) //도착지점이 -1일 경우 노드에 도착점 추가 시에 강제 종료
					{
						if (floodX + dx == endGridX && floodY + dy == endGridY)
						{
							return true;
						}
					}
				}
				floodColor.insert({ floodX + dx, floodY + dy });
				floodNumber++;
				//지정된 값 이상으로 floodFill 발생 시 함수 강제 종료
				if (floodNumber > limit) { return false; }
				qX.push(floodX + dx);
				qY.push(floodY + dy);
			}
		}
	}

	//탐색 종료
	if (hasEndPoint == true)
	{
		return false;
	}
	else
	{
		return floodNumber;
	}

}

//@
//@biref floodFill을 실행해 이동 가능한 타일을 반환합니다.목적지 없음
export int floodFill(std::unordered_set<std::pair<int, int>, pairHash> walkableTile, int startGridX, int startGridY, int limit)
{
	return floodFill(walkableTile, startGridX, startGridY, limit, -1, -1);
}