#include <SDL.h>

export module Install;

import std;
import globalVar;
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

export class Install : public Ani, public AI, public Coord, public Drawable
{
public:
    ItemData leadItem;
    float timeResource = 0;
    Light* myLight = nullptr;

    Install(int inputX, int inputY, int inputZ, int leadItemCode)
    {
        leadItem = itemDex[leadItemCode];
        setAniPriority(3);
        prt(lowCol::orange, L"[Install:constructor] �����ڰ� ȣ��Ǿ���. ������ ��ǥ�� %d,%d,%d�̴�.\n", inputX, inputY, inputZ);
        setGrid(inputX, inputY, inputZ);

        errorBox(World::ins()->getTile(inputX, inputY, inputZ).InstallPtr != nullptr, L"������ġ�� �̹� ��ġ���� �����Ѵ�!");
        World::ins()->getTile(inputX, inputY, inputZ).InstallPtr = this;
        updateTile();

        if (itemDex[leadItemCode].checkFlag(itemFlag::LIGHT_ON))
        {

            myLight = new Light(inputX + leadItem.lightDelX, inputY + leadItem.lightDelY, inputZ, leadItem.lightRange, leadItem.lightIntensity, { leadItem.lightR,leadItem.lightG,leadItem.lightB });//�ӽ÷� �̷��� ��������
            //Player::ins()->updateVision(Player::ins()->getEyeSight());
        }

        deactivateAI();//������ �����ϰ� �⺻������ ��Ȱ��ȭ

        //�ֺ� Ÿ���� �м��� extraIndex ����
        int dx = 0;
        int dy = 0;
        for (int i = 0; i < 8; i++)
        {
            dir2Coord(i, dx, dy);
            Install* targetInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
            if (targetInstall != nullptr) targetInstall->updateSprIndex();
        }
        updateSprIndex();
    }

    ~Install()
    {
        delete myLight;
        prt(lowCol::orange, L"[Install:destructor] �Ҹ��ڰ� ȣ��Ǿ���. \n");

        //�ֺ� Ÿ���� �м��� extraIndex ����
        int dx = 0;
        int dy = 0;
        for (int i = 0; i < 8; i++)
        {
            dir2Coord(i, dx, dy);
            Install* targetInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
            if (targetInstall != nullptr) targetInstall->updateSprIndex();
        }
        updateSprIndex();
    }

