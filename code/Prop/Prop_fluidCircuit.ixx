export module Prop_fluidCircuit;

import Prop;
import util;
import globalVar;
import wrapVar;

constexpr double TIME_PER_TURN = 60.0;
constexpr double EPSILON = 0.000001;
constexpr int PUMP_POWER = 30000; // 펌프는 일단 1분에 30000mL(30L) 수송 가능


bool Prop::hasSink()
{
    if (leadItem.fluidDemand > 0) return true;
    return false;
}

double Prop::getTotalFluidFlux()
{
    return (fluidFlux[dir16::right] + fluidFlux[dir16::up] + fluidFlux[dir16::left] + fluidFlux[dir16::down] + fluidFlux[dir16::above] + fluidFlux[dir16::below]);
}

bool Prop::isFluidFlowing()
{
    return fluidFlux[dir16::right] != 0
        || fluidFlux[dir16::up] != 0
        || fluidFlux[dir16::left] != 0
        || fluidFlux[dir16::down] != 0
        || fluidFlux[dir16::above] != 0
        || fluidFlux[dir16::below] != 0;
}

void Prop::initFluidFlux()
{
    fluidFlux[dir16::right] = 0;
    fluidFlux[dir16::up] = 0;
    fluidFlux[dir16::left] = 0;
    fluidFlux[dir16::down] = 0;
    fluidFlux[dir16::above] = 0;
    fluidFlux[dir16::below] = 0;
}

double Prop::getInletFluid()
{
    double totalInlet = 0;
    if (fluidFlux[dir16::right] > 0) totalInlet += fluidFlux[dir16::right];
    if (fluidFlux[dir16::up] > 0) totalInlet += fluidFlux[dir16::up];
    if (fluidFlux[dir16::left] > 0) totalInlet += fluidFlux[dir16::left];
    if (fluidFlux[dir16::down] > 0) totalInlet += fluidFlux[dir16::down];
    if (fluidFlux[dir16::above] > 0) totalInlet += fluidFlux[dir16::above];
    if (fluidFlux[dir16::below] > 0) totalInlet += fluidFlux[dir16::below];

    return totalInlet;
}

double Prop::getOutletFluid()
{
    double totalOutlet = 0;
    if (fluidFlux[dir16::right] < 0) totalOutlet -= fluidFlux[dir16::right];
    if (fluidFlux[dir16::up] < 0) totalOutlet -= fluidFlux[dir16::up];
    if (fluidFlux[dir16::left] < 0) totalOutlet -= fluidFlux[dir16::left];
    if (fluidFlux[dir16::down] < 0) totalOutlet -= fluidFlux[dir16::down];
    if (fluidFlux[dir16::above] < 0) totalOutlet -= fluidFlux[dir16::above];
    if (fluidFlux[dir16::below] < 0) totalOutlet -= fluidFlux[dir16::below];

    return totalOutlet;
}

