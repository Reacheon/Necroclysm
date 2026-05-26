export module Sector;

import std;
import util;
import constVar;
import worldGrid;
import ProcGenWorker;
import worldGen;
import city;

// ════════════════════════════════════════════════════════════════════════
// Sector — 지역 절차생성 단위 (1920×1920 타일).
//   PNG 패치(9600타일)와 정렬: 패치 5등분 = 80픽셀 = 1920타일 = 80청크.
//   1픽셀 = 24타일 = 1청크 (1:1 정합) → 섹터 = 80픽셀 = 80청크.
//
//   책임 분리 (의도적 — 위반하면 프로젝트 위험):
//     - Sector_procGenerate.cpp: *단일 슈퍼함수* + 모든 sector-level 절차생성 로직.
//                                3.7M PaintCell을 사전 계산해 SectorPlan.tiles에 저장.
//     - World::createChunk: SectorPlan.tiles를 *블릿*만. 자체 결정 0.
//     - ProcGenWorker: 비동기 실행 (단일 스레드 dedicated)
//
//   왜 청크가 아니라 섹터에서 결정하는가:
//     절차생성은 *광역 스케일*에서 의미 있음 (강 곡선·도시·도로 분기 등).
//     청크(24타일)는 너무 작아 어떤 procgen 결정도 못 내림 — 단순 스트리밍·세이브 단위
//     겸 건물 양자화 그리드(24×24 최소 footprint와 정합).
//     모든 procgen 변경은 Sector_procGenerate.cpp 한 곳만 수정 → 청크 코드 안전.
//
//   강타입의 의도:
//     SectorCoord는 Point3(타일)·청크좌표·패치좌표와 자동변환 안 됨 →
//     함수 호출부에서 좌표계 혼용 차단.
// ════════════════════════════════════════════════════════════════════════

export struct SectorCoord
{
    int x = 0;
    int y = 0;
    int z = 0;

    static constexpr int PIXELS = 80;                          //한 변 픽셀 수 (패치 400px ÷ 5)
    static constexpr int TILES  = PIXELS * TILE_PER_PIXEL;     //1920 = 80 × 24 (= 80청크)

    SectorCoord() = default;
    constexpr SectorCoord(int x_, int y_, int z_) noexcept : x(x_), y(y_), z(z_) {}

    friend constexpr bool operator==(SectorCoord, SectorCoord) noexcept = default;

    struct Hash
    {
        std::size_t operator()(SectorCoord s) const noexcept
        {
            std::size_t h = static_cast<std::size_t>(s.x + (1 << 20)) * 0x9E3779B97F4A7C15ULL;
            h ^= static_cast<std::size_t>(s.y + (1 << 20)) * 0xBF58476D1CE4E5B9ULL;
            h ^= static_cast<std::size_t>(s.z + 16)        * 0x94D049BB133111EBULL;
            return h;
        }
    };
};

// ── 좌표 변환 ────────────────────────────────────────────────────────────
// 음수 floor division 일관 처리.

export SectorCoord sectorFromTile(Point3 tile) noexcept
{
    auto floorDiv = [](int v) noexcept
    {
        return (v >= 0) ? (v / SectorCoord::TILES)
                        : ((v - (SectorCoord::TILES - 1)) / SectorCoord::TILES);
    };
    // X축은 시암 wrap. tile.x가 render-space(음수/W 초과)일 수 있으니
    // 섹터 인덱스 단계에서 정규화 — SectorCache 키가 [0, W/1920) 범위 유지.
    const int rawSx = floorDiv(tile.x);
    const int sx    = worldWrap::wrapSectorX(rawSx, SectorCoord::TILES);
    return SectorCoord{ sx, floorDiv(tile.y), tile.z };
}

export Point3 sectorOriginTile(SectorCoord sc) noexcept
{
    return Point3{ sc.x * SectorCoord::TILES, sc.y * SectorCoord::TILES, sc.z };
}

// ── PaintCell ───────────────────────────────────────────────────────────
// 타일 1개의 *최종 결정값* — 청크가 그대로 TileData 필드에 복사.
//   procGenerate가 모든 결정을 내리고 채움. 청크는 블릿만.
//
//   현재는 정적 필드만 (gas·HP 등 동적 상태는 미포함). 향후 procgen이 결정해야 할
//   필드가 더 생기면 여기에 추가 — 청크 인터페이스는 그대로.

