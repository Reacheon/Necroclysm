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

// 레벨업 연출 — 섬광 기둥 스프라이트(spr::levelUpEffect) + 발밑 충격파 + 접지 빛 웅덩이.
//
// 기둥은 도트 스프라이트 애니(80px 9프레임, 균등 80ms — FX_FRAME_DUR 표로 프레임별 조정 가능):
// 4프레임에서 최대가 된 뒤 잦아듦.
// 중심(40,40)을 캐릭터 앵커(origin)에 정렬. 캐릭터 플래시·빛 웅덩이는 같은 엔벨로프로 감쇠.
//
// 좌표 규약: 모든 픽셀은 게임픽셀 단위(1 이펙트 픽셀 = zoomScale 스크린픽셀)로
// 스프라이트 픽셀 그리드에 정렬됨.
// 지면 요소(충격파·웅덩이)는 플레이어 그림자 타원(장축 14px, 단축 5px)의 납작비율 0.36을 따름.
//
// 상태는 발동 시각 하나뿐 — 매 프레임이 경과시간의 순수함수라 step/리스트 관리가 없음.

static Uint64 fxStartTick = 0; // 0 = 비활성

constexpr int FX_SPRITE_FRAMES = 9;   // levelUpEffect.png 프레임 수 (절정 = 4번째)
constexpr int FX_FRAME_DUR[FX_SPRITE_FRAMES] = { 80, 80, 80, 80, 80, 80, 80, 80, 80 }; // 프레임별 길이(ms) — 표라서 개별 조정 가능
constexpr int FX_TOTAL_MS = 900;      // 총 수명 — 충격파(0.85초)가 끝까지 퍼지도록 스프라이트(720ms)보다 길게
constexpr float FX_TAU = 6.2831853f;

export namespace levelUpFX
{
	// 캐릭터 플래시는 여기서 즉시 세팅 — draw()는 엔티티 렌더 이후에 돌아서 draw()에만 맡기면 오버레이가 1프레임 늦음
	void trigger() { fxStartTick = SDL_GetTicks(); if (PlayerPtr != nullptr) PlayerPtr->flash = { 255, 255, 255, 150 }; }

	// renderTile()의 포그 이후에 호출 — 밤/어둠 위에서도 빛기둥이 살아남음
	void draw()
	{
		if (fxStartTick == 0 || PlayerPtr == nullptr) return;
		Uint64 t = SDL_GetTicks() - fxStartTick;
		if (t >= FX_TOTAL_MS) { fxStartTick = 0; return; }
		float tSec = t / 1000.0f;

		// 프레임 룩업 — 경과시간이 속한 프레임 인덱스. 표 끝을 지나면 FX_SPRITE_FRAMES(기둥 종료, 충격파 여운 구간).
		int frame = 0;
		for (Uint64 acc = 0; frame < FX_SPRITE_FRAMES && t >= acc + FX_FRAME_DUR[frame]; ++frame) acc += FX_FRAME_DUR[frame];

		int z = std::max(1, (int)zoomScale); // 줌 1~5 정수 스냅 전제 — 게임픽셀 1개 = z 스크린픽셀

		// 플레이어 앵커 (Entity::drawSelf와 동일 공식 — X축 시암 wrap 준수)
		int cx = (cameraW / 2) + z * (worldWrap::signedDeltaRenderX(cameraX, PlayerPtr->getX()) + PlayerPtr->getIntegerFakeX());
		int baseY = (cameraH / 2) + z * (PlayerPtr->getY() - cameraY + PlayerPtr->getIntegerFakeY() + 4); // 발밑 지면선

		// 배치 버퍼: 이번 프레임의 지면 이펙트 픽셀(충격파·웅덩이)을 모아 drawRectBatch 1회로 그림
		constexpr size_t FX_MAX_PX = 1024;
		static SDL_Color cols[FX_MAX_PX];
		static Point2 pts[FX_MAX_PX];
		size_t n = 0;
		// putGround: 지면 평면 좌표 (gy 양수 = 화면 아래쪽)
		auto putGround = [&](int gx, int gy, SDL_Color c, int a) { if (n >= FX_MAX_PX || a < 6) return; c.a = (Uint8)std::min(a, 255); cols[n] = c; pts[n] = { cx + gx * z, baseY + gy * z }; n++; };

		// ---- 섬광 엔벨로프: 스프라이트의 절정(4번째 프레임)까지 유지 후 마지막 프레임까지 선형 감쇠 ----
		float bright = frame <= 3 ? 1.0f : std::max(0.0f, (FX_SPRITE_FRAMES - 1 - frame) / 5.0f);

		// 캐릭터 백색 섬광: 기둥과 같은 엔벨로프로 감쇠 (첫 프레임 점등은 trigger()가 담당)
		PlayerPtr->flash = { 255, 255, 255, (Uint8)(150 * bright) };

		// ---- 섬광 기둥: 도트 스프라이트 애니 — 중심(40,40)을 캐릭터 앵커(origin = baseY-4)에 정렬 ----
		if (frame < FX_SPRITE_FRAMES)
		{
			setZoom((float)z);
			drawSpriteCenter(spr::levelUpEffect, frame, cx, baseY - 4 * z);
			setZoom(1.0f);
		}

		// ---- 단발 충격파: 납작한 타원 링 하나가 감속하며 퍼짐 (전연 2픽셀 두께뿐, 뒤따르는 파문 없음) ----
		auto ring = [&](int r, SDL_Color c, int a)
		{
			if (a < 6 || r < 1) return;
			int steps = std::max(24, r * 8);
			int prevX = 9999, prevY = 9999;
			for (int i = 0; i < steps; i++)
			{
				float ang = (float)i / steps * FX_TAU;
				int gx = (int)std::lround(std::cos(ang) * r);
				int gy = (int)std::lround(std::sin(ang) * r * 0.36f); // 그림자 타원과 같은 납작비율
				if (gx == prevX && gy == prevY) continue; // 인접 중복 픽셀 제거 (이중 블렌딩 방지)
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
				float eased = 1.0f - (1.0f - u) * (1.0f - u); // 감속 곡선 — 실제 파문처럼 멀어질수록 느려짐
				int rHead = (int)(1.0f + eased * (R_MAX - 1));
				float spread = 1.0f - (float)rHead / (R_MAX + 1); // 퍼질수록 잦아듦
				ring(rHead, { 255, 255, 255 }, (int)(235 * spread * spread));
				if (rHead >= 2) ring(rHead - 1, { 255, 245, 215 }, (int)(120 * spread * spread)); // 전연 안쪽 1픽셀 — 두께감만
			}
		}

		// ---- 접지 빛 웅덩이: 그림자와 같은 14×5 타원 — 폭발 직후 강하게, 섬광과 함께 잦아듦 ----
		{
			int glowA = (int)(55 * bright) + (tSec < 0.25f ? (int)(95 * (1.0f - tSec / 0.25f)) : 0);
			constexpr int poolHalf[5] = { 4, 6, 7, 6, 4 }; // dy -2..2 행별 반폭 (타원 스캔라인)
			constexpr int poolLv[5] = { 5, 8, 10, 8, 5 };  // 행별 알파 가중 (중앙이 밝음)
			for (int dy = -2; dy <= 2; dy++)
			{
				for (int gx = -poolHalf[dy + 2]; gx < poolHalf[dy + 2]; gx++) putGround(gx, dy, { 255, 250, 225 }, glowA * poolLv[dy + 2] / 10);
			}
		}

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		drawRectBatch(1, 1, cols, pts, n, (float)z);
	}
}
