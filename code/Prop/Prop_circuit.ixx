import Prop;

import std;
import util;
import globalVar;
import constVar;
import World;
import globalTime;
import ItemStack;
import Entity;


constexpr double TIME_PER_TURN = 60.0;
constexpr double EPSILON = 0.000001;
constexpr double SYSTEM_VOLTAGE = 24.0; //저항 계산에만 사용됨

/*
*
* relayR,U,L,D : 트랜지스터와 동일한 방향성 가짐
*
*
* <트랜지스터>
* transistorR : 게이트핀이 우측, 상단과 하단이 메인라인
* transistorU : 게이트핀이 상단, 좌측과 우측이 메인라인
* transistorL : 게이트핀이 좌측, 상단과 하단이 메인라인
* transistorD : 게이트핀이 하단, 좌측과 우측이 메인라인
*
* <논리게이트>
* 논리게이트 4방향(출력=이름방향, VCC=출력의 90°CCW, 입력=나머지). dc 회전 R->U->L->D (CCW)
* andGate(2입력) : R{out R,vcc U,in L+D} U{out U,vcc L,in D+R} L{out L,vcc D,in R+U} D{out D,vcc R,in U+L}
* andGate는 단순히 소모전력이 2고, 각 핀은 최대 1을 받는데 받은 전력이 2 이상이면 작동함
*
* orGate & xorGate : andGate와 동일 핀배치, 다만 ON/OFF 판정 방식만 다름
* notGate(1입력) : R{out R,vcc U,in L} U{out U,vcc L,in D} L{out L,vcc D,in R} D{out D,vcc R,in U}
*
* srLatch 4방향: 출력=이름방향, VCC=출력의 90°CCW, S핀=출력반대, Reset핀=나머지(90°CW). dc 회전 R->U->L->D (CCW)
* srLatchR : 출력 R, VCC U, S L, Reset D
* srLatchU : 출력 U, VCC L, S D, Reset R
* srLatchL : 출력 L, VCC D, S R, Reset U
* srLatchD : 출력 D, VCC R, S U, Reset L
*
* leverRL : 좌우만 연결
* leverUD : 상하만 연결
*
* 트랜지스터와 논리게이트는 릴레이와 다르게 상태 변경 시에 모든 회로가 재계산되어야 함
* 트랜지스터와 논리게이트는 게이트 <-> 메인 라인간 전파가 안 되게 잘 수정할 것
* 게이트->메인라인으로 누설이 안되게 하려면 BFS 탐색 과정에서 skipBFSSet에 추가해주면 됨
* 메인라인->게이트로 누설이 안되게 하려면 isCableConnected 함수에서 체크해주면 됨
*
* <파워뱅크>
* 파워뱅크는 출력과 접지(충전부)가 한 타일에 동시에 있는 특이한 구조임.
* 출력=이름방향, 충전입력(접지)=반대편. dc 회전은 R->T->L->B (CCW)
* powerBankR : 오른쪽이 출력, 왼쪽이 충전입력(접지)
* powerBankT : 위쪽이 출력, 아래쪽이 충전입력(접지)
* powerBankL : 왼쪽이 출력, 오른쪽이 충전입력(접지)
* powerBankB : 아래쪽이 출력, 위쪽이 충전입력(접지)
*
* <그라운드>
* gndUsePower는 전방향(상하좌우, z축 제외) 입력임. gndUsePower>0이면 반드시 지향성 접지가 존재하지 않음에 유의
* 반대로 지향성 접지가 존재하면 당연히 gndUsePower는 0과 같음
*/


/*
* <BFS 작동 로직>
*
* 1. BFS 탐색으로 연결된 모든 회로망 탐색
*    - 시작점: 부하 또는 전압원 (CROSS 케이블 문제로 전선에서 시작 X)
*    - 이전 탐색 결과가 있으면 재사용 (saveFrontierQueue, saveVisitedSet)
*
* 2. 각 노드 방문 시 처리:
*    - 전압원(VOLTAGE_SOURCE): voltagePropVec에 추가, 가용 전력 누적
*      → 파워뱅크: min(electricMaxPower, powerStorage) 사용
*    - 일반 부하(gndUsePower > 0): 총 부하량에 누적
*    - 충전포트: 내부 배터리 잔량에 따라 동적으로 gndUsePower 계산
*    - 택트스위치: 다음 턴 자동 OFF 예약
*    - 압력스위치: 무게(5000 이상) 감지하여 ON/OFF
*
* 3. 6방향 이웃 탐색 시 특수 처리:
*    - CROSSED_CABLE: 진입 방향(수평/수직)에 따라 분리 처리
*    - 지향성 컴포넌트(트랜지스터, 릴레이, 논리게이트 등):
*      → 게이트/베이스 → 메인라인 방향 BFS 차단 (skipBFSSet)
*      → 지향성 부하량(gndUsePowerLeft/Right/Up/Down) 별도 누적
*    - 파워뱅크 충전: 충전량에 따라 로그 함수로 충전속도 감소
*
* 4. 탐색 완료 후 모든 노드에 nodeMaxCharge 설정 (= 총 가용 전력)
*
* 5. 전압원별 전하 밀어내기 시작:
*    - 각 전압원은 (자신의 출력 / 총 출력) 비율로 부하 분담
*    - 저항손실 보상을 위해 LOSS_COMPENSATION_FACTOR(1.2) 적용
*    - pushCharge() 호출하여 출력 방향으로 전하 밀어냄
*/

/////////////////////////////////////////////////////////////////////////////////////////////

/*
* <전하 밀어내기 로직>
*
* 1. 전압원에서 pushCharge() 호출하여 출력 방향으로 전하 밀어냄
*
* 2. 다음 노드 판정 (pushCharge 내부):
*    (2.1) 다음 노드가 접지(부하)를 가지고 있을 경우:
*        → 일반 부하: gndSink로 전송 (소비 후 남은 전력은 계속 전파)
*        → 지향성 부하(Gate/Base): gndSink{Dir}로 전송 후 즉시 종료 (신호 핀은 전력을 통과시키지 않음)
*
*    (2.2) 2.1에서 전송한 charge를 제외한 나머지로 아래 로직들을 실행
*    - 2갈래 이상 분기점:
*        → divideCharge()로 분배 (GND 방향 우선 처리)
*        → 각 방향으로 pushCharge() 재귀 호출
*    - 단일 경로:
*        → pushCharge() 재귀 호출
*    (※주의: pathVisited로 순환 방문 감지 시 해당 경로 종료)
*
* 3. 재귀 호출 완료 후 복귀하면서 transferCharge()로 새로 생긴 빈 공간만큼 전하 전송 수행
*    transferCharge() 내부:
*    - 저항손실 계산: I²R (I = Q / (V × T))
*    - 송신측 nodeCharge 차감, chargeFlux 기록
*    - 수신측 nodeCharge 증가, chargeFlux 기록
*/

bool Prop::hasGround()
{
    //일반 접지
    if (leadItem.gndUsePower > 0) return true;
    //지향성 접지
    if (leadItem.gndUsePowerRight > 0) return true;
    if (leadItem.gndUsePowerUp > 0) return true;
    if (leadItem.gndUsePowerLeft > 0) return true;
    if (leadItem.gndUsePowerDown > 0) return true;

    return false;
}

double Prop::getTotalChargeFlux()
{
    return (chargeFlux[dir16::right] + chargeFlux[dir16::up] + chargeFlux[dir16::left] + chargeFlux[dir16::down] + chargeFlux[dir16::above] + chargeFlux[dir16::below]);
}

bool Prop::isChargeFlowing()
{
    return chargeFlux[dir16::right] != 0
        || chargeFlux[dir16::up] != 0
        || chargeFlux[dir16::left] != 0
        || chargeFlux[dir16::down] != 0
        || chargeFlux[dir16::above] != 0
        || chargeFlux[dir16::below] != 0;
}

void Prop::initChargeFlux()
{
    chargeFlux[dir16::right] = 0;
    chargeFlux[dir16::up] = 0;
    chargeFlux[dir16::left] = 0;
    chargeFlux[dir16::down] = 0;
    chargeFlux[dir16::above] = 0;
    chargeFlux[dir16::below] = 0;
}

double Prop::getInletCharge()
{
    double totalInlet = 0;
    if (chargeFlux[dir16::right] > 0) totalInlet += chargeFlux[dir16::right];
    if (chargeFlux[dir16::up] > 0) totalInlet += chargeFlux[dir16::up];
    if (chargeFlux[dir16::left] > 0) totalInlet += chargeFlux[dir16::left];
    if (chargeFlux[dir16::down] > 0) totalInlet += chargeFlux[dir16::down];
    if (chargeFlux[dir16::above] > 0) totalInlet += chargeFlux[dir16::above];
    if (chargeFlux[dir16::below] > 0) totalInlet += chargeFlux[dir16::below];

    return totalInlet;
}

