export module CityPlan.Renderer;

import std;
import util;
import constVar;
import SectorBiome;
import World;
import CityPlan;
import CityPlan.RoadProfile;
import CityPlan.LaneMap;
import CityGen.Common;
import CityGen.PixelGrid;
import CityGen.Paint;

// ──────────────────────────────────────────────────────────
// CityPlan.Renderer — Phase 2 도로 렌더링 파이프라인
//
//   입력: CityPlan + PixelGrid
//   출력:
//     - LaneMap   : 모든 도로 타일 (lane 별 분류)
//     - RoadMask  : legacy 호환용 (Lots/findSubBlocks 가 사용)
//     - SidewalkMask : Chebyshev 2-tile ring (legacy 동작 동일)
//     - 페인트된 청크 (asphalt/markings/crosswalks/sidewalks/bridge railings)
//
//   Pass 순서
//     1) carveLaneMap        : RoadEdge centerline 들을 LaneMap 으로 carve
//     2) buildSidewalkSet    : LaneMap 외곽 ring → SidewalkMask
//     3) synthesizeLegacyMask: LaneMap → RoadMask (Lots 호환)
//     4) paintAsphaltBase    : 모든 road surface 타일 = blackAsphalt
//     5) paintLaneMarkings   : LaneKind 기반 yellow/white (junction 셀 스킵)
//     6) paintCrosswalks     : RoadGraph 노드 기반 zebra (legacy 의 mask-cluster 폐기)
//     7) paintSidewalks      : paver
//     8) paintBridgeRailings : isOutermost && onBridge && !inJunction
//
//   불변식 (Phase 2 입력은 모두 axis-aligned):
//     legacy 출력과 비교하여 다음만 차이 허용:
//       - 횡단보도 깊이 5 → 3 (의도된 시각 개선)
//       - 횡단보도 검출이 RoadGraph 노드 기반 (mask-cluster 보다 정확)
//     그 외 (asphalt/stripes/sidewalks/bridge rails) 는 1타일 단위로 동일.
// ──────────────────────────────────────────────────────────

// ──────────────── (0) materialTier ↔ legacy RoadTier 매핑 ────────────────
static RoadTier tierFromMaterial(std::uint8_t materialTier)
{
    switch (materialTier)
    {
    case 3: return RoadTier::highway;
    case 2: return RoadTier::arterial;
    case 1: return RoadTier::collector;
    default: return RoadTier::collector;
    }
}

// ──────────────── (1) Carve: RoadGraph → LaneMap ────────────────
// 한 타일에 lane 메타데이터 기록. 충돌 처리:
//   - 같은 edge: skip
//   - 다른 edge 같은 방향: 첫 셀 유지 (continuous extension)
//   - 다른 edge 다른 방향: inJunction = true
//   - 더 높은 materialTier 가 dominant 데이터 차지
static void placeLaneTile(LaneMap& map, Point2 pt, EdgeId eid, int dw, int halfW,
                           Dir8 dir, std::int32_t alongIdx,
                           const RoadProfile& profile, const PixelGrid& grid)
{
    int posFromCurb = dw + halfW;  // 0..totalWidth-1

    // dw 가 어느 lane 안에 떨어지는지 결정
    LaneKind kind = LaneKind::travelLane;
    int acc = 0;
    for (std::size_t i = 0; i < profile.lanes.size(); i++)
    {
        if (posFromCurb < acc + profile.lanes[i].width)
        {
            kind = profile.lanes[i].kind;
            break;
        }
        acc += profile.lanes[i].width;
    }

    LaneTile& cell = map[pt];

    bool hasExisting = (cell.edge != INVALID_ID);
    bool sameEdge    = hasExisting && (cell.edge == eid);
    bool diffDir     = hasExisting && (cell.dir != dir);

    // 다른 방향 엣지가 이미 자리잡았으면 junction (lane 마킹 억제용)
    if (hasExisting && !sameEdge && diffDir)
        cell.inJunction = true;

    // 우선순위: higher materialTier 가 dominant 데이터로 남음
    bool override = !hasExisting || profile.materialTier > cell.materialTier;
    if (!override) return;

    cell.edge         = eid;
    cell.kind         = kind;
    cell.crossOff     = (std::int8_t)std::clamp(dw, -127, 127);
    cell.posFromCurb  = (std::int8_t)std::clamp(posFromCurb, 0, 127);
    cell.dir          = dir;
    cell.alongIdx     = alongIdx;
    cell.materialTier = profile.materialTier;
    cell.isOutermost  = (dw == -halfW || dw == halfW);

    int px = pixelFromTile(pt.x);
    int py = pixelFromTile(pt.y);
    cell.onBridge     = (grid.at(px, py) == PixelType::bridge);
}

