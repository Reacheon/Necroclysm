module;
export module lot:Infra;

import std;

export struct Context
{
    int baseTileX;
    int baseTileY;
};

export struct Footprint
{
    int lotsW;
    int lotsH;
};

export template<typename T>
concept LotDef = requires
{
    { T::SIZE_W } -> std::convertible_to<int>;
    { T::SIZE_H } -> std::convertible_to<int>;
    { T::generate(std::declval<Context&>()) };
};
