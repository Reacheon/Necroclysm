export module log;

import std;
import util;
import drawText;
import globalVar;

export constexpr int initialTimer = 180;
export constexpr int maxLogLine = 8;

export constexpr int LOG_X_LOWER_BOUND = 378;
export constexpr int LOG_Y_LOWER_BOUND = 72;

static std::deque<std::wstring> logStrDeque; // 로그 문자가 저장되는 디큐
static std::deque<int> logTimerDeque; // 로그 문자들의 화면 나타나는 시간이 저장되는 디큐

export class Log 
{
public:
    int x, y, alpha, lifetime, layer;
    std::wstring text;
    bool dead = false;

    void step()
    {
        if(lifetime>0) lifetime--;

        
        if (lifetime == 0)
        {
            x+=2;
            alpha -= 8;
            if (alpha < 0) alpha = 0;
            if (x > cameraW - LOG_X_LOWER_BOUND + 30) dead = true;
        }
        else if (layer <= 3)
        {
            if (x > cameraW - LOG_X_LOWER_BOUND) x-=3;
        }
        else
        {
            x += 2;
            alpha -= 8;
            if (alpha < 0) alpha = 0;

            if (x > cameraW - LOG_X_LOWER_BOUND + 30) dead = true;
        }


        if (y > (cameraH / 2) + LOG_Y_LOWER_BOUND - 90 * layer) y-=8;
    }
};

static std::deque<Log> logMagazine;
export const std::deque<Log>& getLogMagazine() { return logMagazine; }

export void updateLog(std::wstring text)
{
    prt(L"Run updateLog function\n");

    for (auto& logElem : logMagazine) logElem.layer++;

    Log log;
    log.x = cameraW - LOG_X_LOWER_BOUND + 30;
    log.y = (cameraH/2) + LOG_Y_LOWER_BOUND;
    log.text = std::move(text);
    log.lifetime = initialTimer;
    log.alpha = 255;
    log.layer=0;
    logMagazine.push_back(std::move(log));
}

export void stepLogs()
{
    for (auto& log : logMagazine) log.step();
    std::erase_if(logMagazine, [](const Log& l) { return l.dead; });
}