// 한 segment carve. 입력 (a,b) 가 axis-aligned 일 때 legacy carveStrip 과
// 동일한 결과를 LaneMap 에 기록. (대각선 입력은 Phase 3+ 에서 일반화 예정 —
// 현재는 sx/sy 가 모두 0이 아니어도 단순 단계 진행으로 처리, legacy 와 동일)
static void carveSegmentToLaneMap(LaneMap& map, EdgeId eid, Point2 a, Point2 b,
                                   const RoadProfile& profile, const PixelGrid& grid)
{
    int dx = b.x - a.x, dy = b.y - a.y;
    int sx = (dx > 0) - (dx < 0);
    int sy = (dy > 0) - (dy < 0);
    const int halfW = profile.totalWidth / 2;

    // 단일 점: 양 축 cross 새김 (legacy carveStrip 의 sx==sy==0 분기)
    if (sx == 0 && sy == 0)
    {
        for (int dw = -halfW; dw <= halfW; dw++)
        {
            placeLaneTile(map, { a.x + dw, a.y }, eid, dw, halfW, Dir8::east,  0, profile, grid);
            placeLaneTile(map, { a.x, a.y + dw }, eid, dw, halfW, Dir8::south, 0, profile, grid);
        }
        return;
    }

    bool isHorizontal = (sx != 0);  // legacy 와 동일한 "가로 우선" 규약
    Dir8 dir = dirIdxFromStep(sx, sy);

    int x = a.x, y = a.y;
    std::int32_t alongIdx = 0;
    while (true)
    {
        for (int dw = -halfW; dw <= halfW; dw++)
        {
            int tx = isHorizontal ? x : x + dw;
            int ty = isHorizontal ? y + dw : y;
            placeLaneTile(map, { tx, ty }, eid, dw, halfW, dir, alongIdx, profile, grid);
        }
        if (x == b.x && y == b.y) break;
        x += sx; y += sy;
        alongIdx++;
    }
}

export void carveLaneMap(const CityPlan& plan, const PixelGrid& grid, LaneMap& laneMap)
{
    // tier 낮은 순서로 carve → high tier 가 자연스럽게 override.
    //   collector(1) → arterial(2) → highway(3) 순서.
    for (std::uint8_t tier = 1; tier <= 3; tier++)
    {
        for (const RoadEdge& e : plan.graph.edges)
        {
            const RoadProfile* p = RoadProfileCatalog::ins().get(e.profile);
            if (!p) continue;
            if (p->materialTier != tier) continue;
            if (e.centerline.size() < 2) continue;

            for (std::size_t i = 1; i < e.centerline.size(); i++)
                carveSegmentToLaneMap(laneMap, e.id, e.centerline[i-1], e.centerline[i], *p, grid);
        }
    }
}

// ──────────────── (2) Sidewalk ring (Chebyshev 2-tile) ────────────────
// legacy buildSidewalkMask 와 1:1 동일 동작 — LaneMap 의 도로 타일 주위에
// CG_SIDEWALK_WIDTH 깊이의 ring 을 만든다 (도시 픽셀 안에서만, 도로와 겹치지 않음).
export void buildSidewalkSetFromLaneMap(const LaneMap& laneMap,
                                         const std::unordered_set<Point2, Point2::Hash>& allowedPx,
                                         SidewalkMask& sidewalks)
{
    for (const auto& [pt, lt] : laneMap)
    {
        if (!isRoadSurfaceTile(lt)) continue;
        for (int dy = -CG_SIDEWALK_WIDTH; dy <= CG_SIDEWALK_WIDTH; dy++)
        {
            for (int dx = -CG_SIDEWALK_WIDTH; dx <= CG_SIDEWALK_WIDTH; dx++)
            {
                if (dx == 0 && dy == 0) continue;
                Point2 np{ pt.x + dx, pt.y + dy };
                if (laneMap.contains(np)) continue;
                Point2 npx{ pixelFromTile(np.x), pixelFromTile(np.y) };
                if (!allowedPx.contains(npx)) continue;
                sidewalks.insert(np);
            }
        }
    }
}

