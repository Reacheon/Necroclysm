#include<SDL3/SDL.h>

import WorldData;
import std;
import util;
import globalVar;

void WorldData::generateWorld(std::uint64_t inputSeed)
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      1단계 : 월드 생성 준비 및 펄린 노이즈 사전 생성
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    createNoiseMap.operator() < 100 > (desertNoise);





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      2단계 : 고도맵을 이용한 바다와 땅, 산맥, 숲 지형 생성
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int y = 0; y < WORLD_DATA_SIZE; y++)
    {
        for (int x = 0; x < WORLD_DATA_SIZE; x++)
        {
            int dx = std::abs(x - WORLD_DATA_SIZE / 2);
            int dy = std::abs(y - WORLD_DATA_SIZE / 2);

            float penalty = std::min(1.0f, (float)std::sqrt(dx * dx + dy * dy) / (float)(WORLD_DATA_SIZE / 2));
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
            else if (heightMap[x][y] > -0.2f) writeProphecy(x, y, 0, chunkType::shallowSea);
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





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      3단계 : 극지방 생성
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int y = 0; y < WORLD_DATA_SIZE; y++)
    {
        for (int x = 0; x < WORLD_DATA_SIZE; x++)
        {
            constexpr float ELEV_CONST = 1.05;
            constexpr float LATITUDE_CONST = 0.2;

            tempMap[x][y] = ELEV_CONST * (static_cast<float>(WORLD_DATA_SIZE - y) / static_cast<float>(WORLD_DATA_SIZE));
            tempMap[x][y] += LATITUDE_CONST * heightMap[x][y];

            if (tempMap[x][y] > 0.7f)
            {
                if (getProphecy(x, y, 0) == chunkType::dirt)
                {
                    writeProphecy(x, y, 0, chunkType::snow);
                }
                //나중에 산의 경우 snowMountain으로 바꾸는 코드를 넣어볼 것
            }
        }
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      4단계 : 강과 호수 생성
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

                std::unordered_set<Point2, Point2::Hash> lake;

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






    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      5,1단계 : 사막 및 사막도시 배치
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<Point2> cityCoreVec;
    bool cityGenerated = false;
    int cityNumber = 0;
    int loopCount = 0;


    //사막 도시 배치
    Point2 desertCityPos;
    while (1)
    {
        loopCount++;
        errorBox(loopCount > 50000, L"사막 도시 다트 찍기가 잘 안 된다... 루프 카운트가 5만을 초과했다.");

        int randX = randomRange(0, WORLD_DATA_SIZE - 1);
        int randY = randomRange((WORLD_DATA_SIZE - 1) / 2, 84 * (WORLD_DATA_SIZE - 1) / 100);
        //위도 15도 ~ 45도 정도로

        if (randX < 10 || randX >= WORLD_DATA_SIZE - 10) continue;
        if (randY < 10 || randY >= WORLD_DATA_SIZE - 10) continue;


        if (getProphecy(randX, randY, 0) == chunkType::dirt)
        {
            bool cleanDistrict = true;
            constexpr int CLEAN_RANGE = 50;
            for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
            {
                for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
                {
                    if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy)
                        && (getProphecy(randX + dx, randY + dy, 0) != chunkType::dirt))
                        cleanDistrict = false;
                }
            }

            bool noNearbyCity = true;
            constexpr int NO_CITY_DIAMETER = 100;
            for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
            {
                for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
                {
                    if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(randX + dx, randY + dy, 0) == chunkType::city) noNearbyCity = false;
                }
            }

            if (cleanDistrict == false || noNearbyCity == false) continue;
            writeProphecy(randX, randY, 0, chunkType::city);
            cityTypeMap[{randX, randY, 0}] = cityType::desert;
            cityCoreVec.push_back({ randX, randY });
            desertCityPos = { randX, randY };
            cityNumber++;
            break;
        }
    }

    //사막 도시 주변을 사막으로 만들기
    {
        int cursorX = desertCityPos.x;
        int cursorY = desertCityPos.y;
        int desertMaxSize = randomRange(1000, 5000);
        int desertCurrentSize = 1;

        std::vector<Point2> desertPoints;

        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);


        auto desertChunkCond = [&](int x, int y) -> bool
            {
                return getProphecy(x, y, 0) == chunkType::dirt;
            };

        if (desertChunkCond(desertCityPos.x + 1, desertCityPos.y)) frontier.push({ randomKey,{ desertCityPos.x + 1,desertCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (desertChunkCond(desertCityPos.x - 1, desertCityPos.y)) frontier.push({ randomKey,{ desertCityPos.x - 1,desertCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (desertChunkCond(desertCityPos.x, desertCityPos.y + 1)) frontier.push({ randomKey,{ desertCityPos.x ,desertCityPos.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (desertChunkCond(desertCityPos.x, desertCityPos.y - 1)) frontier.push({ randomKey,{ desertCityPos.x ,desertCityPos.y - 1} });

        while (frontier.empty() == false && desertCurrentSize < desertMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (desertChunkCond(targetX, targetY) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::desert);
            desertPoints.push_back({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (desertChunkCond(targetX + 1, targetY)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (desertChunkCond(targetX - 1, targetY)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (desertChunkCond(targetX, targetY + 1)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (desertChunkCond(targetX, targetY - 1)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            desertCurrentSize++;
        }

        for (auto elem : desertPoints)
        {
            for (int ddx = -4; ddx <= 4; ddx++)
            {
                for (int ddy = -4; ddy <= 4; ddy++)
                {
                    if (desertChunkCond(elem.x + ddx, elem.y + ddy))
                    {
                        writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::desert);
                    }
                }
            }
        }
    }

    //사막도시 확장
    {
        int cursorX = desertCityPos.x;
        int cursorY = desertCityPos.y;
        int cityMaxSize = randomRange(150, 300);
        int cityCurrentSize = 1;

        std::vector<Point2> cityPoints;

        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);

        cityType tgtCityType = cityTypeMap[{desertCityPos.x, desertCityPos.y, 0}];

        auto cityChunkCond = [&](int x, int y, cityType inputTgtCityType) -> bool
            {
                if (isCircle(21, x - desertCityPos.x, y - desertCityPos.y) == false) return false;
                return getProphecy(x, y, 0) == chunkType::dirt
                    || getProphecy(x, y, 0) == chunkType::snow
                    || getProphecy(x, y, 0) == chunkType::desert
                    || getProphecy(x, y, 0) == chunkType::forest;
                if (inputTgtCityType == cityType::normal) return getProphecy(x, y, 0) == chunkType::dirt;
                else if (inputTgtCityType == cityType::snow) return getProphecy(x, y, 0) == chunkType::snow;
                else if (inputTgtCityType == cityType::desert) return getProphecy(x, y, 0) == chunkType::desert;
                else errorBox(L"이상한 시티 타입이 cityChunkCond에 입력되었다.");
            };

        if (cityChunkCond(desertCityPos.x + 1, desertCityPos.y, tgtCityType)) frontier.push({ randomKey,{ desertCityPos.x + 1,desertCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(desertCityPos.x - 1, desertCityPos.y, tgtCityType)) frontier.push({ randomKey,{ desertCityPos.x - 1,desertCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(desertCityPos.x, desertCityPos.y + 1, tgtCityType)) frontier.push({ randomKey,{ desertCityPos.x ,desertCityPos.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(desertCityPos.x, desertCityPos.y - 1, tgtCityType)) frontier.push({ randomKey,{ desertCityPos.x ,desertCityPos.y - 1} });

        while (frontier.empty() == false && cityCurrentSize < cityMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (cityChunkCond(targetX, targetY, tgtCityType) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::city);
            cityTypeMap[{targetX, targetY, 0}] = tgtCityType;
            cityPoints.push_back({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX + 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX - 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY + 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY - 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            cityCurrentSize++;
        }

        for (auto elem : cityPoints)
        {
            //사막 도시의 경우 주변 엣지를 사막으로 만드는 기능 추가할 것

            for (int ddx = -2; ddx <= 2; ddx++)
            {
                for (int ddy = -2; ddy <= 2; ddy++)
                {
                    if (cityChunkCond(elem.x + ddx, elem.y + ddy, tgtCityType))
                    {
                        writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::city);
                        cityTypeMap[{elem.x + ddx, elem.y + ddy, 0}] = tgtCityType;

                        if (std::abs(ddx) == 2 || std::abs(ddy) == 2)
                        {
                            std::vector<Point2> nearbyDel = { {1,0},{0,-1},{-1,0},{0,1},{1,-1},{-1,1},{-1,-1},{1,1},{2,0},{0,-2},{-2,0},{0,2} };
                            for (auto targetDel : nearbyDel)
                            {
                                if (getProphecy(elem.x + ddx + targetDel.x, elem.y + ddy + targetDel.y, 0) == chunkType::dirt)
                                {
                                    writeProphecy(elem.x + ddx + targetDel.x, elem.y + ddy + targetDel.y, 0, chunkType::desert);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    //사막도시 확장 후 내부 고립셀 제거
    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) != chunkType::city) cond[x][y] = true;
                else cond[x][y] = false;
            }
        }
        auto internalCityPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& internalCity = *internalCityPtr;
        floodFillFindOrphan(cond, { 0,0 }, internalCity);
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (internalCity[x][y] == true)
                {
                    writeProphecy(x, y, 0, chunkType::city);
                    cityTypeMap[{x, y,0}] = cityType::desert;
                }
            }
        }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      5,2단계 : 화산 및 화산도시 배치
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Point2 volcanoPos;
    while (1)
    {
        loopCount++;
        errorBox(loopCount > 50000, L"화산섬 다트 찍기가 잘 안 된다... 루프 카운트가 5만을 초과했다.");

        int randX = randomRange(200, WORLD_DATA_SIZE - 1 - 200);
        int randY = randomRange(4 * (WORLD_DATA_SIZE - 1) / 10, WORLD_DATA_SIZE - 1 - 200);

        if (randX < 10 || randX >= WORLD_DATA_SIZE - 10) continue;
        if (randY < 10 || randY >= WORLD_DATA_SIZE - 10) continue;

        if (getProphecy(randX, randY, 0) == chunkType::deepSea || getProphecy(randX, randY, 0) == chunkType::shallowSea)
        {
            bool cleanDistrict = true;
            constexpr int CLEAN_RANGE = 100;
            for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
            {
                for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
                {
                    if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy)
                        && getProphecy(randX + dx, randY + dy, 0) != chunkType::deepSea
                        && getProphecy(randX + dx, randY + dy, 0) != chunkType::shallowSea)
                        cleanDistrict = false;
                }
            }

            bool noNearbyCity = true;
            constexpr int NO_CITY_DIAMETER = 100;
            for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
            {
                for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
                {
                    if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(randX + dx, randY + dy, 0) == chunkType::city) noNearbyCity = false;
                }
            }

            if (cleanDistrict == false || noNearbyCity == false) continue;
            writeProphecy(randX, randY, 0, chunkType::volcanicLand);
            volcanoPos = { randX, randY };
            break;
        }
    }

    //화산섬 확장
    {
        int cursorX = volcanoPos.x;
        int cursorY = volcanoPos.y;




        int volcanoMaxSize = randomRange(3000, 5000);
        int volcanoCurrentSize = 1;

        std::vector<Point2> volcanoPoints;

        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);


        auto volcanoChunkCond = [&](int x, int y) -> bool
            {
                return getProphecy(x, y, 0) == chunkType::deepSea
                    || getProphecy(x, y, 0) == chunkType::shallowSea;
            };

        if (volcanoChunkCond(volcanoPos.x + 1, volcanoPos.y)) frontier.push({ randomKey,{ volcanoPos.x + 1,volcanoPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (volcanoChunkCond(volcanoPos.x - 1, volcanoPos.y)) frontier.push({ randomKey,{ volcanoPos.x - 1,volcanoPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (volcanoChunkCond(volcanoPos.x, volcanoPos.y + 1)) frontier.push({ randomKey,{ volcanoPos.x ,volcanoPos.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (volcanoChunkCond(volcanoPos.x, volcanoPos.y - 1)) frontier.push({ randomKey,{ volcanoPos.x ,volcanoPos.y - 1} });

        while (frontier.empty() == false && volcanoCurrentSize < volcanoMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (volcanoChunkCond(targetX, targetY) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::volcanicLand);
            volcanoPoints.push_back({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX + 1, targetY)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX - 1, targetY)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX, targetY + 1)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX, targetY - 1)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            volcanoCurrentSize++;
        }

        for (auto elem : volcanoPoints)
        {
            for (int ddx = -3; ddx <= 3; ddx++)
            {
                for (int ddy = -3; ddy <= 3; ddy++)
                {
                    if (std::abs(ddx) <= 3 && std::abs(ddy) <= 3)
                    {
                        if (volcanoChunkCond(elem.x + ddx, elem.y + ddy))
                        {
                            writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::volcanicLand);
                        }
                    }

                }
            }
        }

    }

    //화산 만들기
    {
        int cursorX = volcanoPos.x;
        int cursorY = volcanoPos.y;
        int volcanoMaxSize = randomRange(150, 350);
        int volcanoCurrentSize = 1;

        std::vector<Point2> volcanoPoints;

        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);

        auto volcanoChunkCond = [&](int x, int y) -> bool
            {
                return getProphecy(x, y, 0) == chunkType::volcanicLand;
            };

        if (volcanoChunkCond(volcanoPos.x + 1, volcanoPos.y)) frontier.push({ randomKey,{ volcanoPos.x + 1,volcanoPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (volcanoChunkCond(volcanoPos.x - 1, volcanoPos.y)) frontier.push({ randomKey,{ volcanoPos.x - 1,volcanoPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (volcanoChunkCond(volcanoPos.x, volcanoPos.y + 1)) frontier.push({ randomKey,{ volcanoPos.x ,volcanoPos.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (volcanoChunkCond(volcanoPos.x, volcanoPos.y - 1)) frontier.push({ randomKey,{ volcanoPos.x ,volcanoPos.y - 1} });

        while (frontier.empty() == false && volcanoCurrentSize < volcanoMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (volcanoChunkCond(targetX, targetY) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::volcano);
            volcanoPoints.push_back({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX + 1, targetY)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX - 1, targetY)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX, targetY + 1)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (volcanoChunkCond(targetX, targetY - 1)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            volcanoCurrentSize++;
        }

        for (auto elem : volcanoPoints)
        {
            for (int ddx = -8; ddx <= 8; ddx++)
            {
                for (int ddy = -8; ddy <= 8; ddy++)
                {
                    if (std::abs(ddx) <= 3 && std::abs(ddy) <= 3)
                    {
                        if (volcanoChunkCond(elem.x + ddx, elem.y + ddy) && isCircle(4, ddx, ddy))
                        {
                            writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::volcano);
                        }
                    }
                    else if (getProphecy(elem.x + ddx, elem.y + ddy, 0) == chunkType::deepSea || getProphecy(elem.x + ddx, elem.y + ddy, 0) == chunkType::shallowSea)
                    {
                        writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::volcanicLand);
                    }
                }
            }
        }
    }

    //화산섬 주변을 얕은 물로 변경
    {
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) == chunkType::volcanicLand)
                {
                    bool waterSide = false;
                    std::vector<Point2> dirSet = { { 1, 0 }, {1,-1}, { 0,-1 },{-1,-1}, { -1,0 },{-1,1}, { 0,1 },{1,1} };
                    for (Point2 del : dirSet)
                    {
                        if (getProphecy(x + del.x, y + del.y, 0) == chunkType::shallowSea || getProphecy(x + del.x, y + del.y, 0) == chunkType::deepSea)
                        {
                            waterSide = true;
                            break;
                        }
                    }

                    if (waterSide)
                    {
                        for (int ddx = -7; ddx <= 7; ddx++)
                        {
                            for (int ddy = -7; ddy <= 7; ddy++)
                            {
                                if (getProphecy(x + ddx, y + ddy, 0) == chunkType::deepSea && isCircle(static_cast<int>(7.0f*(noiseMap30[x+ddx][y+ddy]+1.0)), ddx, ddy))
                                {
                                    writeProphecy(x + ddx, y + ddy, 0, chunkType::shallowSea);
                                }
                            }
                        }
                    }

                }
            }
        }


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
                    if (inlandSea[x][y] == true) writeProphecy(x, y, 0, chunkType::volcanicLand);
                }
            }
        }
    }

    Point2 volcanoCityPos = { 0,0 };
    //화산도시 만들기
    {
        std::unordered_set<Point2,Point2::Hash> volcanicLandSet;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x,y,0) == chunkType::volcanicLand)
                {
                    volcanicLandSet.insert({ x,y });
                }
            }
        }

        for (auto elem : volcanicLandSet)
        {
            bool cleanDistrict = true;
            constexpr int CLEAN_RANGE = 30;
            for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
            {
                for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
                {
                    if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy)
                        && (getProphecy(elem.x + dx, elem.y + dy, 0) != chunkType::volcanicLand))
                        cleanDistrict = false;
                }
            }

            bool noNearbyCity = true;
            constexpr int NO_CITY_DIAMETER = 100;
            for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
            {
                for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
                {
                    if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(elem.x + dx, elem.y + dy, 0) == chunkType::city) noNearbyCity = false;
                }
            }

            if (cleanDistrict == false || noNearbyCity == false) continue;
            writeProphecy(elem.x, elem.y, 0, chunkType::city);
            cityTypeMap[{elem.x, elem.y, 0}] = cityType::volcano;
            cityCoreVec.push_back({ elem.x, elem.y });
            volcanoCityPos = { elem.x, elem.y };
            cityNumber++;
            break;
        }


        if (volcanoCityPos != Point2{0, 0})
        {
            int cursorX = volcanoCityPos.x;
            int cursorY = volcanoCityPos.y;
            int cityMaxSize = randomRange(120, 250);
            int cityCurrentSize = 1;

            std::vector<Point2> cityPoints;

            std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
            float randomKey = randomRangeFloat(0.0, 1.0);

            cityType tgtCityType = cityTypeMap[{volcanoCityPos.x, volcanoCityPos.y, 0}];

            auto cityChunkCond = [&](int x, int y, cityType inputTgtCityType) -> bool
                {
                    if (isCircle(21, x - volcanoCityPos.x, y - volcanoCityPos.y) == false) return false;
                    return getProphecy(x, y, 0) == chunkType::volcanicLand;
                    if (inputTgtCityType == cityType::normal) return getProphecy(x, y, 0) == chunkType::dirt;
                    else if (inputTgtCityType == cityType::snow) return getProphecy(x, y, 0) == chunkType::snow;
                    else if (inputTgtCityType == cityType::desert) return getProphecy(x, y, 0) == chunkType::desert;
                    else errorBox(L"이상한 시티 타입이 cityChunkCond에 입력되었다.");
                };

            if (cityChunkCond(volcanoCityPos.x + 1, volcanoCityPos.y, tgtCityType)) frontier.push({ randomKey,{ volcanoCityPos.x + 1,volcanoCityPos.y } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(volcanoCityPos.x - 1, volcanoCityPos.y, tgtCityType)) frontier.push({ randomKey,{ volcanoCityPos.x - 1,volcanoCityPos.y } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(volcanoCityPos.x, volcanoCityPos.y + 1, tgtCityType)) frontier.push({ randomKey,{ volcanoCityPos.x ,volcanoCityPos.y + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(volcanoCityPos.x, volcanoCityPos.y - 1, tgtCityType)) frontier.push({ randomKey,{ volcanoCityPos.x ,volcanoCityPos.y - 1} });

            while (frontier.empty() == false && cityCurrentSize < cityMaxSize)
            {
                int targetX = frontier.top().second.x;
                int targetY = frontier.top().second.y;
                frontier.pop();
                if (cityChunkCond(targetX, targetY, tgtCityType) == false) continue;

                writeProphecy(targetX, targetY, 0, chunkType::city);
                cityTypeMap[{targetX, targetY, 0}] = tgtCityType;
                cityPoints.push_back({ targetX,targetY });

                float randomKey = randomRangeFloat(0.0, 1.0);
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX + 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX + 1,targetY } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX - 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX - 1,targetY } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX, targetY + 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX, targetY - 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

                cityCurrentSize++;
            }

            for (auto elem : cityPoints)
            {
                for (int ddx = -4; ddx <= 4; ddx++)
                {
                    for (int ddy = -4; ddy <= 4; ddy++)
                    {
                        if (std::abs(ddx) <= 2 && std::abs(ddy) <= 2)
                        {
                            if (cityChunkCond(elem.x + ddx, elem.y + ddy, tgtCityType))
                            {
                                writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::city);
                                cityTypeMap[{elem.x + ddx, elem.y + ddy, 0}] = tgtCityType;
                            }
                        }
                        else if (getProphecy(elem.x + ddx, elem.y + ddy, 0) == chunkType::deepSea || getProphecy(elem.x + ddx, elem.y + ddy, 0) == chunkType::shallowSea)
                        {
                            writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::volcanicLand);
                        }
                    }
                }
            }

        }
    }



    //화산도시 확장 후 내부 고립셀 제거
    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) != chunkType::city) cond[x][y] = true;
                else cond[x][y] = false;
            }
        }
        auto internalCityPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& internalCity = *internalCityPtr;
        floodFillFindOrphan(cond, { 0,0 }, internalCity);
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (internalCity[x][y] == true)
                {
                    writeProphecy(x, y, 0, chunkType::city);
                    cityTypeMap[{x, y,0}] = cityType::volcano;
                }
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      5,3단계 : 정글 및 정글도시 배치
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Point2 jungleCityPos = { 0,0 };
    {
        //정글 도시 배치
        while (1)
        {
            loopCount++;
            errorBox(loopCount > 50000, L"정글 도시 다트 찍기가 잘 안 된다... 루프 카운트가 5만을 초과했다.");

            int randX = randomRange(0, WORLD_DATA_SIZE - 1);
            int randY = randomRange((WORLD_DATA_SIZE - 1) / 2, 84 * (WORLD_DATA_SIZE - 1) / 100);
            //위도 15도 ~ 45도 정도로

            if (randX < 10 || randX >= WORLD_DATA_SIZE - 10) continue;
            if (randY < 10 || randY >= WORLD_DATA_SIZE - 10) continue;

            float desertDist = std::sqrt((randX - desertCityPos.x)*(randX - desertCityPos.x) + (randY - desertCityPos.y) * (randY - desertCityPos.y));
            if (desertDist < 200) continue;


            if (getProphecy(randX, randY, 0) == chunkType::dirt)
            {
                bool cleanDistrict = true;
                constexpr int CLEAN_RANGE = 50;
                for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
                {
                    for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
                    {
                        if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy)
                            && (getProphecy(randX + dx, randY + dy, 0) != chunkType::dirt))
                            cleanDistrict = false;
                    }
                }

                bool noNearbyCity = true;
                constexpr int NO_CITY_DIAMETER = 200;
                for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
                {
                    for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
                    {
                        if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(randX + dx, randY + dy, 0) == chunkType::city) noNearbyCity = false;
                    }
                }

                if (cleanDistrict == false || noNearbyCity == false) continue;
                writeProphecy(randX, randY, 0, chunkType::city);
                cityTypeMap[{randX, randY, 0}] = cityType::jungle;
                cityCoreVec.push_back({ randX, randY });
                jungleCityPos = { randX, randY };
                cityNumber++;
                break;
            }
        }

        //정글 도시 주변을 정글로 만들기
        {
            int cursorX = jungleCityPos.x;
            int cursorY = jungleCityPos.y;
            int jungleMaxSize = randomRange(1500, 3000);
            int jungleCurrentSize = 1;

            std::vector<Point2> junglePoints;

            std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
            float randomKey = randomRangeFloat(0.0, 1.0);


            auto jungleChunkCond = [&](int x, int y) -> bool
                {
                    return getProphecy(x, y, 0) == chunkType::dirt;
                };

            if (jungleChunkCond(jungleCityPos.x + 1, jungleCityPos.y)) frontier.push({ randomKey,{ jungleCityPos.x + 1,jungleCityPos.y } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (jungleChunkCond(jungleCityPos.x - 1, jungleCityPos.y)) frontier.push({ randomKey,{ jungleCityPos.x - 1,jungleCityPos.y } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (jungleChunkCond(jungleCityPos.x, jungleCityPos.y + 1)) frontier.push({ randomKey,{ jungleCityPos.x ,jungleCityPos.y + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (jungleChunkCond(jungleCityPos.x, jungleCityPos.y - 1)) frontier.push({ randomKey,{ jungleCityPos.x ,jungleCityPos.y - 1} });

            while (frontier.empty() == false && jungleCurrentSize < jungleMaxSize)
            {
                int targetX = frontier.top().second.x;
                int targetY = frontier.top().second.y;
                frontier.pop();
                if (jungleChunkCond(targetX, targetY) == false) continue;

                writeProphecy(targetX, targetY, 0, chunkType::jungle);
                junglePoints.push_back({ targetX,targetY });

                float randomKey = randomRangeFloat(0.0, 1.0);
                randomKey = randomRangeFloat(0.0, 1.0);
                if (jungleChunkCond(targetX + 1, targetY)) frontier.push({ randomKey,{ targetX + 1,targetY } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (jungleChunkCond(targetX - 1, targetY)) frontier.push({ randomKey,{ targetX - 1,targetY } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (jungleChunkCond(targetX, targetY + 1)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (jungleChunkCond(targetX, targetY - 1)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

                jungleCurrentSize++;
            }

            for (auto elem : junglePoints)
            {
                for (int ddx = -4; ddx <= 4; ddx++)
                {
                    for (int ddy = -4; ddy <= 4; ddy++)
                    {
                        if (jungleChunkCond(elem.x + ddx, elem.y + ddy))
                        {
                            writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::jungle);
                        }
                    }
                }
            }
        }


        if (jungleCityPos != Point2{ 0, 0 })
        {
            int cursorX = jungleCityPos.x;
            int cursorY = jungleCityPos.y;
            int cityMaxSize = randomRange(120, 250);
            int cityCurrentSize = 1;

            std::vector<Point2> cityPoints;

            std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
            float randomKey = randomRangeFloat(0.0, 1.0);

            cityType tgtCityType = cityTypeMap[{jungleCityPos.x, jungleCityPos.y, 0}];

            auto cityChunkCond = [&](int x, int y, cityType inputTgtCityType) -> bool
                {
                    if (isCircle(21, x - jungleCityPos.x, y - jungleCityPos.y) == false) return false;
                    return getProphecy(x, y, 0) == chunkType::jungle;
                    if (inputTgtCityType == cityType::normal) return getProphecy(x, y, 0) == chunkType::dirt;
                    else if (inputTgtCityType == cityType::snow) return getProphecy(x, y, 0) == chunkType::snow;
                    else if (inputTgtCityType == cityType::desert) return getProphecy(x, y, 0) == chunkType::desert;
                    else errorBox(L"이상한 시티 타입이 cityChunkCond에 입력되었다.");
                };

            if (cityChunkCond(jungleCityPos.x + 1, jungleCityPos.y, tgtCityType)) frontier.push({ randomKey,{ jungleCityPos.x + 1,jungleCityPos.y } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(jungleCityPos.x - 1, jungleCityPos.y, tgtCityType)) frontier.push({ randomKey,{ jungleCityPos.x - 1,jungleCityPos.y } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(jungleCityPos.x, jungleCityPos.y + 1, tgtCityType)) frontier.push({ randomKey,{ jungleCityPos.x ,jungleCityPos.y + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(jungleCityPos.x, jungleCityPos.y - 1, tgtCityType)) frontier.push({ randomKey,{ jungleCityPos.x ,jungleCityPos.y - 1} });

            while (frontier.empty() == false && cityCurrentSize < cityMaxSize)
            {
                int targetX = frontier.top().second.x;
                int targetY = frontier.top().second.y;
                frontier.pop();
                if (cityChunkCond(targetX, targetY, tgtCityType) == false) continue;

                writeProphecy(targetX, targetY, 0, chunkType::city);
                cityTypeMap[{targetX, targetY, 0}] = tgtCityType;
                cityPoints.push_back({ targetX,targetY });

                float randomKey = randomRangeFloat(0.0, 1.0);
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX + 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX + 1,targetY } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX - 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX - 1,targetY } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX, targetY + 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
                randomKey = randomRangeFloat(0.0, 1.0);
                if (cityChunkCond(targetX, targetY - 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

                cityCurrentSize++;
            }

            for (auto elem : cityPoints)
            {
                for (int ddx = -4; ddx <= 4; ddx++)
                {
                    for (int ddy = -4; ddy <= 4; ddy++)
                    {
                        if (std::abs(ddx) <= 2 && std::abs(ddy) <= 2)
                        {
                            if (cityChunkCond(elem.x + ddx, elem.y + ddy, tgtCityType))
                            {
                                writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::city);
                                cityTypeMap[{elem.x + ddx, elem.y + ddy, 0}] = tgtCityType;
                            }
                        }
                        else
                        {
                            if (getProphecy(elem.x + ddx, elem.y + ddy,0) == chunkType::dirt)
                            {
                                writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::jungle);
                            }
                        }
                    }
                }
            }

        }
    }


    //정글도시 확장 후 내부 고립셀 제거
    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) != chunkType::city) cond[x][y] = true;
                else cond[x][y] = false;
            }
        }
        auto internalCityPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& internalCity = *internalCityPtr;
        floodFillFindOrphan(cond, { 0,0 }, internalCity);
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (internalCity[x][y] == true)
                {
                    writeProphecy(x, y, 0, chunkType::city);
                    cityTypeMap[{x, y,0}] = cityType::jungle;
                }
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      5.4단계 : 설원도시 배치
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::unordered_set<Point2, Point2::Hash> snowPoints;
    for (int y = 0; y < WORLD_DATA_SIZE; y++)
    {
        for (int x = 0; x < WORLD_DATA_SIZE; x++)
        {
            if (getProphecy(x, y, 0) == chunkType::snow) snowPoints.insert({ x,y });
        }
    }

    Point2 snowCityPos = { 0,0 };
    for (auto elem : snowPoints)
    {
        bool cleanDistrict = true;
        constexpr int CLEAN_RANGE = 50;
        for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
        {
            for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
            {
                if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy) && getProphecy(elem.x + dx, elem.y + dy, 0) != chunkType::snow)
                    cleanDistrict = false;
            }
        }

        bool noNearbyCity = true;
        constexpr int NO_CITY_DIAMETER = 100;
        for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
        {
            for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
            {
                if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(elem.x + dx, elem.y + dy, 0) == chunkType::city) noNearbyCity = false;
            }
        }

        if (cleanDistrict == false || noNearbyCity == false) continue;
        writeProphecy(elem.x, elem.y, 0, chunkType::city);
        cityTypeMap[{elem.x, elem.y, 0}] = cityType::snow;
        cityCoreVec.push_back({ elem.x, elem.y });
        snowCityPos = { elem.x, elem.y };
        break;
    }

    if (snowCityPos != Point2{ 0, 0 })
    {
        int cursorX = snowCityPos.x;
        int cursorY = snowCityPos.y;
        int cityMaxSize = randomRange(120, 250);
        int cityCurrentSize = 1;

        std::vector<Point2> cityPoints;

        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);

        cityType tgtCityType = cityTypeMap[{snowCityPos.x, snowCityPos.y, 0}];

        auto cityChunkCond = [&](int x, int y, cityType inputTgtCityType) -> bool
            {
                if (isCircle(21, x - snowCityPos.x, y - snowCityPos.y) == false) return false;
                return getProphecy(x, y, 0) == chunkType::snow;
                if (inputTgtCityType == cityType::normal) return getProphecy(x, y, 0) == chunkType::dirt;
                else if (inputTgtCityType == cityType::snow) return getProphecy(x, y, 0) == chunkType::snow;
                else if (inputTgtCityType == cityType::desert) return getProphecy(x, y, 0) == chunkType::desert;
                else errorBox(L"이상한 시티 타입이 cityChunkCond에 입력되었다.");
            };

        if (cityChunkCond(snowCityPos.x + 1, snowCityPos.y, tgtCityType)) frontier.push({ randomKey,{ snowCityPos.x + 1,snowCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(snowCityPos.x - 1, snowCityPos.y, tgtCityType)) frontier.push({ randomKey,{ snowCityPos.x - 1,snowCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(snowCityPos.x, snowCityPos.y + 1, tgtCityType)) frontier.push({ randomKey,{ snowCityPos.x ,snowCityPos.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(snowCityPos.x, snowCityPos.y - 1, tgtCityType)) frontier.push({ randomKey,{ snowCityPos.x ,snowCityPos.y - 1} });

        while (frontier.empty() == false && cityCurrentSize < cityMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (cityChunkCond(targetX, targetY, tgtCityType) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::city);
            cityTypeMap[{targetX, targetY, 0}] = tgtCityType;
            cityPoints.push_back({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX + 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX - 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY + 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY - 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            cityCurrentSize++;
        }

        for (auto elem : cityPoints)
        {
            for (int ddx = -4; ddx <= 4; ddx++)
            {
                for (int ddy = -4; ddy <= 4; ddy++)
                {
                    if (std::abs(ddx) <= 2 && std::abs(ddy) <= 2)
                    {
                        if (cityChunkCond(elem.x + ddx, elem.y + ddy, tgtCityType))
                        {
                            writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::city);
                            cityTypeMap[{elem.x + ddx, elem.y + ddy, 0}] = tgtCityType;
                        }
                    }
                    else
                    {
                        if (getProphecy(elem.x + ddx, elem.y + ddy, 0) == chunkType::dirt)
                        {
                            writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::snow);
                        }
                    }
                }
            }
        }

    }





    //설원도시 확장 후 내부 고립셀 제거
    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) != chunkType::city) cond[x][y] = true;
                else cond[x][y] = false;
            }
        }
        auto internalCityPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& internalCity = *internalCityPtr;
        floodFillFindOrphan(cond, { 0,0 }, internalCity);
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (internalCity[x][y] == true)
                {
                    writeProphecy(x, y, 0, chunkType::city);
                    cityTypeMap[{x, y,0}] = cityType::snow;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      5,5단계 : 항구도시
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::unordered_set<Point2,Point2::Hash> seaSide;
    for (int y = 1; y < WORLD_DATA_SIZE-1; y++)
    {
        for (int x = 1; x < WORLD_DATA_SIZE-1; x++)
        {
            if (getProphecy(x, y, 0) == chunkType::dirt)
            {
                bool hasSea = false;
                
                if (getProphecy(x - 1, y, 0) == chunkType::shallowSea) hasSea=true;
                if (getProphecy(x + 1, y, 0) == chunkType::shallowSea) hasSea = true;
                if (getProphecy(x, y - 1, 0) == chunkType::shallowSea) hasSea = true;
                if (getProphecy(x, y + 1, 0) == chunkType::shallowSea) hasSea = true;

                if (hasSea) seaSide.insert({ x,y });
            }
        }
    }

    Point2 portCityPos = { 0,0 };
    std::unordered_set<Point2, Point2::Hash> portCityPoints;
    for (auto portCandidate : seaSide)
    {

        int randX = portCandidate.x;
        int randY = portCandidate.y;

        if (randX < 10 || randX >= WORLD_DATA_SIZE - 10) continue;
        if (randY < 10 || randY >= WORLD_DATA_SIZE - 10) continue;


        if (getProphecy(randX, randY, 0) == chunkType::dirt)
        {
            bool cleanDistrict = true;
            constexpr int CLEAN_RANGE = 50;
            for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
            {
                for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
                {
                    if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy)
                        && (getProphecy(randX + dx, randY + dy, 0) != chunkType::dirt)
                        && (getProphecy(randX + dx, randY + dy, 0) != chunkType::shallowSea)
                        && (getProphecy(randX + dx, randY + dy, 0) != chunkType::deepSea))
                        cleanDistrict = false;
                }
            }

            bool noNearbyCity = true;
            constexpr int NO_CITY_DIAMETER = 100;
            for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
            {
                for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
                {
                    if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(randX + dx, randY + dy, 0) == chunkType::city) noNearbyCity = false;
                }
            }


            int dirtNumber = 0;
            constexpr int DIRT_FIND_RANGE = 26;
            for (int dx = -DIRT_FIND_RANGE / 2; dx <= DIRT_FIND_RANGE / 2; dx++)
            {
                for (int dy = -DIRT_FIND_RANGE / 2; dy <= DIRT_FIND_RANGE / 2; dy++)
                {
                    if (isCircle(DIRT_FIND_RANGE / 2 + 1, dx, dy)
                        && (getProphecy(randX + dx, randY + dy, 0) == chunkType::dirt))
                        dirtNumber++;
                }
            }


            if (cleanDistrict == false || noNearbyCity == false || dirtNumber < 100) continue;
            writeProphecy(randX, randY, 0, chunkType::city);
            cityTypeMap[{randX, randY, 0}] = cityType::port;
            portCityPos = { randX,randY };
            portCityPoints.insert({ randX,randY });
            cityCoreVec.push_back({ randX, randY });
            break;
        }
    }


    if (portCityPos != Point2{ 0,0 })
    {
        int cursorX = portCityPos.x;
        int cursorY = portCityPos.y;
        int cityMaxSize = randomRange(150, 300);
        int cityCurrentSize = 1;


        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);

        cityType tgtCityType = cityTypeMap[{portCityPos.x, portCityPos.y, 0}];

        auto cityChunkCond = [&](int x, int y, cityType inputTgtCityType) -> bool
            {
                if (isCircle(21, x - portCityPos.x, y - portCityPos.y) == false) return false;
                return getProphecy(x, y, 0) == chunkType::dirt
                    || getProphecy(x, y, 0) == chunkType::snow
                    || getProphecy(x, y, 0) == chunkType::desert
                    || getProphecy(x, y, 0) == chunkType::forest;
                errorBox(L"이상한 시티 타입이 cityChunkCond에 입력되었다.");
            };

        if (cityChunkCond(portCityPos.x + 1, portCityPos.y, tgtCityType)) frontier.push({ randomKey,{ portCityPos.x + 1,portCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(portCityPos.x - 1, portCityPos.y, tgtCityType)) frontier.push({ randomKey,{ portCityPos.x - 1,portCityPos.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(portCityPos.x, portCityPos.y + 1, tgtCityType)) frontier.push({ randomKey,{ portCityPos.x ,portCityPos.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(portCityPos.x, portCityPos.y - 1, tgtCityType)) frontier.push({ randomKey,{ portCityPos.x ,portCityPos.y - 1} });

        while (frontier.empty() == false && cityCurrentSize < cityMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (cityChunkCond(targetX, targetY, tgtCityType) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::city);
            cityTypeMap[{targetX, targetY, 0}] = tgtCityType;
            portCityPoints.insert({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX + 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX - 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY + 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY - 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            cityCurrentSize++;
        }

        auto tempPortCityPoints = portCityPoints;
        for (auto elem : tempPortCityPoints)
        {
            for (int ddx = -2; ddx <= 2; ddx++)
            {
                for (int ddy = -2; ddy <= 2; ddy++)
                {
                    if (cityChunkCond(elem.x + ddx, elem.y + ddy, tgtCityType))
                    {
                        writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::city);
                        cityTypeMap[{elem.x + ddx, elem.y + ddy, 0}] = tgtCityType;
                        portCityPoints.insert({ elem.x + ddx, elem.y + ddy });
                    }
                }
            }
        }

        
        //항구도시에서 바다와 접한 부분을 사각형에 가깝게 만들기
        int minX = 9999;
        int maxX = -1;
        int minY = 9999;
        int maxY = -1;
        for (auto elem : portCityPoints)
        {
            if (elem.x < minX) minX = elem.x;
            if (elem.x > maxX) maxX = elem.x;
            if (elem.y < minY) minY = elem.y;
            if (elem.y > maxY) maxY = elem.y;
        }

        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (x >= minX && x <= maxX)
                {
                    if (y >= minY && y <= maxY)
                    {
                        if (getProphecy(x, y, 0) == chunkType::city
                            || getProphecy(x, y, 0) == chunkType::shallowSea
                            || getProphecy(x, y, 0) == chunkType::deepSea)
                            cond[x][y] = true;
                    }
                }
            }
        }


        std::unordered_set<Point2, Point2::Hash> output;
        floodFill(cond, portCityPos, output);
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                if (output.find({ x,y }) != output.end())
                {
                    if (getProphecy(x, y, 0) == chunkType::shallowSea || getProphecy(x, y, 0) == chunkType::deepSea)
                    {
                        writeProphecy(x, y, 0, chunkType::city);
                        cityTypeMap[{x, y, 0}] = tgtCityType;
                    }
                }
            }
        }
    }



    //항구도시 확장 후 내부 고립셀 제거
    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) != chunkType::city) cond[x][y] = true;
                else cond[x][y] = false;
            }
        }
        auto internalCityPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& internalCity = *internalCityPtr;
        floodFillFindOrphan(cond, { 0,0 }, internalCity);
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (internalCity[x][y] == true)
                {
                    writeProphecy(x, y, 0, chunkType::city);
                    cityTypeMap[{x, y, 0}] = cityType::port;
                }
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      6단계 : 일반도시 배치 및 확장
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int normalCityNumber = 0;
    while (1)
    {
        loopCount++;
        errorBox(loopCount > 50000, L"도시 다트 찍기가 잘 안 된다... 루프 카운트가 5만을 초과했다.");

        int randX = randomRange(0, WORLD_DATA_SIZE - 1);
        int randY = randomRange(0, WORLD_DATA_SIZE - 1);

        if (randX < 10 || randX >= WORLD_DATA_SIZE - 10) continue;
        if (randY < 10 || randY >= WORLD_DATA_SIZE - 10) continue;


        if (getProphecy(randX, randY, 0) == chunkType::dirt)
        {
            bool cleanDistrict = true;
            constexpr int CLEAN_RANGE = 50;
            for (int dx = -CLEAN_RANGE / 2; dx <= CLEAN_RANGE / 2; dx++)
            {
                for (int dy = -CLEAN_RANGE / 2; dy <= CLEAN_RANGE / 2; dy++)
                {
                    if (isCircle(CLEAN_RANGE / 2 + 1, dx, dy)
                        && (getProphecy(randX + dx, randY + dy, 0) != chunkType::dirt))
                        cleanDistrict = false;
                }
            }

            bool noNearbyCity = true;
            constexpr int NO_CITY_DIAMETER = 100;
            for (int dx = -NO_CITY_DIAMETER / 2; dx <= NO_CITY_DIAMETER / 2; dx++)
            {
                for (int dy = -NO_CITY_DIAMETER / 2; dy <= NO_CITY_DIAMETER / 2; dy++)
                {
                    if (isCircle(NO_CITY_DIAMETER / 2 + 1, dx, dy) && getProphecy(randX + dx, randY + dy, 0) == chunkType::city) noNearbyCity = false;
                }
            }

            if (cleanDistrict == false || noNearbyCity == false) continue;
            writeProphecy(randX, randY, 0, chunkType::city);
            cityTypeMap[{randX, randY, 0}] = cityType::normal;
            cityCoreVec.push_back({ randX, randY });
            normalCityNumber++;
        }

        if (normalCityNumber >= 7) break;
    }


    for (auto core : cityCoreVec)
    {
        int cursorX = core.x;
        int cursorY = core.y;
        int cityMaxSize = randomRange(150, 300);
        int cityCurrentSize = 1;

        std::vector<Point2> cityPoints;

        std::priority_queue<std::pair<float, Point2>, std::vector<std::pair<float, Point2>>, std::greater<>> frontier;
        float randomKey = randomRangeFloat(0.0, 1.0);
        
        if (cityTypeMap[{core.x, core.y, 0}] != cityType::normal) continue;
        cityType tgtCityType = cityTypeMap[{core.x, core.y, 0}];

        auto cityChunkCond = [&](int x, int y, cityType inputTgtCityType) -> bool
            {
                if (isCircle(21, x - core.x, y - core.y) == false) return false;
                return getProphecy(x, y, 0) == chunkType::dirt
                    || getProphecy(x, y, 0) == chunkType::snow
                    || getProphecy(x, y, 0) == chunkType::desert
                    || getProphecy(x, y, 0) == chunkType::forest;
                if (inputTgtCityType == cityType::normal) return getProphecy(x, y, 0) == chunkType::dirt;
                else if (inputTgtCityType == cityType::snow) return getProphecy(x, y, 0) == chunkType::snow;
                else if (inputTgtCityType == cityType::desert) return getProphecy(x, y, 0) == chunkType::desert;
                else errorBox(L"이상한 시티 타입이 cityChunkCond에 입력되었다.");
            };

        if (cityChunkCond(core.x + 1, core.y, tgtCityType)) frontier.push({ randomKey,{ core.x + 1,core.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(core.x - 1, core.y, tgtCityType)) frontier.push({ randomKey,{ core.x - 1,core.y } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(core.x, core.y + 1, tgtCityType)) frontier.push({ randomKey,{ core.x ,core.y + 1 } });
        randomKey = randomRangeFloat(0.0, 1.0);
        if (cityChunkCond(core.x, core.y - 1, tgtCityType)) frontier.push({ randomKey,{ core.x ,core.y - 1} });

        while (frontier.empty() == false && cityCurrentSize < cityMaxSize)
        {
            int targetX = frontier.top().second.x;
            int targetY = frontier.top().second.y;
            frontier.pop();
            if (cityChunkCond(targetX, targetY, tgtCityType) == false) continue;

            writeProphecy(targetX, targetY, 0, chunkType::city);
            cityTypeMap[{targetX, targetY, 0}] = tgtCityType;
            cityPoints.push_back({ targetX,targetY });

            float randomKey = randomRangeFloat(0.0, 1.0);
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX + 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX + 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX - 1, targetY, tgtCityType)) frontier.push({ randomKey,{ targetX - 1,targetY } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY + 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY + 1 } });
            randomKey = randomRangeFloat(0.0, 1.0);
            if (cityChunkCond(targetX, targetY - 1, tgtCityType)) frontier.push({ randomKey,{ targetX ,targetY - 1} });

            cityCurrentSize++;
        }

        for (auto elem : cityPoints)
        {
            //사막 도시의 경우 주변 엣지를 사막으로 만드는 기능 추가할 것

            for (int ddx = -2; ddx <= 2; ddx++)
            {
                for (int ddy = -2; ddy <= 2; ddy++)
                {
                    if (cityChunkCond(elem.x + ddx, elem.y + ddy, tgtCityType))
                    {
                        writeProphecy(elem.x + ddx, elem.y + ddy, 0, chunkType::city);
                        cityTypeMap[{elem.x + ddx, elem.y + ddy, 0}] = tgtCityType;

                        if (tgtCityType == cityType::desert)
                        {
                            if (std::abs(ddx) == 2 || std::abs(ddy) == 2)
                            {
                                std::vector<Point2> nearbyDel = { {1,0},{0,-1},{-1,0},{0,1},{1,-1},{-1,1},{-1,-1},{1,1},{2,0},{0,-2},{-2,0},{0,2} };
                                for (auto targetDel : nearbyDel)
                                {
                                    if (getProphecy(elem.x + ddx + targetDel.x, elem.y + ddy + targetDel.y, 0) == chunkType::dirt)
                                    {
                                        writeProphecy(elem.x + ddx + targetDel.x, elem.y + ddy + targetDel.y, 0, chunkType::desert);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }

    //일반도시 확장 후 내부 고립셀 제거
    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) != chunkType::city) cond[x][y] = true;
                else cond[x][y] = false;
            }
        }
        auto internalCityPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& internalCity = *internalCityPtr;
        floodFillFindOrphan(cond, { 0,0 }, internalCity);
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (internalCity[x][y] == true)
                {
                    writeProphecy(x, y, 0, chunkType::city);
                    cityTypeMap[{x, y,0}] = cityType::normal;
                }
            }
        }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      7단계 : 해변 추가
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        auto condPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
        auto& cond = *condPtr;
        for (int y = 0; y < WORLD_DATA_SIZE; y++)
        {
            for (int x = 0; x < WORLD_DATA_SIZE; x++)
            {
                if (getProphecy(x, y, 0) == chunkType::deepSea || getProphecy(x, y, 0) == chunkType::shallowSea)
                {
                    cond[x][y] = true;
                }
                else if (getProphecy(x, y, 0) == chunkType::dirt && -0.13f > heightMap[x][y] && heightMap[x][y] > -0.15f && noiseMapBeach[x][y]>0)
                {
                    cond[x][y] = true;
                }
                else cond[x][y] = false;
            }
        }

        std::unordered_set<Point2, Point2::Hash> output;

        floodFill(cond, { 0,0 }, output);

        for (auto elem : output)
        {
            if (getProphecy(elem.x, elem.y, 0) == chunkType::dirt)
            {
                writeProphecy(elem.x, elem.y, 0, chunkType::beach);
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      8단계 : 도시 내부 청크 및 도로망 배치
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto cityCondPtr = std::make_unique< std::array<std::array<bool, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
    auto& cityCond = *cityCondPtr;
    for (int y = 0; y < WORLD_DATA_SIZE; y++)
    {
        for (int x = 0; x < WORLD_DATA_SIZE; x++)
        {
            if (getProphecy(x, y, 0) == chunkType::city) cityCond[x][y] = true;
        }
    }
    

    int currentBuildingID = 0;
    for (auto core : cityCoreVec)
    {
        std::unordered_set<Point2, Point2::Hash> cityPoints;
        floodFill(cityCond, { core.x,core.y }, cityPoints);

        int minX = 9999;
        int maxX = -9999;
        int minY = 9999;
        int maxY = -9999;
        for (auto elem : cityPoints)
        {
            if (elem.x < minX) minX = elem.x;
            if (elem.x > maxX) maxX = elem.x;
            if (elem.y < minY) minY = elem.y;
            if (elem.y > maxY) maxY = elem.y;
        }
        minX -= 1;
        maxX += 1;
        minY -= 1;
        maxY += 1;

        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                if (cityPoints.find({ x,y }) != cityPoints.end())
                {
                    bool findWater = false;
                    if (getProphecy(x + 1, y, 0) == chunkType::shallowSea
                        || getProphecy(x + 1, y, 0) == chunkType::deepSea
                        || getProphecy(x + 1, y, 0) == chunkType::river
                        || getProphecy(x + 1, y, 0) == chunkType::lake) findWater = true;
                    if (getProphecy(x - 1, y, 0) == chunkType::shallowSea
                        || getProphecy(x - 1, y, 0) == chunkType::deepSea
                        || getProphecy(x - 1, y, 0) == chunkType::river
                        || getProphecy(x - 1, y, 0) == chunkType::lake) findWater = true;
                    if (getProphecy(x, y + 1, 0) == chunkType::shallowSea
                        || getProphecy(x, y + 1, 0) == chunkType::deepSea
                        || getProphecy(x, y + 1, 0) == chunkType::river
                        || getProphecy(x, y + 1, 0) == chunkType::lake) findWater = true;
                    if (getProphecy(x, y - 1, 0) == chunkType::shallowSea
                        || getProphecy(x, y - 1, 0) == chunkType::deepSea
                        || getProphecy(x, y - 1, 0) == chunkType::river
                        || getProphecy(x, y - 1, 0) == chunkType::lake) findWater = true;

                    if (findWater) writeProphecy(x, y, 0, chunkType::cityRoad);
                }
            }
        }


        //도시 건물 랜덤배치
        int failStreak = 0;
        std::vector<chunkType> buildingSet;
        const float BLDG_PROB_2X2 = tuneParam.at(L"BLDG_PROB_2X2");
        const float BLDG_PROB_2X1 = tuneParam.at(L"BLDG_PROB_2X1");
        const float BLDG_PROB_1X2 = tuneParam.at(L"BLDG_PROB_1X2");
        const float BLDG_FAIL_STREAK = tuneParam.at(L"BLDG_FAIL_STREAK");
        while (1)
        {
            int randX = randomRange(minX, maxX);
            int randY = randomRange(minY, maxY);
            if (cityPoints.find({ randX,randY }) != cityPoints.end())
            {
                bool lot2by2 = false;
                bool lot2by1 = false;
                bool lot1by2 = false;
                bool lot1by1 = false;
                int prob = randomRange(1, 100);
                
                SDL_Rect testRect = { randX,randY,0,0 };
                if (prob <= BLDG_PROB_2X2)
                {
                    lot2by2 = true;
                    testRect.w = 2;
                    testRect.h = 2;
                    buildingSet = { chunkType::park, chunkType::hypermarket, chunkType::school, chunkType::parkingLot };
                }
                else if (prob <= BLDG_PROB_2X2 + BLDG_PROB_2X1)
                {
                    lot2by1 = true;
                    testRect.w = 2;
                    testRect.h = 1;
                    buildingSet = { chunkType::policeStation, chunkType::fireStation, chunkType::hotel, chunkType::hospital, chunkType::library };
                }
                else if (prob <= BLDG_PROB_2X2 + BLDG_PROB_2X1 + BLDG_PROB_1X2)
                {
                    lot1by2 = true;
                    testRect.w = 1;
                    testRect.h = 2;
                    buildingSet = { chunkType::policeStation, chunkType::fireStation, chunkType::hotel, chunkType::hospital, chunkType::library };
                }
                else
                {
                    lot1by1 = true;
                    testRect.w = 1;
                    testRect.h = 1;
                    buildingSet = { chunkType::apartment, chunkType::bank, chunkType::house, chunkType::warehouse,
                                    chunkType::cafe, chunkType::cinema, chunkType::junkShop, chunkType::animalHospital,
                                    chunkType::pharmacy, chunkType::restaurant, chunkType::stationeryStore,
                                    chunkType::hardwareStore, chunkType::bookstore, chunkType::patrolStation,
                                    chunkType::convenienceStore, chunkType::bicycleShop, chunkType::temple,
                                    chunkType::church, chunkType::cathedral, chunkType::skyscraper,
                                    chunkType::gasStation, chunkType::shoppingArcade, chunkType::postOffice,
                                    chunkType::autoShop, chunkType::clothingStore, chunkType::jewelryStore,
                                    chunkType::laundromat, chunkType::gardenShop };
                }

                if (getProphecy(randX, randY, 0) == chunkType::city)
                {
                    for (int y = randY; y < randY + testRect.h; y++)
                    {
                        for (int x = randX; x < randX + testRect.w; x++)
                        {
                            if (getProphecy(x, y, 0) != chunkType::city)
                            {
                                failStreak++;
                                if (failStreak > BLDG_FAIL_STREAK) goto BUILD_LOOP_END;
                                goto SKIP_DART;
                            }
                        }
                    }

                    std::unordered_set<Point2, Point2::Hash> cond;
                    Point2 startCoor = { 0,0 };
                    for (int y = minY; y <= maxY; y++)
                    {
                        for (int x = minX; x <= maxX; x++)
                        {
                            if (getProphecy(x, y, 0) == chunkType::cityRoad || getProphecy(x, y, 0) == chunkType::city)
                            {
                                if ((x >= testRect.x  && x < testRect.x + testRect.w
                                    && y >= testRect.y && y < testRect.y + testRect.h) == false)
                                {
                                    cond.insert({ x,y });
                                    startCoor = { x,y };
                                }

                            }
                        }
                    }

                    std::unordered_set<Point2, Point2::Hash> output;
                    floodFillFindOrphan(cond, startCoor, output);
                    if (output.size() > 0)
                    {
                        failStreak++;
                        if (failStreak > BLDG_FAIL_STREAK) goto BUILD_LOOP_END;
                        goto SKIP_DART;
                    }

                    failStreak = 0;
                    currentBuildingID++;
                    chunkType targetChunk = buildingSet[randomRange(0, buildingSet.size() - 1)];
                    for (int y = randY; y < randY + testRect.h; y++)
                    {
                        for (int x = randX; x < randX + testRect.w; x++)
                        {
                            writeProphecy(x, y, 0, targetChunk);
                            buildingID[{x, y, 0}] = currentBuildingID;
                        }
                    }
                }
                else
                {
                    failStreak++;
                    if (failStreak > BLDG_FAIL_STREAK) goto BUILD_LOOP_END;
                }
            }
            SKIP_DART:
        }
    BUILD_LOOP_END:

        //건물이 찍힌 곳을 제외한 곳을 전부 일반 도로로 변경
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                if (getProphecy(x,y,0) == chunkType::city)
                {
                    writeProphecy(x, y, 0, chunkType::cityRoad);
                }
            }
        }

        //2*2 도로 붕괴시키기
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                if (getProphecy(x, y, 0) == chunkType::cityRoad
                    && getProphecy(x+1, y, 0) == chunkType::cityRoad
                    && getProphecy(x, y+1, 0) == chunkType::cityRoad
                    && getProphecy(x+1, y+1, 0) == chunkType::cityRoad)
                {

                    buildingSet = { chunkType::apartment, chunkType::bank, chunkType::house, chunkType::warehouse,
                                    chunkType::cafe, chunkType::cinema, chunkType::junkShop, chunkType::animalHospital,
                                    chunkType::pharmacy, chunkType::restaurant, chunkType::stationeryStore,
                                    chunkType::hardwareStore, chunkType::bookstore, chunkType::patrolStation,
                                    chunkType::convenienceStore, chunkType::bicycleShop, chunkType::temple,
                                    chunkType::church, chunkType::cathedral, chunkType::skyscraper,
                                    chunkType::gasStation, chunkType::shoppingArcade, chunkType::postOffice,
                                    chunkType::autoShop, chunkType::clothingStore, chunkType::jewelryStore,
                                    chunkType::laundromat, chunkType::gardenShop };
                    currentBuildingID++;
                    chunkType targetChunk = buildingSet[randomRange(0, buildingSet.size() - 1)];

                    int randomVal = randomRange(0, 3);
                    for (int repeat = 0; repeat < 4; repeat++)
                    {
                        int tgtX, tgtY;
                        switch ((randomVal + repeat) % 4)
                        {
                        case 0:
                            tgtX = x, tgtY = y;
                            break;
                        case 1:
                            tgtX = x + 1, tgtY = y;
                            break;
                        case 2:
                            tgtX = x, tgtY = y + 1;
                            break;
                        case 3:
                            tgtX = x + 1, tgtY = y + 1;
                            break;
                        }

                        std::unordered_set<Point2, Point2::Hash> cond;
                        Point2 startCoor = { 0,0 };
                        for (int condY = minY; condY <= maxY; condY++)
                        {
                            for (int condX = minX; condX <= maxX; condX++)
                            {
                                if (getProphecy(condX, condY, 0) == chunkType::cityRoad)
                                {
                                    if (condX != tgtX || condY != tgtY)
                                    {
                                        cond.insert({ condX,condY });
                                        startCoor = { condX,condY };
                                    }
                                }
                            }
                        }

                        std::unordered_set<Point2, Point2::Hash> output;
                        floodFillFindOrphan(cond, startCoor, output);
                        if (output.size() == 0)
                        {
                            writeProphecy(tgtX, tgtY, 0, targetChunk);
                            buildingID[{tgtX, tgtY, 0}] = currentBuildingID;
                            break;
                        }
                    }
                }
            }
        }

        //고립된 건물을 도로로 변경하기
        int isolLoopCount = 0;
        const float ISOL_LOOP_COUNT = tuneParam.at(L"ISOL_LOOP_COUNT");
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                isolLoopCount++;
                if (buildingID.contains({ x,y,0 }) && buildingID[{x, y, 0}] != 0)
                {
                    if ((buildingID.contains({ x-1,y,0 }) && buildingID[{x-1, y, 0}] == buildingID[{x, y, 0}]) == false
                        && (buildingID.contains({ x,y-1,0 }) && buildingID[{x, y - 1, 0}] == buildingID[{x, y, 0}]) == false)
                    {

                        //현재 이 건물이 몇칸짜리 건물인지 분석
                        int  buildingWidth = 0, buildingHeight = 0;

                        for (int dx = 0; dx <= 3; dx++)
                        {
                            if (buildingID.contains({ x + dx,y,0 }) && buildingID[{x + dx, y, 0}] == buildingID[{x, y, 0}])
                            {
                                buildingWidth++;
                            }
                        }

                        for (int dy = 0; dy <= 3; dy++)
                        {
                            if (buildingID.contains({ x,y + dy,0 }) && buildingID[{x, y + dy, 0}] == buildingID[{x, y, 0}])
                            {
                                buildingHeight++;
                            }
                        }

                        std::vector<Point2> nearbyPoints;
                        for (int ny = y - 1; ny <= y + buildingHeight; ny++)
                        {
                            for (int nx = x - 1; nx <= x + buildingWidth; nx++)
                            {
                                //건물 내부 제거
                                if (nx >= x && nx <= x + buildingWidth - 1 && ny >= y && ny <= y + buildingHeight - 1) continue;
                                //네 모서리 제거 아래 4줄
                                else if (nx == x + buildingWidth  && ny == y + buildingHeight) continue;
                                else if (nx == x + buildingWidth && ny == y - 1) continue;
                                else if (nx == x - 1 && ny == y + buildingHeight) continue;
                                else if (nx == x - 1 && ny == y - 1) continue;
                                else nearbyPoints.push_back({ nx,ny });
                            }
                        }

                        bool iAmIsolated = true;
                        
                        for (auto nPoint : nearbyPoints)
                        {
                            if (getProphecy(nPoint.x, nPoint.y, 0) == chunkType::cityRoad)
                            {
                                iAmIsolated = false;
                            }
                        }
                        
                        if (iAmIsolated)
                        {
                            for (auto nPoint : nearbyPoints)
                            {
                                if (buildingID.contains({ nPoint.x,nPoint.y,0 }))
                                {
                                    //1*1 건물인지 체크
                                    if ((buildingID.contains({ nPoint.x + 1,nPoint.y,0 }) && buildingID[{nPoint.x + 1, nPoint.y, 0}] == buildingID[{nPoint.x, nPoint.y, 0}]) == false
                                        && (buildingID.contains({ nPoint.x - 1,nPoint.y,0 }) && buildingID[{nPoint.x - 1, nPoint.y, 0}] == buildingID[{nPoint.x, nPoint.y, 0}]) == false
                                        && (buildingID.contains({ nPoint.x ,nPoint.y - 1,0 }) && buildingID[{nPoint.x, nPoint.y - 1, 0}] == buildingID[{nPoint.x, nPoint.y, 0}]) == false
                                        && (buildingID.contains({ nPoint.x ,nPoint.y + 1,0 }) && buildingID[{nPoint.x, nPoint.y + 1, 0}] == buildingID[{nPoint.x, nPoint.y, 0}]) == false)
                                    {
                                        //주변에 도로가 한 타일이라도 있는지 체크
                                        if (getProphecy(nPoint.x - 1, nPoint.y, 0) == chunkType::cityRoad
                                            || getProphecy(nPoint.x + 1, nPoint.y, 0) == chunkType::cityRoad
                                            || getProphecy(nPoint.x, nPoint.y - 1, 0) == chunkType::cityRoad
                                            || getProphecy(nPoint.x, nPoint.y + 1, 0) == chunkType::cityRoad)
                                        {
                                            //만약 도로로 바꿀 경우 2*2 도로가 발생하는지 체크
                                            if (getProphecy(nPoint.x - 1, nPoint.y - 1, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x - 1, nPoint.y, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x, nPoint.y - 1, 0) == chunkType::cityRoad)
                                            {
                                                continue;
                                            }
                                            else if (getProphecy(nPoint.x, nPoint.y - 1, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x + 1, nPoint.y - 1, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x + 1, nPoint.y, 0) == chunkType::cityRoad)
                                            {
                                                continue;
                                            }
                                            else if (getProphecy(nPoint.x - 1, nPoint.y, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x - 1, nPoint.y + 1, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x, nPoint.y + 1, 0) == chunkType::cityRoad)
                                            {
                                                continue;
                                            }
                                            else if (getProphecy(nPoint.x + 1, nPoint.y, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x + 1, nPoint.y + 1, 0) == chunkType::cityRoad
                                                && getProphecy(nPoint.x, nPoint.y + 1, 0) == chunkType::cityRoad)
                                            {
                                                continue;
                                            }
                                            else
                                            {
                                                buildingID.erase({ nPoint.x,nPoint.y,0 });
                                                writeProphecy(nPoint.x, nPoint.y, 0, chunkType::cityRoad);
                                                x = minX, y = minY;
                                                break;
                                            }

                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (isolLoopCount > ISOL_LOOP_COUNT)
                {
                    std::wprintf(L"[경고] isolLoopCount가 상한치를 넘었다.\n");
                    x = 99999;
                    y = 99999;
                }
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //      9단계 : 다익스트라 알고리즘을 이용한 도시 간 도로망 생성 시작
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
       std::unordered_map<chunkType, int> cost;
       cost[chunkType::dirt] = tuneParam.at(L"COST_DIRT");
       cost[chunkType::deepSea] = tuneParam.at(L"COST_DEEP_SEA");
       cost[chunkType::shallowSea] = tuneParam.at(L"COST_SHALLOW_SEA");
       cost[chunkType::beach] = tuneParam.at(L"COST_BEACH");
       cost[chunkType::mountain] = tuneParam.at(L"COST_MOUNTAIN");
       cost[chunkType::river] = tuneParam.at(L"COST_RIVER");
       cost[chunkType::lake] = tuneParam.at(L"COST_LAKE");
       cost[chunkType::forest] = tuneParam.at(L"COST_FOREST");
       cost[chunkType::desert] = tuneParam.at(L"COST_DESERT");
       cost[chunkType::snow] = tuneParam.at(L"COST_SNOW");
       cost[chunkType::volcanicLand] = tuneParam.at(L"COST_VOLCANIC_LAND");
       cost[chunkType::volcano] = tuneParam.at(L"COST_VOLCANO");
       cost[chunkType::jungle] = tuneParam.at(L"COST_JUNGLE");
       cost[chunkType::cityRoad] = tuneParam.at(L"COST_CITY_ROAD");
       cost[chunkType::road] = tuneParam.at(L"COST_ROAD");

       // n! / (r!(n-r)!) = nCr 대략 66개 정도인가?
       for (Point2 cityCore : cityCoreVec)
       {
           std::priority_queue<std::pair<int, Point2>, std::vector<std::pair<int, Point2>>, std::greater<>> openNodes;
           
           //중심에 가까운 도로 찾기
           const int CORE_NEARBY_ROAD = tuneParam.at(L"CORE_NEARBY_ROAD");
           float hiScoreDist = std::numeric_limits<float>::max();
           Point2 hiScorePoint = { 0,0 };
           for (int y = cityCore.y - CORE_NEARBY_ROAD; y <= cityCore.y + CORE_NEARBY_ROAD; y++)
           {
               for (int x = cityCore.x - CORE_NEARBY_ROAD; x <= cityCore.x + CORE_NEARBY_ROAD; x++)
               {
                   if (x >= 0 && x < WORLD_DATA_SIZE && y >= 0 && y < WORLD_DATA_SIZE)
                   {
                       if (getProphecy(x, y, 0) == chunkType::cityRoad)
                       {
                           if (std::pow(x - cityCore.x, 2) + std::pow(y - cityCore.y, 2) < hiScoreDist)
                           {
                               hiScoreDist = std::pow(x - cityCore.x, 2) + std::pow(y - cityCore.y, 2);
                               hiScorePoint = { x,y };
                           }
                       }
                   }
               }
           }

           errorBox(hiScorePoint == Point2{0, 0}, L"중심에 가까운 오픈노드 시작점 도로 찾기가 실패했다.");
           openNodes.push({ 0,hiScorePoint });
           auto distPtr = std::make_unique< std::array<std::array<int, WORLD_DATA_SIZE>, WORLD_DATA_SIZE>>();
           auto& dist = *distPtr;
           for (int y = 0; y < WORLD_DATA_SIZE; y++)
           {
               for (int x = 0; x < WORLD_DATA_SIZE; x++)
               {
                   dist[x][y] = std::numeric_limits<int>::max();
               }
           }
           dist[hiScorePoint.x][hiScorePoint.y] = 0;

           while (openNodes.empty() == false)
           {
               //다익스트라 알고리즘 하나씩 꺼내서 시작
               //주변을 조사하면서 비용 갱신을 시작함. 자기를 거쳐서 그 방향으로 갔을 때 새 비용이 기존 써진 비용보다 낮으면 그걸로 갱신
               //단 건물은 이 갱신 대상에서 완전히 제외
               //도시 내부, 정확히는 cityRoad일 때는 꺾음의 코스트가 없음(도시 내부는 불규칙적인 도로망이니까)
               //노드를 꺼낼 때는 비용이 작은 것부터 꺼내짐. 이건 우선순위 큐가 알아서 처리할거야
               //노드가 없어질 때까지 반복한다. 그러면 이 도시 중심으로부터 각 도시에 진입하는데 걸리는 비용을 알 수 있다.
               //근데 또 각 도시에 진입한 순간을 알아야하는데... 도시별로 추상화 레이어를 하나 더 만들어야하나? 배열 하나면 가능하기는 하지만...
               //private에 배열 하나 만들어두고 위 생성 알고리즘에서 조금씩 수정해야겠군

           }
           
           
       }

    }


}