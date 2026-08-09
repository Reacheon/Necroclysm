module;
#include <SDL3/SDL.h>

export module drawPrimitive;

import std;
import Point;
import errorBox;

static SDL_Renderer* localRenderer;
export void setPrimitiveRenderer(SDL_Renderer* inputRenderer) { localRenderer = inputRenderer; }


export void drawPoint(int x1, int y1, const SDL_Color& col, Uint8 alpha)
{
	SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
	SDL_RenderPoint(localRenderer, x1, y1);
	SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
};
export void drawPoint(int x1, int y1, const SDL_Color& col) { drawPoint(x1, y1, col, 255);}

/////////////////////////////////////////////////////////////////////////////////////////////
export void drawLine(int x1, int y1, int x2, int y2, SDL_Color col, Uint8 alpha)
{
    SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
    SDL_RenderLine(localRenderer, x1, y1, x2, y2);
	SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
};
export void drawLine(int x1, int y1, int x2, int y2, SDL_Color col){ drawLine(x1, y1, x2, y2, col, 255); };
export void drawLine(int x1, int y1, int x2, int y2) { drawLine(x1, y1, x2, y2, { 255,255,255 }, 255); }

//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

export void drawFillRect(const SDL_FRect& rect, const SDL_Color& col, Uint8 alpha)
{
	SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
	SDL_RenderFillRect(localRenderer, &rect);
	SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
}
export void drawFillRect(const SDL_FRect& rect, const SDL_Color& col)
{
	drawFillRect(rect, col, 255);
}

export void drawFillRect(int x, int y, int w, int h, const SDL_Color& col, Uint8 alpha)
{
	SDL_FRect targetRect = { static_cast<float>(x),
							 static_cast<float>(y),
							 static_cast<float>(w),
							 static_cast<float>(h) };
	drawFillRect(targetRect, col, alpha);
}
export void drawFillRect(const SDL_Rect& rect, const SDL_Color& col, Uint8 alpha)
{
	drawFillRect(rect.x, rect.y, rect.w, rect.h, col, alpha);
}
export void drawFillRect(int x, int y, int w, int h, const SDL_Color& col)
{
	drawFillRect(x, y, w, h, col, 255);
}
export void drawFillRect(const SDL_Rect& rect, const SDL_Color& col)
{
	drawFillRect(rect, col, 255);
}

/////////////////////////////////////////////////////////////////////////////////////////////

export void drawRect(int x, int y, int w, int h, const SDL_Color& col, Uint8 alpha)
{
    SDL_FRect targetRect = { x,y,w,h };
    SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);
    SDL_RenderRect(localRenderer, &targetRect);
};
export void drawRect(const SDL_Rect& rect, const SDL_Color& col, Uint8 alpha) { drawRect(rect.x, rect.y, rect.w, rect.h, col, alpha); };
export void drawRect(int x, int y, int w, int h, const SDL_Color& col) { drawRect(x, y, w, h, col, 255); };
export void drawRect(const SDL_Rect& rect, const SDL_Color& col) { drawRect(rect, col, 255); };

//////////////////////////////////////////////////////////////////////////////////////////////

//두께 1 십자가
export void drawCross(int x, int y, int north, int south, int west, int east)
{
    SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderLine(localRenderer, x, y, x, y - north);
    SDL_RenderLine(localRenderer, x, y, x, y + south);
    SDL_RenderLine(localRenderer, x, y, x - west, y);
    SDL_RenderLine(localRenderer, x, y, x + east, y);
}

//두께 2 십자가
export void drawCross2(int x, int y, int north, int south, int west, int east)
{
    SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderLine(localRenderer, x, y, x, y - north);
    SDL_RenderLine(localRenderer, x+1, y, x+1, y - north);

    SDL_RenderLine(localRenderer, x, y, x, y + south + 1);
    SDL_RenderLine(localRenderer, x+1, y, x+1, y + south + 1);

    SDL_RenderLine(localRenderer, x, y, x - west, y);
    SDL_RenderLine(localRenderer, x, y+1, x - west, y+1);

    SDL_RenderLine(localRenderer, x, y, x + east + 1, y);
    SDL_RenderLine(localRenderer, x, y+1, x + east + 1, y+1);
}


//두께 2 십자가
export void drawCross2(int x, int y, int north, int south, int west, int east, SDL_Color col)
{
	SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, 0xff);
	SDL_RenderLine(localRenderer, x, y, x, y - north);
	SDL_RenderLine(localRenderer, x + 1, y, x + 1, y - north);

	SDL_RenderLine(localRenderer, x, y, x, y + south + 1);
	SDL_RenderLine(localRenderer, x + 1, y, x + 1, y + south + 1);

	SDL_RenderLine(localRenderer, x, y, x - west, y);
	SDL_RenderLine(localRenderer, x, y + 1, x - west, y + 1);

	SDL_RenderLine(localRenderer, x, y, x + east + 1, y);
	SDL_RenderLine(localRenderer, x, y + 1, x + east + 1, y + 1);
	SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 255);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//5는 일반적인 둥근 테두리, 6은 하단이 평평한 5번, 7은 상단이 평평한 5번
