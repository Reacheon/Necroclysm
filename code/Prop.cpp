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
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_BOT)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_TOP)) return true;
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
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_BOT)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_TOP)) return true;
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
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_BOT)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_TOP)) return true;
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
    //prt(L"[Prop:runProp] ID : %p의 runProp를 실행시켰다.\n", this);

    if (runUsed) return;

    if (leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
    {
        constexpr int GASOLINE_MAX_POWER = 50;

        if (leadItem.itemCode == itemRefCode::gasolineGeneratorR || leadItem.itemCode == itemRefCode::gasolineGeneratorT || leadItem.itemCode == itemRefCode::gasolineGeneratorL || leadItem.itemCode == itemRefCode::gasolineGeneratorB)
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
        std::queue<Point3> voltageSourceQueue;
        
        int circuitMaxEnergy = 0; //kJ


        auto pushIfConnected = [&](int currentX, int currentY, int currentZ, int dx, int dy, int dz, itemFlag currentDirFlag, itemFlag neighborOppFlag)
            {
                Point3 dirCoord{ currentX + dx, currentY + dy, currentZ + dz };
                if (visitedSet.find(dirCoord) != visitedSet.end()) return;
                Prop* dirProp = TileProp(dirCoord.x, dirCoord.y, dirCoord.z);
                if (!dirProp) return;
                Prop* currentProp = TileProp(currentX, currentY, currentZ);
                if (dz == 0)
                {
                    bool curOK = currentProp->leadItem.checkFlag(currentDirFlag) || currentProp->leadItem.checkFlag(itemFlag::CABLE);
                    if (!curOK) return;
                    if (dirProp->leadItem.checkFlag(itemFlag::CIRCUIT) && (dirProp->leadItem.checkFlag(neighborOppFlag) || dirProp->leadItem.checkFlag(itemFlag::CABLE)))
                    {

                        frontierQueue.push(dirCoord);
                    }
                }
                else
                {
                    bool curOK = currentProp->leadItem.checkFlag(currentDirFlag) && currentProp->leadItem.checkFlag(itemFlag::CABLE);
                    if (!curOK) return;
                    if (dirProp->leadItem.checkFlag(itemFlag::CIRCUIT) && (dirProp->leadItem.checkFlag(neighborOppFlag) && dirProp->leadItem.checkFlag(itemFlag::CABLE)))
                    {
                        frontierQueue.push(dirCoord);
                    }
                }
            };

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
                    hostFlag = itemFlag::CABLE_CNCT_TOP;
                    guestFlag = itemFlag::CABLE_CNCT_BOT;
                    break;
                case dir16::left: 
                    delCoord = { -1,0,0 };
                    hostFlag = itemFlag::CABLE_CNCT_LEFT;
                    guestFlag = itemFlag::CABLE_CNCT_RIGHT;
                    break;
                case dir16::down: 
                    delCoord = { 0,+1,0 }; 
                    hostFlag = itemFlag::CABLE_CNCT_BOT;
                    guestFlag = itemFlag::CABLE_CNCT_TOP;
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

		frontierQueue.push({ cursorX, cursorY, cursorZ });
        while (!frontierQueue.empty())
        {
            Point3 current = frontierQueue.front();
            frontierQueue.pop();

            if (visitedSet.find(current) != visitedSet.end()) continue;
            visitedSet.insert(current);

            std::wprintf(L"[소자 탐색] 현재 커서는 %d,%d에 있습니다.\n", current.x, current.y);

            Prop* currentProp = TileProp(current.x, current.y, current.z);

            if (currentProp && (currentProp->leadItem.checkFlag(itemFlag::CIRCUIT) || currentProp->leadItem.checkFlag(itemFlag::CABLE)))
            {
                currentProp->runUsed = true;
                currentProp->voltageDir.clear();
                if (currentProp->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
                {
                    circuitMaxEnergy += currentProp->leadItem.electricMaxPower;
                    voltageSourceQueue.push(current);
                }

                if (isConnected(current, dir16::right)) frontierQueue.push({ current.x + 1, current.y, current.z });
                if (isConnected(current, dir16::up)) frontierQueue.push({ current.x, current.y - 1, current.z });
                if (isConnected(current, dir16::left)) frontierQueue.push({ current.x - 1, current.y, current.z });
                if (isConnected(current, dir16::down)) frontierQueue.push({ current.x, current.y + 1, current.z });
                if (isConnected(current, dir16::ascend)) frontierQueue.push({ current.x, current.y, current.z + 1 });
                if (isConnected(current, dir16::descend)) frontierQueue.push({ current.x, current.y, current.z - 1 });
            }
        }

        std::wprintf(L"회로 탐색을 종료했습니다.\n");
        if (visitedSet.size() > 0)std::wprintf(L"회로의 사이즈는 %d입니다.\n", visitedSet.size());
        if (circuitMaxEnergy > 0) std::wprintf(L"회로의 최대 에너지 수송량은 %d kJ입니다.\n", circuitMaxEnergy);
        
        //전압 방향 설정
        dir16 currentVoltageDir = dir16::none;

        //첫번째 전압원이 전체적인 방향 결정
        if (voltageSourceQueue.empty() == false)
        {
            Point3 firstSrc = voltageSourceQueue.front();
            voltageSourceQueue.pop();
            Prop* firstSrcProp = TileProp(firstSrc.x, firstSrc.y, getGridZ());

            if (firstSrcProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT)) currentVoltageDir = dir16::right;
            else if (firstSrcProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_UP)) currentVoltageDir = dir16::up;
            else if (firstSrcProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT)) currentVoltageDir = dir16::left;
            else if (firstSrcProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_DOWN)) currentVoltageDir = dir16::down;

            frontierQueue = std::queue<Point3>();
            visitedSet = std::unordered_set<Point3, Point3::Hash>();

            frontierQueue.push(firstSrc);
            firstSrcProp->voltageDir.push_back(currentVoltageDir);

            while (frontierQueue.empty() == false)
            {
                Point3 current = frontierQueue.front();
                frontierQueue.pop();

                if (visitedSet.find(current) != visitedSet.end()) continue;
                visitedSet.insert(current);

                std::wprintf(L"[전압 방향 설정] 현재 커서는 %d,%d에 있습니다.\n", current.x, current.y);

                Prop* currentProp = TileProp(current.x, current.y, current.z);

                // 들어온 방향만 저장 (이미 방문한 이웃에서 온 방향)
                if (isConnected(current, dir16::right) &&
                    visitedSet.find({ current.x + 1, current.y, current.z }) != visitedSet.end())
                {
                    currentProp->voltageDir.push_back(dir16::left);
                }
                else if (isConnected(current, dir16::up) &&
                    visitedSet.find({ current.x, current.y - 1, current.z }) != visitedSet.end())
                {
                    currentProp->voltageDir.push_back(dir16::down);
                }
                else if (isConnected(current, dir16::left) &&
                    visitedSet.find({ current.x - 1, current.y, current.z }) != visitedSet.end())
                {
                    currentProp->voltageDir.push_back(dir16::right);
                }
                else if (isConnected(current, dir16::down) &&
                    visitedSet.find({ current.x, current.y + 1, current.z }) != visitedSet.end())
                {
                    currentProp->voltageDir.push_back(dir16::up);
                }
                else if (isConnected(current, dir16::ascend) &&
                    visitedSet.find({ current.x, current.y, current.z + 1 }) != visitedSet.end())
                {
                    currentProp->voltageDir.push_back(dir16::descend);
                }
                else if (isConnected(current, dir16::descend) &&
                    visitedSet.find({ current.x, current.y, current.z - 1 }) != visitedSet.end())
                {
                    currentProp->voltageDir.push_back(dir16::ascend);
                }

                // 다음 노드들을 큐에 추가
                if (isConnected(current, dir16::right)) frontierQueue.push({ current.x + 1, current.y, current.z });
                if (isConnected(current, dir16::up)) frontierQueue.push({ current.x, current.y - 1, current.z });
                if (isConnected(current, dir16::left)) frontierQueue.push({ current.x - 1, current.y, current.z });
                if (isConnected(current, dir16::down)) frontierQueue.push({ current.x, current.y + 1, current.z });
                if (isConnected(current, dir16::ascend)) frontierQueue.push({ current.x, current.y, current.z + 1 });
                if (isConnected(current, dir16::descend)) frontierQueue.push({ current.x, current.y, current.z - 1 });
            }
        }

        //두번째 전압원부터는 분기점까지만 업데이트
        while (voltageSourceQueue.empty() == false)
        {
        }


        //if (TileProp(getGridX() + 1, getGridY(), getGridZ()) != nullptr)
        //{
        //    if (leadItem.checkFlag(itemFlag::CABLE_CNCT_RIGHT) || leadItem.checkFlag(itemFlag::CABLE))
        //    {
        //        Prop* tgtProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
        //        if (tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_LEFT) || tgtProp->leadItem.checkFlag(itemFlag::CABLE))
        //        {

        //        }
        //    }
        //}

    }

    runUsed = true;
}