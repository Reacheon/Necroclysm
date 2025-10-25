﻿import Prop;

#include <SDL3/SDL.h>

import util;
import globalVar;
import wrapVar;
import constVar;
import World;
import ItemStack;
import ItemPocket;
import ItemData;
import Ani;
import AI;
import Light;
import Coord;
import log;
import Drawable;
import Sticker;
import Particle;

constexpr bool DEBUG_CIRCUIT_LOG = true;

Prop::Prop(Point3 inputCoor, int leadItemCode)
{
    leadItem = cloneFromItemDex(itemDex[leadItemCode], 1);
    setAniPriority(3);
    //prt(L"[Prop:constructor] 생성자가 호출되었다. 생성된 좌표는 %d,%d,%d이다.\n", inputCoor.x, inputCoor.y, inputCoor.z);
    setGrid(inputCoor.x, inputCoor.y, inputCoor.z);

    errorBox(TileProp(inputCoor.x, inputCoor.y, inputCoor.z) != nullptr, L"생성위치에 이미 설치물이 존재한다!");

    if (leadItem.checkFlag(itemFlag::LIGHT_ON))
    {
        leadItem.lightPtr = std::make_unique<Light>(inputCoor.x + leadItem.lightDelX, inputCoor.y + leadItem.lightDelY, inputCoor.z, leadItem.lightRange, leadItem.lightIntensity, SDL_Color{ leadItem.lightR,leadItem.lightG,leadItem.lightB });//임시로 이렇게 만들어놨음
        //PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight);
    }


    deactivateAI();//차량을 제외하고 기본적으로 비활성화



    if (leadItem.randomPropSprSize != 1)
    {
        leadItem.propSprIndex += randomRange(0, leadItem.randomPropSprSize - 1);
    }




    //HP 설정
    leadItem.propHP = leadItem.propMaxHP;
    leadItem.propFakeHP = leadItem.propMaxHP;


}

Prop::~Prop()
{
    Point2 currentChunkCoord = World::ins()->changeToSectorCoord(getGridX(), getGridY());
    Chunk& currentChunk = World::ins()->getChunk(currentChunkCoord.x, currentChunkCoord.y, getGridZ());
    currentChunk.eraseProp(this);

    if (leadItem.checkFlag(itemFlag::CABLE))
    {
        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND))
        {
            Prop* abovePropPtr = TileProp(getGridX(), getGridY(), getGridZ() + 1);
            if (abovePropPtr != nullptr) abovePropPtr->leadItem.eraseFlag(itemFlag::CABLE_Z_DESCEND);
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            Prop* belowPropPtr = TileProp(getGridX(), getGridY(), getGridZ() - 1);
            if (belowPropPtr != nullptr) belowPropPtr->leadItem.eraseFlag(itemFlag::CABLE_Z_ASCEND);
        }
    }

    prt(L"[Prop:destructor] 소멸자가 호출되었다. \n");
}

void Prop::setGrid(int inputGridX, int inputGridY, int inputGridZ)
{
    Point2 prevChunkCoord = World::ins()->changeToSectorCoord(getGridX(), getGridY());
    Chunk& prevChunk = World::ins()->getChunk(prevChunkCoord.x, prevChunkCoord.y, getGridZ());
    prevChunk.eraseProp(this);

    Coord::setGrid(inputGridX, inputGridY, inputGridZ);

    Point2 currentChunkCoord = World::ins()->changeToSectorCoord(getGridX(), getGridY());
    Chunk& currentChunk = World::ins()->getChunk(currentChunkCoord.x, currentChunkCoord.y, getGridZ());
    currentChunk.addProp(this);
}

