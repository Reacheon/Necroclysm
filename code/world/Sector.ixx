export module Sector;

import std;
import util;
import procGen;

// ════════════════════════════════════════════════════════════════════════
// Sector — 지역 절차생성 단위 (4000×4000 타일).
//   PNG 패치(20000타일)와 무관한 게임플레이 격자.
//   PNG 패치 5등분 = 80픽셀 = 4000타일 = 250청크 — 정렬 깔끔.
//
//   책임 분리:
//     - 본 모듈: 좌표 강타입 + Plan 자료구조 + 캐시 + procGenerate (블랙박스)
//     - LoadWindow: 플레이어 주변 9섹터 ensure 정책 (별도 모듈)
//
//   강타입의 의도:
//     SectorCoord는 Point3(타일)·청크좌표·패치좌표와 자동변환 안 됨 →
//     함수 호출부에서 좌표계 혼용 차단. CDDA식 OMT 양자화 회피.
// ════════════════════════════════════════════════════════════════════════

export struct SectorCoord
{
    int x = 0;
    int y = 0;
    int z = 0;

    static constexpr int TILES = 4000;   //한 변 타일 수
    static constexpr int PIXELS = 80;    //한 변 픽셀 수 (4000 / 50)

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
// 음수 floor division 일관 처리 (Patch와 동일 패턴).

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

// ── Plan ────────────────────────────────────────────────────────────────
// 한 섹터의 절차생성 산출물. 처음엔 비어 있음 — 컨텐츠 추가하며 확장.
//   향후 추가 예정:
//     std::vector<EncounterSite>      encounters;
//     std::vector<RoadPolyLine>       branchRoads;
//     std::vector<MiniSiteSpawn>      miniSites;

export struct SectorPlan
{
    SectorCoord coord;

    explicit SectorPlan(SectorCoord c) noexcept : coord(c) {}
};

// ── procGenerate ────────────────────────────────────────────────────────
// 순수 블랙박스: (sc, seed) → SectorPlan.
//   함수 진입 시점에 procGen mmap에서 80×80 픽셀(=6,400바이트)을 로컬 배열로 스냅샷.
//   이후 모든 plan 생성 알고리즘은 local만 참조 → L1 캐시 hot, mmap 페이지 폴트 1회뿐.
//   결정론(시드 동일 → plan 동일).
//
//   픽셀 좌표 변환:
//     월드 픽셀 (0,0) ←→ 월드 타일 (-1,080,000, -540,000) — PNG 좌상단 패치(-54,-27)
//     월드 픽셀 (21600, 10800) ←→ 월드 타일 (0, 0)        — 월드 중심
//
//   범위 밖 섹터 (sc.x ∉ [-270,270) 등): worldPixel이 Sea 반환 → 빈 plan.
//============================================================
export SectorPlan procGenerate(SectorCoord sc, std::uint64_t /*seed*/)
{
    constexpr int PX = SectorCoord::PIXELS;            // 80
    constexpr int TILE_BASE_X = -54 * 400 * 50;        // -1,080,000 (PATCH_X_MIN * PIXEL_PER_PATCH * TILES_PER_PIXEL)
    constexpr int TILE_BASE_Y = -27 * 400 * 50;        //   -540,000

    //섹터 좌상단 픽셀 (월드 픽셀 좌표).
    const int px0 = (sc.x * SectorCoord::TILES - TILE_BASE_X) / 50;   // = sc.x * 80 + 21600
    const int py0 = (sc.y * SectorCoord::TILES - TILE_BASE_Y) / 50;   // = sc.y * 80 + 10800

    //--- 80×80 픽셀 로컬 스냅샷 (6,400 바이트, L1 hot) ---
    std::array<std::array<procGen::Terrain, PX>, PX> local{};
    for (int dy = 0; dy < PX; ++dy)
    {
        for (int dx = 0; dx < PX; ++dx)
        {
            local[dy][dx] = procGen::worldPixel(px0 + dx, py0 + dy);
        }
    }

    //--- plan 생성 ---
    SectorPlan plan(sc);

    //TODO: local 80×80 스냅샷을 입력으로 받는 절차생성 알고리즘:
    //      - 인카운터 사이트 좌표 추출 (Land 픽셀 위에 결정론적 배치)
    //      - T1 도로 폴리라인이 이 섹터를 통과하면 분기 국도 생성
    //      - 미니 사이트 (NPC 정착지·유적·미니 던전) 배치
    //      - 모두 시드 + sc 해시로 결정론 보장

    return plan;
}

// ── Cache ───────────────────────────────────────────────────────────────
// 도시 진입 / 이동 시 lazy하게 채워짐. 결정론(seed 같으면 같은 plan).
//   evict는 LoadWindow 정책으로 호출. clear는 월드 리셋용.

export class SectorCache
{
public:
    static SectorCache& ins()
    {
        static SectorCache c;
        return c;
    }

    const SectorPlan& getOrCompute(SectorCoord sc, std::uint64_t seed)
    {
        auto it = cache_.find(sc);
        if (it != cache_.end()) return *it->second;

        auto plan = std::make_unique<SectorPlan>(procGenerate(sc, seed));
        const SectorPlan& ref = *plan;
        cache_.emplace(sc, std::move(plan));
        return ref;
    }

    void evict(SectorCoord sc)
    {
        cache_.erase(sc);
    }

    void clear()
    {
        cache_.clear();
    }

    std::size_t size() const noexcept { return cache_.size(); }

private:
    SectorCache() = default;
    SectorCache(const SectorCache&) = delete;
    SectorCache& operator=(const SectorCache&) = delete;

    std::unordered_map<SectorCoord, std::unique_ptr<SectorPlan>, SectorCoord::Hash> cache_;
};
