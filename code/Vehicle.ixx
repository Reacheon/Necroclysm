#include <SDL.h>


export module Vehicle;

import std;
import constVar;
import ItemPocket;
import ItemData;
import util;
import Ani;
import AI;
import Coord;
import Drawable;

/*
* 각 파츠가 가지는 데이터
* 특정 부품의 종류, 내구도, 연료량?
* 어차피 플레이어 시야에 그려질 수 있는건 최대 2청크이다. 즉 주변 9청크에 있는 차량만 전부 그려내면 됨
* 차량의 크기제한은 1청크를 넘지 못하게 하는게 낫지 않나 라고 생각
* 문제는 부품이다.
* 수리는 내구도에 따른 내림 계산으로 하면됨
*/


export class Vehicle : public Ani, public AI, public Coord, public Drawable
{
public:
    std::wstring name = L"Vehicle";
    double pullMoveSpd = 3.0; //카트이동 시의 이동속도
    std::unordered_map<std::array<int, 2>, std::unique_ptr<ItemPocket>, decltype(arrayHasher2)> partInfo;
    vehFlag vehType = vehFlag::none;
    dir16 bodyDir = dir16::dir2;
    dir16 wheelDir = dir16::dir2;
    Vec3 spdVec = getZeroVec();
    Vec3 accVec = getZeroVec();
    bool turnOnBrake = false;
    float turnResource = 0;
    gearFlag gearState = gearFlag::drive;
    float xAcc = 0;
    float yAcc = 0;
    float zAcc = 0; //헬기 등에서는 사용될 수 있을 것 같다
    float rotateAcc = 0;
    Point2 trainWheelCenter = { 0,0 }; //열차의 회전중심, 열차바퀴들의 중심좌표로 정해짐
    //헬기용 변수
    __int8 collectiveState = 0;
    __int8 cyclicState = -1;
    __int8 rpmState = 0; //0~6
    bool isAIFirstRun = true;

    //싱글레일(협궤) 관련 변수
    float singleRailSpdVal = 0.0;
    dir16 singleRailSpdDir = dir16::dir2;
    int singleRailMoveCounter = 0;
    std::vector<dir16> singleRailMoveVec;
    Vehicle* rearCart = nullptr;
    bool isPowerCart = false;

    //멀티레일(광궤) 관련 변수;
    float wideRailSpdVal = 0.0;
    dir16 wideRailSpdDir = dir16::dir2;
    std::vector<dir16> wideRailMoveVec;
    bool isPowerTrain = false;
    Vehicle* rearTrain = nullptr;

    Vehicle(int inputX, int inputY, int inputZ, int leadItemCode);
    ~Vehicle();
    bool hasFrame(int inputX, int inputY);
    /////////////////////////////////////////※ 기존 프레임에 부품 추가////////////////////////////////////////////////////
    void addPart(int inputX, int inputY, ItemData inputPart); //기본 부품추가 함수, 모든 함수들이 이 함수를 기본으로 들어감, 여기에 알고리즘 넣을 것
    void addPart(int inputX, int inputY, int dexIndex);
    void addPart(int inputX, int inputY, std::vector<int> dexVec);
    void erasePart(int inputX, int inputY, int index);
    //////////////////////////////////////////////※ 프레임 확장/////////////////////////////////////////////////////////
    void extendPart(int inputX, int inputY, int inputItemCode);
    int getSprIndex(int inputX, int inputY);
    void getRotatePartInfo(dir16 inputDir16);
    std::unordered_set<std::array<int, 2>, decltype(arrayHasher2)> getRotateShadow(dir16 inputDir16);
    void rotateEntityPtr(dir16 inputDir16);
    void rotate(dir16 inputDir16);
    void updateSpr();
    void shift(int dx, int dy);
    void zShift(int dz);
    bool colisionCheck(dir16 inputDir16, int dx, int dy);
    bool colisionCheck(int dx, int dy);//해당 dx,dy만큼 이동했을 때 prop이 벽 또는 기존의 Vehicle과 충돌하는지
    void rush(int dx, int dy);
    void centerShift(int dx, int dy, int dz);
    bool runAI();
    bool runAnimation(bool shutdown);
    void updateTrainCenter();
    void drawSelf() override;
};

