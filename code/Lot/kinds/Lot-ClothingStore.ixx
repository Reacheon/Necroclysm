module;
export module Lot:ClothingStore;

import std;
import :base;

//의류점 Lot 뼈대 — 1x1, 회전 허용. 의류 소매(옷·신발 등). build() 후속 작성.
//  월드맵 심볼: mapset1by1 #57.
export class ClothingStore final : public Lot
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

export inline const ClothingStore clothingStore;
