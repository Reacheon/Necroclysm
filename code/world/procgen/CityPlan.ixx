export module CityPlan;

import std;
import util;

// ──────────────────────────────────────────────────────────
// CityPlan — 도시 생성 중간표현(IR)
//
//   설계 원칙
//     1) 계획(Plan)과 렌더(Paint)를 분리한다.
//        이 모듈은 "도시가 무엇으로 구성되는가"만 정의하며,
//        타일을 직접 칠하거나 청크를 만들지 않는다.
//
//     2) 도로는 방향 무관 polyline + RoadProfile 로 표현.
//        축 정렬(axisH/axisV) 가정을 자료형에 박지 않는다.
//
//     3) 블록은 임의 다각형(BlockPolygon).
//        "한 픽셀 = 한 블록" 동일시를 자료형 차원에서 깬다.
//
//     4) 시부야형 스크램블 교차로/광장/공원 등 비건물 land use는
//        BlockLandUse 로 명시. 건물 배치 파이프라인 밖에서 처리된다.
//
//   계층
//     CityPlan
//       ├─ boundaryPixels   : 도시 영역 (PNG flood-fill 결과, 픽셀 좌표)
//       ├─ anchors          : 포털·다리·도심·광장 등 특수 지점
//       ├─ graph            : 도로 그래프 (RoadNode, RoadEdge)
//       ├─ blocks           : 도로망이 닫아내는 블록 다각형들
//       ├─ districts        : 블록 그룹 + 성격 (downtown/residential/...)
//       └─ landUse          : 블록 단위 용도 (건물/공원/광장/주차/시민시설/공지)
//
//   Phase 0 시점에서는 자료형만 정의된다. Builder/Renderer는 후속 단계.
// ──────────────────────────────────────────────────────────

// ──────────────── ID 타입 ────────────────
// 단순 uint32 + 센티넬. 추후 strong typedef로 승격 가능.
export using NodeId     = std::uint32_t;
export using EdgeId     = std::uint32_t;
export using BlockId    = std::uint32_t;
export using DistrictId = std::uint32_t;
export using AnchorId   = std::uint32_t;
export using ProfileId  = std::uint16_t;

export constexpr std::uint32_t INVALID_ID      = ~0u;
export constexpr std::uint16_t INVALID_PROFILE = (std::uint16_t)~0u;

// ──────────────── Anchor ────────────────
// 도시의 특수 지점. 그래프 노드에 바인딩 가능.
//   portal           : 포탈 진입점. 시부야형 교차로 한가운데에 박는 게 목표.
//   bridge           : 다리가 도시로 들어오는 끝점.
//   gate             : 도시 외곽 진입로 (고속도로 IC 등).
//   scrambleJunction : 도심 스크램블 교차로 — 광장·횡단보도로 둘러싸인 큰 사거리.
//   plazaCenter      : 광장 중심부
export enum class AnchorKind : std::uint8_t
{
    portal,
    bridge,
    gate,
    scrambleJunction,
    plazaCenter,
};

export struct Anchor
{
    AnchorId    id           = INVALID_ID;
    AnchorKind  kind         = AnchorKind::portal;
    Point2      tilePos{ 0, 0 };       // 월드 타일 좌표
    Point2      sourcePixel{ 0, 0 };   // PNG 픽셀 출처 (해당되는 경우)
    NodeId      boundNode    = INVALID_ID; // 그래프 노드 바인딩 (없으면 INVALID_ID)
};

