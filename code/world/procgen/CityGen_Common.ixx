export module CityGen.Common;

import std;
import util;

// ──────────────────────────────────────────────────────────
// CityGen.Common — 도시 생성 파이프라인 공용 자료형/상수
//   여러 단계 모듈에서 공유하는 타입과 튜닝 상수만 모음.
// ──────────────────────────────────────────────────────────

// ──────────────── 도로 계층 폭 ────────────────
// 홀수 폭 = 중앙 1타일이 확실히 정해짐
export constexpr int CG_HIGHWAY_WIDTH   = 23;
export constexpr int CG_ARTERIAL_WIDTH  = 17;
export constexpr int CG_COLLECTOR_WIDTH = 9;

// 사이드워크 폭 (도로 외측 링)
export constexpr int CG_SIDEWALK_WIDTH = 2;

// 횡단보도 — 교차로 바깥으로 진입대 깊이(타일).
//   진입대 전역(offset 1..DEPTH)을 black으로 깔아 차선마킹 제거 →
//   홀수 offset 에 white 줄무늬 (1,3,5 → 3줄 zebra)
export constexpr int CG_CROSSWALK_DEPTH = 5;

// 차선 마킹 점선 파라미터
export constexpr int CG_DASH_ON  = 3;
export constexpr int CG_DASH_OFF = 3;

// 필지 분할 파라미터
export constexpr int CG_LOT_MIN_DIM   = 11;
export constexpr int CG_LOT_SPLIT_DIM = 34;
export constexpr int CG_LOT_SETBACK   = 2;  // 건물-lot경계 여백

// ──────────────── 도로 마스크 자료형 ────────────────
export enum class RoadTier : std::uint8_t { none = 0, collector = 1, arterial = 2, highway = 3 };

export struct MaskCell
{
    RoadTier tier = RoadTier::none;
    bool axisH = false;
    bool axisV = false;
    std::int8_t offH = 127;
    std::int8_t offV = 127;
};

export using RoadMask = std::unordered_map<Point2, MaskCell, Point2::Hash>;
export using SidewalkMask = std::unordered_set<Point2, Point2::Hash>;

// H/V 공유 코드용 축 식별자
export enum class Axis : std::uint8_t { horizontal, vertical };

export inline int halfWidthOfTier(RoadTier t)
{
    switch (t)
    {
    case RoadTier::collector: return CG_COLLECTOR_WIDTH / 2;
    case RoadTier::arterial:  return CG_ARTERIAL_WIDTH / 2;
    case RoadTier::highway:   return CG_HIGHWAY_WIDTH / 2;
    default: return 0;
    }
}

export inline void markMaskTile(RoadMask& mask, int tx, int ty, RoadTier tier, bool isHorizontal, std::int8_t off)
{
    MaskCell& c = mask[{ tx, ty }];
    if ((int)tier > (int)c.tier) c.tier = tier;
    if (isHorizontal) { c.axisH = true; if (off < c.offH) c.offH = off; }
    else              { c.axisV = true; if (off < c.offV) c.offV = off; }
}

export inline int manhattan(Point2 a, Point2 b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
