module;
export module Lot:JewelryStore;

import std;
import :base;

//금은방 Lot 뼈대 — 1x1, 회전 허용. 귀금속 소매(금·은·보석 등). build() 후속 작성.
//  월드맵 심볼: mapset1by1 #58.
export class JewelryStore final : public Lot
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

export inline const JewelryStore jewelryStore;
