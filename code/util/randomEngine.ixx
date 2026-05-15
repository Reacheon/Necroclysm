export module randomEngine;

import std;

namespace
{
    static std::mutex& rngMutex()
    {
        static std::mutex m;
        return m;
    }

    static std::uint64_t& seedRef()
    {
        static std::uint64_t s = 0;
        return s;
    }

    static std::mt19937_64& rng()
    {
        static std::mt19937_64 gen;
        return gen;
    }
}

//@brief 게임 초기 시작 시에 최초 1회 실행
export void initRandomEngine()
{
    std::random_device rd;
    std::uint64_t seed64 = ((std::uint64_t)rd() << 32) ^ (std::uint64_t)rd();
    std::lock_guard lock(rngMutex());
    seedRef() = seed64;
    rng().seed(seed64);
}

export int randomRange(int a, int b)
{
    if (a > b) std::swap(a, b);
    std::lock_guard lock(rngMutex());
    std::uniform_int_distribution<int> dis(a, b);
    return dis(rng());
}

export std::int64_t randomRangeLL(std::int64_t a, std::int64_t b)
{
    if (a > b) std::swap(a, b);
    std::lock_guard lock(rngMutex());
    std::uniform_int_distribution<std::int64_t> dis(a, b);
    return dis(rng());
}

export double randomRangeFloat(double a, double b)
{
    if (a > b) std::swap(a, b);
    std::lock_guard lock(rngMutex());
    std::uniform_real_distribution<double> dis(a, b);
    return dis(rng());
}

export std::uint64_t getSeed()
{
    return seedRef();
}

export template<typename T>
void randomVectorShuffle(std::vector<T>& vec)
{
    if (vec.size() <= 1) return;

    for (size_t i = vec.size() - 1; i > 0; --i)
    {
        size_t j = static_cast<size_t>(randomRangeLL(0, static_cast<std::int64_t>(i)));
        std::swap(vec[i], vec[j]);
    }
}
