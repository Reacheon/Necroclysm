module;
export module cell:School;

import :Infra;

export struct School final
{
    School() = delete;
    static constexpr int SIZE_W = 2;
    static constexpr int SIZE_H = 2;
    static void emit(Context&);
};

static_assert(BuildingDef<School>);
