export module CityPlan;

import std;
import util;
import city;
import worldGen;
import constVar;
import VehiclePlan;
import ProcGenWorker;

// ════════════════════════════════════════════════════════════════════════
// CityPlan — 도시 1개의 절차생성 산출물 (골격).
//
//   SectorPlan과 한 쌍: SectorPlan은 섹터 단위 PaintCell 버퍼,
//   CityPlan은 도시 단위 타일 래스터 — buildCityPlan이 도로/블록/다리를 타일로 구움.
//
//   왜 섹터가 아니라 도시 단위인가:
//     T1 도시(베이징)는 ~120px = 2880타일 > 1920타일 섹터 → 도시가 섹터를 가로지름.
//     섹터마다 따로 생성하면 경계에서 결정이 어긋남. 도시 1개당 1번 전역 생성 →
//     procGenerate가 자기 섹터 범위만큼 잘라서 페인트(클리핑 OK).
//
//   buildCityPlan은 순수 블랙박스 (id, seed) -> CityPlan.
//   CityPlanCache가 CityId 키로 lazy 메모이즈 (miss 시 호출 스레드에서 동기 계산).
//
//   현재 buildCityPlan은 도로 세그먼트 랜덤 제거까지 구현됨 — 살아남은 세그먼트는
//   plan.segments에 저장. 블록/건물/다리/CityPlanInput은 향후 단계에서 본 모듈에 누적됨.
//
//   의존성: city (CityId), util (Point3). 사이클 없음 —
//   worldGen은 CityPlan을 import하지 않음 (Sector_procGenerate.cpp만 소비).
// ════════════════════════════════════════════════════════════════════════

// ── CityTile ────────────────────────────────────────────────────────────
// buildCityPlan이 "깔 타일"을 기록하는 단위. 실타일 좌표 + 무엇을 깔지.
//   PaintCell(Sector 모듈)을 직접 쓰면 import 사이클이 생기므로, 여기선 raw id
//   필드만 보유 → procGenerate 4단계가 PaintCell로 번역해서 블릿.
//   floor/wall/prop == 0 이면 "그 레이어는 안 건드림".
//   prop: createProp 호출 대상 — ramp 등 z 다리 시스템에 쓰임.
export struct CityTile
{
    Point3 pos;          // 실타일 좌표 (x, y, z)
    int    floor = itemID::none;
    int    wall  = itemID::none;
    int    prop  = itemID::none;
};

// ── CityBuildingPixel (debug) ───────────────────────────────────────────
// buildCityPlan 4단계가 산출한 건물의 픽셀 단위 점유 정보 (Map 오버레이 시각화용).
//   pos: 픽셀의 top-left 실타일 좌표. 픽셀 1개 = TILE_PER_PIXEL × TILE_PER_PIXEL 타일.
//   memberIndex: 같은 건물에 속한 픽셀끼리 동일 값 — 색상 해시 키. 2x2 건물이면
//   4개 픽셀이 같은 index를 공유.
export struct CityBuildingPixel
{
    Point3 pos;
    int    memberIndex = -1;
};

// ── CitySymbol (월드맵) ───────────────────────────────────────────────────
// stage 10이 건물 그룹마다 1개 push — 월드맵(Map.ixx)이 그릴 건물 청크 심볼.
//   pos: footprint 좌상단 청크의 실타일 좌표(= g.minPx 기준). w/h: footprint 청크 크기
//   (회전 적용된 그룹의 실제 점유 모양 — 2x1/1x2 분기에 사용). symbol: 건물 종류.
//   실제 스프라이트(아틀라스·인덱스·오프셋·변형)는 Map.ixx resolveSymbol이 결정.
//   Lot이 빈 스켈레톤이라 plan.tiles는 비어도, 심볼은 이 채널로 항상 표시된다.
export struct CitySymbol
{
    Point3    pos;
    int       w = 1;
    int       h = 1;
    MapSymbol symbol = MapSymbol::none;
};