// ──────────────── 차선 / 도로 프로파일 ────────────────
// 차선 종류. 외측 → 안쪽 순서로 RoadProfile.lanes 에 나열한다.
//
//   주행/포장 종류:
//     travelLane       : 일반 주행 차선 (검정 아스팔트, 마킹 없음)
//     shoulder         : 갓길 (검정, 마킹 없음)
//     parkingLane      : 노상 주차
//     bikeLane         : 자전거 도로
//     turnLane         : 좌·우회전 전용
//     busLane          : 버스 전용
//
//   마킹 종류 (도로 위 페인팅. 폭 1타일이 기본):
//     centerlineSolid  : 중앙 노란 실선 (간선급)
//     centerlineDashed : 중앙 노란 점선 (동네 2차선)
//     medianStrip      : 중앙분리대 (width≥3 권장, 노란 실선으로 채움)
//     laneStripe       : 같은 방향 차선 분리선 (흰 점선)
//
//   인도:
//     sidewalk         : 보도 (paver). 일반적으로 프로파일 안에는 들어가지 않고
//                        Renderer 가 도로 외측 ring 으로 별도 추가한다.
export enum class LaneKind : std::uint8_t
{
    travelLane,
    shoulder,
    parkingLane,
    bikeLane,
    turnLane,
    busLane,
    centerlineSolid,
    centerlineDashed,
    medianStrip,
    laneStripe,
    sidewalk,
};

// ──────────────── LaneKind 분류 헬퍼 ────────────────
// 도로 표면 (검정 아스팔트로 깔리는 모든 종류)
export inline bool isRoadSurfaceKind(LaneKind k)
{
    return k == LaneKind::travelLane
        || k == LaneKind::shoulder
        || k == LaneKind::parkingLane
        || k == LaneKind::bikeLane
        || k == LaneKind::turnLane
        || k == LaneKind::busLane
        || k == LaneKind::centerlineSolid
        || k == LaneKind::centerlineDashed
        || k == LaneKind::medianStrip
        || k == LaneKind::laneStripe;
}

// 노란 페인트 (실선 또는 dashed)
export inline bool isYellowMarkKind(LaneKind k)
{
    return k == LaneKind::centerlineSolid
        || k == LaneKind::centerlineDashed
        || k == LaneKind::medianStrip;
}

// dashed 마킹 (페인터가 alongIdx 로 phase 결정)
export inline bool isDashedMarkKind(LaneKind k)
{
    return k == LaneKind::centerlineDashed
        || k == LaneKind::laneStripe;
}

export struct LaneSpec
{
    LaneKind kind  = LaneKind::travelLane;
    int      width = 1; // 타일
};

// 횡단보도 표현 방식.
//   none      : 없음 (소형 도로 / 막다른길)
//   standard  : 진입 leg마다 직각 zebra
//   scramble  : 시부야형 — 직각 4방향 + 대각선 2방향까지
export enum class CrosswalkStyle : std::uint8_t
{
    none,
    standard,
    scramble,
};

// 도로 단면 프로파일.
//   lanes 는 외측 한 인도 → 반대쪽 인도까지 순서대로 나열.
//   totalWidth = sum(lanes[i].width). 대칭이라면 홀수 권장 (중앙선이 정확히 1타일).
export struct RoadProfile
{
    ProfileId             id            = INVALID_PROFILE;
    std::string           name;
    int                   totalWidth    = 0;
    std::vector<LaneSpec> lanes;
    CrosswalkStyle        crosswalkStyle = CrosswalkStyle::standard;
    bool                  divided        = false; // median width≥3 인 경우
    std::uint8_t          materialTier   = 0;     // 렌더러 머티리얼 힌트 (asphalt 등급)
};

// ──────────────── 도로 그래프 ────────────────
// 교차로 종류. Planner 가 incident 엣지 수와 anchor 바인딩을 보고 분류.
export enum class JunctionKind : std::uint8_t
{
    endpoint,    // 막다른길 / 도시 경계 끝점
    bend,        // 굴절점 (분기 없음, 각도만 변함)
    tee,         // 3-way (T자)
    cross,       // 4-way (+)
    scramble,    // 시부야형 스크램블 (anchor와 결합)
    plaza,       // 광장 교차로 (anchor와 결합)
    bridgeEntry, // 다리 진입 교차로
};

