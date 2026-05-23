module;
export module lot:School;

import std;
import :base;

export class School final : public Lot
{
public:
    int sizeW() const override { return 2; }
    int sizeH() const override { return 2; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
    }
};

export inline const School school;
