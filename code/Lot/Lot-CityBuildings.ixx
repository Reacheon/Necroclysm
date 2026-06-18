module;
export module Lot:CityBuildings;

import std;
import constVar;   //MapSymbol — 건물 Lot을 월드맵 심볼로 매핑
import :base;

//건물 kind 싱글톤 참조용 — 같은 모듈 내 sibling 파티션 import (외부 import 아님).
import :House;
import :Apartment;
import :Cafe;
import :Restaurant;
import :ConvenienceStore;
import :ShoppingArcade;
import :Temple;
import :Church;
import :Cathedral;
import :Skyscraper;
import :Bank;
import :Cinema;
import :Warehouse;
import :JunkShop;
import :GasStation;
import :AnimalHospital;
import :HardwareStore;
import :Bookstore;
import :Pharmacy;
import :PatrolStation;
import :BicycleShop;
import :StationeryStore;
import :PoliceStation;
import :FireStation;
import :Hotel;
import :Hospital;
import :Library;
import :Hypermarket;
import :School;
import :Park;
import :PostOffice;
import :AutoShop;
import :ClothingStore;
import :JewelryStore;
import :Laundromat;
import :GardenShop;
import :ParkingLot;

// ════════════════════════════════════════════════════════════════════════
// buildingByFootprint — footprint(청크 W×H)로 채울 건물 Lot을 골라주는 라우터.
//
//   streetByOpenSides/bridgeByOpenSides와 같은 역할: CityPlan_build가 건물 카탈로그를
//   몰라도 되게 격리한다. 새 건물 추가 = 아래 cityBuildingPool에 한 줄 — footprint
//   버킷은 각 Lot의 sizeChunkW/H에서 자동 파생되므로 별도 풀 유지보수 불필요.
//
//   회전 규약(호출자가 회전 수행): 비정사각 Lot은 authored 방향(예: 2x1)으로만 작성하고
//   90/270° 회전으로 1x2도 커버한다. 따라서 한 직사각 Lot이 (2,1)과 (1,2) 그룹 둘 다
//   채울 수 있다 — 단 allowRotation()==false인 Lot은 회전을 못 하므로 authored
//   footprint에만 매칭한다(canFillFootprint가 보장).
//
//   Street/Bridge/Sample은 건물이 아니므로 풀에서 제외.
// ════════════════════════════════════════════════════════════════════════

namespace
{
    //출현 가중치 티어 — 상대값(스케일 임의). 도시 구성 분포를 "보면서 조정"하기 쉽게 한 곳에 모음.
    //  ★ 가중치는 *같은 footprint 버킷 안에서의* 상대 비율이다. 버킷 간(1x1 vs 2x1 vs 2x2)
    //    비율은 stage 4 다트던지기 박스확률(1x1 70% / 2x1 10% / 1x2 10% / 2x2 10%)이 결정.
    //    즉 weight는 "1x1들끼리 누가 더 흔한가"를 정할 뿐, 1x1 전체가 2x2보다 흔한 건 박스확률 몫.
    constexpr int W_VERY_COMMON = 16;  // 어디나 흔함 (주거)
    constexpr int W_COMMON      = 8;   // 일상 (식음·편의·상가·숙박)
    constexpr int W_NORMAL      = 4;   // 보통 (전문 소매·서비스)
    constexpr int W_RARE        = 2;   // 드묾 (희소 업종·도시당 소수)

    //풀 엔트리 — Lot 싱글톤 + 월드맵 심볼 + 출현 가중치.
    struct PoolEntry { const Lot* lot; MapSymbol sym; int weight; };

