export module worldWrap;

import std;
import constVar;

// ════════════════════════════════════════════════════════════════════════
// 월드 X축 원기둥 wrap — 메르카토르 세계지도 동/서 경계 심리스 연결.
//
//   좌표계는 Null Island(서아프리카 기니만, 경도 0°·위도 0°)를 원점으로 함.
//     - 타일 X 범위: [-WORLD_TILE_W/2, +WORLD_TILE_W/2) = [-1,036,800, +1,036,800)
//     - 시암 (wrap 경계) 은 X = ±WORLD_TILE_W/2 = 반대 자오선(180°) = 태평양 한가운데
//     - Y축은 양극(보이지않는 벽)이라 wrap 안 함
//
//   설계 핵심:
//     - 모든 X "저장/인덱싱" 좌표는 mod로 정규화 ([-W/2, +W/2) 중앙 범위 유지)
//     - 두 X 사이의 "거리/방향"은 항상 signedDelta로 계산 (최단 부호 있는 거리)
//     - 시암이 태평양에 있으므로 도시·도로 생성 코드는 wrap을 신경 쓸 필요 없음
//       (도시는 육지에만 생성되고, 시암 근처 도시가 사실상 없음)
//
//   wrap이 적용되는 진입점 (이 함수들만 호출):
//     - World 모듈 chunk map 룩업    — 청크 인덱스 wrap (chunk map key)
//     - sectorFromTile               — 섹터 인덱스 wrap (sector cache key)
//     - worldGrid::worldPixel        — 픽셀 좌표 wrap (mmap 인덱스, [0, W_PIXEL))
//     - Coord::setGrid / setXY       — 플레이어/엔티티 저장 좌표 정규화
//     - 엔티티/스프라이트 화면 좌표 계산 — signedDeltaRenderX 사용
//
//   wrap을 *적용하지 않는* 곳:
//     - 렌더 루프의 tgtX (render-space, 음수/W 초과 가능 — World가 알아서 wrap)
//     - PolyLine 정점 raw 좌표 (그릴 때 첫점 기준 누적 변환)
// ════════════════════════════════════════════════════════════════════════

export namespace worldWrap
{
    inline constexpr int WORLD_PIXEL_W = 43200;
    inline constexpr int WORLD_PIXEL_H = 21600;
    inline constexpr int WORLD_TILE_W  = WORLD_PIXEL_W * TILE_PER_PIXEL; // 2,073,600
    inline constexpr int WORLD_TILE_H  = WORLD_PIXEL_H * TILE_PER_PIXEL; // 1,036,800
    inline constexpr int WORLD_CHUNK_W = WORLD_TILE_W / CHUNK_SIZE_X;    //   129,600
}

namespace worldWrap
{
    //음수 안전 mod → [0, mod). C++ %는 음수면 음수 결과라 보정.
    constexpr int wrapMod(int v, int mod) noexcept
    {
        const int r = v % mod;
        return (r < 0) ? r + mod : r;
    }

    //중앙 정렬 wrap → [-mod/2, +mod/2). 원점이 0인 좌표계용.
    //  v=mod/2 → -mod/2 (반대 자오선의 두 끝은 같은 점), v=mod → 0 (한 바퀴 동일).
    constexpr int wrapModCentered(int v, int mod) noexcept
    {
        return wrapMod(v + mod / 2, mod) - mod / 2;
    }

    //부호 있는 최단 거리 in [-mod/2, +mod/2). 지구 한 바퀴 vs 반대쪽 중 짧은 쪽 자동 선택.
    constexpr int signedDeltaMod(int from, int to, int mod) noexcept
    {
        int d = (to - from) % mod;
        if (d < 0) d += mod;
        if (d >= mod / 2) d -= mod;
        return d;
    }
}

export namespace worldWrap
{
    //타일 좌표(논리) X 중앙 wrap → [-WORLD_TILE_W/2, +WORLD_TILE_W/2).
    //  Null Island 원점 좌표계. 시암은 ±W/2 (태평양).
    constexpr int wrapTileX(int x) noexcept
    {
        return wrapModCentered(x, WORLD_TILE_W);
    }

    //청크 인덱스 X 중앙 wrap → [-WORLD_CHUNK_W/2, +WORLD_CHUNK_W/2).
    constexpr int wrapChunkX(int cx) noexcept
    {
        return wrapModCentered(cx, WORLD_CHUNK_W);
    }

    //픽셀 인덱스 X wrap → [0, WORLD_PIXEL_W).
    //  worldPixel은 mmap-relative 인덱스(0-base)를 받으므로 중앙 wrap 아님.
    constexpr int wrapPixelX(int px) noexcept
    {
        return wrapMod(px, WORLD_PIXEL_W);
    }

    //섹터 인덱스 X 중앙 wrap → [-W_SECTOR/2, +W_SECTOR/2).
    //  Sector 모듈 import 회피 위해 sectorTiles를 인자로 받음.
    constexpr int wrapSectorX(int sx, int sectorTiles) noexcept
    {
        const int sectorW = WORLD_TILE_W / sectorTiles;
        return wrapModCentered(sx, sectorW);
    }

    //타일 좌표 공간 부호 있는 최단 거리. 시암 가로지른 두 점도 짧은 쪽으로 답.
    constexpr int signedDeltaTileX(int from, int to) noexcept
    {
        return signedDeltaMod(from, to, WORLD_TILE_W);
    }

    //청크 좌표 공간 부호 있는 최단 거리 — 인접 청크 거리 비교 등.
    constexpr int signedDeltaChunkX(int from, int to) noexcept
    {
        return signedDeltaMod(from, to, WORLD_CHUNK_W);
    }

    //렌더 X 공간(= 16 * gridX + 8) 부호 있는 최단 거리.
    //  cameraX, Coord::getX() 등이 사용하는 스케일.
    //  엔티티 그릴 때: drawX = (cameraW/2) + zoomScale * signedDeltaRenderX(cameraX, entity.getX())
    constexpr int RENDER_X_SPAN = WORLD_TILE_W * 16;
    constexpr int signedDeltaRenderX(int cameraX, int worldX) noexcept
    {
        return signedDeltaMod(cameraX, worldX, RENDER_X_SPAN);
    }

    //렌더 X 공간 중앙 wrap → [-RENDER_X_SPAN/2, +RENDER_X_SPAN/2).
    //  Coord::setXY 같은 저장점에서 사용.
    constexpr int wrapRenderX(int x) noexcept
    {
        return wrapModCentered(x, RENDER_X_SPAN);
    }
}