// ──────────────── (3) Legacy RoadMask synthesis ────────────────
// CityGen.Lots.findSubBlocks 가 RoadMask 를 사용하므로 합성해 준다.
//   tier      : materialTier 매핑
//   axisH/V   : Dir8 의 가로/세로 분류 (대각선은 둘 다 true)
//   offH/V    : |crossOff| (Lots 가 실제 사용하는 정보는 tier 와 존재 여부뿐)
export void synthesizeLegacyMask(const LaneMap& laneMap, RoadMask& mask)
{
    for (const auto& [pt, lt] : laneMap)
    {
        MaskCell c;
        c.tier = tierFromMaterial(lt.materialTier);

        if (isHorizontalDir(lt.dir))      c.axisH = true;
        else if (isVerticalDir(lt.dir))   c.axisV = true;
        else                              { c.axisH = true; c.axisV = true; }

        int absOff = std::abs((int)lt.crossOff);
        if (absOff > 127) absOff = 127;
        c.offH = (std::int8_t)absOff;
        c.offV = (std::int8_t)absOff;
        mask[pt] = c;
    }
}

// ──────────────── (4) 페인트: 베이스 아스팔트 ────────────────
export void paintAsphaltBaseFromLaneMap(const LaneMap& laneMap, int z)
{
    for (const auto& [pt, lt] : laneMap)
    {
        if (!isRoadSurfaceTile(lt)) continue;
        paintFloor(pt.x, pt.y, z, itemID::blackAsphalt);
    }
}

// ──────────────── (5) 페인트: 차선 마킹 ────────────────
// LaneKind 기반 — junction 셀은 스킵.
//   centerlineSolid / medianStrip → yellowAsphalt (실선)
//   centerlineDashed              → yellowAsphalt (alongIdx % period)
//   laneStripe                    → whiteAsphalt  (alongIdx % period)
export void paintLaneMarkingsFromLaneMap(const LaneMap& laneMap, int z)
{
    const int period = CG_DASH_ON + CG_DASH_OFF;

    for (const auto& [pt, lt] : laneMap)
    {
        if (lt.inJunction) continue;

        switch (lt.kind)
        {
        case LaneKind::centerlineSolid:
        case LaneKind::medianStrip:
            paintFloor(pt.x, pt.y, z, itemID::yellowAsphalt);
            break;

        case LaneKind::centerlineDashed:
        {
            int phase = ((lt.alongIdx % period) + period) % period;
            if (phase < CG_DASH_ON)
                paintFloor(pt.x, pt.y, z, itemID::yellowAsphalt);
            break;
        }

        case LaneKind::laneStripe:
        {
            int phase = ((lt.alongIdx % period) + period) % period;
            if (phase < CG_DASH_ON)
                paintFloor(pt.x, pt.y, z, itemID::whiteAsphalt);
            break;
        }

        default:
            break;
        }
    }
}

// ──────────────── (6) 페인트: 횡단보도 ────────────────
// LaneMap 의 inJunction 클러스터 (서로 다른 방향 엣지가 만난 곳) 기반.
//   legacy findMajorIntersections 와 동일한 형태 알고리즘 — axisH&&axisV
//   대신 inJunction (방향 충돌로 carve 시 이미 표시) 사용.
//
//   필터:
//     - 클러스터에 materialTier ≥ 2 셀이 있어야 함 (collector-only 교차 제외)
//     - bbox 외측 4면 중 ≥ 3면에서 perpendicular 도로가 빠져나가야 함
//       (L-자 굴절 / dead-end 제외)
//
//   페인트:
//     - 각 면 외측으로 깊이 1..CROSSWALK_DEPTH_PHASE2 진입대.
//     - 줄무늬는 차량 진행 방향과 평행. 한 stripe 의 진행 방향 길이 = BAR_LENGTH.
//     - 깊이를 BAR_LENGTH 단위 layer 로 나누고, layer index 가 홀수면 cross-traffic
//       패리티를 1tile shift → 인접 layer 끼리 X 가 어긋난 지그재그 zebra.
//
//   Phase 4: 노드 기반 (anchor 의 scrambleJunction 명시) 으로 보완 예정.