void Prop::updateFluidCircuitNetwork()
{
    if (debug::printCircuitLog) std::wprintf(L"------------------------- 유체 회로망 업데이트 시작 ------------------------\n");
    int cursorX = getGridX();
    int cursorY = getGridY();
    int cursorZ = getGridZ();

    std::queue<Point3> frontierQueue;
    std::unordered_set<Point3, Point3::Hash> visitedSet;
    std::vector<Prop*> pumpPropVec;
    std::vector<Prop*> tankPropVec;


    int circuitTotalSink = 0;

    //==============================================================================
    // 1. 회로 최초 탐색(BFS)
    //==============================================================================

    if (saveFrontierQueue.size() > 0 && saveVisitedSet.size() > 0)
    {
        if (debug::printCircuitLog) std::wprintf(L"------------------------- 이전 유체 회로망 탐색 결과 불러오기 ------------------------\n");
        frontierQueue = saveFrontierQueue;
        visitedSet = saveVisitedSet;
    }
    else frontierQueue.push({ cursorX, cursorY, cursorZ });

    while (!frontierQueue.empty())
    {
        Point3 current = frontierQueue.front();
        frontierQueue.pop();

        Prop* currentProp = TileProp(current.x, current.y, current.z);

        if (visitedSet.find(current) != visitedSet.end()) continue;
        visitedSet.insert(current);

        if (debug::printCircuitLog)
        {
            std::wstring powerState = L"";
            if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON)) powerState = L" [ON]";
            else if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF)) powerState = L" [OFF]";

            std::wprintf(L"[BFS 탐색] %ls (%d,%d,%d)%ls\n",
                currentProp->leadItem.name.c_str(),
                current.x, current.y, current.z,
                powerState.c_str());
        }

        if (currentProp == nullptr)
        {
            std::wprintf(L"[경고] BFS가 nullptr 프롭에 도달함 (%d,%d,%d)\n", current.x, current.y, current.z);
            continue;
        }

        if (currentProp && (currentProp->leadItem.checkFlag(itemFlag::FLUID_CIRCUIT) || currentProp->leadItem.checkFlag(itemFlag::PIPE)))
        {
            currentProp->runUsed = true; //runUsed는 전자회로에서도 동시에 동작하는데... 나중에 꼭 생각해볼 것

            currentProp->totalResistFluid = 0;

            if (currentProp->leadItem.itemCode == itemRefCode::pumpR
                || currentProp->leadItem.itemCode == itemRefCode::pumpU
                || currentProp->leadItem.itemCode == itemRefCode::pumpL
                || currentProp->leadItem.itemCode == itemRefCode::pumpD)
            {
                if (debug::printCircuitLog)
                {
                    std::wprintf(L"  \x1b[33m★ 펌프 감지: %ls \x1b[0m\n",
                        currentProp->leadItem.name.c_str());
                }

                if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) &&
                    currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
                {
                    pumpPropVec.push_back(currentProp);
                }
            }
            else if (currentProp->leadItem.itemCode == itemRefCode::fluidTank)
            {
                if (debug::printCircuitLog)
                {
                    std::wprintf(L"  \x1b[33m★ 탱크 감지: %ls \x1b[0m\n",
                        currentProp->leadItem.name.c_str());
                }

                tankPropVec.push_back(currentProp);
            }

            if (currentProp->leadItem.fluidDemand > 0)
            {
                if (debug::printCircuitLog)
                {
                    std::wprintf(L"  \x1b[91m◆ 싱크 감지: %ls (소비: %d mL)\x1b[0m\n",
                        currentProp->leadItem.name.c_str(),
                        currentProp->leadItem.fluidDemand);
                }
                circuitTotalSink += currentProp->leadItem.fluidDemand;
            }


            const dir16 directions[] = { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below };

            for (int i = 0; i < 6; ++i)
            {
                int dx, dy, dz;
                dirToXYZ(directions[i], dx, dy, dz);
                Point3 nextCoord = { current.x + dx, current.y + dy, current.z + dz };
                Prop* nextProp = TileProp(nextCoord.x, nextCoord.y, nextCoord.z);

                if (isPipeConnected(current, directions[i]))
                {
                    ItemData& nextItem = nextProp->leadItem;
                    if (debug::printCircuitLog)
                        std::wprintf(L"  [연결] %ls (%d,%d) %ls\n",
                            dirToArrow(directions[i]), nextCoord.x, nextCoord.y, nextItem.name.c_str());
                    frontierQueue.push(nextCoord);
                }
            }

        }
    }


    //==============================================================================
    // 3. 펌프 출력 시작
    //==============================================================================

    randomVectorShuffle(pumpPropVec);
    randomVectorShuffle(tankPropVec);

    int totalAvailablePower = 0;
    for (Prop* pumpProp : pumpPropVec)
    {
        if (pumpProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) &&
            pumpProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
        {
            totalAvailablePower += PUMP_POWER;
        }
    }


    //for (Prop* pumpProp : pumpPropVec)
    //{
    //    constexpr double LOSS_COMPENSATION_FACTOR = 1.2;
    //    // ↑ 배관손실로 인해 전부 전달되지 않으므로, 
    //    //   싱크에 가능하면 정확히 필요한 양이 도달하도록 20% 여유분 추가

    //    int x = pumpProp->getGridX();
    //    int y = pumpProp->getGridY();
    //    int z = pumpProp->getGridZ();
    //    double pumpRatio = (double)(PUMP_POWER) / (double)totalAvailablePower;
    //    double voltOutputPower = myMin(std::ceil(circuitTotalSink * pumpRatio), PUMP_POWER);
    //    voltProp->prevPushedCharge = 0;
    //    voltOutputPower *= LOSS_COMPENSATION_FACTOR;  // 저항손실 보존 변수 (기본값 120%)

    //    if (debug::printCircuitLog) std::wprintf(L"========================▼전압원 (%d,%d)%ls : 밀어내기 시작▼========================\n", x, y, voltProp->leadItem.name.c_str());
    //    if (voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) || voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
    //    {
    //        double finalVoltOutput = voltOutputPower;
    //        if (voltProp->leadItem.itemCode == itemRefCode::powerBankR || voltProp->leadItem.itemCode == itemRefCode::powerBankL)
    //            finalVoltOutput = std::min(voltOutputPower, voltProp->leadItem.powerStorage);


    //        if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT) && isConnected({ x,y,z }, dir16::right))
    //            voltProp->prevPushedCharge += pushCharge(voltProp, dir16::right, finalVoltOutput, {}, 0);
    //        else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_UP) && isConnected({ x,y,z }, dir16::up))
    //            voltProp->prevPushedCharge += pushCharge(voltProp, dir16::up, finalVoltOutput, {}, 0);
    //        else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT) && isConnected({ x,y,z }, dir16::left))
    //            voltProp->prevPushedCharge += pushCharge(voltProp, dir16::left, finalVoltOutput, {}, 0);
    //        else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_DOWN) && isConnected({ x,y,z }, dir16::down))
    //            voltProp->prevPushedCharge += pushCharge(voltProp, dir16::down, finalVoltOutput, {}, 0);

    //        voltProp->nodeCharge = voltProp->nodeMaxCharge;
    //    }
    //}

    //if (debug::printCircuitLog)
    //{
    //    std::wprintf(L"======================== 회로망 요약 ========================\n");
    //    std::wprintf(L"노드: %zu개, 전압원: %zu개, 총부하: %d, 총전력: %d\n",
    //        visitedSet.size(), voltagePropVec.size(), circuitTotalSink, circuitMaxEnergy);
    //}
}