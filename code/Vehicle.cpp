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
    prt(L"[Vehicle:constructor] �����ڰ� ȣ��Ǿ���. ������ ��ǥ�� %d,%d,%d�̴�.\n", inputX, inputY, inputZ);
    setGrid(inputX, inputY, inputZ);

    errorBox(World::ins()->getTile(inputX, inputY, inputZ).VehiclePtr != nullptr, L"������ġ�� �̹� ������ �����Ѵ�!");
    World::ins()->getTile(inputX, inputY, inputZ).VehiclePtr = this;

    partInfo[{inputX, inputY}] = new ItemPocket(storageType::null);
    partInfo[{inputX, inputY}]->addItemFromDex(leadItemCode, 1);


    deactivateAI();//������ �����ϰ� �⺻������ ��Ȱ��ȭ
}

Vehicle::~Vehicle()
{
    for (auto it = partInfo.begin(); it != partInfo.end(); it++) delete it->second;
    prt(L"[Vehicle:destructor] �Ҹ��ڰ� ȣ��Ǿ���. \n");
}



bool Vehicle::hasFrame(int inputX, int inputY)
{
    for (int i = 0; i < partInfo[{inputX, inputY}]->itemInfo.size(); i++)
    {
        if (partInfo[{inputX, inputY}]->itemInfo[i].checkFlag(itemFlag::VFRAME)) return true;
    }
    return false;
}


/////////////////////////////////////////�� ���� �����ӿ� ��ǰ �߰�////////////////////////////////////////////////////
void Vehicle::addPart(int inputX, int inputY, ItemData inputPart) //�⺻ ��ǰ�߰� �Լ�, ��� �Լ����� �� �Լ��� �⺻���� ��, ���⿡ �˰��� ���� ��
{
    errorBox(partInfo.find({ inputX, inputY }) == partInfo.end(), L"[Vehicle:addPart] �Է��� ��ġ�� �������� �������� �ʴ´�.");
    errorBox(partInfo[{inputX, inputY}]->itemInfo.size() == 0, L"[Vehicle:addPart] �Է��� ��ġ�� �������� itemInfo.size�� 0�� ����.");
    if (inputPart.checkFlag(itemFlag::TIRE_NORMAL) || inputPart.checkFlag(itemFlag::TIRE_STEER))//Ÿ�̾��� ��� �� �տ� �߰�
    {
        partInfo[{inputX, inputY}]->itemInfo.insert(partInfo[{inputX, inputY}]->itemInfo.begin(), inputPart);
    }
    else//�� �� ��ǰ�� �ڿ� �߰�
    {
        partInfo[{inputX, inputY}]->itemInfo.push_back(inputPart);
    }

    //�������� �߽� ����
    if (inputPart.checkFlag(itemFlag::TRAIN_WHEEL)) updateTrainCenter();

    //prt(L"[Vehicle:addPart] (%d,%d)�� ���ο� ��ǰ %ls�� �߰��Ͽ���.\n", inputX, inputY, inputPart.name.c_str());
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

//////////////////////////////////////////////�� ������ Ȯ��/////////////////////////////////////////////////////////
void Vehicle::extendPart(int inputX, int inputY, int inputItemCode)
{
    //���� ���⿡ ��ǰ�� �ִ��� üũ
    for (int i = 0; i < 4; i++)
    {
        int dir = 2 * i;
        int dx, dy;
        dir2Coord(dir, dx, dy);
        //������ ���
        if (partInfo.find({ inputX + dx, inputY + dy }) != partInfo.end()) break;
        errorBox(i == 3, "[Vehicle:extendPart] �����¿쿡 �������� ���µ� �ش� Ÿ�Ϸ� Ȯ���� �õ��ߴ�.");
    }
    errorBox(partInfo.find({ inputX, inputY }) != partInfo.end(), "[Vehicle:extendPart] �̹� �� ���� �������� �ִ� ��ǥ�� Ȯ���� �õ��ߴ�.");

    partInfo[{inputX, inputY}] = new ItemPocket(storageType::null);
    partInfo[{inputX, inputY}]->addItemFromDex(inputItemCode);
    World::ins()->getTile(inputX, inputY, getGridZ()).VehiclePtr = this;

    //prt(L"[Vehicle:extendPart] %p ������ %d,%d ��ġ�� %d �������� Ȯ�忡 ��������.\n", inputX, inputY, inputItemCode);
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
        std::map<std::array<int, 2>, Entity*> entityWormhole; //��ƼƼ�� ���ο� ��ǥ�� �ű�� ���� �ӽ������� �����ϴ� �����̳�
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

        //ȸ���ϴ� ���⿡ ���� ���� ���� �缳��
        if (ACW2(bodyDir) == wheelDir) wheelDir = ACW2(inputDir16);
        else if (ACW(bodyDir) == wheelDir) wheelDir = ACW(inputDir16);
        else if (CW(bodyDir) == wheelDir) wheelDir = CW(inputDir16);
        else if (CW2(bodyDir) == wheelDir) wheelDir = CW2(inputDir16);
        else wheelDir = inputDir16;

        bodyDir = inputDir16;
    }
    //else errorBox(L"[Vehicle:roate] �̹� ������ �ش� ������ ���ϰ� �ִ�.");
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
    std::map<std::array<int, 2>, Entity*> entityWormhole;//��ƼƼ�� ���ο� ��ǥ�� �ű�� ���� �ӽ������� �����ϴ� �����̳�

    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
        if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
        {
            entityWormhole[{it->first[0], it->first[1]}] = (Entity*)World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr;
            World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr = nullptr;
        }
    }

    //��ƼƼ �ű��
    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr = this;
        if (entityWormhole.find({ it->first[0], it->first[1] }) != entityWormhole.end())
        {
            Entity* tgtEntity = entityWormhole[{it->first[0], it->first[1]}];
            tgtEntity->setGrid(it->first[0] + dx, it->first[1] + dy, getGridZ());//��ġ �׸��� ����
            World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).EntityPtr = tgtEntity;//������ ����


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
    std::map<std::array<int, 2>, Entity*> entityWormhole;//��ƼƼ�� ���ο� ��ǥ�� �ű�� ���� �ӽ������� �����ϴ� �����̳�

    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        World::ins()->getTile(it->first[0], it->first[1], getGridZ()).VehiclePtr = nullptr;
        if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
        {
            entityWormhole[{it->first[0], it->first[1]}] = TileEntity(it->first[0], it->first[1], getGridZ());
            World::ins()->getTile(it->first[0], it->first[1], getGridZ()).EntityPtr = nullptr;
        }
    }

    //��ƼƼ �ű��
    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        World::ins()->getTile(it->first[0], it->first[1], getGridZ() + dz).VehiclePtr = this;
        if (entityWormhole.find({ it->first[0], it->first[1] }) != entityWormhole.end())
        {
            Entity* tgtEntity = entityWormhole[{it->first[0], it->first[1]}];
            tgtEntity->setGrid(it->first[0], it->first[1], getGridZ() + dz);//��ġ �׸��� ����
            World::ins()->getTile(it->first[0], it->first[1], getGridZ() + dz).EntityPtr = tgtEntity;//������ ����
        }
    }
    addGridZ(dz);
};

