module;
export module lot:Sample;

import :Infra;

export struct Sample final
{
    Sample() = delete;
    static constexpr int SIZE_W = 1;
    static constexpr int SIZE_H = 1;
    static void generate(Context&);
};

void Sample::generate(Context& ctx)
{
}

static_assert(LotDef<Sample>);
