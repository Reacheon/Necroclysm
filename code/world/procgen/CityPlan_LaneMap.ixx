export module CityPlan.LaneMap;

import std;
import util;
import CityPlan;

// ──────────────────────────────────────────────────────────
// CityPlan.LaneMap — Phase 2 도로 타일 데이터 모델
//
//   legacy MaskCell (axisH/axisV/offH/offV) 의 후속.
//   각 도로 타일이 어떤 lane 인지 직접 표현 → 페인팅 규칙이 단순/일관해짐.
//
//   특징
//     - 임의 polyline 도로 지원 (8방향 enum)
//     - 차선 표시 phase 결정용 alongIdx (centerline 따라 누적)
//     - 교차로 진입 / 다리 위 표시
//     - 동일 타일에 여러 엣지가 겹치면 highest materialTier 가 dominant
// ──────────────────────────────────────────────────────────

// 8방향 enum. centerline 진행 방향을 정수화.
//   0=E, 1=SE, 2=S, 3=SW, 4=W, 5=NW, 6=N, 7=NE.
//   sign 변환 (sx,sy) → dirIdx 는 dirIdxFromStep() 사용.
export enum class Dir8 : std::int8_t
{
    none = -1,
    east = 0,
    southEast,
    south,
    southWest,
    west,
    northEast = 7,
    north = 6,
    northWest = 5,
};

// (sx, sy) ∈ {-1,0,+1} 의 단계 → Dir8.
// (0,0) 은 none.
export inline Dir8 dirIdxFromStep(int sx, int sy)
{
    if (sx > 0 && sy == 0) return Dir8::east;
    if (sx > 0 && sy > 0)  return Dir8::southEast;
    if (sx == 0 && sy > 0) return Dir8::south;
    if (sx < 0 && sy > 0)  return Dir8::southWest;
    if (sx < 0 && sy == 0) return Dir8::west;
    if (sx < 0 && sy < 0)  return Dir8::northWest;
    if (sx == 0 && sy < 0) return Dir8::north;
    if (sx > 0 && sy < 0)  return Dir8::northEast;
    return Dir8::none;
}

// Dir8 이 축정렬 가로/세로 인지 검사 (legacy axisH/axisV 대응).
export inline bool isHorizontalDir(Dir8 d)
{
    return d == Dir8::east || d == Dir8::west;
}
export inline bool isVerticalDir(Dir8 d)
{
    return d == Dir8::north || d == Dir8::south;
}

// ──────────────── LaneTile ────────────────
// 한 도로 타일의 모든 메타데이터.
//   inJunction: 다른 방향 엣지가 같은 타일을 차지할 때 true.
//                lane 마킹 (centerlineSolid/laneStripe/centerlineDashed/medianStrip)
//                 페인터는 inJunction 셀에서 마킹을 출력하지 않는다.
//   onBridge   : 이 타일이 위치한 픽셀이 PixelType::bridge.
//                다리 난간 페인터가 outermost lane 검사 + onBridge 로 결정.
export struct LaneTile
{
    EdgeId        edge          = INVALID_ID;
    LaneKind      kind          = LaneKind::travelLane;

    // centerline 으로부터의 부호 있는 횡단 거리. -halfW..+halfW.
    std::int8_t   crossOff      = 0;

    // 외측(curb)에서 안쪽으로의 위치. 0..totalWidth-1.
    std::int8_t   posFromCurb   = 0;

    // centerline 진행 방향 (8방향).
    Dir8          dir           = Dir8::none;

    // centerline 따라 누적된 인덱스. 점선 phase 계산용.
    std::int32_t  alongIdx      = 0;

    // 도로 등급 힌트 (legacy materialTier 1=collector, 2=arterial, 3=highway).
    std::uint8_t  materialTier  = 0;

    // outermost lane (crossOff == ±halfW). 다리 난간 판정.
    bool          isOutermost   = false;

    // 교차로 영역 (다른 방향 엣지와 겹침).
    bool          inJunction    = false;

    // 이 타일이 위치한 픽셀이 PixelType::bridge.
    bool          onBridge      = false;
};

export using LaneMap = std::unordered_map<Point2, LaneTile, Point2::Hash>;

// ──────────────── 분류 헬퍼 ────────────────
// 도로 표면 (검정 아스팔트로 깔리는 모든 종류)
export inline bool isRoadSurfaceTile(const LaneTile& t)
{
    return isRoadSurfaceKind(t.kind);
}

// 진짜 보행자 인도 lane (현재 프로파일은 안 쓰지만 후일 확장)
export inline bool isPedestrianTile(const LaneTile& t)
{
    return t.kind == LaneKind::sidewalk;
}
