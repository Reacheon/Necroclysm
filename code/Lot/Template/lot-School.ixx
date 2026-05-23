module;
export module lot:School;

import :Infra;

export struct School final
{
    School() = delete;
    static constexpr int SIZE_W = 2;
    static constexpr int SIZE_H = 2;
    static void generate(Context&);
};

void School::generate(Context& ctx)
{
}

static_assert(LotDef<School>);
