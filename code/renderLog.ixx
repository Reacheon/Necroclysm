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


export __int64 renderLog(SDL_Renderer* renderer)
{
	__int64 timeStampStart = getNanoTimer();

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

		std::wstring shadowText = removeColorCodes(log.text);

		if (queryTextWidth(shadowText) < 310)
		{
			int textX = log.x + 68;
			int textY = log.y + 27;

			// 그림자 (4방향)
			drawText(shadowText, textX - 1, textY, col::black);
			drawText(shadowText, textX + 1, textY, col::black);
			drawText(shadowText, textX, textY - 2, col::black);
			drawText(shadowText, textX, textY + 2, col::black);

			// 본문
			drawText(log.text, textX, textY);
		}
		else
		{
			std::array<std::wstring, 2> arr = wordSplitter(log.text, 310);
			std::wstring shadow1 = removeColorCodes(arr[0]);
			std::wstring shadow2 = removeColorCodes(arr[1]);

			int textX = log.x + 68;
			int textY1 = log.y + 27 - 10;
			int textY2 = log.y + 27 + 10;

			// 1줄째 그림자
			drawText(shadow1, textX - 1, textY1, col::black);
			drawText(shadow1, textX + 1, textY1, col::black);
			drawText(shadow1, textX, textY1 - 2, col::black);
			drawText(shadow1, textX, textY1 + 2, col::black);

			// 1줄째 본문
			drawText(arr[0], textX, textY1);

			// 2줄째 그림자
			drawText(shadow2, textX - 1, textY2, col::black);
			drawText(shadow2, textX + 1, textY2, col::black);
			drawText(shadow2, textX, textY2 - 2, col::black);
			drawText(shadow2, textX, textY2 + 2, col::black);

			// 2줄째 본문
			drawText(arr[1], textX, textY2);
		}
	}

	// step 처리
	if (!stopLog)
	{
		stepLogs();
	}

	return (getNanoTimer() - timeStampStart);
}