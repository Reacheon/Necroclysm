export module log;

import std;
import util;
import drawText;
import globalVar;

export constexpr int initialTimer = 180;
export constexpr int maxLogLine = 8;

static std::deque<std::wstring> logStrDeque; // 로그 문자가 저장되는 디큐
static std::deque<int> logTimerDeque; // 로그 문자들의 화면 나타나는 시간이 저장되는 디큐

export const std::deque<std::wstring>& getLogStrDeque() { return logStrDeque; }
export const std::deque<int>& getLogTimerDeque() { return logTimerDeque; }
export void minusLogTimerDeque(int index) { logTimerDeque[index]--; }

export void initLog()
{
    for (int i = 0; i < maxLogLine; i++)
    {
        logStrDeque.push_back(L"Dummy");
        logTimerDeque.push_back(0);
    }
}

export void updateLog(std::wstring text)
{
    
    prt(L"Run updateLog function\n");
    std::array<std::wstring, 2> arr = textSplitter(text, 450);
    if (arr[1] != L"")
    {
        updateLog(arr[0]);
        updateLog(arr[1]);
    }
    else
    {
        logStrDeque.pop_back();
        logTimerDeque.pop_back();
        logStrDeque.push_front(arr[0]);
        logTimerDeque.push_front(initialTimer);
    }
}

export void clearLog()
{
    logStrDeque.clear();
    logTimerDeque.clear();

    for (int i = 0; i < maxLogLine; i++)
    {
        logStrDeque.push_back(L"Dummy");
        logTimerDeque.push_back(0);
    }
}
