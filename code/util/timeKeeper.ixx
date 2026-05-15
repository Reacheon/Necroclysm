export module timeKeeper;

import std;
import nanoTimer;

export void prtTime(std::int64_t startTime, const char* funcName)
{
    auto currentTime = getNanoTimer();
    auto elapsedTime = currentTime - startTime;
    std::printf("%s: %f ms (%lld ns)\n", funcName, static_cast<double>(elapsedTime) / 1000000.0, elapsedTime);
}

export void prtTimeAvg(std::int64_t startTime, const char* funcName, int count)
{
    static std::vector<std::int64_t> elapsedTimes;
    static std::int64_t maxTime = 0;
    static int currentCount = 0;

    std::int64_t currentTime = getNanoTimer();
    std::int64_t elapsedTime = currentTime - startTime;

    if (elapsedTime > maxTime)
    {
        maxTime = elapsedTime;
    }

    elapsedTimes.push_back(elapsedTime);
    currentCount++;

    if (currentCount == count)
    {
        double totalTime = 0.0;
        for (const auto& time : elapsedTimes)
        {
            totalTime += static_cast<double>(time);
        }

        double averageTime = totalTime / count;

        std::printf("%s: Max Time = %f ms (%lld ns), Average Time = %f ms\n",
            funcName,
            static_cast<double>(maxTime) / 1000000.0,
            maxTime,
            averageTime / 1000000.0);

        // Reset for next series of measurements
        elapsedTimes.clear();
        maxTime = 0;
        currentCount = 0;
    }
}
