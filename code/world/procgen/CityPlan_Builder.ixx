export module CityPlan.Builder;

import std;
import util;
import constVar;
import SectorBiome;
import CityPlan;
import CityPlan.RoadProfile;

// ──────────────────────────────────────────────────────────
// CityPlan.Builder — legacy planner 출력을 CityPlan IR 로 캡처하는 어댑터
//
//   Phase 2 시점:
//     - portal / bridge → Anchor
//     - arterial / highway 픽셀 path → RoadEdge (centerline 타일)
//     - **collector 격자도 IR 의 RoadEdge 로 표현** (Phase 2 추가)
//       각 도시 픽셀의 N/W 모서리에 짧은 edge 를 1개씩 (legacy carveCollectorGrid 와 동일 위치).
//       centerline 은 픽셀 가로/세로 길이의 직선 (50타일 + halfW offset).
//
//   Phase 2 미포함:
//     - Block / District / LandUse — Phase 3.
//
//   불변식:
//     이 모듈로 만든 plan.graph 를 LaneMap 으로 carve 하면 legacy 와
//     1타일 단위로 동일한 도로 footprint (그리고 stripe 위치) 가 나와야 함.
// ──────────────────────────────────────────────────────────

// 픽셀 path → 타일 centerline polyline.
//   pixelCenterTile() 로 픽셀당 1 점씩.
static std::vector<Point2> pixelPathToTileCenterline(const std::vector<Point2>& pxPath)
{
    std::vector<Point2> centerline;
    centerline.reserve(pxPath.size());
    for (const Point2& p : pxPath)
        centerline.push_back({ pixelCenterTile(p.x), pixelCenterTile(p.y) });
    return centerline;
}

// 컴포넌트 픽셀 bbox 를 타일 단위 bbox 로.
static void fillTileBoundsFromComponent(CityPlan& plan, const std::vector<Point2>& component)
{
    if (component.empty()) return;
    int mnPx = component.front().x, mxPx = component.front().x;
    int mnPy = component.front().y, mxPy = component.front().y;
    for (const Point2& p : component)
    {
        if (p.x < mnPx) mnPx = p.x;
        if (p.x > mxPx) mxPx = p.x;
        if (p.y < mnPy) mnPy = p.y;
        if (p.y > mxPy) mxPy = p.y;
    }
    plan.tileMinX = mnPx * TILE_PER_PIXEL;
    plan.tileMinY = mnPy * TILE_PER_PIXEL;
    plan.tileMaxX = (mxPx + 1) * TILE_PER_PIXEL - 1;
    plan.tileMaxY = (mxPy + 1) * TILE_PER_PIXEL - 1;
}

