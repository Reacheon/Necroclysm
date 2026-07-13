module;
export module Lot:base;

import std;
import util;
import constVar;
export import VehiclePlan; //차량 관련 헬퍼는 해당 모듈 파일 참조

//한 z층 평면. 벡터는 y*w + x 인덱싱, itemID::none = 미설정.
export struct LotPlane
{
    std::vector<int> floor;
    std::vector<int> wall;
    std::vector<int> prop;
};

//Lot 내부 좌표 (LotBuilder 그리드 0..w/h).
export struct LotSpawnItemStack
{
    int x = 0;
    int y = 0;
    int z = 0;
    std::vector<std::pair<int, int>> items;
};

export struct LotSpawnMonster
{
    int x = 0;
    int y = 0;
    int z = 0;
    int entityCode = 0;
};

//plan은 VehicleBuilder.finish()가 만든 불변 footprint. createVehicleFromPlan가 replay.
//  shared_ptr — CityPlan/SectorPlan 캐시 복사 시 깊은 복사 회피, const라 공유 안전.
export struct LotSpawnVehicle
{
    int x = 0;
    int y = 0;
    int z = 0;
    std::shared_ptr<const VehiclePlan> plan;
};

//setProp으로 깐 prop의 leadItem.pocketPtr에 채울 아이템 목록.
//  (x,y,z)는 prop이 깔리는 lot 로컬 좌표 — setProp과 동일 좌표여야 인스턴스화 시점에 매칭됨.
//  setProp 사전호출 여부는 LotBuilder에서 강제하지 않음 — 런타임에 prop 없으면 silent skip.
export struct LotSpawnPropContents
{
    int x = 0;
    int y = 0;
    int z = 0;
    std::vector<std::pair<int, int>> items;
};

export struct LotResult
{
    int w = 0;
    int h = 0;
    std::map<int, LotPlane> planes;  //쓰여진 z만 보유 (sparse)
    std::vector<LotSpawnItemStack> itemStacks;
    std::vector<LotSpawnMonster>   monsters;
    std::vector<LotSpawnVehicle>   vehicles;
    std::vector<LotSpawnPropContents>   propContents;
};

export class LotBuilder
{
    int w_;
    int h_;
    std::map<int, LotPlane> planes_;
    std::vector<LotSpawnItemStack> itemStacks_;
    std::vector<LotSpawnMonster>   monsters_;
    //차량은 addVehicle가 VehicleBuilder&를 반환해 호출자가 부품을 채우므로, take()까지
    //  살아있는 빌더를 deque로 보관(재할당 시 참조 무효 방지). x/y/z는 spawn anchor.
    struct PendingVehicle { int x; int y; int z; VehicleBuilder builder; };
    std::deque<PendingVehicle>     vehBuilders_;
    std::vector<LotSpawnPropContents>   propContents_;

    LotPlane& planeAt(int z)
    {
        auto& p = planes_[z];
        if (p.floor.empty())
        {
            p.floor.assign(w_ * h_, itemID::none);
            p.wall.assign(w_ * h_, itemID::none);
            p.prop.assign(w_ * h_, itemID::none);
        }
        return p;
    }

    //검증 훅 — errorBox 사용 (assert는 Release에서 no-op).
    //  floor는 레이어링 허용(asphalt 위 yellow 중앙선 등 정상 패턴),
    //  wall/prop/spawn은 한 (x,y,z)에 하나만 — 중복은 실수로 간주.
    void checkBounds(int x, int y, const wchar_t* who) const
    {
        errorBox(x < 0 || x >= w_ || y < 0 || y >= h_,
            std::wstring(L"LotBuilder::") + who + L" bounds 위반: (x=" +
            std::to_wstring(x) + L", y=" + std::to_wstring(y) +
            L"), 허용 [0," + std::to_wstring(w_) + L")x[0," + std::to_wstring(h_) + L")");
    }

public:
    LotBuilder(int w, int h) : w_(w), h_(h) {}

