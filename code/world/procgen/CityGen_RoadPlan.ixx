export module CityGen.RoadPlan;

import std;
import util;
import constVar;
import SectorBiome;
import CityGen.Common;

// ──────────────────────────────────────────────────────────
// CityGen.RoadPlan — 도로망 계획·carve·교차로 분석
//
//   - 픽셀 레벨 A*  → 직선 위주 경로
//   - Highway / Arterial / Collector 3계층 carve
//   - 사이드워크 마스크 (도로 외측 링)
//   - 교차로 검출 (axisH && axisV 클러스터 + L-회전 필터)
// ──────────────────────────────────────────────────────────

// ──────────────── 픽셀 레벨 A* ────────────────
// 4연결 + 회전 페널티로 직선 경로 유도. allowed 픽셀 집합 안에서만 이동.
export std::vector<Point2> pixelAStar(Point2 start, Point2 goal,
    const std::unordered_set<Point2, Point2::Hash>& allowed, int turnPenalty = 30)
{
    if (start == goal) return { start };
    constexpr int STEP_COST = 10;
    struct NodeKey { Point2 pos; int dir; bool operator==(const NodeKey&) const = default; };
    struct NodeHash {
        size_t operator()(const NodeKey& k) const noexcept {
            return Point2::Hash{}(k.pos) ^ (std::hash<int>{}(k.dir) << 1);
        }
    };
    constexpr int dx[] = { 1,-1,0,0 }, dy[] = { 0,0,1,-1 };
    std::unordered_map<NodeKey, int, NodeHash> gScore;
    std::unordered_map<NodeKey, NodeKey, NodeHash> cameFrom;
    auto h = [&](Point2 p) { return STEP_COST * (std::abs(p.x - goal.x) + std::abs(p.y - goal.y)); };
    using QEntry = std::tuple<int, int, int, int>;
    std::priority_queue<QEntry, std::vector<QEntry>, std::greater<QEntry>> open;
    NodeKey startKey{ start, -1 };
    gScore[startKey] = 0;
    open.push({ h(start), start.x, start.y, -1 });
    while (!open.empty())
    {
        auto [f, px, py, dir] = open.top(); open.pop();
        Point2 pos{ px, py };
        NodeKey curKey{ pos, dir };
        if (pos == goal)
        {
            std::vector<Point2> path;
            NodeKey k = curKey;
            while (true) { path.push_back(k.pos); auto it = cameFrom.find(k); if (it == cameFrom.end()) break; k = it->second; }
            std::reverse(path.begin(), path.end());
            return path;
        }
        int curG = gScore[curKey];
        for (int nd = 0; nd < 4; nd++)
        {
            Point2 np{ pos.x + dx[nd], pos.y + dy[nd] };
            if (!allowed.contains(np)) continue;
            int stepCost = STEP_COST + ((dir != -1 && nd != dir) ? turnPenalty : 0);
            int tentG = curG + stepCost;
            NodeKey nk{ np, nd };
            auto git = gScore.find(nk);
            if (git == gScore.end() || tentG < git->second)
            {
                gScore[nk] = tentG;
                cameFrom[nk] = curKey;
                open.push({ tentG + h(np), np.x, np.y, nd });
            }
        }
    }
    return {};
}

// ──────────────── 도로 carve ────────────────
// 축정렬 strip 한 줄을 마스크에 새김. 대각 입력은 최단 거리로 계단 carve.
export void carveStrip(RoadMask& mask, int fromX, int fromY, int toX, int toY, RoadTier tier, int width)
{
    int halfW = width / 2;
    int sx = (toX > fromX) ? 1 : (toX < fromX ? -1 : 0);
    int sy = (toY > fromY) ? 1 : (toY < fromY ? -1 : 0);

    if (sx == 0 && sy == 0)
    {
        for (int dw = -halfW; dw <= halfW; dw++)
        {
            std::int8_t off = (std::int8_t)std::abs(dw);
            markMaskTile(mask, fromX + dw, fromY, tier, true, off);
            markMaskTile(mask, fromX, fromY + dw, tier, false, off);
        }
        return;
    }

    bool isHorizontal = (sx != 0);
    int x = fromX, y = fromY;
    while (true)
    {
        for (int dw = -halfW; dw <= halfW; dw++)
        {
            int tx = isHorizontal ? x : x + dw;
            int ty = isHorizontal ? y + dw : y;
            std::int8_t off = (std::int8_t)std::abs(dw);
            markMaskTile(mask, tx, ty, tier, isHorizontal, off);
        }
        if (x == toX && y == toY) break;
        x += sx; y += sy;
    }
}

