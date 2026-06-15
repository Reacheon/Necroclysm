module;
export module Lot:FireStation;

import std;
import :base;

//소방서 Lot 뼈대 — 2x1 authored, 회전 허용(1x2 커버). 월드맵 심볼 추가로 회전 가능 전환. build() 후속 작성.
export class FireStation final : public Lot
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

export inline const FireStation fireStation;
