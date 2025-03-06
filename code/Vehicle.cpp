#include <SDL.h>
#define AI_TURN_END return true;
#define AI_TURN_CONTINUE return false;

import Vehicle;
import std;
import globalVar;
import wrapVar;
import textureVar;
import constVar;
import util;
import World;
import ItemPocket;
import ItemData;
import Entity;
import Player;
import AI;
import Light;
import Ani;
import Coord;
import Drawable;
import log;
import Prop;
import drawSprite;

Vehicle::Vehicle(int inputX, int inputY, int inputZ, int leadItemCode)
{
    trainWheelCenter = { inputX, inputY };

    setAniPriority(3);
    prt(L"[Vehicle:constructor] 생성자가 호출되었다. 생성된 좌표는 %d,%d,%d이다.\n", inputX, inputY, inputZ);
    setGrid(inputX, inputY, inputZ);

    errorBox(World::ins()->getTile(inputX, inputY, inputZ).VehiclePtr != nullptr, L"생성위치에 이미 프롭이 존재한다!");
    World::ins()->getTile(inputX, inputY, inputZ).VehiclePtr = this;

    partInfo[{inputX, inputY}] = new ItemPocket(storageType::null);
    partInfo[{inputX, inputY}]->addItemFromDex(leadItemCode, 1);


    deactivateAI();//차량을 제외하고 기본적으로 비활성화
}

Vehicle::~Vehicle()
{
    for (auto it = partInfo.begin(); it != partInfo.end(); it++) delete it->second;
    prt(L"[Vehicle:destructor] 소멸자가 호출되었다. \n");
}



bool Vehicle::hasFrame(int inputX, int inputY)
{
    for (int i = 0; i < partInfo[{inputX, inputY}]->itemInfo.size(); i++)
    {
        if (partInfo[{inputX, inputY}]->itemInfo[i].checkFlag(itemFlag::VFRAME)) return true;
    }
    return false;
}


/////////////////////////////////////////※ 기존 프레임에 부품 추가////////////////////////////////////////////////////
void Vehicle::addPart(int inputX, int inputY, ItemData inputPart) //기본 부품추가 함수, 모든 함수들이 이 함수를 기본으로 들어감, 여기에 알고리즘 넣을 것
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

    //prt(L"[Vehicle:addPart] (%d,%d)에 새로운 부품 %ls를 추가하였다.\n", inputX, inputY, inputPart.name.c_str());
    updateSpr();
}
void Vehicle::addPart(int inputX, int inputY, int dexIndex) { addPart(inputX, inputY, itemDex[dexIndex]); }
void Vehicle::addPart(int inputX, int inputY, std::vector<int> dexVec)
{
    for (int i = 0; i < dexVec.size(); i++) addPart(inputX, inputY, dexVec[i]);
}

void Vehicle::erasePart(int inputX, int inputY, int index)
{
    if (partInfo[{ inputX, inputY }]->itemInfo[index].checkFlag(itemFlag::TRAIN_WHEEL)) updateTrainCenter();

    partInfo[{ inputX, inputY }]->eraseItemInfo(index);
}

//////////////////////////////////////////////※ 프레임 확장/////////////////////////////////////////////////////////
void Vehicle::extendPart(int inputX, int inputY, int inputItemCode)
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

    //prt(L"[Vehicle:extendPart] %p 차량이 %d,%d 위치로 %d 아이템을 확장에 성공헀다.\n", inputX, inputY, inputItemCode);
    updateSpr();
}


int Vehicle::getSprIndex(int inputX, int inputY)
{
    errorBox(partInfo[{inputX, inputY}]->itemInfo.size() == 0, "[Vehicle:getSprIndex] The vehicle part are empty(itemInfo size is zero)");
    return partInfo[{inputX, inputY}]->itemInfo[0].sprIndex;
}