export struct RoadNode
{
    NodeId               id        = INVALID_ID;
    Point2               tilePos{ 0, 0 };
    JunctionKind         kind      = JunctionKind::endpoint;
    std::vector<EdgeId>  incident; // 노드를 만지는 엣지 ID
};

// 도로 한 구간. a→b polyline (양 끝점 포함). 임의 방향 가능.
export struct RoadEdge
{
    EdgeId               id      = INVALID_ID;
    NodeId               a       = INVALID_ID;
    NodeId               b       = INVALID_ID;
    ProfileId            profile = INVALID_PROFILE;
    std::vector<Point2>  centerline; // a→b 폴리라인 (월드 타일)
};

export struct RoadGraph
{
    std::vector<RoadNode> nodes;
    std::vector<RoadEdge> edges;
};

// ──────────────── 블록 다각형 ────────────────
// 도로망이 닫아내는 임의 다각형.
//   vertices   : CCW 순서의 정점 (월드 타일)
//   borderEdges: 변에 대응하는 도로 엣지 ID (vertices.size() 와 1:1 — i번째 변은 vertices[i] → vertices[(i+1)%N])
//                 도로가 아닌 변(예: 강변, 도시 경계)은 INVALID_ID.
export struct BlockPolygon
{
    BlockId              id          = INVALID_ID;
    std::vector<Point2>  vertices;
    std::vector<EdgeId>  borderEdges;
    Point2               centroid{ 0, 0 };
    int                  approxArea  = 0; // 타일² (근사)
};

// ──────────────── 구역 (District) ────────────────
// 블록 그룹 + 성격. Planner 가 LandUse 결정시 참조.
//   downtown    : 도심. 고밀도, 사무·상업 우세, 스크램블 가능.
//   commercial  : 상업가. 가게·식당 우세, 거리형 상가.
//   residential : 주거. 주택·아파트.
//   industrial  : 공업·창고. 슈퍼블록, 주차 많음.
//   waterfront  : 수변. 산책로·보트하우스 가능.
//   greenbelt   : 녹지대. 공원 위주, 건물 밀도 낮음.
//   civicCore   : 시민중심. 시청·역·박물관 등 큰 단일 시설.
export enum class DistrictKind : std::uint8_t
{
    downtown,
    commercial,
    residential,
    industrial,
    waterfront,
    greenbelt,
    civicCore,
};

export struct District
{
    DistrictId            id    = INVALID_ID;
    DistrictKind          kind  = DistrictKind::residential;
    std::vector<BlockId>  blocks;
};

// ──────────────── Land Use ────────────────
// 블록 단위 용도. 건물 외 용도를 명시적으로 다루기 위해 도입.
//   buildings  : 일반 — BSP 분할 후 건물 배치
//   park       : 녹지 공원 — 잔디·산책로·벤치, 건물 없음
//   plaza      : 광장 (포장형) — 시부야 스크램블 결합 가능
//   parkingLot : 노외 주차장 — 아스팔트 + 주차 그리드
//   civic      : 시민·문화시설 단일 랜드마크 (시청, 역, 박물관)
//   vacant     : 공지 — 미배치 placeholder
export enum class LandUse : std::uint8_t
{
    buildings,
    park,
    plaza,
    parkingLot,
    civic,
    vacant,
};

export struct BlockLandUse
{
    BlockId block = INVALID_ID;
    LandUse use   = LandUse::buildings;
};

// ──────────────── CityPlan 최상위 ────────────────
export struct CityPlan
{
    int                          sectorZ        = 0;
    std::vector<Point2>          boundaryPixels; // PNG 픽셀 좌표 (city 분류된 것만)

    std::vector<Anchor>          anchors;
    RoadGraph                    graph;
    std::vector<BlockPolygon>    blocks;
    std::vector<District>        districts;
    std::vector<BlockLandUse>    landUse;

    // 타일 좌표계 경계 (전체를 감싸는 AABB)
    int tileMinX = 0, tileMinY = 0;
    int tileMaxX = 0, tileMaxY = 0;
};

