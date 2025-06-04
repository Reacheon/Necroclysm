export module isCircle;

import std;

namespace std
{
	template<>
	struct hash<std::array<int, 2>> 
	{
		std::size_t operator()(const std::array<int, 2>& arr) const noexcept 
		{
			std::size_t seed = 0;
			for (const auto& elem : arr) seed ^= std::hash<int>{}(elem)+0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
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

export void initCircle()
{
	std::unordered_set<std::array<int, 2>> blank;
	circleVec.push_back(blank);
	for (int i = 1; i < 70; i++) circleVec.push_back(makeCircle(i, false));
}

export bool isCircle(int range, int revX, int revY)
{
	if (circleVec[range].find({ revX,revY }) != circleVec[range].end()) return true;
	else return false;
}

