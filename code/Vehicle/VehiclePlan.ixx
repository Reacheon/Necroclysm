module;
export module VehiclePlan;

import std;
import util;
import constVar;

//차량을 anchor 기준 offset footprint로 기록한 순수 데이터.
//  Lot 작성 시점에 footprint 전체를 데이터로 남겨 부품별 bounds 검증을 가능케 하고,
//  createVehicleFromPlan가 청크 로드 시점에 실제 Vehicle로 replay한다.
//  op 순서를 보존한다 (extendPart 인접성 / addPart 프레임 선행 때문).
export enum class vehOpKind { extend, part, cargo };

export struct VehiclePlanOp
{
    vehOpKind kind = vehOpKind::extend;
    int dx = 0;                    //anchor 기준 offset (z는 anchor.z 고정)
    int dy = 0;
    std::vector<int> items;        //extend: 프레임 1개 / part: 부품 목록(레이어 순서) / cargo: 컨테이너 1개
    int cargoContent = 0;          //cargo: items[0] 컨테이너에 채울 내용물 itemCode
    int cargoCount = 0;
};

export struct VehiclePlan
{
    int leadItem = itemID::none;   //anchor 타일 프레임
    vehFlag type = vehFlag::none;
    std::wstring name = L"Vehicle";
    dir16 bodyDir = dir16::dir2;    //명시적 facing. 작성한 footprint 방향 그대로 배치된다.
    std::vector<VehiclePlanOp> ops;
};

//명령형 차량 작성 recorder. Vehicle/World 의존 없는 순수 객체.
//  좌표는 Lot-local 절대좌표로 받아 내부에서 anchor offset으로 저장.
//  부품마다 bounds(할당 타일 범위)·인접성·중복을 작성 시점에 errorBox로 검증한다.
export class VehicleBuilder
{
    int anchorX_;
    int anchorY_;
    int w_;                        //Lot 그리드 폭/높이 (bounds용). 음수면 무경계(startArea 등).
    int h_;
    VehiclePlan plan_;
    std::set<std::pair<int, int>> frames_;  //배치된 프레임 offset 집합

    void checkBounds(int x, int y, const wchar_t* who) const
    {
        if (w_ < 0 || h_ < 0) return;
        errorBox(x < 0 || x >= w_ || y < 0 || y >= h_,
            std::wstring(L"VehicleBuilder::") + who + L" footprint bounds 위반: (x=" +
            std::to_wstring(x) + L", y=" + std::to_wstring(y) +
            L"), 허용 [0," + std::to_wstring(w_) + L")x[0," + std::to_wstring(h_) + L")");
    }

public:
    //leadItem/type/name = 차량 정체성(코어). anchor 타일에 leadItem 프레임이 깔린다.
    VehicleBuilder(int anchorX, int anchorY, int w, int h, int leadItem, vehFlag type, std::wstring name)
        : anchorX_(anchorX), anchorY_(anchorY), w_(w), h_(h)
    {
        plan_.leadItem = leadItem;
        plan_.type = type;
        plan_.name = std::move(name);
        frames_.insert({ 0, 0 });  //anchor 타일은 항상 프레임 — 첫 extendPart의 인접 대상
    }

    void setDir(dir16 d) { plan_.bodyDir = d; }

    void extendPart(int x, int y, int itemCode)
    {
        checkBounds(x, y, L"extendPart");
        const int dx = x - anchorX_;
        const int dy = y - anchorY_;
        errorBox(frames_.find({ dx, dy }) != frames_.end(),
            L"VehicleBuilder::extendPart 이미 프레임이 있는 offset으로 확장: (dx=" +
            std::to_wstring(dx) + L", dy=" + std::to_wstring(dy) + L")");
        const bool adj = frames_.find({ dx + 1, dy }) != frames_.end()
            || frames_.find({ dx - 1, dy }) != frames_.end()
            || frames_.find({ dx, dy + 1 }) != frames_.end()
            || frames_.find({ dx, dy - 1 }) != frames_.end();
        errorBox(!adj,
            L"VehicleBuilder::extendPart 상하좌우에 프레임이 없는 offset으로 확장: (dx=" +
            std::to_wstring(dx) + L", dy=" + std::to_wstring(dy) + L")");
        plan_.ops.push_back({ vehOpKind::extend, dx, dy, { itemCode }, 0, 0 });
        frames_.insert({ dx, dy });
    }

    void addPart(int x, int y, std::vector<int> items)
    {
        checkBounds(x, y, L"addPart");
        const int dx = x - anchorX_;
        const int dy = y - anchorY_;
        errorBox(frames_.find({ dx, dy }) == frames_.end(),
            L"VehicleBuilder::addPart 프레임이 없는 offset에 부품 추가: (dx=" +
            std::to_wstring(dx) + L", dy=" + std::to_wstring(dy) + L")");
        plan_.ops.push_back({ vehOpKind::part, dx, dy, std::move(items), 0, 0 });
    }

    void addPart(int x, int y, int item) { addPart(x, y, std::vector<int>{ item }); }

    //컨테이너 부품(연료탱크 등)의 내부 pocket에 내용물을 채움. 같은 타일에 앞선 addPart가
    //  containerCode 아이템을 이미 넣었어야 한다 (op 순서 보존으로 replay 시 보장).
    void addCargo(int x, int y, int containerCode, int contentCode, int count)
    {
        checkBounds(x, y, L"addCargo");
        const int dx = x - anchorX_;
        const int dy = y - anchorY_;
        errorBox(frames_.find({ dx, dy }) == frames_.end(),
            L"VehicleBuilder::addCargo 프레임이 없는 offset에 카고: (dx=" +
            std::to_wstring(dx) + L", dy=" + std::to_wstring(dy) + L")");
        plan_.ops.push_back({ vehOpKind::cargo, dx, dy, { containerCode }, contentCode, count });
    }

    std::shared_ptr<const VehiclePlan> finish()
    {
        return std::make_shared<const VehiclePlan>(std::move(plan_));
    }
};