double Prop::getOutletCharge()
{
    double totalOutlet = 0;
    if (chargeFlux[dir16::right] < 0) totalOutlet -= chargeFlux[dir16::right];
    if (chargeFlux[dir16::up] < 0) totalOutlet -= chargeFlux[dir16::up];
    if (chargeFlux[dir16::left] < 0) totalOutlet -= chargeFlux[dir16::left];
    if (chargeFlux[dir16::down] < 0) totalOutlet -= chargeFlux[dir16::down];
    if (chargeFlux[dir16::above] < 0) totalOutlet -= chargeFlux[dir16::above];
    if (chargeFlux[dir16::below] < 0) totalOutlet -= chargeFlux[dir16::below];

    return totalOutlet;
}

void Prop::updateCircuitNetwork()
{
    if (debug::printCircuitLog) dbgPrt(L"------------------------- 회로망 업데이트 시작 ------------------------\n");
    int cursorX = getGridX();
    int cursorY = getGridY();
    int cursorZ = getGridZ();

    std::queue<Point3> frontierQueue;
    std::unordered_set<Point3> visitedSet;
    std::vector<Prop*> voltagePropVec;

    /*
    * skipBFSSet: 게이트/베이스 핀에서 메인라인으로의 BFS 확장 차단용
    *   - 트랜지스터/릴레이의 베이스 → 메인라인 누설 방지
    *   - 논리게이트의 입력핀 → 출력핀 누설 방지
    *   - 해당 노드는 BFS에 추가되지만, 그 노드에서의 확장은 1회만 허용
    */
    std::unordered_set<Point3> skipBFSSet;

    int circuitMaxEnergy = 0;
    int circuitTotalLoad = 0;

    //==============================================================================
    // 1. 회로 최초 탐색(BFS)
    //==============================================================================

    if (saveFrontierQueue.size() > 0 && saveVisitedSet.size() > 0)
    {
        if (debug::printCircuitLog) dbgPrt(L"------------------------- 이전 회로망 탐색 결과 불러오기 ------------------------\n");
        frontierQueue = saveFrontierQueue;
        visitedSet = saveVisitedSet;
    }
    else frontierQueue.push({ cursorX, cursorY, cursorZ });

    //BFS는 전선에서 시작하지 않음(CROSS 케이블때문에), 부하나 전압원에서 항상 시작할 것
    while (!frontierQueue.empty())
    {

        Point3 current = frontierQueue.front();
        frontierQueue.pop();
        crossStates.clear();

        Prop* currentProp = TileProp(current.x, current.y, current.z);

        if (visitedSet.find(current) != visitedSet.end()) continue;
        visitedSet.insert(current);

        if (debug::printCircuitLog)
        {
            std::wstring powerState = L"";
            if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON)) powerState = L" [ON]";
            else if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF)) powerState = L" [OFF]";

            dbgPrt(L"[BFS 탐색] %ls (%d,%d,%d)%ls\n",
                currentProp->leadItem.name.c_str(),
                current.x, current.y, current.z,
                powerState.c_str());
        }

        if (currentProp == nullptr)
        {
            dbgPrt(L"[경고] BFS가 nullptr 프롭에 도달함 (%d,%d,%d)\n", current.x, current.y, current.z);
            continue;
        }

        if (currentProp && (currentProp->leadItem.checkFlag(itemFlag::CIRCUIT) || currentProp->leadItem.checkFlag(itemFlag::CABLE)))
        {

            currentProp->runUsed = true;
            if (currentProp->leadItem.itemCode == itemID::powerBankR || currentProp->leadItem.itemCode == itemID::powerBankT || currentProp->leadItem.itemCode == itemID::powerBankL || currentProp->leadItem.itemCode == itemID::powerBankB) currentProp->runUsed = false;

            currentProp->totalLossCharge = 0;

            if (currentProp->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
            {
                if (debug::printCircuitLog)
                {
                    dbgPrt(L"  \x1b[33m★ 전원 감지: %ls (출력: %d)\x1b[0m\n",
                        currentProp->leadItem.name.c_str(),
                        currentProp->leadItem.electricMaxPower);
                }

                if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) &&
                    currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
                {
                    circuitMaxEnergy += currentProp->leadItem.electricMaxPower;
                    voltagePropVec.push_back(currentProp);
                }
                else if (currentProp->leadItem.itemCode == itemID::powerBankR || currentProp->leadItem.itemCode == itemID::powerBankT || currentProp->leadItem.itemCode == itemID::powerBankL || currentProp->leadItem.itemCode == itemID::powerBankB)
                {
                    circuitMaxEnergy += std::min(static_cast<double>(currentProp->leadItem.electricMaxPower), currentProp->leadItem.powerStorage);
                    voltagePropVec.push_back(currentProp);

                }

            }
            if (currentProp->leadItem.itemCode == itemID::chargingPort)//충전포트일 경우...
            {
                currentProp->leadItem.gndUsePower = 1;
                ItemStack* hereStack = TileItemStack(current.x, current.y, current.z);
                if (hereStack != nullptr)
                {
                    std::vector<ItemData>& hereItems = hereStack->getPocket()->itemInfo;
                    for (ItemData& item : hereItems)
                    {
                        if (item.itemCode == itemID::battery || item.itemCode == itemID::batteryPack)
                        {
                            if (item.powerStorage < item.powerStorageMax)
                            {
                                currentProp->leadItem.gndUsePower += item.powerStorageMax - std::floor(item.powerStorage);
                            }
                        }
                    }
                }
            }

            //현재 프롭이 소비전력이 있을 경우 loadSet에 추가
            //뒤의 isConnect 6방향 체크에 지향성 부하 loadSet 추가 메커니즘이 있음(주의할 것)
            if (currentProp->leadItem.gndUsePower > 0)
            {
                if (debug::printCircuitLog)
                {
                    dbgPrt(L"  \x1b[91m◆ 부하 감지: %ls (소비: %d)\x1b[0m\n",
                        currentProp->leadItem.name.c_str(),
                        currentProp->leadItem.gndUsePower);
                }
                circuitTotalLoad += currentProp->leadItem.gndUsePower;
            }

            //택트 스위치일 경우 다음 턴 시작 시에 종료
            if (currentProp->leadItem.itemCode == itemID::tactSwitchRL || currentProp->leadItem.itemCode == itemID::tactSwitchUD)
            {
                if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON))
                {
                    if (currentProp->leadItem.checkFlag(itemFlag::PROP_NEXT_TURN_POWER_OFF) == false)
                    {
                        currentProp->leadItem.addFlag(itemFlag::PROP_NEXT_TURN_POWER_OFF);
                    }
                    else
                    {
                        currentProp->leadItem.eraseFlag(itemFlag::PROP_NEXT_TURN_POWER_OFF);
                        currentProp->leadItem.eraseFlag(itemFlag::PROP_POWER_ON);
                        currentProp->leadItem.addFlag(itemFlag::PROP_POWER_OFF);
                    }
                }
            }
            else if (currentProp->leadItem.itemCode == itemID::pressureSwitchRL || currentProp->leadItem.itemCode == itemID::pressureSwitchUD)
            {
                int totalWeight = 0;

                //아이템 무게
                ItemStack* tgtItemStack = TileItemStack(current.x, current.y, current.z);
                if (tgtItemStack != nullptr)
                {
                    auto& tileItemInfo = tgtItemStack->getPocket()->itemInfo;
                    for (int i = 0; i < tileItemInfo.size(); i++)
                    {
                        totalWeight += tileItemInfo[i].weight * tileItemInfo[i].number;
                    }
                }

                //엔티티 무게
                Entity* ePtr = TileEntity(current.x, current.y, current.z);
                if (ePtr != nullptr) totalWeight += ePtr->entityInfo.weight;


                if (totalWeight >= 5000.0 && currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
                {
                    currentProp->leadItem.eraseFlag(itemFlag::PROP_POWER_OFF);
                    currentProp->leadItem.addFlag(itemFlag::PROP_POWER_ON);
                }
                else if (totalWeight < 5000.0 && currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON))
                {
                    currentProp->leadItem.eraseFlag(itemFlag::PROP_POWER_ON);
                    currentProp->leadItem.addFlag(itemFlag::PROP_POWER_OFF);
                }
            }


            const dir16 directions[] = { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below };

            if (skipBFSSet.find(current) == skipBFSSet.end())
            {
                for (int i = 0; i < 6; ++i)
                {
                    int dx, dy, dz;
                    dirToXYZ(directions[i], dx, dy, dz);
                    Point3 nextCoord = { current.x + dx, current.y + dy, current.z + dz };
                    Prop* nextProp = TileProp(nextCoord.x, nextCoord.y, nextCoord.z);

                    if (nextProp && nextProp->leadItem.checkFlag(itemFlag::CROSSED_CABLE))
                    {
                        if (crossStates[current] == crossFlag::horizontal && (directions[i] == dir16::up || directions[i] == dir16::down))
                        {
                            if (debug::printCircuitLog)
                                dbgPrt(L"  \x1b[90m[SKIP] %ls방향 크로스케이블 방향 불일치\x1b[0m\n", dirToArrow(directions[i]));
                            crossStates.erase(current);
                            continue;
                        }
                        if (crossStates[current] == crossFlag::vertical && (directions[i] == dir16::right || directions[i] == dir16::left))
                        {
                            if (debug::printCircuitLog)
                                dbgPrt(L"  \x1b[90m[SKIP] %ls방향 크로스케이블 방향 불일치\x1b[0m\n", dirToArrow(directions[i]));
                            crossStates.erase(current);
                            continue;
                        }
                    }

                    if (isCableConnected(current, directions[i]))
                    {
                        ItemData& nextItem = nextProp->leadItem;

                        if (debug::printCircuitLog)
                            dbgPrt(L"  [연결] %ls (%d,%d) %ls\n",
                                dirToArrow(directions[i]), nextCoord.x, nextCoord.y, nextItem.name.c_str());

                        bool isSignalInput = false;
                        if (nextProp != nullptr && nextProp->hasGround())
                        {
                            //파워뱅크 충전속도 제한
                            if (nextItem.itemCode == itemID::powerBankR || nextItem.itemCode == itemID::powerBankT || nextItem.itemCode == itemID::powerBankL || nextItem.itemCode == itemID::powerBankB)
                            {
                                double ratio = (nextItem.powerStorage) / static_cast<double>(nextItem.powerStorageMax);
                                errorBox(ratio > 1, L"파워뱅크의 에너지가 100%를 초과하였다.");
                                errorBox(ratio < 0, L"파워뱅크의 에너지 백분율이 음수로 떨어졌다.");

                                if (nextItem.itemCode == itemID::powerBankR)
                                {
                                    nextItem.gndUsePowerLeft = itemDex[nextItem.itemCode].gndUsePowerLeft;
                                    nextItem.gndUsePowerLeft *= std::log(1 + 20 * (1.02 - ratio)) / std::log(1 + 20 * 1.02);
                                    nextItem.gndUsePowerLeft = myMin(nextItem.gndUsePowerLeft, nextItem.powerStorageMax - nextItem.powerStorage);
                                    if (nextItem.gndUsePowerLeft < 0) nextItem.gndUsePowerLeft = 0;
                                }
                                else if (nextItem.itemCode == itemID::powerBankT)
                                {
                                    nextItem.gndUsePowerDown = itemDex[nextItem.itemCode].gndUsePowerDown;
                                    nextItem.gndUsePowerDown *= std::log(1 + 20 * (1.02 - ratio)) / std::log(1 + 20 * 1.02);
                                    nextItem.gndUsePowerDown = myMin(nextItem.gndUsePowerDown, nextItem.powerStorageMax - nextItem.powerStorage);
                                    if (nextItem.gndUsePowerDown < 0) nextItem.gndUsePowerDown = 0;
                                }
                                else if (nextItem.itemCode == itemID::powerBankL)
                                {
                                    nextItem.gndUsePowerRight = itemDex[nextItem.itemCode].gndUsePowerRight;
                                    nextItem.gndUsePowerRight *= std::log(1 + 20 * (1.02 - ratio)) / std::log(1 + 20 * 1.02);
                                    nextItem.gndUsePowerRight = myMin(nextItem.gndUsePowerRight, nextItem.powerStorageMax - nextItem.powerStorage);
                                    if (nextItem.gndUsePowerRight < 0) nextItem.gndUsePowerRight = 0;
                                }
                                else if (nextItem.itemCode == itemID::powerBankB)
                                {
                                    nextItem.gndUsePowerUp = itemDex[nextItem.itemCode].gndUsePowerUp;
                                    nextItem.gndUsePowerUp *= std::log(1 + 20 * (1.02 - ratio)) / std::log(1 + 20 * 1.02);
                                    nextItem.gndUsePowerUp = myMin(nextItem.gndUsePowerUp, nextItem.powerStorageMax - nextItem.powerStorage);
                                    if (nextItem.gndUsePowerUp < 0) nextItem.gndUsePowerUp = 0;
                                }
                            }

                            if (directions[i] == dir16::right && nextProp->leadItem.gndUsePowerLeft > 0)
                            {
                                if (debug::printCircuitLog)
                                    dbgPrt(L"    └─ \x1b[35m◆ 지향성부하: 소비=%d\x1b[0m\n", nextProp->leadItem.gndUsePowerLeft);
                                circuitTotalLoad += nextProp->leadItem.gndUsePowerLeft;
                            }
                            else if (directions[i] == dir16::up && nextProp->leadItem.gndUsePowerDown > 0)
                            {
                                if (debug::printCircuitLog)
                                    dbgPrt(L"    └─ \x1b[35m◆ 지향성부하: 소비=%d\x1b[0m\n", nextProp->leadItem.gndUsePowerDown);
                                circuitTotalLoad += nextProp->leadItem.gndUsePowerDown;
                            }
                            else if (directions[i] == dir16::left && nextProp->leadItem.gndUsePowerRight > 0)
                            {
                                if (debug::printCircuitLog)
                                    dbgPrt(L"    └─ \x1b[35m◆ 지향성부하: 소비=%d\x1b[0m\n", nextProp->leadItem.gndUsePowerRight);
                                circuitTotalLoad += nextProp->leadItem.gndUsePowerRight;
                            }
                            else if (directions[i] == dir16::down && nextProp->leadItem.gndUsePowerUp > 0)
                            {
                                if (debug::printCircuitLog)
                                    dbgPrt(L"    └─ \x1b[35m◆ 지향성부하: 소비=%d\x1b[0m\n", nextProp->leadItem.gndUsePowerUp);
                                circuitTotalLoad += nextProp->leadItem.gndUsePowerUp;
                            }

                            //베이스에서 메인라인으로 BFS를 추가하는 것을 막음
                            if (nextProp->leadItem.itemCode == itemID::transistorL && directions[i] == dir16::right) isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::transistorU && directions[i] == dir16::down) isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::transistorR && directions[i] == dir16::left) isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::transistorD && directions[i] == dir16::up) isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::relayL && directions[i] == dir16::right) isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::relayU && directions[i] == dir16::down) isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::relayR && directions[i] == dir16::left) isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::relayD && directions[i] == dir16::up) isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::andGateR && (directions[i] == dir16::right || directions[i] == dir16::up))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::andGateU && (directions[i] == dir16::up || directions[i] == dir16::left))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::andGateL && (directions[i] == dir16::left || directions[i] == dir16::down))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::andGateD && (directions[i] == dir16::down || directions[i] == dir16::right))
                                isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::orGateR && (directions[i] == dir16::right || directions[i] == dir16::up))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::orGateU && (directions[i] == dir16::up || directions[i] == dir16::left))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::orGateL && (directions[i] == dir16::left || directions[i] == dir16::down))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::orGateD && (directions[i] == dir16::down || directions[i] == dir16::right))
                                isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::xorGateR && (directions[i] == dir16::right || directions[i] == dir16::up))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::xorGateU && (directions[i] == dir16::up || directions[i] == dir16::left))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::xorGateL && (directions[i] == dir16::left || directions[i] == dir16::down))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::xorGateD && (directions[i] == dir16::down || directions[i] == dir16::right))
                                isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::notGateR && (directions[i] == dir16::right))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::notGateU && (directions[i] == dir16::up))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::notGateL && (directions[i] == dir16::left))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::notGateD && (directions[i] == dir16::down))
                                isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::srLatchR && (directions[i] == dir16::right || directions[i] == dir16::up))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::srLatchU && (directions[i] == dir16::up || directions[i] == dir16::left))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::srLatchL && (directions[i] == dir16::left || directions[i] == dir16::down))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::srLatchD && (directions[i] == dir16::down || directions[i] == dir16::right))
                                isSignalInput = true;

                            if (nextProp->leadItem.itemCode == itemID::powerBankR && (directions[i] == dir16::right))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::powerBankT && (directions[i] == dir16::up))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::powerBankL && (directions[i] == dir16::left))
                                isSignalInput = true;
                            else if (nextProp->leadItem.itemCode == itemID::powerBankB && (directions[i] == dir16::down))
                                isSignalInput = true;
                        }

                        if (isSignalInput)
                        {
                            if (debug::printCircuitLog)
                                dbgPrt(L"    └─ \x1b[36m[신호입력핀] BFS 차단, 전하만 공급\x1b[0m\n");

                            nextProp->nodeMaxCharge = circuitMaxEnergy;
                            nextProp->nodeCharge = circuitMaxEnergy;
                            continue;
                        }

                        if (nextProp->leadItem.checkFlag(itemFlag::CROSSED_CABLE))
                        {
                            if (directions[i] == dir16::down || directions[i] == dir16::up) crossStates[nextCoord] = crossFlag::vertical;
                            else if (directions[i] == dir16::right || directions[i] == dir16::left) crossStates[nextCoord] = crossFlag::horizontal;
                        }
                        frontierQueue.push(nextCoord);
                    }
                }
            }
            else
            {
                if (debug::printCircuitLog)
                    dbgPrt(L"  \x1b[90m[SKIP-BFS] %ls - 신호핀에서 메인라인 확장 차단\x1b[0m\n", currentProp->leadItem.name.c_str());
                skipBFSSet.erase(current);
            }

        }
    }

    //==============================================================================
    // 2. 최대 전력 설정
    //==============================================================================
    for (auto coord : visitedSet)
    {
        Prop* propPtr = TileProp(coord.x, coord.y, coord.z);
        if (propPtr != nullptr)
        {
            propPtr->nodeMaxCharge = circuitMaxEnergy;

            //(전자회로용) 항상 전자 가득 찬 상태
            propPtr->nodeCharge = circuitMaxEnergy;

        }
    }

    //==============================================================================
    // 3. 전압원 전송 시작
    //==============================================================================

    randomVectorShuffle(voltagePropVec);

    int totalAvailablePower = 0;
    for (Prop* voltProp : voltagePropVec)
    {
        if (voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) &&
            voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
        {
            totalAvailablePower += voltProp->leadItem.electricMaxPower;
        }
    }


    for (Prop* voltProp : voltagePropVec)
    {
        constexpr double LOSS_COMPENSATION_FACTOR = 1.2;
        // ↑ 저항손실(I²R)로 인해 도중에 전하가 소모되므로, 
        //   부하에 정확히 필요한 양이 도달하도록 20% 여유분 추가

        voltProp->nodeCharge = voltProp->nodeMaxCharge;
        int x = voltProp->getGridX();
        int y = voltProp->getGridY();
        int z = voltProp->getGridZ();
        double voltRatio = (double)voltProp->leadItem.electricMaxPower / (double)totalAvailablePower;
        double voltOutputPower = myMin(std::ceil(circuitTotalLoad * voltRatio), voltProp->leadItem.electricMaxPower);
        voltOutputPower *= LOSS_COMPENSATION_FACTOR;  // 저항손실 보존 변수 (기본값 120%)

        if (debug::printCircuitLog) dbgPrt(L"========================▼전압원 (%d,%d)%ls : 밀어내기 시작▼========================\n", x, y, voltProp->leadItem.name.c_str());
        if (voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) || voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
        {
            double finalVoltOutput = voltOutputPower;
            if (voltProp->leadItem.itemCode == itemID::powerBankR || voltProp->leadItem.itemCode == itemID::powerBankT || voltProp->leadItem.itemCode == itemID::powerBankL || voltProp->leadItem.itemCode == itemID::powerBankB)
                finalVoltOutput = std::min(voltOutputPower, voltProp->leadItem.powerStorage);


            if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT) && isCableConnected({ x,y,z }, dir16::right))
                pushCharge(voltProp, dir16::right, finalVoltOutput, {}, 0);
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_UP) && isCableConnected({ x,y,z }, dir16::up))
                pushCharge(voltProp, dir16::up, finalVoltOutput, {}, 0);
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT) && isCableConnected({ x,y,z }, dir16::left))
                pushCharge(voltProp, dir16::left, finalVoltOutput, {}, 0);
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_DOWN) && isCableConnected({ x,y,z }, dir16::down))
                pushCharge(voltProp, dir16::down, finalVoltOutput, {}, 0);

            voltProp->nodeCharge = voltProp->nodeMaxCharge;
        }
    }

    if (debug::printCircuitLog)
    {
        dbgPrt(L"======================== 회로망 요약 ========================\n");
        dbgPrt(L"노드: %zu개, 전압원: %zu개, 총부하: %d, 총전력: %d\n",
            visitedSet.size(), voltagePropVec.size(), circuitTotalLoad, circuitMaxEnergy);
    }
}

