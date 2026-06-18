module;
export module Lot:Hotel;

import std;
import :base;

//호텔 Lot 뼈대 — 2x1 authored, 회전 허용(1x2 커버). 숙박/주거형. build() 후속 작성.
//  월드맵 심볼: mapset2by2 #11(2x1 수평) / #12(1x2 수직) — resolveSymbol이 footprint로 분기.
export class Hotel final : public Lot
{
public:
    int sizeChunkW() const override { return 2; }
    int sizeChunkH() const override { return 1; }
    bool allowRotation() const override { return true; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
    }
};

export inline const Hotel hotel;
