export module CityGen.Paint;

import std;
import util;
import constVar;
import SectorBiome;
import World;
import CityGen.Common;
import CityGen.PixelGrid;
import CityGen.RoadPlan;

// ──────────────────────────────────────────────────────────
// CityGen.Paint — RoadMask/SidewalkMask를 실제 월드 타일로 칠함
//
// Pass 순서:
//   (a) 베이스 아스팔트
//   (b) 차선 마킹  (외측 가장자리선 없음 → sidewalk 와 직접 접함)
//   (c) 횡단보도   (arterial+ 교차로 진입대 black 정리 후 zebra)
//   (d) 사이드워크 (paver)
//   (e) 다리 난간  (bridge 픽셀 위 도로의 외측 가장자리에 wireFence)
//
// H/V 듀얼 코드는 Axis 파라미터로 통합.
// ──────────────────────────────────────────────────────────

// ──────────────── 청크 보장 헬퍼 ────────────────
export inline void ensureChunk(int tileX, int tileY, int z)
{
    int cx, cy;
    World::ins()->changeToChunkCoord(tileX, tileY, cx, cy);
    if (!World::ins()->existChunk(cx, cy, z)) World::ins()->createChunk(cx, cy, z);
}

export inline void paintFloor(int tileX, int tileY, int z, int floorId)
{
    ensureChunk(tileX, tileY, z);
    setFloor({ tileX, tileY, z }, floorId);
}

export inline void paintWall(int tileX, int tileY, int z, int wallId)
{
    ensureChunk(tileX, tileY, z);
    setWall({ tileX, tileY, z }, wallId);
}

// ──────────────── (a) 베이스 아스팔트 ────────────────
export void paintAsphaltBase(const RoadMask& mask, int z)
{
    for (const auto& [pt, cell] : mask)
    {
        if (cell.tier == RoadTier::none) continue;
        paintFloor(pt.x, pt.y, z, itemID::blackAsphalt);
    }
}

// ──────────────── (b) 차선 마킹 ────────────────
//   외측 가장자리선(offset==halfW)은 칠하지 않음 — 도로 끝이 검정 아스팔트로
//   끝나고 paver sidewalk와 직접 맞닿아 시각적으로 깔끔.
//   collector (9w, halfW=4):
//     offset 0     : 노란 점선 (2차선 중앙선)
//   arterial (17w, halfW=8):
//     offset 0     : 노란 실선 (중앙)
//     offset 4     : 흰 점선 (같은방향 차선 분리 — 4차선 효과)
//   highway (23w, halfW=11):
//     offset 0,1   : 노란 실선 (3타일 미디언)
//     offset 5     : 흰 점선 (차선 분리 — 6차선 효과)
//   교차로(axisH && axisV) 타일은 마킹 없음
enum class LaneMark : std::uint8_t { none, yellowSolid, yellowDashed, whiteSolid, whiteDashed };

static LaneMark classifyMark(const MaskCell& c, Axis axis)
{
    if (c.axisH && c.axisV) return LaneMark::none;
    bool isOnAxis = (axis == Axis::horizontal) ? c.axisH : c.axisV;
    if (!isOnAxis) return LaneMark::none;
    int o = (axis == Axis::horizontal) ? (int)c.offH : (int)c.offV;
    switch (c.tier)
    {
    case RoadTier::collector:
        if (o == 0) return LaneMark::yellowDashed;
        break;
    case RoadTier::arterial:
        if (o == 0) return LaneMark::yellowSolid;
        if (o == 4) return LaneMark::whiteDashed;
        break;
    case RoadTier::highway:
        if (o == 0 || o == 1) return LaneMark::yellowSolid;
        if (o == 5) return LaneMark::whiteDashed;
        break;
    default: break;
    }
    return LaneMark::none;
}

static int markFloorId(LaneMark m)
{
    switch (m)
    {
    case LaneMark::yellowSolid:
    case LaneMark::yellowDashed: return itemID::yellowAsphalt;
    case LaneMark::whiteSolid:
    case LaneMark::whiteDashed:  return itemID::whiteAsphalt;
    default: return -1;
    }
}

static bool isDashedMark(LaneMark m)
{
    return m == LaneMark::yellowDashed || m == LaneMark::whiteDashed;
}

// 한 축의 점선 마킹을 sweep 으로 칠함.
//   axis=H → row(y) 별로 모아 x로 정렬, localPos period로 dash 패턴 적용.
//   axis=V → col(x) 별로 모아 y로 정렬.
static void paintDashedMarks(const RoadMask& mask, int z, Axis axis)
{
    // primary = 같은 라인을 묶는 키, secondary = 라인 위 위치
    std::unordered_map<int, std::vector<std::tuple<int, LaneMark>>> lines;
    for (const auto& [pt, cell] : mask)
    {
        LaneMark m = classifyMark(cell, axis);
        if (!isDashedMark(m)) continue;
        int primary = (axis == Axis::horizontal) ? pt.y : pt.x;
        int secondary = (axis == Axis::horizontal) ? pt.x : pt.y;
        lines[primary].push_back({ secondary, m });
    }
    const int period = CG_DASH_ON + CG_DASH_OFF;
    for (auto& [primary, secs] : lines)
    {
        std::sort(secs.begin(), secs.end(),
            [](auto& a, auto& b) { return std::get<0>(a) < std::get<0>(b); });
        int localPos = 0;
        int prev = std::numeric_limits<int>::min();
        for (const auto& [sec, m] : secs)
        {
            if (sec != prev + 1) localPos = 0;
            if ((localPos % period) < CG_DASH_ON)
            {
                int x = (axis == Axis::horizontal) ? sec : primary;
                int y = (axis == Axis::horizontal) ? primary : sec;
                paintFloor(x, y, z, markFloorId(m));
            }
            localPos++;
            prev = sec;
        }
    }
}

