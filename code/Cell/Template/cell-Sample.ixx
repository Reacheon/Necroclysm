module;
export module cell:Sample;

import :Infra;

export struct Sample final
{
    Sample() = delete;
    static constexpr int SIZE_W = 1;
    static constexpr int SIZE_H = 1;
    static void emit(Context&);
};

static_assert(BuildingDef<Sample>);
