module;
export module lot:Sample;

import std;
import :base;

export class Sample final : public Lot
{
public:
    int sizeW() const override { return 1; }
    int sizeH() const override { return 1; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
    }
};

export inline const Sample sample;
