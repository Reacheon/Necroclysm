module;
export module Blueprint:bicycle;

import std;
import :base;
import constVar;
import util;
import Vehicle;

//Bicycle — startArea에서 추출한 자전거. 1x3 footprint (수직).
//   vehType은 원본 동일 vehFlag::none (자전거는 차량 enum에 별도 항목 없음).

export class BicycleBlueprint final : public Blueprint
{
public:
    int leadItem() const override { return itemID::metalFrame; }
    vehFlag type() const override { return vehFlag::none; }
    std::wstring name() const override { return L"Bicycle"; }

    void build(Vehicle* myBike, Point3 anchor) const override
    {
        const int vX = anchor.x;
        const int vY = anchor.y;

        myBike->extendPart(vX, vY - 1, itemID::metalFrame);
        myBike->extendPart(vX, vY + 1, itemID::metalFrame);

        myBike->addPart(vX, vY - 1, { itemID::tire,          itemID::bicycleHandlebar });
        myBike->addPart(vX, vY,     { itemID::bicyclePedal,  itemID::bicycleSaddle    });
        myBike->addPart(vX, vY + 1, { itemID::tire,          itemID::shoppingBasket   });
    }
};

export inline const BicycleBlueprint bicycleBlueprint;