export struct PaintCell
{
    int           floor     = itemID::none;
    int           wall      = itemID::none;
    std::uint16_t randomVal = 0;
    std::uint8_t  flags     = 0;   //bit0 walkable, bit1 hasSnow, bit2 blocker, bit3 isWet
    std::uint8_t  _pad      = 0;
};

//flag 비트 정의.
export inline constexpr std::uint8_t TILE_FLAG_WALKABLE = 0x01;
export inline constexpr std::uint8_t TILE_FLAG_HAS_SNOW = 0x02;
export inline constexpr std::uint8_t TILE_FLAG_BLOCKER  = 0x04;
export inline constexpr std::uint8_t TILE_FLAG_IS_WET   = 0x08;

// ── SectorProp / SectorSkyTile ──────────────────────────────────────────
// 희소(sparse) 채널 — sector 평면 3.7M PaintCell에 들어가지 않는 데이터.
//   props: 어느 z층에서든 createProp 호출 대상. ramp 등.
//   skyTiles: sector의 본 z층(sc.z)과 다른 z의 floor/wall (다리 deck z+1 등).
//             3.7M 평면을 z층마다 따로 깔면 메모리 폭발이라 sparse로 보관.
//
//   호출 측(World::createChunk): sc.z=0 SectorPlan 단일 조회로 본 z층은 dense
//   tiles, 그 외 z는 skyTiles 필터링해 적용. props는 모든 z를 대상으로 필터링.

export struct SectorProp
{
    Point3 pos;
    int    itemId = itemID::none;
};

export struct SectorSkyTile
{
    Point3       pos;           // z는 sc.z와 다른 값
    int          floor = itemID::none;
    int          wall  = itemID::none;
    std::uint8_t flags = 0;     //TILE_FLAG_* 비트 (walkable 등)
};

// ── SectorPlan ──────────────────────────────────────────────────────────
// 한 섹터의 *모든* 절차생성 산출물. 청크가 소비하는 단일 진리원천.
//
//   tiles: 3.7M (1920×1920) PaintCell. procGenerate가 모두 채움. ~37MB/sector.
//          [dy * SectorCoord::TILES + dx]로 인덱싱 (row-major).
//   props: 어느 z층의 prop 좌표/itemId든 모두 담긴 sparse 리스트.
//   skyTiles: sc.z와 다른 z의 floor/wall sparse 리스트 (다리 deck 등).
//
//   향후 추가 (모두 procGenerate에서 채움):
//     std::vector<EncounterSite>  encounters;
//     std::vector<RoadPolyLine>   branchRoads;
//     std::vector<MiniSiteSpawn>  miniSites;

export struct SectorPlan
{
    SectorCoord                  coord;
    std::vector<PaintCell>       tiles;     //1920 × 1920 = 3,686,400개 (본 z층)
    std::vector<SectorProp>      props;     //sparse, 모든 z
    std::vector<SectorSkyTile>   skyTiles;  //sparse, sc.z 외 z만

    SectorPlan() = default;
    explicit SectorPlan(SectorCoord c) noexcept : coord(c) {}

    //이동 가능, 복사 금지 (37MB 복사 방지).
    SectorPlan(SectorPlan&&) noexcept = default;
    SectorPlan& operator=(SectorPlan&&) noexcept = default;
    SectorPlan(const SectorPlan&) = delete;
    SectorPlan& operator=(const SectorPlan&) = delete;
};

// ── procGenerate ────────────────────────────────────────────────────────
// 단일 슈퍼함수 — 본 모듈 인터페이스에 *선언만*, 정의는 Sector_procGenerate.cpp.
//   sector-level 절차생성 로직 *전부*가 여기에 누적된다.
//
//   순수 블랙박스: (sc, seed) → SectorPlan.
//   ProcGenWorker 백그라운드 스레드에서 호출 가능 — World 참조 X, mmap만 read-only.

export SectorPlan procGenerate(SectorCoord sc, std::uint64_t seed);

// ── Cache ───────────────────────────────────────────────────────────────
// 플레이어 이동·텔레포트로 lazy하게 채워짐. 결정론(seed 같으면 같은 plan).

export class SectorCache
{
public:
    static SectorCache& ins()
    {
        static SectorCache c;
        return c;
    }

    //비동기 ensure. 캐시 hit / 진행 중이면 no-op. 미스만 워커에 큐잉.
    void requestAsync(SectorCoord sc, std::uint64_t seed)
    {
        std::lock_guard lk(mtx_);
        if (cache_.contains(sc)) return;

        auto promise = std::make_shared<std::promise<SectorPlan>>();
        cache_.emplace(sc, promise->get_future().share());

        ProcGenWorker::ins().submit([sc, seed, promise]() {
            try {
                promise->set_value(procGenerate(sc, seed));
            } catch (...) {
                try { promise->set_exception(std::current_exception()); } catch (...) {}
            }
        });
    }

