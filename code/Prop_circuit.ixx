import Prop;

import std;
import util;
import globalVar;
import constVar;
import wrapVar;

constexpr bool DEBUG_CIRCUIT_LOG = true;

void Prop::updateCircuitNetwork()
{
    int cursorX = getGridX();
    int cursorY = getGridY();
    int cursorZ = getGridZ();

    std::queue<Point3> frontierQueue;
    std::unordered_set<Point3, Point3::Hash> visitedSet;
    std::vector<Point3> visitedVec;//����׿� ���߿� ���� ��
    std::vector<Prop*> voltagePropVec;
    std::vector<Prop*> loadVec; //���ϰ� �������� ���ڱ���

    int circuitMaxEnergy = 0;
    int circuitTotalLoad = 0;
    bool hasGround = false;


    //==============================================================================
    // 1. ȸ�� ���� Ž��(BFS)
    //==============================================================================
    frontierQueue.push({ cursorX, cursorY, cursorZ });
    while (!frontierQueue.empty())
    {
        Point3 current = frontierQueue.front();
        frontierQueue.pop();

        if (visitedSet.find(current) != visitedSet.end()) continue;
        visitedSet.insert(current);
        visitedVec.push_back(current);


        Prop* currentProp = TileProp(current.x, current.y, current.z);

        if (currentProp && (currentProp->leadItem.checkFlag(itemFlag::CIRCUIT) || currentProp->leadItem.checkFlag(itemFlag::CABLE)))
        {
            currentProp->runUsed = true;
            currentProp->voltageDir.clear();

            currentProp->nodeInputElectron = 0;
            currentProp->nodeOutputElectron = 0;

            if (currentProp->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
            {
                if (currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) &&
                    currentProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
                {
                    circuitMaxEnergy += currentProp->leadItem.electricMaxPower;
                    voltagePropVec.push_back(currentProp);
                }
            }

            if (currentProp->leadItem.electricUsePower > 0) loadVec.push_back(currentProp);



            const dir16 directions[] = { dir16::right, dir16::up, dir16::left, dir16::down, dir16::ascend, dir16::descend };
            const itemFlag groundFlags[][2] = {
                { itemFlag::VOLTAGE_GND_LEFT, itemFlag::VOLTAGE_GND_ALL },
                { itemFlag::VOLTAGE_GND_DOWN, itemFlag::VOLTAGE_GND_ALL },
                { itemFlag::VOLTAGE_GND_RIGHT, itemFlag::VOLTAGE_GND_ALL },
                { itemFlag::VOLTAGE_GND_UP, itemFlag::VOLTAGE_GND_ALL },
                { itemFlag::VOLTAGE_GND_ALL, itemFlag::VOLTAGE_GND_ALL },
                { itemFlag::VOLTAGE_GND_ALL, itemFlag::VOLTAGE_GND_ALL }
            };

            for (int i = 0; i < 6; ++i)
            {
                if (isConnected(current, directions[i]))
                {
                    int dx, dy, dz;
                    dirToXYZ(directions[i], dx, dy, dz);
                    Point3 nextCoord = { current.x + dx, current.y + dy, current.z + dz };
                    Prop* nextProp = TileProp(nextCoord.x, nextCoord.y, nextCoord.z);

                    frontierQueue.push(nextCoord);

                    if (nextProp &&
                        (nextProp->leadItem.checkFlag(groundFlags[i][0]) ||
                            nextProp->leadItem.checkFlag(groundFlags[i][1])))
                    {
                        hasGround = true;
                    }
                }
            }
        }
    }

    for (int i = 0; i < loadVec.size(); i++)
    {
        circuitTotalLoad += loadVec[i]->leadItem.electricUsePower;
    }

    // ��尡 2�� �̸��̸� ������� ����
    if (visitedSet.size() < 2)
    {
        runUsed = true;
        return;
    }


    //std::wprintf(L"\n����������������������������������������������������������������������\n");
    //std::wprintf(L"�� ȸ�� �м� �Ϸ�                  ��\n");
    //std::wprintf(L"����������������������������������������������������������������������\n");
    //std::wprintf(L"�� ��� ��: %3d                    ��\n", visitedSet.size());
    //std::wprintf(L"�� �ִ� ����: %3d kJ               ��\n", circuitMaxEnergy);
    //std::wprintf(L"�� ���� ����: %s                   ��\n", hasGround ? L"Yes" : L"No ");
    //std::wprintf(L"����������������������������������������������������������������������\n\n");

    //==============================================================================
    // 2. �ִ� ���� ����
    //==============================================================================
    for (auto coord : visitedSet)
    {
        Prop* propPtr = TileProp(coord.x, coord.y, coord.z);
        if (propPtr != nullptr)
        {
            propPtr->nodeMaxElectron = circuitMaxEnergy;
        }
    }

    //==============================================================================
    // 3. ���п� ���� ����
    //==============================================================================
    double totalPushedElectron = 0;

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
        voltProp->nodeElectron = voltProp->nodeMaxElectron;
        int x = voltProp->getGridX();
        int y = voltProp->getGridY();
        int z = voltProp->getGridZ();
        double voltRatio = (double)voltProp->leadItem.electricMaxPower / (double)totalAvailablePower;
        double voltOutputPower = myMin(std::ceil(circuitTotalLoad * voltRatio), voltProp->leadItem.electricMaxPower);
        voltProp->prevPushedElectron = 0;
        voltOutputPower *= voltProp->lossCompensationFactor;  // ���׼ս� ���� ���� (�⺻�� 110%)

        if (DEBUG_CIRCUIT_LOG) std::wprintf(L"========================�����п� %p : �о�� ���ۡ�========================\n", voltProp);
        if (voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) || voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
        {
            if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT) && isConnected({ x,y,z }, dir16::right))
                voltProp->prevPushedElectron += pushElectron(voltProp, dir16::right, voltOutputPower, {}, 0);
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_UP) && isConnected({ x,y,z }, dir16::up))
                voltProp->prevPushedElectron += pushElectron(voltProp, dir16::up, voltOutputPower, {}, 0);
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT) && isConnected({ x,y,z }, dir16::left))
                voltProp->prevPushedElectron += pushElectron(voltProp, dir16::left, voltOutputPower, {}, 0);
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_DOWN) && isConnected({ x,y,z }, dir16::down))
                voltProp->prevPushedElectron += pushElectron(voltProp, dir16::down, voltOutputPower, {}, 0);

            totalPushedElectron += voltProp->prevPushedElectron;
            voltProp->nodeElectron = voltProp->nodeMaxElectron;
        }
    }


    //==============================================================================
    // 4. ���� ��ǰ�� ���� �Ҹ�
    //==============================================================================
    for (int i = 0; i < loadVec.size(); i++)
    {
        Prop* loadProp = loadVec[i];
        if (loadProp->groundChargeEnergy >= static_cast<double>(loadProp->leadItem.electricUsePower))
        {
            if (loadProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF))
                loadProp->propTurnOn();
        }
        else
        {
            if (loadProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON))
                loadProp->propTurnOff();
        }

        loadProp->groundChargeEnergy = 0;
    }


    //==============================================================================
    // 5. ���� ���� ���
    //==============================================================================
    //std::wprintf(L"\n����������������������������������������������������������������������������\n");
    //std::wprintf(L"�� ���� ȸ�� ����                     ��\n");
    //std::wprintf(L"����������������������������������������������������������������������������\n");

    //for (auto coord : visitedVec)
    //{
    //    Prop* propPtr = TileProp(coord.x, coord.y, coord.z);
    //    if (propPtr != nullptr)
    //    {
    //        std::wstring nodeType = L"Wire";
    //        if (propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE)) nodeType = L"Gen ";
    //        else if (propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_ALL) ||
    //            propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_UP) ||
    //            propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_DOWN) ||
    //            propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_LEFT) ||
    //            propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_RIGHT))
    //            nodeType = L"GND ";

    //        wchar_t status = L'��';
    //        if (propPtr->nodeElectron == 0) status = L'��';
    //        else if (propPtr->nodeElectron == propPtr->nodeMaxElectron) status = L'��';
    //        else status = L'��';

    //        std::wprintf(L"�� %s (%3d,%3d): %3d/%3d %c     ��\n",
    //            nodeType.c_str(),
    //            coord.x, coord.y,
    //            propPtr->nodeElectron, propPtr->nodeMaxElectron,
    //            status);
    //    }
    //}

    //std::wprintf(L"����������������������������������������������������������������������������\n\n");
}

