#include <SDL.h>

export module renderFPS;

import std;
import util;
import globalVar;
import constVar;
import drawText;

export void renderFPS(__int64 loopTime)
//프레임 표시 기능
{
	const int samplingTime = 100;
	static int lastUpdateTime = 0;
	int currentTime = SDL_GetTicks();
	static std::vector<float> fpsArr;
	static float loopFPS, avgFPS;

	static std::vector<float> turnCycleArr, stepEventArr, renderTileArr, renderStickerArr, renderUIArr, renderLogArr;
	static __int64 turnCycleAvg, stepEventAvg, renderTileAvg, renderStickerAvg, renderUIAvg, renderLogAvg;

	static std::vector<float> analysisArr,tileArr, corpseArr, itemArr, entityArr, damageArr, fogArr, markerArr;
	static __int64 analysisAvg,tileAvg, corpseAvg, itemAvg, entityAvg, damageAvg, fogAvg, markerAvg;

	if (currentTime - lastUpdateTime > samplingTime)
	{
		loopFPS = 1000000000.0 / (float)loopTime;

		fpsArr.push_back(loopFPS);
		if (fpsArr.size() > 10) fpsArr.erase(fpsArr.begin());
		avgFPS = std::accumulate(fpsArr.begin(), fpsArr.end(), 0.0) / fpsArr.size();

		auto getAvg = [](std::vector<float>& inputArr,__int64 inputVal)->__int64
			{
				inputArr.push_back(inputVal);
				if (inputArr.size() > 10) inputArr.erase(inputArr.begin());
				return std::accumulate(inputArr.begin(), inputArr.end(), 0) / inputArr.size();
			};
		turnCycleAvg = getAvg(turnCycleArr, dur::turnCycle);
		stepEventAvg = getAvg(stepEventArr, dur::stepEvent);
		renderTileAvg = getAvg(renderTileArr, dur::renderTile);
		renderStickerAvg = getAvg(renderStickerArr, dur::renderSticker);
		renderUIAvg = getAvg(renderUIArr, dur::renderUI);
		renderLogAvg = getAvg(renderLogArr, dur::renderLog);

		analysisAvg = getAvg(analysisArr, dur::analysis);
		tileAvg = getAvg(tileArr, dur::tile);
		corpseAvg = getAvg(corpseArr, dur::corpse);
		itemAvg = getAvg(itemArr, dur::item);
		entityAvg = getAvg(entityArr, dur::entity);
		damageAvg = getAvg(damageArr, dur::damage);
		fogAvg = getAvg(fogArr, dur::fog);
		markerAvg = getAvg(markerArr, dur::marker);

		lastUpdateTime = currentTime;
	}

	//프레임 표시
	auto drawShadowText = [](std::wstring input, int x, int y)
		{
			drawText(L"#000000" + input, x + 1, y + 1);
			drawText(L"#FFFFFF" + input, x, y);
		};
	auto drawShadowTextRed = [](std::wstring input, int x, int y)
		{
			drawText(L"#000000" + input, x + 1, y + 1);
			drawText(L"#FF0000" + input, x, y);
		};
	
	
	setFontSize(12);
	
	drawTextShadow(col2Str(col::white) + decimalCutter(avgFPS, 2), 20, 160);
	


	setFontSize(9);

	drawTextShadow(col2Str(col::white)+L"turnCycle : " + decimalCutter(turnCycleAvg / 1000000.0, 5) + L" ms", 20, 180);
	drawTextShadow(col2Str(col::white) + L"stepEvent : " + decimalCutter(stepEventAvg / 1000000.0, 5) + L" ms", 20, 180 + 12 * 1);
	drawTextShadow(col2Str(col::white) + L"renderSticker : " + decimalCutter(renderStickerAvg / 1000000.0, 5) + L" ms", 20, 180 + 12 * 2);
	drawTextShadow(col2Str(col::white) + L"renderUI : " + decimalCutter(renderUIAvg / 1000000.0, 5) + L" ms", 20, 180 + 12 * 3);
	drawTextShadow(col2Str(col::white) + L"renderLog : " + decimalCutter(renderLogAvg / 1000000.0, 5) + L" ms", 20, 180 + 12 * 4);
	drawTextShadow(col2Str(col::white) + L"renderTile : " + decimalCutter(renderTileAvg / 1000000.0, 5) + L" ms", 20, 180 + 12 * 5);

	drawTextShadow(col2Str(col::white) + L">analysis : " + decimalCutter(analysisAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 6);
	drawTextShadow(col2Str(col::white) + L">tile : " + decimalCutter(tileAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 7);
	drawTextShadow(col2Str(col::white) + L">corpse : " + decimalCutter(corpseAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 8);
	drawTextShadow(col2Str(col::white) + L">itemTile : " + decimalCutter(itemAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 9);
	drawTextShadow(col2Str(col::white) + L">entityTile : " + decimalCutter(entityAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 10);
	drawTextShadow(col2Str(col::white) + L">damageTile : " + decimalCutter(damageAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 11);
	drawTextShadow(col2Str(col::white) + L">fogTile : " + decimalCutter(fogAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 12);
	drawTextShadow(col2Str(col::white) + L">markerTile : " + decimalCutter(markerAvg / 1000000.0, 5) + L" ms", 30, 180 + 12 * 13);
}