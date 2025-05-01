#include <atomic>

export module sequenceCounter;

static std::atomic<unsigned __int64> itemCounter = 0;
static std::atomic<unsigned __int64> entityCounter = 0;

export unsigned __int64 genItemID()
{
    return ++itemCounter;
}

export unsigned __int64 genEntityID()
{
    return ++entityCounter;
}
