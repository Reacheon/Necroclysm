module;
export module Lot:Bridge;

import std;
import :base;
import :Street;       //streetDir 비트 컨벤션 공유 (도로/다리 같은 마스크 의미)
import constVar;

// ════════════════════════════════════════════════════════════════════════
// Bridge — 강을 가로지르는 다리 lot. z=+1 deck + 양 끝 ramp + pillarWall + 가드레일.
//
//   Street와 동일 패턴 (openSides_ 비트마스크, sizeChunk 1×1). 직선 NS/EW 2축만 —
//   강이 직선이라 ([[project_cityriver_internal]]) 코너/T 다리 불필요. 다리 결정
//   stage가 NS/EW 외 마스크를 만들지 않도록 보장 → bridgeByOpenSides default nullptr.
//
//   ── N픽셀(2~4px) 폭 강 대응: 다리 = N개 연속 1청크 세그먼트 ──
//   강이 N픽셀 폭이면 deck가 N청크를 가로질러야 함. 멀티청크 lot 대신 물 픽셀
//   1개=1청크 lot 1개를 유지하고 role만 바꿔 페인트(픽셀단위 stage 9 모델 보존):
//     N=1 → [Single]                         (양 끝 ramp 자급 — 기존과 동일)
//     N=2 → [EndLow][EndHigh]
//     N≥3 → [EndLow] + (N-2)×[Mid] + [EndHigh]
//   EndLow = 저좌표 강변(NS의 북/EW의 서)에서 ramp up, deck를 고좌표 청크 경계까지
//   꽉 채워 이웃 세그먼트 deck와 맞붙임. EndHigh = 미러. Mid = 풀청크 deck + 아래
//   깊은물, ramp·pillar·진입로 전부 없음(보트가 다리 밑 항법 밴드를 가로지르므로
//   벽 금지). 세그먼트 경계에서 deck/가드레일/깊은물이 연속, ramp는 강변에만 있어
//   다리 중간에서 z 변경 없음.
//
//   z 레이어 (NS·Single 기준. EW는 x/y 좌표 교환. End/Mid는 hasLowRamp/hasHighRamp로
//   해당 끝의 ramp·pillar·진입로를 끄고 그쪽 deck를 청크 경계까지 확장):
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

//다리 세그먼트 역할 — N픽셀 강을 N청크로 쪼갤 때 각 청크가 맡는 위치.
//  Single = 양 끝 ramp 자급(N=1). EndLow/EndHigh = 저/고좌표 강변 끝(한쪽만 ramp,
//  반대쪽 deck를 청크 경계까지 채워 이웃과 맞붙음). Mid = 양 끝 사이 순수 deck.
export enum class BridgeRole : std::uint8_t { Single, EndLow, Mid, EndHigh };

export class Bridge final : public Lot
{
    std::uint8_t openSides_;
    BridgeRole   role_;

public:
    constexpr Bridge(std::uint8_t openSides, BridgeRole role) : openSides_(openSides), role_(role) {}

