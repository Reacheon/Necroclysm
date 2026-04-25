export module CityGen.Lots;

import std;
import util;
import constVar;
import World;
import BuildingTemplate;
import CityGen.Common;
import CityGen.Paint;

// ──────────────────────────────────────────────────────────
// CityGen.Lots — 도시 픽셀 안에서 블록 → 필지 → 건물까지
//
//   1) 구역제 (Zoning) : 포털로부터의 거리로 core/midtown/suburb 분류
//   2) 블록 검출       : 도로+사이드워크 제외 영역의 연결성분
//   3) BSP 필지 분할   : 블록을 재귀로 쪼개 가변 lot 생성
//   4) 건물 페인트     : lot 안에 facade 방향으로 footprint 배치
// ──────────────────────────────────────────────────────────

// ──────────────── 1. 구역제 ────────────────
//   core    : d/maxD <= 0.30
//   midtown : 0.30 < d/maxD <= 0.60
//   suburb  : 0.60 < d/maxD
export using ZoneMap = std::unordered_map<Point2, Zone, Point2::Hash>;

export ZoneMap computeZoneMap(const std::vector<Point2>& component, Point2 portalPx)
{
    ZoneMap out;
    out.reserve(component.size());
    int maxD = 1;
    for (const Point2& p : component)
    {
        int d = manhattan(p, portalPx);
        if (d > maxD) maxD = d;
    }
    for (const Point2& p : component)
    {
        double r = (double)manhattan(p, portalPx) / maxD;
        Zone z;
        if (r <= 0.30)      z = Zone::core;
        else if (r <= 0.60) z = Zone::midtown;
        else                z = Zone::suburb;
        out[p] = z;
    }
    return out;
}

// ──────────────── 2. 블록 검출 ────────────────
export struct SubBlock
{
    int minX, minY, maxX, maxY;
    std::uint8_t exterior = 0;  // N=bit0, E=bit1, S=bit2, W=bit3
};

export std::vector<SubBlock> findSubBlocks(const RoadMask& mask, const SidewalkMask& sidewalks,
    int tileMinX, int tileMinY)
{
    // 픽셀 내 실제 블록 영역: 북/서 collector+사이드워크 이후 ~ 남/동 사이드워크 이전
    int blockMinX = tileMinX + CG_COLLECTOR_WIDTH + CG_SIDEWALK_WIDTH;
    int blockMinY = tileMinY + CG_COLLECTOR_WIDTH + CG_SIDEWALK_WIDTH;
    int blockMaxX = tileMinX + TILE_PER_PIXEL - 1 - CG_SIDEWALK_WIDTH;
    int blockMaxY = tileMinY + TILE_PER_PIXEL - 1 - CG_SIDEWALK_WIDTH;
    if (blockMinX > blockMaxX || blockMinY > blockMaxY) return {};

    std::vector<SubBlock> out;
    std::unordered_set<Point2, Point2::Hash> visited;

    auto isBlocked = [&](int tx, int ty) {
        auto it = mask.find({ tx, ty });
        if (it != mask.end() && it->second.tier != RoadTier::none) return true;
        if (sidewalks.contains({ tx, ty })) return true;
        return false;
    };

    for (int y = blockMinY; y <= blockMaxY; y++)
    {
        for (int x = blockMinX; x <= blockMaxX; x++)
        {
            if (visited.contains({ x, y })) continue;
            if (isBlocked(x, y)) continue;

            std::queue<Point2> q;
            q.push({ x, y }); visited.insert({ x, y });
            int mnx = x, mny = y, mxx = x, mxy = y;
            while (!q.empty())
            {
                Point2 cur = q.front(); q.pop();
                if (cur.x < mnx) mnx = cur.x;
                if (cur.x > mxx) mxx = cur.x;
                if (cur.y < mny) mny = cur.y;
                if (cur.y > mxy) mxy = cur.y;
                constexpr int dx[] = { 1,-1,0,0 }, dy[] = { 0,0,1,-1 };
                for (int k = 0; k < 4; k++)
                {
                    Point2 n{ cur.x + dx[k], cur.y + dy[k] };
                    if (n.x < blockMinX || n.x > blockMaxX || n.y < blockMinY || n.y > blockMaxY) continue;
                    if (visited.contains(n)) continue;
                    if (isBlocked(n.x, n.y)) continue;
                    visited.insert(n); q.push(n);
                }
            }

            if ((mxx - mnx + 1) < 6 || (mxy - mny + 1) < 6) continue;

            SubBlock sb{ mnx, mny, mxx, mxy, 0 };
            // exterior 판정: 블록 변의 중앙 바로 바깥이 도로/사이드워크면 해당 방향 "도로 접근 가능"
            int cmx = (mnx + mxx) / 2, cmy = (mny + mxy) / 2;
            if (isBlocked(cmx, mny - 1)) sb.exterior |= 0b0001;
            if (isBlocked(mxx + 1, cmy)) sb.exterior |= 0b0010;
            if (isBlocked(cmx, mxy + 1)) sb.exterior |= 0b0100;
            if (isBlocked(mnx - 1, cmy)) sb.exterior |= 0b1000;
            out.push_back(sb);
        }
    }
    return out;
}

