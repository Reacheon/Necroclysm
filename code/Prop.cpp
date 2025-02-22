#include <SDL.h>

import Prop;
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
import Player;
import Sticker;
import Particle;

Prop::Prop(int inputX, int inputY, int inputZ, int leadItemCode)
{
    leadItem = itemDex[leadItemCode];
    setAniPriority(3);
    //prt(L"[Prop:constructor] �����ڰ� ȣ��Ǿ���. ������ ��ǥ�� %d,%d,%d�̴�.\n", inputX, inputY, inputZ);
    setGrid(inputX, inputY, inputZ);

    errorBox(TileProp(inputX, inputY, inputZ) != nullptr, L"������ġ�� �̹� ��ġ���� �����Ѵ�!");
    World::ins()->getTile(inputX, inputY, inputZ).PropPtr = this;

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

    //HP ����
    leadItem.propHP = leadItem.propMaxHP;
    leadItem.propFakeHP = leadItem.propMaxHP;

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

                itemPtr1 = new ItemStack(itemPos.x, itemPos.y, itemPos.z, { {392,1} });

                for (int i = 0; i < 8; i++)
                {
                    new Particle(getX() + 16 + randomRange(-16, 16), getY() + randomRange(0, 8) , randomRange(0, 7), randomRangeFloat(-1.2,1.2), randomRangeFloat(-2.6,-3.2), 0.18, randomRange(25,35));
                }
            }
            else
            {
                if (1/*TileWall(getGridX() - 1, getGridY(), getGridZ()) == 0*/) itemPos = { getGridX() - 1, getGridY(), getGridZ() };
                else  itemPos = { getGridX(), getGridY(), getGridZ() };

                itemPtr1 = new ItemStack(itemPos.x, itemPos.y, itemPos.z, { {392,1} });

                for (int i = 0; i < 8; i++)
                {
                    new Particle(getX() - 16 + randomRange(-16, 16), getY() + randomRange(0, 8), randomRange(0, 7), randomRangeFloat(-1.2, 1.2), randomRangeFloat(-2.6, -3.2), 0.18, randomRange(25, 35));
                }
            }
            
            
            addAniUSetPlayer(itemPtr1, aniFlag::drop);
            //addAniUSetPlayer(itemPtr2, aniFlag::drop);
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

    if (leadItem.checkFlag(itemFlag::TREE))//������ ��� �׸���
    {
        drawSpriteCenter
        (
            spr::propset,
            sprIndex+8,
            dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
        );
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


    drawSpriteCenter
    (
        spr::propset,
        sprIndex,
        dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
        dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
    );

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



    if (displayHPBarCount > 0)//��ü HP ǥ��
    {
        int pivotX = dst.x + dst.w / 2 - (int)(8 * zoomScale);
        int pivotY = dst.y + dst.h / 2 + (int)(16 * zoomScale);
        SDL_Rect dst = { pivotX, pivotY, (int)(16 * zoomScale),(int)(3 * zoomScale) };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        drawFillRect(dst, col::black, alphaHPBar);


        if (leadItem.propFakeHP > leadItem.propHP) { leadItem.propFakeHP-=((float)leadItem.propMaxHP/100.0); }
        else if (leadItem.propFakeHP < leadItem.propHP) leadItem.propFakeHP = leadItem.propHP;

        if (leadItem.propFakeHP != leadItem.propHP)
        {
            if (alphaFakeHPBar > 20) { alphaFakeHPBar -= 20; }
            else 
            { 
                alphaFakeHPBar = 0;
                leadItem.propFakeHP = leadItem.propHP;
            }
        }
        else { alphaFakeHPBar = 255; }

        float ratioFakeHP = myMax((float)0.0, (leadItem.propFakeHP) / (float)(leadItem.propMaxHP));
        dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioFakeHP),(int)(1 * zoomScale) };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        drawFillRect(dst, col::white, alphaFakeHPBar);

        float ratioHP = myMax((float)0.0, (float)(leadItem.propHP) / (float)(leadItem.propMaxHP));
        dst = { pivotX + (int)(1.0 * zoomScale), pivotY + (int)(1.0 * zoomScale), (int)(14 * zoomScale * ratioHP),(int)(1 * zoomScale) };
        if (ratioHP > 0 && dst.w == 0) { dst.w = 1; }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        drawFillRect(dst, lowCol::green, alphaHPBar);

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
    }

    SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //�ؽ��� ���� ����
    setZoom(1.0);
};