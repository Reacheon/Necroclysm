import Prop;

#include <SDL3/SDL.h>

import util;
import globalVar;
import constVar;
import textureVar;
import wrapVar;
import drawSprite;
import World;
import globalTime;

import drawText;


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
            sprIndex + 8,
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

        bool isRightCable = rightProp != nullptr && (rightProp->leadItem.checkFlag(itemFlag::CABLE)||rightProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_LEFT));
        bool isUpCable = topProp != nullptr && (topProp->leadItem.checkFlag(itemFlag::CABLE) || topProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_DOWN));
        bool isLeftCable = leftProp != nullptr && (leftProp->leadItem.checkFlag(itemFlag::CABLE) || leftProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_RIGHT));
        bool isDownCable = botProp != nullptr && (botProp->leadItem.checkFlag(itemFlag::CABLE) || botProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_UP));

        if (isRightCable || isUpCable || isLeftCable || isDownCable)
        {
            int cableSprIndex = 2720;

            if (isRightCable && !isUpCable && !isLeftCable && !isDownCable) cableSprIndex = 3036;
            else if (!isRightCable && isUpCable && !isLeftCable && !isDownCable) cableSprIndex = 3037;
            else if (!isRightCable && !isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3034;
            else if (!isRightCable && !isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3035;

            else if (isRightCable && isUpCable && !isLeftCable && !isDownCable) cableSprIndex = 3033;
            else if (isRightCable && !isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3240;
            else if (isRightCable && !isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3029;
            else if (!isRightCable && isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3033;
            else if (!isRightCable && isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3243;
            else if (!isRightCable && !isUpCable && isLeftCable && isDownCable) cableSprIndex = 3027;

            else if (isRightCable && isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3032;
            else if (isRightCable && isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3030;
            else if (isRightCable && !isUpCable && isLeftCable && isDownCable) cableSprIndex = 3028;
            else if (!isRightCable && isUpCable && isLeftCable && isDownCable) cableSprIndex = 3026;

            else if (isRightCable && isUpCable && isLeftCable && isDownCable) cableSprIndex = 3024;


            drawSpriteCenter
            (
                spr::propset,
                cableSprIndex,
                dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
            );


            if (nodeInputElectron > 0)
            {
                int drawX = dst.x + dst.w / 2 + zoomScale * getIntegerFakeX();
                int drawY = dst.y + dst.h / 2 + zoomScale * getIntegerFakeY();

                bool rConnected = isConnected(this, dir16::right);
                bool uConnected = isConnected(this, dir16::up);
                bool lConnected = isConnected(this, dir16::left);
                bool dConnected = isConnected(this, dir16::down);

                if (rConnected)
                {
                    Prop* rProp = TileProp({ getGridX() + 1,getGridY(),getGridZ() });
                    if (rProp->nodeInputElectron > 0 || rProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 3041, drawX, drawY);
                }

                if (uConnected)
                {
                    Prop* uProp = TileProp({ getGridX(),getGridY() - 1,getGridZ() });
                    if (uProp->nodeInputElectron > 0 || uProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 3042, drawX, drawY);
                }

                if (lConnected)
                {
                    Prop* lProp = TileProp({ getGridX() - 1,getGridY(),getGridZ() });
                    if (lProp->nodeInputElectron > 0 || lProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 3043, drawX, drawY);
                }

                if (dConnected)
                {
                    Prop* dProp = TileProp({ getGridX(),getGridY() + 1,getGridZ() });
                    if (dProp->nodeInputElectron > 0 || dProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 3044, drawX, drawY);
                }
            }
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
        sprIndex += 7;
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


    if (leadItem.itemCode == itemRefCode::leverRL)
    {
        bool rConnected = isConnected(this, dir16::right);
        bool uConnected = isConnected(this, dir16::up);
        bool lConnected = isConnected(this, dir16::left);
        bool dConnected = isConnected(this, dir16::down);


        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (nodeInputElectron > 0 || nodeOutputElectron > 0) sprIndex += 4;
            else sprIndex += 3;
        }
        else
        {
            Prop* rProp = TileProp({ getGridX() + 1,getGridY(),getGridZ() });
            Prop* lProp = TileProp({ getGridX() - 1,getGridY(),getGridZ() });

            if (rProp != nullptr && rConnected && rProp->nodeOutputElectron > 0)
            {
                sprIndex += 1;
            }
            else if (lProp != nullptr && lConnected && lProp->nodeOutputElectron > 0)
            {
                sprIndex += 2;
            }
            else sprIndex += 0;
        }
    }
    else if (leadItem.itemCode == itemRefCode::leverUD)
    {
        bool rConnected = isConnected(this, dir16::right);
        bool uConnected = isConnected(this, dir16::up);
        bool lConnected = isConnected(this, dir16::left);
        bool dConnected = isConnected(this, dir16::down);

        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (nodeInputElectron > 0 || nodeOutputElectron > 0) sprIndex += 4;
            else sprIndex += 3;
        }
        else
        {
            Prop* uProp = TileProp({ getGridX(),getGridY() - 1,getGridZ() });
            Prop* dProp = TileProp({ getGridX(),getGridY() + 1,getGridZ() });
            if (uProp != nullptr && uConnected && uProp->nodeOutputElectron > 0)
            {
                sprIndex += 1;
            }
            else if (dProp != nullptr && dConnected && dProp->nodeOutputElectron > 0)
            {
                sprIndex += 2;
            }
            else sprIndex += 0;
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
        SDL_Point pt = { 24.0 * zoomScale,40.0 * zoomScale };
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

    if (leadItem.checkFlag(itemFlag::CABLE))
    {
        setFlip((getGridZ() % 2 != 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

        int drawX = dst.x + dst.w / 2 + zoomScale * getIntegerFakeX();
        int drawY = dst.y + dst.h / 2 + zoomScale * getIntegerFakeY();

        bool rConnected = isConnected(this, dir16::right);
        bool uConnected = isConnected(this, dir16::up);
        bool lConnected = isConnected(this, dir16::left);
        bool dConnected = isConnected(this, dir16::down);

        if (nodeInputElectron > 0)
        {
            if (rConnected && lConnected && !uConnected && !dConnected) //─
            {
                drawSpriteCenter(spr::propset, 2949, drawX, drawY);
            }
            else if (!rConnected && !lConnected && uConnected && dConnected) //│
            {
                drawSpriteCenter(spr::propset, 2950, drawX, drawY);
            }
            else
            {
                drawSpriteCenter(spr::propset, 2944, drawX, drawY);

                if (rConnected)
                {
                    Prop* rProp = TileProp({ getGridX() + 1,getGridY(),getGridZ() });
                    if (rProp->nodeInputElectron > 0 || rProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 2945, drawX, drawY);
                }
                
                if (uConnected)
                {
                    Prop* uProp = TileProp({ getGridX(),getGridY() - 1,getGridZ() });
                    if (uProp->nodeInputElectron > 0 || uProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 2946, drawX, drawY);
                }

                if(lConnected)
                {
                    Prop* lProp = TileProp({ getGridX() - 1,getGridY(),getGridZ() });
                    if (lProp->nodeInputElectron > 0 || lProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 2947, drawX, drawY);
                }

                if(dConnected)
                {
                    Prop* dProp = TileProp({ getGridX(),getGridY() + 1,getGridZ() });
                    if (dProp->nodeInputElectron > 0 || dProp->nodeOutputElectron > 0)
                        drawSpriteCenter(spr::propset, 2948, drawX, drawY);
                }
            }
        }



        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND) || leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            bool downConnected = false;
            Prop* downProp = TileProp(getGridX(), getGridY() + 1, getGridZ());
            if (downProp != nullptr && (downProp->leadItem.checkFlag(itemFlag::CABLE) || downProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_UP))) downConnected = true;

            if (leadItem.itemCode == itemRefCode::copperCable) drawSpriteCenter(spr::propset, 2993 + downConnected, drawX, drawY);
            if (leadItem.itemCode == itemRefCode::silverCable) drawSpriteCenter(spr::propset, 2993 + 16 + downConnected, drawX, drawY);

            if (nodeInputElectron > 0) drawSpriteCenter(spr::propset, 2944, drawX, drawY);
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND))
        {
            if (leadItem.itemCode == itemRefCode::copperCable)
            {
                drawSpriteCenter(spr::propset, 2995, drawX, drawY);//상단으로 이어진 구리 케이블
            }
            else if (leadItem.itemCode == itemRefCode::silverCable)
            {
                drawSpriteCenter(spr::propset, 2995 + 16, drawX, drawY);//상단으로 이어진 은 케이블
            }

            if (nodeInputElectron > 0) drawSpriteCenter(spr::propset, 2951, drawX, drawY);
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            if (leadItem.itemCode == itemRefCode::copperCable)
            {
                drawSpriteCenter(spr::propset, 2997, drawX, drawY);//하단으로 이어진 구리 케이블
            }
            else if (leadItem.itemCode == itemRefCode::silverCable)
            {
                drawSpriteCenter(spr::propset, 2997 + 16, drawX, drawY);//하단으로 이어진 은 케이블
            }

            if (nodeInputElectron > 0) drawSpriteCenter(spr::propset, 2952, drawX, drawY);
        }

        setFlip(SDL_FLIP_NONE);
    }




    if (leadItem.checkFlag(itemFlag::VOLTAGE_SOURCE) == false)
    {
        for (int i = 0; i < voltageDir.size(); i++)
        {
            int arrowSprIndex = 2953;
            if (voltageDir[i] == dir16::right) arrowSprIndex += 0;
            else if (voltageDir[i] == dir16::up) arrowSprIndex += 1;
            else if (voltageDir[i] == dir16::left) arrowSprIndex += 2;
            else if (voltageDir[i] == dir16::down) arrowSprIndex += 3;
            else if (voltageDir[i] == dir16::ascend) arrowSprIndex += 4;
            else if (voltageDir[i] == dir16::descend) arrowSprIndex += 5;

            SDL_SetTextureAlphaMod(spr::propset->getTexture(), 130);
            drawSpriteCenter
            (
                spr::propset,
                arrowSprIndex,
                dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
            );
            SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
        }
    }

    //if (leadItem.checkFlag(itemFlag::CIRCUIT))
    //{

    //    setFontSize(9);
    //    renderTextOutlineCenter(std::to_wstring(nodeElectron) + L"/" + std::to_wstring(nodeMaxElectron),
    //        dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
    //        dst.y + dst.h / 2 - (int)(12 * zoomScale) + zoomScale * getIntegerFakeY(),
    //        col::white);
    //}

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