    int sizeChunkW() const override { return 1; }
    int sizeChunkH() const override { return 1; }
    bool allowRotation() const override { return true; }

protected:
    void build(LotBuilder& b, std::uint64_t /*seed*/) const override
    {
        using namespace streetDir;
        const bool n = openSides_ & N;
        const bool e = openSides_ & E;
        const bool s = openSides_ & S;
        const bool w = openSides_ & W;

        constexpr int LO = 4, HI = 19;

        //── role → 어느 끝에 ramp/진입로/pillar를 둘지. Single은 양 끝, End는 한 끝,
        //   Mid는 둘 다 없음. ramp 없는 끝은 deck를 청크 경계(0/23)까지 채워 이웃과 맞붙임.
        const bool hasLowRamp  = (role_ == BridgeRole::Single || role_ == BridgeRole::EndLow);
        const bool hasHighRamp = (role_ == BridgeRole::Single || role_ == BridgeRole::EndHigh);

        //── 0) deck 범위 — ramp 있는 끝만 진입로 2줄(y=0,1 / y=22,23)을 비워 진입로 위로
        //   deck 그림자 차단. ramp 없는 끝은 청크 경계까지 deck를 깔아 이웃 세그먼트와 연속.
        int deckXLo = 0, deckXHi = 23, deckYLo = 0, deckYHi = 23;
        if      (n && s && !e && !w) { deckYLo = hasLowRamp ? 2 : 0; deckYHi = hasHighRamp ? 21 : 23; }
        else if (e && w && !n && !s) { deckXLo = hasLowRamp ? 2 : 0; deckXHi = hasHighRamp ? 21 : 23; }

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
            //끝 ramp — ramp 있는 끝만(lot 전 폭 x=0..23). 진입로 끝줄(y=2/21)=deck 진입 자리.
            //   z=0/z=+1 양방향 페어로 차량+보행 모두 다리 위↔아래 통행. Mid는 ramp 없음.
            for (int x = 0; x <= 23; ++x)
            {
                if (hasLowRamp)  { b.setProp(x, 2,  0, itemID::rampUp); b.setProp(x, 2,  1, itemID::rampDown); }   //북쪽 끝
                if (hasHighRamp) { b.setProp(x, 21, 0, itemID::rampUp); b.setProp(x, 21, 1, itemID::rampDown); }   //남쪽 끝
            }
            //pillarWall (z=0) — ramp 있는 끝만, deck 전체 폭(x=0..23). 다리 밑 지지 + 역진입/시야 차단.
            //   Mid는 벽 없음 — 보트가 다리 밑 깊은물을 막힘 없이 통과해야 함.
            for (int x = 0; x < 24; ++x)
            {
                if (hasLowRamp)  b.setWall(x, 3,  0, itemID::pillarWall);
                if (hasHighRamp) b.setWall(x, 20, 0, itemID::pillarWall);
            }
            //z=0 진입로 — ramp 있는 끝만 3줄(북 y=0..2 / 남 y=21..23). 강변 Street ↔ 다리 deck 시각 전환.
            for (int x = 0; x < 24; ++x)
            {
                const bool inLaneX = (x >= LO && x <= HI);
                const int floor = inLaneX ? itemID::blackAsphalt : itemID::paver;
                for (int dy = 0; dy < 3; ++dy)
                {
                    if (hasLowRamp)  b.setFloor(x, dy,      0, floor);
                    if (hasHighRamp) b.setFloor(x, 23 - dy, 0, floor);
                }
            }
            //진입로 모서리 가드레일 (z=0, ramp 있는 끝만 3타일씩 — 진입 방향 시각 유도)
            if (hasLowRamp)
            {
                b.setWall(0,  0,  0, itemID::guardrail);
                b.setWall(0,  1,  0, itemID::guardrail);
                b.setWall(0,  2,  0, itemID::guardrail);
                b.setWall(23, 0,  0, itemID::guardrail);
                b.setWall(23, 1,  0, itemID::guardrail);
                b.setWall(23, 2,  0, itemID::guardrail);
            }
            if (hasHighRamp)
            {
                b.setWall(0,  23, 0, itemID::guardrail);
                b.setWall(0,  22, 0, itemID::guardrail);
                b.setWall(0,  21, 0, itemID::guardrail);
                b.setWall(23, 23, 0, itemID::guardrail);
                b.setWall(23, 22, 0, itemID::guardrail);
                b.setWall(23, 21, 0, itemID::guardrail);
            }
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
            //끝 ramp — ramp 있는 끝만(lot 전 폭 y=0..23). 진입로 끝줄(x=2/21)=deck 진입 자리.
            for (int y = 0; y <= 23; ++y)
            {
                if (hasLowRamp)  { b.setProp(2,  y, 0, itemID::rampUp); b.setProp(2,  y, 1, itemID::rampDown); }   //서쪽 끝
                if (hasHighRamp) { b.setProp(21, y, 0, itemID::rampUp); b.setProp(21, y, 1, itemID::rampDown); }   //동쪽 끝
            }
            //pillarWall (z=0) — ramp 있는 끝만, deck 전체 폭(y=0..23). Mid는 벽 없음(보트 통과).
            for (int y = 0; y < 24; ++y)
            {
                if (hasLowRamp)  b.setWall(3,  y, 0, itemID::pillarWall);
                if (hasHighRamp) b.setWall(20, y, 0, itemID::pillarWall);
            }
            //z=0 진입로 — ramp 있는 끝만 3줄(서 x=0..2 / 동 x=21..23). 강변 Street ↔ 다리 deck 시각 전환.
            for (int y = 0; y < 24; ++y)
            {
                const bool inLaneY = (y >= LO && y <= HI);
                const int floor = inLaneY ? itemID::blackAsphalt : itemID::paver;
                for (int dx = 0; dx < 3; ++dx)
                {
                    if (hasLowRamp)  b.setFloor(dx,      y, 0, floor);
                    if (hasHighRamp) b.setFloor(23 - dx, y, 0, floor);
                }
            }
            //진입로 모서리 가드레일 (z=0, ramp 있는 끝만 3타일씩 — NS 대칭)
            if (hasLowRamp)
            {
                b.setWall(0,  0,  0, itemID::guardrail);
                b.setWall(1,  0,  0, itemID::guardrail);
                b.setWall(2,  0,  0, itemID::guardrail);
                b.setWall(0,  23, 0, itemID::guardrail);
                b.setWall(1,  23, 0, itemID::guardrail);
                b.setWall(2,  23, 0, itemID::guardrail);
            }
            if (hasHighRamp)
            {
                b.setWall(23, 0,  0, itemID::guardrail);
                b.setWall(22, 0,  0, itemID::guardrail);
                b.setWall(21, 0,  0, itemID::guardrail);
                b.setWall(23, 23, 0, itemID::guardrail);
                b.setWall(22, 23, 0, itemID::guardrail);
                b.setWall(21, 23, 0, itemID::guardrail);
            }
        }
    }
};

