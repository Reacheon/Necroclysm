#include <SDL.h>

export module Vehicle;

import std;
import globalVar;
import textureVar;
import constVar;
import util;
import World;
import ItemPocket;
import ItemData;
import Ani;
import Entity;
import Player;
import AI;
import Light;
import Coord;
import log;
import Install;
import Drawable;
import drawSprite;

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
    std::wstring name = L"FNAME";
    std::unordered_map<std::array<int, 2>, ItemPocket*, decltype(arrayHasher2)> partInfo;
    bool isVehicle = false;
    vehFlag vehType = vehFlag::none;
    dir16 bodyDir = dir16::dir2;
    dir16 wheelDir = dir16::dir2;
    Vec3 spdVec = getZeroVec();
    Vec3 accVec = getZeroVec();
    bool turnOnBrake = false;
    float timeResource = 0;
    gearFlag gearState = gearFlag::drive;
    float xAcc = 0;
    float yAcc = 0;
    float zAcc = 0; //헬기 등에서는 사용될 수 있을 것 같다
    float rotateAcc = 0;
    Vehicle* rearTrain = nullptr;

    Point2 trainWheelCenter = { 0,0 }; //열차의 회전중심, 열차바퀴들의 중심좌표로 정해짐

    //헬기용 변수
    __int8 collectiveState = 0;
    __int8 cyclicState = -1;
    __int8 rpmState = 0; //0~6

    bool isAIFirstRun = true;

    float trainSpdVal = 0.0;
    dir16 trainSpdDir = dir16::dir2;
    int trainMoveCounter = 0;

    Vehicle(int inputX, int inputY, int inputZ, int leadItemCode)
    {
        trainWheelCenter = { inputX, inputY };

        setAniPriority(3);
        prt(lowCol::orange, L"[Vehicle:constructor] 생성자가 호출되었다. 생성된 좌표는 %d,%d,%d이다.\n", inputX, inputY, inputZ);
        setGrid(inputX, inputY, inputZ);

        errorBox(World::ins()->getTile(inputX, inputY, inputZ).VehiclePtr != nullptr, L"생성위치에 이미 프롭이 존재한다!");
        World::ins()->getTile(inputX, inputY, inputZ).VehiclePtr = this;

        partInfo[{inputX, inputY}] = new ItemPocket(storageType::null);
        partInfo[{inputX, inputY}]->addItemFromDex(leadItemCode, 1);

        updateTile(inputX, inputY);

        deactivateAI();//차량을 제외하고 기본적으로 비활성화
    }

    ~Vehicle()
    {
        for (auto it = partInfo.begin(); it != partInfo.end(); it++) delete it->second;
        prt(lowCol::orange, L"[Vehicle:destructor] 소멸자가 호출되었다. \n");
    }

    void updateTile(int inputX, int inputY)
    {
        if (partInfo.find({ inputX,inputY }) != partInfo.end())
        {
            World::ins()->getTile(inputX, inputY, getGridZ()).update();
            if (partInfo[{inputX, inputY}]->itemInfo.size() > 0)
            {
                //walkable 체크
                if (World::ins()->getTile(inputX, inputY, getGridZ()).walkable == true)
                {
                    for (int i = 0; i < partInfo[{inputX, inputY}]->itemInfo.size(); i++)
                    {
                        if (!partInfo[{inputX, inputY}]->itemInfo[i].checkFlag(itemFlag::INSTALL_WALKABLE))
                        {
                            World::ins()->getTile(inputX, inputY, getGridZ()).walkable = false;
                            break;
                        }
                    }
                }

                //blocker 체크
                if (World::ins()->getTile(inputX, inputY, getGridZ()).blocker == false)
                {
                    for (int i = 0; i < partInfo[{inputX, inputY}]->itemInfo.size(); i++)
                    {
                        if (partInfo[{inputX, inputY}]->itemInfo[i].checkFlag(itemFlag::INSTALL_BLOCKER))
                        {
                            World::ins()->getTile(inputX, inputY, getGridZ()).blocker = true;
                            break;
                        }
                    }
                }
            }
        }
        else errorBox(L"[Vehicle:updateTile] updateTile이 부품이 없는 타일을 선택지로 가리켰다.");
    }
    void updateAllTiles()
    {
        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            updateTile(it->first[0], it->first[1]);
        }
    }

    bool hasFrame(int inputX, int inputY)
    {
        for (int i = 0; i < partInfo[{inputX, inputY}]->itemInfo.size(); i++)
        {
            if (partInfo[{inputX, inputY}]->itemInfo[i].checkFlag(itemFlag::VFRAME)) return true;
        }
        return false;
    }


    /////////////////////////////////////////※ 기존 프레임에 부품 추가////////////////////////////////////////////////////
    void addPart(int inputX, int inputY, ItemData inputPart) //기본 부품추가 함수, 모든 함수들이 이 함수를 기본으로 들어감, 여기에 알고리즘 넣을 것
    {
        errorBox(partInfo.find({ inputX, inputY }) == partInfo.end(), L"[Vehicle:addPart] 입력한 위치에 프레임이 존재하지 않는다.");
        errorBox(partInfo[{inputX, inputY}]->itemInfo.size() == 0, L"[Vehicle:addPart] 입력한 위치의 프레임의 itemInfo.size가 0과 같다.");
        if (inputPart.checkFlag(itemFlag::TIRE_NORMAL) || inputPart.checkFlag(itemFlag::TIRE_STEER))//타이어일 경우 맨 앞에 추가
        {
            partInfo[{inputX, inputY}]->itemInfo.insert(partInfo[{inputX, inputY}]->itemInfo.begin(), inputPart);
        }
        else//그 외 부품은 뒤에 추가
        {
            partInfo[{inputX, inputY}]->itemInfo.push_back(inputPart);
        }

        //열차바퀴 중심 설정
        if (inputPart.checkFlag(itemFlag::TRAIN_WHEEL)) updateTrainCenter();

        prt(lowCol::orange, L"[Vehicle:addPart] (%d,%d)에 새로운 부품 %ls를 추가하였다.\n", inputX, inputY, inputPart.name.c_str());
        updateSpr();
    }
    void addPart(int inputX, int inputY, int dexIndex) { addPart(inputX, inputY, itemDex[dexIndex]); }
    void addPart(int inputX, int inputY, std::vector<int> dexVec)
    {
        for (int i = 0; i < dexVec.size(); i++) addPart(inputX, inputY, dexVec[i]);
    }

    void erasePart(int inputX, int inputY, int index)
    {
        if (partInfo[{ inputX, inputY }]->itemInfo[index].checkFlag(itemFlag::TRAIN_WHEEL)) updateTrainCenter();

        partInfo[{ inputX, inputY }]->eraseItemInfo(index);
    }

    //////////////////////////////////////////////※ 프레임 확장/////////////////////////////////////////////////////////
    void extendPart(int inputX, int inputY, int inputItemCode)
    {
        //십자 방향에 부품이 있는지 체크
        for (int i = 0; i < 4; i++)
        {
            int dir = 2 * i;
            int dx, dy;
            dir2Coord(dir, dx, dy);
            //존재할 경우
            if (partInfo.find({ inputX + dx, inputY + dy }) != partInfo.end()) break;
            errorBox(i == 3, "[Vehicle:extendPart] 상하좌우에 프레임이 없는데 해당 타일로 확장을 시도했다.");
        }
        errorBox(partInfo.find({ inputX, inputY }) != partInfo.end(), "[Vehicle:extendPart] 이미 이 프롭 프레임이 있는 좌표로 확장을 시도했다.");

        partInfo[{inputX, inputY}] = new ItemPocket(storageType::null);
        partInfo[{inputX, inputY}]->addItemFromDex(inputItemCode);
        World::ins()->getTile(inputX, inputY, getGridZ()).VehiclePtr = this;

        prt(lowCol::orange, L"[Vehicle:extendPart] %p 차량이 %d,%d 위치로 %d 아이템을 확장에 성공헀다.\n", inputX, inputY, inputItemCode);
        updateSpr();
    }


    int getSprIndex(int inputX, int inputY)
    {
        errorBox(partInfo[{inputX, inputY}]->itemInfo.size() == 0, "[Vehicle:getSprIndex] The vehicle part are empty(itemInfo size is zero)");
        return partInfo[{inputX, inputY}]->itemInfo[0].sprIndex;
    }

    std::unordered_map<std::array<int, 2>, ItemPocket*, decltype(arrayHasher2)> getRotatePartInfo(dir16 inputDir16)
    {
        if (bodyDir != inputDir16)
        {
            std::unordered_map<std::array<int, 2>, ItemPocket*, decltype(arrayHasher2)> newPartInfo;
            auto currentCoordTransform = coordTransform[bodyDir];
            auto targetCoordTransform = coordTransform[inputDir16];
            for (int x = getGridX() - maxVehicleSize / 2; x <= getGridX() + maxVehicleSize / 2; x++)
            {
                for (int y = getGridY() - maxVehicleSize / 2; y <= getGridY() + maxVehicleSize / 2; y++)
                {
                    if (partInfo.find({ x,y }) != partInfo.end())
                    {
                        std::array<int, 2> originCoord = currentCoordTransform[{x - getGridX(), y - getGridY()}];
                        std::array<int, 2> dstCoord;
                        for (auto it = targetCoordTransform.begin(); it != targetCoordTransform.end(); it++)
                        {
                            if (it->second == originCoord)
                            {
                                dstCoord = it->first;
                                break;
                            }
                        }
                        newPartInfo[{dstCoord[0] + getGridX(), dstCoord[1] + getGridY()}] = partInfo[{x, y}];

                    }
                }
            }
            return newPartInfo;
        }
        else return partInfo;
    }

    void rotateEntityPtr(dir16 inputDir16)
    {
        if (bodyDir != inputDir16)
        {
            std::map<std::array<int, 2>, Entity*> entityWormhole; //엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너
            for (auto it = partInfo.begin(); it != partInfo.end(); it++)
            {
                if (World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr != nullptr)
                {
                    entityWormhole[{it->first[0], it->first[1]}] = (Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
                    World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr = nullptr;
                }
            }

            auto currentCoordTransform = coordTransform[bodyDir];
            auto targetCoordTransform = coordTransform[inputDir16];
            for (int x = getGridX() - maxVehicleSize / 2; x <= getGridX() + maxVehicleSize / 2; x++)
            {
                for (int y = getGridY() - maxVehicleSize / 2; y <= getGridY() + maxVehicleSize / 2; y++)
                {
                    if (partInfo.find({ x,y }) != partInfo.end())
                    {
                        std::array<int, 2> originCoord = currentCoordTransform[{x - getGridX(), y - getGridY()}];
                        std::array<int, 2> dstCoord;
                        for (auto it = targetCoordTransform.begin(); it != targetCoordTransform.end(); it++)
                        {
                            if (it->second == originCoord)
                            {
                                dstCoord = it->first;
                                break;
                            }
                        }

                        if (entityWormhole.find({ x, y }) != entityWormhole.end())
                        {
                            entityWormhole[{x, y}]->setGrid(dstCoord[0] + getGridX(), dstCoord[1] + getGridY(), getGridZ());
                            World::ins()->getTile(dstCoord[0] + getGridX(), dstCoord[1] + getGridY(), getGridZ()).EntityPtr = entityWormhole[{x, y}];
                        }
                    }
                }
            }
        }
    }

    void rotate(dir16 inputDir16)
    {
        if (bodyDir != inputDir16)
        {
            for (auto it = partInfo.begin(); it != partInfo.end(); it++)
            {
                World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
            }

            rotateEntityPtr(inputDir16);
            partInfo = getRotatePartInfo(inputDir16);

            for (auto it = partInfo.begin(); it != partInfo.end(); it++)
            {
                World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = this;
            }

            //회전하는 방향에 대해 바퀴 방향 재설정
            if (ACW2(bodyDir) == wheelDir) wheelDir = ACW2(inputDir16);
            else if (ACW(bodyDir) == wheelDir) wheelDir = ACW(inputDir16);
            else if (CW(bodyDir) == wheelDir) wheelDir = CW(inputDir16);
            else if (CW2(bodyDir) == wheelDir) wheelDir = CW2(inputDir16);
            else wheelDir = inputDir16;

            bodyDir = inputDir16;
        }
        else errorBox(L"[Vehicle:roate] 이미 차량이 해당 방향을 향하고 있다.");
        updateSpr();
    }

    void updateSpr()
    {
        dir16 refDir;
        switch (bodyDir)
        {
        case dir16::dir1:
        case dir16::dir1_5:
        case dir16::dir2:
        case dir16::dir2_5:
        case dir16::dir3:
            refDir = dir16::dir2;
            break;
        case dir16::dir3_5:
        case dir16::dir4:
        case dir16::dir4_5:
            refDir = dir16::dir4;
            break;
        case dir16::dir5:
        case dir16::dir5_5:
        case dir16::dir6:
        case dir16::dir6_5:
        case dir16::dir7:
            refDir = dir16::dir6;
            break;
        case dir16::dir7_5:
        case dir16::dir0:
        case dir16::dir0_5:
            refDir = dir16::dir0;
            break;
        }

        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            int tgtX = it->first[0];
            int tgtY = it->first[1];
            int tgtRelX = tgtX - getGridX();
            int tgtRelY = tgtY - getGridY();
            ItemPocket* tgtPocket = it->second;
            for (int layer = 0; layer < tgtPocket->itemInfo.size(); layer++)
            {
                if (tgtPocket->itemInfo[layer].checkFlag(itemFlag::PROP_WALL_CONNECT))
                {
                    auto checkWallGroup = [=](int dx, int dy)->bool
                        {
                            int currentGroup = tgtPocket->itemInfo[layer].tileConnectGroup;
                            std::array<int, 2> value1 = coordTransform[bodyDir][{tgtRelX, tgtRelY}];
                            std::array<int, 2> key1;
                            for (auto it = coordTransform[refDir].begin(); it != coordTransform[refDir].end(); it++)
                            {
                                if (it->second == value1)
                                {
                                    key1 = it->first;
                                }
                            }
                            std::array<int, 2> value2 = coordTransform[refDir][{key1[0] + dx, key1[1] + dy}];
                            std::array<int, 2> key2;
                            for (auto it = coordTransform[bodyDir].begin(); it != coordTransform[bodyDir].end(); it++)
                            {
                                if (it->second == value2)
                                {
                                    key2 = it->first;
                                }
                            }
                            if (partInfo.find({ getGridX() + key2[0], getGridY() + key2[1] }) != partInfo.end())
                            {
                                auto tgtItemInfo = partInfo[{getGridX() + key2[0], getGridY() + key2[1]}]->itemInfo;
                                for (int i = 0; i < tgtItemInfo.size(); i++)
                                {
                                    if (/*tgtItemInfo[i].checkFlag(itemFlag::PROP_WALL_CONNECT) && */tgtItemInfo[i].tileConnectGroup == currentGroup)
                                    {
                                        return true;
                                    }
                                }
                                return false;
                            }
                        };
                    bool topTile = checkWallGroup(0, -1);
                    bool botTile = checkWallGroup(0, 1);
                    bool leftTile = checkWallGroup(-1, 0);
                    bool rightTile = checkWallGroup(1, 0);
                    int extraIndex = connectGroupExtraIndex(topTile, botTile, leftTile, rightTile);
                    tgtPocket->itemInfo[layer].extraSprIndexSingle = extraIndex;
                }
                else if (tgtPocket->itemInfo[layer].checkFlag(itemFlag::PROP_DIR_DEPEND))
                {
                    tgtPocket->itemInfo[layer].extraSprIndexSingle = dir16toInt16(bodyDir);
                }
                else if (tgtPocket->itemInfo[layer].checkFlag(itemFlag::TIRE_STEER))
                {
                    tgtPocket->itemInfo[layer].extraSprIndexSingle = dir16toInt16(wheelDir);
                }
            }
        }
    }

    void shift(int dx, int dy)
    {
        std::map<std::array<int, 2>, Entity*> entityWormhole;//엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너

        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
            if (World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr != nullptr)
            {
                entityWormhole[{it->first[0], it->first[1]}] = (Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
                World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr = nullptr;
            }
        }

        //엔티티 옮기기
        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr = this;
            if (entityWormhole.find({ it->first[0], it->first[1] }) != entityWormhole.end())
            {
                Entity* tgtEntity = entityWormhole[{it->first[0], it->first[1]}];
                tgtEntity->setGrid(it->first[0] + dx, it->first[1] + dy, getGridZ());//위치 그리드 변경
                World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).EntityPtr = tgtEntity;//포인터 변경


            }
        }

        std::unordered_map<std::array<int, 2>, ItemPocket*, decltype(arrayHasher2)> shiftPartInfo;
        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            shiftPartInfo[{it->first[0] + dx, it->first[1] + dy}] = partInfo[{it->first[0], it->first[1]}];
        }
        partInfo = shiftPartInfo;

        addGridX(dx);
        addGridY(dy);
    }

    void zShift(int dz)
    {
        std::map<std::array<int, 2>, Entity*> entityWormhole;//엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너

        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
            if (World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr != nullptr)
            {
                entityWormhole[{it->first[0], it->first[1]}] = (Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
                World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr = nullptr;
            }
        }

        //엔티티 옮기기
        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            World::ins()->getTile(it->first[0], it->first[1], getGridZ() + dz).VehiclePtr = this;
            if (entityWormhole.find({ it->first[0], it->first[1] }) != entityWormhole.end())
            {
                Entity* tgtEntity = entityWormhole[{it->first[0], it->first[1]}];
                tgtEntity->setGrid(it->first[0], it->first[1], getGridZ() + dz);//위치 그리드 변경
                World::ins()->getTile(it->first[0], it->first[1], getGridZ() + dz).EntityPtr = tgtEntity;//포인터 변경
            }
        }
        addGridZ(dz);
    };

    bool colisionCheck(dir16 inputDir16, int dx, int dy)
    {
        auto rotatePartInfo = getRotatePartInfo(inputDir16);
        for (auto it = rotatePartInfo.begin(); it != rotatePartInfo.end(); it++)
        {
            //벽 충돌 체크
            if (World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).wall != 0) return true;

            //프롭 충돌 체크
            Vehicle* targetPtr = (Vehicle*)World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr;
            if (targetPtr != nullptr && targetPtr != this) return true;
        }
        return false;
    }

    bool colisionCheck(int dx, int dy)//해당 dx,dy만큼 이동했을 때 prop이 벽 또는 기존의 Vehicle과 충돌하는지
    {
        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            //벽 충돌 체크
            if (World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).wall != 0) return true;

            //프롭 충돌 체크
            Vehicle* targetPtr = (Vehicle*)World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr;
            if (targetPtr != nullptr && targetPtr != this)
            {
                prt(L"(%d,%d)만큼 이동했을 때 포인터 %p와 충돌했다.\n", dx, dy, targetPtr);
                return true;
            }
        }
        return false;
    }

    //bool colisionCheck(dir16 inputDir16)
    //{
    //    return colisionCheck(inputDir16, 0, 0);
    //}

    void rush(int dx, int dy)
    {
        if (dx == 0 && dy == 0) return;
        //iAmDictator();
        aniUSet.insert(this);
        setAniType(aniFlag::propRush);
        setDelGrid(dx, dy);
        shift(getDelGridX(), getDelGridY());
        turnCycle = turn::monsterAnime;
    }

    void centerShift(int dx, int dy, int dz)
    {
    }

    bool runAI()
    {
        if (isAIFirstRun)
        {
            isAIFirstRun = false;
        }

        //prt(lowCol::orange,L"[Vehicle:AI] ID : %p의 AI를 실행시켰다.\n", this);
        while (1)
        {

            //prt(lowCol::orange, L"[Vehicle:AI] ID : %p의 timeResource는 %f입니다.\n", this, getTimeResource());
            if (getTimeResource() >= 2.0)
            {
                clearTimeResource();
                addTimeResource(2.0);
            }

            if (isVehicle)
            {
                if (trainMoveCounter > 0)
                {
                    int dx = 0;
                    int dy = 0;
                    itemFlag straightFlag, leftFlag, rightFlag;
                    if (trainSpdDir == dir16::dir0)
                    {
                        straightFlag = itemFlag::RAIL_CNCT_LEFT;
                        leftFlag = itemFlag::RAIL_CNCT_TOP;
                        rightFlag = itemFlag::RAIL_CNCT_BOT;
                    }
                    else if (trainSpdDir == dir16::dir2)
                    {
                        straightFlag = itemFlag::RAIL_CNCT_BOT;
                        leftFlag = itemFlag::RAIL_CNCT_LEFT;
                        rightFlag = itemFlag::RAIL_CNCT_RIGHT;
                    }
                    else if (trainSpdDir == dir16::dir4)
                    {
                        straightFlag = itemFlag::RAIL_CNCT_RIGHT;
                        leftFlag = itemFlag::RAIL_CNCT_BOT;
                        rightFlag = itemFlag::RAIL_CNCT_TOP;
                    }
                    else if (trainSpdDir == dir16::dir6)
                    {
                        straightFlag = itemFlag::RAIL_CNCT_TOP;
                        leftFlag = itemFlag::RAIL_CNCT_RIGHT;
                        rightFlag = itemFlag::RAIL_CNCT_LEFT;
                    }

                    if (trainSpdDir != bodyDir)
                    {
                        if (colisionCheck(trainSpdDir, 0, 0) == false)
                        {
                            rotate(trainSpdDir);
                        }
                        else
                        {
                            trainMoveCounter = 0;
                            break;
                        }
                    }


                    while (trainMoveCounter > 0)
                    {


                        int prevX = dx;
                        int prevY = dy;

                        if (trainSpdDir == dir16::dir0) dx++;
                        else if (trainSpdDir == dir16::dir2) dy--;
                        else if (trainSpdDir == dir16::dir4) dx--;
                        else if (trainSpdDir == dir16::dir6) dy++;

                        if (colisionCheck(dx, dy) == true)
                        {
                            rush(prevX, prevY);
                            trainMoveCounter = 0;
                            return false;
                        }

                        Install* targetInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
                        if (targetInstall != nullptr)
                        {

                            if (targetInstall->leadItem.checkFlag(straightFlag))
                            {
                                trainMoveCounter--;

                                if (trainMoveCounter == 0)
                                {
                                    rush(dx, dy);
                                    if (rearTrain != nullptr)
                                    {
                                        rearTrain->trainMoveCounter = 99;
                                        rearTrain->runAI();
                                        rearTrain->runAI();
                                    }
                                    return false;
                                }
                                else
                                {
                                    if (targetInstall->leadItem.checkFlag(itemFlag::RAIL_BUFFER))
                                    {
                                        updateLog(L"레일이 버퍼를 만나 강제로 정지했다.\n");
                                        trainMoveCounter = 0;
                                        rush(dx, dy);

                                        return false;
                                    }
                                    else if (targetInstall->leadItem.checkFlag(leftFlag))
                                    {
                                        trainSpdDir = ACW2(ACW2(trainSpdDir));
                                        rush(dx, dy);
                                        updateSpr();
                                        return false;
                                    }
                                    else if (targetInstall->leadItem.checkFlag(rightFlag))
                                    {
                                        trainSpdDir = CW2(CW2(trainSpdDir));
                                        rush(dx, dy);
                                        updateSpr();
                                        return false;
                                    }
                                    else if (targetInstall->leadItem.checkFlag(straightFlag))
                                    {
                                    }
                                    else errorBox(L"존재할 수 없는 레일이 발견되었다.\n");
                                }
                            }
                            else //이어진 레일이 아닐 경우(탈선)
                            {
                                prt(L"[열차] (%d,%d)의 이동할 레일이 이 차량과 연결되지 않는다. (탈선) \n", getGridX() + dx, getGridY() + dy);
                                trainMoveCounter = 0;
                            }
                        }
                        else //이동할 대상 타일에 레일설치물이 없을 경우(탈선)
                        {
                            prt(L"[열차] 이동할 대상 위치 (%d,%d)에 어떤 설치물도 존재하지 않는다. (탈선) \n", getGridX() + dx, getGridY() + dy);
                            trainMoveCounter = 0;
                        }
                    }
                }



                if (getTimeResource() >= 1.0)
                {
                    useTimeResource(1.0);

                    if (vehType == vehFlag::car || vehType == vehFlag::heli)
                    {
                        if (spdVec.compZ != 0)
                        {
                            zShift(spdVec.compZ);
                        }

                        if (bodyDir != wheelDir)
                        {
                            Vec3 wheelDirSpd = getDefaultVec(dir16toInt16(wheelDir));
                            int sgn = 1;
                            if (gearState == gearFlag::reverse) sgn = -1;


                            if (wheelDir == ACW2(bodyDir))
                            {
                                spdVec = scalarMultiple(wheelDirSpd, sgn * spdVec.getLength());
                                //spdVec = scalarMultiple(wheelDirSpd, 1.0 * spdVec.getLength() / 1.414);
                            }
                            else if (wheelDir == ACW(bodyDir))
                            {
                                spdVec = scalarMultiple(wheelDirSpd, sgn * spdVec.getLength());
                                //spdVec = scalarMultiple(wheelDirSpd, 0.87330464009 * spdVec.getLength() / 2.0);
                            }
                            else if (wheelDir == CW(bodyDir))
                            {
                                spdVec = scalarMultiple(wheelDirSpd, sgn * spdVec.getLength());
                                //spdVec = scalarMultiple(wheelDirSpd, 0.87330464009 * spdVec.getLength() / 2.0);
                            }
                            else if (wheelDir == CW2(bodyDir))
                            {
                                spdVec = scalarMultiple(wheelDirSpd, sgn * spdVec.getLength());
                                //spdVec = scalarMultiple(wheelDirSpd, 1.0 * spdVec.getLength() / 1.414);
                            }
                        }

                        //가속도에 의한 속도 가감
                        spdVec.addVec(accVec);
                        accVec = getZeroVec();
                        //마찰에 의한 손실
                        if (spdVec.getLength() != 0)
                        {
                            int beforeXSpdSgn = sgn(spdVec.compX);
                            int beforeYSpdSgn = sgn(spdVec.compY);
                            float massCoeff = 1.5;
                            float frictionCoeff = 1.0;
                            float delSpd = frictionCoeff / massCoeff;
                            Vec3 brakeDirVec = scalarMultiple(spdVec, -1.0);
                            Vec3 brakeDirNormVec = brakeDirVec.getNormDirVec();
                            Vec3 brakeVec = scalarMultiple(brakeDirNormVec, delSpd);

                            if (spdVec.getLength() < brakeVec.getLength())
                            {
                                spdVec = getZeroVec();
                                continue;
                            }
                            else spdVec.addVec(brakeVec);
                        }
                        xAcc += spdVec.compX;
                        yAcc += spdVec.compY;

                        int dstX = std::floor(xAcc);
                        int dstY = std::floor(yAcc);
                        // (0,0)에서 (dstX,dstY)까지 직선을 그어야 함
                        std::vector<std::array<int, 2>> path;
                        makeLine(path, dstX, dstY);

                        int dxFinal = 0;
                        int dyFinal = 0;
                        for (int i = 1; i < path.size(); i++)
                        {
                            if (colisionCheck(path[i][0], path[i][1]))//충돌할 경우
                            {
                                dxFinal = path[i - 1][0];
                                dyFinal = path[i - 1][1];
                                break;
                            }

                            if (i == path.size() - 1)
                            {
                                dxFinal = dstX;
                                dyFinal = dstY;
                            }
                        }

                        if (dxFinal != 0 || dyFinal != 0)
                        {
                            rush(dxFinal, dyFinal);
                            if (bodyDir != wheelDir)
                            {
                                rotateAcc += std::abs(dxFinal) + std::abs(dyFinal);
                                if (wheelDir == ACW2(bodyDir) && rotateAcc > 7)  rotate(ACW2(bodyDir));
                                else if (wheelDir == ACW(bodyDir) && rotateAcc > 11)  rotate(ACW(bodyDir));
                                else if (wheelDir == CW(bodyDir) && rotateAcc > 11) rotate(CW(bodyDir));
                                else if (wheelDir == CW2(bodyDir) && rotateAcc > 7) rotate(CW2(bodyDir));
                            }
                            xAcc -= std::floor(xAcc);
                            yAcc -= std::floor(yAcc);
                        }
                        updateSpr();
                        return false;
                    }
                    else if (vehType == vehFlag::train)
                    {
                        if (rpmState >= 1)//열차 rpm state에 따른 가속도 추가
                        {
                            trainSpdVal = 10;
                            int vx = getGridX();
                            int vy = getGridY();
                            int vz = getGridZ();
                            Install* tgtInstall = (Install*)World::ins()->getTile(vx, vy, vz).InstallPtr;
                            if (tgtInstall != nullptr)
                            {
                                if (gearState == gearFlag::drive)//주행기어
                                {
                                    if (bodyDir == dir16::dir0)//동쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) trainSpdDir = dir16::dir0;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) trainSpdDir = dir16::dir2;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) trainSpdDir = dir16::dir6;
                                    }
                                    else if (bodyDir == dir16::dir2)//북쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) trainSpdDir = dir16::dir2;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) trainSpdDir = dir16::dir4;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) trainSpdDir = dir16::dir0;
                                    }
                                    else if (bodyDir == dir16::dir4)//서쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) trainSpdDir = dir16::dir4;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) trainSpdDir = dir16::dir2;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) trainSpdDir = dir16::dir6;
                                    }
                                    else if (bodyDir == dir16::dir6)//남쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) trainSpdDir = dir16::dir6;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) trainSpdDir = dir16::dir4;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) trainSpdDir = dir16::dir0;
                                    }
                                }
                                else if (gearState == gearFlag::reverse)//후진기어
                                {
                                    if (bodyDir == dir16::dir0)//동쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) trainSpdDir = dir16::dir4;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) trainSpdDir = dir16::dir2;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) trainSpdDir = dir16::dir6;
                                    }
                                    else if (bodyDir == dir16::dir2)//북쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) trainSpdDir = dir16::dir6;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) trainSpdDir = dir16::dir4;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) trainSpdDir = dir16::dir0;
                                    }
                                    else if (bodyDir == dir16::dir4)//서쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) trainSpdDir = dir16::dir0;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) trainSpdDir = dir16::dir2;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) trainSpdDir = dir16::dir6;
                                    }
                                    else if (bodyDir == dir16::dir6)//남쪽 방향 열차
                                    {
                                        if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) trainSpdDir = dir16::dir2;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) trainSpdDir = dir16::dir4;
                                        else if (tgtInstall->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) trainSpdDir = dir16::dir0;
                                    }
                                }
                            }
                        }

                        //마찰에 의한 손실
                        if (trainSpdVal != 0)
                        {
                            float massCoeff = 1.5;
                            float frictionCoeff = 1.0;
                            float delSpd = frictionCoeff / massCoeff;

                            if (delSpd > trainSpdVal) trainSpdVal = 0;
                            else trainSpdVal - delSpd;
                        }

                        trainMoveCounter = trainSpdVal;
                        return false;
                    }
                }
            }

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //위의 모든 패턴 조건을 만족하지않을시 return true
            //prt(lowCol::orange, L"[Vehicle:AI] AI가 true를 반환했다. AI를 종료합니다.\n");
            isAIFirstRun = true;
            return true;
        }
    }

    bool runAnimation(bool shutdown)
    {
        if (getAniType() == aniFlag::propRush)
        {
            addTimer();

            {
                
                static float totalDist = 0;
                static float totalMove = 0;
                static std::vector<std::array<int, 2>> line;
                static int lineCheck = 0;
                if (getTimer() == 1)
                {
                    line.clear();
                    makeLine(line, getDelGridX(), getDelGridY());
                    extraRenderVehList.push_back(this);
                    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
                    {
                        void* iPtr = World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
                        if (iPtr != nullptr) extraRenderEntityList.push_back(iPtr);
                    }

                    cameraFix = false;

                    totalDist = std::sqrt(std::pow(getDelX(), 2) + std::pow(getDelY(), 2));
                    totalMove = 0;
                    lineCheck = 0;
                    setFakeX(-getDelX());
                    setFakeY(-getDelY());
                }


                float spd = 3.0;
                float xSpd, ySpd;
                int relX = getDelX();
                int relY = getDelY();
                float dist = std::sqrt(std::pow(relX, 2) + std::pow(relY, 2));
                float cosVal = relX / dist;
                float sinVal = relY / dist;

                xSpd = spd * cosVal;
                ySpd = spd * sinVal;

                if (World::ins()->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).VehiclePtr == this)
                {
                    totalMove += std::sqrt(std::pow(xSpd, 2) + std::pow(ySpd, 2));
                    int arrIndex = floor(myMin(1.0, (totalMove / totalDist)) * (float)(line.size() - 1));
                    if (arrIndex >= lineCheck)
                    {
                        std::array<int, 2> visionTarget = line[arrIndex];
                        //prt(L"시야 업데이트의 중심은 (%d,%d)이고 인덱스는 %d이며 사이즈는 %d이다.\n", Player::ins()->getGridX() + visionTarget[0], Player::ins()->getGridY() + visionTarget[1], arrIndex, line.size());

                        Player::ins()->updateVision(Player::ins()->getEyeSight(), Player::ins()->getGridX() - getDelGridX() + visionTarget[0], Player::ins()->getGridY() - getDelGridY() + visionTarget[1]);
                        lineCheck++;
                    }
                }

                setFloatFakeX(getFloatFakeX() + xSpd);
                setFloatFakeY(getFloatFakeY() + ySpd);
                //prt(L"x방향의 속도를 %f, y방향의 속도를 %f만큼 더했다.현재의 fake 좌표는 (%f,%f)이다.\n", xSpd, ySpd, getFloatFakeX(), getFloatFakeY());

                if (xSpd > 0 && getFloatFakeX() > 0) { setFloatFakeX(0); }
                if (xSpd < 0 && getFloatFakeX() < 0) { setFloatFakeX(0); }
                if (ySpd > 0 && getFloatFakeY() > 0) { setFloatFakeY(0); }
                if (ySpd < 0 && getFloatFakeY() < 0) { setFloatFakeY(0); }


                for (auto it = partInfo.begin(); it != partInfo.end(); it++)
                {
                    if (World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr != nullptr)
                    {
                        ((Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr)->setFloatFakeX(getFloatFakeX());
                        ((Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr)->setFloatFakeY(getFloatFakeY());
                    }
                }

                cameraX = Player::ins()->getX() + Player::ins()->getFakeX();
                cameraY = Player::ins()->getY() + Player::ins()->getFakeY();

                if (getFloatFakeX() == 0 && getFloatFakeY() == 0)//도착
                {
                    //prt(L"도착했다! 현재의 fake 좌표는 (%f,%f)이다.\n", getFloatFakeX(), getFloatFakeY());
                    
                    extraRenderSet.clear();
                    setDelGrid(0, 0);
                    setFakeX(0);
                    setFakeY(0);
                    for (auto it = partInfo.begin(); it != partInfo.end(); it++)//엔티티 페이크 설정
                    {
                        if (World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr != nullptr)
                        {
                            Entity* tgtEntity = (Entity*)(World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr);
                            tgtEntity->setFakeX(0);
                            tgtEntity->setFakeY(0);
                            tgtEntity->setDelGrid(0, 0);
                        }
                    }

                    cameraFix = true;
                    Player::ins()->updateVision(Player::ins()->getEyeSight());
                    resetTimer();
                    setAniType(aniFlag::null);
                    extraRenderVehList.erase(std::find(extraRenderVehList.begin(), extraRenderVehList.end(), (void*)this));
                    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
                    {
                        void* iPtr = World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
                        if(iPtr!=nullptr) extraRenderEntityList.erase(std::find(extraRenderEntityList.begin(), extraRenderEntityList.end(), iPtr));
                    }
                    return true;
                }
            }
        }
        return false;
    }

    void updateTrainCenter()
    {
        std::vector<Point2> trainWheelList;

        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            ItemPocket* pocketPtr = it->second;
            for (int i = 0; i < pocketPtr->itemInfo.size(); i++)
            {
                if (pocketPtr->itemInfo[i].checkFlag(itemFlag::TRAIN_WHEEL))
                {
                    if (std::find(trainWheelList.begin(), trainWheelList.end(), it->first) == trainWheelList.end()) //열차 바퀴 좌표가 중복된 값이 없으면
                    {
                        trainWheelList.push_back({ it->first[0],it->first[1] });
                    }
                }
            }
        }
        //trainWheelList의 중간좌표를 구함
        trainWheelCenter = calcMidpoint(trainWheelList);
    }

    void drawSelf() override
    {
        int tileSize = 16 * zoomScale;
        auto drawVehicleComponent = [=](Vehicle* vPtr, int tgtX, int tgtY, int layer, int alpha)
            {
                SDL_Rect dst;
                dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
                dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
                dst.w = tileSize;
                dst.h = tileSize;

                setZoom(zoomScale);
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), alpha); //텍스쳐 투명도 설정
                SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
                int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;
                drawSpriteCenter
                (
                    spr::propset,
                    sprIndex,
                    dst.x + dst.w / 2 + zoomScale * vPtr->getFakeX(),
                    dst.y + dst.h / 2 + zoomScale * vPtr->getFakeY()
                );
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
                setZoom(1.0);
            };


        for (auto it = this->partInfo.begin(); it != this->partInfo.end(); it++)
        {
            ////////////////////////////////일반 차량부품/////////////////////////////////////////////////
            for (int layer = 0; layer < (it->second)->itemInfo.size(); layer++)
            {
                //바닥프롭,천장프롭 플래그가 없는 일반 프롭일 경우
                if (!(it->second)->itemInfo[layer].checkFlag(itemFlag::VEH_ROOF))
                {
                    drawVehicleComponent(this, (it->first)[0], (it->first)[1], layer, 255);
                }
            }

            ////////////////////////////////천장 차량부품////////////////////////////////////////////////////
            int propCeilAlpha = 255;
            if (World::ins()->getTile(Player::ins()->getGridX(), Player::ins()->getGridY(), Player::ins()->getGridZ()).VehiclePtr == this) propCeilAlpha = 50;

            for (int layer = 0; layer < (it->second)->itemInfo.size(); layer++)
            {
                if ((it->second)->itemInfo[layer].checkFlag(itemFlag::VEH_ROOF))
                {
                    if ((it->second)->itemInfo[layer].itemCode == 314)
                    {
                        //rotorList.push_back({ it->first[0],it->first[1] });
                    }
                    else drawVehicleComponent(this, it->first[0], it->first[1], layer, propCeilAlpha);
                }
            }
        }
    };
};

