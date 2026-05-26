module;
export module lot:base;

import std;
import constVar;

//한 z층에 깔리는 floor/wall/prop 평면. 각 벡터는 y*w + x 순서, itemID::none = 미설정
export struct LotPlane
{
    std::vector<int> floor;
    std::vector<int> wall;
    std::vector<int> prop;
};

export struct LotResult
{
    int w = 0;
    int h = 0;
    std::map<int, LotPlane> planes;  //쓰여진 z만 존재 (sparse)
};

export class LotBuilder
{
    int w_;
    int h_;
    std::map<int, LotPlane> planes_;

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

public:
    LotBuilder(int w, int h) : w_(w), h_(h) {}

    void setFloor(int x, int y, int z, int tile) { planeAt(z).floor[y * w_ + x] = tile; }
    void setWall(int x, int y, int z, int wall)  { planeAt(z).wall[y * w_ + x] = wall; }
    void setProp(int x, int y, int z, int prop)  { planeAt(z).prop[y * w_ + x] = prop; }

    LotResult take() { return { w_, h_, std::move(planes_) }; }
};

export class Lot
{
public:
    virtual ~Lot() = default;
    //Lot footprint — *청크 단위*. 1 청크 = TILE_PER_PIXEL(24) 타일.
    //generate()가 자동으로 ×TILE_PER_PIXEL 변환해 LotBuilder 그리드 크기 결정 →
    //기여자가 36 같은 raw 타일 수 잘못 박는 사고 차단. 양자화 게임의 footprint
    //불변식(24·48·72…)이 이름과 변환 컨벤션으로 자연 강제됨.
    //CLAUDE.md footprint 24/48/72는 1/2/3 청크.
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
