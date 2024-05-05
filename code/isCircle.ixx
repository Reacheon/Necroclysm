export module isCircle;

import std;

namespace std {
	template<>
	struct hash<std::array<int, 2>> {
		std::size_t operator()(const std::array<int, 2>& arr) const noexcept {
			std::size_t h1 = std::hash<int>{}(arr[0]);
			std::size_t h2 = std::hash<int>{}(arr[1]);
			return h1 ^ (h2 << 1);
		}
	};
}

static bool firstExecute = true;
static std::vector<std::unordered_set<std::array<int, 2>>> circleVec;//원의 모양이 저장된 템플릿

std::unordered_set<std::array<int, 2>>  makeCircle(int r, bool outline)
{
	std::unordered_set<std::array<int,2>> circle;

	int cx=0;
	int cy = 0;
	int x0 = 0;
	int y0 = (r-1);
	int p = 1 - (r-1);
	int a,b;

	//상하좌우에 점 하나씩 찍음

	a = x0 + cx;
	b = y0 + cy;
	circle.insert({ a,b });
	a = x0 + cx;
	b = -y0 + cy;
	circle.insert({ a,b });
	a = y0 + cx;
	b = x0 + cy;
	circle.insert({ a,b });
	a = -y0 + cx;
	b = x0 + cy;
	circle.insert({ a,b });

	x0 = 1;
	while (x0 < y0)
	{
		if (p < 0)
		{
			p += x0 + x0 + 1;
		}
		else
		{
			p += x0 + x0 + 1 - y0 - y0;
			y0--;

			if (outline == false)
			{
				for (int i = 0; i < x0; i++)
				{
					a = i + cx;
					b = y0 + cy;
					circle.insert({ a,b });
					a = i + cx;
					b = -y0 + cy;
					circle.insert({ a,b });
					a = -i + cx;
					b = y0 + cy;
					circle.insert({ a,b });
					a = -i + cx;
					b = -y0 + cy;
					circle.insert({ a,b });
					a = y0 + cx;
					b = i + cy;
					circle.insert({ a,b });
					a = y0 + cx;
					b = -i + cy;
					circle.insert({ a,b });
					a = -y0 + cx;
					b = i + cy;
					circle.insert({ a,b });
					a = -y0 + cx;
					b = -i + cy;
					circle.insert({ a,b });
				}
			}
		}


		a = x0 + cx;
		b = y0 + cy;
		circle.insert({ a,b });

		a = x0 + cx;
		b = -y0 + cy;
		circle.insert({ a,b });
		a = -x0 + cx;
		b = y0 + cy;
		circle.insert({ a,b });
		a = -x0 + cx;
		b = -y0 + cy;
		circle.insert({ a,b });
		a = y0 + cx;
		b = x0 + cy;
		circle.insert({ a,b });
		a = y0 + cx;
		b = -x0 + cy;
		circle.insert({ a,b });
		a = -y0 + cx;
		b = x0 + cy;
		circle.insert({ a,b });
		a = -y0 + cx;
		b = -x0 + cy;
		circle.insert({ a,b });

		x0++;
	}


	if (outline == false)
	{
		for (int i = cy - y0 + 1; i < cy + y0; i++)
		{
			for (int j = cx - y0 + 1; j < cx + y0; j++)
			{
				a = j;
				b = i;
				circle.insert({ a,b });
			}
		}
	}


	return circle;
}

export bool isCircle(int range, int revX, int revY)
{
	//if (firstExecute == true)//최초 실행 make Circle Template
	//{
	//	circleVec.push_back({ 0,0 });
	//	const int maxRange = 15;
	//	for (int i = 1; i < maxRange; i++) circleVec.push_back(makeCircle(i, false));
	//	firstExecute = false;
	//}

	if (range >= circleVec.size()) //범위를 넘을 경우 동적 할당
	{
		for(int i = circleVec.size(); i <= range; i++) circleVec.push_back(makeCircle(i, false));
	}

	
	if (circleVec[range].find({ revX,revY }) != circleVec[range].end()) return true;
	else return false;
}