void Prop::updateSprIndex()
{
    bool topTile = false;
    bool botTile = false;
    bool leftTile = false;
    bool rightTile = false;


    if (leadItem.checkFlag(itemFlag::CABLE))//전선일 경우
    {
        auto wireCheck = [this](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::CABLE))//같은 전선일 경우
                    {
                        return true;
                    }
                    else
                    {
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_DOWN)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_UP)) return true;
                        else if ((dx == 1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_LEFT)) return true;
                        else if ((dx == -1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_RIGHT)) return true;
                        else return false;
                    }
                }
                else return false;
            };

        topTile = wireCheck(0, -1);
        botTile = wireCheck(0, 1);
        leftTile = wireCheck(-1, 0);
        rightTile = wireCheck(1, 0);
        leadItem.extraSprIndexSingle = connectGroupExtraIndex(topTile, botTile, leftTile, rightTile);
    }
    else if (leadItem.checkFlag(itemFlag::PIPE))//배관일 경우
    {
        auto pipeCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::PIPE))//같은 전선일 경우
                    {
                        return true;
                    }
                    else
                    {
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_DOWN)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_UP)) return true;
                        else if ((dx == 1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_LEFT)) return true;
                        else if ((dx == -1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_RIGHT)) return true;
                        else return false;
                    }
                }
                else return false;
            };

        topTile = pipeCheck(0, -1);
        botTile = pipeCheck(0, 1);
        leftTile = pipeCheck(-1, 0);
        rightTile = pipeCheck(1, 0);
        leadItem.extraSprIndexSingle = connectGroupExtraIndex(topTile, botTile, leftTile, rightTile);
    }
    else if (leadItem.checkFlag(itemFlag::CONVEYOR))//컨베이어 벨트일 경우
    {
        auto conveyorCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR))//같은 전선일 경우
                    {
                        return true;
                    }
                    else
                    {
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_DOWN)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_UP)) return true;
                        else if ((dx == 1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_LEFT)) return true;
                        else if ((dx == -1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_RIGHT)) return true;
                        else return false;
                    }
                }
                else return false;
            };

        topTile = conveyorCheck(0, -1);
        botTile = conveyorCheck(0, 1);
        leftTile = conveyorCheck(-1, 0);
        rightTile = conveyorCheck(1, 0);
        leadItem.extraSprIndexSingle = connectGroupExtraIndex(topTile, botTile, leftTile, rightTile);
    }
    else if (leadItem.tileConnectGroup != -1)
    {
        auto sameCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.tileConnectGroup != 0)
                    {
                        if (tgtProp->leadItem.tileConnectGroup == leadItem.tileConnectGroup)
                        {
                            return true;
                        }
                        else return false;
                    }
                    else
                    {
                        if (tgtProp->leadItem.itemCode == leadItem.itemCode)
                        {
                            return true;
                        }
                        else return false;
                    }
                }
                else return false;
            };

        int x = getGridX();
        int y = getGridY();

        topTile = sameCheck(0, -1);
        botTile = sameCheck(0, 1);
        leftTile = sameCheck(-1, 0);
        rightTile = sameCheck(1, 0);
        leadItem.extraSprIndexSingle = connectGroupExtraIndex(topTile, botTile, leftTile, rightTile);
    }
}

