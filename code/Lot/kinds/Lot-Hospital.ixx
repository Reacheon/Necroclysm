module;
export module Lot:Hospital;

import std;
import :base;

//병원 Lot 뼈대 — 2x1 authored, 회전 허용(1x2 커버). 의료(응급·수술 등). build() 후속 작성.
//  월드맵 심볼: mapset2by2 #13(2x1 수평) / #14(1x2 수직) — resolveSymbol이 footprint로 분기.
export class Hospital final : public Lot
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

export inline const Hospital hospital;
