export module WorldData;

import std;
import util;

export constexpr int WORLD_DATA_SIZE = 1080; //가로세로 1080 픽셀
export constexpr int WORLD_MAX_HEIGHT = 32; //z=-16~ z=15

export enum class chunkType : std::uint8_t
{
    none,
    dirt,
    deepSea,
    shallowSea,
    beach,
    mountain,
    river,
    lake,
    forest,
    desert,
    snow,
    city,//일단 디버그용
    volcanicLand,
    volcano,
    jungle,
};

export enum class cityType : std::uint8_t
{
    normal,
    snow,
    desert,
    port,
    sky,
    underground,
    seastead,
    volcano,
    jungle,
};

export class WorldData
{
private:
    std::array<chunkType, WORLD_DATA_SIZE* WORLD_DATA_SIZE* WORLD_MAX_HEIGHT> prophecy;
    std::unordered_map<Point3, cityType, Point3::Hash> cityTypeMap;
public:

    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMap30;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMap10;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMapBeach;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMapForest;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> heightMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> filledHeightMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> tempMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> desertMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> desertNoise;

    WorldData(std::uint64_t inputSeed) //생성자이며 최초에 지형 생성을 시작함
    {
        generateWorld(inputSeed);
    };


    WorldData(const WorldData&) = delete; //복사 생성 명시적 거부
    WorldData& operator=(const WorldData&) = delete; //복사 대입 명시적 거부


    chunkType getProphecy(int x, int y, int z) const
    {
        return prophecy[(z + WORLD_MAX_HEIGHT / 2) * (WORLD_DATA_SIZE * WORLD_DATA_SIZE) + y * WORLD_DATA_SIZE + x];
    }

    void writeProphecy(int x, int y, int z, chunkType inputChunkType)
    {
        prophecy[(z + WORLD_MAX_HEIGHT / 2) * (WORLD_DATA_SIZE * WORLD_DATA_SIZE) + y * WORLD_DATA_SIZE + x] = inputChunkType;
    }

    void generateWorld(std::uint64_t inputSeed);

};

export std::unique_ptr<WorldData> currentWorld; //현재 게임에서 로드된 월드
