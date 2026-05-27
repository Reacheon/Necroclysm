module;
export module Blueprint:base;

import std;
import util;
import constVar;
import Vehicle;

//Blueprint — Lot이 참조하는 차량 설계도 베이스.
//
//   Lot은 const Blueprint* 만 들고 있고, 실제 Vehicle 인스턴스는
//   createChunk 마지막 단계의 createVehicleFromBlueprint가 createVehicle + build()
//   + rotatePartInfo 순으로 만든다.
//
//   구체 blueprint는 inline const 전역 인스턴스로 두어 포인터 안정성 보장
//   (lot:Street, lot:Bridge 등과 동일 패턴).
//
//   build(target, anchor) 안에서 extendPart/addPart로 부품 트리를 채운다. target은
//   anchor 좌표에 leadItem만 깔린 빈 차량 — anchor 절대좌표 기준 상대 오프셋으로
//   부품을 박으면 회전 원점이 anchor와 정합해 rotatePartInfo가 올바르게 동작.
//
//   파생 클래스명 컨벤션: '<Name>Blueprint' 접미어 강제 — MSVC C++20 모듈 export class
//   이름 충돌 시 silent vtable corruption 회피 (feedback_msvc_module_class_name_collision).

export class Blueprint
{
public:
    virtual ~Blueprint() = default;

    virtual int leadItem() const = 0;
    virtual vehFlag type() const = 0;
    virtual std::wstring name() const = 0;
    virtual void build(Vehicle* target, Point3 anchor) const = 0;
};