export void drawStadium(int x, int y, int w, int h, SDL_Color color, int alpha, int edge)
{
	int xEnd = x + w - 1;
	int yEnd = y + h - 1;
	switch (edge)
	{
	default:
	{
		break;
	}
	case 1:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y + 1;
		rect.w = w;
		rect.h = h - 2;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 1, y, xEnd - 1, y);

		SDL_RenderLine(localRenderer, x + 1, yEnd, xEnd - 1, yEnd);
		break;
	}
	case 2:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y + 2;
		rect.w = w;
		rect.h = h - 4;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 2, y, xEnd - 2, y);
		SDL_RenderLine(localRenderer, x + 1, y + 1, xEnd - 1, y + 1);

		SDL_RenderLine(localRenderer, x + 1, yEnd - 1, xEnd - 1, yEnd - 1);
		SDL_RenderLine(localRenderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
		break;
	}
	case 3:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y + 3;
		rect.w = w;
		rect.h = h - 6;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 3, y, xEnd - 3, y);
		SDL_RenderLine(localRenderer, x + 2, y + 1, xEnd - 2, y + 1);
		SDL_RenderLine(localRenderer, x + 1, y + 2, xEnd - 1, y + 2);

		SDL_RenderLine(localRenderer, x + 1, yEnd - 2, xEnd - 1, yEnd - 2);
		SDL_RenderLine(localRenderer, x + 2, yEnd - 1, xEnd - 2, yEnd - 1);
		SDL_RenderLine(localRenderer, x + 3, yEnd, xEnd - 3, yEnd);
		break;
	}
	case 4:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y + 4;
		rect.w = w;
		rect.h = h - 8;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 4, y, xEnd - 4, y);
		SDL_RenderLine(localRenderer, x + 2, y + 1, xEnd - 2, y + 1);
		SDL_RenderLine(localRenderer, x + 1, y + 2, xEnd - 1, y + 2);
		SDL_RenderLine(localRenderer, x + 1, y + 3, xEnd - 1, y + 3);

		SDL_RenderLine(localRenderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
		SDL_RenderLine(localRenderer, x + 1, yEnd - 2, xEnd - 1, yEnd - 2);
		SDL_RenderLine(localRenderer, x + 2, yEnd - 1, xEnd - 2, yEnd - 1);
		SDL_RenderLine(localRenderer, x + 4, yEnd, xEnd - 4, yEnd);
		break;
	}
	case 5:
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y + 5;
		rect.w = w;
		rect.h = h - 10;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 5, y, xEnd - 5, y);
		SDL_RenderLine(localRenderer, x + 3, y + 1, xEnd - 3, y + 1);
		SDL_RenderLine(localRenderer, x + 2, y + 2, xEnd - 2, y + 2);
		SDL_RenderLine(localRenderer, x + 1, y + 3, xEnd - 1, y + 3);
		SDL_RenderLine(localRenderer, x + 1, y + 4, xEnd - 1, y + 4);

		SDL_RenderLine(localRenderer, x + 1, yEnd - 4, xEnd - 1, yEnd - 4);
		SDL_RenderLine(localRenderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
		SDL_RenderLine(localRenderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
		SDL_RenderLine(localRenderer, x + 3, yEnd - 1, xEnd - 3, yEnd - 1);
		SDL_RenderLine(localRenderer, x + 5, yEnd, xEnd - 5, yEnd);

		break;
	}
	case 6://하단 직선
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y + 5;
		rect.w = w;
		rect.h = h - 5;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 5, y, xEnd - 5, y);
		SDL_RenderLine(localRenderer, x + 3, y + 1, xEnd - 3, y + 1);
		SDL_RenderLine(localRenderer, x + 2, y + 2, xEnd - 2, y + 2);
		SDL_RenderLine(localRenderer, x + 1, y + 3, xEnd - 1, y + 3);
		SDL_RenderLine(localRenderer, x + 1, y + 4, xEnd - 1, y + 4);
		break;
	}
	case 7://상단 직선
	{
		SDL_SetRenderDrawColor(localRenderer, color.r, color.g, color.b, alpha);
		SDL_FRect rect;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h - 5;
		SDL_RenderFillRect(localRenderer, &rect);

		SDL_RenderLine(localRenderer, x + 1, yEnd - 4, xEnd - 1, yEnd - 4);
		SDL_RenderLine(localRenderer, x + 1, yEnd - 3, xEnd - 1, yEnd - 3);
		SDL_RenderLine(localRenderer, x + 2, yEnd - 2, xEnd - 2, yEnd - 2);
		SDL_RenderLine(localRenderer, x + 3, yEnd - 1, xEnd - 3, yEnd - 1);
		SDL_RenderLine(localRenderer, x + 5, yEnd, xEnd - 5, yEnd);

		break;
	}
	}
}

