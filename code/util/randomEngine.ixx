export module randomEngine;

import std;

namespace
{
    static std::mutex& rngMutex()
    {
        static std::mutex m;
        return m;
    }

    static std::uint64_t& currentSeedRef()
    {
        static std::uint64_t s = 0;
        return s;
    }

    static std::mt19937_64& rng()
    {
        static std::mt19937_64 gen([] {
            std::random_device rd;
            std::uint64_t seed64 = ((std::uint64_t)rd() << 32) ^ (std::uint64_t)rd();
            currentSeedRef() = seed64;
            return seed64;
            }());
        return gen;
    }
}

export int randomRange(int a, int b)
{
    std::lock_guard lock(rngMutex());
    std::uniform_int_distribution<int> dis(a, b);
    return dis(rng());
}

export long long randomRangeLL(long long a, long long b)
{
    std::lock_guard lock(rngMutex());
    std::uniform_int_distribution<long long> dis(a, b);
    return dis(rng());
}

export double randomRangeFloat(double a, double b)
{
    std::lock_guard lock(rngMutex());
    std::uniform_real_distribution<double> dis(a, b);
    return dis(rng());
}

export std::uint64_t getSeed()
{
    (void)rng();
    return currentSeedRef();
}

export template<typename T>
void randomVectorShuffle(std::vector<T>& vec)
{
    if (vec.size() <= 1) return;

    for (size_t i = vec.size() - 1; i > 0; --i)
    {
        size_t j = static_cast<size_t>(randomRangeLL(0, static_cast<long long>(i)));
        std::swap(vec[i], vec[j]);
    }
}