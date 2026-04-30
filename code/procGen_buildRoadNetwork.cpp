module procGen;

import std;
import util;

namespace procGen
{
    //@brief 도시 좌표들을 바탕으로 도로 폴리라인 네트워크를 생성한다. 순수 블랙박스 함수.
    //@param onRoad 옵션 진행 콜백. 폴리라인 1개 완성될 때마다 호출. default no-op이면 출력 영향 X.
    std::vector<RoadPolyLine> buildRoadNetwork(std::uint64_t seed, const PixelCostGrid& grid, const std::vector<CityNode>& cities, RoadSink onRoad)
    {
        return {};
    }
}
