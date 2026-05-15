module;
#include <atomic>
#include <cstdint>

export module sequenceCounter;

static std::atomic<std::uint64_t> itemCounter = 0;
static std::atomic<std::uint64_t> entityCounter = 0;

export std::uint64_t genItemID()
{
    return ++itemCounter;
}

export std::uint64_t genEntityID()
{
    return ++entityCounter;
}
