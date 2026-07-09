module worldGen;

import std;
import util;

using namespace worldGrid;  // Terrain, worldPixel 등 unqualified 접근

// ════════════════════════════════════════════════════════════════════════
// placeSites — 2티어 국지 도로망 성장 + 교외 인카운터 사이트 배치 (월드 1회 부트의 4단계).
//
//   1티어(도시간 폴리라인)만 있던 구현은 사이트마다 전용 막다른 가지를 역산했는데,
//   "도로가 시설을 위해 존재"하는 역인과라 짧은 스텁이 반복되는 절차티가 남았다.
//   여기서는 인과를 현실 방향으로 뒤집는다:
//     [B] 2티어 도로가 1티어 변·도시 경계에서 자기 이유로 먼저 성장 —
//         · 방사: 도시 경계에서 밖으로 뻗는 교외 지방도 (도시 가장자리 타일 프리펜드로
//           CityPlan 진입 등록과 정합 — 1티어 cityEdgeTileForRoad 미러)
//         · 피더: 오지로 뻗는 막다른 지방도 (주축 단조 진출 + 측면 jog 랜덤워크)
//         · 링크: 멀리 걸어간 뒤 *1티어*에 닿으면 그 자리서 접속 → 본선↔본선 연결로.
//           2티어끼리 접속은 불허 — 본선 옆 마이크로 루프 클러터의 주범
//         · 서브가지: 피더 위에서 한 번 더 성장 (깊이 2 고정)
//         · 뿌리 간격 강제 + 지역 밀도 노이즈(컷오프): 빗살 발아 방지, 도로 촘촘한
//           근교와 2티어가 아예 없는 오지의 대비 (균일 밀도 = 절차티)
//     [C] 사이트는 2티어 도로 *변*에 수직 스퍼(2~4px)로 얹힘 — 시설이 도로의 끝점이
//         아니라 길가에 들어선다. 다트 일부는 가지 말단 구간에 몰아 "시설까지 내려고
//         만든 길" 패턴 제조. 대형 사이트(공항/군부대/교도소/원전)만 1티어 직결
//         전용 가지(직선/L/Z 오프램프)를 유지 — 현실에서도 고속도로 직결 시설.
//     [D] 정산: 성장은 투기적 — 사이트도 존치 자식도 못 얻은 막다른 가지는 철거,
//         존치 가지도 마지막 수요 지점 뒤 꼬리는 트림. 남는 데드엔드는 전부 목적지로
//         끝난다 (링크=연결로는 그 자체로 존치).
//   accept마다 폴리라인(minor=true)을 roads에 직접 append — 기존 소비처(월드맵
//   오토타일/숲 clearing/Sector 페인터)가 자동 소비.
//
//   실행 전제: transitionToMmap *이후* 호출 (지형은 worldGrid::worldPixel,
//        절차 산맥은 isMountainChunk — 둘 다 mmap 필요). isHighwayChunk/isForestChunk는
//        호출 금지 — activePolyLines/activeSites 포인터 기준 캐시가 이 시점엔 미설정
//        또는 재생성 시 stale. 도로 침범 검사는 로컬 마스크(core+1링)로 자체 래스터.
//
//   픽셀 좌표(1px=1청크=24타일)는 알고리즘 내부 전용, 반환/append는 실타일 좌표.
//   폴리라인 verts·도시 center가 전부 [0,W) 픽셀 유래라 wrap 없는 raw 뺄셈 허용
//   (placeCities 동일 관례 — 시암=태평양). 범위 밖 픽셀은 worldPixel이 Sea를 반환해
//   지형 검사에서 자동 reject되므로 별도 경계 검사 불필요.
//
//   헬퍼 분리 안 함 (딥 모듈): placeCities와 동일하게 전 로직 인라인.
// ════════════════════════════════════════════════════════════════════════

