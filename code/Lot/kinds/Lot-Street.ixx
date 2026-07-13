module;
export module Lot:Street;

import std;
import :base;
import constVar;

// ════════════════════════════════════════════════════════════════════════
// Street — 24타일 도로 Lot. 4면 출구 마스크로 NS/EW/코너/T/십자를 모두 표현.
//
//   설계도 1개에 매개변수만 다르게 줘서 16조합을 한 클래스로 처리.
//   openSides_ 비트마스크 (N=1, E=2, S=4, W=8) → 어느 면이 다음 도로 lot으로 이어지는지.
//
//   24타일 단면:
//     x(or y) [0..3]   4타일 paver-band — 인도 (해당 방향 출구 없으면 paver, 있으면 asphalt)
//     x(or y) [4..19]  16타일 lane     — 차도 (중앙 16×16은 항상 asphalt)
//     x(or y) [20..23] 4타일 paver-band — 인도 (위와 동일)
//
//   16조합 중 11종 인스턴스 export (직선 2 + 코너 4 + T자 4 + 십자 1).
//   degree<2 (단독 N/E/S/W, 또는 마스크 0)는 CityPlan_build stage 7이 제거 보장 → 미지원.
//
//   디테일 수준:
//     - 베이스 페인트(paver/asphalt 갈림): 11종 모두 동일 코드 경로로 정확히 작동
//     - 중앙선·가로수: 직선 NS/EW만. 코너/T/십자는 베이스만 깔림
//       (안쪽 곡선·T자 갈림 처리 등은 시각 확인 후 후속 작업)
//
//   비트 컨벤션은 CityPlan_build의 RoadPixel::Dir과 동일 (N=1, E=2, S=4, W=8) —
//   roads[idx].openBits를 streetByOpenSides()에 그대로 넘기면 됨.
// ════════════════════════════════════════════════════════════════════════

export namespace streetDir
{
    inline constexpr std::uint8_t N = 1;
    inline constexpr std::uint8_t E = 2;
    inline constexpr std::uint8_t S = 4;
    inline constexpr std::uint8_t W = 8;
}

export class Street final : public Lot
{
    std::uint8_t openSides_;

public:
    constexpr explicit Street(std::uint8_t openSides) : openSides_(openSides) {}

