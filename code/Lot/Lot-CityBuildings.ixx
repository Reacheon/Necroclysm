module;
export module Lot:CityBuildings;

import std;
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
import :Hypermarket;
import :School;
import :Park;

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
    //도시 절차배치 대상 건물 Lot 마스터 목록. 추가 시 여기 한 줄.
    const Lot* const cityBuildingPool[] = {
        //1x1
        &house, &apartment, &cafe, &restaurant, &convenienceStore, &shoppingArcade,
        &temple, &church, &cathedral, &skyscraper, &bank, &cinema, &warehouse,
        &junkShop, &gasStation, &animalHospital, &hardwareStore, &bookstore,
        &pharmacy, &patrolStation, &bicycleShop, &stationeryStore,
        //2x1 (회전 가능: policeStation, 비회전: fireStation)
        &policeStation, &fireStation,
        //2x2
        &hypermarket, &school, &park,
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

//footprint(gw,gh)를 채울 건물 Lot을 균등 추첨. 후보 없으면 nullptr(호출자가 스킵).
//  회전 결정은 호출자(CityPlan_build stage 10)가 그룹의 도로 인접면을 보고 수행한다.
//  가중치(중심부 마천루 가중 등)는 추후 — 현재는 균등.
export const Lot* buildingByFootprint(int gw, int gh, std::mt19937_64& rng)
{
    const Lot* hits[std::size(cityBuildingPool)];
    int n = 0;
    for (const Lot* l : cityBuildingPool)
        if (canFillFootprint(*l, gw, gh)) hits[n++] = l;

    if (n == 0) return nullptr;
    return hits[std::uniform_int_distribution<int>{0, n - 1}(rng)];
}