bool Prop::isConnected(Point3 currentCoord, dir16 dir)
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
    case dir16::ascend:
        delCoord = { 0,0,+1 };
        hostFlag = itemFlag::CABLE_Z_ASCEND;
        guestFlag = itemFlag::CABLE_Z_DESCEND;
        break;
    case dir16::descend:
        delCoord = { 0,0,-1 };
        hostFlag = itemFlag::CABLE_Z_DESCEND;
        guestFlag = itemFlag::CABLE_Z_ASCEND;
        break;
    default:
        errorBox(L"[Error] isConnected lambda function received invalid direction argument.\n");
        break;
    }
    Prop* targetProp = TileProp(currentCoord.x + delCoord.x, currentCoord.y + delCoord.y, currentCoord.z + delCoord.z);

    if (targetProp == nullptr) return false;

    if ((dir == dir16::right || dir == dir16::left) && targetProp->leadItem.itemCode == itemRefCode::leverRL)
    {
        if (targetProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }
    else if ((dir == dir16::up || dir == dir16::down) && targetProp->leadItem.itemCode == itemRefCode::leverUD)
    {
        if (targetProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF)) return false;
    }

    if (dir == dir16::ascend || dir == dir16::descend)
    {
        if (currentProp->leadItem.checkFlag(itemFlag::CABLE) && currentProp->leadItem.checkFlag(hostFlag) &&
            targetProp->leadItem.checkFlag(itemFlag::CABLE) && targetProp->leadItem.checkFlag(guestFlag))
        {
            return true;
        }
        else return false;
    }
    else if (dir == dir16::right || dir == dir16::up || dir == dir16::left || dir == dir16::down)
    {
        if ((currentProp->leadItem.checkFlag(itemFlag::CABLE) || currentProp->leadItem.checkFlag(hostFlag)) &&
            (targetProp->leadItem.checkFlag(itemFlag::CABLE) || targetProp->leadItem.checkFlag(guestFlag)))
            return true;
        else return false;
    }
    else errorBox(L"[Error] isConnected lambda function received invalid direction argument.\n");
}

