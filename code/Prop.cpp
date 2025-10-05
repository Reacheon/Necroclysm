import Prop;

#include <SDL3/SDL.h>

import globalVar;
import wrapVar;
import textureVar;
import constVar;
import util;
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
import drawSprite;
import globalTime;
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

void Prop::drawSelf()
{
    int tileSize = 16 * zoomScale;
    int bigShift = 16 * (leadItem.checkFlag(itemFlag::PROP_BIG));
    SDL_Rect dst;
    dst.x = cameraW / 2 + zoomScale * ((16 * getGridX() + 8) - cameraX) - ((16 * zoomScale) / 2);
    dst.y = cameraH / 2 + zoomScale * ((16 * getGridY() + 8 - bigShift) - cameraY) - ((16 * zoomScale) / 2);
    dst.w = tileSize;
    dst.h = tileSize;


    setZoom(zoomScale);
    if (leadItem.checkFlag(itemFlag::TREE) && getGridX() == PlayerX() && getGridY() - 1 == PlayerY() && getGridZ() == PlayerZ() && !leadItem.checkFlag(itemFlag::STUMP))
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 100); //텍스쳐 투명도 설정
    }
    else
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
    }

    SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
    int sprIndex = leadItem.propSprIndex + leadItem.extraSprIndexSingle + 16 * leadItem.extraSprIndex16;

    if (leadItem.checkFlag(itemFlag::TREE))//나무일 경우 그림자
    {
        drawSpriteCenter
        (
            spr::propset,
            sprIndex+8,
            dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
        );
    }


    if (leadItem.checkFlag(itemFlag::CABLE_BEHIND))
    {
		Prop* rightProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
		Prop* topProp = TileProp(getGridX(), getGridY() - 1, getGridZ());
		Prop* leftProp = TileProp(getGridX() - 1, getGridY(), getGridZ());
		Prop* botProp = TileProp(getGridX(), getGridY() + 1, getGridZ());

        bool isRightCable = rightProp != nullptr && rightProp->leadItem.checkFlag(itemFlag::CABLE);
		bool isTopCable = topProp != nullptr && topProp->leadItem.checkFlag(itemFlag::CABLE);
		bool isLeftCable = leftProp != nullptr && leftProp->leadItem.checkFlag(itemFlag::CABLE);
		bool isBotCable = botProp != nullptr && botProp->leadItem.checkFlag(itemFlag::CABLE);

        if (isRightCable || isTopCable || isLeftCable || isBotCable)
        {
            int cableSprIndex = 2720;

            if (isRightCable && !isTopCable && !isLeftCable && !isBotCable) cableSprIndex = 2732;
            else if (!isRightCable && isTopCable && !isLeftCable && !isBotCable) cableSprIndex = 2733;
            else if (!isRightCable && !isTopCable && isLeftCable && !isBotCable) cableSprIndex = 2730;
            else if (!isRightCable && !isTopCable && !isLeftCable && isBotCable) cableSprIndex = 2731;

            else if (isRightCable && isTopCable && !isLeftCable && !isBotCable) cableSprIndex = 2729;
            else if (isRightCable && !isTopCable && isLeftCable && !isBotCable) cableSprIndex = 2936;
            else if (isRightCable && !isTopCable && !isLeftCable && isBotCable) cableSprIndex = 2725;
            else if (!isRightCable && isTopCable && isLeftCable && !isBotCable) cableSprIndex = 2729;
            else if (!isRightCable && isTopCable && !isLeftCable && isBotCable) cableSprIndex = 2939;
            else if (!isRightCable && !isTopCable && isLeftCable && isBotCable) cableSprIndex = 2723;

            else if (isRightCable && isTopCable && isLeftCable && !isBotCable) cableSprIndex = 2728;
            else if (isRightCable && isTopCable && !isLeftCable && isBotCable) cableSprIndex = 2726;
            else if (isRightCable && !isTopCable && isLeftCable && isBotCable) cableSprIndex = 2724;
            else if (!isRightCable && isTopCable && isLeftCable && isBotCable) cableSprIndex = 2722;

            else if (isRightCable && isTopCable && isLeftCable && isBotCable) cableSprIndex = 2720;


            drawSpriteCenter
            (
                spr::propset,
                cableSprIndex,
                dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
            );
        }
    }

    if (leadItem.checkFlag(itemFlag::PLANT_SEASON_DEPENDENT) && !leadItem.checkFlag(itemFlag::STUMP))
    {
        if (World::ins()->getTile(getGridX(), getGridY(), PlayerZ()).hasSnow == true) sprIndex += 4;
        else
        {
            if (getSeason() == seasonFlag::summer) { sprIndex += 1; }
            else if (getSeason() == seasonFlag::autumn) { sprIndex += 2; }
            else if (getSeason() == seasonFlag::winter) { sprIndex += 3; }
        }
    }
    else if (leadItem.checkFlag(itemFlag::STUMP))
    {
        sprIndex +=7;
    }
    else if (leadItem.itemCode == itemRefCode::gasolineGeneratorR ||
        leadItem.itemCode == itemRefCode::gasolineGeneratorT ||
        leadItem.itemCode == itemRefCode::gasolineGeneratorL ||
        leadItem.itemCode == itemRefCode::gasolineGeneratorB)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            static Uint32 lastUpdateTime = 0;
            static int animFrame = 0;

            Uint32 currentTime = SDL_GetTicks();

            if (currentTime - lastUpdateTime >= 100)
            {
                animFrame = (animFrame + 1) % 3;
                lastUpdateTime = currentTime;
            }

            sprIndex += (2 + animFrame);
        }
    }


    drawSpriteCenter
    (
        spr::propset,
        sprIndex,
        dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
        dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
    );

    if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
    {
        if (leadItem.itemCode == itemRefCode::gasolineGeneratorR ||
            leadItem.itemCode == itemRefCode::gasolineGeneratorT ||
            leadItem.itemCode == itemRefCode::gasolineGeneratorL ||
            leadItem.itemCode == itemRefCode::gasolineGeneratorB)
        {
            int portSprIndex;
            if (leadItem.itemCode == itemRefCode::gasolineGeneratorR) portSprIndex = 3200;
            else if (leadItem.itemCode == itemRefCode::gasolineGeneratorT)  portSprIndex = 3201;
            else if (leadItem.itemCode == itemRefCode::gasolineGeneratorL)   portSprIndex = 3202;
            else if (leadItem.itemCode == itemRefCode::gasolineGeneratorB)   portSprIndex = 3203;

            float pulseSpeed = 0.003f; // 펄스 속도 (작을수록 느림)
            float minBrightness = 0.6f; // 최소 밝기 (0.0~1.0)
            float maxBrightness = 1.0f; // 최대 밝기

            float pulse = (sin(SDL_GetTicks() * pulseSpeed) + 1.0f) * 0.5f; // 0.0~1.0 사이값
            float colorAlpha = minBrightness + (maxBrightness - minBrightness) * pulse;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetTextureColorMod(spr::propset->getTexture(),
                (Uint8)(255.0f * colorAlpha),
                (Uint8)(255.0f * colorAlpha),
                (Uint8)(255.0f * colorAlpha));

            drawSpriteCenter
            (
                spr::propset,
                portSprIndex,
                dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
            );

            SDL_SetTextureColorMod(spr::propset->getTexture(), 255, 255, 255);
        }
    }

    if (treeAngle != 0)
    {
        int extraSprIndex = 9;
        if (World::ins()->getTile(getGridX(), getGridY(), PlayerZ()).hasSnow == true) extraSprIndex += 4;
        else
        {
            if (getSeason() == seasonFlag::summer) { extraSprIndex += 1; }
            else if (getSeason() == seasonFlag::autumn) { extraSprIndex += 2; }
            else if (getSeason() == seasonFlag::winter) { extraSprIndex += 3; }
        }
        SDL_Point pt = { 24.0*zoomScale,40.0*zoomScale };
        drawSpriteCenterRotate
        (
            spr::propset,
            leadItem.propSprIndex + extraSprIndex,
            dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * getIntegerFakeY(),
            treeAngle,
            &pt
        );
    }



    if (displayHPBarCount > 0)//개체 HP 표기
    {
        int pivotX = dst.x + dst.w / 2 - (int)(8 * zoomScale);
        int pivotY = dst.y + dst.h / 2 + (int)(16 * zoomScale);
        if (leadItem.propFakeHP > leadItem.propHP) leadItem.propFakeHP -= ((float)leadItem.propMaxHP / 100.0);
        else if (leadItem.propFakeHP < leadItem.propHP) leadItem.propFakeHP = leadItem.propHP;
        if (leadItem.propFakeHP != leadItem.propHP)
        {
            if (alphaFakeHPBar > 20) alphaFakeHPBar -= 20;
            else
            {
                alphaFakeHPBar = 0;
                leadItem.propFakeHP = leadItem.propHP;
            }
        }
        else alphaFakeHPBar = 0;
        if (displayHPBarCount > 1) displayHPBarCount--;
        else if (displayHPBarCount == 1)
        {
            alphaHPBar -= 10;
            if (alphaHPBar <= 0)
            {
                alphaHPBar = 0;
                displayHPBarCount = 0;
            }
        }
        draw3pxGauge(
            pivotX,
            pivotY,
            zoomScale,
            (float)leadItem.propHP / (float)leadItem.propMaxHP,
            alphaHPBar,
            lowCol::green,
            (float)leadItem.propFakeHP / (float)leadItem.propMaxHP,
            alphaFakeHPBar
        );
    }

    SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
    setZoom(1.0);
};



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


        std::queue<Point2> frontierQueue;
        std::unordered_set<Point2, Point2::Hash> visitedSet;
        std::queue<Point2> voltageSourceQueue;
        
        int circuitMaxEnergy = 0; //kJ

        if (cursorX == 17 && cursorY == 8)
        {
            int a = 2;
        }


        auto pushIfConnected = [&](int curretX, int currentY, int dx, int dy, itemFlag currentDirFlag, itemFlag neighborOppFlag)
            {
                Point2 dirCoord{ curretX + dx, currentY + dy };
                if (visitedSet.find(dirCoord) != visitedSet.end()) return;

                Prop* dirProp = TileProp(dirCoord.x, dirCoord.y, getGridZ());
                if (!dirProp) return;

				Prop* currentProp = TileProp(curretX, currentY, getGridZ());
                bool curOK = currentProp->leadItem.checkFlag(currentDirFlag) || currentProp->leadItem.checkFlag(itemFlag::CABLE);
                if (!curOK) return;

                if (dirProp->leadItem.checkFlag(itemFlag::CIRCUIT) && (dirProp->leadItem.checkFlag(neighborOppFlag) || dirProp->leadItem.checkFlag(itemFlag::CABLE)))
                {
                    frontierQueue.push(dirCoord);
                }
            };

		frontierQueue.push({ cursorX, cursorY });
        while (!frontierQueue.empty())
        {
            Point2 current = frontierQueue.front();
            frontierQueue.pop();

            if (visitedSet.find(current) != visitedSet.end()) continue;
            visitedSet.insert(current);

            std::wprintf(L"[소자 탐색] 현재 커서는 %d,%d에 있습니다.\n", current.x, current.y);

            Prop* currentProp = TileProp(current.x, current.y, getGridZ());

            if (currentProp && (currentProp->leadItem.checkFlag(itemFlag::CIRCUIT) || currentProp->leadItem.checkFlag(itemFlag::CABLE)))
            {
                currentProp->runUsed = true;
                if (currentProp->leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE))
                {
                    circuitMaxEnergy += currentProp->leadItem.electricMaxPower;
                    voltageSourceQueue.push(current);
                }
                pushIfConnected(current.x, current.y, +1, 0, itemFlag::CABLE_CNCT_RIGHT, itemFlag::CABLE_CNCT_LEFT);
                pushIfConnected(current.x, current.y, 0, -1, itemFlag::CABLE_CNCT_TOP, itemFlag::CABLE_CNCT_BOT);
                pushIfConnected(current.x, current.y, -1, 0, itemFlag::CABLE_CNCT_LEFT, itemFlag::CABLE_CNCT_RIGHT);
                pushIfConnected(current.x, current.y, 0, +1, itemFlag::CABLE_CNCT_BOT, itemFlag::CABLE_CNCT_TOP);
            }
        }

        std::wprintf(L"회로 탐색을 종료했습니다.\n");
        if (visitedSet.size() > 0)std::wprintf(L"회로의 사이즈는 %d입니다.\n", visitedSet.size());
        if (circuitMaxEnergy > 0) std::wprintf(L"회로의 최대 에너지 수송량은 %d kJ입니다.\n", circuitMaxEnergy);
        
        //전압 방향 설정
        dir16 currentVoltageDir = dir16::none;
        while (!voltageSourceQueue.empty())
        {
			Point2 current = voltageSourceQueue.front();
			voltageSourceQueue.pop();
            Prop* sourceProp = TileProp(current.x, current.y, getGridZ());
           
            if(sourceProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_RIGHT)) currentVoltageDir = dir16::dir0;
            else if (sourceProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_TOP)) currentVoltageDir = dir16::dir2;
            else if (sourceProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_LEFT)) currentVoltageDir = dir16::dir4;
			else if (sourceProp->leadItem.checkFlag(itemFlag::VOLTAGE_OUTPUT_BOT)) currentVoltageDir = dir16::dir6;
            
            frontierQueue = std::queue<Point2>();
            visitedSet = std::unordered_set<Point2, Point2::Hash>();

            while (!frontierQueue.empty())
            {
                Point2 current = frontierQueue.front();
                frontierQueue.pop();
            }
        }




        if (TileProp(getGridX() + 1, getGridY(), getGridZ()) != nullptr)
        {
            if (leadItem.checkFlag(itemFlag::CABLE_CNCT_RIGHT) || leadItem.checkFlag(itemFlag::CABLE))
            {
                Prop* tgtProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
                if (tgtProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_LEFT) || tgtProp->leadItem.checkFlag(itemFlag::CABLE))
                {

                }
            }
        }

    }

    runUsed = true;
}