export module procGen;

import std;
import util;

//============================================================
// 내부 전용 타입 — 외부로 노출되면 절대 안됨
//============================================================
namespace procGen
{
    enum class Terrain : std::uint8_t
    {
        Land,
        Sea,
        FreshWater, //강이나 호수 모두 포함
        Bridge,
        CityZone, //사전 마킹된 도시의 영역
        CityCenter, //사전 마킹된 도시의 중심점
        Mountain,
        Polar,
        Tundra,
        Subarctic,
        Monsoon,
        Sabanna,
    };

    struct PixelCostGrid
    {
        static constexpr int W = 43200;
        static constexpr int H = 21600;
        std::unique_ptr<Terrain[]> data;

        Terrain at(int px, int py) const noexcept
        {
            return data[static_cast<std::size_t>(py) * W + px];
        }
    };

    //픽셀 좌표 (1픽셀 = 50타일). 절차적 생성 알고리즘 내부 전용.
    //타일 좌표 Point3와 강타입 분리 — 함수 파라미터에서 절대 혼용 불가능.
    //(Point3는 게임 전반에서 쓰는 실타일 좌표이므로 도시/폴리라인 등 외부 데이터는 그대로 Point3 사용)
    struct PixelCoord
    {
        int x{}, y{}, z{};

        PixelCoord() = default;
        constexpr PixelCoord(int x_, int y_, int z_) noexcept
            : x(x_), y(y_), z(z_) {}

        friend constexpr bool operator==(PixelCoord, PixelCoord) noexcept = default;
    };
}

//============================================================
// 외부 인터페이스
//============================================================
export namespace procGen
{
    inline constexpr int TILES_PER_PIXEL = 50;//1픽셀당 실타일 수
    inline constexpr int WORLD_PIXEL_W   = 43200;
    inline constexpr int WORLD_PIXEL_H   = 21600;

    enum class CityTier : std::uint8_t { T1, T2, T3 };

    struct CityNode
    {
        Point3 center;     //실타일 좌표
        CityTier tier;
    };

    struct RoadPolyLine
    {
        std::vector<Point3> verts;  //실타일 좌표
    };

    struct WorldGenResult //오로지 generateWorld 함수의 반환값을 위한 페어 구조체
    {
        std::vector<CityNode> cities;
        std::vector<RoadPolyLine> roads;
    };


    PixelCostGrid loadWorldGrid();
    std::vector<CityNode> placeCities(std::uint64_t seed, const PixelCostGrid& grid);
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const PixelCostGrid& grid, const std::vector<CityNode>& cities);

    /*
     * @brief 월드 골격(도시 좌표 + 도로 폴리라인)을 게임 시작 초기 1회 절차적 생성
     *
     * 도시 내부 도로/타일 페인팅/랜덤 인카운터 등은 청크로드 시점에 지연 생성
     *
     * @param seed 난수 시드
     * @return 도시 노드와 도로 폴리라인 묶음 (실타일 좌표)
     */
    WorldGenResult generateWorld(std::uint64_t seed)
    {
        //1. PNG 데이터 로드
        PixelCostGrid grid = loadWorldGrid();

        //2. 도시 위치 배열
        std::vector<CityNode> cities = placeCities(seed,grid);

        //3. 도로망 생성
        std::vector<RoadPolyLine> roadNetwork = buildRoadNetwork(seed, grid, cities);

        //4. 결과값 반환
        return { cities , roadNetwork};
    }

}