export void drawStadium(SDL_Rect rect, SDL_Color color, int alpha, int edge){ drawStadium(rect.x, rect.y, rect.w, rect.h, color, alpha, edge); }

/////////////////////////////////////////////////////////////////////////////////////////////
// 채워진 타원 (미드포인트 타원 알고리즘)
// cx, cy = 중심 좌표, rx = 가로 반지름, ry = 세로 반지름
export void drawFillEllipse(int cx, int cy, int rx, int ry, const SDL_Color& col, Uint8 alpha)
{
	if (rx <= 0 || ry <= 0) return;

	SDL_SetRenderDrawBlendMode(localRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(localRenderer, col.r, col.g, col.b, alpha);

	// 스캔라인 방식: y좌표별로 정확히 한 번만 그림 (alpha 누적 방지)
	float rx2 = (float)rx * rx;
	float ry2 = (float)ry * ry;
	for (int dy = -ry; dy <= ry; dy++)
	{
		// 타원 방정식: x^2/rx^2 + y^2/ry^2 = 1 → x = rx * sqrt(1 - y^2/ry^2)
		float xSpan = rx * std::sqrt(1.0f - (float)(dy * dy) / ry2);
		int x1 = cx - (int)xSpan;
		int x2 = cx + (int)xSpan;
		SDL_RenderLine(localRenderer, x1, cy + dy, x2, cy + dy);
	}

	SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
}
export void drawFillEllipse(int cx, int cy, int rx, int ry, const SDL_Color& col) { drawFillEllipse(cx, cy, rx, ry, col, 255); }

// 채워진 원 (반지름 r)
export void drawFillCircle(int cx, int cy, int r, const SDL_Color& col, Uint8 alpha) { drawFillEllipse(cx, cy, r, r, col, alpha); }
export void drawFillCircle(int cx, int cy, int r, const SDL_Color& col) { drawFillEllipse(cx, cy, r, r, col, 255); }

/////////////////////////////////////////////////////////////////////////////////////////////

export void drawRectBatch(int rectW, int rectH, SDL_Color* cols, const Point2* pts, size_t count, float inputZoomScale)
{
	if (!cols || !pts || !cols || count == 0) return;

	constexpr int MAX_RECT = 4096;

	static SDL_Vertex vertices[MAX_RECT * 4];
	static int indices[MAX_RECT * 6];

	if (count > MAX_RECT) errorBox(L"drawRectBatch: 렌더링 가능한 사각형 한도를 초과하였다.");

	const float rectWf = static_cast<float>(rectW) * inputZoomScale;
	const float rectHf = static_cast<float>(rectH) * inputZoomScale;

	for (size_t i = 0; i < count; ++i)
	{
		const SDL_Color& color = cols[i];
		const float normalizedR = color.r / 255.0f;
		const float normalizedG = color.g / 255.0f;
		const float normalizedB = color.b / 255.0f;
		const float normalizedA = color.a / 255.0f;

		const float x = pts[i].x;
		const float y = pts[i].y;

		const size_t vIdx = i * 4;
		vertices[vIdx] = { { x,         y },         { normalizedR, normalizedG, normalizedB, normalizedA }, { 0.0f, 0.0f } };
		vertices[vIdx + 1] = { { x + rectWf, y },         { normalizedR, normalizedG, normalizedB, normalizedA }, { 1.0f, 0.0f } };
		vertices[vIdx + 2] = { { x + rectWf, y + rectHf }, { normalizedR, normalizedG, normalizedB, normalizedA }, { 1.0f, 1.0f } };
		vertices[vIdx + 3] = { { x,         y + rectHf }, { normalizedR, normalizedG, normalizedB, normalizedA }, { 0.0f, 1.0f } };

		const int baseIdx = static_cast<int>(i * 4);
		const size_t iIdx = i * 6;
		indices[iIdx] = baseIdx;
		indices[iIdx + 1] = baseIdx + 1;
		indices[iIdx + 2] = baseIdx + 2;
		indices[iIdx + 3] = baseIdx;
		indices[iIdx + 4] = baseIdx + 2;
		indices[iIdx + 5] = baseIdx + 3;
	}

	SDL_RenderGeometry(localRenderer, nullptr, vertices, count * 4, indices, count * 6);
}

// 토글 활성 상태 스네이크 이펙트 (아이콘 가장자리에 흰색 빛이 돌아다니는 효과)
// x, y, w, h: 아이콘의 스크린 좌표 및 크기 (줌 적용 후 픽셀 단위)
export void drawToggleSnakeEffect(int x, int y, int w, int h)
{
	// 둘레 길이 (각 코너 픽셀은 한 번만 포함)
	int perim = 2 * w + 2 * h - 4;
	if (perim <= 0) return;

	Uint64 ticks = SDL_GetTicks();
	// 초당 80px 이동 → 48x48(perim=188) 기준 약 2.35초/바퀴
	float headPos = std::fmod(ticks * 80.0f / 1000.0f, (float)perim);
	float snakeLen = perim * 0.30f;

	SDL_SetRenderDrawBlendMode(localRenderer, SDL_BLENDMODE_BLEND);

	// 둘레 위치(t) → 스크린 좌표 변환 (시계방향: 상→우→하→좌)
	auto toXY = [&](float t) -> std::pair<int, int> {
		int it = ((int)t % perim + perim) % perim;
		if (it < w) return { x + it, y };
		it -= w;
		if (it < h - 1) return { x + w - 1, y + 1 + it };
		it -= (h - 1);
		if (it < w - 1) return { x + w - 2 - it, y + h - 1 };
		it -= (w - 1);
		return { x, y + h - 2 - it };
	};

	// 안쪽 1px 오프셋 방향 (두께 2px용)
	auto inward = [&](float t) -> std::pair<int, int> {
		int it = ((int)t % perim + perim) % perim;
		if (it < w) return { 0, 1 };
		it -= w;
		if (it < h - 1) return { -1, 0 };
		it -= (h - 1);
		if (it < w - 1) return { 0, -1 };
		return { 1, 0 };
	};

	// 스네이크 2마리 (180도 간격으로 배치)
	int steps = (int)snakeLen;
	for (int s = 0; s < 2; s++)
	{
		float base = headPos + s * (perim * 0.5f);
		for (int i = 0; i <= steps; i++)
		{
			float t = base - i;
			auto [px, py] = toXY(t);
			auto [dx, dy] = inward(t);

			// 머리 쪽이 밝고 꼬리 쪽으로 페이드아웃
			float ratio = 1.0f - (float)i / snakeLen;
			Uint8 alpha = (Uint8)(220 * ratio * ratio);
			if (alpha < 8) continue;

			SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, alpha);
			SDL_RenderPoint(localRenderer, (float)px, (float)py);
			SDL_RenderPoint(localRenderer, (float)(px + dx), (float)(py + dy));
		}
	}

	SDL_SetRenderDrawColor(localRenderer, 0xff, 0xff, 0xff, 0xff);
}

