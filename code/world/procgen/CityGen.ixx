export module CityGen;

import std;
import util;
import constVar;
import SectorBiome;
import BuildingTemplate;
import WorldLandmark;
import CityGen.Common;
import CityGen.PixelGrid;
import CityGen.RoadPlan;
import CityGen.Paint;
import CityGen.Lots;
import CityPlan;
import CityPlan.Builder;
import CityPlan.LaneMap;
import CityPlan.Renderer;

// ──────────────────────────────────────────────────────────
// CityGen — 도시 생성 메인 엔트리 (오케스트레이션만)
//
// 파이프라인:
//   1) PNG 픽셀 스캔 → 도시 연결 컴포넌트 + 포털/다리 추출
//   2) 도로망 그래프 구축 (Highway/Arterial/Collector 3계층)
//   3) RoadMask carve
//   4) 사이드워크 마스크
//   5) 페인트 (asphalt → 차선 → 횡단보도 → 사이드워크 → 다리난간)
//   6) 구역제 (Zoning) 계산
//   7) 블록 검출 → BSP 필지 → 가변 건물 배치
//
// 단계별 구현은 CityGen.* 서브모듈로 분리.
// ──────────────────────────────────────────────────────────

export struct GeneratedCity
{
    Point2 portalTile;
    Point2 portalPixel;
    int minTileX, minTileY;
    int maxTileX, maxTileY;
    int cityPixelCount;
    int bridgeCount;
    int buildingCount;
    int arterialCount;
};

export struct MissingPortalCity
{
    int minPixelX, minPixelY;
    int maxPixelX, maxPixelY;
    int pixelCount;
};

// 한 컴포넌트(도시 한 덩어리)에서 추출한 메타데이터
struct ComponentInfo
{
    Point2 portalPx{ 0, 0 };
    bool hasPortal = false;
    std::vector<Point2> bridges;
    int minPxX, minPxY, maxPxX, maxPxY;
};

static ComponentInfo scanComponent(const std::vector<Point2>& component, const PixelGrid& grid)
{
    ComponentInfo info;
    info.minPxX = std::numeric_limits<int>::max();
    info.minPxY = std::numeric_limits<int>::max();
    info.maxPxX = std::numeric_limits<int>::min();
    info.maxPxY = std::numeric_limits<int>::min();
    for (const Point2& p : component)
    {
        PixelType pt = grid.at(p.x, p.y);
        if (pt == PixelType::portal) { info.portalPx = p; info.hasPortal = true; }
        if (pt == PixelType::bridge) info.bridges.push_back(p);
        if (p.x < info.minPxX) info.minPxX = p.x;
        if (p.y < info.minPxY) info.minPxY = p.y;
        if (p.x > info.maxPxX) info.maxPxX = p.x;
        if (p.y > info.maxPxY) info.maxPxY = p.y;
    }
    return info;
}

// 한 컴포넌트의 블록 픽셀들을 돌며 BSP 필지에 건물을 배치한다.
static int placeBuildings(const std::vector<Point2>& component, const PixelGrid& grid,
    const RoadMask& mask, const SidewalkMask& sidewalks, const ZoneMap& zones, int sectorZ)
{
    int placed = 0;
    for (const Point2& p : component)
    {
        if (grid.at(p.x, p.y) != PixelType::city) continue;

        int tileMinX = p.x * TILE_PER_PIXEL;
        int tileMinY = p.y * TILE_PER_PIXEL;

        Zone pxZone = Zone::midtown;
        auto zit = zones.find(p);
        if (zit != zones.end()) pxZone = zit->second;

        std::mt19937_64 pxRng(hashSeed(p.x, p.y, sectorZ) ^ worldSeed);
        auto subBlocks = findSubBlocks(mask, sidewalks, tileMinX, tileMinY);

        for (const SubBlock& sb : subBlocks)
        {
            int sbW = sb.maxX - sb.minX + 1;
            int sbH = sb.maxY - sb.minY + 1;
            if (sbW < CG_LOT_MIN_DIM || sbH < CG_LOT_MIN_DIM) continue;

            Lot root{ sb.minX, sb.minY, sbW, sbH, sb.exterior };
            std::vector<Lot> lots;
            bspRecurse(root, pxRng, lots, 0);

            for (const Lot& lot : lots)
            {
                if (lot.exterior == 0) continue;
                if (lot.w < 8 || lot.h < 8) continue;

                Facade facade = pickFacade(lot, pxRng);
                int facadeLen = (facade == Facade::north || facade == Facade::south) ? lot.w : lot.h;
                int depthLen  = (facade == Facade::north || facade == Facade::south) ? lot.h : lot.w;

                bool allowMall = (sbW >= 44 && sbH >= 38);
                BuildingType type = pickBuildingType(
                    facadeLen - 2 * CG_LOT_SETBACK,
                    depthLen  - 2 * CG_LOT_SETBACK,
                    pxRng, pxZone, allowMall);

                PlacedBuilding pb = generateBuildingInLot(
                    lot.x, lot.y, lot.w, lot.h, sectorZ,
                    facade, type, pxRng, CG_LOT_SETBACK);

                if (!buildingFootprintClear(mask, sidewalks, pb)) continue;
                paintBuilding(pb);
                LandmarkRegistry::ins().registerBuilding(pb.tileX, pb.tileY, pb.z, pb.w, pb.h, pb.type);
                placed++;
            }
        }
    }
    return placed;
}

