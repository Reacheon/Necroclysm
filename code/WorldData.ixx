export module WorldData;

import std;

export constexpr int WORLD_DATA_SIZE = 1080; //가로세로 1080 픽셀
export constexpr int WORLD_MAX_HEIGHT = 32; //z=-16~ z=15

export enum class chunkType
{
    none,
    deepSea,
    shallowSea,
    beach,
    meadow,
    mountain,
    river,
    lake,
    forest,
    desert,
    snow,
};

export class WorldData
{
private:
    std::array<chunkType, WORLD_DATA_SIZE* WORLD_DATA_SIZE* WORLD_MAX_HEIGHT> prophecy;

public:
    
    WorldData() //생성자이며 최초에 지형 생성을 시작함
    {
        prophecy.fill(chunkType::none);

        //지상 0층 지도 생성 시작
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                writeProphecy(x, y, 0, chunkType::deepSea);
            }
        }
    };
    
    
    WorldData(const WorldData&) = delete; //복사 생성 명시적 거부
    WorldData& operator=(const WorldData&) = delete; //복사대 대입 명시적 거부
    
   
    chunkType getProphecy(int x, int y, int z) const
    {
        return prophecy[(z + WORLD_MAX_HEIGHT / 2) * (WORLD_DATA_SIZE * WORLD_DATA_SIZE) + y * WORLD_DATA_SIZE + x];
    }

    void writeProphecy(int x, int y, int z, chunkType inputChunkType)
    {
        prophecy[(z + WORLD_MAX_HEIGHT / 2) * (WORLD_DATA_SIZE * WORLD_DATA_SIZE) + y * WORLD_DATA_SIZE + x] = inputChunkType;
    }
   
    
};

export std::unique_ptr<WorldData> currentWorld; //현재 게임에서 로드된 월드
