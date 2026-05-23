module;
export module lot:base;

import std;

//한 z층에 깔리는 floor/wall/prop 평면. 각 벡터는 y*w + x 순서, 0 = 미설정
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
            p.floor.assign(w_ * h_, 0);
            p.wall.assign(w_ * h_, 0);
            p.prop.assign(w_ * h_, 0);
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
    virtual int sizeW() const = 0;
    virtual int sizeH() const = 0;

    LotResult generate(std::uint64_t seed) const
    {
        LotBuilder b(sizeW(), sizeH());
        build(b, seed);
        return b.take();
    }

protected:
    virtual void build(LotBuilder& b, std::uint64_t seed) const = 0;
};
