import Vehicle;
import std;
import globalVar;
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
import log;
import Prop;

Vehicle::Vehicle(int inputX, int inputY, int inputZ, int leadItemCode)
{
    trainWheelCenter = { inputX, inputY };

    setAniPriority(3);
    prt(L"[Vehicle:constructor] 생성자가 호출되었다. 생성된 좌표는 %d,%d,%d이다.\n", inputX, inputY, inputZ);
    setGrid(inputX, inputY, inputZ);

    errorBox(TileVehicle(inputX, inputY, inputZ) != nullptr, L"생성위치에 이미 프롭이 존재한다!");
    TileVehicle(inputX, inputY, inputZ) = this;

    partInfo[{inputX, inputY, inputZ}] = std::make_unique<ItemPocket>(storageType::null);
    partInfo[{inputX, inputY, inputZ}]->addItemFromDex(leadItemCode, 1);


    deactivateAI();//차량을 제외하고 기본적으로 비활성화
}

Vehicle::~Vehicle()
{
    // 광역 청크 소멸(게임 종료 등) 도중에는 청크가 이미 unordered_map에서 분리됨 →
    // TileVehicle()/getChunk()의 .at()이 throw. 부품마다 다른 청크에 걸쳐 있을 수 있어
    // 각 위치마다 tryGetChunk로 확인 후 접근.
    const int gz = getGridZ();

    //점유 타일에서 Vehicle 포인터 제거 (청크별 안전 접근)
    for (const auto& [pos, pocket] : partInfo)
    {
        Point2 partCC = World::ins()->changeToChunkCoord(pos.x, pos.y);
        if (Chunk* partChunk = World::ins()->tryGetChunk(partCC.x, partCC.y, gz))
        {
            int localX = pos.x - (partCC.x * CHUNK_SIZE_X);
            int localY = pos.y - (partCC.y * CHUNK_SIZE_Y);
            partChunk->getChunkTile(localX, localY).VehiclePtr = nullptr;
        }
    }

    //청크에서 등록 해제 (현재 위치 청크)
    Point2 currentChunkCoord = World::ins()->changeToChunkCoord(getGridX(), getGridY());
    if (Chunk* currentChunk = World::ins()->tryGetChunk(currentChunkCoord.x, currentChunkCoord.y, gz))
    {
        currentChunk->eraseVehicle(this);
    }

    prt(L"[Vehicle:destructor] 소멸자가 호출되었다. \n");
}



bool Vehicle::hasFrame(int inputX, int inputY)
{
    auto it = partInfo.find({ inputX, inputY, getGridZ() });
    if (it != partInfo.end())
    {
        std::vector<ItemData>& vParts = partInfo[{inputX, inputY, getGridZ()}]->itemInfo;
        for (int i = 0; i < vParts.size(); i++)
        {
            if (vParts[i].checkFlag(itemFlag::VFRAME)) return true;
        }
    }
    return false;
}


/////////////////////////////////////////※ 기존 프레임에 부품 추가////////////////////////////////////////////////////
void Vehicle::addPart(int inputX, int inputY, int dexIndex)
{
    errorBox(partInfo.find({ inputX, inputY, getGridZ() }) == partInfo.end(), L"[Vehicle:addPart] 입력한 위치에 프레임이 존재하지 않는다.");

    ItemData inputPart = cloneFromItemDex(itemDex[dexIndex], 1);

    // 타이어의 경우 맨 앞에 추가 (기존 로직 유지)
    if (inputPart.checkFlag(itemFlag::TIRE_NORMAL) || inputPart.checkFlag(itemFlag::TIRE_STEER))
    {
        partInfo[{inputX, inputY, getGridZ()}]->itemInfo.insert(partInfo[{inputX, inputY, getGridZ()}]->itemInfo.begin(), std::move(inputPart));
    }
    else
    {
        // 새로운 부품의 priority
        int newPriority = inputPart.propDrawPriority;

        // priority에 따른 삽입 위치 찾기
        auto& itemVec = partInfo[{inputX, inputY, getGridZ()}]->itemInfo;
        auto insertPos = itemVec.end();

        // 뒤에서부터 탐색하여 새 부품보다 낮거나 같은 priority를 가진 위치 찾기
        for (auto it = itemVec.rbegin(); it != itemVec.rend(); ++it)
        {
            if (it->propDrawPriority <= newPriority)
            {
                insertPos = it.base(); // reverse_iterator를 forward_iterator로 변환
                break;
            }
        }

        itemVec.insert(insertPos, std::move(inputPart));
    }

    // 열차바퀴 중심 설정
    if (inputPart.checkFlag(itemFlag::TRAIN_WHEEL)) updateTrainCenter();

    updateSpr();
}
void Vehicle::addPart(int inputX, int inputY, std::vector<int> dexVec)
{
    for (int i = 0; i < dexVec.size(); i++) addPart(inputX, inputY, dexVec[i]);
}

