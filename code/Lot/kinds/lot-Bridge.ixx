module;
export module Lot:Bridge;

import std;
import :base;
import :Street;       //streetDir 비트 컨벤션 공유 (도로/다리 같은 마스크 의미)
import constVar;

// ════════════════════════════════════════════════════════════════════════
// Bridge — 강을 가로지르는 다리 lot. z=+1 deck + 양 끝 ramp + pillarWall + 가드레일.
//
//   Street와 동일 패턴 (openSides_ 비트마스크, sizeChunk 1×1). 직선 NS/EW 2종만 —
//   강이 1픽셀 폭 직선이라 ([[project_cityriver_internal]]) 코너/T 다리 불필요.
//   다리 결정 stage가 NS/EW 외 마스크를 만들지 않도록 보장 → bridgeByOpenSides
//   default가 nullptr.
//
//   z 레이어 (NS 기준, EW는 x/y 좌표 교환):
//     z=0:
//       y=0..2 / y=21..23 : 진입로 floor (paver 4 + asphalt 16 + paver 4).
//                           강변 Street ↔ 다리 deck 시각 전환 + 진입 유도.
//       y=2    / y=21     : rampUp prop — z=0↔z=+1 양방향 페어 한 쪽.
//                           진입로의 다리쪽 끝줄 = deck 진입 자리.
//       y=3    / y=20     : pillarWall — 다리 밑 지지 + 역진입 차단 + 시야 차단.
//       y=4..19           : 손 안 댐 (deepFreshWater/deepSeaWater 그대로 → 보트 통과).
//       모서리 4구석 3타일: 진입로 가드레일 (점 형태, 진입 방향 시각 유도).
//     z=+1:
//       y=2..21           : deck floor (paver 4 + asphalt 16 + paver 4).
//                           양 끝 2칸(y=0,1 / y=22,23) 비워 진입로 위로 그림자 X.
//       x=0 / x=23        : deck 양 옆 가드레일 (deck 길이만큼).
//       x=11..12          : 중앙 노란 점선 (deck 길이만큼).
//       y=2    / y=21     : rampDown prop (페어 다른 쪽).
//
//   ramp/pillarWall은 lot 전 폭(0..23) — 차량+보행 모두 다리로 올라가고 deck 밑
//   지지/그림자가 인도까지 끊김 없이 이어짐.
// ════════════════════════════════════════════════════════════════════════

export class Bridge final : public Lot
{
    std::uint8_t openSides_;

public:
    constexpr explicit Bridge(std::uint8_t openSides) : openSides_(openSides) {}

    int sizeChunkW() const override { return 1; }
    int sizeChunkH() const override { return 1; }

protected:
    void build(LotBuilder& b, std::uint64_t /*seed*/) const override
    {
        using namespace streetDir;
        const bool n = openSides_ & N;
        const bool e = openSides_ & E;
        const bool s = openSides_ & S;
        const bool w = openSides_ & W;

        constexpr int LO = 4, HI = 19;

        //── 0) deck 범위 — 진입로 양 끝 2줄(y=0,1 / y=22,23)에는 deck를 안 깔아
        //   진입로 위로 deck 그림자가 떨어지지 않게. ramp 줄(y=2/21)이 deck 끝.
        int deckXLo = 0, deckXHi = 23, deckYLo = 0, deckYHi = 23;
        if      (n && s && !e && !w) { deckYLo = 2; deckYHi = 21; }
        else if (e && w && !n && !s) { deckXLo = 2; deckXHi = 21; }

        //── 1) z=+1 deck 베이스 페인트 — Street와 동일 paver/asphalt 룰, z만 +1
        for (int y = deckYLo; y <= deckYHi; ++y)
        {
            const bool inLaneY = (y >= LO && y <= HI);
            const bool nBand   = (y < LO);
            const bool sBand   = (y > HI);

            for (int x = deckXLo; x <= deckXHi; ++x)
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

                b.setFloor(x, y, 1, floor);
            }
        }

        //── 2) 중앙선 + 가드레일 + ramp + pillarWall — 방향별 분기
        constexpr int DASH = 6;

