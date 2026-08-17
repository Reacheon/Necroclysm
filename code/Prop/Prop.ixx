export module Prop;

import std;
import constVar;
import util;
import ItemData;
import Ani;
import AI;
import Light;
import Coord;
import Drawable;

export enum class crossFlag
{
    horizontal,
    vertical,
    omni,
};

export class Prop : public Ani, public AI, public Coord, public Drawable
{
private:

public:

    ItemData leadItem;
    int displayHPBarCount = 0; //양수 200으로 설정시 점점 떨어지다가 1이 되면 alpha를 대신 줄임. alpha마저 모두 줄면 0으로
    int alphaHPBar = 0;
    int alphaFakeHPBar = 0;
    float treeAngle = 0.0; //벌목 때 나무들이 가지는 앵글, 0이 아닐 경우 활성화됨

    bool runUsed = false; //runProp
    bool fluidRunUsed = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int nodeMaxCharge = 0; //회로망 전체의 총 가용 전력. 예로 20W 발전기 2개가 연결된 회로망은 절대 40W 이상이 전송될 수 없다.
    double nodeCharge = 0;
    double totalLossCharge = 0; //이번 턴에 저항으로 손실된 모든 에너지값

    double totalResistFluid = 0; //이번 턴에 저항으로 넘어가지 못한 유체

    std::unordered_map<dir16, double> chargeFlux = { {dir16::right,0},{dir16::up,0},{dir16::left,0},{dir16::down,0},{dir16::above,0},{dir16::below,0} };
    /*  ▲ chargeFlux 부호 규칙:
    *       - 음수(-): 해당 방향으로 전하를 "보냄" (출력)
    *       - 양수(+): 해당 방향에서 전하를 "받음" (입력)
    *   예: chargeFlux[dir16::right] = -5 → 오른쪽으로 5만큼 보냄
    *       chargeFlux[dir16::left] = +5  → 왼쪽에서 5만큼 받음
    */

    int delayMaxStack = 3;
    double delayStartTurn = 0;

    std::unordered_map<Point3, crossFlag> crossStates;

    double gndSink = 0.0;
    double gndSinkRight = 0.0;
    double gndSinkUp = 0.0;
    double gndSinkLeft = 0.0;
    double gndSinkDown = 0.0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    fluidType nodeFluidType = fluidType::NONE;
    double nodeFluidAmount = 0; //mL 단위

    double sinkFluidAmount = 0.0;
    fluidType sinkFluidType = fluidType::NONE;

    //스프라이트 그리기용 참고 변수
    fluidType jetFluidType = fluidType::NONE;
    dir16 jetFluidDir = dir16::none;

    std::unordered_map<dir16, double> fluidFlux = { {dir16::right,0},{dir16::up,0},{dir16::left,0},{dir16::down,0},{dir16::above,0},{dir16::below,0} }; 
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    double plantGrowthPercent = 0; //식물의 성장 정도를 나타내는 퍼센트

    float energyPercent = 100.0f; //0.0~100.0 프롭의 에너지이며 모닥불 등의 연료 시스템에 사용됨

    Prop(Point3 inputCoor, int leadItemCode);

    ~Prop();

    void setGrid(int inputGridX, int inputGridY, int inputGridZ) override;
    void updateSprIndex();
    bool runAI();
    bool runAnimation(bool shutdown);
    void drawSelf() override;

    void propTurnOn();
    void propTurnOff();

    //▼전기 회로
    double getTotalChargeFlux();
    bool isChargeFlowing();
    void initChargeFlux();
    double getInletCharge();
    double getOutletCharge();
    void updateCircuitNetwork();
    bool isCableConnected(Point3 currentCoord, dir16 dir);
    bool isCableConnected(Prop* currentProp, dir16 dir);
    //@brief 스위치나 그 외 요소를 무시하고 순수하게 연결만 되었는지 확인
    bool isCableLinked(Point3 currentCoord, dir16 dir);
    bool isCableLinked(Prop* currentProp, dir16 dir);
    bool isGround(Point3 currentCoord, dir16 dir);
    void transferCharge(Prop* donorProp, Prop* acceptorProp, double txChargeAmount, const std::wstring& indent, dir16 txDir, bool isGroundTransfer);
    double pushCharge(Prop* donorProp, dir16 txDir, double txChargeAmount, std::unordered_set<Prop*> pathVisited, int depth);
    void divideCharge(Prop* propPtr, double inputCharge, std::vector<dir16> possibleDirs, std::unordered_set<Prop*> pathVisited, int depth);
    void initChargeBFS(std::queue<Point3> startPointSet);
    bool hasGround();
    void loadAct();

    //▼유체 회로
    double getTotalFluidFlux();
    bool isFluidFlowing();
    void initFluidFlux();
    double getInletFluid();
    double getOutletFluid();
    void updateFluidCircuitNetwork();
    bool isPipeConnected(Point3 currentCoord, dir16 dir);
    bool isPipeConnected(Prop* currentProp, dir16 dir);
    //@brief 밸브나 그 외 요소를 무시하고 순수하게 연결만 되었는지 확인
    bool isPipeLinked(Point3 currentCoord, dir16 dir);
    bool isPipeLinked(Prop* currentProp, dir16 dir);
    dir16 getHoleDirection();
    bool isSink();
    bool isSameFluid(Prop* prop1, Prop* prop2);
    double pushFluid(Prop* donorProp, dir16 txDir, double txChargeAmount, std::unordered_set<Prop*> pathVisited, int depth);
    void transferFluid(Prop* thisProp, Prop* nextProp, double txChargeAmount, const std::wstring& indent, dir16 txDir, bool isGroundTransfer = false);
    void divideFluid(Prop* propPtr, double inputCharge, std::vector<dir16> possibleDirs, std::unordered_set<Prop*> pathVisited, int depth);
    void initFluidBFS(std::queue<Point3> startPointSet);
    void loadFluidAct();
};