std::unordered_map<std::array<int, 2>, ItemPocket*, decltype(arrayHasher2)> Vehicle::getRotatePartInfo(dir16 inputDir16)
{
    if (bodyDir != inputDir16)
    {
        std::unordered_map<std::array<int, 2>, ItemPocket*, decltype(arrayHasher2)> newPartInfo;
        auto currentCoordTransform = coordTransform[bodyDir];
        auto targetCoordTransform = coordTransform[inputDir16];
        for (int x = getGridX() - MAX_VEHICLE_SIZE / 2; x <= getGridX() + MAX_VEHICLE_SIZE / 2; x++)
        {
            for (int y = getGridY() - MAX_VEHICLE_SIZE / 2; y <= getGridY() + MAX_VEHICLE_SIZE / 2; y++)
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

void Vehicle::rotateEntityPtr(dir16 inputDir16)
{
    if (bodyDir != inputDir16)
    {
        std::map<std::array<int, 2>, Entity*> entityWormhole; //엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너
        for (auto it = partInfo.begin(); it != partInfo.end(); it++)
        {
            if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
            {
                entityWormhole[{it->first[0], it->first[1]}] = (Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
                World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr = nullptr;
            }
        }

        auto currentCoordTransform = coordTransform[bodyDir];
        auto targetCoordTransform = coordTransform[inputDir16];
        for (int x = getGridX() - MAX_VEHICLE_SIZE / 2; x <= getGridX() + MAX_VEHICLE_SIZE / 2; x++)
        {
            for (int y = getGridY() - MAX_VEHICLE_SIZE / 2; y <= getGridY() + MAX_VEHICLE_SIZE / 2; y++)
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

void Vehicle::rotate(dir16 inputDir16)
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
    //else errorBox(L"[Vehicle:roate] 이미 차량이 해당 방향을 향하고 있다.");
    updateSpr();
}

void Vehicle::updateSpr()
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
            if (tgtPocket->itemInfo[layer].checkFlag(itemFlag::VPART_WALL_CONNECT))
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
            else if (tgtPocket->itemInfo[layer].checkFlag(itemFlag::VPART_DIR_DEPEND))
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

void Vehicle::shift(int dx, int dy)
{
    std::map<std::array<int, 2>, Entity*> entityWormhole;//엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너

    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
        if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
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

void Vehicle::zShift(int dz)
{
    std::map<std::array<int, 2>, Entity*> entityWormhole;//엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너

    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
        if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
        {
            entityWormhole[{it->first[0], it->first[1]}] = TileEntity(it->first[0], it->first[1], getGridZ());
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

bool Vehicle::colisionCheck(dir16 inputDir16, int dx, int dy)
{
    auto rotatePartInfo = getRotatePartInfo(inputDir16);
    for (auto it = rotatePartInfo.begin(); it != rotatePartInfo.end(); it++)
    {
        //벽 충돌 체크
        if (TileWall(it->first[0] + dx, it->first[1] + dy, getGridZ()) != 0) return true;

        //프롭 충돌 체크
        Vehicle* targetPtr = (Vehicle*)World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr;
        if (targetPtr != nullptr && targetPtr != this) return true;
    }
    return false;
}

bool Vehicle::colisionCheck(int dx, int dy)//해당 dx,dy만큼 이동했을 때 prop이 벽 또는 기존의 Vehicle과 충돌하는지
{
    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        //벽 충돌 체크
        if (TileWall(it->first[0] + dx, it->first[1] + dy, getGridZ()) != 0) return true;

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

void Vehicle::rush(int dx, int dy)
{
    if (dx == 0 && dy == 0) return;
    //iAmDictator();
    setDelGrid(dx, dy);
    shift(getDelGridX(), getDelGridY());
    addAniUSetMonster(this, aniFlag::propRush);

    cameraFix = false;
    setFakeX(-getDelX());
    setFakeY(-getDelY());
    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
        {
            TileEntity(it->first[0], it->first[1], getGridZ())->setFakeX(getFakeX());
            TileEntity(it->first[0], it->first[1], getGridZ())->setFakeY(getFakeY());
        }
    }
}

void Vehicle::centerShift(int dx, int dy, int dz)
{
}

bool Vehicle::runAnimation(bool shutdown)
{
    //prt(L"Vehicle %p의 runAnimation이 실행되었다.\n", this);
    //move는 사실상 pulled 카트 용도로만 사용됨
    if (getAniType() == aniFlag::move)//만약 플레이어 인스턴스의 좌표와 목적좌표가 다를 경우
    {
        // 1 / 60초마다 runAnimation이 실행됨
        addTimer();
        const double spd = pullMoveSpd;
        if (getX() + getIntegerFakeX() > getDstX()) addFakeX(-spd);
        else if (getX() + getIntegerFakeX() < getDstX()) addFakeX(+spd);
        if (getY() + getIntegerFakeY() > getDstY()) addFakeY(-spd);
        else if (getY() + getIntegerFakeY() < getDstY()) addFakeY(+spd);

        if (std::abs(getIntegerFakeX()) >= 16.0 || std::abs(getIntegerFakeY()) >= 16.0)
        {
            resetTimer();
            setAniType(aniFlag::null);
            shift(getDstGridX() - getGridX(), getDstGridY() - getGridY());
            setGrid(getDstGridX(), getDstGridY(), getGridZ());
            setFakeX(0);
            setFakeY(0);
            return true;
        }
    }
    else if (getAniType() == aniFlag::propRush)
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
                    void* iPtr = TileEntity(it->first[0], it->first[1], getGridZ());
                    if (iPtr != nullptr) extraRenderEntityList.push_back(iPtr);
                }
                totalDist = std::sqrt(std::pow(getDelX(), 2) + std::pow(getDelY(), 2));
                totalMove = 0;
                lineCheck = 0;

                cameraFix = false;
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

            if (World::ins()->getTile(PlayerX(), PlayerY(), PlayerZ()).VehiclePtr == this)
            {
                totalMove += std::sqrt(std::pow(xSpd, 2) + std::pow(ySpd, 2));
                int arrIndex = floor(myMin(1.0, (totalMove / totalDist)) * (float)(line.size() - 1));
                if (arrIndex >= lineCheck)
                {
                    std::array<int, 2> visionTarget = line[arrIndex];
                    //prt(L"시야 업데이트의 중심은 (%d,%d)이고 인덱스는 %d이며 사이즈는 %d이다.\n", PlayerX() + visionTarget[0], PlayerY() + visionTarget[1], arrIndex, line.size());

                    Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight, PlayerX() - getDelGridX() + visionTarget[0], PlayerY() - getDelGridY() + visionTarget[1]);
                    lineCheck++;
                }
            }

            setFakeX(getFakeX() + xSpd);
            setFakeY(getFakeY() + ySpd);
            //prt(L"x방향의 속도를 %f, y방향의 속도를 %f만큼 더했다.현재의 fake 좌표는 (%f,%f)이다.\n", xSpd, ySpd, getFakeX(), getFakeY());

            if (xSpd > 0 && getFakeX() > 0) { setFakeX(0); }
            if (xSpd < 0 && getFakeX() < 0) { setFakeX(0); }
            if (ySpd > 0 && getFakeY() > 0) { setFakeY(0); }
            if (ySpd < 0 && getFakeY() < 0) { setFakeY(0); }

            for (auto it = partInfo.begin(); it != partInfo.end(); it++)
            {
                if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
                {
                    TileEntity(it->first[0], it->first[1], getGridZ())->setFakeX(getFakeX());
                    TileEntity(it->first[0], it->first[1], getGridZ())->setFakeY(getFakeY());
                }
            }

            cameraX = Player::ins()->getX() + Player::ins()->getIntegerFakeX();
            cameraY = Player::ins()->getY() + Player::ins()->getIntegerFakeY();

            if (getFakeX() == 0 && getFakeY() == 0)//도착
            {
                prt(L"도착했다! 현재의 fake 좌표는 (%f,%f)이다.\n", getFakeX(), getFakeY());

                extraRenderEntityList.clear();
                setDelGrid(0, 0);
                setFakeX(0);
                setFakeY(0);
                for (auto it = partInfo.begin(); it != partInfo.end(); it++)//엔티티 페이크 설정
                {
                    if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
                    {
                        Entity* tgtEntity = TileEntity(it->first[0], it->first[1], getGridZ());
                        tgtEntity->setFakeX(0);
                        tgtEntity->setFakeY(0);
                        tgtEntity->setDelGrid(0, 0);
                    }
                }

                cameraFix = true;
                Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
                resetTimer();
                setAniType(aniFlag::null);
                extraRenderVehList.erase(std::find(extraRenderVehList.begin(), extraRenderVehList.end(), (void*)this));
                for (auto it = partInfo.begin(); it != partInfo.end(); it++)
                {
                    void* iPtr = TileEntity(it->first[0], it->first[1], getGridZ());
                    if (iPtr != nullptr)
                    {
                        auto eraseIt = std::find(extraRenderEntityList.begin(), extraRenderEntityList.end(), iPtr);
                        if (eraseIt != extraRenderEntityList.end()) extraRenderEntityList.erase(eraseIt);
                    }
                }
                return true;
            }
        }
    }
    else if (getAniType() == aniFlag::minecartRush)
    {
        addTimer();

        if (singleRailMoveVec.size() > 0)
        {
            if (singleRailMoveVec[0] == dir16::dir0) setFakeX(getIntegerFakeX() + 4.0);
            else if (singleRailMoveVec[0] == dir16::dir2) setFakeY(getIntegerFakeY() - 4.0);
            else if (singleRailMoveVec[0] == dir16::dir4) setFakeX(getIntegerFakeX() - 4.0);
            else if (singleRailMoveVec[0] == dir16::dir6) setFakeY(getIntegerFakeY() + 4.0);

            for (auto it = partInfo.begin(); it != partInfo.end(); it++)
            {
                if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
                {
                    TileEntity(it->first[0], it->first[1], getGridZ())->setFakeX(getIntegerFakeX());
                    TileEntity(it->first[0], it->first[1], getGridZ())->setFakeY(getIntegerFakeY());
                }
            }

            if(getTimer()==1) bodyDir = singleRailMoveVec[0];

            // prt(L"[Vehicle : train %p ] 타이머 %d : 연산 후의 fake 좌표는 (%d,%d)이다.\n", this, getTimer(),getIntegerFakeX(), getIntegerFakeY());

            cameraX = Player::ins()->getX() + Player::ins()->getIntegerFakeX();
            cameraY = Player::ins()->getY() + Player::ins()->getIntegerFakeY();

            if (getTimer() >= 4)
            {
                Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight, PlayerX() + (Player::ins()->getIntegerFakeX() / 16), PlayerY() + (Player::ins()->getIntegerFakeY() / 16));
                //prt(L"[Vehicle : train %p ] 카운터가 4보다 커져 fake 좌표가 초기화되었다.\n", this);
                singleRailMoveVec.erase(singleRailMoveVec.begin());
                resetTimer();
            }
        }
        else
        {
            //prt(L"[Vehicle : train %p ] 이동이 전부 완료된 후의 페이크 좌표는 (%f,%f)이다.\n", this, getFakeX(), getFakeY());
            extraRenderVehList.clear();
            extraRenderEntityList.clear();
            Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
            Player::ins()->updateMinimap();
            cameraFix = true;
            resetTimer();
            return true;
        }


    }
    return false;
}

void Vehicle::updateTrainCenter()
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

void Vehicle::drawSelf()
{
    std::vector<Point2> rotorList;
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
            
            if (vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].itemCode == itemVIPCode::minecart)
            {
                if (bodyDir == dir16::dir0 || bodyDir == dir16::dir4) sprIndex += 0;
                else sprIndex += 1;
            }

            
            drawSpriteCenter
            (
                spr::propset,
                sprIndex,
                dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
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
        if (World::ins()->getTile(PlayerX(), PlayerY(), PlayerZ()).VehiclePtr == this) propCeilAlpha = 50;

        for (int layer = 0; layer < (it->second)->itemInfo.size(); layer++)
        {
            if ((it->second)->itemInfo[layer].checkFlag(itemFlag::VEH_ROOF))
            {
                if ((it->second)->itemInfo[layer].itemCode == 314)
                {
                    rotorList.push_back({ it->first[0],it->first[1] });
                }
                else drawVehicleComponent(this, it->first[0], it->first[1], layer, propCeilAlpha);
            }
        }
    }

    for (int i = 0; i < rotorList.size(); i++)
    {
        int propCeilAlpha = 255;
        if (World::ins()->getTile(PlayerX(), PlayerY(), PlayerZ()).VehiclePtr == this) propCeilAlpha = 50;

        int tgtX = rotorList[i].x;
        int tgtY = rotorList[i].y;
        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), propCeilAlpha); //텍스쳐 투명도 설정
        SDL_SetTextureBlendMode(spr::mainRotor->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
        //int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;
        drawSpriteCenter
        (
            spr::mainRotor,
            0,
            dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
        );
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 255); //텍스쳐 투명도 설정
        setZoom(1.0);
    }
};