module;
export module Lot:rotate;

import std;
import util;
import constVar;
import globalVar;
import ItemData;
import :base;

// ════════════════════════════════════════════════════════════════════════
//  Lot 회전 — LotResult를 회전한 사본을 만드는 순수 후처리.
//
//  기여자는 Lot을 정규 방향(canonical) 하나로만 작성하고, 90/180/270° 회전판은
//  이 모듈이 LotResult 값을 변환해 자동 생성한다 → Lot마다 회전 코드 부채 0.
//
//  규약
//    · 회전은 반시계(CCW). 아이템의 rotatedCCW90ItemCode 방향과 일치.
//    · 방향성 타일/prop은 rotatedCCW90ItemCode 4-사이클이 완전해야 회전된다.
//      대칭 아이템은 0 — 회전 시 항등으로 남는다.
//    · 비정사각 footprint(24/48/72)는 90/270°에서 W/H가 스왑된다. 다중 청크 Lot의
//      실제 배치는 후속 예약 레이어가 처리한다(이번 범위 밖).
//    · 회전량은 lotRot enum. z는 불변(평면 내 회전). 분기는 lotRot 값으로 직접 한다
//      — 열거자 정수값에 의존하지 않는다.
// ════════════════════════════════════════════════════════════════════════

export enum class lotRot { none, ccw90, ccw180, ccw270 };
export LotResult rotateLotResult(const LotResult& src, lotRot rot);
       int       rotateItemCode(int code, lotRot rot);
       dir16     rotateDir(dir16 d, lotRot rot);         
       void      rotCell(int x, int y, int w, int h, lotRot rot, int& outputX, int& outputY);
       void      rotOffset(int dx, int dy, lotRot rot, int& outputX, int& outputY);
       int       oneCCW(int code);

//Lot 생성 후 회전. generate는 :base 멤버라 자유 함수로 위임한다
//  (멤버로 두면 :base가 :rotate를 import해야 해 파티션 순환이 생긴다).
export LotResult generateRotated(const Lot& lot, std::uint64_t seed, lotRot rot)
{
    return rotateLotResult(lot.generate(seed), rot);
}

//LotResult를 회전한 새 값을 반환한다.
LotResult rotateLotResult(const LotResult& src, lotRot rot)
{
    if (rot == lotRot::none) return src;

    const int W = src.w;
    const int H = src.h;
    const bool swap = (rot == lotRot::ccw90 || rot == lotRot::ccw270);
    const int OW = swap ? H : W;
    const int OH = swap ? W : H;

    LotResult dst;
    dst.w = OW;
    dst.h = OH;

    //평면(floor/wall/prop): z별로 셀 좌표와 아이템 코드를 함께 회전한다.
    for (const auto& [z, plane] : src.planes)
    {
        LotPlane out;
        out.floor.assign(static_cast<std::size_t>(OW) * OH, itemID::none);
        out.wall .assign(static_cast<std::size_t>(OW) * OH, itemID::none);
        out.prop .assign(static_cast<std::size_t>(OW) * OH, itemID::none);

        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
            {
                const std::size_t si = static_cast<std::size_t>(y) * W + x;
                const int f  = plane.floor[si];
                const int wl = plane.wall[si];
                const int p  = plane.prop[si];
                if (f == itemID::none && wl == itemID::none && p == itemID::none) continue;

                int outputX, outputY;
                rotCell(x, y, W, H, rot, outputX, outputY);
                const std::size_t di = static_cast<std::size_t>(outputY) * OW + outputX;
                if (f  != itemID::none) out.floor[di] = rotateItemCode(f,  rot);
                if (wl != itemID::none) out.wall [di] = rotateItemCode(wl, rot);
                if (p  != itemID::none) out.prop [di] = rotateItemCode(p,  rot);
            }

        dst.planes.emplace(z, std::move(out));
    }

    //spawn 채널: 위치(x,y)만 회전, z 불변.
    for (const auto& s : src.itemStacks)
    {
        int outputX, outputY; rotCell(s.x, s.y, W, H, rot, outputX, outputY);
        dst.itemStacks.push_back({ outputX, outputY, s.z, s.items });
    }
    for (const auto& m : src.monsters)
    {
        int outputX, outputY; rotCell(m.x, m.y, W, H, rot, outputX, outputY);
        dst.monsters.push_back({ outputX, outputY, m.z, m.entityCode });
    }
    for (const auto& c : src.propContents)
    {
        int outputX, outputY; rotCell(c.x, c.y, W, H, rot, outputX, outputY);
        dst.propContents.push_back({ outputX, outputY, c.z, c.items });
    }

    //차량: anchor와 VehiclePlan(오프셋·bodyDir)을 회전. 부품 코드는 그대로 두고
    //  스프라이트가 bodyDir로 회전된다 — createVehicleFromPlan이 회전된 plan을 replay.
    for (const auto& v : src.vehicles)
    {
        int outputX, outputY; rotCell(v.x, v.y, W, H, rot, outputX, outputY);

        LotSpawnVehicle nv;
        nv.x = outputX; nv.y = outputY; nv.z = v.z;
        if (v.plan)
        {
            VehiclePlan rp = *v.plan;
            rp.bodyDir = rotateDir(rp.bodyDir, rot);
            for (auto& op : rp.ops)
            {
                int odx, ody; rotOffset(op.dx, op.dy, rot, odx, ody);
                op.dx = odx; op.dy = ody;
            }
            nv.plan = std::make_shared<const VehiclePlan>(std::move(rp));
        }
        dst.vehicles.push_back(std::move(nv));
    }

    return dst;
}