void Vehicle::erasePart(int inputX, int inputY, int index)
{
    if (partInfo[{ inputX, inputY, getGridZ() }]->itemInfo[index].checkFlag(itemFlag::TRAIN_WHEEL)) updateTrainCenter();

    partInfo[{ inputX, inputY, getGridZ() }]->eraseItemInfo(index);
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
        if (partInfo.find({ inputX + dx, inputY + dy, getGridZ() }) != partInfo.end()) break;
        errorBox(i == 3, L"[Vehicle:extendPart] 상하좌우에 프레임이 없는데 해당 타일로 확장을 시도했다.");
    }
    errorBox(partInfo.find({ inputX, inputY, getGridZ() }) != partInfo.end(), L"[Vehicle:extendPart] 이미 이 프롭 프레임이 있는 좌표로 확장을 시도했다.");

    partInfo[{inputX, inputY, getGridZ()}] = std::make_unique<ItemPocket>(storageType::null);
    partInfo[{inputX, inputY, getGridZ()}]->addItemFromDex(inputItemCode);
    TileVehicle(inputX, inputY, getGridZ()) = this;

    //prt(L"[Vehicle:extendPart] %p 차량이 %d,%d 위치로 %d 아이템을 확장에 성공다.\n", inputX, inputY, inputItemCode);
    updateSpr();
}

void Vehicle::setGrid(int inputGridX, int inputGridY, int inputGridZ)
{
    // 갓 생성된 Vehicle은 기본 좌표 (0,0,0)에서 시작하므로, 플레이어가 원점에서 멀리 떨어져
    // wipeOrphanedChunks가 (0,0,0)을 비운 상태에서는 prev 청크가 존재하지 않는다.
    // 소멸자와 동일하게 tryGetChunk로 안전하게 조회. (없으면 erase 자체가 의미 없음)
    Point2 prevChunkCoord = World::ins()->changeToChunkCoord(getGridX(), getGridY());
    if (Chunk* prevChunk = World::ins()->tryGetChunk(prevChunkCoord.x, prevChunkCoord.y, getGridZ()))
    {
        prevChunk->eraseVehicle(this);
    }

    Coord::setGrid(inputGridX, inputGridY, inputGridZ);

    Point2 currentChunkCoord = World::ins()->changeToChunkCoord(getGridX(), getGridY());
    Chunk& currentChunk = World::ins()->getChunk(currentChunkCoord.x, currentChunkCoord.y, getGridZ());
    currentChunk.addVehicle(this);
}

int Vehicle::getSprIndex(int inputX, int inputY)
{
    errorBox(partInfo[{inputX, inputY, getGridZ()}]->itemInfo.size() == 0, L"[Vehicle:getSprIndex] The vehicle part are empty(itemInfo size is zero)");
    return partInfo[{inputX, inputY, getGridZ()}]->itemInfo[0].getSprIndex();
}

void Vehicle::rotatePartInfo(dir16 inputDir16)
{
    if (bodyDir != inputDir16)
    {
        std::unordered_map<Point3, std::unique_ptr<ItemPocket>, Point3::Hash> newPartInfo;
        auto currentCoordTransform = coordTransform[bodyDir];
        auto targetCoordTransform = coordTransform[inputDir16];
        // straddle 시 z가 섞여있을 수 있어 partInfo 직접 순회 (xy 스캔 X) + z 보존
        for (auto& [pos, pocket] : partInfo)
        {
            Point2 originCoord = currentCoordTransform[{pos.x - getGridX(), pos.y - getGridY()}];
            Point2 dstCoord;
            for (const auto& [coord, transformedCoord] : targetCoordTransform)
            {
                if (transformedCoord == originCoord)
                {
                    dstCoord = coord;
                    break;
                }
            }
            newPartInfo[{dstCoord.x + getGridX(), dstCoord.y + getGridY(), pos.z}] = std::move(pocket);
        }
        partInfo = std::move(newPartInfo);
    }
}

