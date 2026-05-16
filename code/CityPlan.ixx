export module CityPlan;

import std;
import util;
import city;
import worldGen;

// ════════════════════════════════════════════════════════════════════════
// CityPlan — 도시 1개의 절차생성 산출물 (골격).
//
//   SectorPlan과 한 쌍: SectorPlan은 섹터 단위 PaintCell 버퍼,
//   CityPlan은 도시 단위 타일 래스터 — buildCityPlan이 도로/블록/다리를 타일로 구움.
//
//   왜 섹터가 아니라 도시 단위인가:
//     T1 도시(베이징)는 ~120px = 5760타일 > 3840타일 섹터 → 도시가 섹터를 가로지름.
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
//   floor/wall == 0 이면 "그 레이어는 안 건드림".
export struct CityTile
{
    Point3        pos;          // 실타일 좌표 (x, y, z)
    std::uint16_t floor = 0;
    std::uint16_t wall  = 0;
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

// ── Cache ───────────────────────────────────────────────────────────────
// CityId 키 메모이즈 캐시. 플레이어 근처 섹터가 로드될 때 procGenerate가 lazy 조회.
//   결정론: 같은 (id, seed)면 같은 plan.
//
//   설계: getOrCompute 단일 진입점 — miss면 *호출 스레드에서 즉시* buildCityPlan
//   계산 후 캐시. 워커 위임(requestAsync) 없음 → procGenerate가 ProcGenWorker
//   단일 스레드에서 호출해도 "자기 자신이 채울 future를 기다리는" 데드락이 원천
//   불가능. 도시 플랜은 래스터라 MB 단위(T1은 수십 MB)지만 동기 계산으로 충분
//   (SectorCache의 147MB miss 경로도 lock 잡고 동기 계산 — 같은 패턴).

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
