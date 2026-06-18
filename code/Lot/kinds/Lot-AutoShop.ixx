module;
export module Lot:AutoShop;

import std;
import :base;

//정비소(차량 정비소) Lot 뼈대 — 1x1, 회전 허용. 자동차 정비(부품·공구 등). build() 후속 작성.
//  월드맵 심볼: mapset1by1 #56.
export class AutoShop final : public Lot
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

export inline const AutoShop autoShop;
