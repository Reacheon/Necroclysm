#include <SDL3/SDL.h>

import Vehicle;
import std;
import globalVar;
import World;
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
    auto drawVehicleComponent = [=](Vehicle* vPtr, int tgtX, int tgtY, int tgtZ, int layer, int alpha)
        {
            SDL_Rect dst;
            dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
            dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
            dst.w = tileSize;
            dst.h = tileSize;

            setZoom(zoomScale);
            SDL_SetTextureAlphaMod(spr::vehset->getTexture(), alpha); //텍스쳐 투명도 설정
            SDL_SetTextureBlendMode(spr::vehset->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
            int sprIndex = vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].vehSprIndex + vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].extraSprIndex16;

            if (vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].itemCode == itemID::minecart)
            {
                if (bodyDir == dir16::dir0 || bodyDir == dir16::dir4) sprIndex += 0;
                else sprIndex += 1;
            }


            drawSpriteCenter
            (
                spr::vehset,
                sprIndex,
                dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
            );
            SDL_SetTextureAlphaMod(spr::vehset->getTexture(), 255); //텍스쳐 투명도 설정


            if (vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].pocketPtr != nullptr)
            {
                ItemPocket* pocketPtr = vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].pocketPtr.get();
                if (pocketPtr->itemInfo.size() > 0)
                {
                    drawSpriteCenter
                    (
                        spr::itemset,
                        pocketPtr->itemInfo[pocketPtr->itemInfo.size() - 1].getSprIndex(),
                        dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                        dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
                    );
                }
            }

            setZoom(1.0);
        };


    //천장 투명도: 플레이어가 이 차량 위에 있거나 LotEditor에서 호버 중이면 천장(과 그 위 부품들)을 반투명화
    int propCeilAlpha = 255;
    if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == this) propCeilAlpha = 50;
    if (lotEditorActive && lotEditorHoverVeh == (void*)this) propCeilAlpha = 50; //LotEditor: 호버한 차량 천장 반투명

    for (const auto& [pos, pocket] : this->partInfo)
    {
        if (pos.z != PlayerZ()) continue; // 다른 z 파츠는 플레이어 시야에 안 보임

        const std::vector<ItemData>& items = pocket->itemInfo;

        //이 타일의 천장(첫 VEH_ROOF) 인덱스. 천장 자신부터 그 위 부품 전부가 투명화 대상이 된다(천장 없으면 대상 없음)
        int ceilStart = (int)items.size();
        for (int i = 0; i < (int)items.size(); i++)
        {
            if (items[i].checkFlag(itemFlag::VEH_ROOF)) { ceilStart = i; break; }
        }

        //벡터(=설치) 순서대로 단일 패스 렌더. 아래->위로 그려지므로 나중에 설치한 부품이 위에 온다
        for (int layer = 0; layer < (int)items.size(); layer++)
        {
            //헬기로터는 스프라이트가 인접 타일을 덮으므로 전역 마지막 패스로 미룬다
            if (items[layer].itemCode == itemID::helicopterRotor)
            {
                rotorList.push_back({ pos.x, pos.y });
                continue;
            }

            const int alpha = (layer >= ceilStart) ? propCeilAlpha : 255;
            drawVehicleComponent(this, pos.x, pos.y, pos.z, layer, alpha);
        }
    }

    for (int i = 0; i < rotorList.size(); i++)
    {
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