namespace {

// 횡단보도 깊이 — 진입대 전체가 진행 방향으로 차지하는 타일 수.
//   여러 개의 BAR_LENGTH 짜리 bar layer 가 stack 되며 layer 별로 X 패리티가 flip
//   → 지그재그 zebra. 8 = 2 layer (4+4).
constexpr int CROSSWALK_DEPTH_PHASE2 = 8;

// 흰색 bar 하나의 진행 방향 길이. layer 가 이 단위로 끊기고 다음 layer 는
// cross-traffic 패리티가 1tile shift 된다.
constexpr int CROSSWALK_BAR_LENGTH = 4;

struct LaneJunction
{
    int minX, minY, maxX, maxY;
    std::uint8_t maxTier;
};

std::vector<LaneJunction> findLaneJunctions(const LaneMap& laneMap)
{
    std::unordered_set<Point2, Point2::Hash> visited;
    std::vector<LaneJunction> out;

    for (const auto& [pt, lt] : laneMap)
    {
        if (!lt.inJunction) continue;
        if (lt.materialTier < 2) continue;
        if (visited.contains(pt)) continue;

        // BFS — inJunction && materialTier ≥ 2 인 4-연결 셀 클러스터
        std::queue<Point2> q;
        q.push(pt); visited.insert(pt);
        int mnx = pt.x, mny = pt.y, mxx = pt.x, mxy = pt.y;
        std::uint8_t maxT = lt.materialTier;
        constexpr int dx[] = { 1,-1,0,0 }, dy[] = { 0,0,1,-1 };

        while (!q.empty())
        {
            Point2 cur = q.front(); q.pop();
            if (cur.x < mnx) mnx = cur.x;
            if (cur.y < mny) mny = cur.y;
            if (cur.x > mxx) mxx = cur.x;
            if (cur.y > mxy) mxy = cur.y;

            for (int k = 0; k < 4; k++)
            {
                Point2 n{ cur.x + dx[k], cur.y + dy[k] };
                if (visited.contains(n)) continue;
                auto it = laneMap.find(n);
                if (it == laneMap.end()) continue;
                if (!it->second.inJunction) continue;
                if (it->second.materialTier < 2) continue;
                visited.insert(n); q.push(n);
                if (it->second.materialTier > maxT) maxT = it->second.materialTier;
            }
        }

        out.push_back({ mnx, mny, mxx, mxy, maxT });
    }
    return out;
}

bool hasContinuationOnSide(const LaneMap& laneMap, int fixed, int spanLo, int spanHi, bool sideIsX)
{
    // sideIsX=true: 이 면이 X축 스팬 (N/S 면), perpendicular = 세로 방향 도로
    // sideIsX=false: Y축 스팬 (W/E 면), perpendicular = 가로 방향 도로
    bool needHorizDir = !sideIsX;

    for (int s = spanLo; s <= spanHi; s++)
    {
        Point2 p = sideIsX ? Point2{ s, fixed } : Point2{ fixed, s };
        auto it = laneMap.find(p);
        if (it == laneMap.end()) continue;
        if (!isRoadSurfaceTile(it->second)) continue;

        Dir8 d = it->second.dir;
        bool match = needHorizDir ? isHorizontalDir(d) : isVerticalDir(d);
        if (match) return true;
    }
    return false;
}

// 면 너머에 arterial+ 도로가 perpendicular 방향으로 빠져나가는지 검사.
// 이 검사가 통과한 면(=큰 도로가 빠져나가는 쪽) 에만 zebra 를 그린다 →
// arterial-collector 비대칭 교차로에서 collector 쪽 (작은 도로) 의 zebra 가 사라져
// 시각적 잡음이 줄어든다 (사용자 피드백 반영).
bool hasArterialContinuationOnSide(const LaneMap& laneMap, int fixed, int spanLo, int spanHi, bool sideIsX)
{
    bool needHorizDir = !sideIsX;

    for (int s = spanLo; s <= spanHi; s++)
    {
        Point2 p = sideIsX ? Point2{ s, fixed } : Point2{ fixed, s };
        auto it = laneMap.find(p);
        if (it == laneMap.end()) continue;
        if (!isRoadSurfaceTile(it->second)) continue;
        if (it->second.materialTier < 2) continue;  // arterial+ 만

        Dir8 d = it->second.dir;
        bool match = needHorizDir ? isHorizontalDir(d) : isVerticalDir(d);
        if (match) return true;
    }
    return false;
}

bool hasPerpendicularContinuation(const LaneMap& laneMap, const LaneJunction& j)
{
    int sides = 0;
    if (hasContinuationOnSide(laneMap, j.minY - 1, j.minX, j.maxX, true))  sides++; // N
    if (hasContinuationOnSide(laneMap, j.maxY + 1, j.minX, j.maxX, true))  sides++; // S
    if (hasContinuationOnSide(laneMap, j.minX - 1, j.minY, j.maxY, false)) sides++; // W
    if (hasContinuationOnSide(laneMap, j.maxX + 1, j.minY, j.maxY, false)) sides++; // E
    return sides >= 3;
}

} // anonymous namespace

