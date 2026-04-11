module;
#include <SDL3/SDL.h>

module Status;

import std;
import util;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import constVar;
import Player;

void Status::drawRadarChart(const std::function<SDL_Color(const SDL_Rect&)>& stadiumCol)
{
	//육각형 스테이터스 레이더 차트 (flat-top)
	{
		int cx = statusBase.x + 151;
		int cy = statusBase.y + 271;
		int r = 44;
		double hexAngle[6];
		for (int i = 0; i < 6; i++)
			hexAngle[i] = std::numbers::pi / 180.0 * (60.0 * i);

		// ratio 값 (0.0~1.0): 각 꼭짓점의 실제 데이터 비율
		// i=0: Bionic Capacity, i=1: Progress, i=2: Profic Avg
		// i=3: Status Avg, i=4: Locked, i=5: Mutation Threshold
		double proficSum = 0.0;
		for (int i = 0; i < TALENT_SIZE; i++)
			proficSum += PlayerPtr->getProficLevel(i);
		double statAvgVal = (PlayerPtr->entityInfo.statStr
			+ PlayerPtr->entityInfo.statInt
			+ PlayerPtr->entityInfo.statDex) / 3.0;

		double ratio[6] = {
			0.9,                                                       // Bionic Capacity (미구현)
			0.6,                                                       // Progress (미구현)
			std::clamp(proficSum / TALENT_SIZE / MAX_PROFIC_LEVEL, 0.0, 1.0), // Profic Avg
			std::clamp(statAvgVal / 10.0, 0.0, 1.0),                  // Status Avg
			0.0,                                                       // Locked
			0.9,                                                       // Mutation Threshold (미구현)
		};

		//삼각형 팬으로 채워진 육각형을 그리는 람다
		auto drawFilledHex = [&](float* vx, float* vy, SDL_Color c, Uint8 a)
			{
				SDL_Vertex vertices[8];
				int indices[18];

				//중심 정점
				vertices[0].position = { (float)cx, (float)cy };
				vertices[0].color = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, a / 255.0f };
				for (int i = 0; i < 6; i++)
				{
					vertices[i + 1].position = { vx[i], vy[i] };
					vertices[i + 1].color = vertices[0].color;
				}
				//삼각형 팬 인덱스
				for (int i = 0; i < 6; i++)
				{
					indices[i * 3] = 0;
					indices[i * 3 + 1] = i + 1;
					indices[i * 3 + 2] = (i + 1) % 6 + 1;
				}
				SDL_RenderGeometry(renderer, nullptr, vertices, 7, indices, 18);
			};

		//1) 검은 배경 육각형
		float bx[6], by[6];
		for (int i = 0; i < 6; i++)
		{
			bx[i] = cx + (float)(r * std::cos(hexAngle[i]));
			by[i] = cy + (float)(r * std::sin(hexAngle[i]));
		}
		drawFilledHex(bx, by, { 0, 0, 0 }, 255);

		//2) 스탯 레이더 (ratio 반영, 최소 비율 보장)
		// 0.0→minR, 1.0→1.0 으로 선형 보간하여 죽는 구간 없이 매핑
		constexpr double minR = 0.12;
		float sx[6], sy[6];
		for (int i = 0; i < 6; i++)
		{
			double mapped = minR + ratio[i] * (1.0 - minR);
			sx[i] = cx + (float)(r * mapped * std::cos(hexAngle[i]));
			sy[i] = cy + (float)(r * mapped * std::sin(hexAngle[i]));
		}
		drawFilledHex(sx, sy, { 0x44, 0xaa, 0xff }, 160);

		//3) 스탯 레이더 테두리
		for (int i = 0; i < 6; i++)
			drawLine((int)sx[i], (int)sy[i], (int)sx[(i + 1) % 6], (int)sy[(i + 1) % 6], SDL_Color{ 0x44, 0xaa, 0xff }, 220);

		//4) 흰색 외곽 테두리
		for (int i = 0; i < 6; i++)
			drawLine((int)bx[i], (int)by[i], (int)bx[(i + 1) % 6], (int)by[(i + 1) % 6], col::white);
	}

	//vertex 버튼 타이틀+값 그리기 람다
	auto drawVertexBtn = [](const SDL_Rect& btn, const std::wstring& title, const std::wstring& value, SDL_Color bgCol = col::black)
	{
		drawStadium(btn, bgCol, 150, 4);
		int cx = btn.x + btn.w / 2;

		setFontSize(12);
		setFont(fontType::mainFontMedium);
		if (queryTextWidth(removeColorCodes(title)) > btn.w)
		{
			auto split = wordSplitter(title, btn.w);
			// 컬러코드가 첫 줄에만 있으면 둘째 줄에도 적용
			std::wstring colorPrefix;
			if (split[0].size() >= 7 && split[0][0] == L'#')
				colorPrefix = split[0].substr(0, 7);
			if (!split[1].empty() && split[1][0] != L'#' && !colorPrefix.empty())
				split[1] = colorPrefix + split[1];
			drawTextCenter(split[0], cx, btn.y + 6);
			drawTextCenter(split[1], cx, btn.y + 6 + 12);
		}
		else
		{
			drawTextCenter(title, cx, btn.y + 12);
		}

		setFont(fontType::mainFontBold);
		setFontSize(18);
		drawTextCenter(value, cx, btn.y + 38);
		setFont(fontType::mainFont);
	};

	SDL_Rect vertex1Btn = { statusBase.x + 181,statusBase.y + 176,70,56 };
	drawVertexBtn(vertex1Btn, L"#e1772eMutation Threshold", L"72%", stadiumCol(vertex1Btn));

	SDL_Rect vertex2Btn = { statusBase.x + 208,statusBase.y + 244,70,56 };
	drawVertexBtn(vertex2Btn, L"#e1772eBionic Capacity", L"6 / 10", stadiumCol(vertex2Btn));

	SDL_Rect vertex3Btn = { statusBase.x + 181,statusBase.y + 312,70,56 };
	drawVertexBtn(vertex3Btn, L"#e1772eProgress", L"20%", stadiumCol(vertex3Btn));

	SDL_Rect vertex4Btn = { statusBase.x + 52,statusBase.y + 312,70,56 };
	{
		// Profic Avg: 전체 숙련도 레벨 평균 (최대 27)
		double proficSum = 0.0;
		for (int i = 0; i < TALENT_SIZE; i++)
			proficSum += PlayerPtr->getProficLevel(i);
		double proficAvg = proficSum / TALENT_SIZE;
		std::wstringstream wss;
		wss << std::fixed << std::setprecision(1) << proficAvg;
		drawVertexBtn(vertex4Btn, L"#e1772eProfic Avg", wss.str(), stadiumCol(vertex4Btn));
	}

	SDL_Rect vertex5Btn = { statusBase.x + 25,statusBase.y + 244,70,56 };
	{
		// Status Avg: STR/INT/DEX 평균 (최대 10)
		double statAvg = (PlayerPtr->entityInfo.statStr
			+ PlayerPtr->entityInfo.statInt
			+ PlayerPtr->entityInfo.statDex) / 3.0;
		std::wstringstream wss;
		wss << std::fixed << std::setprecision(1) << statAvg;
		drawVertexBtn(vertex5Btn, L"#e1772eStatus Avg", wss.str(), stadiumCol(vertex5Btn));
	}

	SDL_Rect vertex6Btn = { statusBase.x + 52,statusBase.y + 176,70,56 };
	drawStadium(vertex6Btn, col::black, 150, 4);
	setFontSize(14);
	setFont(fontType::mainFontMedium);
	drawTextCenter(col2Str(col::gray) + L"Locked", vertex6Btn.x + vertex6Btn.w / 2, vertex6Btn.y + vertex6Btn.h / 2);
	setFont(fontType::mainFont);
}