bool Prop::isCableConnected(Point3 currentCoord, dir16 dir)
{
    Prop* currentProp = TileProp(currentCoord.x, currentCoord.y, currentCoord.z);

    Point3 delCoord = { 0,0,0 };
    itemFlag hostFlag, guestFlag;
    switch (dir)
    {
    case dir16::right:
        delCoord = { +1,0,0 };
        hostFlag = itemFlag::CABLE_CNCT_RIGHT;
        guestFlag = itemFlag::CABLE_CNCT_LEFT;
        break;
    case dir16::up:
        delCoord = { 0,-1,0 };
        hostFlag = itemFlag::CABLE_CNCT_UP;
        guestFlag = itemFlag::CABLE_CNCT_DOWN;
        break;
    case dir16::left:
        delCoord = { -1,0,0 };
        hostFlag = itemFlag::CABLE_CNCT_LEFT;
        guestFlag = itemFlag::CABLE_CNCT_RIGHT;
        break;
    case dir16::down:
        delCoord = { 0,+1,0 };
        hostFlag = itemFlag::CABLE_CNCT_DOWN;
        guestFlag = itemFlag::CABLE_CNCT_UP;
        break;
    case dir16::above:
        delCoord = { 0,0,+1 };
        hostFlag = itemFlag::CABLE_Z_ASCEND;
        guestFlag = itemFlag::CABLE_Z_DESCEND;
        break;
    case dir16::below:
        delCoord = { 0,0,-1 };
        hostFlag = itemFlag::CABLE_Z_DESCEND;
        guestFlag = itemFlag::CABLE_Z_ASCEND;
        break;
    default:
        errorBox(L"isCableConnected가 잘못된 방향 인자를 받았다.\n");
        break;
    }
    Prop* targetProp = TileProp(currentCoord.x + delCoord.x, currentCoord.y + delCoord.y, currentCoord.z + delCoord.z);

    if (targetProp == nullptr) return false;

    if (currentProp->leadItem.itemCode == itemID::tactSwitchRL
        || currentProp->leadItem.itemCode == itemID::tactSwitchUD
        || currentProp->leadItem.itemCode == itemID::leverRL
        || currentProp->leadItem.itemCode == itemID::leverUD
        || currentProp->leadItem.itemCode == itemID::pressureSwitchRL
        || currentProp->leadItem.itemCode == itemID::pressureSwitchUD
        )
    {
        if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }

    ItemData& tgtItem = targetProp->leadItem;

    if ((dir == dir16::right || dir == dir16::left) && tgtItem.itemCode == itemID::leverRL)
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) && tgtItem.itemCode == itemID::leverUD)
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::right || dir == dir16::left) && tgtItem.itemCode == itemID::tactSwitchRL)
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) && tgtItem.itemCode == itemID::tactSwitchUD)
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::right || dir == dir16::left) && tgtItem.itemCode == itemID::pressureSwitchRL)
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) && tgtItem.itemCode == itemID::pressureSwitchUD)
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }


    if ((dir == dir16::right || dir == dir16::left) && (tgtItem.itemCode == itemID::relayU || tgtItem.itemCode == itemID::relayD))
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) && (tgtItem.itemCode == itemID::relayR || tgtItem.itemCode == itemID::relayL))
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }

    if ((dir == dir16::right || dir == dir16::left) && (tgtItem.itemCode == itemID::transistorU || tgtItem.itemCode == itemID::transistorD))
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) && (tgtItem.itemCode == itemID::transistorR || tgtItem.itemCode == itemID::transistorL))
    {
        if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }

    //논리게이트 VCC핀(출력의 90°CCW): OFF일 때 차단 / 출력핀(이름방향) 다이오드 바이패스
    if (dir == dir16::down && tgtItem.itemCode == itemID::andGateR) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::right && tgtItem.itemCode == itemID::andGateU) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::up && tgtItem.itemCode == itemID::andGateL) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::andGateD) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::andGateR) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::andGateU) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::andGateL) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::andGateD) return false;

    if (dir == dir16::down && tgtItem.itemCode == itemID::orGateR) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::right && tgtItem.itemCode == itemID::orGateU) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::up && tgtItem.itemCode == itemID::orGateL) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::orGateD) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::orGateR) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::orGateU) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::orGateL) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::orGateD) return false;

    if (dir == dir16::down && tgtItem.itemCode == itemID::xorGateR) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::right && tgtItem.itemCode == itemID::xorGateU) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::up && tgtItem.itemCode == itemID::xorGateL) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::xorGateD) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::xorGateR) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::xorGateU) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::xorGateL) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::xorGateD) return false;


    if (dir == dir16::down && tgtItem.itemCode == itemID::notGateR) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::right && tgtItem.itemCode == itemID::notGateU) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::up && tgtItem.itemCode == itemID::notGateL) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::notGateD) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::notGateR) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::notGateU) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::notGateL) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::notGateD) return false;


    //SR래치 VCC핀(출력의 90°CCW): OFF일 때 차단
    if (dir == dir16::down && tgtItem.itemCode == itemID::srLatchR) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::right && tgtItem.itemCode == itemID::srLatchU) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::up && tgtItem.itemCode == itemID::srLatchL) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    else if (dir == dir16::left && tgtItem.itemCode == itemID::srLatchD) { if (tgtItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false; }
    //SR래치 출력핀(이름방향) 다이오드 바이패스
    else if (dir == dir16::left && tgtItem.itemCode == itemID::srLatchR) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::srLatchU) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::srLatchL) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::srLatchD) return false;

    if (dir == dir16::left && tgtItem.itemCode == itemID::delayR) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::delayL) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::delayU) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::delayD) return false;

    if (dir == dir16::right && tgtItem.itemCode == itemID::diodeL) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::diodeD)return false;
    else if (dir == dir16::left && tgtItem.itemCode == itemID::diodeR)return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::diodeU)return false;

    if (dir == dir16::left && tgtItem.itemCode == itemID::powerBankR) return false;
    else if (dir == dir16::right && tgtItem.itemCode == itemID::powerBankL) return false;
    else if (dir == dir16::down && tgtItem.itemCode == itemID::powerBankT) return false;
    else if (dir == dir16::up && tgtItem.itemCode == itemID::powerBankB) return false;


    ItemData& crtItem = currentProp->leadItem;

    //(트랜지스터) 메인라인에서 베이스 방향 절연
    if (crtItem.itemCode == itemID::transistorL && dir == dir16::left) return false;
    else if (crtItem.itemCode == itemID::transistorU && dir == dir16::up) return false;
    else if (crtItem.itemCode == itemID::transistorR && dir == dir16::right) return false;
    else if (crtItem.itemCode == itemID::transistorD && dir == dir16::down) return false;

    if (crtItem.itemCode == itemID::relayL && dir == dir16::left) return false;
    else if (crtItem.itemCode == itemID::relayU && dir == dir16::up) return false;
    else if (crtItem.itemCode == itemID::relayR && dir == dir16::right) return false;
    else if (crtItem.itemCode == itemID::relayD && dir == dir16::down) return false;

    //(논리게이트) 메인라인에서 입력핀1,2 방향 절연
    if (crtItem.itemCode == itemID::andGateR && (dir == dir16::left || dir == dir16::down)) return false;
    else if (crtItem.itemCode == itemID::andGateU && (dir == dir16::down || dir == dir16::right)) return false;
    else if (crtItem.itemCode == itemID::andGateL && (dir == dir16::right || dir == dir16::up)) return false;
    else if (crtItem.itemCode == itemID::andGateD && (dir == dir16::up || dir == dir16::left)) return false;

    if (crtItem.itemCode == itemID::orGateR && (dir == dir16::left || dir == dir16::down)) return false;
    else if (crtItem.itemCode == itemID::orGateU && (dir == dir16::down || dir == dir16::right)) return false;
    else if (crtItem.itemCode == itemID::orGateL && (dir == dir16::right || dir == dir16::up)) return false;
    else if (crtItem.itemCode == itemID::orGateD && (dir == dir16::up || dir == dir16::left)) return false;

    if (crtItem.itemCode == itemID::xorGateR && (dir == dir16::left || dir == dir16::down)) return false;
    else if (crtItem.itemCode == itemID::xorGateU && (dir == dir16::down || dir == dir16::right)) return false;
    else if (crtItem.itemCode == itemID::xorGateL && (dir == dir16::right || dir == dir16::up)) return false;
    else if (crtItem.itemCode == itemID::xorGateD && (dir == dir16::up || dir == dir16::left)) return false;

    //(NOT게이트) 메인라인에서 입력핀 방향 절연
    if (crtItem.itemCode == itemID::notGateR && dir == dir16::left) return false;
    else if (crtItem.itemCode == itemID::notGateU && dir == dir16::down) return false;
    else if (crtItem.itemCode == itemID::notGateL && dir == dir16::right) return false;
    else if (crtItem.itemCode == itemID::notGateD && dir == dir16::up) return false;

    if (crtItem.itemCode == itemID::srLatchR && (dir == dir16::left || dir == dir16::down)) return false;
    else if (crtItem.itemCode == itemID::srLatchU && (dir == dir16::down || dir == dir16::right)) return false;
    else if (crtItem.itemCode == itemID::srLatchL && (dir == dir16::right || dir == dir16::up)) return false;
    else if (crtItem.itemCode == itemID::srLatchD && (dir == dir16::up || dir == dir16::left)) return false;

    if (crtItem.itemCode == itemID::delayR && dir == dir16::right && crtItem.checkFlag(itemFlag::PROP_POWER_OFF))
    {
        return false;
    }
    if (crtItem.itemCode == itemID::delayL && dir == dir16::left && crtItem.checkFlag(itemFlag::PROP_POWER_OFF))
    {
        return false;
    }
    if (crtItem.itemCode == itemID::delayU && dir == dir16::up && crtItem.checkFlag(itemFlag::PROP_POWER_OFF))
    {
        return false;
    }
    if (crtItem.itemCode == itemID::delayD && dir == dir16::down && crtItem.checkFlag(itemFlag::PROP_POWER_OFF))
    {
        return false;
    }

    //(파워뱅크) 메인라인(현재)에서 입력부 차단
    if (crtItem.itemCode == itemID::powerBankR && dir == dir16::left) return false;
    else if (crtItem.itemCode == itemID::powerBankL && dir == dir16::right) return false;
    else if (crtItem.itemCode == itemID::powerBankT && dir == dir16::down) return false;
    else if (crtItem.itemCode == itemID::powerBankB && dir == dir16::up) return false;

    if (dir == dir16::above || dir == dir16::below)
    {
        bool currentCondition = currentProp->leadItem.checkFlag(itemFlag::CABLE) && currentProp->leadItem.checkFlag(hostFlag);
        bool targetCondition = targetProp->leadItem.checkFlag(itemFlag::CABLE) && targetProp->leadItem.checkFlag(guestFlag);

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else if (dir == dir16::right || dir == dir16::up || dir == dir16::left || dir == dir16::down)
    {
        bool currentCondition = (currentProp->leadItem.checkFlag(itemFlag::CABLE) || currentProp->leadItem.checkFlag(hostFlag));
        if (crtItem.checkFlag(itemFlag::CROSSED_CABLE))
        {
            if (crossStates.find(currentCoord) != crossStates.end())
            {
                if (crossStates[currentCoord] == crossFlag::horizontal && (dir == dir16::up || dir == dir16::down)) currentCondition = false;
                else if (crossStates[currentCoord] == crossFlag::vertical && (dir == dir16::right || dir == dir16::left)) currentCondition = false;
            }
        }

        bool targetCondition = (targetProp->leadItem.checkFlag(itemFlag::CABLE) || targetProp->leadItem.checkFlag(guestFlag));

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else
    {
        errorBox(L"isCableConnected가 잘못된 방향 인자를 받았다.\n");
        return false;
    }
}

bool Prop::isCableConnected(Prop* currentProp, dir16 dir)
{
    return isCableConnected({ currentProp->getGridX(),currentProp->getGridY(),currentProp->getGridZ() }, dir);
}

bool Prop::isCableLinked(Point3 currentCoord, dir16 dir)
{
    Prop* currentProp = TileProp(currentCoord.x, currentCoord.y, currentCoord.z);
    errorBox(currentProp == nullptr, L"isCableLinked에 인자로 입력된 위치의 프롭이 널포인터이다.");

    Point3 delCoord = { 0,0,0 };
    itemFlag hostFlag, guestFlag;
    switch (dir)
    {
    case dir16::right:
        delCoord = { +1,0,0 };
        hostFlag = itemFlag::CABLE_CNCT_RIGHT;
        guestFlag = itemFlag::CABLE_CNCT_LEFT;
        break;
    case dir16::up:
        delCoord = { 0,-1,0 };
        hostFlag = itemFlag::CABLE_CNCT_UP;
        guestFlag = itemFlag::CABLE_CNCT_DOWN;
        break;
    case dir16::left:
        delCoord = { -1,0,0 };
        hostFlag = itemFlag::CABLE_CNCT_LEFT;
        guestFlag = itemFlag::CABLE_CNCT_RIGHT;
        break;
    case dir16::down:
        delCoord = { 0,+1,0 };
        hostFlag = itemFlag::CABLE_CNCT_DOWN;
        guestFlag = itemFlag::CABLE_CNCT_UP;
        break;
    case dir16::above:
        delCoord = { 0,0,+1 };
        hostFlag = itemFlag::CABLE_Z_ASCEND;
        guestFlag = itemFlag::CABLE_Z_DESCEND;
        break;
    case dir16::below:
        delCoord = { 0,0,-1 };
        hostFlag = itemFlag::CABLE_Z_DESCEND;
        guestFlag = itemFlag::CABLE_Z_ASCEND;
        break;
    default:
        errorBox(L"isCableLinked 함수가 잘못된 방향 인자를 입력받았다.");
        break;
    }
    Prop* targetProp = TileProp(currentCoord.x + delCoord.x, currentCoord.y + delCoord.y, currentCoord.z + delCoord.z);
    if (targetProp == nullptr) return false;

    if (dir == dir16::above || dir == dir16::below)
    {
        bool currentCondition = currentProp->leadItem.checkFlag(itemFlag::CABLE) && currentProp->leadItem.checkFlag(hostFlag);
        bool targetCondition = targetProp->leadItem.checkFlag(itemFlag::CABLE) && targetProp->leadItem.checkFlag(guestFlag);

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else if (dir == dir16::right || dir == dir16::up || dir == dir16::left || dir == dir16::down)
    {
        bool currentCondition = (currentProp->leadItem.checkFlag(itemFlag::CABLE) || currentProp->leadItem.checkFlag(hostFlag));
        bool targetCondition = (targetProp->leadItem.checkFlag(itemFlag::CABLE) || targetProp->leadItem.checkFlag(guestFlag));

        if (currentCondition && targetCondition) return true;
        else return false;
    }
    else errorBox(L"isCableLinked 함수가 잘못된 방향 인자를 입력받았다.");
}

bool Prop::isCableLinked(Prop* currentProp, dir16 dir)
{
    return isCableLinked({ currentProp->getGridX(), currentProp->getGridY(), currentProp->getGridZ() }, dir);
}

bool Prop::isGround(Point3 current, dir16 dir)
{
    errorBox(dir == dir16::above || dir == dir16::below, L"isGround에 z축 방향이 입력되었다.n\n");

    int dx, dy, dz;
    dirToXYZ(dir, dx, dy, dz);
    Prop* nextProp = TileProp(current.x + dx, current.y + dy, current.z + dz);

    if (nextProp == nullptr || nextProp->hasGround() == false)
        return false;

    if (isCableConnected(current, dir))
    {
        if (nextProp->leadItem.gndUsePower > 0) return true;
        if (dir == dir16::right && nextProp->leadItem.gndUsePowerLeft > 0) return true;
        if (dir == dir16::up && nextProp->leadItem.gndUsePowerDown > 0) return true;
        if (dir == dir16::left && nextProp->leadItem.gndUsePowerRight > 0) return true;
        if (dir == dir16::down && nextProp->leadItem.gndUsePowerUp > 0) return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Prop::pushCharge(Prop* donorProp, dir16 txDir, double txChargeAmount, std::unordered_set<Prop*> pathVisited, int depth)
{
    errorBox(donorProp == nullptr, L"pushCharge 함수에서 donorProp이 널포인터이다.\n");
    int dx, dy, dz;
    dirToXYZ(txDir, dx, dy, dz);
    Point3 nextCoord = { donorProp->getGridX() + dx, donorProp->getGridY() + dy, donorProp->getGridZ() + dz };
    Prop* nextProp = TileProp(nextCoord);
    errorBox(nextProp == nullptr, L"pushCharge 함수에서 recieverProp이 널포인터이다.\n");

    txChargeAmount = std::min(donorProp->nodeCharge, txChargeAmount);

    errorBox(txChargeAmount > donorProp->nodeCharge + EPSILON, L"pushCharge에서 보낼 전자가 실제 존재하는 전자의 숫자보다 많다.\n");
    errorBox(!isCableConnected({ donorProp->getGridX(), donorProp->getGridY(), donorProp->getGridZ() }, txDir),L"pushCharge에서 보낼 방향과 donorProp이 전기적으로 연결되지 않았다.\n");

    std::wstring indent(depth * 2, L' ');

    if (pathVisited.find(donorProp) != pathVisited.end())
    {
        if (debug::printCircuitLog)
            dbgPrt(L"%s[PUSH-SKIP] (%d,%d)%ls 이미 방문됨\n", indent.c_str(), donorProp->getGridX(), donorProp->getGridY(), donorProp->leadItem.name.c_str());
        return 0;
    }
    pathVisited.insert(donorProp);
    if (pathVisited.find(nextProp) != pathVisited.end()) return 0;



    if (debug::printCircuitLog) dbgPrt(L"%s[PUSH] (%d,%d)%ls → (%d,%d)%ls [%ls] 시도: %.2f\n",
        indent.c_str(),
        donorProp->getGridX(), donorProp->getGridY(), donorProp->leadItem.name.c_str(),
        nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
        dirToArrow(txDir),
        txChargeAmount);

    double gndTxEnergy = 0;
    Point3 current = { donorProp->getGridX(), donorProp->getGridY(), donorProp->getGridZ() };
    if (isGround(current, txDir)) //해당 방향이 GND일 경우 전하 소비 후에 즉시 종료 return
    {
        double remainEnergy;
        bool isDirectionalGnd = false;
        int requiredPower = 0;

        if (txDir == dir16::right && nextProp->leadItem.gndUsePowerLeft > 0)
        {
            remainEnergy = nextProp->leadItem.gndUsePowerLeft - nextProp->gndSinkLeft;
            requiredPower = nextProp->leadItem.gndUsePowerLeft;
            isDirectionalGnd = true;
        }
        else if (txDir == dir16::up && nextProp->leadItem.gndUsePowerDown > 0)
        {
            remainEnergy = nextProp->leadItem.gndUsePowerDown - nextProp->gndSinkDown;
            requiredPower = nextProp->leadItem.gndUsePowerDown;
            isDirectionalGnd = true;
        }
        else if (txDir == dir16::left && nextProp->leadItem.gndUsePowerRight > 0)
        {
            remainEnergy = nextProp->leadItem.gndUsePowerRight - nextProp->gndSinkRight;
            requiredPower = nextProp->leadItem.gndUsePowerRight;
            isDirectionalGnd = true;
        }
        else if (txDir == dir16::down && nextProp->leadItem.gndUsePowerUp > 0)
        {
            remainEnergy = nextProp->leadItem.gndUsePowerUp - nextProp->gndSinkUp;
            requiredPower = nextProp->leadItem.gndUsePowerUp;
            isDirectionalGnd = true;
        }
        else
        {
            remainEnergy = nextProp->leadItem.gndUsePower - nextProp->gndSink;
            requiredPower = nextProp->leadItem.gndUsePower;
        }

        if (debug::printCircuitLog)
        {
            dbgPrt(L"%s  └─ \x1b[33m[GND진입] %ls GND, 요구=%d, 잔여용량=%.2f, 시도량=%.2f\x1b[0m\n",
                indent.c_str(),
                isDirectionalGnd ? L"지향성" : L"일반",
                requiredPower,
                remainEnergy,
                txChargeAmount);
        }

        if (remainEnergy > EPSILON)
        {
            gndTxEnergy = std::min(std::min(txChargeAmount, remainEnergy), nextProp->nodeCharge);
            nextProp->nodeCharge -= gndTxEnergy;

            if (debug::printCircuitLog)
            {
                dbgPrt(L"%s      → 실제소비=%.2f, 남은용량=%.2f\n",
                    indent.c_str(),
                    gndTxEnergy,
                    remainEnergy - gndTxEnergy);
            }

            if (txDir == dir16::right && nextProp->leadItem.gndUsePowerLeft > 0) nextProp->gndSinkLeft += gndTxEnergy;
            else if (txDir == dir16::up && nextProp->leadItem.gndUsePowerDown > 0) nextProp->gndSinkDown += gndTxEnergy;
            else if (txDir == dir16::left && nextProp->leadItem.gndUsePowerRight > 0) nextProp->gndSinkRight += gndTxEnergy;
            else if (txDir == dir16::down && nextProp->leadItem.gndUsePowerUp > 0) nextProp->gndSinkUp += gndTxEnergy;
            else nextProp->gndSink += gndTxEnergy;
        }
        else if (debug::printCircuitLog)
        {
            dbgPrt(L"%s      → \x1b[90m용량 소진됨, 스킵\x1b[0m\n", indent.c_str());
        }

        if (isDirectionalGnd)
        {
            transferCharge(donorProp, nextProp, gndTxEnergy, indent, txDir, true);
            return gndTxEnergy;
        }
    }

    double pushedCharge = std::min(txChargeAmount - gndTxEnergy, nextProp->nodeCharge);

    if (pushedCharge > EPSILON)
    {
        std::vector<dir16> possibleDirs;

        if (nextProp->leadItem.checkFlag(itemFlag::CROSSED_CABLE))
        {
            if (txDir == dir16::right || txDir == dir16::left) crossStates[nextCoord] = crossFlag::horizontal;
            else if (txDir == dir16::up || txDir == dir16::down) crossStates[nextCoord] = crossFlag::vertical;
        }

        for (auto dir : { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below })
        {
            if (dir == reverse(txDir)) continue;
            if (isCableConnected({ nextProp->getGridX(), nextProp->getGridY(), nextProp->getGridZ() }, dir))
            {
                possibleDirs.push_back(dir);
            }
        }

        if (possibleDirs.empty() == false)
        {
            if (possibleDirs.size() == 1)
            {
                auto newPathVisited = pathVisited;
                pushCharge(nextProp, possibleDirs[0], pushedCharge, newPathVisited, depth + 1);
            }
            else if (possibleDirs.size() > 1)
            {
                divideCharge(nextProp, pushedCharge, possibleDirs, pathVisited, depth + 1);
            }
        }
    }

    // 재귀 복귀: 하위 노드들이 전하를 소비해서 생긴 빈 공간만큼 전송
    double finalTxCharge = std::min(txChargeAmount, nextProp->nodeMaxCharge - nextProp->nodeCharge);
    transferCharge(donorProp, nextProp, finalTxCharge, indent, txDir, false);
    return finalTxCharge;
}


void Prop::divideCharge(Prop* propPtr, double inputCharge, std::vector<dir16> possibleDirs, std::unordered_set<Prop*> pathVisited, int depth)
{
    std::wstring indent(depth * 2, L' ');  // 인덴트 생성

    if (debug::printCircuitLog)
    {
        dbgPrt(L"%s[DIVIDE] (%d,%d)%ls 분배시작: %.2f → %zu방향\n",
            indent.c_str(),
            propPtr->getGridX(), propPtr->getGridY(), propPtr->leadItem.name.c_str(),
            inputCharge, possibleDirs.size());
    }

    double remainingCharge = inputCharge;
    std::vector<dir16> dirsToRemove;
    std::vector<dir16> gndDirs;
    std::vector<dir16> nonGndDirs;
    dirsToRemove.reserve(6);
    gndDirs.reserve(6);
    nonGndDirs.reserve(6);

    int loopCount = 0;
    while (remainingCharge > EPSILON && !possibleDirs.empty())
    {
        loopCount++;
        dirsToRemove.clear();
        gndDirs.clear();
        nonGndDirs.clear();
        double gndPushedCharge = 0;
        double loopPushedCharge = 0;

        //접지 우선 배분
        for (auto dir : possibleDirs)
        {
            if (isGround({ propPtr->getGridX(), propPtr->getGridY(), propPtr->getGridZ() }, dir))
            {
                gndDirs.push_back(dir);
            }
            else nonGndDirs.push_back(dir);
        }

        if (gndDirs.size() > 0)
        {
            double gndSplitCharge = remainingCharge / gndDirs.size();

            if (debug::printCircuitLog)
            {
                dbgPrt(L"%s  [DIV-GND] 접지 %zu방향, 각 %.2f씩\n",
                    indent.c_str(), gndDirs.size(), gndSplitCharge);
            }

            for (auto dir : gndDirs)
            {
                auto newPathVisited = pathVisited;
                double branchPushedCharge = pushCharge(propPtr, dir, gndSplitCharge, newPathVisited, depth + 1);
                gndPushedCharge += branchPushedCharge;
                if (branchPushedCharge < EPSILON) dirsToRemove.push_back(dir);
            }

            possibleDirs.erase
            (
                std::remove_if
                (
                    possibleDirs.begin(),
                    possibleDirs.end(),
                    [&dirsToRemove](dir16 d) { return std::find(dirsToRemove.begin(), dirsToRemove.end(), d) != dirsToRemove.end(); }
                ),
                possibleDirs.end()
            );

            remainingCharge -= gndPushedCharge;
        }

        dirsToRemove.clear();
        if (possibleDirs.empty()) break;

        if (nonGndDirs.size() > 0)
        {
            double splitCharge = remainingCharge / nonGndDirs.size();

            if (debug::printCircuitLog)
            {
                dbgPrt(L"%s  [DIV-LOOP] 일반 %zu방향, 각 %.2f씩\n",
                    indent.c_str(), nonGndDirs.size(), splitCharge);
            }

            for (auto dir : nonGndDirs)
            {
                auto newPathVisited = pathVisited;
                double branchPushedCharge = pushCharge(propPtr, dir, splitCharge, newPathVisited, depth);
                loopPushedCharge += branchPushedCharge;
                if (branchPushedCharge < EPSILON) dirsToRemove.push_back(dir);
            }

            for (auto dir : dirsToRemove) possibleDirs.erase(std::remove(possibleDirs.begin(), possibleDirs.end(), dir), possibleDirs.end());
            remainingCharge -= loopPushedCharge;
        }

        if (debug::printCircuitLog && (gndPushedCharge > EPSILON || loopPushedCharge > EPSILON))
        {
            dbgPrt(L"%s  [DIV-RESULT] 루프%d: GND=%.2f, 일반=%.2f, 잔여=%.2f\n",
                indent.c_str(), loopCount, gndPushedCharge, loopPushedCharge, remainingCharge);
        }

        if (loopPushedCharge < EPSILON && gndPushedCharge < EPSILON) break;
    }

    if (debug::printCircuitLog)
    {
        dbgPrt(L"%s[DIVIDE-END] (%d,%d)%ls 총 %d회 반복, 미분배=%.2f\n",
            indent.c_str(),
            propPtr->getGridX(), propPtr->getGridY(), propPtr->leadItem.name.c_str(),
            loopCount, remainingCharge);
    }
}


void Prop::transferCharge(Prop* thisProp, Prop* nextProp, double txChargeAmount, const std::wstring& indent, dir16 txDir, bool isGroundTransfer = false)
{
    if (txChargeAmount < EPSILON)
    {
        if (debug::printCircuitLog)
        {
            dbgPrt(L"%s[전송 스킵] (%d,%d)%ls → (%d,%d)%ls 양:%.8f (EPSILON 미만)\n",
                indent.c_str(),
                thisProp->getGridX(), thisProp->getGridY(), thisProp->leadItem.name.c_str(),
                nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
                txChargeAmount);
        }
        return;
    }

    double current = txChargeAmount / (SYSTEM_VOLTAGE * TIME_PER_TURN);
    double electricLoss = current * current * thisProp->leadItem.electricResistance * TIME_PER_TURN;
    double requiredFromDonor = txChargeAmount + electricLoss;
    thisProp->totalLossCharge += electricLoss;

    if (requiredFromDonor > thisProp->nodeCharge + EPSILON)
    {
        double availableRatio = thisProp->nodeCharge / requiredFromDonor;
        requiredFromDonor = thisProp->nodeCharge;
        txChargeAmount *= availableRatio;
        electricLoss = requiredFromDonor - txChargeAmount;
    }

    thisProp->nodeCharge -= requiredFromDonor;
    thisProp->chargeFlux[txDir] -= txChargeAmount;
    if (thisProp->leadItem.itemCode == itemID::powerBankR || thisProp->leadItem.itemCode == itemID::powerBankT || thisProp->leadItem.itemCode == itemID::powerBankL || thisProp->leadItem.itemCode == itemID::powerBankB)
        thisProp->leadItem.powerStorage -= requiredFromDonor;

    if (isGroundTransfer == false) nextProp->nodeCharge += txChargeAmount;
    nextProp->chargeFlux[reverse(txDir)] += txChargeAmount;

    if (debug::printCircuitLog)
    {
        if (isGroundTransfer)
        {
            dbgPrt(L"\x1b[33m%s[전송 GND] (%d,%d)%ls [%.2f→%.2f] → (%d,%d)%ls 전송:%.2f 손실:%.2f 부하:%.2f/%d\x1b[0m\n",
                indent.c_str(),
                thisProp->getGridX(), thisProp->getGridY(), thisProp->leadItem.name.c_str(),
                thisProp->nodeCharge + requiredFromDonor, thisProp->nodeCharge,
                nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
                txChargeAmount, electricLoss,
                nextProp->getTotalChargeFlux(), nextProp->leadItem.gndUsePower);
        }
        else
        {
            dbgPrt(L"%s[전송] (%d,%d)%ls [%.2f→%.2f] → (%d,%d)%ls [%.2f/%d] 전송:%.2f 손실:%.2f\n",
                indent.c_str(),
                thisProp->getGridX(), thisProp->getGridY(), thisProp->leadItem.name.c_str(),
                thisProp->nodeCharge + requiredFromDonor, thisProp->nodeCharge,
                nextProp->getGridX(), nextProp->getGridY(), nextProp->leadItem.name.c_str(),
                nextProp->nodeCharge, nextProp->nodeMaxCharge,
                txChargeAmount, electricLoss);
        }
    }
}

void Prop::initChargeBFS(std::queue<Point3> startPointSet)
{
    std::queue<Point3> frontierQueue = startPointSet;
    std::unordered_set<Point3> visitedSet;

    while (!frontierQueue.empty())
    {
        Point3 current = frontierQueue.front();
        frontierQueue.pop();

        if (visitedSet.find(current) != visitedSet.end()) continue;
        visitedSet.insert(current);

        Prop* thisProp = TileProp(current.x, current.y, current.z);
        if (thisProp)
        {
            thisProp->nodeCharge = thisProp->nodeMaxCharge;
            thisProp->gndSink = 0;
        }


        const dir16 directions[] = { dir16::right, dir16::up, dir16::left, dir16::down, dir16::above, dir16::below };
        for (int i = 0; i < 6; ++i)
        {
            if (isCableConnected(current, directions[i]))
            {
                int dx, dy, dz;
                dirToXYZ(directions[i], dx, dy, dz);
                Point3 nextCoord = { current.x + dx, current.y + dy, current.z + dz };
                Prop* nextProp = TileProp(nextCoord.x, nextCoord.y, nextCoord.z);
                if (nextProp != nullptr)
                {
                    //만약 BFS로 해당 방향으로 추가했을 때 GND가 지향성 GND 전력요구를 가지고 있을 경우
                    ItemData& nextItem = nextProp->leadItem;

                    thisProp->chargeFlux[directions[i]] = 0;
                    nextProp->chargeFlux[reverse(directions[i])] = 0;

                    if (directions[i] == dir16::right && nextItem.gndUsePowerLeft > 0)
                        nextProp->gndSinkLeft = 0;
                    else if (directions[i] == dir16::up && nextItem.gndUsePowerDown > 0)
                        nextProp->gndSinkDown = 0;
                    else if (directions[i] == dir16::left && nextItem.gndUsePowerRight > 0)
                        nextProp->gndSinkRight = 0;
                    else if (directions[i] == dir16::down && nextItem.gndUsePowerUp > 0)
                        nextProp->gndSinkUp = 0;

                }
                frontierQueue.push(nextCoord);
            }
        }
    }
}

/*
* loadAct(): 전하 계산 완료 후 호출되어 부하의 ON/OFF 상태 결정
* 호출 시점: updateCircuitNetwork() 완료 후, 턴 사이클에서 별도 호출
* 판정 기준: chargeFlux(받은 전하량) >= gndUsePower(필요 전력)
* ※ 충전포트같이 이하여도 작동하는 예외 존재하니 유의할 것
*/
void Prop::loadAct()
{

    int iCode = leadItem.itemCode;

    //모든 계산이 종료된 후 부하에 공급된 전하량이 usePower 이상인지 이하인지 판단하여 부하 프롭이 켜지거나 꺼짐
    //단 논리게이트들은 공급된 전하량이 아니라 별도의 로직으로 처리
    if (iCode == itemID::transistorR
        || iCode == itemID::transistorU
        || iCode == itemID::transistorL
        || iCode == itemID::transistorD)
    {
        bool baseInput = false;

        if (iCode == itemID::transistorR && gndSinkRight >= 1.0) baseInput = true;
        else if (iCode == itemID::transistorU && gndSinkUp >= 1.0) baseInput = true;
        else if (iCode == itemID::transistorL && gndSinkLeft >= 1.0) baseInput = true;
        else if (iCode == itemID::transistorD && gndSinkDown >= 1.0) baseInput = true;

        if (baseInput)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::relayR
        || iCode == itemID::relayU
        || iCode == itemID::relayL
        || iCode == itemID::relayD)
    {
        bool baseInput = false;

        if (iCode == itemID::relayR && gndSinkRight >= 1.0) baseInput = true;
        else if (iCode == itemID::relayU && gndSinkUp >= 1.0) baseInput = true;
        else if (iCode == itemID::relayL && gndSinkLeft >= 1.0) baseInput = true;
        else if (iCode == itemID::relayD && gndSinkDown >= 1.0) baseInput = true;

        if (baseInput)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::andGateR || iCode == itemID::andGateU || iCode == itemID::andGateL || iCode == itemID::andGateD)
    {
        bool firstInput, secondInput;

        if (iCode == itemID::andGateR)
        {
            firstInput = gndSinkLeft >= 1.0;
            secondInput = gndSinkDown >= 1.0;
        }
        else if (iCode == itemID::andGateU)
        {
            firstInput = gndSinkDown >= 1.0;
            secondInput = gndSinkRight >= 1.0;
        }
        else if (iCode == itemID::andGateL)
        {
            firstInput = gndSinkRight >= 1.0;
            secondInput = gndSinkUp >= 1.0;
        }
        else
        {
            firstInput = gndSinkUp >= 1.0;
            secondInput = gndSinkLeft >= 1.0;
        }

        if (firstInput && secondInput)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::orGateR || iCode == itemID::orGateU || iCode == itemID::orGateL || iCode == itemID::orGateD)
    {
        bool firstInput, secondInput;

        if (iCode == itemID::orGateR)
        {
            firstInput = gndSinkLeft >= 1.0;
            secondInput = gndSinkDown >= 1.0;
        }
        else if (iCode == itemID::orGateU)
        {
            firstInput = gndSinkDown >= 1.0;
            secondInput = gndSinkRight >= 1.0;
        }
        else if (iCode == itemID::orGateL)
        {
            firstInput = gndSinkRight >= 1.0;
            secondInput = gndSinkUp >= 1.0;
        }
        else
        {
            firstInput = gndSinkUp >= 1.0;
            secondInput = gndSinkLeft >= 1.0;
        }

        if (firstInput || secondInput)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::xorGateR || iCode == itemID::xorGateU || iCode == itemID::xorGateL || iCode == itemID::xorGateD)
    {
        bool firstInput, secondInput;

        if (iCode == itemID::xorGateR)
        {
            firstInput = gndSinkLeft >= 1.0;
            secondInput = gndSinkDown >= 1.0;
        }
        else if (iCode == itemID::xorGateU)
        {
            firstInput = gndSinkDown >= 1.0;
            secondInput = gndSinkRight >= 1.0;
        }
        else if (iCode == itemID::xorGateL)
        {
            firstInput = gndSinkRight >= 1.0;
            secondInput = gndSinkUp >= 1.0;
        }
        else
        {
            firstInput = gndSinkUp >= 1.0;
            secondInput = gndSinkLeft >= 1.0;
        }

        if (firstInput != secondInput)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::notGateR || iCode == itemID::notGateU || iCode == itemID::notGateL || iCode == itemID::notGateD)
    {
        bool inputActive;
        if (iCode == itemID::notGateR) inputActive = gndSinkLeft >= 1.0;
        else if (iCode == itemID::notGateU) inputActive = gndSinkDown >= 1.0;
        else if (iCode == itemID::notGateL) inputActive = gndSinkRight >= 1.0;
        else inputActive = gndSinkUp >= 1.0;

        if (inputActive == false)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::srLatchR || iCode == itemID::srLatchU || iCode == itemID::srLatchL || iCode == itemID::srLatchD)
    {
        bool setInput, resetInput;

        if (iCode == itemID::srLatchR)
        {
            setInput = gndSinkLeft >= 1.0;
            resetInput = gndSinkDown >= 1.0;
        }
        else if (iCode == itemID::srLatchU)
        {
            setInput = gndSinkDown >= 1.0;
            resetInput = gndSinkRight >= 1.0;
        }
        else if (iCode == itemID::srLatchL)
        {
            setInput = gndSinkRight >= 1.0;
            resetInput = gndSinkUp >= 1.0;
        }
        else
        {
            setInput = gndSinkUp >= 1.0;
            resetInput = gndSinkLeft >= 1.0;
        }

        if (setInput && resetInput) // 금지상태는 랜덤
        {
            if (randomRange(0, 1) == 0)
            {
                if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
                    propTurnOn();
            }
            else
            {
                if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
                    propTurnOff();
            }
        }
        else if (setInput) // Set
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
                propTurnOn();
        }
        else if (resetInput) // Reset
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
                propTurnOff();
        }
    }
    else if (iCode == itemID::delayR || iCode == itemID::delayU || iCode == itemID::delayL || iCode == itemID::delayD)
    {
        reserveDelayInit.erase(this);

        bool inputActive;
        if (iCode == itemID::delayR) inputActive = gndSinkLeft >= 1.0;
        else if (iCode == itemID::delayU) inputActive = gndSinkDown >= 1.0;
        else if (iCode == itemID::delayL) inputActive = gndSinkRight >= 1.0;
        else inputActive = gndSinkUp >= 1.0;

        if (inputActive == true)
        {
            if (delayStartTurn == 0.0) delayStartTurn = getElapsedTurn();

            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                if (getElapsedTurn() - delayStartTurn >= static_cast<double>(delayMaxStack) - EPSILON) propTurnOn();
            }
        }
        else
        {
            reserveDelayInit.insert(this);
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
    else if (iCode == itemID::powerBankR || iCode == itemID::powerBankT || iCode == itemID::powerBankL || iCode == itemID::powerBankB) //파워뱅크 충전의 경우 부하에 미달해도 작동
    {
        ItemData& loadItem = leadItem;
        double inletCharge = 0;
        if (iCode == itemID::powerBankR) inletCharge = gndSinkLeft;
        else if (iCode == itemID::powerBankT) inletCharge = gndSinkDown;
        else if (iCode == itemID::powerBankL) inletCharge = gndSinkRight;
        else inletCharge = gndSinkUp;

        loadItem.powerStorage += inletCharge;

        constexpr double CHARGE_EPSILON = 0.5;
        if (loadItem.powerStorage >= loadItem.powerStorageMax - CHARGE_EPSILON)
        {
            loadItem.powerStorage = loadItem.powerStorageMax;
        }
    }
    else if (iCode == itemID::chargingPort)
    {
        ItemStack* hereStack = TileItemStack(getGridX(), getGridY(), getGridZ());
        if (hereStack != nullptr)
        {
            std::vector<ItemData>& items = hereStack->getPocket()->itemInfo;
            double inletCharge = gndSink;  // 충전포트는 무방향 부하
            if (inletCharge > 0)
            {
                // 충전 가능한 아이템 인덱스 수집
                std::vector<int> chargeableIndices;
                for (int i = 0; i < items.size(); i++)
                {
                    if (items[i].itemCode == itemID::battery || items[i].itemCode == itemID::batteryPack)
                    {
                        if (items[i].powerStorage < items[i].powerStorageMax)
                        {
                            chargeableIndices.push_back(i);
                        }
                    }
                }

                // 균등 분배
                if (chargeableIndices.size() > 0)
                {
                    double chargePerItem = inletCharge / chargeableIndices.size();

                    for (int idx : chargeableIndices)
                    {
                        ItemData& item = items[idx];
                        double remaining = item.powerStorageMax - item.powerStorage;
                        double actualCharge = std::min(chargePerItem, remaining);

                        item.powerStorage += actualCharge;

                        constexpr double CHARGE_EPSILON = 0.5;
                        if (item.powerStorage >= item.powerStorageMax - CHARGE_EPSILON)
                        {
                            item.powerStorage = item.powerStorageMax;
                        }
                    }
                }
            }
        }
    }
    else //일반적인 부하들은 gndSink가 usePower 이상이면 켜지고 아니면 꺼짐
    {
        if (gndSink >= static_cast<double>(leadItem.gndUsePower))
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
            {
                propTurnOn();
            }
        }
        else
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {
                propTurnOff();
            }
        }
    }
}
