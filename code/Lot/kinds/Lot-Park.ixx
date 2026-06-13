module;
export module Lot:Park;

import std;
import :base;

//공원 Lot 뼈대 — footprint/회전만 정의. build() 후속 작성.
export class Park final : public Lot
{
public:
    int sizeChunkW() const override { return 2; }
    int sizeChunkH() const override { return 2; }
    bool allowRotation() const override { return true; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
    }
};

export inline const Park park;