export void carvePixelPath(RoadMask& mask, const std::vector<Point2>& pxPath, RoadTier tier, int width)
{
    for (size_t i = 1; i < pxPath.size(); i++)
    {
        int x0 = pixelCenterTile(pxPath[i - 1].x);
        int y0 = pixelCenterTile(pxPath[i - 1].y);
        int x1 = pixelCenterTile(pxPath[i].x);
        int y1 = pixelCenterTile(pxPath[i].y);
        carveStrip(mask, x0, y0, x1, y1, tier, width);
    }
}

// Collector 격자 — 각 픽셀의 북/서 모서리에 도로
export void carveCollectorGrid(RoadMask& mask, const std::vector<Point2>& component)
{
    int halfW = CG_COLLECTOR_WIDTH / 2;
    for (const Point2& p : component)
    {
        int tileMinX = p.x * TILE_PER_PIXEL;
        int tileMinY = p.y * TILE_PER_PIXEL;
        int tileMaxX = tileMinX + TILE_PER_PIXEL - 1;
        int tileMaxY = tileMinY + TILE_PER_PIXEL - 1;

        int nCY = tileMinY + halfW;
        carveStrip(mask, tileMinX, nCY, tileMaxX, nCY, RoadTier::collector, CG_COLLECTOR_WIDTH);

        int wCX = tileMinX + halfW;
        carveStrip(mask, wCX, tileMinY, wCX, tileMaxY, RoadTier::collector, CG_COLLECTOR_WIDTH);
    }
}

// ──────────────── Arterial / Highway 계획 ────────────────
export struct ArterialPlan
{
    std::vector<std::vector<Point2>> paths;
};

export ArterialPlan buildArterials(Point2 portalPx, const std::vector<Point2>& bridges,
    const std::unordered_set<Point2, Point2::Hash>& allowed)
{
    ArterialPlan plan;

    for (const Point2& b : bridges)
    {
        auto path = pixelAStar(portalPx, b, allowed, 30);
        if (!path.empty()) plan.paths.push_back(std::move(path));
    }

    constexpr double ALPHA = 1.4;
    for (size_t i = 0; i < bridges.size(); i++)
    {
        for (size_t j = i + 1; j < bridges.size(); j++)
        {
            int direct = manhattan(bridges[i], bridges[j]);
            int viaPortal = manhattan(bridges[i], portalPx) + manhattan(portalPx, bridges[j]);
            if (direct <= (int)(ALPHA * viaPortal / 2.0))
            {
                auto path = pixelAStar(bridges[i], bridges[j], allowed, 30);
                if (!path.empty()) plan.paths.push_back(std::move(path));
            }
        }
    }

    if (bridges.empty())
    {
        Point2 best = portalPx;
        int bestD = 0;
        for (const Point2& p : allowed)
        {
            int d = manhattan(p, portalPx);
            if (d > bestD) { bestD = d; best = p; }
        }
        if (bestD > 0)
        {
            auto path = pixelAStar(portalPx, best, allowed, 30);
            if (!path.empty()) plan.paths.push_back(std::move(path));
        }
    }
    return plan;
}

// 가장 먼 다리쌍을 관통 고속도로로
export std::vector<std::vector<Point2>> buildHighways(const std::vector<Point2>& bridges,
    const std::unordered_set<Point2, Point2::Hash>& allowed)
{
    std::vector<std::vector<Point2>> out;
    if (bridges.size() < 2) return out;

    size_t bi = 0, bj = 1;
    int bestD = -1;
    for (size_t i = 0; i < bridges.size(); i++)
        for (size_t j = i + 1; j < bridges.size(); j++)
        {
            int d = manhattan(bridges[i], bridges[j]);
            if (d > bestD) { bestD = d; bi = i; bj = j; }
        }
    auto path = pixelAStar(bridges[bi], bridges[bj], allowed, 50);
    if (!path.empty()) out.push_back(std::move(path));
    return out;
}

