module;
export module cell:Infra;

import std;

export struct Context
{
    int baseTileX;
    int baseTileY;
};

export struct KindMeta
{
    int cellsW;
    int cellsH;
};

export template<typename T>
concept BuildingDef = requires
{
    { T::SIZE_W } -> std::convertible_to<int>;
    { T::SIZE_H } -> std::convertible_to<int>;
    { T::emit(std::declval<Context&>()) };
};