// ── CityRoadCell (월드맵) ─────────────────────────────────────────────────
// stage 7까지 확정된 도로 픽셀의 openBits 래스터 — 월드맵이 도로 autotile 심볼 선택에 사용.
//   pos: 청크 좌상단 실타일 좌표. openBits: N=1,E=2,S=4,W=8. plan.segments(디버그 폴리라인)와
//   별개로, 렌더러가 4방향 비트마스크 → 도로 스프라이트(직선/코너/T/십자)를 직접 매핑한다.
//   isBridge: 강/해협을 건너는 다리 칸(stage 8). 렌더러가 일반 도로 대신 다리 전용 심볼로 그림.
export struct CityRoadCell
{
    Point3       pos;
    std::uint8_t openBits = 0;
    bool         isBridge = false;
};

// ── CityItemStack / CityMonster ──────────────────────────────────────────
// Lot이 깔 spawn (sparse). tiles와 평행 채널 — 페이로드 자료형이 달라
// (items 리스트 / entityCode) prop처럼 단일 itemID로 합쳐지지 않음.
export struct CityItemStack
{
    Point3 pos;
    std::vector<std::pair<int, int>> items;
};

export struct CityMonster
{
    Point3 pos;
    int    entityCode = 0;
};

//Lot이 깔 차량 spawn (sparse). plan은 VehicleBuilder가 만든 불변 footprint(shared_ptr).
//  Lot 로컬 좌표를 절대 Point3(anchor)로 옮긴 후 그대로 SectorPlan으로 흘러간다.
export struct CityVehicle
{
    Point3 pos;
    std::shared_ptr<const VehiclePlan> plan;
};

//Prop 내부 ItemPocket에 채울 아이템 (sparse). pos는 prop이 깔린 절대 타일 좌표.
//  CityTile.prop과 평행 채널 — 같은 pos에 prop이 있어야 인스턴스화가 매칭됨.
export struct CityPropContents
{
    Point3 pos;
    std::vector<std::pair<int, int>> items;
};

export struct CityPlan
{
    city::CityId id{};

    //  buildCityPlan이 채우는 "깔 타일 목록" — 플랜을 설정한다 = 여기에 push.
    //  procGenerate 4단계가 자기 섹터 범위만 잘라서 PaintCell에 블릿 (클리핑 OK).
    //  도시 본체 전체가 이 리스트 — buildCityPlan이 도로·블록·다리를 전부
    //  타일로 구워서 push. 지오메트리 표현 없음, 래스터(타일 목록)가 곧 산출물.
    std::vector<CityTile> tiles;

    //  살아남은 도시 내부 도로 세그먼트 — Map 오버레이 디버그 시각화용.
    //  buildCityPlan 7.5에서 다트 던지기 후 남은 segments를 그대로 이동.
    //  각 segment.verts는 항상 2점 (양 끝). 도시당 수천 개라 메모리 추가는 있지만
    //  도로 분할 알고리즘 디버깅에 시각 확인이 필수라 보관.
    std::vector<worldGen::RoadPolyLine> segments;

    //  강을 가로지르는 다리 폴리라인 — z+1 deck + ramp 시스템 (16단계 페인트).
    //  segments와 분리: 11-15단계의 평면 도로 페인트 대상이 아니어야 하고, Map에선
    //  다른 색/스타일로 그릴 수 있게 채널 분리. 각 폴리라인 verts는 양 끝점 2개.
    std::vector<worldGen::RoadPolyLine> bridges;

    //  건물 픽셀 분포 (debug) — buildCityPlan 4단계 다트던지기 산출물 시각화용.
    //  픽셀 1개 = TILE_PER_PIXEL 타일 정사각형. 같은 건물의 모든 픽셀은 동일 memberIndex
    //  보유 → Map 오버레이가 memberIndex 해시 색상으로 칠하면 건물 단위가 한 덩어리로 보임.
    std::vector<CityBuildingPixel> buildings;

    //  월드맵(Map.ixx) 청크 심볼 — stage 10이 건물 그룹별 1개 push (종류·footprint·앵커).
    std::vector<CitySymbol> symbols;

    //  월드맵 도로 autotile 래스터 — stage 7 후 openBits 픽셀을 그대로 기록.
    std::vector<CityRoadCell> roadCells;