std::unordered_set<Point3, Point3::Hash> Vehicle::getRotateShadow(dir16 inputDir16)
{
    if (bodyDir != inputDir16)
    {
        std::unordered_set<Point3, Point3::Hash> newPartInfo;
        auto currentCoordTransform = coordTransform[bodyDir];
        auto targetCoordTransform = coordTransform[inputDir16];
        for (const auto& [pos, pocket] : partInfo)
        {
            Point2 originCoord = currentCoordTransform[{pos.x - getGridX(), pos.y - getGridY()}];
            Point2 dstCoord;
            for (const auto& [coord, transformedCoord] : targetCoordTransform)
            {
                if (transformedCoord == originCoord)
                {
                    dstCoord = coord;
                    break;
                }
            }
            newPartInfo.insert({ dstCoord.x + getGridX(), dstCoord.y + getGridY(), pos.z });
        }
        return newPartInfo;
    }
    else
    {
        std::unordered_set<Point3, Point3::Hash> newPartInfo;
        for (const auto& [pos, pocket] : partInfo)
        {
            newPartInfo.insert({ pos.x, pos.y, pos.z });
        }
        return newPartInfo;
    }
}

void Vehicle::rotateEntityPtr(dir16 inputDir16)
{
    if (bodyDir != inputDir16)
    {
        // straddle 대응: (x,y) 충돌 가능성이 있어도 z 다르면 별개. Point3 키로 wormhole.
        std::unordered_map<Point3, std::unique_ptr<Entity>, Point3::Hash> entityWormhole;
        for (const auto& [pos, pocket] : partInfo)
        {
            if (TileEntity(pos.x, pos.y, pos.z) != nullptr)
            {
                entityWormhole[pos] = std::move(World::ins()->getTile(pos.x, pos.y, pos.z).EntityPtr);
            }
        }

        auto currentCoordTransform = coordTransform[bodyDir];
        auto targetCoordTransform = coordTransform[inputDir16];
        for (const auto& [pos, pocket] : partInfo)
        {
            Point2 originCoord = currentCoordTransform[{pos.x - getGridX(), pos.y - getGridY()}];
            Point2 dstCoord;
            for (const auto& [coord, transformedCoord] : targetCoordTransform)
            {
                if (transformedCoord == originCoord)
                {
                    dstCoord = coord;
                    break;
                }
            }

            if (entityWormhole.find(pos) != entityWormhole.end())
            {
                EntityPtrMove(std::move(entityWormhole[pos]), { dstCoord.x + getGridX(), dstCoord.y + getGridY(), pos.z });
            }
        }
    }
}

