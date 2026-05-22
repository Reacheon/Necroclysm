module;
export module cell;

//공용 구현
export import :Infra;

//각 셀 모듈들
export import :Sample;
export import :School;


export enum class cellFlag
{
    School,
    Hospital,
    Bank,
};

export void blit(cellFlag input, Context& ctx)
{
    switch (input)
    {
    case cellFlag::School:
        School::emit(ctx);
        break;
    }
};
