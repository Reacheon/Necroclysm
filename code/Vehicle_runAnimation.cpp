import Vehicle;
import std;
import globalVar;
import World;
import constVar;
import util;
import Entity;
import Player;
import Ani;
import Coord;
import Drawable;

bool Vehicle::runAnimation(bool shutdown)
{
    //dbgPrt(L"Vehicle %p의 runAnimation이 실행되었다.\n", this);
    //move는 사실상 pulled 카트 용도로만 사용됨
    if (getAniType() == aniFlag::move)//만약 플레이어 인스턴스의 좌표와 목적좌표가 다를 경우
    {
        // 1 / 60초마다 runAnimation이 실행됨
        addTimer();
        const double spd = pullMoveSpd;
        if (getFakeX() > 0)
        {
            addFakeX(-spd);
            if (getFakeX() < 0) setFakeX(0);
        }
        else if (getFakeX() < 0)
        {
            addFakeX(+spd);
            if (getFakeX() > 0) setFakeX(0);
        }

        if (getFakeY() > 0)
        {
            addFakeY(-spd);
            if (getFakeY() < 0) setFakeY(0);
        }
        else if (getFakeY() < 0)
        {
            addFakeY(+spd);
            if (getFakeY() > 0) setFakeY(0);
        }

        if (std::abs(getIntegerFakeX()) == 0 && std::abs(getIntegerFakeY()) == 0)
        {
            resetTimer();
            setAniType(aniFlag::null);
            setFakeX(0);
            setFakeY(0);
            return true;
        }
    }
    else if (getAniType() == aniFlag::propRush)
    {
        addTimer();

        //if (getTimer() > 300) return true;

        {
            static float totalDist = 0;
            static float totalMove = 0;
            static std::vector<Point2> lineRevPath;
            static Point3 startPoint;
            static int lineCheck = 0;
            static Point3 currentCoreGrid;

            if (getTimer() == 1)
            {
                lineRevPath.clear();
                makeLine(lineRevPath, getDelGridX(), getDelGridY());

                totalDist = std::sqrt(std::pow(getDelX(), 2) + std::pow(getDelY(), 2));
                totalMove = 0;
                lineCheck = 0;

                currentCoreGrid = getClosestGridWithFake();
                startPoint = currentCoreGrid;
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

            setFakeX(getFakeX() + xSpd);
            setFakeY(getFakeY() + ySpd);
            //dbgPrt(L"x방향의 속도를 %f, y방향의 속도를 %f만큼 더했다.현재의 fake 좌표는 (%f,%f)이다.\n", xSpd, ySpd, getFakeX(), getFakeY());

            if (xSpd > 0 && getFakeX() > 0) { setFakeX(0); }
            if (xSpd < 0 && getFakeX() < 0) { setFakeX(0); }
            if (ySpd > 0 && getFakeY() > 0) { setFakeY(0); }
            if (ySpd < 0 && getFakeY() < 0) { setFakeY(0); }

            for (const auto& [pos, pocket] : partInfo)
            {
                if (TileEntity(pos.x, pos.y, getGridZ()) != nullptr)
                {
                    TileEntity(pos.x, pos.y, getGridZ())->setFakeX(getFakeX());
                    TileEntity(pos.x, pos.y, getGridZ())->setFakeY(getFakeY());
                }
            }


            if (currentCoreGrid != getClosestGridWithFake())
            {
                currentCoreGrid = getClosestGridWithFake();
                for (int i = 0; i < lineRevPath.size(); i++)
                {
                    if (currentCoreGrid.x == startPoint.x + lineRevPath[i].x && currentCoreGrid.y == startPoint.y + lineRevPath[i].y)
                    {
                        for (int j = lineCheck; j <= i; j++)
                        {
                            updateHeadlight({ startPoint.x + lineRevPath[i].x,startPoint.y + lineRevPath[i].y,getGridZ() });
                            if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == this)
                                PlayerPtr->updateVision(PlayerInfo().eyeSight, startPoint.x + (PlayerX() - getGridX()) + lineRevPath[i].x, startPoint.y + (PlayerY() - getGridY()) + lineRevPath[i].y);
                            lineCheck++;
                        }
                    }
                }
            }

            cameraX = PlayerPtr->getX() + PlayerPtr->getIntegerFakeX();
            cameraY = PlayerPtr->getY() + PlayerPtr->getIntegerFakeY();

            if (getFakeX() == 0 && getFakeY() == 0)//도착
            {
                //dbgPrt(L"도착했다! 현재의 fake 좌표는 (%f,%f)이다.\n", getFakeX(), getFakeY());

                extraRenderEntityList.clear();
                setDelGrid(0, 0);
                setFakeX(0);
                setFakeY(0);
                for (const auto& [pos, pocket] : partInfo)//엔티티 페이크 설정
                {
                    if (TileEntity(pos.x, pos.y, getGridZ()) != nullptr)
                    {
                        Entity* tgtEntity = TileEntity(pos.x, pos.y, getGridZ());
                        tgtEntity->setFakeX(0);
                        tgtEntity->setFakeY(0);
                        tgtEntity->setDelGrid(0, 0);
                    }
                }

                cameraFix = true;
                PlayerPtr->updateVision(PlayerInfo().eyeSight);
                PlayerPtr->updateMinimap();
                resetTimer();
                setAniType(aniFlag::null);
                extraRenderVehList.erase(std::find(extraRenderVehList.begin(), extraRenderVehList.end(), this));
                for (const auto& [pos, pocket] : partInfo)
                {
                    Drawable* iPtr = TileEntity(pos.x, pos.y, getGridZ());
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

            for (const auto& [pos, pocket] : partInfo)
            {
                if (TileEntity(pos.x, pos.y, getGridZ()) != nullptr)
                {
                    TileEntity(pos.x, pos.y, getGridZ())->setFakeX(getIntegerFakeX());
                    TileEntity(pos.x, pos.y, getGridZ())->setFakeY(getIntegerFakeY());
                }
            }

            if (getTimer() == 1)
            {
                if (gearState == gearFlag::drive) bodyDir = singleRailMoveVec[0];
                else if (gearState == gearFlag::reverse) bodyDir = reverse(singleRailMoveVec[0]);
            }

            // dbgPrt(L"[Vehicle : train %p ] 타이머 %d : 연산 후의 fake 좌표는 (%d,%d)이다.\n", this, getTimer(),getIntegerFakeX(), getIntegerFakeY());

            cameraX = PlayerPtr->getX() + PlayerPtr->getIntegerFakeX();
            cameraY = PlayerPtr->getY() + PlayerPtr->getIntegerFakeY();

            if (getTimer() >= 4)
            {
                PlayerPtr->updateVision(PlayerInfo().eyeSight, PlayerX() + (PlayerPtr->getIntegerFakeX() / 16), PlayerY() + (PlayerPtr->getIntegerFakeY() / 16));
                //dbgPrt(L"[Vehicle : train %p ] 카운터가 4보다 커져 fake 좌표가 초기화되었다.\n", this);
                singleRailMoveVec.erase(singleRailMoveVec.begin());
                resetTimer();
            }
        }
        else
        {
            //dbgPrt(L"[Vehicle : train %p ] 이동이 전부 완료된 후의 페이크 좌표는 (%f,%f)이다.\n", this, getFakeX(), getFakeY());
            extraRenderVehList.clear();
            extraRenderEntityList.clear();
            PlayerPtr->updateVision(PlayerInfo().eyeSight);
            PlayerPtr->updateMinimap();
            cameraFix = true;
            resetTimer();
            return true;
        }


    }
    return false;
}
