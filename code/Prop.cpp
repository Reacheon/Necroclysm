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
    //prt(L"[Prop:constructor] �����ڰ� ȣ��Ǿ���. ������ ��ǥ�� %d,%d,%d�̴�.\n", inputX, inputY, inputZ);
    setGrid(inputX, inputY, inputZ);

    errorBox(TileProp(inputX, inputY, inputZ) != nullptr, L"������ġ�� �̹� ��ġ���� �����Ѵ�!");
    World::ins()->getTile(inputX, inputY, inputZ).PropPtr = this;
    updateTile();

    if (itemDex[leadItemCode].checkFlag(itemFlag::LIGHT_ON))
    {

        myLight = new Light(inputX + leadItem.lightDelX, inputY + leadItem.lightDelY, inputZ, leadItem.lightRange, leadItem.lightIntensity, { leadItem.lightR,leadItem.lightG,leadItem.lightB });//�ӽ÷� �̷��� ��������
        //Player::ins()->updateVision(Player::ins()->entityInfo.eyeSight);
    }


    deactivateAI();//������ �����ϰ� �⺻������ ��Ȱ��ȭ

    //�ֺ� Ÿ���� �м��� extraIndex ����
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
    prt(L"[Prop:destructor] �Ҹ��ڰ� ȣ��Ǿ���. \n");

    //�ֺ� Ÿ���� �м��� extraIndex ����
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



    if (leadItem.checkFlag(itemFlag::WIRE))//������ ���
    {
        auto wireCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::WIRE))//���� ������ ���
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
    else if (leadItem.checkFlag(itemFlag::PIPE))//����� ���
    {
        auto pipeCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::PIPE))//���� ������ ���
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
    else if (leadItem.checkFlag(itemFlag::CONVEYOR))//�����̾� ��Ʈ�� ���
    {
        auto conveyorCheck = [=](int dx, int dy) -> bool
            {
                Prop* tgtProp = TileProp(getGridX() + dx, getGridY() + dy, getGridZ());
                if (tgtProp != nullptr)
                {
                    if (tgtProp->leadItem.checkFlag(itemFlag::CONVEYOR))//���� ������ ���
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
    //walkable üũ
    if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable == true)
    {
        if (leadItem.checkFlag(itemFlag::PROP_WALKABLE) == false)
        {
            World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable = false;
        }
    }

    //blocker üũ
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
    //prt(L"[Prop:AI] ID : %p�� AI�� ������״�.\n", this);
    while (1)
    {

        //prt(L"[Prop:AI] ID : %p�� turnResource�� %f�Դϴ�.\n", this, getTurnResource());
        if (getTurnResource() >= 2.0)
        {
            clearTurnResource();
            addTurnResource(2.0);
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //���� ��� ���� ������ �������������� return true
        //prt(L"[Prop:AI] AI�� true�� ��ȯ�ߴ�. AI�� �����մϴ�.\n");
        return true;
    }
}

bool Prop::runAnimation(bool shutdown)
{
    //prt(L"Prop %p�� runAnimation�� ����Ǿ���.\n", this);
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
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 100); //�ؽ��� ���� ����
    }
    else
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //�ؽ��� ���� ����
    }

    SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //������ ����
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
    SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //�ؽ��� ���� ����
    setZoom(1.0);
};