// ──────────────── 사이드워크 마스크 ────────────────
// 도로 주변 CG_SIDEWALK_WIDTH 타일 링 (도로 제외, 도시 픽셀 내부).
// Chebyshev 거리 사용 → 코너 포함해 균일한 링.
export void buildSidewalkMask(const RoadMask& roads,
    const std::unordered_set<Point2, Point2::Hash>& allowedPixels,
    SidewalkMask& sidewalks)
{
    for (const auto& [pt, cell] : roads)
    {
        if (cell.tier == RoadTier::none) continue;
        for (int dy = -CG_SIDEWALK_WIDTH; dy <= CG_SIDEWALK_WIDTH; dy++)
        {
            for (int dx = -CG_SIDEWALK_WIDTH; dx <= CG_SIDEWALK_WIDTH; dx++)
            {
                if (dx == 0 && dy == 0) continue;
                Point2 np{ pt.x + dx, pt.y + dy };
                if (roads.contains(np)) continue;
                Point2 npx{ pixelFromTile(np.x), pixelFromTile(np.y) };
                if (!allowedPixels.contains(npx)) continue;
                sidewalks.insert(np);
            }
        }
    }
}

// ──────────────── 주요 교차로 검출 ────────────────
// arterial 이상 tier의 (axisH && axisV) 연결성분.
//
// L-회전 필터:
//   arterial이 가로→세로로 꺾이는 코너도 axisH && axisV 영역을 만들지만
//   실제 "교차로"는 아님. 진짜 교차로는 bbox 외부 4면 중 3면 이상에서
//   "수직 방향" 도로가 연속되어야 함:
//     N/S 면 → axisV 도로가 연속 (세로 도로가 위/아래로 빠져나감)
//     W/E 면 → axisH 도로가 연속 (가로 도로가 좌/우로 빠져나감)
//   2면 이하만 만족하면 단순 회전이거나 dead-end → 스킵
export struct IntersectionBBox
{
    int minX, minY, maxX, maxY;
    RoadTier tier;
};

// 한 면 위에서 "수직 방향" 도로가 한 칸이라도 연속되는지 검사.
//   sideAxis=true → 면이 X 스팬(N/S면), 점검 좌표는 (s, fixed). axisV 필요.
//   sideAxis=false → 면이 Y 스팬(W/E면), 점검 좌표는 (fixed, s). axisH 필요.
static bool hasContinuationOnSide(const RoadMask& mask, int fixed, int spanLo, int spanHi, bool sideIsX)
{
    for (int s = spanLo; s <= spanHi; s++)
    {
        Point2 p = sideIsX ? Point2{ s, fixed } : Point2{ fixed, s };
        auto it = mask.find(p);
        if (it == mask.end() || it->second.tier == RoadTier::none) continue;
        bool needAxisH = !sideIsX;  // W/E면은 가로 도로가 빠져나감
        if (needAxisH ? it->second.axisH : it->second.axisV) return true;
    }
    return false;
}

static bool hasPerpendicularContinuation(const RoadMask& mask, const IntersectionBBox& ib)
{
    int sides = 0;
    if (hasContinuationOnSide(mask, ib.minY - 1, ib.minX, ib.maxX, true))  sides++;  // N
    if (hasContinuationOnSide(mask, ib.maxY + 1, ib.minX, ib.maxX, true))  sides++;  // S
    if (hasContinuationOnSide(mask, ib.minX - 1, ib.minY, ib.maxY, false)) sides++;  // W
    if (hasContinuationOnSide(mask, ib.maxX + 1, ib.minY, ib.maxY, false)) sides++;  // E
    return sides >= 3;
}

export std::vector<IntersectionBBox> findMajorIntersections(const RoadMask& mask)
{
    std::unordered_set<Point2, Point2::Hash> visited;
    std::vector<IntersectionBBox> raw;
    for (const auto& [pt, cell] : mask)
    {
        if (!(cell.axisH && cell.axisV)) continue;
        if ((int)cell.tier < (int)RoadTier::arterial) continue;
        if (visited.contains(pt)) continue;

        std::queue<Point2> q;
        q.push(pt); visited.insert(pt);
        int mnx = pt.x, mny = pt.y, mxx = pt.x, mxy = pt.y;
        RoadTier tier = cell.tier;
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
                auto it = mask.find(n);
                if (it == mask.end()) continue;
                if (!(it->second.axisH && it->second.axisV)) continue;
                if ((int)it->second.tier < (int)RoadTier::arterial) continue;
                visited.insert(n); q.push(n);
                if ((int)it->second.tier > (int)tier) tier = it->second.tier;
            }
        }
        raw.push_back({ mnx, mny, mxx, mxy, tier });
    }

    std::vector<IntersectionBBox> out;
    out.reserve(raw.size());
    for (const auto& ib : raw)
        if (hasPerpendicularContinuation(mask, ib))
            out.push_back(ib);
    return out;
}