        if (n && s && !e && !w)
        {
            //NS 다리 ──────────────────────────────────────────
            //중앙 노란 점선 (z=+1, deck 길이만큼 — 인접 강변 Street와 자연스럽게 이어짐)
            for (int y = deckYLo; y <= deckYHi; ++y)
            {
                if (y % DASH < 3)
                {
                    b.setFloor(11, y, 1, itemID::yellowAsphaltRightHalf);
                    b.setFloor(12, y, 1, itemID::yellowAsphaltLeftHalf);
                }
            }
            //양 옆 가드레일 (z=+1, deck 길이만큼)
            for (int y = deckYLo; y <= deckYHi; ++y)
            {
                b.setWall(0,  y, 1, itemID::guardrail);
                b.setWall(23, y, 1, itemID::guardrail);
            }
            //양 끝 ramp — lot 전 폭(x=0..23). 진입로 끝줄(y=2/21) = deck 진입 자리.
            //   z=0/z=+1 양방향 페어로 차량+보행 모두 다리 위↔아래 통행.
            for (int x = 0; x <= 23; ++x)
            {
                b.setProp(x, 2,  0, itemID::rampUp);     //도로 → 다리 (북쪽 끝 진입)
                b.setProp(x, 2,  1, itemID::rampDown);   //다리 → 도로 (북쪽 끝 출구)
                b.setProp(x, 21, 0, itemID::rampUp);     //도로 → 다리 (남쪽 끝)
                b.setProp(x, 21, 1, itemID::rampDown);
            }
            //pillarWall (z=0) — deck 전체 폭(x=0..23)에 박아 다리 밑 지지 + 역진입/시야 차단.
            //   인도까지 채워야 deck 그림자가 강 위까지 끊김 없이 이어짐.
            for (int x = 0; x < 24; ++x)
            {
                b.setWall(x, 3,  0, itemID::pillarWall);
                b.setWall(x, 20, 0, itemID::pillarWall);
            }
            //z=0 진입로 — 양 끝 3줄 (y=0..2, y=21..23). 강변 Street ↔ 다리 deck 시각 전환.
            for (int x = 0; x < 24; ++x)
            {
                const bool inLaneX = (x >= LO && x <= HI);
                const int floor = inLaneX ? itemID::blackAsphalt : itemID::paver;
                for (int dy = 0; dy < 3; ++dy)
                {
                    b.setFloor(x, dy,      0, floor);
                    b.setFloor(x, 23 - dy, 0, floor);
                }
            }
            //진입로 모서리 가드레일 (z=0, 4 모서리 3타일씩 — 진입 방향 시각 유도)
            b.setWall(0,  0,  0, itemID::guardrail);
            b.setWall(0,  1,  0, itemID::guardrail);
            b.setWall(0,  2,  0, itemID::guardrail);
            b.setWall(23, 0,  0, itemID::guardrail);
            b.setWall(23, 1,  0, itemID::guardrail);
            b.setWall(23, 2,  0, itemID::guardrail);
            b.setWall(0,  23, 0, itemID::guardrail);
            b.setWall(0,  22, 0, itemID::guardrail);
            b.setWall(0,  21, 0, itemID::guardrail);
            b.setWall(23, 23, 0, itemID::guardrail);
            b.setWall(23, 22, 0, itemID::guardrail);
            b.setWall(23, 21, 0, itemID::guardrail);
        }
        else if (e && w && !n && !s)
        {
            //EW 다리 ──────────────────────────────────────────
            //중앙 노란 점선 (z=+1, deck 길이만큼)
            for (int x = deckXLo; x <= deckXHi; ++x)
            {
                if (x % DASH < 3)
                {
                    b.setFloor(x, 11, 1, itemID::yellowAsphaltBottomHalf);
                    b.setFloor(x, 12, 1, itemID::yellowAsphaltTopHalf);
                }
            }
            //양 옆 가드레일 (z=+1, deck 길이만큼)
            for (int x = deckXLo; x <= deckXHi; ++x)
            {
                b.setWall(x, 0,  1, itemID::guardrail);
                b.setWall(x, 23, 1, itemID::guardrail);
            }
            //양 끝 ramp — lot 전 폭(y=0..23). 진입로 끝줄(x=2/21) = deck 진입 자리.
            for (int y = 0; y <= 23; ++y)
            {
                b.setProp(2,  y, 0, itemID::rampUp);
                b.setProp(2,  y, 1, itemID::rampDown);
                b.setProp(21, y, 0, itemID::rampUp);
                b.setProp(21, y, 1, itemID::rampDown);
            }
            //pillarWall (z=0) — deck 전체 폭(y=0..23)에 박아 다리 밑 지지 + 역진입/시야 차단.
            for (int y = 0; y < 24; ++y)
            {
                b.setWall(3,  y, 0, itemID::pillarWall);
                b.setWall(20, y, 0, itemID::pillarWall);
            }
            //z=0 진입로 — 양 끝 3줄 (x=0..2, x=21..23). 강변 Street ↔ 다리 deck 시각 전환.
            for (int y = 0; y < 24; ++y)
            {
                const bool inLaneY = (y >= LO && y <= HI);
                const int floor = inLaneY ? itemID::blackAsphalt : itemID::paver;
                for (int dx = 0; dx < 3; ++dx)
                {
                    b.setFloor(dx,      y, 0, floor);
                    b.setFloor(23 - dx, y, 0, floor);
                }
            }
            //진입로 모서리 가드레일 (z=0, 4 모서리 3타일씩 — NS 대칭)
            b.setWall(0,  0,  0, itemID::guardrail);
            b.setWall(1,  0,  0, itemID::guardrail);
            b.setWall(2,  0,  0, itemID::guardrail);
            b.setWall(0,  23, 0, itemID::guardrail);
            b.setWall(1,  23, 0, itemID::guardrail);
            b.setWall(2,  23, 0, itemID::guardrail);
            b.setWall(23, 0,  0, itemID::guardrail);
            b.setWall(22, 0,  0, itemID::guardrail);
            b.setWall(21, 0,  0, itemID::guardrail);
            b.setWall(23, 23, 0, itemID::guardrail);
            b.setWall(22, 23, 0, itemID::guardrail);
            b.setWall(21, 23, 0, itemID::guardrail);
        }
    }
};

export inline const Bridge bridgeNS(streetDir::N | streetDir::S);
export inline const Bridge bridgeEW(streetDir::E | streetDir::W);

//마스크 → Bridge 라우팅. 다리 결정 stage가 NS/EW만 박도록 보장 → 그 외 입력은
//호출자 버그 (코너/T 다리는 강 데이터 규약상 발생 불가).
export inline const Bridge* bridgeByOpenSides(std::uint8_t openSides)
{
    using namespace streetDir;
    switch (openSides)
    {
    case N|S: return &bridgeNS;
    case E|W: return &bridgeEW;
    default:  return nullptr;
    }
}