void Vehicle::rotate(dir16 inputDir16)
{
    if (bodyDir != inputDir16)
    {
        for (const auto& [pos, pocket] : partInfo)
        {
            TileVehicle(pos.x, pos.y, pos.z) = nullptr;
        }

        rotateEntityPtr(inputDir16);
        rotatePartInfo(inputDir16);

        for (const auto& [pos, pocket] : partInfo)
        {
            TileVehicle(pos.x, pos.y, pos.z) = this;
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
    updateHeadlight();

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

    for (const auto& [pos, pocket] : partInfo)
    {
        int tgtX = pos.x;
        int tgtY = pos.y;
        int tgtRelX = tgtX - getGridX();
        int tgtRelY = tgtY - getGridY();
        ItemPocket* tgtPocket = pocket.get();
        for (int layer = 0; layer < tgtPocket->itemInfo.size(); layer++)
        {
            if (tgtPocket->itemInfo[layer].checkFlag(itemFlag::VPART_WALL_CONNECT))
            {
                auto checkWallGroup = [=](int dx, int dy)->bool
                    {
                        int currentGroup = tgtPocket->itemInfo[layer].tileConnectGroup;
                        Point2 value1 = coordTransform[bodyDir][{tgtRelX, tgtRelY}];
                        Point2 key1;
                        for (const auto& [coord, transformedCoord] : coordTransform[refDir])
                        {
                            if (transformedCoord == value1)
                            {
                                key1 = coord;
                            }
                        }
                        Point2 value2 = coordTransform[refDir][{key1.x + dx, key1.y + dy}];
                        Point2 key2;
                        for (const auto& [coord, transformedCoord] : coordTransform[bodyDir])
                        {
                            if (transformedCoord == value2)
                            {
                                key2 = coord;
                            }
                        }
                        if (partInfo.find({ getGridX() + key2.x, getGridY() + key2.y, getGridZ() }) != partInfo.end())
                        {
                            std::vector<ItemData>& tgtItemInfo = partInfo[{getGridX() + key2.x, getGridY() + key2.y, getGridZ()}]->itemInfo;
                            for (int i = 0; i < tgtItemInfo.size(); i++)
                            {
                                if (/*tgtItemInfo[i].checkFlag(itemFlag::PROP_WALL_CONNECT) && */tgtItemInfo[i].tileConnectGroup == currentGroup)
                                {
                                    return true;
                                }
                            }
                            return false;
                        }
                        return false;
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
    if (dx == 0 && dy == 0) return;

    // 큰 점프도 한 칸씩 분해 — 경유 ramp 무시 방지
    std::vector<Point2> path;
    makeLine(path, dx, dy);

    Point2 prev = { 0, 0 };
    int totalDx = 0, totalDy = 0;
    int currentZ = getGridZ();

    for (const auto& pt : path)
    {
        int stepDx = pt.x - prev.x;
        int stepDy = pt.y - prev.y;
        prev = pt;
        if (stepDx == 0 && stepDy == 0) continue;

        // ramp 트리거 검사: 어떤 파츠든 ramp 타일에 닿으면 텔레포트 발동
        // 정방향(ramp 같은 z 진입) + 역방향(ramp 위 z+1 타일 진입 = 다리 끝 도달 시 하강) 모두 처리
        int dz = 0;
        Point2 rampHitXY{ 0, 0 };
        bool triggered = false;
        for (const auto& [pos, pocket] : partInfo)
        {
            int destX = pos.x + stepDx;
            int destY = pos.y + stepDy;
            Prop* destProp = TileProp(destX, destY, pos.z);
            Prop* belowProp = TileProp(destX, destY, pos.z - 1);
            Prop* aboveProp = TileProp(destX, destY, pos.z + 1);
            if (destProp != nullptr && destProp->leadItem.checkFlag(itemFlag::RAMP_UP)
                && TileFloor(destX + stepDx, destY + stepDy, pos.z + 1) != 0)
            {
                rampHitXY = { destX, destY }; dz = 1; triggered = true; break;
            }
            if (destProp != nullptr && destProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
                && TileFloor(destX + stepDx, destY + stepDy, pos.z - 1) != 0)
            {
                rampHitXY = { destX, destY }; dz = -1; triggered = true; break;
            }
            if (belowProp != nullptr && belowProp->leadItem.checkFlag(itemFlag::RAMP_UP)
                && TileFloor(destX + stepDx, destY + stepDy, pos.z - 1) != 0)
            {
                rampHitXY = { destX, destY }; dz = -1; triggered = true; break;
            }
            if (aboveProp != nullptr && aboveProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
                && TileFloor(destX + stepDx, destY + stepDy, pos.z + 1) != 0)
            {
                rampHitXY = { destX, destY }; dz = 1; triggered = true; break;
            }
        }

        std::unordered_map<Point3, Point3, Point3::Hash> partOldToNew;
        int appliedDx = stepDx, appliedDy = stepDy;
        int appliedZ = currentZ;

        if (triggered)
        {
            // 후미: step 방향 dot 최소인 파츠
            int minDot = std::numeric_limits<int>::max();
            int rearX = 0, rearY = 0;
            for (const auto& [pos, pocket] : partInfo)
            {
                int dot = pos.x * stepDx + pos.y * stepDy;
                if (dot < minDot) { minDot = dot; rearX = pos.x; rearY = pos.y; }
            }
            // 후미 새 위치 = rampHit + step (즉 ramp 1칸 앞에 후미)
            int offsetX = (rampHitXY.x + stepDx) - rearX;
            int offsetY = (rampHitXY.y + stepDy) - rearY;
            appliedDx = offsetX; appliedDy = offsetY; appliedZ = currentZ + dz;
            for (const auto& [pos, pocket] : partInfo)
            {
                partOldToNew[pos] = { pos.x + offsetX, pos.y + offsetY, appliedZ };
            }

            // 텔레포트 도착지 전 범위 장애물 검사 — 하나라도 막히면 ramp 진입 자체 차단
            bool blocked = false;
            Point3 blockPos{ 0,0,0 };
            const wchar_t* blockReason = L"";
            for (const auto& [oldPos, newPos] : partOldToNew)
            {
                if (TileFloor(newPos.x, newPos.y, newPos.z) == 0)
                {
                    blocked = true; blockPos = newPos; blockReason = L"floor 없음"; break;
                }
                if (TileWall(newPos.x, newPos.y, newPos.z) != 0)
                {
                    blocked = true; blockPos = newPos; blockReason = L"wall"; break;
                }
                Prop* p = TileProp(newPos.x, newPos.y, newPos.z);
                if (p != nullptr && !p->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER))
                {
                    blocked = true; blockPos = newPos; blockReason = L"prop"; break;
                }
                Vehicle* v = TileVehicle(newPos.x, newPos.y, newPos.z);
                if (v != nullptr && v != this)
                {
                    blocked = true; blockPos = newPos; blockReason = L"vehicle"; break;
                }
            }
            if (blocked)
            {
                prt(L"[Vehicle:shift] ramp 텔레포트 막힘 - %s 도착지(%d,%d,%d)\n",
                    blockReason, blockPos.x, blockPos.y, blockPos.z);
                break;
            }
        }
        else
        {
            for (const auto& [pos, pocket] : partInfo)
            {
                partOldToNew[pos] = { pos.x + stepDx, pos.y + stepDy, pos.z };
            }
        }

        // 이동 적용
        std::unordered_map<Point3, std::unique_ptr<Entity>, Point3::Hash> entityWormhole;
        for (const auto& [oldPos, newPos] : partOldToNew)
        {
            TileVehicle(oldPos.x, oldPos.y, oldPos.z) = nullptr;
            if (TileEntity(oldPos.x, oldPos.y, oldPos.z) != nullptr)
            {
                entityWormhole[oldPos] = std::move(World::ins()->getTile(oldPos.x, oldPos.y, oldPos.z).EntityPtr);
            }
        }
        for (const auto& [oldPos, newPos] : partOldToNew)
        {
            TileVehicle(newPos.x, newPos.y, newPos.z) = this;
            if (entityWormhole.find(oldPos) != entityWormhole.end())
            {
                EntityPtrMove(std::move(entityWormhole[oldPos]), newPos);
            }
        }
        std::unordered_map<Point3, std::unique_ptr<ItemPocket>, Point3::Hash> shiftPartInfo;
        for (auto& [pos, pocket] : partInfo)
        {
            shiftPartInfo[partOldToNew[pos]] = std::move(pocket);
        }
        partInfo = std::move(shiftPartInfo);

        totalDx += appliedDx;
        totalDy += appliedDy;
        currentZ = appliedZ;

        if (triggered) break; // 텔레포트로 이동 완료, 잔여 path 무시
    }

    setGrid(getGridX() + totalDx, getGridY() + totalDy, currentZ);
    updateHeadlight();
}

void Vehicle::zShift(int dz)
{
    std::unordered_map<Point2, std::unique_ptr<Entity>, Point2::Hash> entityWormhole;//엔티티를 새로운 좌표로 옮기기 전에 임시적으로 저장하는 컨테이너

    for (const auto& [pos, pocket] : partInfo)
    {
        TileVehicle(pos.x, pos.y, getGridZ()) = nullptr;
        if (TileEntity(pos.x, pos.y, getGridZ()) != nullptr)
        {
            entityWormhole[{pos.x, pos.y}] = std::move(World::ins()->getTile(pos.x, pos.y, getGridZ()).EntityPtr);
        }
    }

    //엔티티 옮기기
    for (const auto& [pos, pocket] : partInfo)
    {
        TileVehicle(pos.x, pos.y, getGridZ() + dz) = this;
        if (entityWormhole.find({ pos.x, pos.y }) != entityWormhole.end())
        {
            World::ins()->getTile(pos.x, pos.y, getGridZ() + dz).EntityPtr = std::move(entityWormhole[{pos.x, pos.y}]);
            TileEntity(pos.x, pos.y, getGridZ() + dz)->setGrid(pos.x, pos.y, getGridZ() + dz);//위치 그리드 변경
        }
    }

    std::unordered_map<Point3, std::unique_ptr<ItemPocket>, Point3::Hash> shiftPartInfo;
    for (auto& [pos, pocket] : partInfo)
    {
        shiftPartInfo[{pos.x, pos.y, pos.z + dz}] = std::move(pocket);
    }
    partInfo = std::move(shiftPartInfo);

    setGrid(getGridX(), getGridY(), getGridZ() + dz);
    updateHeadlight();

};

bool Vehicle::colisionCheck(dir16 inputDir16, int dx, int dy)
{
    auto rotatedPartInfo = getRotateShadow(inputDir16);

    // ramp 트리거 검사 (정방향 + 역방향)
    int dz = 0;
    Point2 rampHitXY{ 0, 0 };
    bool triggered = false;
    for (const auto& pos : rotatedPartInfo)
    {
        int destX = pos.x + dx, destY = pos.y + dy;
        Prop* destProp = TileProp(destX, destY, pos.z);
        Prop* belowProp = TileProp(destX, destY, pos.z - 1);
        Prop* aboveProp = TileProp(destX, destY, pos.z + 1);
        if (destProp != nullptr && destProp->leadItem.checkFlag(itemFlag::RAMP_UP)
            && TileFloor(destX + dx, destY + dy, pos.z + 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = 1; triggered = true; break;
        }
        if (destProp != nullptr && destProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
            && TileFloor(destX + dx, destY + dy, pos.z - 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = -1; triggered = true; break;
        }
        if (belowProp != nullptr && belowProp->leadItem.checkFlag(itemFlag::RAMP_UP)
            && TileFloor(destX + dx, destY + dy, pos.z - 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = -1; triggered = true; break;
        }
        if (aboveProp != nullptr && aboveProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
            && TileFloor(destX + dx, destY + dy, pos.z + 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = 1; triggered = true; break;
        }
    }

    if (triggered)
    {
        // 텔레포트 후 위치 계산 — 후미 = rampHit + step
        int minDot = std::numeric_limits<int>::max();
        int rearX = 0, rearY = 0;
        for (const auto& pos : rotatedPartInfo)
        {
            int dot = pos.x * dx + pos.y * dy;
            if (dot < minDot) { minDot = dot; rearX = pos.x; rearY = pos.y; }
        }
        int offsetX = (rampHitXY.x + dx) - rearX;
        int offsetY = (rampHitXY.y + dy) - rearY;
        for (const auto& pos : rotatedPartInfo)
        {
            int nx = pos.x + offsetX, ny = pos.y + offsetY, nz = pos.z + dz;
            if (TileFloor(nx, ny, nz) == 0)
            {
                prt(L"[Vehicle:colisionCheck(dir)] ramp 텔레포트 막힘 - floor 없음 (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
            if (TileWall(nx, ny, nz) != 0)
            {
                prt(L"[Vehicle:colisionCheck(dir)] ramp 텔레포트 막힘 - wall (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
            Prop* p = TileProp(nx, ny, nz);
            if (p != nullptr && !p->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER))
            {
                prt(L"[Vehicle:colisionCheck(dir)] ramp 텔레포트 막힘 - prop (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
            Vehicle* v = TileVehicle(nx, ny, nz);
            if (v != nullptr && v != this)
            {
                prt(L"[Vehicle:colisionCheck(dir)] ramp 텔레포트 막힘 - vehicle (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
        }
        return false;
    }

    // 정규 이동 충돌 체크
    for (const auto& pos : rotatedPartInfo)
    {
        int nx = pos.x + dx, ny = pos.y + dy, nz = pos.z;
        if (TileWall(nx, ny, nz) != 0) return true;
        Prop* p = TileProp(nx, ny, nz);
        if (p != nullptr && !p->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER)) return true;
        Vehicle* v = TileVehicle(nx, ny, nz);
        if (v != nullptr && v != this) return true;
    }
    return false;
}

bool Vehicle::colisionCheck(int dx, int dy)
{
    // ramp 트리거 검사 (정방향 + 역방향)
    int dz = 0;
    Point2 rampHitXY{ 0, 0 };
    bool triggered = false;
    for (const auto& [pos, pocket] : partInfo)
    {
        int destX = pos.x + dx, destY = pos.y + dy;
        Prop* destProp = TileProp(destX, destY, pos.z);
        Prop* belowProp = TileProp(destX, destY, pos.z - 1);
        Prop* aboveProp = TileProp(destX, destY, pos.z + 1);
        if (destProp != nullptr && destProp->leadItem.checkFlag(itemFlag::RAMP_UP)
            && TileFloor(destX + dx, destY + dy, pos.z + 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = 1; triggered = true; break;
        }
        if (destProp != nullptr && destProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
            && TileFloor(destX + dx, destY + dy, pos.z - 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = -1; triggered = true; break;
        }
        if (belowProp != nullptr && belowProp->leadItem.checkFlag(itemFlag::RAMP_UP)
            && TileFloor(destX + dx, destY + dy, pos.z - 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = -1; triggered = true; break;
        }
        if (aboveProp != nullptr && aboveProp->leadItem.checkFlag(itemFlag::RAMP_DOWN)
            && TileFloor(destX + dx, destY + dy, pos.z + 1) != 0)
        {
            rampHitXY = { destX, destY }; dz = 1; triggered = true; break;
        }
    }

    if (triggered)
    {
        int minDot = std::numeric_limits<int>::max();
        int rearX = 0, rearY = 0;
        for (const auto& [pos, pocket] : partInfo)
        {
            int dot = pos.x * dx + pos.y * dy;
            if (dot < minDot) { minDot = dot; rearX = pos.x; rearY = pos.y; }
        }
        int offsetX = (rampHitXY.x + dx) - rearX;
        int offsetY = (rampHitXY.y + dy) - rearY;
        for (const auto& [pos, pocket] : partInfo)
        {
            int nx = pos.x + offsetX, ny = pos.y + offsetY, nz = pos.z + dz;
            if (TileFloor(nx, ny, nz) == 0)
            {
                prt(L"[Vehicle:colisionCheck] ramp 텔레포트 막힘 - floor 없음 (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
            if (TileWall(nx, ny, nz) != 0)
            {
                prt(L"[Vehicle:colisionCheck] ramp 텔레포트 막힘 - wall (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
            Prop* p = TileProp(nx, ny, nz);
            if (p != nullptr && !p->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER))
            {
                prt(L"[Vehicle:colisionCheck] ramp 텔레포트 막힘 - prop (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
            Vehicle* v = TileVehicle(nx, ny, nz);
            if (v != nullptr && v != this)
            {
                prt(L"[Vehicle:colisionCheck] ramp 텔레포트 막힘 - vehicle (%d,%d,%d)\n", nx, ny, nz);
                return true;
            }
        }
        return false;
    }

    for (const auto& [pos, pocket] : partInfo)
    {
        int nx = pos.x + dx, ny = pos.y + dy, nz = pos.z;
        if (TileWall(nx, ny, nz) != 0) return true;
        Prop* p = TileProp(nx, ny, nz);
        if (p != nullptr && !p->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER)) return true;
        Vehicle* v = TileVehicle(nx, ny, nz);
        if (v != nullptr && v != this)
        {
            prt(L"(%d,%d)만큼 이동했을 때 포인터 %p와 충돌했다.\n", dx, dy, v);
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
    cameraFix = false;
    setFakeX(-getDelX());
    setFakeY(-getDelY());
    extraRenderVehList.push_back(this);
    for (const auto& [pos, pocket] : partInfo)
    {
        if (auto e = TileEntity(pos.x, pos.y, getGridZ()))
        {
            e->setFakeX(-getDelX());
            e->setFakeY(-getDelY());
            extraRenderEntityList.push_back((e));
        }
    }

    updateHeadlight(getClosestGridWithFake());
    if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == this) PlayerPtr->updateVision(PlayerInfo().eyeSight, getClosestGridWithFake().x, getClosestGridWithFake().y);

    addAniToMonsterTurn(this, aniFlag::propRush);
}

void Vehicle::centerShift(int dx, int dy, int dz)
{
}

void Vehicle::updateHeadlight()
{
    for (const auto& [pos, pocket] : partInfo)
    {
        for (int i = 0; i < pocket->itemInfo.size(); i++)
        {
            if (pocket->itemInfo[i].checkFlag(itemFlag::HEADLIGHT))
            {
                if (headlightOn)
                {
                    if (pocket->itemInfo[i].lightPtr != nullptr)
                    {
                        Light* thisLight = pocket->itemInfo[i].lightPtr.get();
                        thisLight->dir = bodyDir;
                        thisLight->moveLight(pos.x, pos.y, pos.z);
                    }
                }
            }
        }
    }
}

void Vehicle::updateHeadlight(Point3 fakeCoor) //코어가 해당 위치에 가정하고 계산합니다.
{
    for (const auto& [pos, pocket] : partInfo)
    {
        for (int i = 0; i < pocket->itemInfo.size(); i++)
        {
            if (pocket->itemInfo[i].checkFlag(itemFlag::HEADLIGHT))
            {
                if (headlightOn)
                {
                    if (pocket->itemInfo[i].lightPtr != nullptr)
                    {
                        Light* thisLight = pocket->itemInfo[i].lightPtr.get();
                        thisLight->dir = bodyDir;
                        int revX = pos.x - getGridX();
                        int revY = pos.y - getGridY();

                        thisLight->moveLight(fakeCoor.x + revX, fakeCoor.y + revY, pos.z);
                    }
                }
            }
        }
    }
}

void Vehicle::updateTrainCenter()
{
    std::vector<Point2> trainWheelList;

    for (const auto& [pos, pocket] : partInfo)
    {
        ItemPocket* pocketPtr = pocket.get();
        for (int i = 0; i < pocketPtr->itemInfo.size(); i++)
        {
            if (pocketPtr->itemInfo[i].checkFlag(itemFlag::TRAIN_WHEEL))
            {
                Point2 pos2{ pos.x, pos.y };
                if (std::find(trainWheelList.begin(), trainWheelList.end(), pos2) == trainWheelList.end()) //열차 바퀴 좌표가 중복된 값이 없으면
                {
                    trainWheelList.push_back(pos2);
                }
            }
        }
    }
    //trainWheelList의 중간좌표를 구함
    trainWheelCenter = calcMidpoint(trainWheelList);
}

int Vehicle::getGasolineFuel()
{
    int gasolineNumber = 0;
    for (const auto& [pos, pocket] : partInfo)
    {
        ItemPocket* partPtr = pocket.get();
        for (int i = 0; i < partPtr->itemInfo.size(); i++)
        {
            ItemPocket* tankPtr = partPtr->itemInfo[i].pocketPtr.get();
            if (tankPtr != nullptr)
            {
                for (int j = 0; j < tankPtr->itemInfo.size(); j++)
                {
                    if (tankPtr->itemInfo[j].itemCode == itemID::gasoline)
                    {
                        gasolineNumber += tankPtr->itemInfo[j].number;
                    }
                }
            }
        }
    }
    return gasolineNumber;
}

int Vehicle::getDiselFuel()
{
    int diselNumber = 0;
    for (const auto& [pos, pocket] : partInfo)
    {
        ItemPocket* partPtr = pocket.get();
        for (int i = 0; i < partPtr->itemInfo.size(); i++)
        {
            ItemPocket* tankPtr = partPtr->itemInfo[i].pocketPtr.get();
            if (tankPtr != nullptr)
            {
                for (int j = 0; j < tankPtr->itemInfo.size(); j++)
                {
                    if (tankPtr->itemInfo[j].itemCode == itemID::diesel)
                    {
                        diselNumber += tankPtr->itemInfo[j].number;
                    }
                }
            }
        }
    }
    return diselNumber;
}

int Vehicle::getElectricityFuel()
{
    int electricityNumber = 0;
    for (const auto& [pos, pocket] : partInfo)
    {
        ItemPocket* partPtr = pocket.get();
        for (int i = 0; i < partPtr->itemInfo.size(); i++)
        {
            ItemPocket* tankPtr = partPtr->itemInfo[i].pocketPtr.get();
            if (tankPtr != nullptr)
            {
                for (int j = 0; j < tankPtr->itemInfo.size(); j++)
                {
                    if (tankPtr->itemInfo[j].itemCode == itemID::electricity)
                    {
                        electricityNumber += tankPtr->itemInfo[j].number;
                    }
                }
            }
        }
    }
    return electricityNumber;
}

ItemData* Vehicle::getMainEngine()
{
    for (const auto& [pos, pocket] : partInfo)
    {
        ItemPocket* pocketPtr = pocket.get();
        for (int i = 0; i < pocketPtr->itemInfo.size(); i++)
        {
            if (pocketPtr->itemInfo[i].checkFlag(itemFlag::ENGINE_GASOLINE)) return &pocketPtr->itemInfo[i];
            else if (pocketPtr->itemInfo[i].checkFlag(itemFlag::ENGINE_DIESEL)) return &pocketPtr->itemInfo[i];
            else if (pocketPtr->itemInfo[i].checkFlag(itemFlag::ENGINE_ELECTRIC)) return &pocketPtr->itemInfo[i];
        }
    }
    return nullptr;
}

int Vehicle::getEngineFuel()
{
    int fuelNumber = 0;

    if (getMainEngine() == nullptr) return 0;
    else
    {
        if (getMainEngine()->checkFlag(itemFlag::ENGINE_GASOLINE)) fuelNumber = getGasolineFuel();
        else if (getMainEngine()->checkFlag(itemFlag::ENGINE_DIESEL)) fuelNumber = getDiselFuel();
        else if (getMainEngine()->checkFlag(itemFlag::ENGINE_ELECTRIC)) fuelNumber = getElectricityFuel();
    }
    return fuelNumber;
}


void Vehicle::useEngineFuel(int fuelAmount)
{
    int fuelNumber = 0;
    int targetFuelCode = 0;

    if (getMainEngine() == nullptr) return;
    else
    {
        if (getMainEngine()->checkFlag(itemFlag::ENGINE_GASOLINE)) targetFuelCode = itemID::gasoline;
        else if (getMainEngine()->checkFlag(itemFlag::ENGINE_DIESEL)) targetFuelCode = itemID::diesel;
        else if (getMainEngine()->checkFlag(itemFlag::ENGINE_ELECTRIC)) targetFuelCode = itemID::electricity;
    }

    for (const auto& [pos, pocket] : partInfo)
    {
        int eraseNumber = 0;
        ItemPocket* partPtr = pocket.get();
        for (int i = 0; i < partPtr->itemInfo.size(); i++)
        {
            ItemPocket* tankPtr = partPtr->itemInfo[i].pocketPtr.get();
            if (tankPtr != nullptr)
            {
                for (int j = 0; j < tankPtr->itemInfo.size(); j++)
                {
                    if (tankPtr->itemInfo[j].itemCode == targetFuelCode)
                    {
                        if (eraseNumber < fuelAmount)
                        {
                            if (tankPtr->itemInfo[j].number > fuelAmount - eraseNumber)
                            {
                                tankPtr->itemInfo[j].number -= fuelAmount - eraseNumber;
                                eraseNumber = fuelAmount;
                            }
                            else
                            {
                                eraseNumber += tankPtr->itemInfo[j].number;
                                tankPtr->itemInfo.erase(tankPtr->itemInfo.begin() + j);
                                j--;
                            }
                        }
                        else break;
                    }
                }
            }
        }
    }
}