/////////////////////////////////////////////////////////////////////////////////////////////

export void draw3pxGauge(int x, int y, float zoomScale, float ratio, float alpha, SDL_Color inputGaugeCol = { 0,0,0 }, float fakeRatio = 0, float alphaFake = 0)
{
	SDL_Rect dst = { x, y, (int)(16 * zoomScale),(int)(3 * zoomScale) };

	SDL_SetRenderDrawBlendMode(localRenderer, SDL_BLENDMODE_BLEND);
	drawFillRect(dst, { 0,0,0 }, alpha);

	if (alphaFake != 0)
	{
		SDL_Rect fakeGauge = { x + (int)(1.0 * zoomScale), y + (int)(1.0 * zoomScale), (int)(14 * zoomScale * fakeRatio),(int)(1 * zoomScale) };
		if (fakeRatio > 0 && fakeGauge.w == 0) { fakeGauge.w = 1; }
		SDL_SetRenderDrawBlendMode(localRenderer, SDL_BLENDMODE_BLEND);
		drawFillRect(fakeGauge, { 0xff,0xff,0xff }, alphaFake);
	}

	SDL_Color gaugeCol = inputGaugeCol;
	if(gaugeCol.r == 0 && gaugeCol.g == 0 && gaugeCol.b == 0)
	{
		constexpr SDL_Color red = { 0xd0,0x3f,0x3f };
		constexpr SDL_Color yellow = { 0xd0,0xc3,0x3f };
		constexpr SDL_Color green = { 0x75,0xd0,0x3f };
		if (ratio < 0.2f) gaugeCol = red; //red
		else if (ratio < 0.5f) gaugeCol = yellow; //yellow
		else gaugeCol = green; //green
    }

	if (ratio > 0)
	{
		SDL_Rect valGauge = { x + (int)(1.0 * zoomScale), y + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratio),(int)(1 * zoomScale) };
		if (valGauge.w == 0) { valGauge.w = 1; }
		SDL_SetRenderDrawBlendMode(localRenderer, SDL_BLENDMODE_BLEND);
		drawFillRect(valGauge, gaugeCol, alpha);
	}
}