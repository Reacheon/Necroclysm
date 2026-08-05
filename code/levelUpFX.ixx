module;
#include <SDL3/SDL.h>

export module levelUpFX;

import std;
import util;
import globalVar;
import Sprite;
import textureVar;
import drawSprite;
import Player;

static Uint64 fxStartTick = 0;

constexpr int FX_SPRITE_FRAMES = 9;
constexpr int FX_FRAME_DUR[FX_SPRITE_FRAMES] = { 80, 80, 80, 80, 80, 80, 80, 80, 80 };
constexpr int FX_TOTAL_MS = 900;
constexpr float FX_TAU = 6.2831853f;

export namespace levelUpFX
{
	void trigger() { fxStartTick = SDL_GetTicks(); if (PlayerPtr != nullptr) PlayerPtr->flash = { 255, 255, 255, 150 }; }

	void draw()
	{
		if (fxStartTick == 0 || PlayerPtr == nullptr) return;
		Uint64 t = SDL_GetTicks() - fxStartTick;
		if (t >= FX_TOTAL_MS) { fxStartTick = 0; return; }
		float tSec = t / 1000.0f;

		int frame = 0;
		for (Uint64 acc = 0; frame < FX_SPRITE_FRAMES && t >= acc + FX_FRAME_DUR[frame]; ++frame) acc += FX_FRAME_DUR[frame];

		int z = std::max(1, (int)zoomScale);

		int cx = (cameraW / 2) + z * ((PlayerPtr->getX() - cameraX) + PlayerPtr->getIntegerFakeX());
		int baseY = (cameraH / 2) + z * (PlayerPtr->getY() - cameraY + PlayerPtr->getIntegerFakeY() + 4);

		constexpr size_t FX_MAX_PX = 1024;
		static SDL_Color cols[FX_MAX_PX];
		static Point2 pts[FX_MAX_PX];
		size_t n = 0;
		auto putGround = [&](int gx, int gy, SDL_Color c, int a) { if (n >= FX_MAX_PX || a < 6) return; c.a = (Uint8)std::min(a, 255); cols[n] = c; pts[n] = { cx + gx * z, baseY + gy * z }; n++; };

		float bright = frame <= 3 ? 1.0f : std::max(0.0f, (FX_SPRITE_FRAMES - 1 - frame) / 5.0f);

		PlayerPtr->flash = { 255, 255, 255, (Uint8)(150 * bright) };

		if (frame < FX_SPRITE_FRAMES)
		{
			setZoom((float)z);
			drawSpriteCenter(spr::levelUpEffect, frame, cx, baseY - 4 * z);
			setZoom(1.0f);
		}

		auto ring = [&](int r, SDL_Color c, int a)
		{
			if (a < 6 || r < 1) return;
			int steps = std::max(24, r * 8);
			int prevX = 9999, prevY = 9999;
			for (int i = 0; i < steps; i++)
			{
				float ang = (float)i / steps * FX_TAU;
				int gx = (int)std::lround(std::cos(ang) * r);
				int gy = (int)std::lround(std::sin(ang) * r * 0.36f);
				if (gx == prevX && gy == prevY) continue;
				prevX = gx; prevY = gy;
				putGround(gx, gy, c, a);
			}
		};
		{
			constexpr int R_MAX = 15;
			constexpr float WAVE_DUR = 0.85f;
			float u = tSec / WAVE_DUR;
			if (u < 1.0f)
			{
				float eased = 1.0f - (1.0f - u) * (1.0f - u);
				int rHead = (int)(1.0f + eased * (R_MAX - 1));
				float spread = 1.0f - (float)rHead / (R_MAX + 1);
				ring(rHead, { 255, 255, 255 }, (int)(235 * spread * spread));
				if (rHead >= 2) ring(rHead - 1, { 255, 245, 215 }, (int)(120 * spread * spread));
			}
		}

		{
			int glowA = (int)(55 * bright) + (tSec < 0.25f ? (int)(95 * (1.0f - tSec / 0.25f)) : 0);
			constexpr int poolHalf[5] = { 4, 6, 7, 6, 4 };
			constexpr int poolLv[5] = { 5, 8, 10, 8, 5 };
			for (int dy = -2; dy <= 2; dy++)
			{
				for (int gx = -poolHalf[dy + 2]; gx < poolHalf[dy + 2]; gx++) putGround(gx, dy, { 255, 250, 225 }, glowA * poolLv[dy + 2] / 10);
			}
		}

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawRectBatch(1, 1, cols, pts, n, (float)z);
	}
}
