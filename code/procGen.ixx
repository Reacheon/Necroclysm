export module procGen;

import std;
import util;

//내부 연산 용도 구조체들 (외부로 빠져나가면 절대 안됨)
namespace procGen
{
    struct PixelCoord { int x, y, z; }; //강타입 제약용

    enum class Terrain : std::uint8_t
    {
        Land,
        Sea,
        River,
        Bridge,
        CityZone,
        Mountain
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
}

export namespace procGen
{
    enum class CityTier : std::uint8_t { T1, T2, T3 };

    struct CityNode 
    {
        Point3 center;
        CityTier tier;
    };

    struct RoadPolyLine 
    {
        std::vector<Point3> verts;
    };

    struct WorldGenResult //오로지 generateWorld 함수의 반환값을 위한 페어 구조체
    {
        std::vector<CityNode> cities;
        std::vector<RoadPolyLine> roads;
    };


    PixelCostGrid loadWorldGrid();
    std::vector<CityNode> placeCities(std::uint64_t seed, const PixelCostGrid& grid);
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const PixelCostGrid& grid, std::vector<CityNode>& cities);

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