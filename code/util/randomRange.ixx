export module randomRange;

import std;

std::random_device rd;
std::mt19937 gen(rd());

export int randomRange(int a, int b)
{
	std::uniform_int_distribution<int> dis(a, b);
	return dis(gen);
}


export double randomRangeFloat(double a, double b)
{
    double fraction = static_cast<double>(gen()) / static_cast<double>(gen.max());
    return a + fraction * (b - a);
}


export int rollDice(int numDice, int sides)
{
    if (numDice <= 0 || sides <= 0) return 0;

    std::uniform_int_distribution<int> dis(1, sides);
    int total = 0;

    for (int i = 0; i < numDice; ++i) {
        total += dis(gen);
    }

    return total;
}