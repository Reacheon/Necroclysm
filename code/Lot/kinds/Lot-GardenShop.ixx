module;
export module Lot:GardenShop;

import std;
import :base;

//원예점 Lot 뼈대 — 1x1, 회전 허용. 원예 소매(씨앗·화분·모종 등). build() 후속 작성.
//  월드맵 심볼: mapset1by1 #60.
export class GardenShop final : public Lot
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

export inline const GardenShop gardenShop;