bool Prop::isGround(Point3 currentCoord, dir16 dir)
{
    Prop* currentProp = TileProp(currentCoord.x, currentCoord.y, currentCoord.z);
    Point3 delCoord = { 0,0,0 };
    itemFlag groundFlag;
    switch (dir)
    {
    case dir16::right:
        delCoord = { +1,0,0 };
        groundFlag = itemFlag::VOLTAGE_GND_LEFT;
        break;
    case dir16::up:
        delCoord = { 0,-1,0 };
        groundFlag = itemFlag::VOLTAGE_GND_DOWN;
        break;
    case dir16::left:
        delCoord = { -1,0,0 };
        groundFlag = itemFlag::VOLTAGE_GND_RIGHT;
        break;
    case dir16::down:
        delCoord = { 0,+1,0 };
        groundFlag = itemFlag::VOLTAGE_GND_UP;
        break;
    case dir16::ascend:
    case dir16::descend:
        groundFlag = itemFlag::VOLTAGE_GND_ALL;
        break;
    default:
        errorBox(L"[Error] isGround lambda function received invalid direction argument.\n");
        break;
    }
    Prop* targetProp = TileProp(currentCoord.x + delCoord.x, currentCoord.y + delCoord.y, currentCoord.z + delCoord.z);
    if (targetProp == nullptr) return false;
    if (targetProp->leadItem.checkFlag(groundFlag) || targetProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_ALL))
    {
        //�ߺ��ΰ�? ����ȭ�� ���ؼ� ��� �� ����
        if (isConnected(currentCoord, dir)) return true;
    }
    else return false;
}