bool Prop::runAI()
{
    //prt(L"[Prop:AI] ID : %p의 AI를 실행시켰다.\n", this);
    while (1)
    {

        //prt(L"[Prop:AI] ID : %p의 turnResource는 %f입니다.\n", this, getTurnResource());
        if (getTurnResource() >= 2.0)
        {
            clearTurnResource();
            addTurnResource(2.0);
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //위의 모든 패턴 조건을 만족하지않을시 return true
        //prt(L"[Prop:AI] AI가 true를 반환했다. AI를 종료합니다.\n");
        return true;
    }
}

bool Prop::runAnimation(bool shutdown)
{
    //prt(L"Prop %p의 runAnimation이 실행되었다.\n", this);
    if (getAniType() == aniFlag::propRush)
    {
        addTimer();
    }
    else if (getAniType() == aniFlag::drop)
    {
        addTimer();
        int pX = getX();
        int pY = getY();

        if (getTimer() == 1) setFakeY(-4);
        else if (getTimer() == 2) setFakeY(-5);
        else if (getTimer() == 4) setFakeY(-6);
        else if (getTimer() == 7) setFakeY(-7);
        else if (getTimer() == 10) setFakeY(-6);
        else if (getTimer() == 12) setFakeY(-5);
        else if (getTimer() == 13) setFakeY(-4);
        else if (getTimer() == 16)
        {
            setFakeY(0);
            resetTimer();
            setAniType(aniFlag::null);
            return true;
        }
    }
    else if (getAniType() == aniFlag::treeFalling)
    {
        addTimer();

        if (getTimer() == 1) leadItem.addFlag(itemFlag::STUMP);

        if (PlayerX() <= getGridX()) treeAngle += 0.7 + (float)(getTimer()) * 0.04;
        else treeAngle -= 0.7 + (float)(getTimer()) * 0.04;

        if (treeAngle >= 90.0 || treeAngle <= -90.0)
        {
            Point3 itemPos;
            ItemStack* itemPtr1;
            ItemStack* itemPtr2;
            if (treeAngle >= 90.0)
            {
                if (1/*TileWall(getGridX() + 1, getGridY(), getGridZ()) == 0*/) itemPos = { getGridX() + 1, getGridY(), getGridZ() };
                else  itemPos = { getGridX(), getGridY(), getGridZ() };

                createItemStack(itemPos, { {392,1} });
                for (int i = 0; i < 8; i++)
                {
                    new Particle(getX() + 16 + randomRange(-16, 16), getY() + randomRange(0, 8), randomRange(0, 7), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
                }
            }
            else
            {
                if (1/*TileWall(getGridX() - 1, getGridY(), getGridZ()) == 0*/) itemPos = { getGridX() - 1, getGridY(), getGridZ() };
                else  itemPos = { getGridX(), getGridY(), getGridZ() };

                createItemStack(itemPos, { {392,1} });
                for (int i = 0; i < 8; i++)
                {
                    new Particle(getX() - 16 + randomRange(-16, 16), getY() + randomRange(0, 8), randomRange(0, 7), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
                }
            }


            addAniUSetPlayer(TileItemStack(itemPos.x, itemPos.y, itemPos.z), aniFlag::drop);
            treeAngle = 0;
            resetTimer();
            setAniType(aniFlag::null);
            return true;
        }
    }
    return false;
}





void Prop::runPropFunc()
{
    if (runUsed) return;

    if (leadItem.checkFlag(itemFlag::CIRCUIT))
    {
        int cursorX = getGridX();
        int cursorY = getGridY();
        int cursorZ = getGridZ();

        std::queue<Point3> frontierQueue;
        std::unordered_set<Point3, Point3::Hash> visitedSet;
        std::vector<Point3> visitedVec;//디버그용 나중에 지울 것
        std::vector<Prop*> voltagePropVec;
        std::vector<Prop*> loadVec; //부하가 가해지는 전자기기들

        int circuitMaxEnergy = 0;
        int circuitTotalLoad = 0;
        bool hasGround = false;


        //==============================================================================
        // 1. 회로 최초 탐색(BFS)
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

        // 노드가 2개 미만이면 출력하지 않음
        if (visitedSet.size() < 2)
        {
            runUsed = true;
            return;
        }


        //std::wprintf(L"\n┌─────────────────────────────────┐\n");
        //std::wprintf(L"│ 회로 분석 완료                  │\n");
        //std::wprintf(L"├─────────────────────────────────┤\n");
        //std::wprintf(L"│ 노드 수: %3d                    │\n", visitedSet.size());
        //std::wprintf(L"│ 최대 전력: %3d kJ               │\n", circuitMaxEnergy);
        //std::wprintf(L"│ 접지 존재: %s                   │\n", hasGround ? L"Yes" : L"No ");
        //std::wprintf(L"└─────────────────────────────────┘\n\n");

        //==============================================================================
        // 2. 최대 전력 설정
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
        // 3. 전압원 전송 시작
        //==============================================================================
        double totalPushedElectron = 0;

        randomShuffle(voltagePropVec.begin(), voltagePropVec.end());

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
            double voltOutputPower = myMin(ceil(circuitTotalLoad * ((double)voltProp->leadItem.electricMaxPower / (double)totalAvailablePower)), voltProp->leadItem.electricMaxPower);
            voltOutputPower *= voltProp->lossCompensationFactor;  // 저항손실 보존 변수 (기본값 110%)

            if (DEBUG_CIRCUIT_LOG) std::wprintf(L"========================▼전압원 %p : 밀어내기 시작▼========================\n", voltProp);
            if (voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_ON) || voltProp->leadItem.checkFlag(itemFlag::PROP_POWER_OFF) == false)
            {
                if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT) && isConnected({ x,y,z }, dir16::right))
                    totalPushedElectron += pushElectron(voltProp, dir16::right, voltOutputPower, {}, 0);
                else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_UP) && isConnected({ x,y,z }, dir16::up))
                    totalPushedElectron += pushElectron(voltProp, dir16::up, voltOutputPower, {}, 0);
                else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT) && isConnected({ x,y,z }, dir16::left))
                    totalPushedElectron += pushElectron(voltProp, dir16::left, voltOutputPower, {}, 0);
                else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_DOWN) && isConnected({ x,y,z }, dir16::down))
                    totalPushedElectron += pushElectron(voltProp, dir16::down, voltOutputPower, {}, 0);

                voltProp->nodeElectron = voltProp->nodeMaxElectron;
            }
        }

        //==============================================================================
        // 4. 부하 부품들 전력 소모
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
        // 5. 최종 상태 출력
        //==============================================================================
        //std::wprintf(L"\n┌────────────────────────────────────┐\n");
        //std::wprintf(L"│ 최종 회로 상태                     │\n");
        //std::wprintf(L"├────────────────────────────────────┤\n");

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

        //        wchar_t status = L'○';
        //        if (propPtr->nodeElectron == 0) status = L'○';
        //        else if (propPtr->nodeElectron == propPtr->nodeMaxElectron) status = L'●';
        //        else status = L'◐';

        //        std::wprintf(L"│ %s (%3d,%3d): %3d/%3d %c     │\n",
        //            nodeType.c_str(),
        //            coord.x, coord.y,
        //            propPtr->nodeElectron, propPtr->nodeMaxElectron,
        //            status);
        //    }
        //}

        //std::wprintf(L"└────────────────────────────────────┘\n\n");
    }

    runUsed = true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
        //중복인가? 최적화를 위해선 없어도 될 수도
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

    // 들여쓰기 생성
    std::wstring indent(depth * 2, L' ');  // depth마다 2칸씩
    std::wprintf(L"%s[PUSH] (%d,%d) → (%d,%d) 시도: %.2f\n",
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
                std::wprintf(L"%s[전송-GND] (%d,%d)[%.2f] → (%d,%d)[GND]: 요청=%.2f, 소모=%.2f (부하 남은수요=%.2f)\n",
                    indent.c_str(),
                    donorProp->getGridX(), donorProp->getGridY(),
                    donorProp->nodeElectron + consumeEnergy,  // 전송 전 상태
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
        std::wprintf(L"%s[전송] (%d,%d)[%.2f] → (%d,%d)[%.2f/%.2f]: 요청=%.2f, 전송=%.2f\n",
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


void Prop::propTurnOn()
{
    leadItem.eraseFlag(itemFlag::PROP_POWER_OFF);
    leadItem.addFlag(itemFlag::PROP_POWER_ON);

    int iCode = leadItem.itemCode;
    if (iCode == itemRefCode::bollardLight)
    {
        leadItem.lightPtr = std::make_unique<Light>(getGridX() + leadItem.lightDelX, getGridY() + leadItem.lightDelY, getGridZ(), leadItem.lightRange, leadItem.lightIntensity, SDL_Color{ leadItem.lightR,leadItem.lightG,leadItem.lightB });
    }

}

void Prop::propTurnOff()
{
    leadItem.eraseFlag(itemFlag::PROP_POWER_ON);
    leadItem.addFlag(itemFlag::PROP_POWER_OFF);

    int iCode = leadItem.itemCode;
    if (iCode == itemRefCode::bollardLight)
    {
        leadItem.lightPtr = nullptr;
    }
}