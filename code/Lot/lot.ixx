module;
export module lot;

//공용 구현
export import :Infra;

//각 Lot 모듈들
export import :Sample;
export import :School;


export enum class lotFlag
{
    School,
    Hospital,
    Bank,
};

export void blitLot(lotFlag input, Context& ctx)
{
    switch (input)
    {
    case lotFlag::School:
        School::generate(ctx);
        break;
    }
};

export Footprint queryLotFootprint(lotFlag input)
{
    switch (input)
    {
    case lotFlag::School:
        return { School::SIZE_W, School::SIZE_H };
    case lotFlag::Hospital:
    case lotFlag::Bank:
        return {};
    }
    return {};
}