export void paintLaneMarkings(const RoadMask& mask, int z)
{
    // Pass A: 솔리드 — localPos 불필요, 한 번에 칠함 (양쪽 축 동시)
    for (const auto& [pt, cell] : mask)
    {
        for (Axis ax : { Axis::horizontal, Axis::vertical })
        {
            LaneMark m = classifyMark(cell, ax);
            if (m != LaneMark::none && !isDashedMark(m))
                paintFloor(pt.x, pt.y, z, markFloorId(m));
        }
    }
    // Pass B/C: 점선 — 축별 sweep
    paintDashedMarks(mask, z, Axis::horizontal);
    paintDashedMarks(mask, z, Axis::vertical);
}

// ──────────────── (c) 횡단보도 ────────────────
//   각 면 외측으로 offset 1..CG_CROSSWALK_DEPTH 진입대를 한꺼번에 다시 칠함:
//     홀수 offset → whiteAsphalt (zebra 줄무늬)
//     짝수 offset → blackAsphalt (gap — (b)에서 칠한 차선마킹 제거)
//   이로써 zebra 줄 사이로 노란 중앙선/흰 점선이 비치는 현상이 사라짐.
//   각 스트라이프는 해당 면의 전폭을 1타일 두께로 횡단 (차량 진행 방향에 수직).
export void paintCrosswalks(const RoadMask& mask, int z)
{
    auto isRoad = [&](int x, int y) {
        auto it = mask.find({ x, y });
        return it != mask.end() && it->second.tier != RoadTier::none;
    };

    auto intersections = findMajorIntersections(mask);
    for (const auto& ib : intersections)
    {
        for (int off = 1; off <= CG_CROSSWALK_DEPTH; off++)
        {
            int floorId = (off % 2 == 1) ? itemID::whiteAsphalt : itemID::blackAsphalt;

            // N/S — 진입로는 axisV. 스트라이프는 수평 (X 스팬, Y 두께 1).
            for (int sign : { -1, +1 })
            {
                int y = (sign < 0) ? (ib.minY - off) : (ib.maxY + off);
                for (int x = ib.minX; x <= ib.maxX; x++)
                    if (isRoad(x, y)) paintFloor(x, y, z, floorId);
            }
            // W/E — 스트라이프는 수직 (Y 스팬, X 두께 1).
            for (int sign : { -1, +1 })
            {
                int x = (sign < 0) ? (ib.minX - off) : (ib.maxX + off);
                for (int y = ib.minY; y <= ib.maxY; y++)
                    if (isRoad(x, y)) paintFloor(x, y, z, floorId);
            }
        }
    }
}

// ──────────────── (d) 사이드워크 ────────────────
//   횡단보도는 whiteAsphalt이므로 색상이 분리되어 교차로에서 zebra가 도드라짐.
export void paintSidewalks(const SidewalkMask& sidewalks, int z)
{
    for (const Point2& pt : sidewalks)
        paintFloor(pt.x, pt.y, z, itemID::paver);
}

// ──────────────── (e) 다리 난간 ────────────────
//   bridge 픽셀 위 도로의 외측 가장자리에 wireFence 벽.
//   axisH only + offH == halfW → 수평도로의 N/S 외측 행 = 난간
//   axisV only + offV == halfW → 수직도로의 E/W 외측 열 = 난간
//   (교차로는 스킵: 다리 위 교차로는 드물고 시각적으로 난잡해짐)
//   바닥은 asphalt 유지 → "다리 상판 + 가드레일" 시각.
export void paintBridgeRailings(const RoadMask& mask, const PixelGrid& grid, int z)
{
    for (const auto& [pt, cell] : mask)
    {
        if (cell.tier == RoadTier::none) continue;
        int px = pixelFromTile(pt.x);
        int py = pixelFromTile(pt.y);
        if (grid.at(px, py) != PixelType::bridge) continue;

        int halfW = halfWidthOfTier(cell.tier);
        if (halfW <= 0) continue;

        bool isOuterH = cell.axisH && !cell.axisV && (int)cell.offH == halfW;
        bool isOuterV = cell.axisV && !cell.axisH && (int)cell.offV == halfW;
        if (isOuterH || isOuterV)
            paintWall(pt.x, pt.y, z, itemID::wireFence);
    }
}