bool Prop::isConnected(Prop* currentProp, dir16 dir)
{
    return isConnected({ currentProp->getGridX(),currentProp->getGridY(),currentProp->getGridZ() }, dir);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Prop::pushElectron(Prop* donorProp, dir16 txDir, double txElectronAmount, std::unordered_set<Prop*> pathVisited, int depth)
{
    errorBox(donorProp == nullptr, L"[Error] pushElectron: null donor\n");
    int dx, dy, dz;
    dirToXYZ(txDir, dx, dy, dz);
    Prop* acceptorProp = TileProp(donorProp->getGridX() + dx, donorProp->getGridY() + dy, donorProp->getGridZ() + dz);
    errorBox(acceptorProp == nullptr, L"[Error] pushElectron: no acceptor found\n");

    errorBox(txElectronAmount > donorProp->nodeElectron, L"[Error] pushElectron: insufficient electron\n");
    errorBox(!isConnected({ donorProp->getGridX(), donorProp->getGridY(), donorProp->getGridZ() }, txDir),
        L"[Error] pushElectron: not connected\n");

    if (pathVisited.find(donorProp) != pathVisited.end()) return 0;
    pathVisited.insert(donorProp);
    if (pathVisited.find(acceptorProp) != pathVisited.end()) return 0;

    // �鿩���� ����
    std::wstring indent(depth * 2, L' ');  // depth���� 2ĭ��
    std::wprintf(L"%s[PUSH] (%d,%d) �� (%d,%d) �õ�: %.2f\n",
        indent.c_str(),
        donorProp->getGridX(), donorProp->getGridY(),
        acceptorProp->getGridX(), acceptorProp->getGridY(),
        txElectronAmount);

    bool isGrounded = false;
    if (acceptorProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_ALL)) isGrounded = true;
    else if (txDir == dir16::right && acceptorProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_LEFT))
    {
        isGrounded = true;
    }
    else if (txDir == dir16::up && acceptorProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_DOWN))
    {
        isGrounded = true;
    }
    else if (txDir == dir16::left && acceptorProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_RIGHT))
    {
        isGrounded = true;
    }
    else if (txDir == dir16::down && acceptorProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_UP))
    {
        isGrounded = true;
    }

    if (isGrounded)
    {
        double remainEnergy = acceptorProp->leadItem.electricUsePower - acceptorProp->groundChargeEnergy;

        if (remainEnergy > 0)
        {
            double consumeEnergy = std::min(txElectronAmount, remainEnergy);

            donorProp->nodeElectron -= consumeEnergy;
            donorProp->nodeOutputElectron += consumeEnergy;
            acceptorProp->groundChargeEnergy += consumeEnergy;
            acceptorProp->nodeInputElectron += consumeEnergy;

            if (DEBUG_CIRCUIT_LOG)
            {
                std::wprintf(L"%s[����-GND] (%d,%d)[%.2f] �� (%d,%d)[GND]: ��û=%.2f, �Ҹ�=%.2f (���� ��������=%.2f)\n",
                    indent.c_str(),
                    donorProp->getGridX(), donorProp->getGridY(),
                    donorProp->nodeElectron + consumeEnergy,  // ���� �� ����
                    acceptorProp->getGridX(), acceptorProp->getGridY(),
                    txElectronAmount,
                    consumeEnergy,
                    remainEnergy - consumeEnergy);
            }

            return consumeEnergy;
        }
        else return 0;
    }

    double pushedElectron = std::min(txElectronAmount, acceptorProp->nodeElectron);

    double dividedElectron = 0;
    if (pushedElectron > 0)
    {
        std::vector<dir16> possibleDirs;
        for (auto dir : { dir16::right, dir16::up, dir16::left, dir16::down, dir16::ascend, dir16::descend })
        {
            if (dir == reverse(txDir)) continue;
            if (isConnected({ acceptorProp->getGridX(), acceptorProp->getGridY(), acceptorProp->getGridZ() }, dir))
            {
                possibleDirs.push_back(dir);
            }
        }

        if (possibleDirs.empty() == false)
        {
            dividedElectron = divideElectron(acceptorProp, pushedElectron, possibleDirs, pathVisited, depth + 1);
        }
    }

    double finalTxElectron = std::min(txElectronAmount, acceptorProp->nodeMaxElectron - acceptorProp->nodeElectron);

    if (DEBUG_CIRCUIT_LOG)
    {
        std::wprintf(L"%s[����] (%d,%d)[%.2f] �� (%d,%d)[%.2f/%.2f]: ��û=%.2f, ����=%.2f\n",
            indent.c_str(),
            donorProp->getGridX(), donorProp->getGridY(),
            donorProp->nodeElectron,
            acceptorProp->getGridX(), acceptorProp->getGridY(),
            acceptorProp->nodeElectron, acceptorProp->nodeMaxElectron,
            txElectronAmount, finalTxElectron);
    }

    donorProp->nodeElectron -= finalTxElectron;
    donorProp->nodeOutputElectron += finalTxElectron;
    acceptorProp->nodeElectron += finalTxElectron;
    acceptorProp->nodeInputElectron += finalTxElectron;
    return finalTxElectron;
}


