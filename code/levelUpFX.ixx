module;
#include <SDL3/SDL.h>

export module levelUpFX;

import std;
import util;
import globalVar;
import Player;

// 레벨업 연출 — 픽셀아트 섬광 기둥 + 발밑 충격파 + 접지 빛 웅덩이.
//
// 생애주기 (60ms/프레임, 총 16프레임 ≈ 1초):
//   프레임 0     가는 빛(폭 4px)이 절반 높이로 솟구침
//   프레임 1     전체 높이 도달, 폭 9px
//   프레임 2~4   절정 — 폭 14px(그림자 장축), 최대 밝기 유지
//   프레임 5~15  폭과 밝기가 함께 잦아들며 가는 선으로 수렴 후 소멸
// 내부에서 흐르는 광류 없이 폭·밝기 엔벨로프로만 움직임 (순간 섬광의 질감).
//
// 좌표 규약: 모든 픽셀은 게임픽셀 단위(1 이펙트 픽셀 = zoomScale 스크린픽셀)로
// 스프라이트 픽셀 그리드에 정렬됨. 기둥 폭은 짝수(좌우 대칭 쌍)라 그리드 정렬이 유지됨.
// 지면 요소(충격파·웅덩이)는 플레이어 그림자 타원(장축 14px, 단축 5px)의 납작비율 0.36을 따름.
//
// 상태는 발동 시각 하나뿐 — 매 프레임이 경과시간의 순수함수라 step/리스트 관리가 없음.

static Uint64 fxStartTick = 0; // 0 = 비활성

constexpr int FX_FRAME_MS = 60;      // 픽셀아트 애니 1프레임 길이
constexpr int FX_TOTAL_FRAMES = 16;  // 총 길이 약 1초
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
		int frame = (int)(t / FX_FRAME_MS);
		if (frame >= FX_TOTAL_FRAMES) { fxStartTick = 0; return; }
		float tSec = t / 1000.0f;

		int z = std::max(1, (int)zoomScale); // 줌 1~5 정수 스냅 전제 — 게임픽셀 1개 = z 스크린픽셀

		// 플레이어 앵커 (Entity::drawSelf와 동일 공식 — X축 시암 wrap 준수)
		int cx = (cameraW / 2) + z * (worldWrap::signedDeltaRenderX(cameraX, PlayerPtr->getX()) + PlayerPtr->getIntegerFakeX());
		int baseY = (cameraH / 2) + z * (PlayerPtr->getY() - cameraY + PlayerPtr->getIntegerFakeY() + 4); // 발밑 지면선

		// 배치 버퍼: 이번 프레임의 모든 이펙트 픽셀을 모아 drawRectBatch 1회로 그림
		constexpr size_t FX_MAX_PX = 3072;
		static SDL_Color cols[FX_MAX_PX];
		static Point2 pts[FX_MAX_PX];
		size_t n = 0;
		// putUp: 기둥 좌표 (gy 0 = 발밑 바로 위, 위로 증가) / putGround: 지면 평면 좌표 (gy 양수 = 화면 아래쪽)
		auto putUp = [&](int gx, int gy, SDL_Color c, int a) { if (n >= FX_MAX_PX || a < 6) return; c.a = (Uint8)std::min(a, 255); cols[n] = c; pts[n] = { cx + gx * z, baseY - (gy + 1) * z }; n++; };
		auto putGround = [&](int gx, int gy, SDL_Color c, int a) { if (n >= FX_MAX_PX || a < 6) return; c.a = (Uint8)std::min(a, 255); cols[n] = c; pts[n] = { cx + gx * z, baseY + gy * z }; n++; };

		// ---- 섬광 엔벨로프 ----
		float wf = frame == 0 ? 2.0f : (frame == 1 ? 4.5f : (frame <= 4 ? 7.0f : 7.0f * (15 - frame) / 11.0f)); // 폭 (컬럼 쌍 수)
		float bright = frame <= 4 ? 1.0f : (15 - frame) / 11.0f; // 밝기 — 절정 유지 후 소멸까지 선형 감쇠
		int hScale = frame == 0 ? 50 : 100; // 높이 % — 첫 프레임만 절반 (솟구치는 순간)

		// 캐릭터 백색 섬광: 기둥과 같은 엔벨로프로 감쇠 (첫 프레임 점등은 trigger()가 담당)
		PlayerPtr->flash = { 255, 255, 255, (Uint8)(150 * bright) };

		// ---- 섬광 기둥: 좌우대칭 7겹 컬럼 (절정 폭 14게임픽셀 = 그림자 장축) ----
		struct FxCol { int h; SDL_Color c; int baseA; };
		constexpr FxCol pillar[7] = {
			{ 54, { 255, 255, 255 }, 190 }, // 코어: 순백 — 뒤의 캐릭터가 비쳐 보이도록 반투명 유지
			{ 53, { 255, 255, 255 }, 178 },
			{ 52, { 255, 255, 255 }, 162 },
			{ 50, { 255, 248, 220 }, 142 }, // 미드: 따뜻한 금빛으로 전이
			{ 48, { 255, 238, 195 }, 118 },
			{ 45, { 205, 220, 250 }, 88 },  // 아우터: 차가운 청백
			{ 42, { 170, 205, 255 }, 52 },
		};
		for (int d = 0; d < 7; d++)
		{
			const FxCol& col = pillar[d];
			float edge = std::clamp(wf - d, 0.0f, 1.0f); // 폭 엔벨로프 가장자리 — 바깥 컬럼은 부분 알파로 부드럽게 등장/퇴장
			if (edge <= 0.0f) continue;
			int h = col.h * hScale / 100;
			// 접지 곡선: 실제 그림자 타원(단축 반지름 2.5)은 중앙이 너무 평평해 직선으로 읽힘 → 포물선으로 곡률 과장 [3,3,3,2,2,1,0]
			int bot = (int)std::lround(3.2 * (1.0 - std::pow((d + 0.5) / 7.0, 2)));
			for (int gy = -bot; gy < h; gy++)
			{
				float v = (float)(gy + bot) / (h + bot);
				float vert = v < 0.55f ? 1.0f : (1.0f - v) / 0.45f; // 상단 45% 구간 알파 그라데이션 페이드
				int a = (int)(col.baseA * vert * edge * bright);
				putUp(-(d + 1), gy, col.c, a);
				putUp(d, gy, col.c, a);
			}
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
