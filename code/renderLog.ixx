#include <SDL3/SDL.h>
export module renderLog;
import std;
import util;
import constVar;
import drawText;
import globalVar;
import log;

export __int64 renderLog(SDL_Renderer* renderer)
{
	__int64 timeStampStart = getNanoTimer();//시간 측정 시작
	std::deque<std::wstring> logStrDeque = getLogStrDeque();
	std::deque<int> logTimerDeque = getLogTimerDeque();

	setFont(fontType::mainFont);
	setFontSize(20); // 픽셀 폰트는 12pt 고정

	int textHeight = queryTextHeight(L"가", true);
	int pivotX = 12;
	int pivotY = -180;

	for (int i = 0; i < maxLogLine; i++)
	{
		if (logTimerDeque[i] > 0)
		{
			std::wstring shadowText = removeColorCodes(logStrDeque[i]);

			// 그림자 (4방향, 1.5배 오프셋)
			drawText(shadowText, pivotX-1, cameraH + pivotY - textHeight * i, col::black); // 5-1→6 (왼쪽)
			drawText(shadowText, pivotX+1, cameraH + pivotY - textHeight * i, col::black); // 5+1→9 (오른쪽) - 2픽셀 간격
			drawText(shadowText, pivotX, cameraH + pivotY - textHeight * i - 2, col::black); // 5, -1→8, -2 (위)
			drawText(shadowText, pivotX, cameraH + pivotY - textHeight * i + 2, col::black); // 5, +1→8, +2 (아래)

			// 본문
			drawText(logStrDeque[i], pivotX, cameraH + pivotY - textHeight * i); // 5→8, 158→237 (×1.5)

			if (stopLog == false)
			{
				minusLogTimerDeque(i);
			}
		}
	}
	return (getNanoTimer() - timeStampStart);
}