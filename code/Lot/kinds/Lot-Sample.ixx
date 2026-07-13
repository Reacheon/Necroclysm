module;
export module Lot:Sample;

import std;
import :base;
import constVar;
import util;

//LotBuilder 헬퍼들(setFloor/setWall/setProp/addItemStack/addMonster/addVehicle/
//addPropContents)이 LotResult 경유로 실타일에 도달하는지 시각 확인용
//테스트 Lot. 게임 컨텐츠 X. debugConsole 38.
//
//이름이 SampleLot인 이유: code/GUI/Sample.ixx에도 `export class Sample`이 있어
//MSVC C++20 모듈에서 silent vtable corruption이 발생. 자세한 회피 규칙은
//feedback_msvc_module_class_name_collision 메모리 참조.

export class SampleLot final : public Lot
{
public:
    int sizeChunkW() const override { return 1; }
    int sizeChunkH() const override { return 1; }
    bool allowRotation() const override { return true; }

protected:
    void build(LotBuilder& b, std::uint64_t /*seed*/) const override
    {
        for (int y = 0; y < 24; ++y)
            for (int x = 0; x < 24; ++x)
                b.setFloor(x, y, 0, itemID::paver);

        for (int x = 8; x < 16; ++x)
            b.setWall(x, 0, 0, itemID::stoneWall);

        b.setProp(12, 12, 0, itemID::woodenSign);

        //냉장고 + 내부 아이템 — addPropContents로 prop 내부 ItemPocket 채움 검증.
        //  냉장고. setProp 직후 같은 좌표에 addPropContents 호출하면 인스턴스화 후
        //  leadItem.pocketPtr에 아이템이 자동 주입.
        b.setProp(15, 12, 0, itemID::refrigerator);
        b.addPropContents(15, 12, 0, { { itemID::pickaxe, 1 }, { itemID::hoe, 1 } });

        b.addItemStack(0,  0,  0, { { itemID::pickaxe,    1 } });
        b.addItemStack(23, 0,  0, { { itemID::hoe,        1 } });
        b.addItemStack(0,  23, 0, { { itemID::scythe,     1 } });
        b.addItemStack(23, 23, 0, { { itemID::fellingAxe, 1 } });

        b.addMonster(11, 11, 0, 5);   //5 = 허수아비

        //자전거 — 1×3 footprint. addVehicle가 돌려준 VehicleBuilder에 부품을 직접 박는다.
        //  각 부품은 Lot 할당 범위(24×24)에 대해 bounds 자동 검증.
        {
            const int vX = 5;
            const int vY = 12;
            auto& bike = b.addVehicle(vX, vY, 0, itemID::metalFrame, vehFlag::none, L"Bicycle");
            bike.extendPart(vX, vY - 1, itemID::metalFrame);
            bike.extendPart(vX, vY + 1, itemID::metalFrame);
            bike.addPart(vX, vY - 1, { itemID::tire,         itemID::bicycleHandlebar });
            bike.addPart(vX, vY,     { itemID::bicyclePedal, itemID::bicycleSaddle    });
            bike.addPart(vX, vY + 1, { itemID::tire,         itemID::shoppingBasket   });
            //장바구니 내부 pocket에 농산물 적재.
            bike.addCargo(vX, vY + 1, itemID::shoppingBasket, itemID::tomato,  3);
            bike.addCargo(vX, vY + 1, itemID::shoppingBasket, itemID::cabbage, 2);
            bike.setDir(dir16::dir2);
        }
    }
};

export inline const SampleLot sampleLot;
