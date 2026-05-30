module;
export module Lot:ConvenienceStore;

import std;
import :base;
import constVar;
import util;

export class ConvenienceStore final : public Lot
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

export inline const ConvenienceStore convenienceStore;
