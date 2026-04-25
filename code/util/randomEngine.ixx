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
    if (a > b) std::swap(a, b);
    std::lock_guard lock(rngMutex());
    std::uniform_int_distribution<int> dis(a, b);
    return dis(rng());
}

export long long randomRangeLL(long long a, long long b)
{
    if (a > b) std::swap(a, b);
    std::lock_guard lock(rngMutex());
    std::uniform_int_distribution<long long> dis(a, b);
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
    (void)rng();
    return currentSeedRef();
}

// 월드 생성용 고정 시드. 게임 시작 시 랜덤하게 결정될 예정이나 현재는 고정값
// 도시 배치/도로/건물 등 월드젠이 이 시드로부터 결정론적으로 파생됨
export inline std::uint64_t worldSeed = 0xC0DEBABE0000CAFEULL;

// 좌표 해시 기반 서브시드: 동일 좌표는 항상 동일 시드 생성
// 재현성을 위해 worldSeed와 좌표만으로 결정됨 (RNG 전역 상태 미사용)
export std::uint64_t hashSeed(std::int64_t a, std::int64_t b, std::int64_t c = 0)
{
    std::uint64_t h = worldSeed;
    auto mix = [&](std::int64_t v) {
        std::uint64_t x = static_cast<std::uint64_t>(v);
        x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        h ^= x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    };
    mix(a); mix(b); mix(c);
    return h;
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