// 포털을 가진 한 컴포넌트를 도시로 빌드.
//   GeneratedCity 결과를 채워 반환.
static GeneratedCity buildCity(const std::vector<Point2>& component, const ComponentInfo& info,
    PixelGrid& grid, int sectorZ)
{
    std::unordered_set<Point2, Point2::Hash> allowed;
    allowed.reserve(component.size());
    for (const Point2& p : component) allowed.insert(p);

    // 1. 계획 (legacy A*) — Phase 3 에서 RoadGraph 기반 planner 로 대체 예정
    ArterialPlan arterialPlan = buildArterials(info.portalPx, info.bridges, allowed);
    auto highways = buildHighways(info.bridges, allowed);

    // 2. CityPlan IR 빌드 — anchors + arterial/highway edges + collector edges
    //    Phase 2: collector 격자도 IR 의 RoadEdge 로 표현 (Builder 가 처리)
    CityPlan plan = buildPlanFromLegacy(
        component, info.portalPx, info.bridges,
        arterialPlan.paths, highways, sectorZ);

    // 3. LaneMap carve — RoadGraph → 타일별 lane 정보
    //    내부적으로 tier 낮은 순서로 carve → high tier 가 자연스럽게 override.
    LaneMap laneMap;
    carveLaneMap(plan, grid, laneMap);

    // 4. Sidewalk Chebyshev 2-tile ring (legacy 동작 동일)
    SidewalkMask sidewalks;
    buildSidewalkSetFromLaneMap(laneMap, allowed, sidewalks);

    // 5. legacy RoadMask 합성 — Lots/findSubBlocks 호환용
    RoadMask mask;
    synthesizeLegacyMask(laneMap, mask);

    // 6. 페인트 (Phase 2: 모두 LaneMap 기반)
    paintAsphaltBaseFromLaneMap(laneMap, sectorZ);
    paintLaneMarkingsFromLaneMap(laneMap, sectorZ);
    paintCrosswalksFromLaneMap(laneMap, sectorZ);  // depth 3 (legacy 5 보다 좁음)
    paintSidewalksFromSet(sidewalks, sectorZ);
    paintBridgeRailingsFromLaneMap(laneMap, sectorZ);

    // 6. 구역제
    ZoneMap zones = computeZoneMap(component, info.portalPx);

    // 7. 건물
    int buildingCount = placeBuildings(component, grid, mask, sidewalks, zones, sectorZ);

    GeneratedCity gc;
    gc.portalTile = { pixelCenterTile(info.portalPx.x), pixelCenterTile(info.portalPx.y) };
    gc.portalPixel = info.portalPx;
    gc.minTileX = info.minPxX * TILE_PER_PIXEL;
    gc.minTileY = info.minPxY * TILE_PER_PIXEL;
    gc.maxTileX = (info.maxPxX + 1) * TILE_PER_PIXEL - 1;
    gc.maxTileY = (info.maxPxY + 1) * TILE_PER_PIXEL - 1;
    gc.cityPixelCount = (int)component.size();
    gc.bridgeCount = (int)info.bridges.size();
    gc.buildingCount = buildingCount;
    gc.arterialCount = (int)arterialPlan.paths.size() + (int)highways.size();
    return gc;
}

export std::vector<GeneratedCity> generateCitiesInSector(int sectorX, int sectorY, int sectorZ,
    std::vector<MissingPortalCity>& missing)
{
    std::vector<GeneratedCity> result;
    if (sectorZ != 0) return result;

    PixelGrid grid;
    grid.loadSector(sectorX, sectorY, sectorZ);

    std::unordered_set<Point2, Point2::Hash> visited;

    int sectorPxMinX = sectorX * PIXEL_PER_SECTOR;
    int sectorPxMinY = sectorY * PIXEL_PER_SECTOR;
    int sectorPxMaxX = sectorPxMinX + PIXEL_PER_SECTOR;
    int sectorPxMaxY = sectorPxMinY + PIXEL_PER_SECTOR;

    for (int py = sectorPxMinY; py < sectorPxMaxY; py++)
    {
        for (int px = sectorPxMinX; px < sectorPxMaxX; px++)
        {
            if (visited.contains({ px, py })) continue;
            if (!isCityTerritory(grid.at(px, py))) continue;

            std::vector<Point2> component = floodCity(grid, { px, py }, sectorZ, visited);
            ComponentInfo info = scanComponent(component, grid);

            if (!info.hasPortal)
            {
                missing.push_back({ info.minPxX, info.minPxY, info.maxPxX, info.maxPxY,
                                    (int)component.size() });
                continue;
            }

            result.push_back(buildCity(component, info, grid, sectorZ));
        }
    }
    return result;
}
