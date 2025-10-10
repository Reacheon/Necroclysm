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

        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND) || leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            bool downConnected = false;
            Prop* downProp = TileProp(getGridX(), getGridY() + 1, getGridZ());
            if (downProp != nullptr && (downProp->leadItem.checkFlag(itemFlag::CABLE) || downProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_TOP))) downConnected = true;

            if (leadItem.itemCode == itemRefCode::cooperCable) drawSpriteCenter(spr::propset, 2993 + downConnected, drawX, drawY);
            if (leadItem.itemCode == itemRefCode::silverCable) drawSpriteCenter(spr::propset, 2993 + 16 + downConnected, drawX, drawY);
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND))
        {
            if (leadItem.itemCode == itemRefCode::cooperCable)
            {
                drawSpriteCenter(spr::propset, 2995, drawX, drawY);//상단으로 이어진 구리 케이블
            }
            else if (leadItem.itemCode == itemRefCode::silverCable)
            {
                drawSpriteCenter(spr::propset, 2995 + 16, drawX, drawY);//상단으로 이어진 은 케이블
            }
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            if (leadItem.itemCode == itemRefCode::cooperCable)
            {
                drawSpriteCenter(spr::propset, 2997, drawX, drawY);//하단으로 이어진 구리 케이블
            }
            else if (leadItem.itemCode == itemRefCode::silverCable)
            {
                drawSpriteCenter(spr::propset, 2997 + 16, drawX, drawY);//하단으로 이어진 은 케이블
            }
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