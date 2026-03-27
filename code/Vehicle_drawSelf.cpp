#include <SDL3/SDL.h>

import Vehicle;
import std;
import globalVar;
import wrapFunc;
import textureVar;
import constVar;
import util;
import ItemPocket;
import ItemData;
import Player;
import Coord;
import Drawable;
import drawSprite;

void Vehicle::drawSelf()
{
    std::vector<Point2> rotorList;
    int tileSize = 16 * zoomScale;
    auto drawVehicleComponent = [=](Vehicle* vPtr, int tgtX, int tgtY, int layer, int alpha)
        {
            SDL_Rect dst;
            dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
            dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
            dst.w = tileSize;
            dst.h = tileSize;

            setZoom(zoomScale);
            SDL_SetTextureAlphaMod(spr::propset->getTexture(), alpha); //텍스쳐 투명도 설정
            SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
            int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;

            if (vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].itemCode == itemID::minecart)
            {
                if (bodyDir == dir16::dir0 || bodyDir == dir16::dir4) sprIndex += 0;
                else sprIndex += 1;
            }


            drawSpriteCenter
            (
                spr::propset,
                sprIndex,
                dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
            );
            SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255); //텍스쳐 투명도 설정


            if (vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].pocketPtr != nullptr)
            {
                ItemPocket* pocketPtr = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].pocketPtr.get();
                if (pocketPtr->itemInfo.size() > 0)
                {
                    drawSpriteCenter
                    (
                        spr::itemset,
                        getItemSprIndex(pocketPtr->itemInfo[pocketPtr->itemInfo.size() - 1]),
                        dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                        dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
                    );
                }
            }

            setZoom(1.0);
        };


    for (const auto& [pos, pocket] : this->partInfo)
    {
        ////////////////////////////////일반 차량부품/////////////////////////////////////////////////
        for (int layer = 0; layer < pocket->itemInfo.size(); layer++)
        {
            //바닥프롭,천장프롭 플래그가 없는 일반 프롭일 경우
            if (!pocket->itemInfo[layer].checkFlag(itemFlag::VEH_ROOF))
            {
                drawVehicleComponent(this, pos.x, pos.y, layer, 255);
            }
        }

        ////////////////////////////////천장 차량부품////////////////////////////////////////////////////
        int propCeilAlpha = 255;
        if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == this) propCeilAlpha = 50;

        for (int layer = 0; layer < pocket->itemInfo.size(); layer++)
        {
            if (pocket->itemInfo[layer].checkFlag(itemFlag::VEH_ROOF))
            {
                if (pocket->itemInfo[layer].itemCode == 314)
                {
                    rotorList.push_back({ pos.x, pos.y });
                }
                else drawVehicleComponent(this, pos.x, pos.y, layer, propCeilAlpha);
            }
        }
    }

    for (int i = 0; i < rotorList.size(); i++)
    {
        int propCeilAlpha = 255;
        if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == this) propCeilAlpha = 50;

        int tgtX = rotorList[i].x;
        int tgtY = rotorList[i].y;
        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), propCeilAlpha); //텍스쳐 투명도 설정
        SDL_SetTextureBlendMode(spr::mainRotor->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
        //int sprIndex = vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].propSprIndex + vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY}]->itemInfo[layer].extraSprIndex16;
        drawSpriteCenter
        (
            spr::mainRotor,
            0,
            dst.x + dst.w / 2 + zoomScale * getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * getIntegerFakeY()
        );
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 255); //텍스쳐 투명도 설정
        setZoom(1.0);
    }
};
