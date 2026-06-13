module;
export module Lot:BicycleShop;

import std;
import :base;

//자전거가게 Lot 뼈대 — footprint/회전만 정의. build() 후속 작성.
export class BicycleShop final : public Lot
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

export inline const BicycleShop bicycleShop;
