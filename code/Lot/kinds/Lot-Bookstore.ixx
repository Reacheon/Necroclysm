module;
export module Lot:Bookstore;

import std;
import :base;

//서점 Lot 뼈대 — footprint/회전만 정의. build() 후속 작성.
export class Bookstore final : public Lot
{
public:
    int sizeChunkW() const override { return 1; }
    int sizeChunkH() const override { return 1; }
    bool allowRotation() const override { return true; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
    }
};

export inline const Bookstore bookstore;
