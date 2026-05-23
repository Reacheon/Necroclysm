module;
export module lot:StreetNS;

import std;
import :base;
import constVar;



export class StreetNS final : public Lot
{
public:
    int sizeW() const override { return 1; }
    int sizeH() const override { return 1; }

protected:
    void build(LotBuilder& b, std::uint64_t seed) const override
    {
        //x 0..3      : 서쪽 paver 인도 4타일
        //x 4..19     : 도로 16타일 (NS 방향, 양방 8타일)
        //  x 11,12  : 노란 중앙선 (점선 6타일 주기로 on/off)
        //x 20..23    : 동쪽 paver 인도 4타일
        //
        //y는 도로 진행 방향 — 24타일 lot 길이가 TREE_PERIOD(=6) 정렬과 정합.

        constexpr int TREE_PERIOD = 6;
        constexpr int STREET_TREE_KINDS[6] = {
            itemID::ginkgoTree, itemID::cherryTree,   itemID::mapleTree,
            itemID::magnoliaTree, itemID::oakTree,    itemID::juniperTree,
        };

        for (int y = 0; y < 24; y++)
        {
            const bool dashOn = (y % TREE_PERIOD) < 3;

            for (int x = 0; x < 24; x++)
            {
                if (x <= 3 || x >= 20)
                {
                    b.setFloor(x, y, 0, itemID::paver);
                }
                else if (x == 11 && dashOn)
                {
                    b.setFloor(x, y, 0, itemID::yellowAsphaltRightHalf);
                }
                else if (x == 12 && dashOn)
                {
                    b.setFloor(x, y, 0, itemID::yellowAsphaltLeftHalf);
                }
                else
                {
                    b.setFloor(x, y, 0, itemID::blackAsphalt);
                }
            }
        }

        //가로수 — 도로변 paver(x=3 서쪽, x=20 동쪽) 안쪽 끝에 TREE_PERIOD 주기로 배치.
        //  CityPlan_build와 동일한 패턴: paver → dirt 교체 후 가로수 prop 설치.
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<int> kindDist(0, 5);
        for (int y = 0; y < 24; y += TREE_PERIOD)
        {
            for (int sideX : { 3, 20 })
            {
                b.setFloor(sideX, y, 0, itemID::dirt);
                b.setProp(sideX, y, 0, STREET_TREE_KINDS[kindDist(rng)]);
            }
        }
    }
};

export inline const StreetNS streetNS;
