export module Prop_fluidCircuit;

import Prop;
import util;
import constVar;
import globalVar;
import wrapFunc;

constexpr double TIME_PER_TURN = 60.0;
constexpr double EPSILON = 0.000001;
constexpr int PUMP_POWER = 30000; // 펌프는 일단 1분에 30000mL(30L) 수송 가능
constexpr int INFINITE_DEMAND_INT = std::numeric_limits<int>::max();
constexpr double INFINITE_DEMAND_DOUBLE = std::numeric_limits<double>::max();

/*
* <취수 배관> : intakePipeR, intakePipeU, intakePipeL, intakePipeD
* 유체 타일 위에 있는 취수 파이프, 연결 가능한 방향이 단 한 방향 뿐이다.
* 바로 옆에 펌프가 설치될 경우 펌프가 이 취수배관의 유체를 다른 타일(자기 방향)로 밀어낸다.
* 
* <유체 탱크> : fluidTank
* 유체를 저장한다. 탱크 옆에 펌프가 설치될 경우 탱크의 유체를 펌프 방향으로 밀어낸다.
* 또한 펌프 이외에도 현재 자신에 저장된 양(적절한 수위)에 따라 독립적인 유압원으로 펌프 작동 후에 퍼진다.
* 
* <펌프> : pumpR, pumpU, pumpL, pumpD
* 펌프 출력 방향의 반대편에 있는 타일의 유체를 끌어서 펌프 방향으로 보낸다.
* 예를 들어 pumpR은 왼쪽에 취수배관이나 탱크가 있을 경우 그 유체를 오른쪽으로 밀어낸다.
* 펌프는 전기회로로 작동한다. 지금은 ON/OFF만 판단하지만 나중에는 공급된 전력에 따라 밀어내는 양을 수정할 예정
* 
* <스프링클러> : sprinklerRL, sprinklerUD
* 전기회로에서의 부하에 해당한다. 파이프의 끝부분의 뚫린 구멍과 더불어 유이한 싱크이다. 조금이라도 흐르면 작동하지만 일정 임계점을 넘으면
* 넓게 흩뿌려지면서 주변 타일을 젖음 상태로 만든다. 이는 농사에 사용된다. 유압이 강하게 공급될 수록 더 널리 흩뿌린다.
* 
* <밸브, 솔레노이드 밸브>
* 전자회로에서의 스위치 개념, 솔레노이드 밸브는 플레이어가 누르는게 아니라 전력이 공급되면 온오프가 된다.
* 
* <수직배관,수직꺾임배관> verticalPipe
* z축으로 다른 층과 연결된 파이프이다.
* Vertical Pipe : 위층과 아래층만 연결
* Vertical Pipe ↱(RB) : 아래층과 오른쪽 방향의 타일을 연결
* Vertical Pipe ↰(LB) : 아래층과 왼쪽 방향의 타일을 연결
* Vertical Pipe ↳(RA) : 윗층과 오른쪽 방향의 타일을 연결
* Vertical Pipe ↲ (LA) : 윗층과 왼쪽 방향의 타일을 연결
* 
* 자세한 아이템들의 refCode는 constVar.ixx를 참조할 것
* 
* [작동 순서]
* 1. [펌프단계] 펌프들이 자신이 할 수 있는만큼 최대치의 양을 밀어낸다. (전자회로처럼 셔플없고 병렬 분배도 없음)
* 2. [탱크단계] 탱크들이 현재 저장된 유량(적절한 비례상수로 구현된 수위)에 따라 압력을 발생시켜 좌우로 밀어내기를 발생시킨다.
* 3. [평활화단계] 유체파이프들이 주변 타일과 유량(수위)가 다르면 주변 타일과 평활화가 되는 transfer가 일어난다(push가 아님에 유의)
* 4. 다 한 후에 싱크로 전달된 유량에 따라 다양한 작동이 일어남. 현재 있는 싱크는 2개인데 첫번째는 파이프의 구멍, 두번째는 스프링클러이다.
*  4-1. 구멍은 무한한 SINK값을 가진다.
*  4-2. 스프링클러는 유한한 SINK값을 가진다.
* 5. fluidSink에 누적된 값에서 스프링클러가 우선 사용된다. 
* 5. 스프링클러는 유량이 조금이라도 흡수됐으면 주변에 물을 흩뿌림. 단 주변 타일을 젖게 만드려면 더 넓게 젖게 만드려면 싱크의 유량 일정량 필요
* 6. fluidSink에서 스프링클러 소모량을 제외한 나머지는 구멍으로 흘러내리며 물웅덩이 타일을 만듬. 이건 아마 벼농사같은 곳에 사용될 듯 하다.
* 
* [※ 유의할 점]
* -펌프와 탱크의 셔플이 일어나지 않아 모든 턴에서 작동 순서는 항상 같다.
* -배관저항(Q^2R)으로 인해 발생하는 손실은 원래 지나가야할 유체를 조금 덜 지나가게 만든다. 전기회로랑 다르게 열로 손실되는 값이 아니다.
* 포화에 이르기까지의 시간을 지연시키고 프로파일을 자연스럽게 만드는 역할을 한다. 너무 과하게 만들지는 말 것.
* -펌프의 직병렬은 너무 복잡하게 생각하지 말 것. 어느 정도 물리적으로 어긋나도 이거는 그냥 넘어가자.
* 전자회로랑 다르게 유압회로까지 한눈에 정확하게 직관적으로 계산할 수 있는 사람은 많지않다.
* -전송할 위치에 유체의 종류가 다를 경우 섞이지 않고 완전히 막히게 설계해야 한다. 유의할 것
*/



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
    std::unordered_set<Prop*> loadSinkSet;


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

        if (currentProp && (currentProp->leadItem.checkFlag(itemFlag::FLUID_CIRCUIT)))
        {
            currentProp->fluidRunUsed = true;

            currentProp->totalResistFluid = 0;

            if (currentProp->leadItem.itemCode == itemID::pumpR
                || currentProp->leadItem.itemCode == itemID::pumpU
                || currentProp->leadItem.itemCode == itemID::pumpL
                || currentProp->leadItem.itemCode == itemID::pumpD)
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
            else if (currentProp->leadItem.itemCode == itemID::fluidTank)
            {
                if (debug::printCircuitLog)
                {
                    std::wprintf(L"  \x1b[33m★ 탱크 감지: %ls \x1b[0m\n",
                        currentProp->leadItem.name.c_str());
                }

                tankPropVec.push_back(currentProp);
            }
            else if (currentProp->leadItem.itemCode == itemID::intakePipeR
                || currentProp->leadItem.itemCode == itemID::intakePipeU
                || currentProp->leadItem.itemCode == itemID::intakePipeL
                || currentProp->leadItem.itemCode == itemID::intakePipeD)
            {
                if (TileFloor(current) == itemID::deepFreshWater || TileFloor(current) == itemID::shallowFreshWater)
                {
                    currentProp->nodeFluidAmount = currentProp->leadItem.maxFluid;
                    currentProp->nodeFluidType = fluidType::WATER;
                }
                else if (TileFloor(current) == itemID::deepSeaWater || TileFloor(current) == itemID::shallowSeaWater)
                {
                    currentProp->nodeFluidAmount = currentProp->leadItem.maxFluid;
                    currentProp->nodeFluidType = fluidType::SEAWATER;
                }
            }

            if (currentProp->isSink())
            {
                loadSinkSet.insert(currentProp);

                if (debug::printCircuitLog)
                {
                    std::wprintf(L"  \x1b[91m◆ 싱크 감지: %ls (%d,%d,%d)\x1b[0m\n",
                        currentProp->leadItem.name.c_str(),
                        current.x, current.y, current.z);
                }
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
    // 2. 펌프 출력 시작
    //==============================================================================

    for (Prop* pumpProp : pumpPropVec)
    {
        int x = pumpProp->getGridX();
        int y = pumpProp->getGridY();
        int z = pumpProp->getGridZ();

        if (debug::printCircuitLog) std::wprintf(L"========================▼펌프 (%d,%d) : 밀어내기 시작▼========================\n", x, y);
        if (pumpProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) &&
            pumpProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
        {
            if (pumpProp->leadItem.itemCode == itemID::pumpR)
            {
                if (isPipeConnected({ x,y,z }, dir16::left))
                {
                    Prop* srcProp = TileProp(x - 1, y, z);
                    if (srcProp && (srcProp->leadItem.itemCode == itemID::fluidTank
                        || srcProp->leadItem.itemCode == itemID::intakePipeR))
                    {
                        pushFluid(srcProp, dir16::right, std::min((double)PUMP_POWER, srcProp->nodeFluidAmount), {}, 0);
                    }
                }
            }
            else if (pumpProp->leadItem.itemCode == itemID::pumpL)
            {
                if (isPipeConnected({ x,y,z }, dir16::right))
                {
                    Prop* srcProp = TileProp(x + 1, y, z);
                    if (srcProp && (srcProp->leadItem.itemCode == itemID::fluidTank
                        || srcProp->leadItem.itemCode == itemID::intakePipeL))
                    {
                        pushFluid(srcProp, dir16::left, std::min((double)PUMP_POWER, srcProp->nodeFluidAmount), {}, 0);
                    }
                }
            }
            else if (pumpProp->leadItem.itemCode == itemID::pumpU)
            {
                if (isPipeConnected({ x,y,z }, dir16::down))
                {
                    Prop* srcProp = TileProp(x, y + 1, z);
                    if (srcProp && srcProp->leadItem.itemCode == itemID::intakePipeU)
                    {
                        pushFluid(srcProp, dir16::up, std::min((double)PUMP_POWER, srcProp->nodeFluidAmount), {}, 0);
                    }
                }
            }
            else if (pumpProp->leadItem.itemCode == itemID::pumpD)
            {
                if (isPipeConnected({ x,y,z }, dir16::up))
                {
                    Prop* srcProp = TileProp(x, y - 1, z);
                    if (srcProp && srcProp->leadItem.itemCode == itemID::intakePipeD)
                    {
                        pushFluid(srcProp, dir16::down, std::min((double)PUMP_POWER, srcProp->nodeFluidAmount), {}, 0);
                    }
                }
            }
        }
    }


    //==============================================================================
    // 3. sinkSet(구멍,스프링클러 등) 작동 시작
    //==============================================================================
    for (auto sinkProp : loadSinkSet)
    {
        // 0. 초기값 설정
        double totalInlet = sinkProp->sinkFluidAmount; // 이번 턴에 들어온 총 유량
        double remainFluid = totalInlet;

        double consumedByDevice = 0.0;
        double leakedByHole = 0.0;

        // fluidDemand가 설정된 프롭은 해당 양만큼 우선 소비함
        if (sinkProp->leadItem.fluidDemand > 0)
        {
            consumedByDevice = std::min(remainFluid, (double)sinkProp->leadItem.fluidDemand);
            remainFluid -= consumedByDevice;
        }

        // 구멍이 하나라도 존재하면 남은 유체는 모두 밖으로 배출됨 (무한 싱크)
        bool hasHole = false;
        for (auto dir : { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below })
        {
            Point3 coord = { sinkProp->getGridX(), sinkProp->getGridY(), sinkProp->getGridZ() };
            // 현재 타일에서 해당 방향으로 구멍이 뚫려있는지 확인
            if (sinkProp->getHoleDirection() != dir16::none)
            {
                hasHole = true;
                break;
            }
        }

        if (hasHole)
        {
            leakedByHole = remainFluid; // 구멍은 무한히 받아들이므로 남은 전량 누수
            remainFluid = 0.0;
        }

        // 3. 로그 출력 (std::wprintf 사용)
        if (totalInlet > EPSILON)
        {
            if (debug::printCircuitLog)
            {
                std::wprintf(L"  \x1b[96m▶ [SINK 처리] (%d,%d)%ls \x1b[0m\n",
                    sinkProp->getGridX(), sinkProp->getGridY(), sinkProp->leadItem.name.c_str());

                std::wprintf(L"      │ 총 유입량: %.2f mL\n", totalInlet);
            }

            if (consumedByDevice > EPSILON)
            {
                /*
                * 스프링클러는 요구량의 50%만 접수돼도 켜짐 상태가 된다.
                * 단 50%~99.9% 구간은 주변 3*3 타일까지만 물을 흩뿌릴 수 있다.
                * 100% 구간부터는 5*5까지 물을 흩뿌릴 수 있다.
                */

                if (consumedByDevice >= sinkProp->leadItem.fluidDemand / 2)
                {
                    sinkProp->leadItem.eraseFlag(itemFlag::PROP_POWER_OFF);
                    sinkProp->leadItem.addFlag(itemFlag::PROP_POWER_ON);
                }
                else
                {
                    sinkProp->leadItem.eraseFlag(itemFlag::PROP_POWER_ON);
                    sinkProp->leadItem.addFlag(itemFlag::PROP_POWER_OFF);
                }


                if (debug::printCircuitLog) std::wprintf(L"      │ ├─ \x1b[32m[장치소비] %.2f / %d (충족률: %.1f%%)\x1b[0m\n",
                    consumedByDevice,
                    sinkProp->leadItem.fluidDemand,
                    (consumedByDevice / sinkProp->leadItem.fluidDemand) * 100.0);
            }

            if (leakedByHole > EPSILON) //누수 알고리즘
            {
                if (debug::printCircuitLog) std::wprintf(L"      │ └─ \x1b[34m[누수발생] %.2f (구멍으로 배출)\x1b[0m\n",leakedByHole);
                
                sinkProp->jetFluidType = sinkProp->sinkFluidType;
                sinkProp->jetFluidDir = sinkProp->getHoleDirection();
                Point3 del = dir2Coord(sinkProp->jetFluidDir);
                addItemToTile(sinkProp->getGrid() + del, fluidTypeToCode(sinkProp->jetFluidType), std::floor(leakedByHole));
            }
            else if (hasHole && leakedByHole <= EPSILON)
            {
                if (debug::printCircuitLog) std::wprintf(L"      │ └─ \x1b[90m[누수없음] 잔여 유량 없음\x1b[0m\n");
            }
            else if (!hasHole && remainFluid > EPSILON)
            {
                // 구멍이 없고 Demand보다 많이 들어온 경우 (막힌 관 끝에 압력이 차는 상황 등)
                if (debug::printCircuitLog) std::wprintf(L"      │ └─ \x1b[33m[잔류] %.2f (배출구 없음)\x1b[0m\n", remainFluid);
            }

            if (debug::printCircuitLog) std::wprintf(L"      └──────────────────────────────────\n");
        }

    }


    if (debug::printCircuitLog)
    {
        std::wprintf(L"======================== 유압 회로망 요약 ========================\n");
        std::wprintf(L"노드: %zu개, 펌프: %zu개\n",
            visitedSet.size(), pumpPropVec.size());
    }
}

bool Prop::isPipeConnected(Point3 currentCoord, dir16 dir)
{
    Prop* currentProp = TileProp(currentCoord.x, currentCoord.y, currentCoord.z);
    errorBox(currentProp == nullptr, L"currentProp is nullptr in isPipeConnected");
    ItemData& crtItem = currentProp->leadItem;

    Point3 delCoord = { 0,0,0 };
    itemFlag hostFlag, guestFlag;
    switch (dir)
    {
    case dir16::right:
        delCoord = { +1,0,0 };
        hostFlag = itemFlag::PIPE_CNCT_RIGHT;
        guestFlag = itemFlag::PIPE_CNCT_LEFT;
        break;
    case dir16::up:
        delCoord = { 0,-1,0 };
        hostFlag = itemFlag::PIPE_CNCT_UP;
        guestFlag = itemFlag::PIPE_CNCT_DOWN;
        break;
    case dir16::left:
        delCoord = { -1,0,0 };
        hostFlag = itemFlag::PIPE_CNCT_LEFT;
        guestFlag = itemFlag::PIPE_CNCT_RIGHT;
        break;
    case dir16::down:
        delCoord = { 0,+1,0 };
        hostFlag = itemFlag::PIPE_CNCT_DOWN;
        guestFlag = itemFlag::PIPE_CNCT_UP;
        break;
    case dir16::above:
        delCoord = { 0,0,+1 };
        hostFlag = itemFlag::PIPE_CNCT_ABOVE;
        guestFlag = itemFlag::PIPE_CNCT_BELOW;
        break;
    case dir16::below:
        delCoord = { 0,0,-1 };
        hostFlag = itemFlag::PIPE_CNCT_BELOW;
        guestFlag = itemFlag::PIPE_CNCT_ABOVE;
        break;
    default:
        errorBox(L"[Error] isPipeConnected lambda function received invalid direction argument.\n");
        break;
    }
    Prop* targetProp = TileProp(currentCoord.x + delCoord.x, currentCoord.y + delCoord.y, currentCoord.z + delCoord.z);
    if (targetProp == nullptr) return false;
    ItemData& tgtItem = targetProp->leadItem;

    if (crtItem.itemCode == itemID::valveRL
        || crtItem.itemCode == itemID::valveUD
        || crtItem.itemCode == itemID::solenoidValveRL
        || crtItem.itemCode == itemID::solenoidValveUD)
    {
        if (crtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }


    if ((dir == dir16::right || dir == dir16::left) 
        && (tgtItem.itemCode == itemID::valveRL || tgtItem.itemCode == itemID::solenoidValveRL)
        && tgtItem.checkFlag(itemFlag::PROP_POWER_OFF))
    {
        return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) 
        && (tgtItem.itemCode == itemID::valveUD || tgtItem.itemCode == itemID::solenoidValveUD)
        && tgtItem.checkFlag(itemFlag::PROP_POWER_OFF))
    {
        return false;
    }

    if (dir == dir16::above || dir == dir16::below)
    {
        //전선과 달리 일반파이프는 z축 연결에 사용 불가
        bool currentCondition = currentProp->leadItem.checkFlag(hostFlag);
        bool targetCondition = targetProp->leadItem.checkFlag(guestFlag);

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else if (dir == dir16::right || dir == dir16::up || dir == dir16::left || dir == dir16::down)
    {
        bool currentCondition = (currentProp->leadItem.checkFlag(itemFlag::PIPE) || currentProp->leadItem.checkFlag(hostFlag));
        bool targetCondition = (targetProp->leadItem.checkFlag(itemFlag::PIPE) || targetProp->leadItem.checkFlag(guestFlag));

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else errorBox(L"[Error] isPipeConnected lambda function received invalid direction argument.\n");
}

bool Prop::isPipeConnected(Prop* currentProp, dir16 dir)
{
    return isPipeConnected({ currentProp->getGridX(),currentProp->getGridY(),currentProp->getGridZ() }, dir);
}

bool Prop::isPipeLinked(Point3 currentCoord, dir16 dir)
{
    Prop* currentProp = TileProp(currentCoord.x, currentCoord.y, currentCoord.z);
    errorBox(currentProp == nullptr, L"currentProp is nullptr in isPipeLinked");

    Point3 delCoord = { 0,0,0 };
    itemFlag hostFlag, guestFlag;
    switch (dir)
    {
    case dir16::right:
        delCoord = { +1,0,0 };
        hostFlag = itemFlag::PIPE_CNCT_RIGHT;
        guestFlag = itemFlag::PIPE_CNCT_LEFT;
        break;
    case dir16::up:
        delCoord = { 0,-1,0 };
        hostFlag = itemFlag::PIPE_CNCT_UP;
        guestFlag = itemFlag::PIPE_CNCT_DOWN;
        break;
    case dir16::left:
        delCoord = { -1,0,0 };
        hostFlag = itemFlag::PIPE_CNCT_LEFT;
        guestFlag = itemFlag::PIPE_CNCT_RIGHT;
        break;
    case dir16::down:
        delCoord = { 0,+1,0 };
        hostFlag = itemFlag::PIPE_CNCT_DOWN;
        guestFlag = itemFlag::PIPE_CNCT_UP;
        break;
    case dir16::above:
        delCoord = { 0,0,+1 };
        hostFlag = itemFlag::PIPE_CNCT_ABOVE;
        guestFlag = itemFlag::PIPE_CNCT_BELOW;
        break;
    case dir16::below:
        delCoord = { 0,0,-1 };
        hostFlag = itemFlag::PIPE_CNCT_BELOW;
        guestFlag = itemFlag::PIPE_CNCT_ABOVE;
        break;
    default:
        errorBox(L"[Error] isPipeLinked received invalid direction argument.\n");
        break;
    }
    Prop* targetProp = TileProp(currentCoord.x + delCoord.x, currentCoord.y + delCoord.y, currentCoord.z + delCoord.z);
    if (targetProp == nullptr) return false;

    if (dir == dir16::above || dir == dir16::below)
    {
        bool currentCondition = currentProp->leadItem.checkFlag(hostFlag);
        bool targetCondition = targetProp->leadItem.checkFlag(guestFlag);

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else if (dir == dir16::right || dir == dir16::up || dir == dir16::left || dir == dir16::down)
    {
        bool currentCondition = (currentProp->leadItem.checkFlag(itemFlag::PIPE) || currentProp->leadItem.checkFlag(hostFlag));
        bool targetCondition = (targetProp->leadItem.checkFlag(itemFlag::PIPE) || targetProp->leadItem.checkFlag(guestFlag));

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else errorBox(L"[Error] isPipeLinked received invalid direction argument.\n");
}

bool Prop::isPipeLinked(Prop* currentProp, dir16 dir)
{
    return isPipeLinked({ currentProp->getGridX(),currentProp->getGridY(),currentProp->getGridZ() }, dir);
}

dir16 Prop::getHoleDirection()
{
    if (leadItem.checkFlag(itemFlag::PIPE))
    {
        if (isPipeLinked(this, dir16::left)
            && !isPipeLinked(this, dir16::up)
            && !isPipeLinked(this, dir16::down)
            && !isPipeLinked(this, dir16::right))
        {
            return dir16::right;
        }

        if (!isPipeLinked(this, dir16::left)
            && isPipeLinked(this, dir16::up)
            && !isPipeLinked(this, dir16::down)
            && !isPipeLinked(this, dir16::right))
        {
            return dir16::down;
        }

        if (!isPipeLinked(this, dir16::left)
            && !isPipeLinked(this, dir16::up)
            && isPipeLinked(this, dir16::down)
            && !isPipeLinked(this, dir16::right))
        {
            return dir16::up;
        }


        if (!isPipeLinked(this, dir16::left)
            && !isPipeLinked(this, dir16::up)
            && !isPipeLinked(this, dir16::down)
            && isPipeLinked(this, dir16::right))
        {
            return dir16::left;
        }
    }
    else // 일반 유체 부품
    {
        //일단은 유체 부품은 CNCT가 최대 2개인 경우밖에 없어 구멍이 2개 생길 여지는 없다
        if (leadItem.checkFlag(itemFlag::PIPE_CNCT_RIGHT) && !isPipeLinked(this, dir16::right))
            return dir16::right;
        if (leadItem.checkFlag(itemFlag::PIPE_CNCT_UP) && !isPipeLinked(this, dir16::up))
            return dir16::up;
        if (leadItem.checkFlag(itemFlag::PIPE_CNCT_LEFT) && !isPipeLinked(this, dir16::left))
            return dir16::left;
        if (leadItem.checkFlag(itemFlag::PIPE_CNCT_DOWN) && !isPipeLinked(this, dir16::down))
            return dir16::down;
    }

    return dir16::none;
}

bool Prop::isSink()
{
    if (leadItem.fluidDemand > 0) return true;
    if (getHoleDirection() != dir16::none) return true;
    return false;
}

//2개의 프롭의 유체 종류가 같은지 비교(한쪽이라도 NONE일 경우 같다고 반환)
//connect 체크를 하지 않음에 유의할 것
bool Prop::isSameFluid(Prop* prop1, Prop* prop2)
{
    errorBox(prop1 == nullptr || prop2 == nullptr, L"Inserted parameter is nullptr in isSameFluid");
    if (prop1->nodeFluidType == fluidType::NONE || prop2->nodeFluidType == fluidType::NONE) return true;
    if (prop1->nodeFluidType == prop2->nodeFluidType) return true;
    return false;
}

double Prop::pushFluid (Prop* donorProp, dir16 txDir, double txFluidAmount, std::unordered_set<Prop*> pathVisited, int depth)
{
    errorBox(donorProp == nullptr, L"[Error] pushFluid: null donor\n");
    int dx, dy, dz;
    dirToXYZ(txDir, dx, dy, dz);
    Point3 nextCoord = { donorProp->getGridX() + dx, donorProp->getGridY() + dy, donorProp->getGridZ() + dz };
    Prop* nextProp = TileProp(nextCoord);
    errorBox(nextProp == nullptr, L"[Error] pushFluid: no acceptor found\n");

    txFluidAmount = std::min(donorProp->nodeFluidAmount, txFluidAmount);

    errorBox(txFluidAmount > donorProp->nodeFluidAmount + EPSILON, L"[Error] pushFluid: insufficient fluid\n");
    errorBox(!isPipeConnected({ donorProp->getGridX(), donorProp->getGridY(), donorProp->getGridZ() }, txDir),
        L"[Error] pushFluid: not connected\n");

    if (isSameFluid(donorProp, nextProp) == false) return 0; //종류가 다른 유체일 경우 섞이지 않고 벽처럼 작동함

    std::wstring indent(depth * 2, L' ');

    if (pathVisited.find(donorProp) != pathVisited.end())
    {
        if (debug::printCircuitLog)
            std::wprintf(L"%s[PUSH-SKIP] (%d,%d)%ls 이미 방문됨\n", indent.c_str(), donorProp->getGridX(), donorProp->getGridY(), donorProp->leadItem.name.c_str());
        return 0;
    }
    pathVisited.insert(donorProp);
    if (pathVisited.find(nextProp) != pathVisited.end()) return 0;


    if (debug::printCircuitLog) std::wprintf(L"%s[PUSH] (%d,%d)%ls → (%d,%d)%ls [%ls] 시도: %.2f\n",
        indent.c_str(),
        donorProp->getGridX(), donorProp->getGridY(), donorProp->leadItem.name.c_str(),
        nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
        dirToArrow(txDir),
        txFluidAmount);

    double sinkTxFluid = 0;
    Point3 current = { donorProp->getGridX(), donorProp->getGridY(), donorProp->getGridZ() };
    if (nextProp->isSink())
    {
        double requiredFluid = 0;

        if (nextProp->getHoleDirection() == dir16::none)
        {
            requiredFluid = nextProp->leadItem.fluidDemand - nextProp->sinkFluidAmount;
        }
        else
        {
            requiredFluid = INFINITE_DEMAND_DOUBLE;
        }

        if (debug::printCircuitLog)
        {
            std::wprintf(L"%s  └─ \x1b[33m[SINK진입] SINK, 요구=%ls, 잔여용량=%ls, 시도량=%.2f\x1b[0m\n",
                indent.c_str(),
                (requiredFluid == INFINITE_DEMAND_DOUBLE) ? L"\u221E"/*∞*/ : decimalCutter(nextProp->leadItem.fluidDemand, 2).c_str(),
                (requiredFluid == INFINITE_DEMAND_DOUBLE) ? L"\u221E"/*∞*/ : decimalCutter(requiredFluid, 2).c_str(),
                txFluidAmount);
        }

        if (requiredFluid > EPSILON)
        {
            sinkTxFluid = std::min(std::min(txFluidAmount, requiredFluid), nextProp->nodeFluidAmount);
            nextProp->nodeFluidAmount -= sinkTxFluid;

            if (debug::printCircuitLog)
            {
                std::wprintf(L"%s      → 실제소비=%.2f, 남은용량=%ls\n",
                    indent.c_str(),
                    sinkTxFluid,
                    (requiredFluid == INFINITE_DEMAND_DOUBLE) ? L"\u221E"/*∞*/ : decimalCutter(requiredFluid - sinkTxFluid, 2).c_str());
            }

            nextProp->sinkFluidAmount += sinkTxFluid;
            nextProp->sinkFluidType = donorProp->nodeFluidType;
        }
        else if (debug::printCircuitLog)
        {
            std::wprintf(L"%s      → \x1b[90m용량 소진됨, 스킵\x1b[0m\n", indent.c_str());
        }
    }

    double pushedFluid = std::min(txFluidAmount - sinkTxFluid, nextProp->nodeFluidAmount);

    if (pushedFluid > EPSILON)
    {
        std::vector<dir16> possibleDirs;

        for (auto dir : { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below })
        {
            if (dir == reverse(txDir)) continue;
            if (isPipeConnected({ nextProp->getGridX(), nextProp->getGridY(), nextProp->getGridZ() }, dir))
            {
                possibleDirs.push_back(dir);
            }
        }

        if (possibleDirs.empty() == false)
        {
            if (possibleDirs.size() == 1)
            {
                auto newPathVisited = pathVisited;
                pushFluid(nextProp, possibleDirs[0], pushedFluid, newPathVisited, depth + 1);
            }
            else if (possibleDirs.size() > 1)
            {
                divideFluid(nextProp, pushedFluid, possibleDirs, pathVisited, depth + 1);
            }
        }
    }

    // 재귀 복귀: 하위 노드들이 유체를 소비해서 생긴 빈 공간만큼 전송
    double finalTxFluid = std::min(txFluidAmount, nextProp->leadItem.maxFluid - nextProp->nodeFluidAmount);
    transferFluid(donorProp, nextProp, finalTxFluid, indent, txDir, false);
    return finalTxFluid;
}


void Prop::divideFluid(Prop* propPtr, double inputFluid, std::vector<dir16> possibleDirs, std::unordered_set<Prop*> pathVisited, int depth)
{
    std::wstring indent(depth * 2, L' ');

    if (debug::printCircuitLog)
    {
        std::wprintf(L"%s[DIVIDE] (%d,%d)%ls 분배시작: %.2f → %zu방향\n",
            indent.c_str(),
            propPtr->getGridX(), propPtr->getGridY(), propPtr->leadItem.name.c_str(),
            inputFluid, possibleDirs.size());
    }

    double remainingFluid = inputFluid;
    std::vector<dir16> dirsToRemove;
    dirsToRemove.reserve(6);

    int loopCount = 0;
    while (remainingFluid > EPSILON && !possibleDirs.empty())
    {
        loopCount++;
        dirsToRemove.clear();
        double pushedFluid = 0;

        double splitFluid = remainingFluid / possibleDirs.size();

        if (debug::printCircuitLog)
        {
            std::wprintf(L"%s  [DIV] %zu방향, 각 %.2f씩\n",
                indent.c_str(), possibleDirs.size(), splitFluid);
        }

        for (auto dir : possibleDirs)
        {
            auto newPathVisited = pathVisited;
            double branchPushedFluid = pushFluid(propPtr, dir, splitFluid, newPathVisited, depth + 1);
            pushedFluid += branchPushedFluid;
            if (branchPushedFluid < EPSILON) dirsToRemove.push_back(dir);
        }

        for (auto dir : dirsToRemove)
            possibleDirs.erase(std::remove(possibleDirs.begin(), possibleDirs.end(), dir), possibleDirs.end());

        remainingFluid -= pushedFluid;

        if (debug::printCircuitLog && pushedFluid > EPSILON)
        {
            std::wprintf(L"%s  [DIV-RESULT] 루프%d: 전송=%.2f, 잔여=%.2f\n",
                indent.c_str(), loopCount, pushedFluid, remainingFluid);
        }

        if (pushedFluid < EPSILON) break;
    }

    if (debug::printCircuitLog)
    {
        std::wprintf(L"%s[DIVIDE-END] (%d,%d)%ls 총 %d회 반복, 미분배=%.2f\n",
            indent.c_str(),
            propPtr->getGridX(), propPtr->getGridY(), propPtr->leadItem.name.c_str(),
            loopCount, remainingFluid);
    }
}


void Prop::transferFluid(Prop* thisProp, Prop* nextProp, double txFluidAmount, const std::wstring& indent, dir16 txDir, bool isSinkTransfer)
{
    if (txFluidAmount < EPSILON)
    {
        if (debug::printCircuitLog)
        {
            std::wprintf(L"%s[전송 스킵] (%d,%d)%ls → (%d,%d)%ls 양:%.8f (EPSILON 미만)\n",
                indent.c_str(),
                thisProp->getGridX(), thisProp->getGridY(), thisProp->leadItem.name.c_str(),
                nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
                txFluidAmount);
        }
        return;
    }

    //유체의 종류에 따라 마찰저항이 달라지도록 수정할 것
    constexpr double PIPE_RESIST = 0.000001; //일단 아무 값으로 고정
    double frictionLoss = txFluidAmount * txFluidAmount * PIPE_RESIST;

    double actualTransfer = txFluidAmount - frictionLoss;
    if (actualTransfer < 0) actualTransfer = 0;

    if (actualTransfer > thisProp->nodeFluidAmount)
        actualTransfer = thisProp->nodeFluidAmount;

    thisProp->totalResistFluid += frictionLoss;

    thisProp->nodeFluidAmount -= actualTransfer;
    thisProp->fluidFlux[txDir] -= actualTransfer;

    if (thisProp->nodeFluidAmount <= EPSILON)
    {
        thisProp->nodeFluidAmount = 0;
        thisProp->nodeFluidType = fluidType::NONE;
    }

    if (nextProp->nodeFluidAmount <= EPSILON || nextProp->nodeFluidType == fluidType::NONE)
    {
        nextProp->nodeFluidType = thisProp->nodeFluidType;
    }

    if (isSinkTransfer == false) nextProp->nodeFluidAmount += actualTransfer;

    nextProp->fluidFlux[reverse(txDir)] += actualTransfer;

    if (debug::printCircuitLog)
    {
        if (isSinkTransfer)
        {
            std::wprintf(L"\x1b[33m%s[전송 SINK] (%d,%d)%ls [%.2f→%.2f] → (%d,%d)%ls 전송:%.2f 마찰손실:%.2f 부하:%.2f/%d\x1b[0m\n",
                indent.c_str(),
                thisProp->getGridX(), thisProp->getGridY(), thisProp->leadItem.name.c_str(),
                thisProp->nodeFluidAmount + actualTransfer, thisProp->nodeFluidAmount,
                nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
                actualTransfer, frictionLoss,
                nextProp->sinkFluidAmount, nextProp->leadItem.fluidDemand);
        }
        else
        {
            std::wprintf(L"%s[전송] (%d,%d)%ls [%.2f→%.2f] → (%d,%d)%ls [%.2f/%d] 전송:%.2f 마찰손실:%.2f\n",
                indent.c_str(),
                thisProp->getGridX(), thisProp->getGridY(), thisProp->leadItem.name.c_str(),
                thisProp->nodeFluidAmount + actualTransfer, thisProp->nodeFluidAmount,
                nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
                nextProp->nodeFluidAmount, nextProp->leadItem.maxFluid,
                actualTransfer, frictionLoss);
        }
    }
}

