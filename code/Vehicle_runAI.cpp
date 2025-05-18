#define AI_TURN_END return true;
#define AI_TURN_CONTINUE return false;

import Vehicle;
import globalVar;
import wrapVar;
import constVar;
import util;
import Entity;
import Player;

bool Vehicle::runAI()
{
    if (isAIFirstRun) isAIFirstRun = false;

    //prt(L"[Vehicle:AI] ID : %p의 AI를 실행시켰다.\n", this);

    while (1)
    {

        //prt(L"[Vehicle:AI] ID : %p의 turnResource는 %f입니다.\n", this, getTurnResource());
        if (getTurnResource() > 2.0)
        {
            clearTurnResource();
            addTurnResource(2.0);
        }

        if (vehType == vehFlag::train)
        {
            if (rpmState > 0 && TileProp(getGridX(), getGridY(), getGridZ()) != nullptr)
            {
                if (bodyDir == dir16::dir0 || bodyDir == dir16::dir4)
                {
                    if (TileProp(getGridX(), getGridY(), getGridZ())->leadItem.itemCode == itemVIPCode::wideRailHMid)
                    {
                        if (gearState == gearFlag::drive) spdVec = scalarMultiple(dir16ToVec(bodyDir), (float)rpmState);
                        else if (gearState == gearFlag::reverse) spdVec = scalarMultiple(dir16ToVec(reverse(bodyDir)), (float)rpmState);
                        else spdVec = { 0,0,0 };

                    }
                }
                else if (bodyDir == dir16::dir2 || bodyDir == dir16::dir6)
                {
                    if (TileProp(getGridX(), getGridY(), getGridZ())->leadItem.itemCode == itemVIPCode::wideRailVMid)
                    {
                        if (gearState == gearFlag::drive) spdVec = scalarMultiple(dir16ToVec(bodyDir), (float)rpmState);
                        else if (gearState == gearFlag::reverse) spdVec = scalarMultiple(dir16ToVec(reverse(bodyDir)), (float)rpmState);
                        else spdVec = { 0,0,0 };
                    }
                }
            }
        }

        //========================================================= ACT1 광차 =========================================================
        if (singleRailSpdVal > 0)
        {
            int trainPrevX = getX(), trainPrevY = getY(), trainPrevZ = getGridZ();
            int trainCursorX = getGridX(), trainCursorY = getGridY(), trainCursorZ = getGridZ();
            int prevX = trainCursorX, prevY = trainCursorY, prevZ = trainCursorZ;
            dir16 initSpdDir = singleRailSpdDir;
            int initGridX = getGridX();
            int initGridY = getGridY();
            int initSingleRailSpdVal = singleRailSpdVal;

            if (isPowerCart && gearState == gearFlag::reverse)
            {
                Vehicle* finalCart = nullptr;
                Vehicle* cartCursor = this;
                std::vector<Vehicle*> cartList = { this, };
                while (1)
                {
                    if (cartCursor->rearCart != nullptr)
                    {
                        cartCursor = cartCursor->rearCart;
                        cartList.push_back(cartCursor);
                    }
                    else
                    {
                        finalCart = cartCursor;
                        break;
                    }
                }

                for (int i = cartList.size() - 1; i >= 1; i--)
                {
                    cartList[i]->singleRailSpdVal = singleRailSpdVal;
                    cartList[i]->gearState = gearFlag::reverse;
                    int dir = coord2Dir(cartList[i]->getGridX() - cartList[i - 1]->getGridX(), cartList[i]->getGridY() - cartList[i - 1]->getGridY());
                    cartList[i]->singleRailSpdDir = int8todir16(dir);
                    cartList[i]->runAI();
                }
            }

            //[반복문] 계산된 방향과 속도값으로 singleRailMoveVec을 순서대로 채워넣음
            for (int i = 0; i < singleRailSpdVal; i++)
            {
                //1. 열차의 현재 위치에 따라 속도의 방향 수정////////////////////////////////////////////////////////////////////
                Prop* currentRail = TileProp(trainCursorX, trainCursorY, trainCursorZ);
                dir16 prevSpdDir = singleRailSpdDir;
                if (currentRail != nullptr)
                {
                    if (currentRail->leadItem.checkFlag(itemFlag::RAIL_BUFFER))//버퍼 레일 강제정지
                    {
                        prt(L"열차가 버퍼 레일을 만나서 정지하였다.\n");
                        singleRailSpdVal = 0;
                        break;
                    }

                    if (gearState == gearFlag::drive || gearState == gearFlag::reverse)//주행기어
                    {
                        if (singleRailSpdDir == dir16::dir0)//동쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = dir16::dir0;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = dir16::dir2;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = dir16::dir6;
                        }
                        else if (singleRailSpdDir == dir16::dir2)//북쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = dir16::dir2;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = dir16::dir4;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = dir16::dir0;
                        }
                        else if (singleRailSpdDir == dir16::dir4)//서쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = dir16::dir4;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = dir16::dir2;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = dir16::dir6;
                        }
                        else if (singleRailSpdDir == dir16::dir6)//남쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = dir16::dir6;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = dir16::dir4;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = dir16::dir0;
                        }
                    }
                }
                else
                {
                    prt(L"[Vehicle:train %p] 현재 이 차량의 위치에 레일이 설치되어있지 않다.\n", this);
                    singleRailSpdVal = 0;
                    break;
                }

                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //3. 속도 방향에 따라 좌회전,우회전,직진 플래그 설정///////////////////////////////////////////////////////////
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                itemFlag straightFlag, leftFlag, rightFlag;
                dir16 straightDir = singleRailSpdDir, leftDir, rightDir;
                int dx = 0, dy = 0;
                int dxLeft = 0, dyLeft = 0;
                int dxRight = 0, dyRight = 0;

                auto intDir = [](dir16 input)->int
                    {
                        switch (input)
                        {
                        case dir16::dir0:
                            return 0;
                            break;
                        case dir16::dir2:
                            return 2;
                            break;
                        case dir16::dir4:
                            return 4;
                            break;
                        case dir16::dir6:
                            return 6;
                            break;
                        }
                    };

                if (singleRailSpdDir == dir16::dir0)
                {
                    straightFlag = itemFlag::RAIL_CNCT_RIGHT;
                    leftFlag = itemFlag::RAIL_CNCT_TOP;
                    rightFlag = itemFlag::RAIL_CNCT_BOT;

                    leftDir = dir16::dir2;
                    rightDir = dir16::dir6;
                }
                else if (singleRailSpdDir == dir16::dir2)
                {
                    straightFlag = itemFlag::RAIL_CNCT_TOP;
                    leftFlag = itemFlag::RAIL_CNCT_LEFT;
                    rightFlag = itemFlag::RAIL_CNCT_RIGHT;

                    leftDir = dir16::dir4;
                    rightDir = dir16::dir0;
                }
                else if (singleRailSpdDir == dir16::dir4)
                {
                    straightFlag = itemFlag::RAIL_CNCT_LEFT;
                    leftFlag = itemFlag::RAIL_CNCT_BOT;
                    rightFlag = itemFlag::RAIL_CNCT_TOP;

                    leftDir = dir16::dir6;
                    rightDir = dir16::dir2;
                }
                else if (singleRailSpdDir == dir16::dir6)
                {
                    straightFlag = itemFlag::RAIL_CNCT_BOT;
                    leftFlag = itemFlag::RAIL_CNCT_RIGHT;
                    rightFlag = itemFlag::RAIL_CNCT_LEFT;

                    leftDir = dir16::dir0;
                    rightDir = dir16::dir4;
                }

                dir2Coord(intDir(straightDir), dx, dy);
                dir2Coord(intDir(leftDir), dxLeft, dyLeft);
                dir2Coord(intDir(rightDir), dxRight, dyRight);

                prevX = trainCursorX, prevY = trainCursorY, prevZ = trainCursorZ;
                if (currentRail->leadItem.checkFlag(straightFlag))
                {
                    if (colisionCheck(singleRailSpdDir, trainCursorX - getGridX() + dx, trainCursorY - getGridY() + dy) == true)
                    {
                        prt(L"[Vehicle:train ] %p 열차가 1칸 이동했을 때 다른 객체와 충돌하여 속도가 0으로 설정되었다.\n", this);
                        singleRailSpdDir = prevSpdDir;
                        singleRailSpdVal = 0;
                        break;
                    }
                    else
                    {
                        trainCursorX += dx;
                        trainCursorY += dy;
                        singleRailMoveVec.push_back(straightDir);
                    }
                }
                else if (currentRail->leadItem.checkFlag(leftFlag))
                {
                    if (colisionCheck(singleRailSpdDir, trainCursorX - getGridX() + dxLeft, trainCursorY - getGridY() + dyLeft) == true)
                    {
                        prt(L"[Vehicle:train ] %p 열차가 1칸 이동했을 때 다른 객체와 충돌하여 속도가 0으로 설정되었다.\n", this);
                        singleRailSpdDir = prevSpdDir;
                        singleRailSpdVal = 0;
                        break;
                    }
                    else
                    {
                        trainCursorX += dxLeft;
                        trainCursorY += dyLeft;
                        singleRailMoveVec.push_back(leftDir);
                    }
                }
                else if (currentRail->leadItem.checkFlag(rightFlag))
                {
                    if (colisionCheck(singleRailSpdDir, trainCursorX - getGridX() + dxRight, trainCursorY - getGridY() + dyRight) == true)
                    {
                        prt(L"[Vehicle:train ] %p 열차가 1칸 이동했을 때 다른 객체와 충돌하여 속도가 0으로 설정되었다.\n", this);
                        singleRailSpdDir = prevSpdDir;
                        singleRailSpdVal = 0;
                        break;
                    }
                    else
                    {
                        trainCursorX += dxRight;
                        trainCursorY += dyRight;
                        singleRailMoveVec.push_back(rightDir);
                    }
                }
            }

            //prt(L"[Vehicle:train] 열차의 이동이 (%d,%d)로 실행되었다. 변위는 (%d,%d)이다.\n", trainCursorX, trainCursorY, trainCursorX - getGridX(), trainCursorY - getGridY());
            singleRailSpdVal = 0;
            shift(trainCursorX - getGridX(), trainCursorY - getGridY());

            //if(gearState == gearFlag::drive) bodyDir = singleRailSpdDir;
            //else if (gearState == gearFlag::reverse)  bodyDir = reverse(singleRailSpdDir);


            if (true)
            {
                setFakeX(trainPrevX - getX());
                setFakeY(trainPrevY - getY());
                for (auto it = partInfo.begin(); it != partInfo.end(); it++)
                {
                    if (TileEntity(it->first[0], it->first[1], getGridZ()) != nullptr)
                    {
                        TileEntity(it->first[0], it->first[1], getGridZ())->setFakeX(getIntegerFakeX());
                        TileEntity(it->first[0], it->first[1], getGridZ())->setFakeY(getIntegerFakeY());
                    }
                }

                //prt(L"[Vehicle : train %p ] 차량의 fake 좌표는 (%f,%f)로 설정했다\n", this, getFakeX(), getFakeY());

                //iAmDictator();
                addAniUSetMonster(this, aniFlag::minecartRush);

                extraRenderVehList.push_back(this);
                for (auto it = partInfo.begin(); it != partInfo.end(); it++)
                {
                    Drawable* iPtr =TileEntity(it->first[0], it->first[1], getGridZ());
                    if (iPtr != nullptr) extraRenderEntityList.push_back(iPtr);
                }
                cameraFix = false;
                PlayerPtr->updateVision(PlayerPtr->entityInfo.eyeSight, PlayerX() + (PlayerPtr->getIntegerFakeX() / 16), PlayerY() + (PlayerPtr->getIntegerFakeY() / 16));
            }

            if (gearState == gearFlag::drive)
            {
                if (rearCart != nullptr)
                {
                    rearCart->singleRailSpdVal = initSingleRailSpdVal;
                    int dir = coord2Dir(initGridX - rearCart->getGridX(), initGridY - rearCart->getGridY());
                    rearCart->singleRailSpdDir = int8todir16(dir);
                    rearCart->gearState = gearFlag::drive;
                    rearCart->runAI();
                }
            }

            AI_TURN_CONTINUE;
        }

        //========================================================= ACT2 광차 속도 설정 ===============================================
        if (getTurnResource() >= 1.0)
        {
            if (vehType == vehFlag::minecart && rpmState >= 1)
            {
                useTurnResource(1.0);

                //열차 엣지 (7,-16)
                int vx = getGridX();
                int vy = getGridY();
                int vz = getGridZ();

                //1. 차량 속도값 설정
                singleRailSpdVal = 10;
                if (singleRailSpdVal != 0)//마찰에 의한 손실
                {
                    float massCoeff = 1.5;
                    float frictionCoeff = 1.0;
                    float delSpd = frictionCoeff / massCoeff;

                    if (delSpd > singleRailSpdVal) singleRailSpdVal = 0;
                    else singleRailSpdVal - delSpd;
                }

                //2. 현재 차량의 위치에 있는 레일에 따라 시작하는 속도의 방향(singleRailSpdDir)을 정한다.
                Prop* currentRail = TileProp(vx, vy, vz);
                if (currentRail != nullptr)
                {
                    if (gearState == gearFlag::drive)//주행기어
                    {
                        if (bodyDir == dir16::dir0)//동쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = dir16::dir0;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = dir16::dir2;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = dir16::dir6;
                        }
                        else if (bodyDir == dir16::dir2)//북쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = dir16::dir2;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = dir16::dir4;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = dir16::dir0;
                        }
                        else if (bodyDir == dir16::dir4)//서쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = dir16::dir4;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = dir16::dir2;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = dir16::dir6;
                        }
                        else if (bodyDir == dir16::dir6)//남쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = dir16::dir6;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = dir16::dir4;
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = dir16::dir0;
                        }
                    }
                    else if (gearState == gearFlag::reverse)//후진기어
                    {
                        if (bodyDir == dir16::dir0)//동쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = reverse(dir16::dir0);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = reverse(dir16::dir2);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = reverse(dir16::dir6);
                        }
                        else if (bodyDir == dir16::dir2)//북쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = reverse(dir16::dir2);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = reverse(dir16::dir4);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = reverse(dir16::dir0);
                        }
                        else if (bodyDir == dir16::dir4)//서쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = reverse(dir16::dir4);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_TOP)) singleRailSpdDir = reverse(dir16::dir2);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = reverse(dir16::dir6);
                        }
                        else if (bodyDir == dir16::dir6)//남쪽 방향 열차
                        {
                            if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_BOT)) singleRailSpdDir = reverse(dir16::dir6);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_LEFT)) singleRailSpdDir = reverse(dir16::dir4);
                            else if (currentRail->leadItem.checkFlag(itemFlag::RAIL_CNCT_RIGHT)) singleRailSpdDir = reverse(dir16::dir0);
                        }
                    }
                }
                else
                {
                    prt(L"[Vehicle:train] 현재 이 차량의 위치에 레일이 설치되어있지 않다.\n");
                    AI_TURN_CONTINUE;
                }
                AI_TURN_CONTINUE;
            }
        }

        //========================================================= ACT3 차량 헬기 속도 설정 ===========================================
        if (getTurnResource() >= 1.0)
        {
            if ((spdVec.compX != 0 || spdVec.compY != 0 || spdVec.compZ != 0) || (accVec.compX != 0 || accVec.compY != 0 || accVec.compZ != 0))
            {
                if (vehType == vehFlag::car || vehType == vehFlag::heli)
                {
                    useTurnResource(1.0);

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

                    if (dstX != 0 || dstY != 0)
                    {
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
                                prt(L"[Vehicle] Path 이동 중 충돌이 발생하여 이동을 멈춥니다.\n");
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
                                if (wheelDir == ACW2(bodyDir) && rotateAcc > 7 && colisionCheck(ACW2(bodyDir), 0, 0) == false) rotate(ACW2(bodyDir));
                                else if (wheelDir == ACW(bodyDir) && rotateAcc > 11 && colisionCheck(ACW(bodyDir), 0, 0) == false) rotate(ACW(bodyDir));
                                else if (wheelDir == CW(bodyDir) && rotateAcc > 11 && colisionCheck(CW(bodyDir), 0, 0) == false) rotate(CW(bodyDir));
                                else if (wheelDir == CW2(bodyDir) && rotateAcc > 7 && colisionCheck(CW2(bodyDir), 0, 0) == false) rotate(CW2(bodyDir));
                            }
                            xAcc -= std::floor(xAcc);
                            yAcc -= std::floor(yAcc);
                        }
                        updateSpr();
                        AI_TURN_CONTINUE;
                    }
                }
                
            }
        }


        //========================================================= ACT4 열차 속도 설정 ===========================================
        if (getTurnResource() >= 1.0)
        {
            if ((spdVec.compX != 0 || spdVec.compY != 0 || spdVec.compZ != 0) || (accVec.compX != 0 || accVec.compY != 0 || accVec.compZ != 0))
            {
                if (vehType == vehFlag::train && isPowerTrain)
                {
                    useTurnResource(1.0);

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

                    if (dstX != 0 || dstY != 0)
                    {
                        Vehicle* finalTrain = nullptr;
                        Vehicle* trainCursor = this;
                        std::vector<Vehicle*> trainList = { this, };
                        while (1)
                        {
                            if (trainCursor->rearTrain != nullptr)
                            {
                                trainCursor = trainCursor->rearTrain;
                                trainList.push_back(trainCursor);
                            }
                            else
                            {
                                finalTrain = trainCursor;
                                break;
                            }
                        }


                        if(gearState == gearFlag::reverse) std::reverse(trainList.begin(), trainList.end());
                        
                        for (int j = 0; j < trainList.size(); j++)
                        {
                            std::vector<std::array<int, 2>> path;
                            makeLine(path, dstX, dstY);

                            int dxFinal = 0;
                            int dyFinal = 0;
                            for (int i = 1; i < path.size(); i++)
                            {
                                if (trainList[j]->colisionCheck(path[i][0], path[i][1]))//충돌할 경우
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
                                trainList[j]->rush(dxFinal, dyFinal);
                                trainList[j]->xAcc -= std::floor(xAcc);
                                trainList[j]->yAcc -= std::floor(yAcc);
                            }
                            updateSpr();
                        }


                        AI_TURN_CONTINUE;
                    }
                }

            }
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //위의 모든 패턴 조건을 만족하지않을시 return true
        //prt(L"[Vehicle:AI] AI가 true를 반환했다. AI를 종료합니다.\n");
        isAIFirstRun = true;
        AI_TURN_END;
    }
}