    int sizeChunkW() const override { return 1; }
    int sizeChunkH() const override { return 1; }
    bool allowRotation() const override { return true; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
        using namespace streetDir;
        const bool n = openSides_ & N;
        const bool e = openSides_ & E;
        const bool s = openSides_ & S;
        const bool w = openSides_ & W;

        constexpr int LO = 4;     //lane 시작 — paver-band 폭 4
        constexpr int HI = 19;    //lane 끝 (inclusive) — [4..19] = 16타일 차도

        //── 1) 베이스 페인트 — paver/asphalt 결정 ─────────────────────────
        //   중앙 16×16: 항상 asphalt
        //   변 16×4 4개:  해당 방향 출구 켜짐 → asphalt(도로가 lot 밖으로 빠짐), 꺼짐 → paver(인도로 막음)
        //   코너 4×4 4개: 일단 paver (양 출구 켜졌을 때의 안쪽 곡선은 TODO)
        for (int y = 0; y < 24; ++y)
        {
            const bool inLaneY = (y >= LO && y <= HI);
            const bool nBand   = (y < LO);
            const bool sBand   = (y > HI);

            for (int x = 0; x < 24; ++x)
            {
                const bool inLaneX = (x >= LO && x <= HI);
                const bool wBand   = (x < LO);
                const bool eBand   = (x > HI);

                int floor = itemID::paver;
                if      (inLaneX && inLaneY)     floor = itemID::blackAsphalt;
                else if (inLaneX && nBand && n)  floor = itemID::blackAsphalt;
                else if (inLaneX && sBand && s)  floor = itemID::blackAsphalt;
                else if (inLaneY && wBand && w)  floor = itemID::blackAsphalt;
                else if (inLaneY && eBand && e)  floor = itemID::blackAsphalt;

                b.setFloor(x, y, 0, floor);
            }
        }

        //── 2) 중앙선 — 직선 패턴 한정 ───────────────────────────────────
        //   NS 직선 (n+s, e=w=0): x=11,12에 세로 점선 (lot 전 길이 — 다음 lot과 자연스럽게 이어짐)
        //   EW 직선 (e+w, n=s=0): y=11,12에 가로 점선
        //   코너/T/십자는 안 그림 — 충돌·곡선 처리가 별개 작업
        constexpr int DASH = 6;   //점선 주기: 3 on / 3 off
        if (n && s && !e && !w)
        {
            for (int y = 0; y < 24; ++y)
            {
                if (y % DASH < 3)
                {
                    b.setFloor(11, y, 0, itemID::yellowAsphaltRightHalf);
                    b.setFloor(12, y, 0, itemID::yellowAsphaltLeftHalf);
                }
            }
        }
        else if (e && w && !n && !s)
        {
            for (int x = 0; x < 24; ++x)
            {
                if (x % DASH < 3)
                {
                    b.setFloor(x, 11, 0, itemID::yellowAsphaltBottomHalf);
                    b.setFloor(x, 12, 0, itemID::yellowAsphaltTopHalf);
                }
            }
        }

        //── 3) 가로수 — 직선 패턴 한정 ───────────────────────────────────
        //   paver-band 안쪽 끝(LO-1=3, HI+1=20)에 DASH 주기로 배치.
        //   paver → dirt 교체 후 tree prop. CityPlan_build와 동일 패턴.
        constexpr int TREE_KINDS[7] = {
            itemID::ginkgoTree,   itemID::cherryTree,  itemID::mapleTree,
            itemID::magnoliaTree, itemID::oakTree,     itemID::juniperTree,
            itemID::zelkovaTree,
        };
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<int> kindDist(0, 6);

        if (n && s && !e && !w)
        {
            for (int y = 0; y < 24; y += DASH)
            {
                for (int sideX : { LO - 1, HI + 1 })   //3, 20
                {
                    b.setFloor(sideX, y, 0, itemID::dirt);
                    b.setProp (sideX, y, 0, TREE_KINDS[kindDist(rng)]);
                }
            }
        }
        else if (e && w && !n && !s)
        {
            for (int x = 0; x < 24; x += DASH)
            {
                for (int sideY : { LO - 1, HI + 1 })
                {
                    b.setFloor(x, sideY, 0, itemID::dirt);
                    b.setProp (x, sideY, 0, TREE_KINDS[kindDist(rng)]);
                }
            }
        }
    }
};

//── 직선 (2종) ───────────────────────────────────────────────────
export inline const Street streetNS   (streetDir::N | streetDir::S);
export inline const Street streetEW   (streetDir::E | streetDir::W);

//── 코너 (4종) ───────────────────────────────────────────────────
export inline const Street streetNE   (streetDir::N | streetDir::E);
export inline const Street streetNW   (streetDir::N | streetDir::W);
export inline const Street streetSE   (streetDir::E | streetDir::S);
export inline const Street streetSW   (streetDir::S | streetDir::W);

//── T자 (4종) ────────────────────────────────────────────────────
export inline const Street streetNES  (streetDir::N | streetDir::E | streetDir::S);
export inline const Street streetNEW  (streetDir::N | streetDir::E | streetDir::W);
export inline const Street streetNSW  (streetDir::N | streetDir::S | streetDir::W);
export inline const Street streetESW  (streetDir::E | streetDir::S | streetDir::W);

//── 십자 (1종) ───────────────────────────────────────────────────
export inline const Street streetCross(streetDir::N | streetDir::E | streetDir::S | streetDir::W);

// ── 마스크 → 인스턴스 라우팅 ──────────────────────────────────────
//   CityPlan_build에서 픽셀별 roads[idx].openBits를 그대로 넘김.
//   degree<2 (단독 N/E/S/W) 및 0은 stage 7이 제거 보장 → nullptr 반환.
//   nullptr가 나오면 호출자 측 버그 (stage 7 invariant 위반).
export inline const Street* streetByOpenSides(std::uint8_t openSides)
{
    using namespace streetDir;
    switch (openSides)
    {
    case N|S:     return &streetNS;
    case E|W:     return &streetEW;
    case N|E:     return &streetNE;
    case N|W:     return &streetNW;
    case E|S:     return &streetSE;
    case S|W:     return &streetSW;
    case N|E|S:   return &streetNES;
    case N|E|W:   return &streetNEW;
    case N|S|W:   return &streetNSW;
    case E|S|W:   return &streetESW;
    case N|E|S|W: return &streetCross;
    default:      return nullptr;
    }
}
