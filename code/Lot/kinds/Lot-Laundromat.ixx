module;
export module Lot:Laundromat;

import std;
import :base;

//세탁소 Lot 뼈대 — 1x1, 회전 허용. 빨래방(세탁기·건조기 등). build() 후속 작성.
//  월드맵 심볼: mapset1by1 #59.
export class Laundromat final : public Lot
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

export inline const Laundromat laundromat;
