export module WorldData;

import std;

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
};

export class WorldData
{
private:
    std::array<chunkType, WORLD_DATA_SIZE* WORLD_DATA_SIZE* WORLD_MAX_HEIGHT> prophecy;
public:

    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMap30;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMap10;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMapBeach;

    WorldData(std::uint64_t inputSeed) //생성자이며 최초에 지형 생성을 시작함
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

        std::mt19937_64 gen(inputSeed);

        auto randFloat = [&](float a, float b) -> float
            {
                if (a > b) std::swap(a, b);
                std::uniform_real_distribution<double> dis(a, b);
                return dis(gen);
            };

        auto createNoiseMap = [&]<int FREQ>(std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>&outputNoise)
        {
            constexpr int gridSize = WORLD_DATA_SIZE / FREQ + 1;
            std::array<std::array<std::pair<float, float>, gridSize>, gridSize> unitVector;

            for (int y = 0; y < (WORLD_DATA_SIZE / FREQ + 1); y++)
            {
                for (int x = 0; x < (WORLD_DATA_SIZE / FREQ + 1); x++)
                {
                    float rad = randFloat(0, 2 * std::numbers::pi);
                    unitVector[x][y] = { std::cos(rad),std::sin(rad) };
                }
            }

            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    float s, t, u, v;
                    int x0 = x / FREQ;
                    int y0 = y / FREQ;

                    //x0와 y0랑 스케일을 맞춘 x와 y 좌표
                    float scaledX = (float)x / FREQ;
                    float scaledY = (float)y / FREQ;

                    s = unitVector[x0][y0].first * (scaledX - x0) + unitVector[x0][y0].second * (scaledY - y0);
                    t = unitVector[x0 + 1][y0].first * (scaledX - (x0 + 1)) + unitVector[x0 + 1][y0].second * (scaledY - y0);
                    u = unitVector[x0][y0 + 1].first * (scaledX - x0) + unitVector[x0][y0 + 1].second * (scaledY - (y0 + 1));
                    v = unitVector[x0 + 1][y0 + 1].first * (scaledX - (x0 + 1)) + unitVector[x0 + 1][y0 + 1].second * (scaledY - (y0 + 1));

                    float sx = 3 * std::pow(scaledX - x0, 2) - 2 * std::pow(scaledX - x0, 3);
                    float sy = 3 * std::pow(scaledY - y0, 2) - 2 * std::pow(scaledY - y0, 3);
                    float a = s + sx * (t - s);
                    float b = u + sx * (v - u);
                    outputNoise[x][y] = a + sy * (b - a);
                }
            }
        };


        createNoiseMap.operator() < 120 > (noiseMap);
        createNoiseMap.operator() < 30 > (noiseMap30);
        createNoiseMap.operator() < 10 > (noiseMap10);
        createNoiseMap.operator() < 120 > (noiseMapBeach);


        //육지 생성
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                int dx = std::abs(x - WORLD_DATA_SIZE / 2);
                int dy = std::abs(y - WORLD_DATA_SIZE / 2);

                float penalty = std::min(1.0f,(float)sqrt(dx*dx+dy*dy) / (float)(WORLD_DATA_SIZE / 2));

                float height = (noiseMap[x][y] + noiseMap30[x][y] * 0.25f + noiseMap10[x][y] * 0.1f) / 1.35f - penalty * penalty * penalty;

                if (height > -0.2f)
                {
                    if (height < -0.18f && noiseMapBeach[x][y] > -0.2f)
                    {
                        writeProphecy(x, y, 0, chunkType::beach);
                    }
                    else
                    {
                        writeProphecy(x, y, 0, chunkType::dirt);
                    }
                }
            }
        }

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
   
    
};

export std::unique_ptr<WorldData> currentWorld; //현재 게임에서 로드된 월드
