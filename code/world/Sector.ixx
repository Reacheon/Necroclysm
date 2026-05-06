export module Sector;

import std;
import util;
import constVar;
import procGen;
import ProcGenWorker;

// ════════════════════════════════════════════════════════════════════════
// Sector — 지역 절차생성 단위 (3840×3840 타일).
//   PNG 패치(19200타일)와 정렬: 패치 5등분 = 80픽셀 = 3840타일 = 240청크.
//   1픽셀 = 48타일 = 3청크 (정합) → 섹터 = 80픽셀 = 240청크.
//
//   책임 분리 (의도적 — 위반하면 프로젝트 위험):
//     - Sector_procGenerate.cpp: *단일 슈퍼함수* + 모든 sector-level 절차생성 로직.
//                                14.7M PaintCell을 사전 계산해 SectorPlan.tiles에 저장.
//     - World::createChunk: SectorPlan.tiles를 *블릿*만. 자체 결정 0.
//     - ProcGenWorker: 비동기 실행 (단일 스레드 dedicated)
//
//   왜 청크가 아니라 섹터에서 결정하는가:
//     절차생성은 *광역 스케일*에서 의미 있음 (강 곡선·도시·도로 분기 등).
//     청크(16타일)는 너무 작아 어떤 procgen 결정도 못 내림 — 단순 스트리밍·세이브 단위.
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
    static constexpr int TILES  = PIXELS * TILE_PER_PIXEL;     //3840 = 80 × 48

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
    return SectorCoord{ floorDiv(tile.x), floorDiv(tile.y), tile.z };
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
    std::uint16_t floor     = 0;
    std::uint16_t wall      = 0;
    std::uint16_t ceil      = 0;
    std::uint16_t randomVal = 0;
    std::uint8_t  flags     = 0;   //bit0 walkable, bit1 hasSnow, bit2 blocker, bit3 isWet
    std::uint8_t  _pad      = 0;
};

//flag 비트 정의.
export inline constexpr std::uint8_t TILE_FLAG_WALKABLE = 0x01;
export inline constexpr std::uint8_t TILE_FLAG_HAS_SNOW = 0x02;
export inline constexpr std::uint8_t TILE_FLAG_BLOCKER  = 0x04;
export inline constexpr std::uint8_t TILE_FLAG_IS_WET   = 0x08;

// ── SectorPlan ──────────────────────────────────────────────────────────
// 한 섹터의 *모든* 절차생성 산출물. 청크가 소비하는 단일 진리원천.
//
//   tiles: 14.7M (3840×3840) PaintCell. procGenerate가 모두 채움. ~147MB/sector.
//          [dy * SectorCoord::TILES + dx]로 인덱싱 (row-major).
//
//   향후 추가 (모두 procGenerate에서 채움):
//     std::vector<EncounterSite>  encounters;
//     std::vector<RoadPolyLine>   branchRoads;
//     std::vector<MiniSiteSpawn>  miniSites;

export struct SectorPlan
{
    SectorCoord            coord;
    std::vector<PaintCell> tiles;   //3840 × 3840 = 14,745,600개

    SectorPlan() = default;
    explicit SectorPlan(SectorCoord c) noexcept : coord(c) {}

    //이동 가능, 복사 금지 (147MB 복사 방지).
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

// ── loadNearbySectors ───────────────────────────────────────────────────
// 플레이어 위치 변경 지점(Player::setGrid 등)에서 호출되는 통합 로딩 함수.
//
//   3×3 섹터: SectorCache::requestAsync — sector procgen 비동기 큐잉
//             섹터 한 변 3840타일 마진 → 워커가 끝낼 시간 충분.
//   5×5 섹터 내 도시 layout: TODO (CityLayoutCache 추가 시)
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

    //5×5 외곽 — 도시 layout precompute (TODO: CityLayoutCache)
}
