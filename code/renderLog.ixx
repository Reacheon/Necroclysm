module;
#include <SDL3/SDL.h>
export module renderLog;
import std;
import util;
import constVar;
import textureVar;
import drawText;
import globalVar;
import log;
import drawSprite;

// 한 줄을 4방향 그림자 + 본문으로 그린다. (x, y)는 그 줄의 기준 좌표.
static void drawLogLine(const std::wstring& text, int x, int y)
{
	std::wstring shadow = removeColorCodes(text);

	drawText(shadow, x - 1, y, col::black);
	drawText(shadow, x + 1, y, col::black);
	drawText(shadow, x, y - 2, col::black);
	drawText(shadow, x, y + 2, col::black);

	drawText(text, x, y);
}

export std::int64_t renderLog(SDL_Renderer* renderer)
{
	std::int64_t timeStampStart = getNanoTimer();

	const auto& magazine = getLogMagazine();

	setFont(fontType::mainFont);
	setFontSize(18);

	for (const auto& log : magazine)
	{
		if (log.dead) continue;

		int width = spr::logBackground->getW();
		int height = spr::logBackground->getH();
		SDL_SetTextureAlphaMod(spr::logBackground->getTexture(), log.alpha);
		drawSprite(spr::logBackground, log.x, log.y);

		int textX = log.x + 68;
		int centerY = log.y + 27; // 1줄 기준점. 줄 수가 늘면 이 점을 중심으로 위아래 분산
		constexpr int lineGap = 20; // 줄 간격
		constexpr int lineGap3 = lineGap - 1; // 3줄일 땐 바깥 줄을 1px씩 안으로 (오버플로우 완화)

		std::wstring shadowText = removeColorCodes(log.text);

		if (queryTextWidth(shadowText) < 310)
		{
			// 1줄
			drawLogLine(log.text, textX, centerY);
		}
		else
		{
			std::array<std::wstring, 3> arr = wordSplitter3(log.text, 310);

			if (arr[2].empty())
			{
				// 2줄: 중심 기준 ±(lineGap/2)
				drawLogLine(arr[0], textX, centerY - lineGap / 2);
				drawLogLine(arr[1], textX, centerY + lineGap / 2);
			}
			else
			{
				// 3줄: 중심 기준 -lineGap3 / 0 / +lineGap3
				drawLogLine(arr[0], textX, centerY - lineGap3);
				drawLogLine(arr[1], textX, centerY);
				drawLogLine(arr[2], textX, centerY + lineGap3);
			}
		}
	}

	// step 처리
	if (!stopLog)
	{
		stepLogs();
	}

	return (getNanoTimer() - timeStampStart);
}