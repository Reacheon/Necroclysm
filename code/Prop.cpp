import Prop;

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
        
        if(leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
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
        auto wireCheck = [=](int dx, int dy) -> bool
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

        if(getTimer()==1) leadItem.addFlag(itemFlag::STUMP);

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
                    new Particle(getX() + 16 + randomRange(-16, 16), getY() + randomRange(0, 8) , randomRange(0, 7), randomRangeFloat(-1.2,1.2), randomRangeFloat(-2.6,-3.2), 0.18, randomRange(25,35));
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

    if (leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
    {
        constexpr int GASOLINE_MAX_POWER = 50;

        if (leadItem.itemCode == itemRefCode::gasolineGeneratorR || leadItem.itemCode == itemRefCode::gasolineGeneratorT ||
            leadItem.itemCode == itemRefCode::gasolineGeneratorL || leadItem.itemCode == itemRefCode::gasolineGeneratorB)
        {
            if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
            {

            }
        }
    }

    if (leadItem.checkFlag(itemFlag::CIRCUIT))
    {
        int cursorX = getGridX();
        int cursorY = getGridY();
        int cursorZ = getGridZ();

        std::queue<Point3> frontierQueue;
        std::unordered_set<Point3, Point3::Hash> visitedSet;
        std::vector<Point3> visitedVec;//디버그용 나중에 지울 것
        std::vector<Prop*> voltagePropVec;

        int circuitMaxEnergy = 0;
        bool hasGround = false;

        auto isConnected = [&](Point3 currentCoord, dir16 dir) -> bool
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
            };

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //1. 회로 탐색
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

                if (currentProp->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
                {
                    circuitMaxEnergy += currentProp->leadItem.electricMaxPower;
                    voltagePropVec.push_back(currentProp);
                }

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

        // 노드가 2개 미만이면 출력하지 않음
        if (visitedSet.size() < 2)
        {
            runUsed = true;
            return;
        }

        std::wprintf(L"\n┌─────────────────────────────────┐\n");
        std::wprintf(L"│ 회로 분석 완료                  │\n");
        std::wprintf(L"├─────────────────────────────────┤\n");
        std::wprintf(L"│ 노드 수: %3d                    │\n", visitedSet.size());
        std::wprintf(L"│ 최대 전력: %3d kJ               │\n", circuitMaxEnergy);
        std::wprintf(L"│ 접지 존재: %s                   │\n", hasGround ? L"Yes" : L"No ");
        std::wprintf(L"└─────────────────────────────────┘\n\n");

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //2. 최대 전력 설정
        for (auto coord : visitedSet)
        {
            Prop* propPtr = TileProp(coord.x, coord.y, coord.z);
            if (propPtr != nullptr) propPtr->nodeMaxElectron = circuitMaxEnergy;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //3. 전력 전송 함수
        std::function<int(Prop*, dir16, int, std::unordered_set<Prop*>)> pushElectron;
        pushElectron = [&](Prop* donorProp, dir16 txDir, int txElectronAmount, std::unordered_set<Prop*> pathVisited) -> int
            {
                if (pathVisited.find(donorProp) != pathVisited.end()) return 0;
                pathVisited.insert(donorProp);

                errorBox(donorProp == nullptr, L"[Error] pushElectron: null donor\n");

                int dx, dy, dz;
                dirToXYZ(txDir, dx, dy, dz);
                Prop* acceptorProp = TileProp(donorProp->getGridX() + dx, donorProp->getGridY() + dy, donorProp->getGridZ() + dz);

                errorBox(acceptorProp == nullptr, L"[Error] pushElectron: null acceptor\n");
                errorBox(txElectronAmount > donorProp->nodeElectron, L"[Error] pushElectron: insufficient electron\n");
                errorBox(!isConnected({ donorProp->getGridX(), donorProp->getGridY(), donorProp->getGridZ() }, txDir),
                    L"[Error] pushElectron: not connected\n");

                if (txElectronAmount <= 0) return 0;

                // 접지 체크
                bool isGrounded = false;
                if (acceptorProp->leadItem.checkFlag(itemFlag::VOLTAGE_GND_ALL))
                {
                    isGrounded = true;
                }
                // 방향별 접지 체크
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
                    std::wprintf(L"  ⚡ GND: (%d,%d) 소모 %d\n",
                        acceptorProp->getGridX(), acceptorProp->getGridY(), txElectronAmount);
                    donorProp->nodeElectron -= txElectronAmount;
                    return txElectronAmount;
                }

                int pushedElectron = std::min(txElectronAmount, acceptorProp->nodeElectron);

                if (pushedElectron == 0)
                {
                    // Acceptor가 비어있음 - 직접 전송
                    int finalTxElectron = std::min(txElectronAmount, acceptorProp->nodeMaxElectron - acceptorProp->nodeElectron);
                    donorProp->nodeElectron -= finalTxElectron;
                    acceptorProp->nodeElectron += finalTxElectron;
                    return finalTxElectron;
                }
                else
                {
                    // Acceptor에 전자 있음 - 재귀로 밀어냄
                    std::vector<dir16> possibleDirs;
                    for (auto dir : { dir16::right, dir16::up, dir16::left, dir16::down, dir16::ascend, dir16::descend })
                    {
                        if (dir == reverse(txDir)) continue;
                        if (isConnected({ acceptorProp->getGridX(), acceptorProp->getGridY(), acceptorProp->getGridZ() }, dir))
                        {
                            possibleDirs.push_back(dir);
                        }
                    }

                    if (possibleDirs.empty()) return 0;

                    int outputElectron = 0;
                    int splitElectron = pushedElectron / possibleDirs.size();
                    for (auto dir : possibleDirs)
                    {
                        auto newPathVisited = pathVisited;
                        outputElectron += pushElectron(acceptorProp, dir, splitElectron, newPathVisited);
                    }

                    acceptorProp->nodeElectron -= outputElectron;

                    int finalTxElectron = std::min(txElectronAmount, acceptorProp->nodeMaxElectron - acceptorProp->nodeElectron);
                    donorProp->nodeElectron -= finalTxElectron;
                    acceptorProp->nodeElectron += finalTxElectron;

                    return outputElectron;
                }
            };

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //4. 전압원에서 전송 시작
        for (int i = 0; i < voltagePropVec.size(); i++)
        {
            Prop* voltProp = voltagePropVec[i];
            voltProp->nodeElectron = voltProp->nodeMaxElectron;
            int x = voltProp->getGridX();
            int y = voltProp->getGridY();
            int z = voltProp->getGridZ();

            if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT) && isConnected({ x,y,z }, dir16::right))
                pushElectron(voltProp, dir16::right, voltProp->nodeElectron, {});
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_UP) && isConnected({ x,y,z }, dir16::up))
                pushElectron(voltProp, dir16::up, voltProp->nodeElectron, {});
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT) && isConnected({ x,y,z }, dir16::left))
                pushElectron(voltProp, dir16::left, voltProp->nodeElectron, {});
            else if (voltProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_DOWN) && isConnected({ x,y,z }, dir16::down))
                pushElectron(voltProp, dir16::down, voltProp->nodeElectron, {});
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //5. 최종 상태 출력
        std::wprintf(L"\n┌────────────────────────────────────┐\n");
        std::wprintf(L"│ 최종 회로 상태                     │\n");
        std::wprintf(L"├────────────────────────────────────┤\n");

        for (auto coord : visitedVec)
        {
            Prop* propPtr = TileProp(coord.x, coord.y, coord.z);
            if (propPtr != nullptr)
            {
                std::wstring nodeType = L"Wire";
                if (propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE)) nodeType = L"Gen ";
                else if (propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_ALL) ||
                    propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_UP) ||
                    propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_DOWN) ||
                    propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_LEFT) ||
                    propPtr->leadItem.checkFlag(itemFlag::VOLTAGE_GND_RIGHT))
                    nodeType = L"GND ";

                wchar_t status = L'○';
                if (propPtr->nodeElectron == 0) status = L'○';
                else if (propPtr->nodeElectron == propPtr->nodeMaxElectron) status = L'●';
                else status = L'◐';

                std::wprintf(L"│ %s (%3d,%3d): %3d/%3d %c     │\n",
                    nodeType.c_str(),
                    coord.x, coord.y,
                    propPtr->nodeElectron, propPtr->nodeMaxElectron,
                    status);
            }
        }

        std::wprintf(L"└────────────────────────────────────┘\n\n");
    }

    runUsed = true;
}