//──────────────────────────────────────────────────────────────────────────
//  디버그 검증기 — 코드 부채 방지.
//  방향성 아이템(rotatedCCW90ItemCode != 0)의 체인이 도중 0 없이 2 또는 4스텝 안에
//  자기 자신으로 닫히는지 점검해, 부분/깨진 체인을 기여 즉시 잡는다.
//  dataLoader가 itemDex 로드 직후 디버그 빌드에서 1회 호출. (step은 체인 길이.)
//──────────────────────────────────────────────────────────────────────────
export void validateRotationChains()
{
    const int n = static_cast<int>(itemDex.size());
    for (int c = 0; c < n; ++c)
    {
        if (itemDex[c].rotatedCCW90ItemCode == 0) continue;   //비회전 — 검사 대상 아님

        int cur = c;
        int period = 0;          //0 = 4스텝 내 미복귀
        bool brokeZero = false;
        bool outOfRange = false;
        for (int step = 1; step <= 4; ++step)
        {
            const int next = itemDex[cur].rotatedCCW90ItemCode;
            if (next == 0)            { brokeZero = true;  break; }
            if (next >= n)            { outOfRange = true; break; }
            cur = next;
            if (cur == c) { period = step; break; }
        }

        if (brokeZero)
            errorBox(L"[Lot:rotate] 회전 체인 끊김(도중 0): itemCode=" + std::to_wstring(c) +
                L" (" + itemDex[c].name + L")");
        else if (outOfRange)
            errorBox(L"[Lot:rotate] 회전 체인 범위 초과 코드 참조: itemCode=" + std::to_wstring(c) +
                L" (" + itemDex[c].name + L")");
        else if (period == 0)
            errorBox(L"[Lot:rotate] 회전 체인 4스텝 내 미복귀(5-사이클 이상): itemCode=" +
                std::to_wstring(c) + L" (" + itemDex[c].name + L")");
        else if (period != 2 && period != 4)
            errorBox(L"[Lot:rotate] 회전 체인 주기 비정상(2/4 아님, period=" + std::to_wstring(period) +
                L"): itemCode=" + std::to_wstring(c) + L" (" + itemDex[c].name + L")");
    }
}

//══ 내부 프리미티브 ═════════════════════════════════════════════════════════
//  위 회전 함수가 합성하는 저수준 좌표/코드 회전. 비-export(이 TU 전용).
//════════════════════════════════════════════════════════════════════════════

//아이템 코드를 회전량만큼 체인 합성해 따라간다. none·대칭(0)은 항등.
//  도중 0(데이터 버그)이면 그 코드를 유지한다(graceful degrade).
//  크래프트 UI와는 rotatedCCW90ItemCode 필드만 공유한다(UI는 1-스텝).
int rotateItemCode(int code, lotRot rot)
{
    switch (rot)
    {
    case lotRot::ccw90:  return oneCCW(code);
    case lotRot::ccw180: return oneCCW(oneCCW(code));
    case lotRot::ccw270: return oneCCW(oneCCW(oneCCW(code)));
    default:             return code;   //none
    }
}

//dir16을 회전. 22.5° 스텝이라 90°=ACW2², 180°=reverse, 270°=CW2².
//  평면 dir 전용 — above/below 입력 금지(ACW2/CW2/reverse가 errorBox).
dir16 rotateDir(dir16 d, lotRot rot)
{
    switch (rot)
    {
    case lotRot::ccw90:  return ACW2(ACW2(d));
    case lotRot::ccw180: return reverse(d);
    case lotRot::ccw270: return CW2(CW2(d));
    default:             return d;       //none
    }
}

//셀 좌표 (x,y) → 회전 후 좌표. 90/270°는 출력 dims가 H×W로 스왑된다. 화면 y-down 기준 CCW.
//  (검산: top-half 582 → CCW → left-half 580 데이터와 일치.)
void rotCell(int x, int y, int w, int h, lotRot rot, int& outputX, int& outputY)
{
    switch (rot)
    {
    case lotRot::ccw90:  outputX = y;           outputY = (w - 1) - x; break;
    case lotRot::ccw180: outputX = (w - 1) - x; outputY = (h - 1) - y; break;
    case lotRot::ccw270: outputX = (h - 1) - y; outputY = x;           break;
    default:             outputX = x;           outputY = y;           break;   //none
    }
}

//오프셋(벡터) 회전 — 상수항 없는 형태. 차량 부품 오프셋용.
void rotOffset(int dx, int dy, lotRot rot, int& outputX, int& outputY)
{
    switch (rot)
    {
    case lotRot::ccw90:  outputX = dy;  outputY = -dx; break;
    case lotRot::ccw180: outputX = -dx; outputY = -dy; break;
    case lotRot::ccw270: outputX = -dy; outputY = dx;  break;
    default:             outputX = dx;  outputY = dy;  break;   //none
    }
}

//체인 1칸 CCW. none·끊김(0)은 항등.
int oneCCW(int code)
{
    if (code == itemID::none) return itemID::none;
    const int next = itemDex[code].rotatedCCW90ItemCode;
    return next == 0 ? code : next;
}