namespace worldGen
{
    std::vector<SiteNode> placeSites(std::uint64_t seed, const std::vector<CityNode>& cities, std::vector<RoadPolyLine>& roads, SiteSink onSite, RoadSink onRoad)
    {
        const std::int64_t tStart = getNanoTimer();

        //══════════════════════════════════════════════════════════════════
        // 파라미터 — 모든 튜닝 포인트 집중
        //══════════════════════════════════════════════════════════════════

        //[A] 1티어 앵커
        constexpr int ANCHOR_SPACING_PX  = 6;   //도로 위 앵커 간격(픽셀≈km) — 촘촘할수록 발아 후보 밀집
        constexpr int ANCHOR_END_SKIP_PX = 16;  //폴리라인 양끝 스킵 — 도시 진입 strut(kStrutLen) 구간 발아 금지

        //[B] 2티어 성장
        constexpr int    T2_SEGS_MIN   = 3;    //가지당 세그 수 — 주축/측면 교대, 주축부터
        constexpr int    T2_SEGS_MAX   = 7;
        constexpr int    T2_OUT_MIN    = 12;   //주축(본선 수직 진출) 세그 길이(픽셀) — 길게, 오지 관통이 목적
        constexpr int    T2_OUT_MAX    = 28;
        constexpr int    T2_JOG_MIN    = 2;    //측면 jog 세그 길이(픽셀) — 주축 대비 짧게(본선 평행 클러터 방지)
        constexpr int    T2_JOG_MAX    = 6;
        constexpr int    T2_MIN_LEN    = 14;   //절단된 피더가 이 미만이면 폐기 — 토막 스텁 방지
        constexpr int    T2_LINK_MIN   = 18;   //이 워크 길이 미만의 1티어 접속은 폐기 — 본선 옆 마이크로 루프 방지
        constexpr int    T2_LINK_EUCL  = 10;   //접속점이 뿌리에서 이 직선거리 미만이면 폐기 — 헤어핀 루프 방지
        constexpr int    T2_ROOT_DIST  = 14;   //가지 뿌리간 최소 간격(픽셀) — 본선에서 빗살처럼 돋는 것 방지
        constexpr double T2_SPROUT_T1  = 0.50; //1티어 앵커 발아율 상한 (×밀도로 감쇠, 뿌리 간격이 상한 밀도 결정)
        constexpr double T2_SPROUT_SUB = 0.30; //피더 위 서브가지 발아율 상한
        constexpr double CITY_RADIAL_PROB = 0.75; //도시 방사 가지 방향(4카디널)당 시도 확률 — 밀도 노이즈 미적용(교외는 도시가 만든다)
        constexpr int    DENS_CELL_PX  = 96;   //밀도 노이즈 셀 한 변(픽셀) — 지방 단위 대비 스케일
        constexpr double DENS_CUTOFF   = 0.30; //노이즈 보간값 이 미만 지역은 밀도 0 — 2티어가 아예 없는 오지 지방
        constexpr int    SUB_SPACING   = 12;   //피더 위 서브가지 앵커 간격(픽셀)
        constexpr int    SUB_END_SKIP  = 5;    //피더 양끝 서브가지 금지 구간(픽셀)

        //[C] 사이트
        constexpr int SITE_SPUR_MIN    = 2;   //도로변 진입 스퍼 길이(픽셀) — 2 미만은 footprint가 도로 1링과 겹쳐 reject됨
        constexpr int SITE_SPUR_MAX    = 4;
        constexpr double SITE_TERM_BIAS = 0.35; //roadside 다트 중 가지 말단 1/4 구간 추첨 비율 — "시설까지 내려고 만든 길"

        //[D] 정산
        constexpr int TRIM_MARGIN_PX   = 0;   //마지막 수요 지점 뒤 남길 꼬리(픽셀) — 0=정확히 종결.
                                              //  1 이상이면 잘린 꼬리 방향으로 nub가 튀어 철거 흔적이 보임
        constexpr int BRANCH_OUT_MIN   = 10;  //trunk 대형 사이트 전용 가지 첫 진출 세그 길이 최소(픽셀)
        constexpr int BRANCH_OUT_MAX   = 24;  //trunk 가지 첫 진출 세그 길이 최대(픽셀)
        constexpr int BRANCH_SEG_MIN   = 8;   //trunk 후속 세그(측면 jog·재진출) 길이 최소(픽셀)
        constexpr int BRANCH_SEG_MAX   = 18;  //trunk 후속 세그 길이 최대(픽셀)
        constexpr int MIN_SITE_DIST_PX = 10;  //사이트간 최소 간격(픽셀)
        constexpr int MAX_ATTEMPTS_MULT= 100; //다트 시도 한도 (targetTotal * MULT)
        constexpr int MINE_MTN_SCAN_R  = 2;   //광산의 산 인접 요구 스캔 반경(footprint 밖 링)

        //사이트 타입 테이블 — weight: 다트 룰렛 비중, target: 전세계 목표 수(소진 시 제외),
        //  minCityDistPx: 도시 footprint bbox까지 최소 거리(픽셀), needMountain: 광산 전용,
        //  trunk: 1티어 직결 전용 가지(대형 — 현실에서도 고속도로 직결). false면 2티어 도로변.
        struct SiteTypeDef { MapSymbol symbol; int w; int h; int weight; int target; int minCityDistPx; bool needMountain; bool trunk; };
        //  소형 생활시설의 minCityDist는 낮게(3~6) — 도시 방사 가지 위 근교 배치 허용.
        static constexpr SiteTypeDef SITE_TABLE[] = {
            { MapSymbol::shop,         1, 1, 20, 2400,  3, false, false },
            { MapSymbol::energyBank,   1, 1, 14, 1600,  3, false, false },
            { MapSymbol::warpGate,     1, 1,  7,  800,  6, false, false },
            { MapSymbol::lookoutTower, 1, 1, 10, 1200,  5, false, false },
            { MapSymbol::warehouse,    2, 2,  8,  900,  6, false, false },
            { MapSymbol::mine,         1, 1,  7,  800, 10, true , false },
            { MapSymbol::solarPlant,   2, 2,  5,  600, 10, false, false },
            { MapSymbol::researchLab,  2, 2,  4,  500, 14, false, false },
            { MapSymbol::nuclearPlant, 2, 2,  3,  300, 18, false, true  },
            { MapSymbol::prison,       3, 3,  3,  300, 16, false, true  },
            { MapSymbol::militaryBase, 3, 3,  3,  300, 20, false, true  },
            { MapSymbol::airport,      3, 3,  2,  240, 12, false, true  },
        };
        constexpr int N_TYPES = static_cast<int>(std::size(SITE_TABLE));

        prt(L"[worldGen] placeSites start (seed=%llu, roads=%llu)\n", static_cast<std::uint64_t>(seed), static_cast<std::uint64_t>(roads.size()));

        std::vector<SiteNode> sites;
        if (roads.empty()) return sites;

        auto floorDiv = [](int a, int b) noexcept {
            return (a >= 0) ? (a / b) : -(((-a) + b - 1) / b);
        };
        auto maskKey = [](int px, int py) noexcept -> std::uint64_t {
            return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(worldWrap::wrapPixelX(px))) << 32)
                 |  static_cast<std::uint64_t>(static_cast<std::uint32_t>(py));
        };

        //══════════════════════════════════════════════════════════════════
        // [A] 도로 래스터 마스크 + 1티어 앵커 — 한 패스.
        //   각 폴리라인을 픽셀 4-연결 워크(isHighwayChunk와 동일 규약)로 걸으며
        //   ① core(중심 셀)와 mask(+1링 dilation)에 stamp — core는 링크 접속(junction)
        //      판정, mask는 침범/근접 검사용.
        //   ② 걸은 셀을 순서대로 모아 두었다가 양끝 스킵 + 등간격 추출로 앵커화.
        //   앵커가 정확히 도로 셀 위에 놓여 가지 접속점의 오토타일 T자가 보장된다.
        //══════════════════════════════════════════════════════════════════

        struct Anchor { int px; int py; int z; int sdx; int sdy; };   //sdx/sdy = 세그먼트 방향(±) — 수직 진출용
        std::unordered_set<std::uint64_t> roadCore;   //1티어 중심 셀 — 링크 접속(junction) 표적. 2티어는 안 들어감
        std::unordered_set<std::uint64_t> roadMask;   //전 도로 중심+1링 — 침범/근접 판정
        std::vector<Anchor> anchors;

        auto stampDilated = [&](int px, int py) {
            for (int oy = -1; oy <= 1; ++oy)
            for (int ox = -1; ox <= 1; ++ox)
                roadMask.insert(maskKey(px + ox, py + oy));
        };
        auto stampRoad = [&](int px, int py) {
            roadCore.insert(maskKey(px, py));
            stampDilated(px, py);
        };

        std::vector<Anchor> walked;   //폴리라인 1개의 걸은 셀 (재사용 버퍼)
        for (const RoadPolyLine& poly : roads)
        {
            walked.clear();
            for (std::size_t i = 1; i < poly.verts.size(); ++i)
            {
                const Point3& a = poly.verts[i - 1];
                const Point3& b = poly.verts[i];
                const int ax = floorDiv(a.x - TILE_BASE_X, TILE_PER_PIXEL);
                const int ay = floorDiv(a.y - TILE_BASE_Y, TILE_PER_PIXEL);
                const int bx = floorDiv(b.x - TILE_BASE_X, TILE_PER_PIXEL);
                const int by = floorDiv(b.y - TILE_BASE_Y, TILE_PER_PIXEL);
                const int dx = std::abs(bx - ax), dy = std::abs(by - ay);
                const int sx = (ax < bx) ? 1 : -1, sy = (ay < by) ? 1 : -1;
                const int moves = dx + dy;
                int x = ax, y = ay, err = dx - dy;
                stampRoad(x, y);
                walked.push_back(Anchor{ x, y, a.z, (dx >= dy) ? sx : 0, (dx >= dy) ? 0 : sy });
                for (int m = 0; m < moves; ++m)
                {
                    if (err > 0) { x += sx; err -= dy * 2; }
                    else         { y += sy; err += dx * 2; }
                    stampRoad(x, y);
                    walked.push_back(Anchor{ x, y, a.z, (dx >= dy) ? sx : 0, (dx >= dy) ? 0 : sy });
                }
            }
            //양끝 스킵 + 등간격 앵커 추출 (걸은 셀 1개 ≈ 1픽셀 호길이 근사).
            const int n = static_cast<int>(walked.size());
            for (int i = ANCHOR_END_SKIP_PX; i < n - ANCHOR_END_SKIP_PX; i += ANCHOR_SPACING_PX)
                anchors.push_back(walked[i]);
        }
        if (anchors.empty()) return sites;

        //══════════════════════════════════════════════════════════════════
        // 지형 술어 + 밀도 노이즈 + rng — [B]/[C] 공용.
        //══════════════════════════════════════════════════════════════════

        //사이트에 허용되는 지형 — 육지 + 기후 바이옴(극지 제외). 물/도시/위성 산 금지.
        auto terrainOk = [](Terrain t) noexcept -> bool {
            switch (t)
            {
            case Terrain::Land: case Terrain::Tundra: case Terrain::Subarctic:
            case Terrain::Monsoon: case Terrain::InsularRainforest:
            case Terrain::Desert: case Terrain::ContinentalRainforest:
                return true;
            default:
                return false;   // Sea/River/Lake/City*/Mountain/Polar
            }
        };
        //2티어/가지 경로에 허용되는 지형 — 물/도시만 금지(다리 없음). 산은 터널 렌더로 관통 허용.
        auto pathOk = [](Terrain t) noexcept -> bool {
            switch (t)
            {
            case Terrain::Sea: case Terrain::River: case Terrain::Lake:
            case Terrain::CityZone: case Terrain::CityCenter:
            case Terrain::CityRiver: case Terrain::CitySea:
                return false;
            default:
                return true;
            }
        };

        //지역 밀도 — DENS_CELL_PX 격자 value noise를 bilinear 보간. CUTOFF 미만은 0으로
        //  잘라 2티어가 아예 없는 지방을 만들고, 잔여 구간은 제곱으로 대비 강조(고밀도 소수).
        //  발아율에 곱해져 도로망의 지방별 표정을 만든다.
        //  셀 해시는 wrap된 픽셀 기준이라 시암(±W/2)에서 보간이 끊기지만 거긴 태평양.
        auto densCell = [seed](int cx, int cy) noexcept -> double {
            std::uint64_t h = seed ^ 0xD05EBEA70AD5ULL;   // 숲/산과 다른 salt
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) * 0xBF58476D1CE4E5B9ULL;
            h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
            h ^= static_cast<std::uint64_t>(static_cast<std::uint32_t>(cy)) * 0x94D049BB133111EBULL;
            h ^= h >> 31;
            return static_cast<double>(h & 0xff) / 255.0;
        };
        auto densityAt = [&](int px, int py) noexcept -> double {
            const int wx = worldWrap::wrapPixelX(px);
            const int cx = floorDiv(wx, DENS_CELL_PX);
            const int cy = floorDiv(py, DENS_CELL_PX);
            const double fx = static_cast<double>(wx - cx * DENS_CELL_PX) / DENS_CELL_PX;
            const double fy = static_cast<double>(py - cy * DENS_CELL_PX) / DENS_CELL_PX;
            const double v = std::lerp(std::lerp(densCell(cx, cy),     densCell(cx + 1, cy),     fx),
                                       std::lerp(densCell(cx, cy + 1), densCell(cx + 1, cy + 1), fx), fy);
            const double t = (v - DENS_CUTOFF) / (1.0 - DENS_CUTOFF);
            return (t > 0.0) ? t * t : 0.0;
        };

        std::mt19937_64 rng(seed ^ 0x517E5EEDULL);
        std::uniform_real_distribution<double> u01{ 0.0, 1.0 };

        //══════════════════════════════════════════════════════════════════
        // [B] 2티어 성장 — 앵커에서 본선 수직으로 진출해 주축/측면 jog를 번갈아 걷는
        //   카디널 랜덤워크. 주축 좌표가 단조라 자기교차가 구조적으로 불가능. jog 방향은
        //   가지당 고정 드리프트(가끔 반전) — 매 jog 랜덤이면 지그재그 절차티.
        //   · 1티어 core에 닿으면 그 자리서 접속 종료 → 링크(본선↔본선 연결로, 루프는
        //     이런 대구조에서만 의미가 있다). 2티어끼리 접속은 불허 — 본선 corridor 안에
        //     사다리꼴 마이크로 루프가 양산되는 클러터의 주범이라 mask 절단으로만 처리.
        //   · core 1링(mask)을 스치면 직진 look-ahead ≤2로 core 도달 시 관통 접속,
        //     아니면(평행 스침) 진입 직전에서 절단
        //   · 물/도시에 막히면 그 직전에서 절단 → 피더(막다른 지방도)
        //   · 뿌리 간격 T2_ROOT_DIST 강제 — 본선에서 빗살처럼 촘촘히 돋는 것 방지.
        //   반환: 0=거부, 1=피더, 2=링크. accept 시 폴리라인 append + mask stamp
        //   + 가지 대장(branches)에 셀·계보 기록, subOut 지정 시 서브가지 앵커 추출.
        //══════════════════════════════════════════════════════════════════

        //2티어 가지 대장 — 정산([D])을 위해 가지별 셀·계보·수요를 기록.
        struct T2Ref { int branchId; int ord; };   //가지 내 셀 참조 (ord = cells 인덱스)
        struct T2Branch
        {
            std::size_t         roadIdx;          //roads 내 폴리라인 인덱스
            Anchor              root;             //발아 앵커 (1티어 or 부모 가지 위)
            std::vector<Anchor> cells;            //커밋 셀 순서 보존
            int                 parent   = -1;    //서브가지의 부모 branchId (-1 = 1티어/도시 발아)
            int                 rootOrd  = 0;     //부모 cells 내 뿌리 위치 (parent >= 0일 때)
            bool                linked   = false; //1티어 접속 여부 — 연결로는 정산 면제
            int                 lastUsed = -1;    //사이트/존치 자식이 쓰는 마지막 셀 ord ([C]/[D]가 갱신)
            bool                cityRoot = false; //도시 방사 가지 — verts 맨 앞에 도시 가장자리 타일 프리펜드
            Point3              cityVert{ 0, 0, 0 };   //프리펜드된 타일 (트림 rebuild 시 재프리펜드)
        };
        std::vector<T2Branch> branches;
        int t2TotalPx = 0;

        SpatialHash rootHash(PixelCostGrid::W, PixelCostGrid::H, 16);
        std::vector<std::pair<int, int>> rootPx;   //accept된 가지 뿌리 픽셀 (rootHash 인덱스 대상)

        auto growBranch = [&](const Anchor& an, int side, int parentId, int parentOrd, const Point3* cityVertPtr, std::vector<T2Ref>* subOut) -> int
        {
            //뿌리 간격 — 기존 가지 뿌리가 T2_ROOT_DIST 안이면 발아 안 함.
            bool nearRoot = false;
            rootHash.forEachInRadius(an.px, an.py, T2_ROOT_DIST, [&](int ri)
            {
                if (nearRoot) return;
                const auto [px, py] = rootPx[static_cast<std::size_t>(ri)];
                const int ddx = px - an.px, ddy = py - an.py;
                if (ddx * ddx + ddy * ddy < T2_ROOT_DIST * T2_ROOT_DIST) nearRoot = true;
            });
            if (nearRoot) return 0;

            const int pdx = -an.sdy * side, pdy = an.sdx * side;   //주축 — 본선 수직 진출(카디널)
            const int nSegs = std::uniform_int_distribution<int>{ T2_SEGS_MIN, T2_SEGS_MAX }(rng);
            int latSign = std::uniform_int_distribution<int>{ 0, 1 }(rng) ? 1 : -1;   //드리프트 방향 — 가지당 고정

            std::vector<Anchor> cells;                  //커밋된 비코어 셀 (사이트 앵커 후보)
            std::vector<std::pair<int, int>> bends;     //세그 경계 픽셀 — verts 용
            bends.emplace_back(an.px, an.py);
            int x = an.px, y = an.py, total = 0;
            bool linked = false, stop = false;

            //링크 확정 조건 — 충분히 걸어왔고(T2_LINK_MIN) 뿌리에서 직선으로도 충분히
            //  떨어져야(T2_LINK_EUCL — 헤어핀 루프 방지) 접속. 아니면 가지 전체 폐기.
            auto linkOk = [&](int jx, int jy, int walkLen) noexcept -> bool {
                const int rdx = jx - an.px, rdy = jy - an.py;
                return walkLen >= T2_LINK_MIN && rdx * rdx + rdy * rdy >= T2_LINK_EUCL * T2_LINK_EUCL;
            };

            for (int s = 0; s < nSegs && !stop; ++s)
            {
                int dx, dy, len;
                if ((s & 1) == 0) { dx = pdx; dy = pdy; len = std::uniform_int_distribution<int>{ T2_OUT_MIN, T2_OUT_MAX }(rng); }
                else
                {
                    if (std::uniform_int_distribution<int>{ 0, 4 }(rng) == 0) latSign = -latSign;   //가끔 드리프트 반전
                    dx = an.sdx * latSign; dy = an.sdy * latSign;
                    len = std::uniform_int_distribution<int>{ T2_JOG_MIN, T2_JOG_MAX }(rng);
                }

                int committed = 0;
                for (int t = 1; t <= len; ++t)
                {
                    const int nx = x + dx, ny = y + dy;
                    if (!pathOk(worldPixel(worldWrap::wrapPixelX(nx), ny))) { stop = true; break; }   //물/도시 — 절단

                    const bool inCore = roadCore.count(maskKey(nx, ny)) != 0;
                    if (total < 2)
                    {
                        //출발 직후 — 부모 1링 면제. core 재밟기(부모 코너·인접 도로)만 거부.
                        if (inCore) return 0;
                    }
                    else if (inCore)
                    {
                        if (!linkOk(nx, ny, total + 1)) return 0;
                        x = nx; y = ny; ++total; ++committed;
                        linked = true; stop = true; break;
                    }
                    else if (roadMask.count(maskKey(nx, ny)) != 0)
                    {
                        //도로 1링 진입 — 직진 look-ahead ≤2로 1티어 core 도달하면 관통 접속,
                        //  아니면(2티어 링·평행 스침) 진입 직전에서 절단.
                        int hit = 0;
                        for (int k = 1; k <= 2; ++k)
                        {
                            const int qx = nx + dx * k, qy = ny + dy * k;
                            if (!pathOk(worldPixel(worldWrap::wrapPixelX(qx), qy))) break;
                            if (roadCore.count(maskKey(qx, qy)) != 0) { hit = k; break; }
                            if (roadMask.count(maskKey(qx, qy)) == 0) break;
                        }
                        if (hit > 0 && linkOk(nx + dx * hit, ny + dy * hit, total + 1 + hit))
                        {
                            for (int k = 0; k < hit; ++k)   //링 통과 셀 커밋 (junction core 셀은 앵커 제외)
                                cells.push_back(Anchor{ nx + dx * k, ny + dy * k, an.z, dx, dy });
                            x = nx + dx * hit; y = ny + dy * hit;
                            total += 1 + hit; committed += 1 + hit;
                            linked = true;
                        }
                        stop = true; break;
                    }

                    x = nx; y = ny; ++total; ++committed;
                    cells.push_back(Anchor{ x, y, an.z, dx, dy });
                }
                if (committed > 0) bends.emplace_back(x, y);
            }

            if (!linked && total < T2_MIN_LEN) return 0;   //짧은 절단 피더 — 스텁 폐기
            if (bends.size() < 2) return 0;

            //accept — 폴리라인(minor) append + stamp + 앵커 등록.
            //  도시 방사면 도시 가장자리 타일을 맨 앞에 — CityPlan 진입 스캔이 이 끝점을
            //  멤버 픽셀로 인식해 내부 도로망과 open/lock 연결 (1티어 진입 규약 미러).
            constexpr int HALF_T = TILE_PER_PIXEL / 2;
            RoadPolyLine poly;
            poly.minor = true;
            poly.verts.reserve(bends.size() + (cityVertPtr != nullptr ? 1u : 0u));
            if (cityVertPtr != nullptr) poly.verts.push_back(*cityVertPtr);
            for (const auto& [bx, by] : bends)
                poly.verts.push_back(Point3{ bx * TILE_PER_PIXEL + TILE_BASE_X + HALF_T, by * TILE_PER_PIXEL + TILE_BASE_Y + HALF_T, an.z });
            roads.push_back(poly);
            if (onRoad) onRoad(poly);

            //2티어는 mask만 stamp — core(=1티어)만 링크 표적. 2티어끼리는 절단으로만 상호작용.
            for (const Anchor& c : cells) stampDilated(c.px, c.py);
            t2TotalPx += static_cast<int>(cells.size());

            rootHash.insert(static_cast<int>(rootPx.size()), an.px, an.py);
            rootPx.emplace_back(an.px, an.py);

            const int myId = static_cast<int>(branches.size());
            if (subOut != nullptr)
                for (int i = SUB_END_SKIP; i < static_cast<int>(cells.size()) - SUB_END_SKIP; i += SUB_SPACING)
                    subOut->push_back(T2Ref{ myId, i });

            branches.push_back(T2Branch{ roads.size() - 1, an, std::move(cells), parentId, parentOrd, linked, -1,
                                         cityVertPtr != nullptr, cityVertPtr != nullptr ? *cityVertPtr : Point3{ 0, 0, 0 } });
            return linked ? 2 : 1;
        };

        //라운드 C: 도시 방사 → 라운드 0: 1티어 앵커 발아 → 라운드 1: 서브가지 (깊이 2 고정).
        int nRadial = 0, nFeeder = 0, nLink = 0, nSub = 0;
        std::vector<T2Ref> subAnchors;

        //라운드 C — 교외 지방도가 도시 경계에서 밖으로. 도시 중심에서 4카디널로 걸어 첫
        //  비도시 픽셀을 뿌리로, 마지막 도시 픽셀의 바깥쪽 가장자리 타일을 프리펜드
        //  (buildRoadNetwork cityEdgeTileForRoad 미러 — CityPlan 진입 등록과 좌표 정합).
        //  뿌리 간격(T2_ROOT_DIST)이 자연 스케일링: 소도시는 1~2방, 대도시는 4방 모두.
        {
            auto isCityT = [](Terrain t) noexcept -> bool {
                return t == Terrain::CityZone || t == Terrain::CityCenter
                    || t == Terrain::CityRiver || t == Terrain::CitySea;
            };
            static constexpr int RD[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
            for (const CityNode& c : cities)
            {
                const int ccx = (c.center.x - TILE_BASE_X) / TILE_PER_PIXEL;
                const int ccy = (c.center.y - TILE_BASE_Y) / TILE_PER_PIXEL;
                for (int d = 0; d < 4; ++d)
                {
                    if (u01(rng) >= CITY_RADIAL_PROB) continue;
                    const int rdx = RD[d][0], rdy = RD[d][1];
                    int px = ccx, py = ccy;
                    bool out = false;
                    for (int stp = 0; stp < 200 && !out; ++stp)
                    {
                        if (!isCityT(worldPixel(worldWrap::wrapPixelX(px), py))) out = true;
                        else { px += rdx; py += rdy; }
                    }
                    if (!out || !pathOk(worldPixel(worldWrap::wrapPixelX(px), py))) continue;   //경계 밖이 물이면 스킵

                    const int cityPx = px - rdx, cityPy = py - rdy;   //마지막 도시 픽셀
                    const int localX = (rdx > 0) ? (TILE_PER_PIXEL - 1) : (rdx < 0) ? 0 : TILE_PER_PIXEL / 2;
                    const int localY = (rdy > 0) ? (TILE_PER_PIXEL - 1) : (rdy < 0) ? 0 : TILE_PER_PIXEL / 2;
                    const Point3 cityVert{ cityPx * TILE_PER_PIXEL + TILE_BASE_X + localX, cityPy * TILE_PER_PIXEL + TILE_BASE_Y + localY, c.center.z };

                    //주축이 방사 방향(rd)이 되도록 앵커 세팅 — pdx=-sdy·side에서 side=1 고정이면 sdx=rdy, sdy=-rdx.
                    const Anchor an{ px, py, c.center.z, rdy, -rdx };
                    if (growBranch(an, 1, -1, 0, &cityVert, &subAnchors) != 0) ++nRadial;
                }
            }
        }

        for (const Anchor& an : anchors)
        {
            if (u01(rng) >= T2_SPROUT_T1 * densityAt(an.px, an.py)) continue;
            const int side = std::uniform_int_distribution<int>{ 0, 1 }(rng) ? 1 : -1;
            const int r = growBranch(an, side, -1, 0, nullptr, &subAnchors);
            if (r == 1) ++nFeeder; else if (r == 2) ++nLink;
        }
        for (const T2Ref& sr : subAnchors)
        {
            const Anchor an = branches[static_cast<std::size_t>(sr.branchId)].cells[static_cast<std::size_t>(sr.ord)];   //복사 — growBranch가 branches를 재할당
            if (u01(rng) >= T2_SPROUT_SUB * densityAt(an.px, an.py)) continue;
            const int side = std::uniform_int_distribution<int>{ 0, 1 }(rng) ? 1 : -1;
            if (growBranch(an, side, sr.branchId, sr.ord, nullptr, nullptr) != 0) ++nSub;
        }
        prt(L"[worldGen] tier2 roads: %d radial + %d feeder + %d link + %d sub (anchors=%llu, %d px)\n",
            nRadial, nFeeder, nLink, nSub, static_cast<std::uint64_t>(anchors.size()), t2TotalPx);

        //사이트 앵커 풀 — 전 가지 셀의 평면 참조 (셀 수 ∝ 호길이라 균일 추첨 = 호길이 비례).
        std::vector<T2Ref> t2refs;
        t2refs.reserve(static_cast<std::size_t>(t2TotalPx));
        for (int b = 0; b < static_cast<int>(branches.size()); ++b)
            for (int i = 0; i < static_cast<int>(branches[static_cast<std::size_t>(b)].cells.size()); ++i)
                t2refs.push_back(T2Ref{ b, i });

        //══════════════════════════════════════════════════════════════════
        // 도시/사이트 SpatialHash — 최소거리 검사용.
        //   도시 거리는 center가 아니라 footprint bbox까지의 클램프 거리 — 대도시(bbox
        //   수십px)의 가장자리 옆에 minCityDist=6짜리 사이트가 붙는 것을 막는다.
        //   질의 반경은 minCityDist + 최대 bbox 반변으로 보수적으로 잡고 람다에서 정밀 판정.
        //══════════════════════════════════════════════════════════════════

        SpatialHash cityHash(PixelCostGrid::W, PixelCostGrid::H, 32);
        int maxBboxHalf = 0;
        for (std::size_t i = 0; i < cities.size(); ++i)
        {
            const CityNode& c = cities[i];
            cityHash.insert(static_cast<int>(i), c.bboxPx + c.bboxW / 2, c.bboxPy + c.bboxH / 2);
            maxBboxHalf = std::max(maxBboxHalf, std::max(c.bboxW, c.bboxH) / 2 + 1);
        }
        SpatialHash siteHash(PixelCostGrid::W, PixelCostGrid::H, 16);
        std::vector<std::pair<int, int>> sitePx;   //accept된 사이트 중심 픽셀 (siteHash 인덱스 대상)

        //══════════════════════════════════════════════════════════════════
        // [C] 다트 루프 — placeCities placeTier 미러.
        //══════════════════════════════════════════════════════════════════

        int remaining[N_TYPES];
        int targetTotal = 0;
        for (int t = 0; t < N_TYPES; ++t) { remaining[t] = SITE_TABLE[t].target; targetTotal += SITE_TABLE[t].target; }

        int placed = 0, attempts = 0;
        int placedByType[N_TYPES] = {};
        const int maxAttempts = targetTotal * MAX_ATTEMPTS_MULT;

        struct Seg { int dx; int dy; int len; };

        while (placed < targetTotal && attempts < maxAttempts)
        {
            ++attempts;

            //① 타입 룰렛 — 잔여 target 있는 타입만, weight 비례.
            int totalWeight = 0;
            for (int t = 0; t < N_TYPES; ++t)
                if (remaining[t] > 0) totalWeight += SITE_TABLE[t].weight;
            if (totalWeight <= 0) break;
            int roll = std::uniform_int_distribution<int>{ 0, totalWeight - 1 }(rng);
            int type = 0;
            for (int t = 0; t < N_TYPES; ++t)
            {
                if (remaining[t] <= 0) continue;
                roll -= SITE_TABLE[t].weight;
                if (roll < 0) { type = t; break; }
            }
            const SiteTypeDef& def = SITE_TABLE[type];

            //② 경로 구성 — 타입별 분기.
            int nSegs; Seg segs[3]; int bendX[4], bendY[4]; int z;
            T2Ref siteRef{ -1, -1 };   //roadside일 때 앵커 셀 참조 — accept 시 가지 수요 기록용
            if (def.trunk || branches.empty())
            {
                //trunk — 1티어 앵커에서 직선/L자/Z자 전용 가지 (국도 오프램프 모델).
                //  90° 고정인 이유: 1티어 도로도 4방향 turn-keep 단순화 결과물이라 월드맵
                //  오토타일(코너 스프라이트)과 스타일이 정합. 45°는 계단현상으로 오히려 절차티.
                //  t2cells가 빈 퇴화 월드(도로 극소)에선 전 타입이 이 경로로 폴백.
                const Anchor& an = anchors[std::uniform_int_distribution<std::size_t>{ 0, anchors.size() - 1 }(rng)];
                int outX = -an.sdy, outY = an.sdx;   //본선 수직 진출 방향(카디널 — 세그먼트가 이미 카디널 스냅)
                if (std::uniform_int_distribution<int>{ 0, 1 }(rng)) { outX = -outX; outY = -outY; }
                const int sideSign = std::uniform_int_distribution<int>{ 0, 1 }(rng) ? 1 : -1;
                const int sideX = an.sdx * sideSign, sideY = an.sdy * sideSign;   //측면 jog(본선 평행 ±)

                //모양 룰렛: 직선 15% / L자(진출→측면) 40% / Z자(진출→측면→재진출) 45%.
                const int shapeRoll = std::uniform_int_distribution<int>{ 0, 99 }(rng);
                nSegs = (shapeRoll < 15) ? 1 : (shapeRoll < 55) ? 2 : 3;
                segs[0] = { outX, outY, std::uniform_int_distribution<int>{ BRANCH_OUT_MIN, BRANCH_OUT_MAX }(rng) };
                if (nSegs >= 2) segs[1] = { sideX, sideY, std::uniform_int_distribution<int>{ BRANCH_SEG_MIN, BRANCH_SEG_MAX }(rng) };
                if (nSegs >= 3) segs[2] = { outX, outY, std::uniform_int_distribution<int>{ BRANCH_SEG_MIN, BRANCH_SEG_MAX }(rng) };
                bendX[0] = an.px; bendY[0] = an.py; z = an.z;
            }
            else
            {
                //roadside — 2티어 셀 추첨 + 수직 스퍼. 시설이 도로 끝이 아니라 길가에.
                //  SITE_TERM_BIAS 비율은 가지 말단 1/4 구간에서 추첨 — "이 시설까지 내려고
                //  만든 길" 패턴 제조([D] 트림과 합쳐져 데드엔드가 목적지로 끝남). 나머지는
                //  전 셀 균일(호길이 비례) — 밀도 높은 지방일수록 사이트도 밀집.
                if (u01(rng) < SITE_TERM_BIAS)
                {
                    siteRef.branchId = std::uniform_int_distribution<int>{ 0, static_cast<int>(branches.size()) - 1 }(rng);
                    const int n = static_cast<int>(branches[static_cast<std::size_t>(siteRef.branchId)].cells.size());
                    siteRef.ord = std::uniform_int_distribution<int>{ n - std::max(1, n / 4), n - 1 }(rng);
                }
                else
                    siteRef = t2refs[std::uniform_int_distribution<std::size_t>{ 0, t2refs.size() - 1 }(rng)];
                const Anchor& c = branches[static_cast<std::size_t>(siteRef.branchId)].cells[static_cast<std::size_t>(siteRef.ord)];
                const int side = std::uniform_int_distribution<int>{ 0, 1 }(rng) ? 1 : -1;
                nSegs = 1;
                segs[0] = { -c.sdy * side, c.sdx * side, std::uniform_int_distribution<int>{ SITE_SPUR_MIN, SITE_SPUR_MAX }(rng) };
                bendX[0] = c.px; bendY[0] = c.py; z = c.z;
            }

            //③ 경로 워크 — 전 픽셀 비물·비도시 + 타도로 교차 금지 + 꺾임점/입구 픽셀 확정.
            //   core는 전 스텝 금지(앵커 자신은 스텝 0이라 미검사), mask(1링)는 출발 직후
            //   (첫 세그 t<2)만 면제 — 자기 도로의 1링 dilation 안에서 출발하기 때문.
            bool ok = true;
            for (int s = 0; s < nSegs && ok; ++s)
            {
                for (int t = 1; t <= segs[s].len && ok; ++t)
                {
                    const int px = bendX[s] + segs[s].dx * t;
                    const int py = bendY[s] + segs[s].dy * t;
                    if (!pathOk(worldPixel(worldWrap::wrapPixelX(px), py)))                        ok = false;
                    else if (roadCore.count(maskKey(px, py)) != 0)                                 ok = false;
                    else if ((s > 0 || t >= 2) && roadMask.count(maskKey(px, py)) != 0)            ok = false;
                }
                bendX[s + 1] = bendX[s] + segs[s].dx * segs[s].len;
                bendY[s + 1] = bendY[s] + segs[s].dy * segs[s].len;
            }
            if (!ok) continue;

            //④ 입구 픽셀(마지막 세그 끝) + footprint 좌상단 도출 — 마지막 진행 축 정렬.
            const int ex = bendX[nSegs], ey = bendY[nSegs];
            const int dirX = segs[nSegs - 1].dx, dirY = segs[nSegs - 1].dy;
            int tlx, tly;   //footprint 좌상단 픽셀
            if      (dirX > 0) { tlx = ex;              tly = ey - def.h / 2; }
            else if (dirX < 0) { tlx = ex - def.w + 1;  tly = ey - def.h / 2; }
            else if (dirY > 0) { tlx = ex - def.w / 2;  tly = ey;             }
            else               { tlx = ex - def.w / 2;  tly = ey - def.h + 1; }

            //⑤ 검사 — 전부 통과 시 accept.
            //  footprint: 지형 + 산(위성∪절차) + 기존 도로 침범. 1링도 산 금지(오토타일
            //  오버행) — 단 광산은 산에 붙어야 하므로 1링 산 검사 면제(footprint만 비산).
            for (int oy = -1; oy <= def.h && ok; ++oy)
            for (int ox = -1; ox <= def.w && ok; ++ox)
            {
                const int px = tlx + ox, py = tly + oy;
                const bool inFoot = (ox >= 0 && ox < def.w && oy >= 0 && oy < def.h);
                if (!inFoot && def.needMountain) continue;   //광산 — 1링 산 허용(산기슭 밀착)
                if (inFoot && !terrainOk(worldPixel(worldWrap::wrapPixelX(px), py))) ok = false;
                else if (inFoot && roadMask.count(maskKey(px, py)) != 0)             ok = false;
                else if (worldPixel(worldWrap::wrapPixelX(px), py) == Terrain::Mountain) ok = false;   //1링 위성 산
                else if (isMountainChunk(px, py, seed))                              ok = false;       //위성∪절차 산 (Sector 6단계와 동일 합집합)
            }
            if (!ok) continue;

            //  도시 최소거리 — footprint 중심에서 도시 bbox까지 클램프 거리.
            const int scx = tlx + def.w / 2, scy = tly + def.h / 2;
            cityHash.forEachInRadius(scx, scy, def.minCityDistPx + maxBboxHalf, [&](int ci)
            {
                if (!ok) return;
                const CityNode& c = cities[static_cast<std::size_t>(ci)];
                const int ddx = std::max({ c.bboxPx - scx, 0, scx - (c.bboxPx + c.bboxW - 1) });
                const int ddy = std::max({ c.bboxPy - scy, 0, scy - (c.bboxPy + c.bboxH - 1) });
                if (ddx * ddx + ddy * ddy < def.minCityDistPx * def.minCityDistPx) ok = false;
            });
            if (!ok) continue;

            //  사이트간 최소거리.
            siteHash.forEachInRadius(scx, scy, MIN_SITE_DIST_PX, [&](int si)
            {
                if (!ok) return;
                const auto [px, py] = sitePx[static_cast<std::size_t>(si)];
                const int ddx = px - scx, ddy = py - scy;
                if (ddx * ddx + ddy * ddy < MIN_SITE_DIST_PX * MIN_SITE_DIST_PX) ok = false;
            });
            if (!ok) continue;

            //  광산 특수 — footprint 밖 MINE_MTN_SCAN_R 링 내에 산(위성 or 절차) 필수.
            if (def.needMountain)
            {
                bool nearMtn = false;
                for (int oy = -MINE_MTN_SCAN_R; oy < def.h + MINE_MTN_SCAN_R && !nearMtn; ++oy)
                for (int ox = -MINE_MTN_SCAN_R; ox < def.w + MINE_MTN_SCAN_R && !nearMtn; ++ox)
                {
                    const int px = tlx + ox, py = tly + oy;
                    if (worldPixel(worldWrap::wrapPixelX(px), py) == Terrain::Mountain) nearMtn = true;
                    else if (isMountainChunk(px, py, seed)) nearMtn = true;
                }
                if (!nearMtn) continue;
            }

            //⑥ accept — SiteNode + 스퍼/가지 폴리라인(minor) 등록.
            //  타일 변환: pos=좌상단 픽셀의 좌상단 타일(CitySymbol 규약), 폴리라인 verts=픽셀 중심.
            SiteNode node;
            node.pos    = Point3{ tlx * TILE_PER_PIXEL + TILE_BASE_X, tly * TILE_PER_PIXEL + TILE_BASE_Y, z };
            node.w      = def.w;
            node.h      = def.h;
            node.symbol = def.symbol;
            sites.push_back(node);

            constexpr int HALF_T = TILE_PER_PIXEL / 2;
            RoadPolyLine branch;
            branch.minor = true;
            branch.verts.reserve(static_cast<std::size_t>(nSegs) + 1);
            for (int s = 0; s <= nSegs; ++s)
                branch.verts.push_back(Point3{ bendX[s] * TILE_PER_PIXEL + TILE_BASE_X + HALF_T, bendY[s] * TILE_PER_PIXEL + TILE_BASE_Y + HALF_T, z });
            roads.push_back(branch);

            //스퍼/가지도 마스크에 stamp — 후속 사이트가 그 위에 앉는 것 방지.
            //  core엔 안 넣음 — 성장 라운드는 이미 끝났고, 시설 진입로는 링크 표적이 아님.
            for (int s = 0; s < nSegs; ++s)
                for (int t = 0; t <= segs[s].len; ++t)
                    stampDilated(bendX[s] + segs[s].dx * t, bendY[s] + segs[s].dy * t);

            if (siteRef.branchId >= 0)   //가지 수요 기록 — [D] 정산의 존치/트림 기준
            {
                int& lu = branches[static_cast<std::size_t>(siteRef.branchId)].lastUsed;
                lu = std::max(lu, siteRef.ord);
            }

            siteHash.insert(static_cast<int>(sitePx.size()), scx, scy);
            sitePx.emplace_back(scx, scy);
            remaining[type]--;
            placedByType[type]++;
            placed++;

            if (onSite) onSite(node);
            if (onRoad) onRoad(branch);
        }

        //══════════════════════════════════════════════════════════════════
        // [D] 정산 — 성장은 투기적이었다: 수요(사이트·존치 자식·1티어 접속)를 못 얻은
        //   막다른 가지는 철거하고, 존치 피더도 마지막 수요 지점 + TRIM_MARGIN 뒤 꼬리를
        //   트림한다. 남는 데드엔드는 전부 시설/자식 가지로 가는 길로 끝난다.
        //   판정은 id 내림차순 — 서브가지는 항상 부모보다 뒤에 append되므로 자식이 먼저
        //   판정되고, 존치 자식은 부모의 뿌리 셀(rootOrd)을 수요로 전파. 링크는 연결로라
        //   무조건 존치·무트림. 마스크는 정리 안 함 — placeSites 로컬이고 이후 소비자 없음.
        //   (WorldGenScreen roadsSnap엔 철거분이 남지만 월드젠 화면 한정 코스메틱.)
        //══════════════════════════════════════════════════════════════════
        {
            auto vertOf = [](int px, int py, int z) noexcept -> Point3 {
                constexpr int HALF_T = TILE_PER_PIXEL / 2;
                return Point3{ px * TILE_PER_PIXEL + TILE_BASE_X + HALF_T, py * TILE_PER_PIXEL + TILE_BASE_Y + HALF_T, z };
            };

            std::vector<char> cullRoad(roads.size(), 0);
            int nCulled = 0, nTrimmed = 0;
            for (int b = static_cast<int>(branches.size()) - 1; b >= 0; --b)
            {
                T2Branch& br = branches[static_cast<std::size_t>(b)];
                if (!br.linked && br.lastUsed < 0) { cullRoad[br.roadIdx] = 1; ++nCulled; continue; }   //수요 0 — 철거

                if (br.parent >= 0)   //존치 — 부모에 수요 전파
                {
                    int& plu = branches[static_cast<std::size_t>(br.parent)].lastUsed;
                    plu = std::max(plu, br.rootOrd);
                }
                if (br.linked) continue;   //연결로 — 무트림

                const int last = static_cast<int>(br.cells.size()) - 1;
                const int upTo = std::min(last, br.lastUsed + TRIM_MARGIN_PX);
                if (upTo >= last) continue;   //꼬리 없음
                ++nTrimmed;

                //verts 재구성 — (도시 가장자리 프리펜드) + 뿌리 + 방향 전환점 + 트림 지점.
                RoadPolyLine& poly = roads[br.roadIdx];
                poly.verts.clear();
                if (br.cityRoot) poly.verts.push_back(br.cityVert);
                poly.verts.push_back(vertOf(br.root.px, br.root.py, br.root.z));
                for (int i = 1; i <= upTo; ++i)
                    if (br.cells[static_cast<std::size_t>(i)].sdx != br.cells[static_cast<std::size_t>(i - 1)].sdx
                     || br.cells[static_cast<std::size_t>(i)].sdy != br.cells[static_cast<std::size_t>(i - 1)].sdy)
                        poly.verts.push_back(vertOf(br.cells[static_cast<std::size_t>(i - 1)].px, br.cells[static_cast<std::size_t>(i - 1)].py, br.root.z));
                poly.verts.push_back(vertOf(br.cells[static_cast<std::size_t>(upTo)].px, br.cells[static_cast<std::size_t>(upTo)].py, br.root.z));
            }
            if (nCulled > 0)
            {
                std::vector<RoadPolyLine> keptRoads;
                keptRoads.reserve(roads.size() - static_cast<std::size_t>(nCulled));
                for (std::size_t i = 0; i < roads.size(); ++i)
                    if (cullRoad[i] == 0) keptRoads.push_back(std::move(roads[i]));
                roads = std::move(keptRoads);
            }
            prt(L"[worldGen] tier2 settle: %d culled, %d trimmed (of %llu branches)\n",
                nCulled, nTrimmed, static_cast<std::uint64_t>(branches.size()));
        }

        prt(L"[worldGen] placeSites done: %d/%d placed, %d attempts, %.1fms\n", placed, targetTotal, attempts, (getNanoTimer() - tStart) / 1'000'000.0);
        for (int t = 0; t < N_TYPES; ++t)
            prt(L"  site type %d: %d/%d\n", t, placedByType[t], SITE_TABLE[t].target);

        return sites;
    }
}
