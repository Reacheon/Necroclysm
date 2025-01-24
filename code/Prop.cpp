#include <SDL.h>

import Prop;
import globalVar;
import wrapVar;
import textureVar;
import constVar;
import util;
import World;
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
import Player;

Prop::Prop(int inputX, int inputY, int inputZ, int leadItemCode)
{
    leadItem = itemDex[leadItemCode];
    setAniPriority(3);
    //prt(L"[Prop:constructor] 생성자가 호출되었다. 생성된 좌표는 %d,%d,%d이다.\n", inputX, inputY, inputZ);
    setGrid(inputX, inputY, inputZ);

    errorBox(TileProp(inputX, inputY, inputZ) != nullptr, L"생성위치에 이미 설치물이 존재한다!");
    World::ins()->getTile(inputX, inputY, inputZ).PropPtr = this;
    updateTile();

    if (itemDex[leadItemCode].checkFlag(itemFlag::LIGHT_ON))
    {

        myLight = new Light(inputX + leadItem.lightDelX, inputY + leadItem.lightDelY, inputZ, leadItem.lightRange, leadItem.lightIntensity, { leadItem.lightR,leadItem.lightG,leadItem.lightB });//임시로 이렇게 만들어놨음
        //Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
    }


    deactivateAI();//차량을 제외하고 기본적으로 비활성화

    //주변 타일을 분석해 extraIndex 설정
    int dx = 0;
    int dy = 0;
    for (int i = 0; i < 8; i++)
    {
        dir2Coord(i, dx, dy);
        Prop* targetProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
        if (targetProp != nullptr) targetProp->updateSprIndex();
    }

    if (leadItem.randomPropSprSize != 1)
    {
        leadItem.propSprIndex += randomRange(0, leadItem.randomPropSprSize - 1);
    }

    updateSprIndex();
}

Prop::~Prop()
{
    delete myLight;
    prt(L"[Prop:destructor] 소멸자가 호출되었다. \n");

    //주변 타일을 분석해 extraIndex 설정
    int dx = 0;
    int dy = 0;
    for (int i = 0; i < 8; i++)
    {
        dir2Coord(i, dx, dy);
        Prop* targetProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
        if (targetProp != nullptr) targetProp->updateSprIndex();
    }
    updateSprIndex();
}

void Prop::updateSprIndex()
{
    bool topTile = false;
    bool botTile = false;
    bool leftTile = false;
    bool rightTile = false;



    if (leadItem.checkFlag(itemFlag::WIRE))//전선일 경우
    {
        auto wireCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::WIRE))//같은 전선일 경우
                    {
                        return true;
                    }
                    else
                    {
                        if ((dx == 0 && dy == -1) && tgtProp->leadItem.checkFlag(itemFlag::WIRE_CNCT_BOT)) return true;
                        else if ((dx == 0 && dy == 1) && tgtProp->leadItem.checkFlag(itemFlag::WIRE_CNCT_TOP)) return true;
                        else if ((dx == 1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::WIRE_CNCT_LEFT)) return true;
                        else if ((dx == -1 && dy == 0) && tgtProp->leadItem.checkFlag(itemFlag::WIRE_CNCT_RIGHT)) return true;
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

        topTile = sameCheck(0, -1);
        botTile = sameCheck(0, 1);
        leftTile = sameCheck(-1, 0);
        rightTile = sameCheck(1, 0);
        leadItem.extraSprIndexSingle = connectGroupExtraIndex(topTile, botTile, leftTile, rightTile);
    }
}

void Prop::updateTile()
{
    World::ins()->getTile(getGridX(), getGridY(), getGridZ()).update();
    //walkable 체크
    if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable == true)
    {
        if (leadItem.checkFlag(itemFlag::PROP_WALKABLE) == false)
        {
            World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable = false;
        }
    }

    //blocker 체크
    if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).blocker == false)
    {
        if (leadItem.checkFlag(itemFlag::PROP_BLOCKER) == true)
        {
            World::ins()->getTile(getGridX(), getGridY(), getGridZ()).blocker = true;
        }
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
    if (leadItem.checkFlag(itemFlag::TREE) && getGridX() == PlayerX() && getGridY() - 1 == PlayerY() && getGridZ() == PlayerZ())
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 100); //텍스쳐 투명도 설정
    }
    else
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
    }

    SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
    int sprIndex = leadItem.propSprIndex + leadItem.extraSprIndexSingle + 16 * leadItem.extraSprIndex16;
    if (leadItem.checkFlag(itemFlag::PLANT_SEASON_DEPENDENT))
    {
        if (World::ins()->getTile(getGridX(), getGridY(), PlayerZ()).hasSnow == true) sprIndex += 5;
        else
        {
            if (getSeason() == seasonFlag::summer) { sprIndex += 1; }
            else if (getSeason() == seasonFlag::autumn) { sprIndex += 2; }
            else if (getSeason() == seasonFlag::winter) { sprIndex += 3; }
        }
    }
    drawSpriteCenter
    (
        spr::propset,
        sprIndex,
        dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
        dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
    );
    SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
    setZoom(1.0);
};