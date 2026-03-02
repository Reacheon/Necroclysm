import Prop;

#include <SDL3/SDL.h>

import util;
import globalVar;
import constVar;
import textureVar;
import wrapFunc;
import drawSprite;
import World;
import globalTime;

import drawText;
import ContextMenu;


void Prop::drawSelf()
{
    constexpr Uint8 HIDE_WIRE_ALPHA = 120;
    constexpr int SHOW_WIRE_HOVER_TIME = 120;

    int iCode = leadItem.itemCode;

    int tileSize = 16 * zoomScale;
    int bigShift = 16 * (leadItem.checkFlag(itemFlag::PROP_BIG));
    SDL_Rect dst;
    dst.x = cameraW / 2 + zoomScale * ((16 * getGridX() + 8) - cameraX) - ((16 * zoomScale) / 2);
    dst.y = cameraH / 2 + zoomScale * ((16 * getGridY() + 8 - bigShift) - cameraY) - ((16 * zoomScale) / 2);
    dst.w = tileSize;
    dst.h = tileSize;

    int drawX = dst.x + dst.w / 2 + zoomScale * getIntegerFakeX();
    int drawY = dst.y + dst.h / 2 + zoomScale * getIntegerFakeY();


    static bool showAllHideWire = false;
    static Point2 prevHoverGrid = { std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };
    static int hoverTime = 0;
    static Uint32 lastFrameTime = 0;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime != lastFrameTime)
    {
        lastFrameTime = currentTime;

        //Point2 currentHoverGrid = getAbsMouseGrid();

        //Prop* hoverProp = TileProp(currentHoverGrid.x, currentHoverGrid.y, PlayerZ());
        //bool isHoveringHiddenWire = (hoverProp != nullptr
        //    && hoverProp->leadItem.checkFlag(itemFlag::CIRCUIT)
        //    && hoverProp->leadItem.checkFlag(itemFlag::HIDE_WIRE));

        //if (!isHoveringHiddenWire)
        //{
        //    hoverTime = 0;
        //    showAllHideWire = false;
        //}
        //else
        //{
        //    hoverTime += 1;
        //    if (hoverTime > SHOW_WIRE_HOVER_TIME) showAllHideWire = true;
        //}


        //prevHoverGrid = currentHoverGrid;

        //▲현재는 이렇게 주석 처리 해놨는데 마우스 오래 올려두면 보이게 하려면 하단1줄(▼) 제거하고 위에 주석처리한 코드들 전부 해제(일단 좀 지켜보자)
        showAllHideWire = false;

        if (ContextMenu::ins() != nullptr)
        {
            Prop* contextProp = TileProp(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
            if (contextProp != nullptr && contextProp->leadItem.checkFlag(itemFlag::CIRCUIT) && contextProp->leadItem.checkFlag(itemFlag::HIDE_WIRE))
            {
                showAllHideWire = true;
            }
        }
    }





    setZoom(zoomScale);
    if (leadItem.checkFlag(itemFlag::TREE) && getGridX() == PlayerX() && getGridY() - 1 == PlayerY() && getGridZ() == PlayerZ() && !leadItem.checkFlag(itemFlag::STUMP))
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 100); //텍스쳐 투명도 설정
    }
    else
    {
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정
    }



    if (leadItem.checkFlag(itemFlag::CIRCUIT) && leadItem.checkFlag(itemFlag::HIDE_WIRE) && leadItem.checkFlag(itemFlag::CABLE))
    {
        if (showAllHideWire) SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
        else return;

    }

    SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
    int sprIndex = leadItem.propSprIndex + leadItem.extraSprIndexSingle + 16 * leadItem.extraSprIndex16;

    if (leadItem.checkFlag(itemFlag::TREE))//나무일 경우 그림자
    {
        drawSpriteCenter
        (
            spr::propset,
            sprIndex + 8,
            drawX,
            drawY
        );
    }




    if (leadItem.checkFlag(itemFlag::CABLE_BEHIND))
    {


        Prop* rightProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
        Prop* topProp = TileProp(getGridX(), getGridY() - 1, getGridZ());
        Prop* lProp = TileProp(getGridX() - 1, getGridY(), getGridZ());
        Prop* dProp = TileProp(getGridX(), getGridY() + 1, getGridZ());

        bool isRightCable = rightProp != nullptr && (rightProp->leadItem.checkFlag(itemFlag::CABLE) || rightProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_LEFT));
        bool isUpCable = topProp != nullptr && (topProp->leadItem.checkFlag(itemFlag::CABLE) || topProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_DOWN));
        bool isLeftCable = lProp != nullptr && (lProp->leadItem.checkFlag(itemFlag::CABLE) || lProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_RIGHT));
        bool isDownCable = dProp != nullptr && (dProp->leadItem.checkFlag(itemFlag::CABLE) || dProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_UP));

        if (isRightCable || isUpCable || isLeftCable || isDownCable)
        {
            int cableSprIndex = 2720;

            if (isRightCable && !isUpCable && !isLeftCable && !isDownCable) cableSprIndex = 3036; //←
            else if (!isRightCable && isUpCable && !isLeftCable && !isDownCable) cableSprIndex = 3037; //↓
            else if (!isRightCable && !isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3034; //→
            else if (!isRightCable && !isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3035; //↑

            else if (isRightCable && isUpCable && !isLeftCable && !isDownCable) cableSprIndex = 3033; // └
            else if (isRightCable && !isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3047; // ─
            else if (isRightCable && !isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3029; // ┌
            else if (!isRightCable && isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3033; // ┘
            else if (!isRightCable && isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3048; // │ 
            else if (!isRightCable && !isUpCable && isLeftCable && isDownCable) cableSprIndex = 3027; // ┐

            else if (isRightCable && isUpCable && isLeftCable && !isDownCable) cableSprIndex = 3032; // ┴
            else if (isRightCable && isUpCable && !isLeftCable && isDownCable) cableSprIndex = 3030; // ├
            else if (isRightCable && !isUpCable && isLeftCable && isDownCable) cableSprIndex = 3028; // ┬
            else if (!isRightCable && isUpCable && isLeftCable && isDownCable) cableSprIndex = 3026; // ┤

            else if (isRightCable && isUpCable && isLeftCable && isDownCable) cableSprIndex = 3024; // ┼


            if (leadItem.checkFlag(itemFlag::HIDE_WIRE) && showAllHideWire == false)
            {
            }
            else
            {
                if (leadItem.checkFlag(itemFlag::HIDE_WIRE) && showAllHideWire == true) SDL_SetTextureAlphaMod(spr::propset->getTexture(), 150);

                drawSpriteCenter
                (
                    spr::propset,
                    cableSprIndex,
                    drawX,
                    drawY
                );


                if (isChargeFlowing())
                {
                    if (chargeFlux[dir16::right] != 0) drawSpriteCenter(spr::propset, 3041, drawX, drawY);; // 오른쪽 방향에서 현재 GND로 전력이 들어옴을 적색선으로 표기(전력흐름 ←)
                    if (chargeFlux[dir16::up] != 0) drawSpriteCenter(spr::propset, 3042, drawX, drawY);;  // 위쪽 방향에서 현재 GND로 전력이 들어옴을 적색선으로 표기(전력흐름 ↓)
                    if (chargeFlux[dir16::left] != 0) drawSpriteCenter(spr::propset, 3043, drawX, drawY);; // 왼쪽 방향에서 현재 GND로 전력이 들어옴을 적색선으로 표기(전력흐름 →)
                    if (chargeFlux[dir16::down] != 0) drawSpriteCenter(spr::propset, 3044, drawX, drawY);; // 아래쪽 방향에서 현재 GND로 전력이 들어옴을 적색선으로 표기(전력흐름 ↑)
                }

                SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
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
    else if (iCode == itemID::gasolineGeneratorR ||
        iCode == itemID::gasolineGeneratorT ||
        iCode == itemID::gasolineGeneratorL ||
        iCode == itemID::gasolineGeneratorB)
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


    if (iCode == itemID::leverRL || iCode == itemID::leverUD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (isChargeFlowing()) sprIndex += 2;
            else sprIndex += 1;

        }

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire)
            {
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
                drawSpriteCenter(spr::propset, sprIndex, drawX, drawY);
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
            }
            sprIndex += 16;
        }
    }
    else if (iCode == itemID::tactSwitchRL || iCode == itemID::tactSwitchUD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (isChargeFlowing()) sprIndex += 2;
            else sprIndex += 1;
        }

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire)
            {
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
                drawSpriteCenter(spr::propset, sprIndex, drawX, drawY);
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
            }
            sprIndex += 16;
        }
    }
    else if (iCode == itemID::pressureSwitchRL || iCode == itemID::pressureSwitchUD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (isChargeFlowing()) sprIndex += 2;
            else sprIndex += 1;
        }

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire)
            {
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
                drawSpriteCenter(spr::propset, sprIndex, drawX, drawY);
                SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
            }
            sprIndex += 16;
        }
    }
    else if (iCode == itemID::transistorR || iCode == itemID::transistorU || iCode == itemID::transistorL || iCode == itemID::transistorD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (iCode == itemID::transistorU || iCode == itemID::transistorD)
            {
                Prop* leftProp = TileProp(getGridX() - 1, getGridY(), getGridZ());
                Prop* rightProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
                if (leftProp != nullptr && leftProp->isChargeFlowing()) sprIndex += 2;
                else if (rightProp != nullptr && rightProp->isChargeFlowing()) sprIndex += 2;
                else sprIndex += 1;
            }
            else if (iCode == itemID::transistorR || iCode == itemID::transistorL)
            {
                Prop* upProp = TileProp(getGridX(), getGridY() - 1, getGridZ());
                Prop* downProp = TileProp(getGridX(), getGridY() + 1, getGridZ());
                if (upProp != nullptr && upProp->isChargeFlowing()) sprIndex += 2;
                else if (downProp != nullptr && downProp->isChargeFlowing()) sprIndex += 2;
                else sprIndex += 1;
            }
        }

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire) SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
            else return;
        }
    }
    else if (iCode == itemID::relayR || iCode == itemID::relayU || iCode == itemID::relayL || iCode == itemID::relayD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            if (iCode == itemID::relayU || iCode == itemID::relayD)
            {
                Prop* leftProp = TileProp(getGridX() - 1, getGridY(), getGridZ());
                Prop* rightProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
                if (leftProp != nullptr && leftProp->isChargeFlowing()) sprIndex += 2;
                else if (rightProp != nullptr && rightProp->isChargeFlowing()) sprIndex += 2;
                else sprIndex += 1;
            }
            else if (iCode == itemID::relayR || iCode == itemID::relayL)
            {
                Prop* upProp = TileProp(getGridX(), getGridY() - 1, getGridZ());
                Prop* downProp = TileProp(getGridX(), getGridY() + 1, getGridZ());
                if (upProp != nullptr && upProp->isChargeFlowing()) sprIndex += 2;
                else if (downProp != nullptr && downProp->isChargeFlowing()) sprIndex += 2;
                else sprIndex += 1;
            }
        }

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire) SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
            else return;
        }
    }
    else if (iCode == itemID::andGateR
        || iCode == itemID::andGateL
        || iCode == itemID::orGateR
        || iCode == itemID::orGateL
        || iCode == itemID::xorGateR
        || iCode == itemID::xorGateL
        || iCode == itemID::notGateR
        || iCode == itemID::notGateL
        || iCode == itemID::srLatchR
        || iCode == itemID::srLatchL
        )
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON)) sprIndex += 1;

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire) SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
            else return;
        }
    }
    else if (iCode == itemID::delayR || iCode == itemID::delayL)
    {
        if (delayMaxStack == 0) sprIndex = 3089;
        else sprIndex = 3089 + delayMaxStack;

        if (iCode == itemID::delayL) sprIndex += 16;

        if (leadItem.checkFlag(itemFlag::HIDE_WIRE))
        {
            if (showAllHideWire) SDL_SetTextureAlphaMod(spr::propset->getTexture(), HIDE_WIRE_ALPHA);
            else return;
        }
    }
    else if (iCode == itemID::valveRL || iCode == itemID::valveUD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            sprIndex += 1;
        }
    }
    else if (iCode == itemID::solenoidValveRL || iCode == itemID::solenoidValveUD)
    {
        if (leadItem.checkFlag(itemFlag::PROP_POWER_ON))
        {
            sprIndex += 1;
        }
        }


    if (leadItem.checkFlag(itemFlag::CABLE) && leadItem.checkFlag(itemFlag::CROSSED_CABLE))
    {
        bool flowH = (chargeFlux[dir16::right] != 0) || (chargeFlux[dir16::left] != 0);
        bool flowV = (chargeFlux[dir16::up] != 0) || (chargeFlux[dir16::down] != 0);

        int baseIndex = (iCode == itemID::silverCable) ? 3128 : 3124;
        int offset = 0;
        if (flowH && flowV) offset = 3;
        else if (flowH) offset = 1;
        else if (flowV) offset = 2;
        sprIndex = baseIndex + offset;
    }

    if (iCode == itemID::diodeR
        || iCode == itemID::diodeU
        || iCode == itemID::diodeL
        || iCode == itemID::diodeD)
    {
        if (isChargeFlowing())  sprIndex += 1;
    }

    if (iCode == itemID::powerBankR || iCode == itemID::powerBankL)
    {
        bool nowCharging = false;
        if (iCode == itemID::powerBankR && chargeFlux[dir16::left] > 0) nowCharging = true;
        else if (iCode == itemID::powerBankL && chargeFlux[dir16::right] > 0) nowCharging = true;

        double ratio = leadItem.powerStorage / static_cast<double>(leadItem.powerStorageMax);

        // 500ms 주기 점멸
        bool blinkOn = (SDL_GetTicks() / 500) % 2 == 0;

        if (nowCharging)
        {
            if (ratio < 0.01)
            {
                if (blinkOn) sprIndex += 1;
            }
            else if (ratio < 0.3333)
            {
                if (blinkOn) sprIndex += 1;
            }
            else if (ratio < 0.6666)
            {
                if (blinkOn) sprIndex += 2;
                else sprIndex += 1;
            }
            else if (ratio < 1.0 - 0.001)
            {
                if (blinkOn) sprIndex += 3;
                else sprIndex += 2;
            }
            else
            {
                sprIndex += 3;
            }
        }
        else
        {
            if (ratio < 0.01)
            {
            }
            else if (ratio < 0.3333)
            {
                sprIndex += 1; // 1칸 점등
            }
            else if (ratio < 0.6666)
            {
                sprIndex += 2; // 2칸 점등
            }
            else
            {
                sprIndex += 3; // 3칸 점등
            }
        }
    }

    if (iCode == itemID::chargingPort)
    {
        if (getInletCharge() > 0) sprIndex += 1;
    }

    if (iCode == itemID::fluidTank)
    {
        bool rConnected = false;
        bool lConnected = false;

        Prop* rProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
        Prop* lProp = TileProp(getGridX() - 1, getGridY(), getGridZ());

        if (rProp != nullptr)
        {
            if (rProp->leadItem.checkFlag(itemFlag::PIPE) || rProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_LEFT))
            {
                rConnected = true;
            }
        }

        if (lProp != nullptr)
        {
            if (lProp->leadItem.checkFlag(itemFlag::PIPE) || lProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_RIGHT))
            {
                lConnected = true;
            }
        }

        if (rConnected && lConnected) sprIndex += 3;
        else if (rConnected) sprIndex += 1;
        else if (lConnected) sprIndex += 2;
    }


    if (iCode == itemID::intakePipeR
        || iCode == itemID::verticalPipeRB
        || iCode == itemID::verticalPipeRA)
    {
        Prop* nextProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
        if(nextProp && (nextProp->leadItem.checkFlag(itemFlag::PIPE)||nextProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_LEFT))) 
            sprIndex += 1;
    }
    else if (iCode == itemID::intakePipeL
        || iCode == itemID::verticalPipeLB
        || iCode == itemID::verticalPipeLA)
    {
        Prop* nextProp = TileProp(getGridX() - 1, getGridY(), getGridZ());
        if (nextProp && (nextProp->leadItem.checkFlag(itemFlag::PIPE) || nextProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_RIGHT)))
            sprIndex += 1;
    }
    else if (iCode == itemID::intakePipeU)
    {
        Prop* nextProp = TileProp(getGridX(), getGridY() - 1, getGridZ());
        if (nextProp && (nextProp->leadItem.checkFlag(itemFlag::PIPE) || nextProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_DOWN)))
            sprIndex += 1;
    }
    else if (iCode == itemID::intakePipeD)
    {
        Prop* nextProp = TileProp(getGridX(), getGridY() + 1, getGridZ());
        if (nextProp && (nextProp->leadItem.checkFlag(itemFlag::PIPE) || nextProp->leadItem.checkFlag(itemFlag::PIPE_CNCT_UP)))
            sprIndex += 1;
    }

    if (iCode == itemID::intakePipeR
        || iCode == itemID::intakePipeU
        || iCode == itemID::intakePipeL
        || iCode == itemID::intakePipeD
        && (iCode != itemID::verticalPipeLA && iCode != itemID::verticalPipeLB
            && iCode != itemID::verticalPipeRA && iCode != itemID::verticalPipeRB))
    {
        int floorItemIndex = TileFloor(getGridX(), getGridY(), getGridZ());
        if(itemDex[floorItemIndex].checkFlag(itemFlag::WATER_SHALLOW) || itemDex[floorItemIndex].checkFlag(itemFlag::WATER_DEEP))
        {
            sprIndex += 8;
        }
    }

    if (iCode == itemID::pumpR
        || iCode == itemID::pumpU
        || iCode == itemID::pumpL
        || iCode == itemID::pumpD)
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
            sprIndex += (1 + animFrame);
        }
    }

    if (iCode != itemID::pipe && jetFluidType != fluidType::NONE)
    {
        static Uint32 jetLastUpdateTime = 0;
        static int jetAnimFrame = 0;
        Uint32 jetCurrentTime = SDL_GetTicks();
        if (jetCurrentTime - jetLastUpdateTime >= 100)
        {
            jetAnimFrame = (jetAnimFrame + 1);
            jetLastUpdateTime = jetCurrentTime;
        }

        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 150);

        if (jetFluidDir == dir16::right)
        {
            drawSpriteCenter(spr::propset, 3296 + (jetAnimFrame % 4), drawX, drawY);
        }
        else if (jetFluidDir == dir16::up)
        {
            drawSpriteCenter(spr::propset, 3300 + (jetAnimFrame % 3), drawX, drawY);
        }
        else if (jetFluidDir == dir16::left)
        {
            drawSpriteCenter(spr::propset, 3304 + (jetAnimFrame % 4), drawX, drawY);
        }
        else if (jetFluidDir == dir16::down)
        {
            drawSpriteCenter(spr::propset, 3308 + (jetAnimFrame % 3), drawX, drawY);
        }

        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
    }

    if (iCode == itemID::potatoCrop
        || iCode == itemID::wheatCrop
        || iCode == itemID::riceCrop
        || iCode == itemID::carrotCrop)
    {
        if (plantGrowthPercent >= 100.0) sprIndex += 4;
        else if (plantGrowthPercent >= 75) sprIndex += 3;
        else if (plantGrowthPercent >= 50) sprIndex += 2;
        else if (plantGrowthPercent >= 25) sprIndex += 1;
    }
    else if (iCode == itemID::tomatoCrop
        || iCode == itemID::watermelonCrop)
    {
        if (plantGrowthPercent >= 100.0) sprIndex += 5;
        else if (plantGrowthPercent >= 60) sprIndex += 4;
        else if (plantGrowthPercent >= 40) sprIndex += 3;
        else if (plantGrowthPercent >= 20) sprIndex += 2;
        else if (plantGrowthPercent >= 10) sprIndex += 1;
    }


    ///////////////////////////////////////////////////////////////////////////
    /////////////////////////////메인 그리기 함수//////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    drawSpriteCenter(spr::propset,sprIndex,drawX,drawY);
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    if (iCode == itemID::pipe && jetFluidType != fluidType::NONE)
    {
        static Uint32 jetLastUpdateTime = 0;
        static int jetAnimFrame = 0;
        Uint32 jetCurrentTime = SDL_GetTicks();
        if (jetCurrentTime - jetLastUpdateTime >= 100)
        {
            jetAnimFrame = (jetAnimFrame + 1);
            jetLastUpdateTime = jetCurrentTime;
        }

        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 150);

        if (jetFluidDir == dir16::right)
        {
            drawSpriteCenter(spr::propset, 3296 + (jetAnimFrame % 4), drawX, drawY);
        }
        else if (jetFluidDir == dir16::up)
        {
            drawSpriteCenter(spr::propset, 3300 + (jetAnimFrame % 3), drawX, drawY);
        }
        else if (jetFluidDir == dir16::left)
        {
            drawSpriteCenter(spr::propset, 3304 + (jetAnimFrame % 4), drawX, drawY);
        }
        else if (jetFluidDir == dir16::down)
        {
            drawSpriteCenter(spr::propset, 3308 + (jetAnimFrame % 3), drawX, drawY);
        }

        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
    }

    if (iCode == itemID::verticalPipeLA 
        || iCode == itemID::verticalPipeLB
        || iCode == itemID::verticalPipeRA 
        || iCode == itemID::verticalPipeRB)
    {
        static Uint32 jetLastUpdateTime = 0;
        static int jetAnimFrame = 0;
        Uint32 jetCurrentTime = SDL_GetTicks();
        if (jetCurrentTime - jetLastUpdateTime >= 100)
        {
            jetAnimFrame = (jetAnimFrame + 1);
            jetLastUpdateTime = jetCurrentTime;
        }

        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 150);

        if (jetFluidDir == dir16::right)
        {
            drawSpriteCenter(spr::propset, 3296 + (jetAnimFrame % 4), drawX + zoomScale*2, drawY);
        }
        else if (jetFluidDir == dir16::left)
        {
            drawSpriteCenter(spr::propset, 3304 + (jetAnimFrame % 4), drawX - zoomScale * 2, drawY);
        }

        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
    }

    //if (iCode == itemID::chargingPort)
    //{
    //    if (getInletCharge() > 0)
    //    {
    //        float pulseSpeed = 0.003f; // 펄스 속도 (작을수록 느림)
    //        float minBrightness = 0.7f; // 최소 밝기 (0.0~1.0)
    //        float maxBrightness = 1.0f; // 최대 밝기

    //        float pulse = (sin(SDL_GetTicks() * pulseSpeed) + 1.0f) * 0.5f; // 0.0~1.0 사이값
    //        float colorAlpha = minBrightness + (maxBrightness - minBrightness) * pulse;

    //        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    //        SDL_SetTextureColorMod(spr::propset->getTexture(),
    //            (Uint8)(255.0f * colorAlpha),
    //            (Uint8)(255.0f * colorAlpha),
    //            (Uint8)(255.0f * colorAlpha));

    //        drawSpriteCenter(spr::propset,2706,drawX,drawY);

    //        SDL_SetTextureColorMod(spr::propset->getTexture(), 255, 255, 255);



    //    }
    //}

    if (iCode == itemID::powerBankR)
    {
        if (chargeFlux[dir16::left] > 0) drawSpriteCenter(spr::propset,3159,drawX,drawY);

        if (chargeFlux[dir16::right] < 0) drawSpriteCenter(spr::propset, 3160, drawX, drawY);
    }
    else if (iCode == itemID::powerBankL)
    {
        if (chargeFlux[dir16::left] > 0) drawSpriteCenter(spr::propset, 3159 + 16, drawX, drawY);

        if (chargeFlux[dir16::right] < 0) drawSpriteCenter(spr::propset, 3160 + 16, drawX, drawY);
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
            drawX,
            drawY,
            treeAngle,
            &pt
        );
    }

    if (leadItem.checkFlag(itemFlag::CABLE) && leadItem.checkFlag(itemFlag::CROSSED_CABLE) == false)
    {
        Prop* rProp = TileProp(getGridX() + 1, getGridY(), getGridZ());
        Prop* uProp = TileProp(getGridX(), getGridY() - 1, getGridZ());
        Prop* lProp = TileProp(getGridX() - 1, getGridY(), getGridZ());
        Prop* dProp = TileProp(getGridX(), getGridY() + 1, getGridZ());

        if (getGridX() == 11 && getGridY() == -15)
        {
            int a = 3;
        }

        bool rConnected = rProp != nullptr && (rProp->leadItem.checkFlag(itemFlag::CABLE) || rProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_LEFT));
        bool uConnected = uProp != nullptr && (uProp->leadItem.checkFlag(itemFlag::CABLE) || uProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_DOWN));
        bool lConnected = lProp != nullptr && (lProp->leadItem.checkFlag(itemFlag::CABLE) || lProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_RIGHT));
        bool dConnected = dProp != nullptr && (dProp->leadItem.checkFlag(itemFlag::CABLE) || dProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_UP));

        if (isChargeFlowing())
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
                    if (rProp->isChargeFlowing())
                        drawSpriteCenter(spr::propset, 2945, drawX, drawY);
                }
                
                if (uConnected)
                {
                    if (uProp->isChargeFlowing())
                        drawSpriteCenter(spr::propset, 2946, drawX, drawY);
                }

                if(lConnected)
                {
                    if (lProp->isChargeFlowing())
                        drawSpriteCenter(spr::propset, 2947, drawX, drawY);
                }

                if(dConnected)
                {
                    if (dProp->isChargeFlowing())
                        drawSpriteCenter(spr::propset, 2948, drawX, drawY);
                }
            }
        }


        setFlip((getGridZ() % 2 != 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND) || leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            bool downConnected = false;
            Prop* downProp = TileProp(getGridX(), getGridY() + 1, getGridZ());
            if (downProp != nullptr && (downProp->leadItem.checkFlag(itemFlag::CABLE) || downProp->leadItem.checkFlag(itemFlag::CABLE_CNCT_UP))) downConnected = true;

            if (iCode == itemID::copperCable) drawSpriteCenter(spr::propset, 2993 + downConnected, drawX, drawY);
            if (iCode == itemID::silverCable) drawSpriteCenter(spr::propset, 2993 + 16 + downConnected, drawX, drawY);

            if (isChargeFlowing()) drawSpriteCenter(spr::propset, 2944, drawX, drawY);
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_ASCEND))
        {
            if (iCode == itemID::copperCable)
            {
                drawSpriteCenter(spr::propset, 2995, drawX, drawY);//상단으로 이어진 구리 케이블
            }
            else if (iCode == itemID::silverCable)
            {
                drawSpriteCenter(spr::propset, 2995 + 16, drawX, drawY);//상단으로 이어진 은 케이블
            }

            if (isChargeFlowing()) drawSpriteCenter(spr::propset, 2951, drawX, drawY);
        }

        if (leadItem.checkFlag(itemFlag::CABLE_Z_DESCEND))
        {
            if (iCode == itemID::copperCable)
            {
                drawSpriteCenter(spr::propset, 2997, drawX, drawY);//하단으로 이어진 구리 케이블
            }
            else if (iCode == itemID::silverCable)
            {
                drawSpriteCenter(spr::propset, 2997 + 16, drawX, drawY);//하단으로 이어진 은 케이블
            }

            if (isChargeFlowing()) drawSpriteCenter(spr::propset, 2952, drawX, drawY);
        }

        setFlip(SDL_FLIP_NONE);
    }

    //if (leadItem.checkFlag(itemFlag::CIRCUIT))
    //{

    //    setFontSize(9);
    //    renderTextOutlineCenter(std::to_wstring(nodeCharge) + L"/" + std::to_wstring(nodeMaxCharge),
    //        drawX,
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