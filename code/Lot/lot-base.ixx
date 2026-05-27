module;
export module Lot:base;

import std;
import util;
import constVar;
import Blueprint;

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

//bp는 Blueprint 모듈의 inline const 전역을 가리킴 — lifetime 안전.
//orientation은 build() 후 spawn 헬퍼가 rotatePartInfo로 적용.
export struct LotSpawnVehicle
{
    int x = 0;
    int y = 0;
    int z = 0;
    const Blueprint* bp = nullptr;
    dir16 orientation = dir16::dir2;
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
    std::vector<LotSpawnVehicle>   vehicles_;
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

    //차량 spawn 등록. bp는 Blueprint 모듈의 inline const 전역 가리켜야 함.
    //  footprint(차량이 부품으로 점유하는 추가 칸)는 *검증하지 않음* — Lot 단계에선
    //  blueprint별 부품 트리 미상이고, createChunk 시점에 덮어쓰기 정책으로 처리.
    //  중복 검증은 anchor (x,y,z) 기준만.
    void addVehicle(int x, int y, int z, const Blueprint* bp, dir16 orientation = dir16::dir2)
    {
        checkBounds(x, y, L"addVehicle");
        errorBox(bp == nullptr, L"LotBuilder::addVehicle bp는 nullptr 불가");
        for (const auto& v : vehicles_)
        {
            errorBox(v.x == x && v.y == y && v.z == z,
                L"LotBuilder::addVehicle 중복 spawn: (x=" + std::to_wstring(x) +
                L", y=" + std::to_wstring(y) + L", z=" + std::to_wstring(z) + L")");
        }
        vehicles_.push_back({ x, y, z, bp, orientation });
    }

    //setProp으로 깐 prop의 내부 ItemPocket(냉장고 안 등)에 아이템 채움.
    //  같은 (x,y,z) 중복 호출 금지 — append 의도면 한 호출에 items 묶어서 전달.
    //  setProp 사전호출은 강제하지 않음 — 런타임에 prop이 없으면 silent skip.
    //  startArea.ixx가 createProp 직후 TileProp(...)->leadItem.pocketPtr 직접 채우는
    //  우회 패턴을 Lot 데이터로 표현하기 위한 채널.
    void addPropContents(int x, int y, int z, std::vector<std::pair<int, int>> items)
    {
        checkBounds(x, y, L"addPropContents");
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
        return { w_, h_, std::move(planes_), std::move(itemStacks_), std::move(monsters_),
                 std::move(vehicles_), std::move(propContents_) };
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

    LotResult generate(std::uint64_t seed) const
    {
        LotBuilder b(sizeChunkW() * TILE_PER_PIXEL, sizeChunkH() * TILE_PER_PIXEL);
        build(b, seed);
        return b.take();
    }

protected:
    virtual void build(LotBuilder& b, std::uint64_t seed) const = 0;
};