    void updateSprIndex()
    {
        bool topTile = false;
        bool botTile = false;
        bool leftTile = false;
        bool rightTile = false;



        if (leadItem.checkFlag(itemFlag::WIRE))//������ ���
        {
            auto wireCheck = [=](int dx, int dy) -> bool
                {
                    Install* tgtInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
                    if (tgtInstall != nullptr)
                    {
                        if (tgtInstall->leadItem.checkFlag(itemFlag::WIRE))//���� ������ ���
                        {
                            return true;
                        }
                        else
                        {
                            if ((dx == 0 && dy == -1) && tgtInstall->leadItem.checkFlag(itemFlag::WIRE_CNCT_BOT)) return true;
                            else if ((dx == 0 && dy == 1) && tgtInstall->leadItem.checkFlag(itemFlag::WIRE_CNCT_TOP)) return true;
                            else if ((dx == 1 && dy == 0) && tgtInstall->leadItem.checkFlag(itemFlag::WIRE_CNCT_LEFT)) return true;
                            else if ((dx == -1 && dy == 0) && tgtInstall->leadItem.checkFlag(itemFlag::WIRE_CNCT_RIGHT)) return true;
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
                    Install* tgtInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
                    if (tgtInstall != nullptr)
                    {
                        if (tgtInstall->leadItem.checkFlag(itemFlag::PIPE))//���� ������ ���
                        {
                            return true;
                        }
                        else
                        {
                            if ((dx == 0 && dy == -1) && tgtInstall->leadItem.checkFlag(itemFlag::PIPE_CNCT_BOT)) return true;
                            else if ((dx == 0 && dy == 1) && tgtInstall->leadItem.checkFlag(itemFlag::PIPE_CNCT_TOP)) return true;
                            else if ((dx == 1 && dy == 0) && tgtInstall->leadItem.checkFlag(itemFlag::PIPE_CNCT_LEFT)) return true;
                            else if ((dx == -1 && dy == 0) && tgtInstall->leadItem.checkFlag(itemFlag::PIPE_CNCT_RIGHT)) return true;
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
                    Install* tgtInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
                    if (tgtInstall != nullptr)
                    {
                        if (tgtInstall->leadItem.checkFlag(itemFlag::CONVEYOR))//���� ������ ���
                        {
                            return true;
                        }
                        else
                        {
                            if ((dx == 0 && dy == -1) && tgtInstall->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_BOT)) return true;
                            else if ((dx == 0 && dy == 1) && tgtInstall->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_TOP)) return true;
                            else if ((dx == 1 && dy == 0) && tgtInstall->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_LEFT)) return true;
                            else if ((dx == -1 && dy == 0) && tgtInstall->leadItem.checkFlag(itemFlag::CONVEYOR_CNCT_RIGHT)) return true;
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
                    Install* tgtInstall = (Install*)World::ins()->getTile(getGridX() + dx, getGridY() + dy, getGridZ()).InstallPtr;
                    if (tgtInstall != nullptr)
                    {
                        if (tgtInstall->leadItem.tileConnectGroup != 0)
                        {
                            if (tgtInstall->leadItem.tileConnectGroup == leadItem.tileConnectGroup)
                            {
                                return true;
                            }
                            else return false;
                        }
                        else
                        {
                            if (tgtInstall->leadItem.itemCode == leadItem.itemCode)
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

    void updateTile()
    {
        World::ins()->getTile(getGridX(), getGridY(), getGridZ()).update();
        //walkable üũ
        if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable == true)
        {
            if (leadItem.checkFlag(itemFlag::INSTALL_WALKABLE) == false)
            {
                World::ins()->getTile(getGridX(), getGridY(), getGridZ()).walkable = false;
            }
        }

        //blocker üũ
        if (World::ins()->getTile(getGridX(), getGridY(), getGridZ()).blocker == false)
        {
            if (leadItem.checkFlag(itemFlag::INSTALL_BLOCKER) == true)
            {
                World::ins()->getTile(getGridX(), getGridY(), getGridZ()).blocker = true;
            }
        }
    }

    bool runAI()
    {
        //prt(lowCol::orange,L"[Install:AI] ID : %p�� AI�� ������״�.\n", this);
        while (1)
        {

            //prt(lowCol::orange, L"[Install:AI] ID : %p�� timeResource�� %f�Դϴ�.\n", this, getTimeResource());
            if (getTimeResource() >= 2.0)
            {
                clearTimeResource();
                addTimeResource(2.0);
            }

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //���� ��� ���� ������ �������������� return true
            //prt(lowCol::orange, L"[Install:AI] AI�� true�� ��ȯ�ߴ�. AI�� �����մϴ�.\n");
            return true;
        }
    }

    bool runAnimation(bool shutdown)
    {
        if (getAniType() == aniFlag::propRush)
        {
            addTimer();
        }
        return false;
    }

    void drawSelf() override
    {
        int tileSize = 16 * zoomScale;
        int bigShift = 16 * (leadItem.checkFlag(itemFlag::INSTALL_BIG));
        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * getGridX() + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * getGridY() + 8 - bigShift) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //�ؽ��� ���� ����
        SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //������ ����
        int sprIndex = leadItem.propSprIndex + leadItem.extraSprIndexSingle + 16 * leadItem.extraSprIndex16;
        if (leadItem.checkFlag(itemFlag::PLANT_SEASON_DEPENDENT))
        {
            if (getSeason() == seasonFlag::summer) { sprIndex += 1; }
            else if (getSeason() == seasonFlag::autumn) { sprIndex += 2; }
            else if (getSeason() == seasonFlag::winter) { sprIndex += 3; }
        }
        drawSpriteCenter
        (
            spr::propset,
            sprIndex,
            dst.x + dst.w / 2 + zoomScale * getFakeX(),
            dst.y + dst.h / 2 + zoomScale * getFakeY()
        );
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //�ؽ��� ���� ����
        setZoom(1.0);
    };
};