    //동기 조회. 미스면 메인에서 즉시 계산. 비동기 진행 중이면 future.get()이 wait.
    //  반환 ref는 cache_가 sc를 evict하지 않는 한 valid (shared_future가 shared state 보유).
    const SectorPlan& getOrCompute(SectorCoord sc, std::uint64_t seed)
    {
        std::shared_future<SectorPlan> fut;
        {
            std::lock_guard lk(mtx_);
            auto it = cache_.find(sc);
            if (it != cache_.end())
            {
                fut = it->second;
            }
            else
            {
                //미스 — 메인에서 즉시 계산 후 ready future로 캐시 채움.
                std::promise<SectorPlan> promise;
                fut = promise.get_future().share();
                cache_.emplace(sc, fut);
                promise.set_value(procGenerate(sc, seed));
            }
        }
        return fut.get();
    }

    void evict(SectorCoord sc)
    {
        std::lock_guard lk(mtx_);
        cache_.erase(sc);
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
    SectorCache() = default;
    SectorCache(const SectorCache&) = delete;
    SectorCache& operator=(const SectorCache&) = delete;

    mutable std::mutex mtx_;
    std::unordered_map<SectorCoord, std::shared_future<SectorPlan>, SectorCoord::Hash> cache_;
};

// ── citiesInRangeOf ─────────────────────────────────────────────────────
// 주어진 타일 위치 기준 5×5 섹터 범위 안에 중심이 있는 도시 id 수집 (X 시암 wrap 처리).
//
//   T1 도시(베이징)는 ~120px = 2880타일 > 1920타일 섹터 → 도시가 섹터를 가로지름.
//   procGenerate 4단계가 자기 섹터 중심 기준으로 호출 — 도시 footprint(≤~1섹터)가
//   이 섹터에 닿으면 중심은 반드시 5×5 범위 안. 과다 조회분은 per-tile 클립이 거름.
//
//   worldGen::activeCities가 nullptr면 (월드젠 전 startArea) 빈 리스트.
//   CityId = activeCities 인덱스. 현재 선형 스캔(~4400개) — 향후 CityIndex로 교체 가능.

export std::vector<city::CityId> citiesInRangeOf(Point3 centerTile)
{
    std::vector<city::CityId> result;
    if (worldGen::activeCities == nullptr) return result;

    const SectorCoord cur = sectorFromTile(centerTile);
    const auto& cities = *worldGen::activeCities;
    for (std::size_t i = 0; i < cities.size(); ++i)
    {
        const Point3 c = cities[i].center;
        if (c.z != centerTile.z) continue;

        //X는 시암 wrap — 섹터 원점 타일 간 부호 거리를 섹터 폭으로 나눠 섹터 델타 산출.
        const SectorCoord cs = sectorFromTile(c);
        const int dSecX = worldWrap::signedDeltaTileX(
            cur.x * SectorCoord::TILES, cs.x * SectorCoord::TILES) / SectorCoord::TILES;
        const int dSecY = cs.y - cur.y;
        if (std::abs(dSecX) <= 2 && std::abs(dSecY) <= 2)
        {
            result.push_back(static_cast<city::CityId>(i));
        }
    }
    return result;
}

// ── loadNearbySectors ───────────────────────────────────────────────────
// 플레이어 위치 변경 지점(Player::setGrid 등)에서 호출되는 통합 로딩 함수.
//
//   3×3 섹터: SectorCache::requestAsync — sector procgen 비동기 큐잉
//             섹터 한 변 1920타일 마진 → 워커가 끝낼 시간 충분.
//
//   도시 CityPlan은 별도 프리워밍 안 함 — procGenerate 4단계가 섹터 생성 중
//   citiesInRangeOf로 찾아 CityPlanCache::getOrCompute로 lazy 생성하므로,
//   3×3 섹터 ensure만으로 근처 도시 플랜이 자동 커버됨.
//
//   idempotent — 매 호출 캐시 히트가 정상.

export void loadNearbySectors(Point3 playerTile, std::uint64_t worldSeed)
{
    SectorCoord cur = sectorFromTile(playerTile);

    //3×3 sector procgen 비동기 ensure
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            SectorCache::ins().requestAsync(
                SectorCoord{ cur.x + dx, cur.y + dy, cur.z },
                worldSeed);
        }
    }
}
