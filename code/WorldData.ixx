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
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> noiseMapForest;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> heightMap;
    std::array<std::array<float, WORLD_DATA_SIZE>, WORLD_DATA_SIZE> filledHeightMap;

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

        //1080의 약수 목록
        //1,2,3,4,5,6,8,9,10,12,15,18,20,24,27,30,36,40,45,54,60,72,90,108,120,135,180,216,270,360,540,1080
        createNoiseMap.operator() < 120 > (noiseMap);
        createNoiseMap.operator() < 30 > (noiseMap30);
        createNoiseMap.operator() < 10 > (noiseMap10);
        createNoiseMap.operator() < 120 > (noiseMapBeach);
        createNoiseMap.operator() < 24 > (noiseMapForest);

        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                int dx = std::abs(x - WORLD_DATA_SIZE / 2);
                int dy = std::abs(y - WORLD_DATA_SIZE / 2);

                float penalty = std::min(1.0f, (float)sqrt(dx * dx + dy * dy) / (float)(WORLD_DATA_SIZE / 2));
                float height = (noiseMap[x][y] + noiseMap30[x][y] * 0.25f + noiseMap10[x][y] * 0.1f) / 1.45f - penalty * penalty * penalty;
                heightMap[x][y] = height; //이거 없어지면 직선 강 생김ㅋ
            }
        }


        //바다 생성
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (heightMap[x][y] > -0.15f) writeProphecy(x, y, 0, chunkType::dirt);
                else if (heightMap[x][y] > -0.3f) writeProphecy(x, y, 0, chunkType::shallowSea);
                else writeProphecy(x, y, 0, chunkType::deepSea);
            }
        }

        //플루드필 : 내륙해 제거
        {
            auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& cond = *condPtr;
            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    if (getProphecy(x, y, 0) == chunkType::shallowSea || getProphecy(x, y, 0) == chunkType::deepSea) cond[x][y] = true;
                    else cond[x][y] = false;
                }
            }
            auto inlandSeaPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& inlandSea = *inlandSeaPtr;
            floodFillFindOrphan(cond, { 0,0 }, inlandSea);
            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    if (inlandSea[x][y] == true) writeProphecy(x, y, 0, chunkType::dirt);
                }
            }
        }

        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (heightMap[x][y] > 0.3)
                {
                    writeProphecy(x, y, 0, chunkType::mountain);
                }
                else if (heightMap[x][y] > 0.2)
                {
                    writeProphecy(x, y, 0, chunkType::forest);
                }
            }
        }


        //플루드필 : 산맥 내부 평원 제거
        {
            auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& cond = *condPtr;
            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    if (getProphecy(x, y, 0) == chunkType::mountain) cond[x][y] = false;
                    else cond[x][y] = true;
                }
            }
            auto inMountainPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& inMountain = *inMountainPtr;
            floodFillFindOrphan(cond, { 0,0 }, inMountain);
            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    if (inMountain[x][y] == true) writeProphecy(x, y, 0, chunkType::mountain);
                }
            }
        }



        //강 생성
        /*
        * https://arxiv.org/abs/1511.04463
        1: Let Open be a priority queue
        2: Let Closed have the same dimensions as DEM
        3: Let Closed be initialized to false
        4: for all c on the edges of DEM do
        5:  Push c onto Open with priority DEM (c)
        6:  Closed(c) ← true
        7: while Open is not empty do
        8:  c ← pop(Open)
        9:  for all neighbors n of c do
        10:  if Closed(n) then repeat loop
        11:  DEM(n) ← max(DEM(n), DEM(c))
        12:  Closed(n) ← true
        13:  Push n onto Open with priority DEM (n)
        */

        {
            filledHeightMap = heightMap;

            std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> openQue;

            auto closeQueMapPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& closeQueMap = *closeQueMapPtr;

            openQue.push({ filledHeightMap[0][0],{ 0,0 } });
            closeQueMap[0][0] = true;
            while (openQue.empty() == false)
            {
                Point2 c = openQue.top().second;
                openQue.pop();

                std::vector<dir16> dirVec = { dir16::dir0, dir16::dir2, dir16::dir4, dir16::dir6 };
                for (dir16 dir : dirVec)
                {
                    int dx = dir2Coord(dir).x;
                    int dy = dir2Coord(dir).y;
                    Point2 n = { c.x + dx, c.y + dy };
                    if (n.x < 0 || n.y < 0 || n.x >= WORLD_DATA_SIZE || n.y >= WORLD_DATA_SIZE) continue;
                    if (closeQueMap[n.x][n.y] == true) continue;
                    //현재 위치를 미세하게 높게 설정
                    //nearafter 쓰면 인텔리센스 망가집니다요 26년 8월 17일
                    filledHeightMap[n.x][n.y] = std::max(filledHeightMap[n.x][n.y], filledHeightMap[c.x][c.y] + randFloat(0.000001f, 0.000005f));
                    closeQueMap[n.x][n.y] = true;
                    openQue.push({ filledHeightMap[n.x][n.y],n });

                }
            }

            std::vector<std::pair<float, Point2>> sortedHeight;
            sortedHeight.reserve(WORLD_DATA_SIZE * WORLD_DATA_SIZE);
            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    sortedHeight.push_back({ filledHeightMap[x][y], {x,y} });
                }
            }
            std::sort(sortedHeight.begin(), sortedHeight.end(), std::greater<>{});

            auto riverScorePtr = std::make_unique< std::array<std::array<int, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& riverScore = *riverScorePtr;
            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    riverScore[x][y] = 1;
                }
            }

            //{ dir16::dir0, dir16::dir2, dir16::dir4, dir16::dir6 };
            //{ dir16::dir0, dir16::dir1, dir16::dir2, dir16::dir3, dir16::dir4, dir16::dir5, dir16::dir6, dir16::dir7 };

            for (auto i : sortedHeight)
            {
                Point2 c = i.second;
                float lowScore = 99.0f;
                dir16 lowScoreDir = dir16::none;
                Point2 lowScoreCoord;
                std::vector<dir16> dirVec = { dir16::dir0, dir16::dir2, dir16::dir4, dir16::dir6 };;
                std::shuffle(dirVec.begin(), dirVec.end(), gen);
                for (dir16 dir : dirVec)
                {
                    int dx = dir2Coord(dir).x;
                    int dy = dir2Coord(dir).y;
                    Point2 n = { c.x + dx, c.y + dy };

                    if (n.x < 0 || n.y < 0 || n.x >= WORLD_DATA_SIZE || n.y >= WORLD_DATA_SIZE) continue;

                    if (filledHeightMap[n.x][n.y] < lowScore)
                    {
                        lowScore = filledHeightMap[n.x][n.y];
                        lowScoreDir = dir;
                        lowScoreCoord = n;
                    }
                }
                riverScore[lowScoreCoord.x][lowScoreCoord.y] += riverScore[c.x][c.y];
            }

            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    if (riverScore[x][y] > 1500)
                    {
                        if (getProphecy(x, y, 0) != chunkType::deepSea && getProphecy(x, y, 0) != chunkType::shallowSea)
                        {
                            writeProphecy(x, y, 0, chunkType::river);
                        }
                    }
                }
            }


            /////////////////호수 만들기////////////////////////////////////////////////////////////
            auto waterDepthPtr = std::make_unique < std::vector<std::pair<float, Point2>>>();
            auto& waterDepth = *waterDepthPtr;

            auto visitedGridPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
            auto& visitedGrid = *visitedGridPtr;

            for (int y = 0; y < WORLD_DATA_SIZE; y++)
            {
                for (int x = 0; x < WORLD_DATA_SIZE; x++)
                {
                    waterDepth.push_back({ filledHeightMap[x][y] - heightMap[x][y],{x,y} });
                }
            }

            std::sort(waterDepth.begin(), waterDepth.end(), std::greater<>{});

            constexpr float MINIMUM_DEPTH = 0.07f;
            constexpr float EXTENTION_DEPTH = 0.0696f;


            for (auto elem : waterDepth)
            {
                Point2 c = elem.second;
                if (getProphecy(c.x, c.y, 0) != chunkType::deepSea 
                    && getProphecy(c.x, c.y, 0) != chunkType::shallowSea)
                {
                    if (elem.first < MINIMUM_DEPTH) break;
                    if (visitedGrid[c.x][c.y] == true) continue;

                    auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
                    auto& cond = *condPtr;

                    std::unordered_set<Point2> lake;

                    for (int y = 0; y < WORLD_DATA_SIZE; y++)
                    {
                        for (int x = 0; x < WORLD_DATA_SIZE; x++)
                        {
                            if (filledHeightMap[x][y] - heightMap[x][y] > EXTENTION_DEPTH)
                            {
                                if (visitedGrid[x][y] == false)
                                {
                                    cond[x][y] = true;
                                }
                            }
                        }
                    }

                    //플루드필로 호수로 만들기 시작
                    floodFill(cond, c, lake);

                    for (auto lakeElem : lake)
                    {
                        writeProphecy(lakeElem.x, lakeElem.y, 0, chunkType::lake);
                        visitedGrid[lakeElem.x][lakeElem.y] = true;
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
