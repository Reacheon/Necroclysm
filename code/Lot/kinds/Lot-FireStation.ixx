module;
export module Lot:FireStation;

import std;
import :base;

//소방서 Lot 뼈대 — 2x1 고정, 회전 없음. build() 후속 작성.
export class FireStation final : public Lot
{
public:
    int sizeChunkW() const override { return 2; }
    int sizeChunkH() const override { return 1; }
    bool allowRotation() const override { return false; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
    }
};

export inline const FireStation fireStation;