    std::vector<CityItemStack> itemStacks;   //sparse
    std::vector<CityMonster>   monsters;     //sparse
    std::vector<CityVehicle>   vehicles;     //sparse
    std::vector<CityPropContents>   propContents;   //sparse — prop 내부 ItemPocket 채움

    CityPlan() = default;
    explicit CityPlan(city::CityId id_) noexcept : id(id_) {}

    //  SectorPlan과 동일 정책 — 향후 대형 vector 보유 대비 이동 전용.
    CityPlan(CityPlan&&) noexcept = default;
    CityPlan& operator=(CityPlan&&) noexcept = default;
    CityPlan(const CityPlan&) = delete;
    CityPlan& operator=(const CityPlan&) = delete;
};

// buildCityPlan — 도시 1개의 플랜을 절차생성하는 순수 블랙박스.
//   정의는 CityPlan_build.cpp. 보통 CityPlanCache::getOrCompute 경유로 호출 (캐시
//   dedup). 순수 함수라 직접 호출도 안전하나, 같은 도시 재계산 방지를 위해 캐시 권장.
export CityPlan buildCityPlan(city::CityId id, std::uint64_t seed);

// ── CityLayout (월드맵 경량) ──────────────────────────────────────────────
// buildCityPlan의 무거운 산출물(tiles + spawn 채널) 없이, 월드맵이 그리는 데 필요한
//   심볼 + 도로망만 담는 경량 구조. 같은 (id, seed)면 full plan의 symbols/roadCells와
//   비트 동일 (CityPlan_build.cpp 결정론 보장) → 플레이어가 안 가본 도시도 정찰지도로 표시.
export struct CityLayout
{
    std::vector<CitySymbol>   symbols;
    std::vector<CityRoadCell> roadCells;
};

// buildCityLayout — stage 9 lot 생성·stage 10 머티리얼라이즈를 건너뛴 경량 빌드.
//   buildCityPlan과 stage 1-8 + 심볼 결정을 공유 → 심볼 위치가 나중의 full 머티리얼라이즈와 일치.
export CityLayout buildCityLayout(city::CityId id, std::uint64_t seed);

// ── Cache ───────────────────────────────────────────────────────────────
// CityId 키 메모이즈 캐시. 플레이어 근처 섹터가 로드될 때 procGenerate가 lazy 조회.
//   결정론: 같은 (id, seed)면 같은 plan.
//
//   설계: getOrCompute 단일 진입점 — miss면 *호출 스레드에서 즉시* buildCityPlan
//   계산 후 캐시. 워커 위임(requestAsync) 없음 → procGenerate가 ProcGenWorker
//   단일 스레드에서 호출해도 "자기 자신이 채울 future를 기다리는" 데드락이 원천
//   불가능. 도시 플랜은 래스터라 MB 단위(T1은 수십 MB)지만 동기 계산으로 충분
//   (SectorCache의 37MB miss 경로도 lock 잡고 동기 계산 — 같은 패턴).

export class CityPlanCache
{
public:
    static CityPlanCache& ins()
    {
        static CityPlanCache c;
        return c;
    }

    //동기 조회. miss면 호출 스레드에서 buildCityPlan 즉시 계산 후 캐시.
    //  procGenerate(ProcGenWorker) / teleport(메인) 어느 쪽이 불러도 안전.
    //  반환 ref는 evict/clear 전까지 valid — unordered_map은 rehash해도 원소 노드
    //  주소가 불변이라 ref/포인터 안정성 보장.
    //  주의: miss 계산은 lock 보유 중 진행 — buildCityPlan이 무거워지면 경합 가능.
    const CityPlan& getOrCompute(city::CityId id, std::uint64_t seed)
    {
        std::lock_guard lk(mtx_);
        auto it = cache_.find(id);
        if (it == cache_.end())
        {
            it = cache_.emplace(id, buildCityPlan(id, seed)).first;
        }
        return it->second;
    }