//축 × role 인스턴스 (constexpr 싱글톤). NS는 저=북/고=남, EW는 저=서/고=동.
export inline const Bridge bridgeNS_Single (streetDir::N | streetDir::S, BridgeRole::Single);
export inline const Bridge bridgeNS_EndLow (streetDir::N | streetDir::S, BridgeRole::EndLow);
export inline const Bridge bridgeNS_Mid    (streetDir::N | streetDir::S, BridgeRole::Mid);
export inline const Bridge bridgeNS_EndHigh(streetDir::N | streetDir::S, BridgeRole::EndHigh);
export inline const Bridge bridgeEW_Single (streetDir::E | streetDir::W, BridgeRole::Single);
export inline const Bridge bridgeEW_EndLow (streetDir::E | streetDir::W, BridgeRole::EndLow);
export inline const Bridge bridgeEW_Mid    (streetDir::E | streetDir::W, BridgeRole::Mid);
export inline const Bridge bridgeEW_EndHigh(streetDir::E | streetDir::W, BridgeRole::EndHigh);

//(마스크, role) → Bridge 라우팅. 다리 결정 stage가 NS/EW 축만 박도록 보장 → 그 외
//마스크는 호출자 버그(코너/T 다리는 강 데이터 규약상 발생 불가). role은 N픽셀 강을
//세그먼트로 쪼갠 위치(Single/EndLow/Mid/EndHigh).
export inline const Bridge* bridgeByOpenSides(std::uint8_t openSides, BridgeRole role)
{
    using namespace streetDir;
    switch (openSides)
    {
    case N|S:
        switch (role)
        {
        case BridgeRole::Single:  return &bridgeNS_Single;
        case BridgeRole::EndLow:  return &bridgeNS_EndLow;
        case BridgeRole::Mid:     return &bridgeNS_Mid;
        case BridgeRole::EndHigh: return &bridgeNS_EndHigh;
        }
        return nullptr;
    case E|W:
        switch (role)
        {
        case BridgeRole::Single:  return &bridgeEW_Single;
        case BridgeRole::EndLow:  return &bridgeEW_EndLow;
        case BridgeRole::Mid:     return &bridgeEW_Mid;
        case BridgeRole::EndHigh: return &bridgeEW_EndHigh;
        }
        return nullptr;
    default:  return nullptr;
    }
}
