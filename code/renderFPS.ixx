#include <SDL.h>

export module renderFPS;

import std;
import util;
import globalVar;
import constVar;
import drawText;

enum class FPSMode
{
    avg,
    hiscore
};

export void renderFPS(__int64 loopTime)
{

    __int64 startTime = getNanoTimer();

    FPSMode mode = FPSMode::avg;


    //drawFillRect(0, 0, 720, 720, col::black);

    const int samplingTime = 100;
    const int samplingSize = 10;
    static int lastUpdateTime = 0;
    int currentTime = SDL_GetTicks();

    static std::deque<float> fpsArr;
    static float loopFPS, avgFPS;

    static std::deque<float> turnCycleArr, stepEventArr, renderTileArr, renderWeatherArr, renderStickerArr, renderUIArr, renderLogArr, loopTimeArr;
    static __int64 turnCycleAvg, stepEventAvg, renderTileAvg, renderWeatherAvg, renderStickerAvg, renderUIAvg, renderLogAvg, loopTimeAvg;

    static std::deque<float> analysisArr, tileArr, corpseArr, itemArr, entityArr, damageArr, fogArr, markerArr;
    static __int64 analysisAvg, tileAvg, corpseAvg, itemAvg, entityAvg, damageAvg, fogAvg, markerAvg;

    auto updateAvg = [](std::deque<float>& arr, float newVal) -> float {
        if (arr.size() >= samplingSize) arr.pop_front();
        arr.push_back(newVal);
        return std::accumulate(arr.begin(), arr.end(), 0.0f) / arr.size();
        };

    if (currentTime - lastUpdateTime > samplingTime)
    {
        loopFPS = 1000000000.0f / loopTime;
        avgFPS = updateAvg(fpsArr, loopFPS);

        turnCycleAvg = updateAvg(turnCycleArr, dur::turnCycle);
        stepEventAvg = updateAvg(stepEventArr, dur::stepEvent);
        renderTileAvg = updateAvg(renderTileArr, dur::renderTile);
        renderWeatherAvg = updateAvg(renderWeatherArr, dur::renderWeather);
        renderStickerAvg = updateAvg(renderStickerArr, dur::renderSticker);
        renderUIAvg = updateAvg(renderUIArr, dur::renderUI);
        renderLogAvg = updateAvg(renderLogArr, dur::renderLog);
        loopTimeAvg = updateAvg(loopTimeArr, dur::totalDelay);

        analysisAvg = updateAvg(analysisArr, dur::analysis);
        tileAvg = updateAvg(tileArr, dur::tile);
        corpseAvg = updateAvg(corpseArr, dur::corpse);
        itemAvg = updateAvg(itemArr, dur::item);
        entityAvg = updateAvg(entityArr, dur::entity);
        damageAvg = updateAvg(damageArr, dur::damage);
        fogAvg = updateAvg(fogArr, dur::fog);
        markerAvg = updateAvg(markerArr, dur::marker);

        lastUpdateTime = currentTime;
    }


    setFontSize(12);
    drawText(col2Str(col::white) + decimalCutter(avgFPS, 2), 20, 170);

    setFontSize(9);
    drawText(col2Str(col::white)+L"turnCycle : " + decimalCutter(turnCycleAvg / 1000000.0, 5) + L" ms", 20, 190);
    drawText(col2Str(col::white) + L"stepEvent : " + decimalCutter(stepEventAvg / 1000000.0, 5) + L" ms", 20, 190 + 12 * 1);
    drawText(col2Str(col::white) + L"renderUI : " + decimalCutter(renderUIAvg / 1000000.0, 5) + L" ms", 20, 190 + 12 * 2);
    drawText(col2Str(col::white) + L"renderTile : " + decimalCutter(renderTileAvg / 1000000.0, 5) + L" ms", 20, 190 + 12 * 3);
    drawText(col2Str(col::white) + L">analysis : " + decimalCutter(analysisAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 4);
    drawText(col2Str(col::white) + L">tile : " + decimalCutter(tileAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 5);
    drawText(col2Str(col::white) + L">corpse : " + decimalCutter(corpseAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 6);
    drawText(col2Str(col::white) + L">itemTile : " + decimalCutter(itemAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 7);
    drawText(col2Str(col::white) + L">entityTile : " + decimalCutter(entityAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 8);
    drawText(col2Str(col::white) + L">damageTile : " + decimalCutter(damageAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 9);
    drawText(col2Str(col::white) + L">fogTile : " + decimalCutter(fogAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 10);
    drawText(col2Str(col::white) + L">markerTile : " + decimalCutter(markerAvg / 1000000.0, 5) + L" ms", 30, 190 + 12 * 11);
    drawText(col2Str(col::white) + L"totalTime : " + decimalCutter(loopTimeAvg / 1000000.0, 5) + L" ms", 20, 190 + 12 * 12);
}