    //비계산 조회 — 캐시에 있으면 포인터, 없으면 nullptr. miss여도 *생성 안 함*.
    //  Map 오버레이가 "이미 생성된 도시만" 그리는 용도. peek 결과 ref는
    //  evict/clear 전까지 valid (unordered_map 원소 주소 안정성 보장).
    const CityPlan* peek(city::CityId id) const
    {
        std::lock_guard lk(mtx_);
        auto it = cache_.find(id);
        if (it == cache_.end()) return nullptr;
        return &it->second;
    }

    void evict(city::CityId id)
    {
        std::lock_guard lk(mtx_);
        cache_.erase(id);
    }

    void clear()
    {
        std::lock_guard lk(mtx_);
        cache_.clear();
    }

    std::size_t size() const noexcept
    {
        std::lock_guard lk(mtx_);
        return cache_.size();
    }

private:
    CityPlanCache() = default;
    CityPlanCache(const CityPlanCache&) = delete;
    CityPlanCache& operator=(const CityPlanCache&) = delete;

    mutable std::mutex mtx_;
    std::unordered_map<city::CityId, CityPlan> cache_;
};

// ── CityLayoutCache ───────────────────────────────────────────────────────
// 월드맵 전용 경량 캐시 (CityPlanCache와 분리). 도시가 맵 화면에 들어오면 Map.ixx가
//   requestAsync로 백그라운드 layout을 요청 → 정찰지도(?건물+도로망)가 채워진다.
//
//   왜 CityPlanCache와 분리하나: procGenerate가 CityPlanCache::getOrCompute 후 plan.tiles를
//   바로 소비한다. layout-only(타일 없음) 엔트리를 거기 넣으면 도시가 인게임에 건물/도로
//   없이 깔리는 버그가 된다. 별도 캐시라 procGenerate는 이걸 절대 보지 않음.
//
//   full ↔ layout: 둘 다 같은 seed면 symbols/roadCells가 동일. 플레이어 접근 시
//   CityPlanCache에 full이 생기고 Map은 full을 우선(roadCellsFor/symbolsFor). layout 엔트리는
//   작아서(POD 벡터 2개) 그대로 둬도 무해 — eviction 불필요.
export class CityLayoutCache
{
public:
    static CityLayoutCache& ins()
    {
        static CityLayoutCache c;
        return c;
    }

    //비계산 조회 — 있으면 포인터, 없으면 nullptr. 렌더 스레드가 매 프레임 호출.
    //  반환 ref는 clear 전까지 valid (unordered_map 원소 주소 안정성). 워커는 신규 키
    //  삽입만 하므로 한 프레임 내 포인터/순회 안전.
    const CityLayout* peek(city::CityId id) const
    {
        std::lock_guard lk(mtx_);
        auto it = cache_.find(id);
        if (it == cache_.end()) return nullptr;
        return &it->second;
    }

    //layout 백그라운드 생성 요청. 이미 캐시됐거나 진행 중이면 무시(dedupe). 빌드는 워커
    //  스레드에서 *락 밖*으로 수행 → 렌더 스레드 peek이 stage 1-8 계산에 블록되지 않음.
    void requestAsync(city::CityId id, std::uint64_t seed)
    {
        {
            std::lock_guard lk(mtx_);
            if (cache_.contains(id)) return;
            if (!inFlight_.insert(static_cast<std::uint32_t>(id)).second) return;   // 이미 큐/진행 중
        }
        ProcGenWorker::ins().submit([id, seed]
        {
            CityLayout lay = buildCityLayout(id, seed);
            auto& self = CityLayoutCache::ins();
            std::lock_guard lk(self.mtx_);
            self.cache_.emplace(id, std::move(lay));
            self.inFlight_.erase(static_cast<std::uint32_t>(id));
        });
    }

    void clear()
    {
        std::lock_guard lk(mtx_);
        cache_.clear();
        inFlight_.clear();
    }

    std::size_t size() const noexcept
    {
        std::lock_guard lk(mtx_);
        return cache_.size();
    }

private:
    CityLayoutCache() = default;
    CityLayoutCache(const CityLayoutCache&) = delete;
    CityLayoutCache& operator=(const CityLayoutCache&) = delete;

    mutable std::mutex mtx_;
    std::unordered_map<city::CityId, CityLayout> cache_;
    std::unordered_set<std::uint32_t>            inFlight_;
};