bool Vehicle::colisionCheck(dir16 inputDir16, int dx, int dy)
{
    auto rotatePartInfo = getRotatePartInfo(inputDir16);
    for (auto it = rotatePartInfo.begin(); it != rotatePartInfo.end(); it++)
    {
        //�� �浹 üũ
        if (TileWall(it->first[0] + dx, it->first[1] + dy, getGridZ()) != 0) return true;

        //���� �浹 üũ
        Vehicle* targetPtr = (Vehicle*)World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr;
        if (targetPtr != nullptr && targetPtr != this) return true;
    }
    return false;
}

bool Vehicle::colisionCheck(int dx, int dy)//�ش� dx,dy��ŭ �̵����� �� prop�� �� �Ǵ� ������ Vehicle�� �浹�ϴ���
{
    for (auto it = partInfo.begin(); it != partInfo.end(); it++)
    {
        //�� �浹 üũ
        if (TileWall(it->first[0] + dx, it->first[1] + dy, getGridZ()) != 0) return true;

        //���� �浹 üũ
        Vehicle* targetPtr = (Vehicle*)World::ins()->getTile(it->first[0] + dx, it->first[1] + dy, getGridZ()).VehiclePtr;
        if (targetPtr != nullptr && targetPtr != this)
        {
            prt(L"(%d,%d)��ŭ �̵����� �� ������ %p�� �浹�ߴ�.\n", dx, dy, targetPtr);
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
    //prt(L"Vehicle %p�� runAnimation�� ����Ǿ���.\n", this);
    //move�� ��ǻ� pulled īƮ �뵵�θ� ����
    if (getAniType() == aniFlag::move)//���� �÷��̾� �ν��Ͻ��� ��ǥ�� ������ǥ�� �ٸ� ���
    {
        // 1 / 60�ʸ��� runAnimation�� �����
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
                    //prt(L"�þ� ������Ʈ�� �߽��� (%d,%d)�̰� �ε����� %d�̸� ������� %d�̴�.\n", PlayerX() + visionTarget[0], PlayerY() + visionTarget[1], arrIndex, line.size());

                    Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight, PlayerX() - getDelGridX() + visionTarget[0], PlayerY() - getDelGridY() + visionTarget[1]);
                    lineCheck++;
                }
            }

            setFakeX(getFakeX() + xSpd);
            setFakeY(getFakeY() + ySpd);
            //prt(L"x������ �ӵ��� %f, y������ �ӵ��� %f��ŭ ���ߴ�.������ fake ��ǥ�� (%f,%f)�̴�.\n", xSpd, ySpd, getFakeX(), getFakeY());

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

            if (getFakeX() == 0 && getFakeY() == 0)//����
            {
                prt(L"�����ߴ�! ������ fake ��ǥ�� (%f,%f)�̴�.\n", getFakeX(), getFakeY());

                extraRenderEntityList.clear();
                setDelGrid(0, 0);
                setFakeX(0);
                setFakeY(0);
                for (auto it = partInfo.begin(); it != partInfo.end(); it++)//��ƼƼ ����ũ ����
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

            // prt(L"[Vehicle : train %p ] Ÿ�̸� %d : ���� ���� fake ��ǥ�� (%d,%d)�̴�.\n", this, getTimer(),getIntegerFakeX(), getIntegerFakeY());

            cameraX = Player::ins()->getX() + Player::ins()->getIntegerFakeX();
            cameraY = Player::ins()->getY() + Player::ins()->getIntegerFakeY();

            if (getTimer() >= 4)
            {
                Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight, PlayerX() + (Player::ins()->getIntegerFakeX() / 16), PlayerY() + (Player::ins()->getIntegerFakeY() / 16));
                //prt(L"[Vehicle : train %p ] ī���Ͱ� 4���� Ŀ�� fake ��ǥ�� �ʱ�ȭ�Ǿ���.\n", this);
                singleRailMoveVec.erase(singleRailMoveVec.begin());
                resetTimer();
            }
        }
        else
        {
            //prt(L"[Vehicle : train %p ] �̵��� ���� �Ϸ�� ���� ����ũ ��ǥ�� (%f,%f)�̴�.\n", this, getFakeX(), getFakeY());
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
                if (std::find(trainWheelList.begin(), trainWheelList.end(), it->first) == trainWheelList.end()) //���� ���� ��ǥ�� �ߺ��� ���� ������
                {
                    trainWheelList.push_back({ it->first[0],it->first[1] });
                }
            }
        }
    }
    //trainWheelList�� �߰���ǥ�� ����
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
            SDL_SetTextureAlphaMod(spr::propset->getTexture(), alpha); //�ؽ��� ���� ����
            SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //������ ����
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
            SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //�ؽ��� ���� ����
            setZoom(1.0);
        };


    for (auto it = this->partInfo.begin(); it != this->partInfo.end(); it++)
    {
        ////////////////////////////////�Ϲ� ������ǰ/////////////////////////////////////////////////
        for (int layer = 0; layer < (it->second)->itemInfo.size(); layer++)
        {
            //�ٴ�����,õ������ �÷��װ� ���� �Ϲ� ������ ���
            if (!(it->second)->itemInfo[layer].checkFlag(itemFlag::VEH_ROOF))
            {
                drawVehicleComponent(this, (it->first)[0], (it->first)[1], layer, 255);
            }
        }

        ////////////////////////////////õ�� ������ǰ////////////////////////////////////////////////////
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
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), propCeilAlpha); //�ؽ��� ���� ����
        SDL_SetTextureBlendMode(spr::mainRotor->getTexture(), SDL_BLENDMODE_BLEND); //������ ����
        //int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;
        drawSpriteCenter
        (
            spr::mainRotor,
            0,
            dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
        );
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 255); //�ؽ��� ���� ����
        setZoom(1.0);
    }
};