module;
export module Lot:ParkingLot;

import std;
import :base;

//공영주차장 Lot 뼈대 — 2x2 authored, 회전 허용. 개방형 주차장(차량 스폰 등). build() 후속 작성.
//  월드맵 심볼: mapset2by2 #9.
export class ParkingLot final : public Lot
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

export inline const ParkingLot parkingLot;