// ──────────────── 빌더 헬퍼 (안전한 ID 할당) ────────────────
// 모든 헬퍼는 자료구조에 항목을 추가하면서 id를 자동 부여한다.
// 외부에서 직접 vector::push_back 하는 대신 이걸 쓰자 — 인덱스 일관성 보장.

export inline AnchorId addAnchor(CityPlan& plan, AnchorKind kind, Point2 tilePos, Point2 sourcePixel)
{
    Anchor a;
    a.id           = (AnchorId)plan.anchors.size();
    a.kind         = kind;
    a.tilePos      = tilePos;
    a.sourcePixel  = sourcePixel;
    a.boundNode    = INVALID_ID;
    plan.anchors.push_back(a);
    return a.id;
}

export inline NodeId addNode(RoadGraph& g, Point2 tilePos, JunctionKind kind = JunctionKind::endpoint)
{
    RoadNode n;
    n.id      = (NodeId)g.nodes.size();
    n.tilePos = tilePos;
    n.kind    = kind;
    g.nodes.push_back(std::move(n));
    return g.nodes.back().id;
}

export inline EdgeId addEdge(RoadGraph& g, NodeId a, NodeId b, ProfileId profile,
                             std::vector<Point2> centerline)
{
    RoadEdge e;
    e.id          = (EdgeId)g.edges.size();
    e.a           = a;
    e.b           = b;
    e.profile     = profile;
    e.centerline  = std::move(centerline);
    g.edges.push_back(std::move(e));

    EdgeId eid = g.edges.back().id;
    if (a != INVALID_ID && a < g.nodes.size()) g.nodes[a].incident.push_back(eid);
    if (b != INVALID_ID && b < g.nodes.size() && b != a) g.nodes[b].incident.push_back(eid);
    return eid;
}

export inline BlockId addBlock(CityPlan& plan, std::vector<Point2> vertices,
                               std::vector<EdgeId> borderEdges, Point2 centroid, int approxArea)
{
    BlockPolygon b;
    b.id           = (BlockId)plan.blocks.size();
    b.vertices     = std::move(vertices);
    b.borderEdges  = std::move(borderEdges);
    b.centroid     = centroid;
    b.approxArea   = approxArea;
    plan.blocks.push_back(std::move(b));
    return plan.blocks.back().id;
}

export inline DistrictId addDistrict(CityPlan& plan, DistrictKind kind, std::vector<BlockId> blocks)
{
    District d;
    d.id     = (DistrictId)plan.districts.size();
    d.kind   = kind;
    d.blocks = std::move(blocks);
    plan.districts.push_back(std::move(d));
    return plan.districts.back().id;
}

export inline void setBlockLandUse(CityPlan& plan, BlockId block, LandUse use)
{
    for (auto& bl : plan.landUse)
    {
        if (bl.block == block) { bl.use = use; return; }
    }
    plan.landUse.push_back({ block, use });
}

// ──────────────── 읽기 전용 조회 ────────────────
export inline const RoadNode* nodeById(const RoadGraph& g, NodeId id)
{
    if (id == INVALID_ID || id >= g.nodes.size()) return nullptr;
    return &g.nodes[id];
}

export inline const RoadEdge* edgeById(const RoadGraph& g, EdgeId id)
{
    if (id == INVALID_ID || id >= g.edges.size()) return nullptr;
    return &g.edges[id];
}

export inline const Anchor* anchorById(const CityPlan& plan, AnchorId id)
{
    if (id == INVALID_ID || id >= plan.anchors.size()) return nullptr;
    return &plan.anchors[id];
}

export inline const BlockPolygon* blockById(const CityPlan& plan, BlockId id)
{
    if (id == INVALID_ID || id >= plan.blocks.size()) return nullptr;
    return &plan.blocks[id];
}

export inline LandUse landUseOf(const CityPlan& plan, BlockId block)
{
    for (const auto& bl : plan.landUse) if (bl.block == block) return bl.use;
    return LandUse::buildings; // 기본값
}
