module;
export module Lot:PostOffice;

import std;
import :base;

//우체국 Lot 뼈대 — 1x1, 회전 허용. 우편/택배(소포·우표 등). build() 후속 작성.
//  월드맵 심볼: mapset1by1 #55.
export class PostOffice final : public Lot
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

export inline const PostOffice postOffice;