// ──────────────── 메인 빌더 ────────────────
// Inputs:
//   component        : flood-fill 결과 도시 픽셀
//   portalPx         : 포털 픽셀 (PNG)
//   bridges          : 다리 픽셀들
//   arterialPaths    : buildArterials() 결과 — 픽셀 좌표 polyline 묶음
//   highwayPaths     : buildHighways()  결과
//   sectorZ          : 섹터 z (지표 0)
//
// 반환: 채워진 CityPlan. graph 에는 arterial/highway edge 만,
//       anchors 에는 portal + bridges 만 포함.
export CityPlan buildPlanFromLegacy(
    const std::vector<Point2>& component,
    Point2 portalPx,
    const std::vector<Point2>& bridges,
    const std::vector<std::vector<Point2>>& arterialPaths,
    const std::vector<std::vector<Point2>>& highwayPaths,
    int sectorZ)
{
    const DefaultProfileIds& ids = defaultProfileIds();

    CityPlan plan;
    plan.sectorZ = sectorZ;
    plan.boundaryPixels = component;
    fillTileBoundsFromComponent(plan, component);

    // Anchors: portal + bridges
    Point2 portalTile{ pixelCenterTile(portalPx.x), pixelCenterTile(portalPx.y) };
    AnchorId portalAnchor = addAnchor(plan, AnchorKind::portal, portalTile, portalPx);

    for (const Point2& b : bridges)
    {
        Point2 bTile{ pixelCenterTile(b.x), pixelCenterTile(b.y) };
        addAnchor(plan, AnchorKind::bridge, bTile, b);
    }

    // 노드 dedup — 같은 타일 좌표는 한 노드로
    //   arterial path 들이 portal 에서 출발하는 등 끝점 공유가 흔하다.
    //   미덮으면 그래프가 끊긴 컴포넌트로 쪼개져 향후 분석이 망가짐.
    std::unordered_map<Point2, NodeId, Point2::Hash> nodeAtTile;
    auto getOrAddNode = [&](Point2 tilePos) -> NodeId
    {
        auto it = nodeAtTile.find(tilePos);
        if (it != nodeAtTile.end()) return it->second;
        NodeId id = addNode(plan.graph, tilePos);
        nodeAtTile[tilePos] = id;
        return id;
    };

    auto addPathAsEdge = [&](const std::vector<Point2>& pxPath, ProfileId profile)
    {
        if (pxPath.size() < 2) return;
        std::vector<Point2> centerline = pixelPathToTileCenterline(pxPath);
        NodeId a = getOrAddNode(centerline.front());
        NodeId b = getOrAddNode(centerline.back());
        addEdge(plan.graph, a, b, profile, std::move(centerline));
    };

    for (const auto& path : arterialPaths) addPathAsEdge(path, ids.arterial4Lane);
    for (const auto& path : highwayPaths)  addPathAsEdge(path, ids.boulevard6Lane);

    // ──────── Collector 격자 edge 추가 (Phase 2) ────────
    // 각 도시 픽셀 (50×50 타일) 의 N/W 모서리에 짧은 edge 1개씩.
    //   horizontal (N): centerline = (tx0, ty0+halfW) → (tx0+49, ty0+halfW)
    //   vertical   (W): centerline = (tx0+halfW, ty0) → (tx0+halfW, ty0+49)
    // legacy carveCollectorGrid 와 동일한 위치/길이.
    //
    // 노드 dedup 으로 인접 픽셀 collector 가 자동 연결되어 그래프 연속성 확보.
    // 하지만 픽셀 [0..49] 와 [50..99] 의 끝점 좌표는 (49) vs (50) 으로 다르기 때문에
    // 인접 픽셀의 collector 끝점은 1타일 갭. Phase 3 에서 정리.
    {
        const RoadProfile* collProf = RoadProfileCatalog::ins().get(ids.neighborhood2Lane);
        if (collProf)
        {
            const int halfW = collProf->totalWidth / 2;
            for (const Point2& p : component)
            {
                int tx0 = p.x * TILE_PER_PIXEL;
                int ty0 = p.y * TILE_PER_PIXEL;
                int tx1 = tx0 + TILE_PER_PIXEL - 1;
                int ty1 = ty0 + TILE_PER_PIXEL - 1;

                // North 모서리 (horizontal collector)
                {
                    Point2 a{ tx0, ty0 + halfW };
                    Point2 b{ tx1, ty0 + halfW };
                    NodeId na = getOrAddNode(a);
                    NodeId nb = getOrAddNode(b);
                    addEdge(plan.graph, na, nb, ids.neighborhood2Lane, { a, b });
                }
                // West 모서리 (vertical collector)
                {
                    Point2 a{ tx0 + halfW, ty0 };
                    Point2 b{ tx0 + halfW, ty1 };
                    NodeId na = getOrAddNode(a);
                    NodeId nb = getOrAddNode(b);
                    addEdge(plan.graph, na, nb, ids.neighborhood2Lane, { a, b });
                }
            }
        }
    }

    // Anchor → 그래프 노드 바인딩. 동일 타일 좌표에 노드가 있으면 연결.
    auto bindAnchorToNode = [&](Anchor& a)
    {
        auto it = nodeAtTile.find(a.tilePos);
        if (it != nodeAtTile.end()) a.boundNode = it->second;
    };
    for (Anchor& a : plan.anchors) bindAnchorToNode(a);
    (void)portalAnchor; // portalAnchor는 위 루프에서 함께 처리됨

    return plan;
}