    void setFloor(int x, int y, int z, int tile)
    {
        checkBounds(x, y, L"setFloor");
        planeAt(z).floor[y * w_ + x] = tile;
    }
    void setWall(int x, int y, int z, int wall)
    {
        checkBounds(x, y, L"setWall");
        auto& slot = planeAt(z).wall[y * w_ + x];
        errorBox(slot != itemID::none,
            L"LotBuilder::setWall 중복 페인트: (x=" + std::to_wstring(x) +
            L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        slot = wall;
    }
    void setProp(int x, int y, int z, int prop)
    {
        checkBounds(x, y, L"setProp");
        auto& slot = planeAt(z).prop[y * w_ + x];
        errorBox(slot != itemID::none,
            L"LotBuilder::setProp 중복 페인트: (x=" + std::to_wstring(x) +
            L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        slot = prop;
    }

    void addItemStack(int x, int y, int z, std::vector<std::pair<int, int>> items)
    {
        checkBounds(x, y, L"addItemStack");
        for (const auto& s : itemStacks_)
        {
            errorBox(s.x == x && s.y == y && s.z == z,
                L"LotBuilder::addItemStack 중복 spawn: (x=" + std::to_wstring(x) +
                L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        }
        itemStacks_.push_back({ x, y, z, std::move(items) });
    }
    void addMonster(int x, int y, int z, int entityCode)
    {
        checkBounds(x, y, L"addMonster");
        for (const auto& m : monsters_)
        {
            errorBox(m.x == x && m.y == y && m.z == z,
                L"LotBuilder::addMonster 중복 spawn: (x=" + std::to_wstring(x) +
                L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        }
        monsters_.push_back({ x, y, z, entityCode });
    }

    //차량 spawn 등록. anchor (x,y,z) + 코어(leadItem/type/name)로 VehicleBuilder를 만들어
    //  참조를 반환 — 호출자가 extendPart/addPart/addCargo/setDir로 footprint를 채운다
    //  (VehiclePrefab helper 또는 inline). 각 부품은 VehicleBuilder가 Lot 할당 범위(w_,h_)에
    //  대해 bounds 검증. anchor (x,y,z) 중복만 여기서 검사. 반환 참조는 deque라 take()까지 유효.
    VehicleBuilder& addVehicle(int x, int y, int z, int leadItem, vehFlag type, std::wstring name)
    {
        checkBounds(x, y, L"addVehicle");
        for (const auto& pv : vehBuilders_)
        {
            errorBox(pv.x == x && pv.y == y && pv.z == z,
                L"LotBuilder::addVehicle 중복 spawn: (x=" + std::to_wstring(x) +
                L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        }
        vehBuilders_.push_back({ x, y, z, VehicleBuilder(x, y, w_, h_, leadItem, type, std::move(name)) });
        return vehBuilders_.back().builder;
    }

    //setProp으로 깐 prop의 내부 ItemPocket(냉장고 안 등)에 아이템 채움.
    //  같은 (x,y,z) 중복 호출 금지 — append 의도면 한 호출에 items 묶어서 전달.
    //  setProp 선행 필수 — 같은 (x,y,z)에 prop이 없으면 작성 시점에 errorBox로 잡는다.
    //  (단, prop이 컨테이너가 아닌 경우(pocket 없음)는 빌더가 itemDex를 모르므로 못 잡고,
    //   런타임 인스턴스화 시점에서 검출한다 — World_createChunk.cpp 2c.1 참조.)
    //  startArea.ixx가 createProp 직후 TileProp(...)->leadItem.pocketPtr 직접 채우는
    //  우회 패턴을 Lot 데이터로 표현하기 위한 채널.
    void addPropContents(int x, int y, int z, std::vector<std::pair<int, int>> items)
    {
        checkBounds(x, y, L"addPropContents");
        errorBox(planeAt(z).prop[y * w_ + x] == itemID::none,
            L"LotBuilder::addPropContents: 같은 좌표에 prop이 없음 - setProp 선행 필요: (x=" +
            std::to_wstring(x) + L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        for (const auto& c : propContents_)
        {
            errorBox(c.x == x && c.y == y && c.z == z,
                L"LotBuilder::addPropContents 중복: (x=" + std::to_wstring(x) +
                L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        }
        propContents_.push_back({ x, y, z, std::move(items) });
    }

    LotResult take()
    {
        std::vector<LotSpawnVehicle> vehicles;
        vehicles.reserve(vehBuilders_.size());
        for (auto& pv : vehBuilders_)
        {
            vehicles.push_back({ pv.x, pv.y, pv.z, pv.builder.finish() });
        }
        return { w_, h_, std::move(planes_), std::move(itemStacks_), std::move(monsters_),
                 std::move(vehicles), std::move(propContents_) };
    }
};

export class Lot
{
public:
    virtual ~Lot() = default;
    //footprint는 *청크 단위*(1 청크 = TILE_PER_PIXEL 타일). raw 타일 수를
    //잘못 박지 못하게 generate()가 ×TILE_PER_PIXEL로 변환해 그리드 크기 결정.
    virtual int sizeChunkW() const = 0;
    virtual int sizeChunkH() const = 0;

    //회전 허용 여부. 모든 Lot이 명시적으로 결정하도록 순수 가상 — 깜빡 시 컴파일 에러.
    //  대다수 건물은 true. 회전 시 형태가 깨지는 랜드마크/유니크 건물만 false를 반환,
    //  배치기는 정규 방향(none)으로만 앉혀야 한다(회전 적용 지점에서 errorBox 가드).
    virtual bool allowRotation() const = 0;

    LotResult generate(std::uint64_t seed) const
    {
        LotBuilder b(sizeChunkW() * TILE_PER_PIXEL, sizeChunkH() * TILE_PER_PIXEL);
        build(b, seed);
        return b.take();
    }

protected:
    //Lot에 정의되는 건물들의 default 방향 컨벤션은 북향(남쪽에 문이 달린) 구조를 기본으로 함.
    //  (정규 방향 하나로만 작성. 90/180/270°판은 rotateLotResult(Lot:rotate)가 CCW로 자동 생성)
    virtual void build(LotBuilder& b, std::uint64_t seed) const = 0;
};