    //도시 절차배치 대상 건물 Lot 마스터 목록. 추가 시 여기 한 줄(Lot + 심볼 + 가중치).
    const PoolEntry cityBuildingPool[] = {
        //1x1
        { &house,            MapSymbol::house,            W_VERY_COMMON },
        { &apartment,        MapSymbol::apartment,        W_VERY_COMMON },
        { &cafe,             MapSymbol::cafe,             W_COMMON      },
        { &restaurant,       MapSymbol::restaurant,       W_COMMON      },
        { &convenienceStore, MapSymbol::convenienceStore, W_COMMON      },
        { &shoppingArcade,   MapSymbol::shoppingArcade,   W_COMMON      },
        { &temple,           MapSymbol::temple,           W_NORMAL      },
        { &church,           MapSymbol::church,           W_NORMAL      },
        { &cathedral,        MapSymbol::cathedral,        W_RARE        },
        { &skyscraper,       MapSymbol::skyscraper,       W_NORMAL      },
        { &bank,             MapSymbol::bank,             W_NORMAL      },
        { &cinema,           MapSymbol::cinema,           W_NORMAL      },
        { &warehouse,        MapSymbol::warehouse,        W_NORMAL      },
        { &junkShop,         MapSymbol::junkShop,         W_NORMAL      },
        { &gasStation,       MapSymbol::gasStation,       W_COMMON      },
        { &animalHospital,   MapSymbol::animalHospital,   W_RARE        },
        { &hardwareStore,    MapSymbol::hardwareStore,    W_NORMAL      },
        { &bookstore,        MapSymbol::bookstore,        W_NORMAL      },
        { &pharmacy,         MapSymbol::pharmacy,         W_NORMAL      },
        { &patrolStation,    MapSymbol::patrolStation,    W_NORMAL      },
        { &bicycleShop,      MapSymbol::bicycleShop,      W_RARE        },
        { &stationeryStore,  MapSymbol::stationeryStore,  W_NORMAL      },
        { &postOffice,       MapSymbol::postOffice,       W_NORMAL      },
        { &autoShop,         MapSymbol::autoShop,         W_NORMAL      },
        { &clothingStore,    MapSymbol::clothingStore,    W_NORMAL      },
        { &jewelryStore,     MapSymbol::jewelryStore,     W_RARE        },
        { &laundromat,       MapSymbol::laundromat,       W_NORMAL      },
        { &gardenShop,       MapSymbol::gardenShop,       W_NORMAL      },
        //2x1 / 1x2 (회전 가능: 모두 — 심볼은 footprint 방향으로 wide/tall 분기)
        { &policeStation,    MapSymbol::policeStation,    W_NORMAL        },
        { &fireStation,      MapSymbol::fireStation,      W_NORMAL        },
        { &hotel,            MapSymbol::hotel,            W_NORMAL      },
        { &hospital,         MapSymbol::hospital,         W_NORMAL      },
        { &library,          MapSymbol::library,          W_NORMAL      },
        //2x2
        { &hypermarket,      MapSymbol::hypermarket,      W_NORMAL      },
        { &school,           MapSymbol::school,           W_NORMAL      },
        { &park,             MapSymbol::park,             W_COMMON      },
        { &parkingLot,       MapSymbol::parkingLot,       W_NORMAL      },
    };

    //Lot이 group footprint(gw,gh)를 채울 수 있는가.
    //  authored 그대로(none 회전) 일치, 또는 회전 가능 시 축 스왑(ccw90/270)으로 일치.
    bool canFillFootprint(const Lot& lot, int gw, int gh)
    {
        const int aw = lot.sizeChunkW();
        const int ah = lot.sizeChunkH();
        if (aw == gw && ah == gh) return true;
        if (lot.allowRotation() && aw == gh && ah == gw) return true;
        return false;
    }
}

//footprint(gw,gh)를 채울 건물 Lot을 가중 추첨. 후보 없으면 nullptr(호출자가 스킵).
//  회전 결정은 호출자(CityPlan_build stage 10)가 그룹의 도로 인접면을 보고 수행한다.
//  가중치는 PoolEntry.weight — footprint 일치 후보들의 weight 합에서 비례 추첨(누적합 워크).
export const Lot* buildingByFootprint(int gw, int gh, std::mt19937_64& rng)
{
    int total = 0;
    for (const auto& e : cityBuildingPool)
        if (canFillFootprint(*e.lot, gw, gh)) total += e.weight;

    if (total <= 0) return nullptr;

    int r = std::uniform_int_distribution<int>{ 0, total - 1 }(rng);
    for (const auto& e : cityBuildingPool)
        if (canFillFootprint(*e.lot, gw, gh))
        {
            r -= e.weight;
            if (r < 0) return e.lot;
        }
    return nullptr;   // 도달 불가 (total>0면 위에서 반환)
}

//Lot 싱글톤 → 월드맵 심볼. 풀에 없는 Lot(Street/Bridge/Sample 등)이면 none.
//  CityPlan_build stage 10이 선택한 건물 Lot을 plan.symbols에 기록할 때 사용.
export MapSymbol mapSymbolOf(const Lot* lot)
{
    for (const auto& e : cityBuildingPool)
        if (e.lot == lot) return e.sym;
    return MapSymbol::none;
}