double Prop::divideElectron(Prop* propPtr, double inputElectron, std::vector<dir16> possibleDirs, std::unordered_set<Prop*> pathVisited, int depth)
{
    double totalPushedElectron = 0;
    double remainingElectron = inputElectron;
    std::vector<dir16> dirsToRemove;
    std::vector<dir16> gndDirs;
    std::vector<dir16> nonGndDirs;
    dirsToRemove.reserve(6);
    gndDirs.reserve(6);
    nonGndDirs.reserve(6);

    while (remainingElectron > 0 && !possibleDirs.empty())
    {
        dirsToRemove.clear();
        gndDirs.clear();
        nonGndDirs.clear();
        double gndPushedElectron = 0;
        double loopPushedElectron = 0;

        //���� �켱 ���

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
            double gndSplitElectron = remainingElectron / gndDirs.size();
            for (auto dir : gndDirs)
            {
                auto newPathVisited = pathVisited;
                double branchPushedElectron = pushElectron(propPtr, dir, gndSplitElectron, newPathVisited, depth);
                gndPushedElectron += branchPushedElectron;
                totalPushedElectron += branchPushedElectron;
                if (branchPushedElectron == 0) dirsToRemove.push_back(dir);
            }

            for (auto dir : dirsToRemove) possibleDirs.erase(std::remove(possibleDirs.begin(), possibleDirs.end(), dir), possibleDirs.end());
            remainingElectron -= gndPushedElectron;
        }

        dirsToRemove.clear();
        if (possibleDirs.empty()) break;

        if (nonGndDirs.size() > 0)
        {
            double splitElectron = remainingElectron / nonGndDirs.size();
            for (auto dir : nonGndDirs)
            {
                auto newPathVisited = pathVisited;
                double branchPushedElectron = pushElectron(propPtr, dir, splitElectron, newPathVisited, depth);
                loopPushedElectron += branchPushedElectron;
                totalPushedElectron += branchPushedElectron;
                if (branchPushedElectron == 0) dirsToRemove.push_back(dir);
            }

            for (auto dir : dirsToRemove) possibleDirs.erase(std::remove(possibleDirs.begin(), possibleDirs.end(), dir), possibleDirs.end());
            remainingElectron -= loopPushedElectron;
        }

        if (loopPushedElectron == 0 && gndPushedElectron == 0) break;
    }

    return totalPushedElectron;
}