export void paintCrosswalksFromLaneMap(const LaneMap& laneMap, int z)
{
    auto isRoadAt = [&](int x, int y)
    {
        auto it = laneMap.find({ x, y });
        return it != laneMap.end() && isRoadSurfaceTile(it->second);
    };

    // 한 타일의 횡단보도 색 결정.
    //   - depthOff: 면 외측으로의 깊이 (1..CROSSWALK_DEPTH_PHASE2)
    //   - crossCoord, crossBase: cross-traffic 좌표 (N/S 면은 X, W/E 면은 Y)
    //   layer = (depthOff-1) / BAR_LENGTH. layer 홀짝에 따라 cross 패리티 1tile shift
    //   → 위/아래 layer 끼리 X(혹은 Y) 가 어긋난 지그재그.
    auto paintCrosswalkTile = [&](int x, int y, int crossCoord, int crossBase, int depthOff)
    {
        if (!isRoadAt(x, y)) return;
        int layer = (depthOff - 1) / CROSSWALK_BAR_LENGTH;
        int shift = layer % 2;
        int floorId = (((crossCoord - crossBase) + shift) % 2 == 0)
                          ? itemID::whiteAsphalt : itemID::blackAsphalt;
        paintFloor(x, y, z, floorId);
    };

    auto raw = findLaneJunctions(laneMap);
    for (const auto& j : raw)
    {
        if (!hasPerpendicularContinuation(laneMap, j)) continue;

        // 면별 arterial+ 연속성 검사 — 큰 도로가 빠져나가는 면에만 zebra 그림.
        //   E-W arterial 이 N-S collector 와 만나면:
        //     W/E 면: arterial 연속 → zebra (수직 줄무늬, 차량 진행에 수직)
        //     N/S 면: collector 만 → 스킵
        //   arterial-arterial 대칭이면 4면 모두 그림.
        bool paintN = hasArterialContinuationOnSide(laneMap, j.minY - 1, j.minX, j.maxX, true);
        bool paintS = hasArterialContinuationOnSide(laneMap, j.maxY + 1, j.minX, j.maxX, true);
        bool paintW = hasArterialContinuationOnSide(laneMap, j.minX - 1, j.minY, j.maxY, false);
        bool paintE = hasArterialContinuationOnSide(laneMap, j.maxX + 1, j.minY, j.maxY, false);

        if (paintN)
        {
            for (int x = j.minX; x <= j.maxX; x++)
                for (int off = 1; off <= CROSSWALK_DEPTH_PHASE2; off++)
                    paintCrosswalkTile(x, j.minY - off, x, j.minX, off);
        }
        if (paintS)
        {
            for (int x = j.minX; x <= j.maxX; x++)
                for (int off = 1; off <= CROSSWALK_DEPTH_PHASE2; off++)
                    paintCrosswalkTile(x, j.maxY + off, x, j.minX, off);
        }
        if (paintW)
        {
            for (int y = j.minY; y <= j.maxY; y++)
                for (int off = 1; off <= CROSSWALK_DEPTH_PHASE2; off++)
                    paintCrosswalkTile(j.minX - off, y, y, j.minY, off);
        }
        if (paintE)
        {
            for (int y = j.minY; y <= j.maxY; y++)
                for (int off = 1; off <= CROSSWALK_DEPTH_PHASE2; off++)
                    paintCrosswalkTile(j.maxX + off, y, y, j.minY, off);
        }
    }
}

// ──────────────── (7) 페인트: 사이드워크 ────────────────
export void paintSidewalksFromSet(const SidewalkMask& sidewalks, int z)
{
    for (const Point2& pt : sidewalks)
        paintFloor(pt.x, pt.y, z, itemID::paver);
}

// ──────────────── (8) 페인트: 다리 난간 ────────────────
// outermost lane && onBridge && !inJunction → wireFence 벽.
//   바닥은 이미 blackAsphalt (다리 상판 시각).
export void paintBridgeRailingsFromLaneMap(const LaneMap& laneMap, int z)
{
    for (const auto& [pt, lt] : laneMap)
    {
        if (!lt.onBridge) continue;
        if (lt.inJunction) continue;
        if (!lt.isOutermost) continue;
        if (!isRoadSurfaceTile(lt)) continue;
        paintWall(pt.x, pt.y, z, itemID::wireFence);
    }
}