// ──────────────── 3. BSP 필지 분할 ────────────────
export struct Lot
{
    int x, y, w, h;
    std::uint8_t exterior;
};

export void bspRecurse(Lot lot, std::mt19937_64& rng, std::vector<Lot>& out, int depth = 0)
{
    bool canVert = (lot.w >= 2 * CG_LOT_MIN_DIM);
    bool canHorz = (lot.h >= 2 * CG_LOT_MIN_DIM);
    bool forceSplit = (std::max(lot.w, lot.h) >= CG_LOT_SPLIT_DIM);

    bool doSplit = false;
    if (canVert || canHorz)
    {
        if (forceSplit) doSplit = true;
        else if (depth < 4 && std::uniform_int_distribution<int>(0, 2)(rng) > 0) doSplit = true;
    }
    if (!doSplit) { out.push_back(lot); return; }

    bool splitVertical;
    if (canVert && canHorz) splitVertical = (lot.w >= lot.h);
    else splitVertical = canVert;

    if (splitVertical)
    {
        int lo = CG_LOT_MIN_DIM;
        int hi = lot.w - CG_LOT_MIN_DIM;
        int pos = std::uniform_int_distribution<int>(lo, hi)(rng);
        Lot L{ lot.x, lot.y, pos, lot.h, (std::uint8_t)(lot.exterior & ~0b0010) };
        Lot R{ lot.x + pos, lot.y, lot.w - pos, lot.h, (std::uint8_t)(lot.exterior & ~0b1000) };
        bspRecurse(L, rng, out, depth + 1);
        bspRecurse(R, rng, out, depth + 1);
    }
    else
    {
        int lo = CG_LOT_MIN_DIM;
        int hi = lot.h - CG_LOT_MIN_DIM;
        int pos = std::uniform_int_distribution<int>(lo, hi)(rng);
        Lot T{ lot.x, lot.y, lot.w, pos, (std::uint8_t)(lot.exterior & ~0b0100) };
        Lot B{ lot.x, lot.y + pos, lot.w, lot.h - pos, (std::uint8_t)(lot.exterior & ~0b0001) };
        bspRecurse(T, rng, out, depth + 1);
        bspRecurse(B, rng, out, depth + 1);
    }
}

export Facade pickFacade(const Lot& lot, std::mt19937_64& rng)
{
    int w[4] = { 0,0,0,0 };
    if (lot.exterior & 0b0001) w[0] = lot.w;
    if (lot.exterior & 0b0010) w[1] = lot.h;
    if (lot.exterior & 0b0100) w[2] = lot.w;
    if (lot.exterior & 0b1000) w[3] = lot.h;
    int total = w[0] + w[1] + w[2] + w[3];
    if (total <= 0) return Facade::north;
    int r = std::uniform_int_distribution<int>(0, total - 1)(rng);
    for (int i = 0; i < 4; i++) { if (r < w[i]) return (Facade)i; r -= w[i]; }
    return Facade::north;
}

// ──────────────── 4. 건물 페인트 + 검증 ────────────────
export bool buildingFootprintClear(const RoadMask& mask, const SidewalkMask& sidewalks,
    const PlacedBuilding& pb)
{
    for (int dy = 0; dy < pb.h; dy++)
    {
        for (int dx = 0; dx < pb.w; dx++)
        {
            Point2 t{ pb.tileX + dx, pb.tileY + dy };
            auto it = mask.find(t);
            if (it != mask.end() && it->second.tier != RoadTier::none) return false;
            if (sidewalks.contains(t)) return false;
        }
    }
    return true;
}

export void paintBuilding(const PlacedBuilding& pb)
{
    for (int dy = 0; dy < pb.h; dy++)
    {
        for (int dx = 0; dx < pb.w; dx++)
        {
            bool onEdge = (dx == 0 || dx == pb.w - 1 || dy == 0 || dy == pb.h - 1);
            if (!onEdge) continue;
            int tx = pb.tileX + dx;
            int ty = pb.tileY + dy;
            bool isDoor = (dx == pb.doorLocalX && dy == pb.doorLocalY);
            if (isDoor)
            {
                ensureChunk(tx, ty, pb.z);
                DestroyWall(tx, ty, pb.z);
            }
            else
            {
                paintWall(tx, ty, pb.z, itemID::concreteWall);
            }
        }
    }
}
