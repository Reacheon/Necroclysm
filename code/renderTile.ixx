module;
#include <SDL3/SDL.h>

export module renderTile;

import std;
import util;
import globalVar;
import constVar;
import textureVar;
import checkCursor;
import ItemStack;
import Entity;
import World;
import Player;
import drawSprite;
import drawText;
import Damage;
import Corpse;
import Craft;
import Vehicle;
import GUI;
import Prop;
import globalTime;
import Drawable;
import TileData;
import Flame;
import HUD;
import Bullet;
import Particle;
import Footprint;
import Wave;
import Wake;
import nervedriveFilter;

SDL_Rect dst, renderRegion;
int tileSize, cameraGridX, cameraGridY, renderRangeW, renderRangeH, pZ;

Point2 vertices[MAX_BATCH];
int indices[MAX_BATCH];
SDL_Color rectColors[MAX_BATCH];
Uint8 batchAlphas[MAX_BATCH];

void analyseRender();
void drawTiles();
void drawCorpses();
void drawFloorProp();
void drawItems();
void drawEntities();
void drawDamages();
void drawBullets();
void drawParticles();
void drawSprinklerSpray();
void drawRampArrows();
void drawBridgeShadows();
void drawMulFogs();
void drawFogs();
void drawMarkers();
void drawDebug();


// 차량과 엔티티는 중복을 허용하면 안됨
std::vector<Point2> tileList, itemList, floorPropList, upperPropList, gasList, blackFogList, grayFogList, lightFogList, flameList, allTileList, mulFogList, wallHPList;
std::unordered_set<Point2, Point2::Hash> lightFogSet, shallowSeaWaves, deepSeaWaves, deepFreshWaves;
std::vector<Drawable*> renderVehList, renderEntityList;
std::unordered_set<Point2, Point2::Hash> raySet;
std::unordered_set<Point2, Point2::Hash> sprinklerSpraySet33;
std::unordered_set<Point2, Point2::Hash> sprinklerSpraySet55;
std::unordered_set<Point2, Point2::Hash> rampUpSet;
std::unordered_set<Point2, Point2::Hash> rampDownSet;
std::vector<Point2> bridgeShadowList;

export std::int64_t renderTile()
{
    std::int64_t timeStampStart = getNanoTimer();

    tileSize = 16 * zoomScale;
    cameraGridX = (cameraX - 8) / (16);
    cameraGridY = (cameraY - 8) / (16);
    renderRangeW = 3 + (cameraW + extraCameraLength) / tileSize;
    renderRangeH = 3 + (cameraH + extraCameraLength) / tileSize;
    pZ = PlayerZ();
    renderRegion = { cameraGridX - (renderRangeW / 2), cameraGridY - (renderRangeH / 2), renderRangeW, renderRangeH };

    tileList.clear();
    itemList.clear();
    floorPropList.clear();
    upperPropList.clear();
    renderVehList.clear();
    renderEntityList.clear();
    gasList.clear();
    blackFogList.clear();
    grayFogList.clear();
    lightFogList.clear();
    lightFogSet.clear();
    flameList.clear();
    mulFogList.clear();
    shallowSeaWaves.clear();
    deepSeaWaves.clear();
    wallHPList.clear();
    sprinklerSpraySet33.clear();
    sprinklerSpraySet55.clear();
    rampUpSet.clear();
    rampDownSet.clear();
    bridgeShadowList.clear();

    if (rangeRay)
    {
        
        raySet.clear();

        if (checkCursor(&tab) == false && checkCursor(&letterbox) == false)
        {
            if (option::inputMethod == input::mouse)
            {
                int revGridX = getAbsMouseGrid().x - PlayerX();
                int revGridY = getAbsMouseGrid().y - PlayerY();
                makeLine(raySet, getAbsMouseGrid().x - PlayerX(), getAbsMouseGrid().y - PlayerY());
                if (revGridX > 0) PlayerPtr->setDirection(0);
                else if (revGridX < 0) PlayerPtr->setDirection(4);
            }
        }
        
    }

    auto PROFILE = [](auto&& f) -> std::int64_t
        {
            const std::int64_t start = getNanoTimer();
            std::forward<decltype(f)>(f)();
            return getNanoTimer() - start;
        };

    dur::analysis = PROFILE([] { analyseRender(); });
    dur::tile = PROFILE([] { drawTiles(); });
    dur::corpse = PROFILE([] { drawCorpses(); });
    dur::floorProp = PROFILE([] { drawFloorProp(); });
    dur::item = PROFILE([] { drawItems(); });
    dur::entity = PROFILE([] { drawEntities(); });
    drawBridgeShadows(); // 다리 그림자 — 바닥/시체/프롭/아이템/엔티티 모두 위에 깔려서 같이 어두워짐
    dur::damage = PROFILE([] { drawDamages(); });
    dur::bullet = PROFILE([] { drawBullets(); });
    dur::particle = PROFILE([] { drawParticles(); });
    dur::sprinklerSpray = PROFILE([] { drawSprinklerSpray(); });
    drawRampArrows();
    dur::mulFog = PROFILE([] { drawMulFogs(); });
    dur::fog = PROFILE([] { drawFogs(); });
    dur::marker = PROFILE([] { drawMarkers(); });
    dur::debug = PROFILE([] { drawDebug(); });

    return (getNanoTimer() - timeStampStart);
}




void analyseRender()
{
    // 루프 분석
    for (int tgtY = renderRegion.y - 1; tgtY < renderRegion.y + renderRegion.h + 1; tgtY++)
    {
        for (int tgtX = renderRegion.x - 1; tgtX < renderRegion.x + renderRegion.w + 1; tgtX++)
        {
            TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, pZ);

            // 바깥쪽이면 타일캐시만 추가하고 거기에 있는 내용은 렌더링하지 않음
            if (tgtX < renderRegion.x || tgtX >= renderRegion.x + renderRegion.w) continue;
            if (tgtY < renderRegion.y || tgtY >= renderRegion.y + renderRegion.h) continue;

            if (lotEditorActive) thisTile->fov = fovFlag::white; //LotEditor: 카메라에 잡힌 전 영역을 밝혀 편집 가능하게

            // 바닥과 벽
            if (thisTile->fov != fovFlag::black)
            {
                tileList.push_back({ tgtX, tgtY });
                if (thisTile->wall != itemID::none && thisTile->displayHPBarCount>0)
                {
                    wallHPList.push_back({ tgtX, tgtY });
                }

                switch (thisTile->floor)
                {
                case itemID::shallowSeaWater:
                    for (int dir = 0; dir < 8; dir++)
                    {
                        int dx, dy;
                        dir2Coord(dir, dx, dy);
                        // 이웃이 로드 영역 밖이면 wave 등록 생략 — 어차피 그쪽 경계는 렌더 안 함.
                        const TileData* nb = World::ins()->tryGetTile(tgtX + dx, tgtY + dy, pZ);
                        if (nb && nb->floor != itemID::deepSeaWater)
                        {
                            shallowSeaWaves.insert({ tgtX + dx, tgtY + dy });
                        }
                    }
                    break;
                case itemID::deepSeaWater:
                    for (int dir = 0; dir < 8; dir++)
                    {
                        int dx, dy;
                        dir2Coord(dir, dx, dy);
                        const TileData* nb = World::ins()->tryGetTile(tgtX + dx, tgtY + dy, pZ);
                        if (nb && nb->floor != itemID::deepSeaWater)
                        {
                            deepSeaWaves.insert({ tgtX + dx, tgtY + dy });
                        }
                    }
                    break;
                case itemID::deepFreshWater:
                    for (int dir = 0; dir < 8; dir++)
                    {
                        int dx, dy;
                        dir2Coord(dir, dx, dy);
                        const TileData* nb = World::ins()->tryGetTile(tgtX + dx, tgtY + dy, pZ);
                        if (nb && nb->floor == itemID::shallowFreshWater)
                        {
                            deepFreshWaves.insert({ tgtX + dx, tgtY + dy });
                        }
                    }
                    break;
                }

            }

            // 바닥프롭
            Prop* fpPtr = thisTile->PropPtr.get();
            if (fpPtr != nullptr && fpPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER)) floorPropList.push_back({ tgtX, tgtY });

            // 아이템
            if (thisTile->fov == fovFlag::white && TileItemStack(tgtX, tgtY, pZ) != nullptr) itemList.push_back({ tgtX, tgtY });

            // 화염
            Flame* flamePtr = thisTile->flamePtr.get();
            if (flamePtr != nullptr) flameList.push_back({ tgtX, tgtY });

            // 차량
            Drawable* vPtr = (Drawable*)((Vehicle*)(thisTile->VehiclePtr));
            if (vPtr != nullptr) renderVehList.push_back(vPtr);

            // 플레이어와 겹치는 일반설치물
            Prop* pPtr = thisTile->PropPtr.get();
            if (pPtr != nullptr && pPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_LOWER) == false)
            {
                //열린 롤업도어 등 — 같은 타일의 엔티티/차량 위에 그려지는 프롭은 별도 리스트로 분리
                if (pPtr->leadItem.checkFlag(itemFlag::PROP_DEPTH_UPPER)) upperPropList.push_back({ tgtX, tgtY });
                else renderEntityList.push_back((Drawable*)pPtr);
            }

            // 일반 객체
            Drawable* ePtr = (Drawable*)(thisTile->EntityPtr.get());
            if (ePtr != nullptr) renderEntityList.push_back(ePtr);

            // 가스
            if (thisTile->gasVec.size() > 0) gasList.push_back({ tgtX, tgtY });

            //스프링클러 스프레이
            if (pPtr != nullptr && (pPtr->leadItem.itemCode == itemID::sprinklerRL || pPtr->leadItem.itemCode == itemID::sprinklerUD))
            {
                if (pPtr->leadItem.checkFlag(itemFlag::PROP_POWER_ON))
                {
                    if (pPtr->sinkFluidAmount >= pPtr->leadItem.fluidDemand) sprinklerSpraySet55.insert({ tgtX,tgtY });
                    else if (pPtr->sinkFluidAmount >= (double)(pPtr->leadItem.fluidDemand)/2.0) sprinklerSpraySet33.insert({ tgtX,tgtY });
                }
            }

            if (pPtr != nullptr)
            {
                if (pPtr->leadItem.checkFlag(itemFlag::RAMP_UP)) rampUpSet.insert({ tgtX, tgtY });
                else if (pPtr->leadItem.checkFlag(itemFlag::RAMP_DOWN)) rampDownSet.insert({ tgtX, tgtY });
            }

            // 다리 그림자: 위 z에 floor 있고 ramp 아닌 경우 (다리 밑 어두운 효과)
            // 단 이 타일에 광원 있으면 스킵 — 헤드라이트 등이 다리 밑 비추면 그림자 제거
            const TileData* aboveTile = World::ins()->tryGetTile(tgtX, tgtY, pZ + 1);
            if (aboveTile != nullptr && aboveTile->floor != itemID::none && thisTile->lightVec.size() == 0)
            {
                Prop* abovePropPtr = aboveTile->PropPtr.get();
                bool aboveIsRamp = abovePropPtr != nullptr &&
                    (abovePropPtr->leadItem.checkFlag(itemFlag::RAMP_UP) ||
                     abovePropPtr->leadItem.checkFlag(itemFlag::RAMP_DOWN));
                if (!aboveIsRamp) bridgeShadowList.push_back({ tgtX, tgtY });
            }

            // 안개
            if (thisTile->fov == fovFlag::black) blackFogList.push_back({ tgtX, tgtY });
            else
            {
                mulFogList.push_back({ tgtX,tgtY });

                if (thisTile->fov == fovFlag::gray) grayFogList.push_back({ tgtX, tgtY });
                else
                {
                    if (thisTile->lightVec.size() > 0)
                    {
                        lightFogList.push_back({ tgtX, tgtY });
                        lightFogSet.insert({ tgtX, tgtY });
                    }
                }
            }
        }
    }

    for (auto it = extraRenderVehList.begin(); it != extraRenderVehList.end(); it++)
    {
        int exVehSize = extraRenderVehList.size(); // 메모리 누수 체크용
        (void)exVehSize;
        renderVehList.push_back(*it);
    }
    for (auto it = extraRenderEntityList.begin(); it != extraRenderEntityList.end(); it++)
    {
        int exEntitySize = extraRenderEntityList.size(); // 메모리 누수 체크용
        (void)exEntitySize;
        renderEntityList.push_back(*it);
    }
}

// 창문은 16x16를 꽉 채우는 사각형이라, 인접한 벽이 창문을 벽으로 취급해 끊김 없이
// 이어 보이도록 벽 연결 판정에 포함한다. (창문은 prop으로 저장되어 tile->wall엔 안 잡힘)
static bool tileHasWindow(int x, int y, int z)
{
    Prop* p = TileProp(x, y, z);
    return p != nullptr && p->leadItem.checkFlag(itemFlag::WINDOW);
}

void drawTiles()
{
    SDL_Texture* tileTexture = spr::tileset->getTexture();
    int tileTextureW = spr::tileset->getW();
    int tileTextureH = spr::tileset->getH();

    
    int tileCounter = 0;

    //가시 타일이 많아 배치 버퍼(MAX_BATCH)를 넘기 직전이면 중간 플러시 후 카운터 리셋.
    //  (기존: 무경계 tileCounter++ -> vertices[] 오버플로우로 인접 static(파도 set 등) 손상 크래시.
    //   LotEditor 전체공개/저배율에서 가시 타일 급증해 노출됨. Map.ixx의 flush 패턴과 동일.)
    auto flushIfFull = [&]()
    {
        if (tileCounter >= MAX_BATCH - 64)
        {
            setZoom(zoomScale);
            drawSpriteBatchCenter(spr::tileset, vertices, indices, batchAlphas, tileCounter);
            tileCounter = 0;
        }
    };

    for (const auto& elem : tileList)
    {
        flushIfFull();
        int tgtX = elem.x;
        int tgtY = elem.y;
        const TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());
        const TileData* topTile = &World::ins()->getTile(tgtX, tgtY - 1, PlayerZ());
        const TileData* botTile = &World::ins()->getTile(tgtX, tgtY + 1, PlayerZ());
        const TileData* leftTile = &World::ins()->getTile(tgtX - 1, tgtY, PlayerZ());
        const TileData* rightTile = &World::ins()->getTile(tgtX + 1, tgtY, PlayerZ());

        setZoom(zoomScale);

        // 공허 캐스케이드: floor/wall 모두 비어 있으면 하늘색(sprite 506)을 베이스로 깔고
        // 그 위에 아래 z 비공허 층을 흐릿한 알파로 오버레이한다.
        // 다리 위·옥상 가장자리에서 하늘색 바탕에 아래 풍경이 살짝 비치는 효과.
        const bool isVoidHere = (thisTile->floor == itemID::none && thisTile->wall == itemID::none);
        if (isVoidHere)
        {
            const int baseVx = cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX));
            const int baseVy = cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY));

            // 베이스: 공허(하늘색) 풀 알파
            vertices[tileCounter] = { baseVx, baseVy };
            indices[tileCounter] = 506;
            batchAlphas[tileCounter] = 255;
            tileCounter++;

            constexpr int MAX_DEPTH     = 3;
            constexpr int OVERLAY_ALPHA = 80;   // 첫(d=1) 층 오버레이 알파
            constexpr int FADE_PER_Z    = 25;   // 한 층 더 내려갈 때마다 추가 감쇠

            for (int d = 1; d <= MAX_DEPTH; ++d)
            {
                const TileData* under = World::ins()->tryGetTile(tgtX, tgtY, pZ - d);
                if (under == nullptr) break;
                if (under->floor == itemID::none && under->wall == itemID::none) continue; // 계속 공허 - 더 내려감

                int alphaInt = OVERLAY_ALPHA - (d - 1) * FADE_PER_Z;
                if (alphaInt <= 0) break;

                const TileData* uTop   = World::ins()->tryGetTile(tgtX,     tgtY - 1, pZ - d);
                const TileData* uBot   = World::ins()->tryGetTile(tgtX,     tgtY + 1, pZ - d);
                const TileData* uLeft  = World::ins()->tryGetTile(tgtX - 1, tgtY,     pZ - d);
                const TileData* uRight = World::ins()->tryGetTile(tgtX + 1, tgtY,     pZ - d);
                const Uint8 alpha = static_cast<Uint8>(alphaInt);

                // 아래층 floor
                if (under->floor != itemID::none)
                {
                    int uDir = 0;
                    int uAni16 = 0;
                    int uAniSingle = 0;
                    if (itemDex[under->floor].tileConnectGroup != -1)
                    {
                        bool tC, bC, lC, rC;
                        if (itemDex[under->floor].tileConnectGroup == 0)
                        {
                            tC = (uTop   != nullptr) && (under->floor == uTop->floor);
                            bC = (uBot   != nullptr) && (under->floor == uBot->floor);
                            lC = (uLeft  != nullptr) && (under->floor == uLeft->floor);
                            rC = (uRight != nullptr) && (under->floor == uRight->floor);
                        }
                        else
                        {
                            int g = itemDex[under->floor].tileConnectGroup;
                            tC = (uTop   != nullptr) && (g == itemDex[uTop->floor].tileConnectGroup);
                            bC = (uBot   != nullptr) && (g == itemDex[uBot->floor].tileConnectGroup);
                            lC = (uLeft  != nullptr) && (g == itemDex[uLeft->floor].tileConnectGroup);
                            rC = (uRight != nullptr) && (g == itemDex[uRight->floor].tileConnectGroup);
                        }
                        uDir = connectGroupExtraIndex(tC, bC, lC, rC);

                        if (itemDex[under->floor].animeSize > 1)
                        {
                            int fps = itemDex[under->floor].animeFPS;
                            int sz  = itemDex[under->floor].animeSize;
                            uAni16 = getMilliTimer() / fps % sz;
                        }
                    }
                    else if (itemDex[under->floor].animeSize > 1)
                    {
                        int fps = itemDex[under->floor].animeFPS;
                        int sz  = itemDex[under->floor].animeSize;
                        uAniSingle = getMilliTimer() / fps % sz;
                    }

                    int uSpr = itemDex[under->floor].tileSprIndex + itemDex[under->floor].extraSprIndexSingle + 16 * itemDex[under->floor].extraSprIndex16;
                    uSpr += 16 * uAni16 + uAniSingle;

                    if (under->floor == itemID::grass)
                    {
                        if (getSeason() == seasonFlag::winter) uSpr += 16;
                        else if (getSeason() == seasonFlag::summer) uSpr += 32;
                    }

                    vertices[tileCounter] = { baseVx, baseVy };
                    indices[tileCounter] = uSpr + uDir;
                    batchAlphas[tileCounter] = alpha;
                    tileCounter++;
                }

                // 아래층 wall
                if (under->wall != itemID::none)
                {
                    int uDir = 0;
                    if (itemDex[under->wall].tileConnectGroup != -1)
                    {
                        bool tC, bC, lC, rC;
                        if (itemDex[under->wall].tileConnectGroup == 0)
                        {
                            tC = (uTop   != nullptr) && (under->wall == uTop->wall);
                            bC = (uBot   != nullptr) && (under->wall == uBot->wall);
                            lC = (uLeft  != nullptr) && (under->wall == uLeft->wall);
                            rC = (uRight != nullptr) && (under->wall == uRight->wall);
                        }
                        else
                        {
                            int g = itemDex[under->wall].tileConnectGroup;
                            tC = (uTop   != nullptr) && (g == itemDex[uTop->wall].tileConnectGroup);
                            bC = (uBot   != nullptr) && (g == itemDex[uBot->wall].tileConnectGroup);
                            lC = (uLeft  != nullptr) && (g == itemDex[uLeft->wall].tileConnectGroup);
                            rC = (uRight != nullptr) && (g == itemDex[uRight->wall].tileConnectGroup);
                        }

                        // 인접 창문은 벽으로 취급 — 벽이 창문 위/아래/옆으로 끊김 없이 이어 보이게
                        tC = tC || tileHasWindow(tgtX,     tgtY - 1, pZ - d);
                        bC = bC || tileHasWindow(tgtX,     tgtY + 1, pZ - d);
                        lC = lC || tileHasWindow(tgtX - 1, tgtY,     pZ - d);
                        rC = rC || tileHasWindow(tgtX + 1, tgtY,     pZ - d);

                        uDir = connectGroupExtraIndex(tC, bC, lC, rC);
                    }

                    vertices[tileCounter] = { baseVx, baseVy };
                    indices[tileCounter] = itemDex[under->wall].tileSprIndex + uDir;
                    batchAlphas[tileCounter] = alpha;
                    tileCounter++;
                }

                break;
            }
        }

        int dirCorrection = 0;
        int tileAniExtraIndex16 = 0;
        int tileAniExtraIndexSingle = 0;
        if (itemDex[thisTile->floor].tileConnectGroup != -1)
        {
            bool topCheck, botCheck, leftCheck, rightCheck;
            if (itemDex[thisTile->floor].tileConnectGroup == 0)
            {
                int currentTileFloor = thisTile->floor;
                int topTileFloor = topTile->floor;
                int botTileFloor = botTile->floor;
                int leftTileFloor = leftTile->floor;
                int rightTileFloor = rightTile->floor;

                topCheck = currentTileFloor == topTileFloor;
                botCheck = currentTileFloor == botTileFloor;
                leftCheck = currentTileFloor == leftTileFloor;
                rightCheck = currentTileFloor == rightTileFloor;
            }
            else
            {
                int currentTileGroup = itemDex[thisTile->floor].tileConnectGroup;
                int topTileGroup = itemDex[topTile->floor].tileConnectGroup;
                int botTileGroup = itemDex[botTile->floor].tileConnectGroup;
                int leftTileGroup = itemDex[leftTile->floor].tileConnectGroup;
                int rightTileGroup = itemDex[rightTile->floor].tileConnectGroup;

                topCheck = currentTileGroup == topTileGroup;
                botCheck = currentTileGroup == botTileGroup;
                leftCheck = currentTileGroup == leftTileGroup;
                rightCheck = currentTileGroup == rightTileGroup;
            }

            dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

            if (itemDex[thisTile->floor].animeSize > 1)
            {
                int animeFPS = itemDex[thisTile->floor].animeFPS;
                int animeSize = itemDex[thisTile->floor].animeSize;
                tileAniExtraIndex16 = getMilliTimer() / animeFPS % animeSize;
            }
        }
        else
        {
            if (itemDex[thisTile->floor].animeSize > 1)
            {
                int animeFPS = itemDex[thisTile->floor].animeFPS;
                int animeSize = itemDex[thisTile->floor].animeSize;
                tileAniExtraIndexSingle = getMilliTimer() / animeFPS % animeSize;
            }
        }

        int sprIndex = itemDex[thisTile->floor].tileSprIndex + itemDex[thisTile->floor].extraSprIndexSingle + 16 * itemDex[thisTile->floor].extraSprIndex16;
        sprIndex += 16 * tileAniExtraIndex16 + tileAniExtraIndexSingle;
        if (thisTile->floor == itemID::none) sprIndex = 506;

        if (thisTile->floor == itemID::grass)
        {
            if (getSeason() == seasonFlag::winter) sprIndex += 16;
            else if (getSeason() == seasonFlag::summer) sprIndex += 32;
        }


        if (thisTile->floor == itemID::farmland && isWetTile({tgtX, tgtY,PlayerZ()})) sprIndex++;

        // 공허 타일은 위쪽 캐스케이드 블록에서 이미 처리됨 - 여기선 emit 생략
        if (!isVoidHere)
        {
            vertices[tileCounter] =
            {
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
            };
            indices[tileCounter] = sprIndex + dirCorrection;
            batchAlphas[tileCounter] = 255;
            tileCounter++;
        }



        // 눈
        if (thisTile->hasSnow == true)
        {
            setZoom(zoomScale);

            int dirCorrection = 0;

            bool topCheck, botCheck, leftCheck, rightCheck;

            int currentSnow = thisTile->hasSnow;
            int topTileSnow = topTile->hasSnow;
            int botTileSnow = botTile->hasSnow;
            int leftTileSnow = leftTile->hasSnow;
            int rightTileSnow = rightTile->hasSnow;

            topCheck = currentSnow == topTileSnow;
            botCheck = currentSnow == botTileSnow;
            leftCheck = currentSnow == leftTileSnow;
            rightCheck = currentSnow == rightTileSnow;

            dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

            vertices[tileCounter] = 
            { 
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)), 
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY)) 
            };
            indices[tileCounter] = 1712 + dirCorrection;
            batchAlphas[tileCounter] = 255;
            tileCounter++;
        }


        // 발자국
        if (Footprint::map.find({ tgtX, tgtY, PlayerZ() }) != Footprint::map.end())
        {
            for (const auto& address : Footprint::map[{tgtX, tgtY, PlayerZ()}])
            {
                vertices[tileCounter] =
                {
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
                };

                indices[tileCounter] = 1952 + address->sprIndex;
                batchAlphas[tileCounter] = address->alpha;
                tileCounter++;
            }
        }

        //유체 웅덩이(유체 아이템으로 인한)
        auto getStackLastFluidCode = [](Point3 coord)->int
            {
                ItemStack* tileStack = TileItemStack(coord);
                if (tileStack == nullptr) return -1;

                std::vector<ItemData>& stackInfo = tileStack->getPocket()->itemInfo;
                for (int i = stackInfo.size() - 1; i >= 0; i--)
                {
                    if (stackInfo[i].checkFlag(itemFlag::LIQUID)) return stackInfo[i].itemCode;
                }
                return -1;
            };

        int currentFluidCode = getStackLastFluidCode({ tgtX,tgtY,PlayerZ() });
        if (currentFluidCode != -1)
        {
            int dirCorrection = 0;

            bool topCheck, botCheck, leftCheck, rightCheck;

            topCheck = getStackLastFluidCode({ tgtX,tgtY - 1,PlayerZ() }) == currentFluidCode;
            botCheck = getStackLastFluidCode({ tgtX,tgtY + 1,PlayerZ() }) == currentFluidCode;
            leftCheck = getStackLastFluidCode({ tgtX - 1,tgtY,PlayerZ() }) == currentFluidCode;
            rightCheck = getStackLastFluidCode({ tgtX + 1,tgtY,PlayerZ() }) == currentFluidCode;

            dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

            int itemSprIndex = -1;
            switch (currentFluidCode)
            {
            case itemID::water:
                itemSprIndex = 2080;
                break;
            case itemID::gasoline:
                itemSprIndex = 2144;
                break;
            default:
                itemSprIndex = 2176;
                break;
            }

            vertices[tileCounter] =
            {
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
            };
            int tileAniExtraIndex16 = getMilliTimer() / 1000 % 2;
            indices[tileCounter] = itemSprIndex + dirCorrection + 16 * tileAniExtraIndex16;
            batchAlphas[tileCounter] = 150;
            tileCounter++;
        }



    }

    for(auto elem : shallowSeaWaves)
    {
        flushIfFull();
        int tgtX = elem.x;
        int tgtY = elem.y;
        // wave 항목은 로드 영역 경계를 벗어난 이웃 타일이 들어올 수 있음 — 청크 누락 시 스킵.
        const TileData* thisTile  = World::ins()->tryGetTile(tgtX,     tgtY,     PlayerZ());
        const TileData* topTile   = World::ins()->tryGetTile(tgtX,     tgtY - 1, PlayerZ());
        const TileData* botTile   = World::ins()->tryGetTile(tgtX,     tgtY + 1, PlayerZ());
        const TileData* leftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY,     PlayerZ());
        const TileData* rightTile = World::ins()->tryGetTile(tgtX + 1, tgtY,     PlayerZ());
        if (!thisTile || !topTile || !botTile || !leftTile || !rightTile) continue;
        int animeExtraIndex = 32 * ((SDL_GetTicks() / 300) % 7);

        if(thisTile->floor != itemID::shallowSeaWater)
        {
            bool topCheck = topTile->floor == itemID::shallowSeaWater;
            bool botCheck = botTile->floor == itemID::shallowSeaWater;
            bool leftCheck = leftTile->floor == itemID::shallowSeaWater;
            bool rightCheck = rightTile->floor == itemID::shallowSeaWater;

            Uint8 alpha = 205;
            int extraIndex = 224;
            auto addWave = [&](int inputIndex)
                {
                    vertices[tileCounter] =
                    {
                        cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                        cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
                    };
                    indices[tileCounter] = inputIndex + animeExtraIndex + extraIndex;
                    batchAlphas[tileCounter] = alpha;
                    tileCounter++;
                };

            if (topCheck && botCheck && leftCheck && rightCheck) addWave(1526);
            else if (topCheck && botCheck && rightCheck) addWave(1520); // →
            else if (leftCheck && botCheck && rightCheck) addWave(1523); //↑
            else if (botCheck && rightCheck && topCheck) addWave(1522); //←
            else if (rightCheck && topCheck && leftCheck) addWave(1521); // ↓
            else if (topCheck && botCheck) addWave(1524);
            else if (rightCheck && leftCheck) addWave(1525);
            else if (rightCheck && topCheck) addWave(1505);
            else if (topCheck && leftCheck)  addWave(1507);
            else if (leftCheck && botCheck)  addWave(1509);
            else if (botCheck && rightCheck) addWave(1511);
            else if (topCheck)  addWave(1506);
            else if (botCheck)  addWave(1510);
            else if (leftCheck) addWave(1508);
            else if (rightCheck)addWave(1504);


            const TileData* topRightTile = World::ins()->tryGetTile(tgtX + 1, tgtY - 1, PlayerZ());
            const TileData* topLeftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY - 1, PlayerZ());
            const TileData* botLeftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY + 1, PlayerZ());
            const TileData* botRightTile = World::ins()->tryGetTile(tgtX + 1, tgtY + 1, PlayerZ());

            // 대각선 이웃은 누락 시 false (해안 코너 렌더 생략)
            bool topRightCheck = topRightTile && topRightTile->floor == itemID::shallowSeaWater;
            bool topLeftCheck  = topLeftTile  && topLeftTile->floor  == itemID::shallowSeaWater;
            bool botLeftCheck  = botLeftTile  && botLeftTile->floor  == itemID::shallowSeaWater;
            bool botRightCheck = botRightTile && botRightTile->floor == itemID::shallowSeaWater;

            if (topRightCheck && !topCheck && !rightCheck) addWave(1514);
            if (topLeftCheck && !topCheck && !leftCheck) addWave(1515);
            if (botLeftCheck && !botCheck && !leftCheck) addWave(1512);
            if (botRightCheck && !botCheck && !rightCheck) addWave(1513);
        }
    }

    for (auto elem : deepSeaWaves)
    {
        flushIfFull();
        int tgtX = elem.x;
        int tgtY = elem.y;

        const TileData* thisTile  = World::ins()->tryGetTile(tgtX,     tgtY,     PlayerZ());
        const TileData* topTile   = World::ins()->tryGetTile(tgtX,     tgtY - 1, PlayerZ());
        const TileData* botTile   = World::ins()->tryGetTile(tgtX,     tgtY + 1, PlayerZ());
        const TileData* leftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY,     PlayerZ());
        const TileData* rightTile = World::ins()->tryGetTile(tgtX + 1, tgtY,     PlayerZ());
        if (!thisTile || !topTile || !botTile || !leftTile || !rightTile) continue;
        int animeExtraIndex = 32 * ((SDL_GetTicks() / 300) % 7);
        if (thisTile->floor == itemID::shallowSeaWater) animeExtraIndex = 0;

        {
            bool topCheck = topTile->floor == itemID::deepSeaWater;
            bool botCheck = botTile->floor == itemID::deepSeaWater;
            bool leftCheck = leftTile->floor == itemID::deepSeaWater;
            bool rightCheck = rightTile->floor == itemID::deepSeaWater;
            auto addWave = [&](int inputIndex)
                {
                    vertices[tileCounter] =
                    {
                        cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                        cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
                    };
                    indices[tileCounter] = inputIndex + animeExtraIndex;
                    batchAlphas[tileCounter] = 200;
                    tileCounter++;
                };

            if (topCheck && botCheck && leftCheck && rightCheck) addWave(1526);
            else if (topCheck && botCheck && rightCheck) addWave(1520); // →
            else if (leftCheck && botCheck && rightCheck) addWave(1523); //↑
            else if (botCheck && rightCheck && topCheck) addWave(1522); //←
            else if (rightCheck && topCheck && leftCheck) addWave(1521); // ↓
            else if (topCheck && botCheck) addWave(1524);
            else if (rightCheck && leftCheck) addWave(1525);
            else if (rightCheck && topCheck) addWave(1505);
            else if (topCheck && leftCheck)  addWave(1507);
            else if (leftCheck && botCheck)  addWave(1509);
            else if (botCheck && rightCheck) addWave(1511);
            else if (topCheck)  addWave(1506);
            else if (botCheck)  addWave(1510);
            else if (leftCheck) addWave(1508);
            else if (rightCheck)addWave(1504);

            const TileData* topRightTile = World::ins()->tryGetTile(tgtX + 1, tgtY - 1, PlayerZ());
            const TileData* topLeftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY - 1, PlayerZ());
            const TileData* botLeftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY + 1, PlayerZ());
            const TileData* botRightTile = World::ins()->tryGetTile(tgtX + 1, tgtY + 1, PlayerZ());

            bool topRightCheck = topRightTile && topRightTile->floor == itemID::deepSeaWater;
            bool topLeftCheck  = topLeftTile  && topLeftTile->floor  == itemID::deepSeaWater;
            bool botLeftCheck  = botLeftTile  && botLeftTile->floor  == itemID::deepSeaWater;
            bool botRightCheck = botRightTile && botRightTile->floor == itemID::deepSeaWater;

            bool topCheckSw = topTile->floor == itemID::shallowSeaWater;
            bool botCheckSw = botTile->floor == itemID::shallowSeaWater;
            bool leftCheckSw = leftTile->floor == itemID::shallowSeaWater;
            bool rightCheckSw = rightTile->floor == itemID::shallowSeaWater;
            if (topRightCheck && (!topCheck && !topCheckSw) && (!rightCheck && !rightCheckSw)) addWave(1514);
            if (topLeftCheck && (!topCheck && !topCheckSw) && (!leftCheck && !leftCheckSw)) addWave(1515);
            if (botLeftCheck && (!botCheck && !botCheckSw) && (!leftCheck && !leftCheckSw)) addWave(1512);
            if (botRightCheck && (!botCheck && !botCheckSw) && (!rightCheck && !rightCheckSw)) addWave(1513);
        }
    }

    for (auto elem : deepFreshWaves)
    {
        flushIfFull();
        int tgtX = elem.x;
        int tgtY = elem.y;

        const TileData* thisTile  = World::ins()->tryGetTile(tgtX,     tgtY,     PlayerZ());
        const TileData* topTile   = World::ins()->tryGetTile(tgtX,     tgtY - 1, PlayerZ());
        const TileData* botTile   = World::ins()->tryGetTile(tgtX,     tgtY + 1, PlayerZ());
        const TileData* leftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY,     PlayerZ());
        const TileData* rightTile = World::ins()->tryGetTile(tgtX + 1, tgtY,     PlayerZ());
        if (!thisTile || !topTile || !botTile || !leftTile || !rightTile) continue;
        int animeExtraIndex = 0;

        {
            bool topCheck = topTile->floor == itemID::deepFreshWater;
            bool botCheck = botTile->floor == itemID::deepFreshWater;
            bool leftCheck = leftTile->floor == itemID::deepFreshWater;
            bool rightCheck = rightTile->floor == itemID::deepFreshWater;
            auto addWave = [&](int inputIndex)
                {
                    vertices[tileCounter] =
                    {
                        cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                        cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
                    };
                    indices[tileCounter] = inputIndex + animeExtraIndex + 496;
                    batchAlphas[tileCounter] = 200;
                    tileCounter++;
                };

            if (topCheck && botCheck && leftCheck && rightCheck) addWave(1526);
            else if (topCheck && botCheck && rightCheck) addWave(1520); // →
            else if (leftCheck && botCheck && rightCheck) addWave(1523); //↑
            else if (botCheck && rightCheck && topCheck) addWave(1522); //←
            else if (rightCheck && topCheck && leftCheck) addWave(1521); // ↓
            else if (topCheck && botCheck) addWave(1524);
            else if (rightCheck && leftCheck) addWave(1525);
            else if (rightCheck && topCheck) addWave(1505);
            else if (topCheck && leftCheck)  addWave(1507);
            else if (leftCheck && botCheck)  addWave(1509);
            else if (botCheck && rightCheck) addWave(1511);
            else if (topCheck)  addWave(1506);
            else if (botCheck)  addWave(1510);
            else if (leftCheck) addWave(1508);
            else if (rightCheck)addWave(1504);

            const TileData* topRightTile = World::ins()->tryGetTile(tgtX + 1, tgtY - 1, PlayerZ());
            const TileData* topLeftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY - 1, PlayerZ());
            const TileData* botLeftTile  = World::ins()->tryGetTile(tgtX - 1, tgtY + 1, PlayerZ());
            const TileData* botRightTile = World::ins()->tryGetTile(tgtX + 1, tgtY + 1, PlayerZ());

            bool topRightCheck = topRightTile && topRightTile->floor == itemID::deepFreshWater;
            bool topLeftCheck  = topLeftTile  && topLeftTile->floor  == itemID::deepFreshWater;
            bool botLeftCheck  = botLeftTile  && botLeftTile->floor  == itemID::deepFreshWater;
            bool botRightCheck = botRightTile && botRightTile->floor == itemID::deepFreshWater;

            bool topCheckSw = topTile->floor == itemID::shallowFreshWater;
            bool botCheckSw = botTile->floor == itemID::shallowFreshWater;
            bool leftCheckSw = leftTile->floor == itemID::shallowFreshWater;
            bool rightCheckSw = rightTile->floor == itemID::shallowFreshWater;
            if (topRightCheck && (!topCheck && !topCheckSw) && (!rightCheck && !rightCheckSw)) addWave(1514);
            if (topLeftCheck && (!topCheck && !topCheckSw) && (!leftCheck && !leftCheckSw)) addWave(1515);
            if (botLeftCheck && (!botCheck && !botCheckSw) && (!leftCheck && !leftCheckSw)) addWave(1512);
            if (botRightCheck && (!botCheck && !botCheckSw) && (!rightCheck && !rightCheckSw)) addWave(1513);
        }
    }

    for (auto elem : Wave::list)
    {
        flushIfFull();
        int tgtX = elem->getGridX();
        int tgtY = elem->getGridY();
        vertices[tileCounter] =
        {
            cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
            cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
        };
        

        if(elem->lifetime <6) indices[tileCounter] = 2018;
        else if(elem->lifetime <12) indices[tileCounter] = 2017;
        else indices[tileCounter] = 2016;

        if (TileFloor(tgtX, tgtY, PlayerZ()) == itemID::shallowFreshWater) indices[tileCounter] += 3;

        
        batchAlphas[tileCounter] = elem->alpha;
        tileCounter++;
    }

    for (auto elem : Wake::list)
    {
        flushIfFull();
        int tgtX = elem->getGridX();
        int tgtY = elem->getGridY();
        vertices[tileCounter] =
        {
            cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
            cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
        };

        if (elem->lifetime < 6) indices[tileCounter] = 2032 + 32 + elem->dir;
        else if (elem->lifetime < 12) indices[tileCounter] = 2032 + 16 + elem->dir;
        else indices[tileCounter] = 2032 + elem->dir;

        if (TileFloor(tgtX, tgtY, PlayerZ()) == itemID::deepFreshWater) indices[tileCounter] += 8;


        batchAlphas[tileCounter] = elem->alpha;
        tileCounter++;
    }

    // wall과 스킬 범위 UI는 파도 위에 그림 — floor < wave < wall 순서 유지
    for (const auto& elem : tileList)
    {
        flushIfFull();
        int tgtX = elem.x;
        int tgtY = elem.y;
        const TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());

        if (thisTile->wall != itemID::none)
        {
            const TileData* topTile = &World::ins()->getTile(tgtX, tgtY - 1, PlayerZ());
            const TileData* botTile = &World::ins()->getTile(tgtX, tgtY + 1, PlayerZ());
            const TileData* leftTile = &World::ins()->getTile(tgtX - 1, tgtY, PlayerZ());
            const TileData* rightTile = &World::ins()->getTile(tgtX + 1, tgtY, PlayerZ());

            setZoom(zoomScale);
            int dirCorrection = 0;
            if (itemDex[thisTile->wall].tileConnectGroup != -1)
            {
                bool topCheck, botCheck, leftCheck, rightCheck;
                if (itemDex[thisTile->wall].tileConnectGroup == 0)
                {
                    int currentTileWall = thisTile->wall;
                    int topTileWall = topTile->wall;
                    int botTileWall = botTile->wall;
                    int leftTileWall = leftTile->wall;
                    int rightTileWall = rightTile->wall;

                    topCheck = currentTileWall == topTileWall;
                    botCheck = currentTileWall == botTileWall;
                    leftCheck = currentTileWall == leftTileWall;
                    rightCheck = currentTileWall == rightTileWall;
                }
                else
                {
                    int currentTileGroup = itemDex[thisTile->wall].tileConnectGroup;
                    int topTileGroup = itemDex[topTile->wall].tileConnectGroup;
                    int botTileGroup = itemDex[botTile->wall].tileConnectGroup;
                    int leftTileGroup = itemDex[leftTile->wall].tileConnectGroup;
                    int rightTileGroup = itemDex[rightTile->wall].tileConnectGroup;

                    topCheck = (currentTileGroup == topTileGroup);
                    botCheck = (currentTileGroup == botTileGroup);
                    leftCheck = (currentTileGroup == leftTileGroup);
                    rightCheck = (currentTileGroup == rightTileGroup);
                }

                // 인접 창문은 벽으로 취급 — 벽이 창문 위/아래/옆으로 끊김 없이 이어 보이게
                topCheck   = topCheck   || tileHasWindow(tgtX,     tgtY - 1, PlayerZ());
                botCheck   = botCheck   || tileHasWindow(tgtX,     tgtY + 1, PlayerZ());
                leftCheck  = leftCheck  || tileHasWindow(tgtX - 1, tgtY,     PlayerZ());
                rightCheck = rightCheck || tileHasWindow(tgtX + 1, tgtY,     PlayerZ());

                dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);
            }

            vertices[tileCounter] =
            {
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
            };
            indices[tileCounter] = itemDex[thisTile->wall].tileSprIndex + dirCorrection;
            batchAlphas[tileCounter] = 255;
            tileCounter++;
        }

        // 스킬 범위 그리기
        if (rangeSet.find({ tgtX, tgtY }) != rangeSet.end())
        {
            bool rightCheck = rangeSet.find({ tgtX + 1, tgtY }) != rangeSet.end();
            bool topCheck = rangeSet.find({ tgtX, tgtY - 1 }) != rangeSet.end();
            bool leftCheck = rangeSet.find({ tgtX - 1, tgtY }) != rangeSet.end();
            bool botCheck = rangeSet.find({ tgtX, tgtY + 1 }) != rangeSet.end();
            int dirCorrection = connectGroupExtraIndex(topCheck, botCheck, leftCheck, rightCheck);

            vertices[tileCounter] =
            {
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
            };
            indices[tileCounter] = 1440 + dirCorrection;
            batchAlphas[tileCounter] = 255;
            tileCounter++;
        }

        if (rangeRay && raySet.find({ tgtX - PlayerX(),tgtY - PlayerY() }) != raySet.end())
        {
            vertices[tileCounter] =
            {
                cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
            };
            indices[tileCounter] = 0;
            batchAlphas[tileCounter] = 150;
            tileCounter++;
        }

        if (rangeSet.size() > 0 && rangeSet.find({ tgtX, tgtY }) != rangeSet.end())
        {
            if (getAbsMouseGrid().x == tgtX && getAbsMouseGrid().y == tgtY)
            {
                vertices[tileCounter] =
                {
                    cameraW / 2 + static_cast<int>(zoomScale * (16 * tgtX + 8 - cameraX)),
                    cameraH / 2 + static_cast<int>(zoomScale * (16 * tgtY + 8 - cameraY))
                };
                indices[tileCounter] = 0;
                batchAlphas[tileCounter] = 150;
                tileCounter++;
            }
        }
    }

    drawSpriteBatchCenter(spr::tileset, vertices, indices, batchAlphas, tileCounter);
    
    
    for (const auto& elem : wallHPList)// 벽 HP 표기
    {
        int tgtX = elem.x;
        int tgtY = elem.y;
        TileData& t = World::ins()->getTile(tgtX, tgtY, PlayerZ());

        if (t.wallFakeHP > t.wallHP) t.wallFakeHP -= ((float)t.wallMaxHP / 100.0);
        else if (t.wallFakeHP < t.wallHP) t.wallFakeHP = t.wallHP;

        if (t.wallFakeHP != t.wallHP)
        {
            if (t.alphaFakeHPBar > 20) t.alphaFakeHPBar -= 20;
            else
            {
                t.alphaFakeHPBar = 0;
                t.wallFakeHP = t.wallHP;
            }
        }
        else t.alphaFakeHPBar = 0;

        if (t.displayHPBarCount > 1) t.displayHPBarCount--;
        else if (t.displayHPBarCount == 1)
        {
            t.alphaHPBar -= 10;
            if (t.alphaHPBar <= 0)
            {
                t.alphaHPBar = 0;
                t.displayHPBarCount = 0;
            }
        }

        int drawingX = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX);
        int drawingY = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY);
        draw3pxGauge(
            drawingX - (int)(8 * zoomScale),
            drawingY,
            zoomScale,
            (float)t.wallHP / (float)t.wallMaxHP,
            t.alphaHPBar,
            lowCol::red,
            (float)t.wallFakeHP / (float)t.wallMaxHP,
            t.alphaFakeHPBar
        );
    }
    
    
    setZoom(1.0);
}

void drawCorpses()
{
    for (const auto& elem : Corpse::list)
    {
        Corpse* adr = elem;
        if (pZ == adr->getZ()) // 플레이어의 z축과 시체의 z축이 같을 경우
        {
            setZoom(zoomScale);
            drawSpriteCenter(adr->getSprite(), adr->getSprIndex(), (cameraW / 2) + zoomScale * worldWrap::signedDeltaRenderX(cameraX, adr->getX()), (cameraH / 2) + zoomScale * (adr->getY() - cameraY));
            setZoom(1.0);
        }
    }
}

void drawFloorProp()
{
    // 바닥 설치물 그리기
    for (const auto& elem : floorPropList)
    {
        int tgtX = elem.x;
        int tgtY = elem.y;
        Prop* iPtr = TileProp(tgtX, tgtY, PlayerZ());
        iPtr->drawSelf();
    }
}


void drawItems()
{
    for (const auto& elem : itemList)
    {
        int tgtX = elem.x;
        int tgtY = elem.y;

        ItemStack* address = TileItemStack(tgtX, tgtY, pZ);
        if (address == nullptr) continue; //삭제된 ItemStack은 건너뜀
        std::vector<ItemData>& pocketInfo = address->getPocket()->itemInfo;
        if (pocketInfo.size() == 0) continue; //빈 포켓은 그리지 않음

        // 테이블 위 아이템스택은 테이블 높이만큼 y -5px 올려 그림
        int tableYOffset = 0;
        Prop* tilePropPtr = TileProp(tgtX, tgtY, pZ);
        if (tilePropPtr != nullptr)
        {
            int propCode = tilePropPtr->leadItem.itemCode;
            if (propCode == itemID::woodenTable || propCode == itemID::steelTable || propCode == itemID::roundWoodenTable) tableYOffset = -5;
        }

        for (int i = pocketInfo.size() - 1; i >= 0; i--)
        {
            if (pocketInfo[pocketInfo.size() - 1].checkFlag(itemFlag::LIQUID) == false)
            {
                setZoom(zoomScale);
                drawSpriteCenter
                (
                    spr::itemset,
                    pocketInfo[pocketInfo.size() - 1].getSprIndex(),
                    (cameraW / 2) + zoomScale * (worldWrap::signedDeltaRenderX(cameraX, address->getX()) + address->getIntegerFakeX()),
                    (cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY() + tableYOffset)
                );
                setZoom(1.0);
                break;
            }
        }
    }
}

void drawEntities()
{
    // 화염 그리기
    for (const auto& elem : flameList)
    {
        int tgtX = elem.x;
        int tgtY = elem.y;
        Flame* tgtFlame = (&World::ins()->getTile(tgtX, tgtY, PlayerZ()))->flamePtr.get();

        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
        SDL_SetTextureBlendMode(spr::propset->getTexture(), SDL_BLENDMODE_BLEND);
        int sprIndex = 0;
        int animeVal = timer::timer600 % 30;
        if (animeVal < 6) sprIndex += 0;
        else if (animeVal < 12) sprIndex += 1;
        else if (animeVal < 18) sprIndex += 2;
        else if (animeVal < 24) sprIndex += 3;
        else sprIndex += 4;
        sprIndex += tgtFlame->sprRandomStart;
        sprIndex = sprIndex % 5;

        drawSpriteCenter
        (
            spr::flameSet,
            tgtFlame->sprInfimum + sprIndex,
            dst.x + dst.w / 2 + zoomScale,
            dst.y + dst.h / 2 + zoomScale
        );
        SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
        setZoom(1.0);
    }

    std::vector<Point2> rotorList;
    // 차량그리기
    std::unordered_set<Drawable*> vehCache;
    for (const auto& elem : renderVehList)
    {
        if (vehCache.find(elem) == vehCache.end())
        {
            elem->drawSelf();
            vehCache.insert(elem);
        }
    }
    // 엔티티&일반설치물 그리기
    std::unordered_set<Drawable*> entityCache;
    for (const auto& elem : renderEntityList)
    {
        if (entityCache.find(elem) == entityCache.end())
        {
            // Nervedrive 필터 월드패스: 플레이어는 틴트 이후 디폴트 타겟에 따로 그림.
            if (lotEditorActive && elem == (Drawable*)PlayerPtr) { entityCache.insert(elem); continue; } //LotEditor: 플레이어 스프라이트 숨김
            if (nervedriveFilter::shouldSkipPlayerInWorld() && elem == (Drawable*)PlayerPtr) { entityCache.insert(elem); continue; }
            elem->drawSelf();
            entityCache.insert(elem);
        }
    }

    // 상단 프롭 그리기 — 동일 타일의 엔티티/차량을 덮는 프롭(열린 롤업도어 등). 가스/안개보다는 아래
    for (const auto& elem : upperPropList)
    {
        Prop* upPtr = TileProp(elem.x, elem.y, pZ);
        if (upPtr != nullptr) upPtr->drawSelf();
    }

    // 헬기 로터 그리기
    for (int i = 0; i < rotorList.size(); i++)
    {
        int tgtX = rotorList[i].x;
        int tgtY = rotorList[i].y;
        Vehicle* vPtr = TileVehicle(tgtX, tgtY, PlayerZ());

        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        if (TileVehicle(PlayerX(), PlayerY(), PlayerZ()) == vPtr)
        {
            SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 50);
            SDL_SetTextureBlendMode(spr::mainRotor->getTexture(), SDL_BLENDMODE_BLEND);
        }
        drawSpriteCenter
        (
            spr::mainRotor,
            0,
            dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
            dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
        );
        SDL_SetTextureAlphaMod(spr::mainRotor->getTexture(), 255);
        setZoom(1.0);
    }

    // 조종 중인 차량의 마커 그리기
    if (ctrlVeh != nullptr)
    {
        Vehicle* vPtr = ctrlVeh;
        for (auto it = vPtr->partInfo.begin(); it != vPtr->partInfo.end(); it++)
        {
            int tgtX = it->first.x;
            int tgtY = it->first.y;
            int tgtZ = it->first.z;
            if (tgtZ != PlayerZ()) continue;

            for (int layer = 0; layer < vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo.size(); layer++)
            {
                if (vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].checkFlag(itemFlag::TIRE_STEER))
                {
                    SDL_Rect dst;
                    dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
                    dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
                    dst.w = tileSize;
                    dst.h = tileSize;

                    setZoom(zoomScale);
                    SDL_SetTextureAlphaMod(spr::vehset->getTexture(), 150);
                    SDL_SetTextureBlendMode(spr::vehset->getTexture(), SDL_BLENDMODE_BLEND);
                    int sprIndex = vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].vehSprIndex + vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].extraSprIndexSingle + 16 * vPtr->partInfo[{tgtX, tgtY, tgtZ}]->itemInfo[layer].extraSprIndex16;
                    drawSpriteCenter
                    (
                        spr::vehset,
                        sprIndex,
                        dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                        dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
                    );
                    SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 150);
                    SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND);
                    drawSpriteCenter
                    (
                        spr::dirMarker,
                        128 + dir16toInt16(vPtr->wheelDir),
                        dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                        dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
                    );
                    SDL_SetTextureAlphaMod(spr::vehset->getTexture(), 255);
                    SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255);
                    setZoom(1.0);
                }
                else if (vPtr->isPowerCart)
                {
                    SDL_Rect dst;
                    dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
                    dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
                    dst.w = tileSize;
                    dst.h = tileSize;

                    setZoom(zoomScale);
                    SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 110);
                    SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND);
                    drawSpriteCenter
                    (
                        spr::dirMarker,
                        224 + dir16toInt16(vPtr->bodyDir),
                        dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                        dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
                    );
                    SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255);
                    setZoom(1.0);
                }
            }

            // 플레이어 속도 표현
            if (tgtX == PlayerX() && tgtY == PlayerY())
            {
                if (vPtr->spdVec.isZeroVec() == false)
                {
                    SDL_Rect dst;
                    dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
                    dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
                    dst.w = tileSize;
                    dst.h = tileSize;

                    setZoom(zoomScale);
                    SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 180);
                    SDL_SetTextureBlendMode(spr::dirMarker->getTexture(), SDL_BLENDMODE_BLEND);
                    int spdExtraIndex = 0;
                    if (vPtr->spdVec.getLength() < 5) spdExtraIndex = 0;
                    else if (vPtr->spdVec.getLength() < 10) spdExtraIndex = 1;
                    else if (vPtr->spdVec.getLength() < 15) spdExtraIndex = 2;
                    else spdExtraIndex = 3;
                    drawSpriteCenter
                    (
                        spr::dirMarker,
                        160 + getNearDir16(vPtr->spdVec) + 16 * spdExtraIndex,
                        dst.x + dst.w / 2 + zoomScale * vPtr->getIntegerFakeX(),
                        dst.y + dst.h / 2 + zoomScale * vPtr->getIntegerFakeY()
                    );
                    SDL_SetTextureAlphaMod(spr::dirMarker->getTexture(), 255);
                    setZoom(1.0);
                }
            }
        }
    }
}

void drawDamages()
{
    for (int i = 0; i < Damage::list.size(); i++)
    {
        Damage* address = Damage::list[i];
        int drawingX = (cameraW / 2) + zoomScale * worldWrap::signedDeltaRenderX(cameraX, address->getX());
        int drawingY = (cameraH / 2) + zoomScale * (address->getY() - cameraY);

        if(zoomScale == 3.0f || zoomScale == 2.0f) setZoom(2.0f);
        else if (zoomScale == 4.0f || zoomScale == 5.0f) setZoom(3.0f);
        SDL_SetTextureAlphaMod(address->getSprite()->getTexture(), address->getAlpha());
        drawSpriteCenter(address->getSprite(), 0, drawingX, drawingY);
        setZoom(1.0);
    }
}

void drawBullets()
{
    for (int i = 0; i < Bullet::list.size(); i++)
    {
        Bullet* address = Bullet::list[i];
        setZoom(zoomScale);
        drawSpriteCenter
        (
            address->sprite,
            address->sprIndex,
            (cameraW / 2) + zoomScale * (worldWrap::signedDeltaRenderX(cameraX, address->getX()) + address->getIntegerFakeX()),
            (cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
        );
        setZoom(1.0);
    }
}

void drawParticles()
{
    for (int i = 0; i < Particle::list.size(); i++)
    {
        Particle* address = Particle::list[i];
        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(address->sprite->getTexture(), address->alpha);
        drawSpriteCenter
        (
            address->sprite,
            address->sprIndex,
            (cameraW / 2) + zoomScale * (worldWrap::signedDeltaRenderX(cameraX, address->getX()) + address->getIntegerFakeX()),
            (cameraH / 2) + zoomScale * (address->getY() - cameraY + address->getIntegerFakeY())
        );
        SDL_SetTextureAlphaMod(address->sprite->getTexture(), 255);
        setZoom(1.0);
    }
}

void drawSprinklerSpray()
{
    int sprIndex = (SDL_GetTicks() / 100) % 3;

    for (auto i : sprinklerSpraySet33)
    {
        int tgtX = i.x;
        int tgtY = i.y;

        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(spr::sprinkler33->getTexture(), 200);
        drawSpriteCenter
        (
            spr::sprinkler33,
            sprIndex,
            dst.x + dst.w / 2 + zoomScale,
            dst.y + dst.h / 2 + zoomScale
        );
        SDL_SetTextureAlphaMod(spr::sprinkler33->getTexture(), 255);
        setZoom(1.0);
    }

    for (auto i : sprinklerSpraySet55)
    {
        int tgtX = i.x;
        int tgtY = i.y;

        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        SDL_SetTextureAlphaMod(spr::sprinkler55->getTexture(), 200);
        drawSpriteCenter
        (
            spr::sprinkler55,
            sprIndex,
            dst.x + dst.w / 2 + zoomScale,
            dst.y + dst.h / 2 + zoomScale
        );
        SDL_SetTextureAlphaMod(spr::sprinkler55->getTexture(), 255);
        setZoom(1.0);
    }
}

void drawRampArrows()
{
    if (rampUpSet.empty() && rampDownSet.empty()) return;

    Uint32 t = SDL_GetTicks();
    float phase = std::fmod(t / 2500.0f, 1.0f);
    float floatOffset = std::sin(phase * 3.14159f * 2.0f) * 3.0f; // 2.5초 부유

    SDL_SetTextureAlphaMod(spr::rampUpTile->getTexture(), 150);
    SDL_SetTextureBlendMode(spr::rampUpTile->getTexture(), SDL_BLENDMODE_BLEND);

    // RAMP_UP은 그대로, RAMP_DOWN은 수직 반전으로 그림
    auto drawArrow = [&](int tgtX, int tgtY)
        {
            SDL_Rect dst;
            dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
            dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
            dst.w = tileSize;
            dst.h = tileSize;

            setZoom(zoomScale);
            drawSpriteCenter(spr::rampUpTile, 0,
                dst.x + dst.w / 2,
                dst.y + dst.h / 2 + (int)(zoomScale * floatOffset));
            setZoom(1.0);
        };

    for (const auto& pos : rampUpSet) drawArrow(pos.x, pos.y);

    setFlip(SDL_FLIP_VERTICAL);
    for (const auto& pos : rampDownSet) drawArrow(pos.x, pos.y);
    setFlip(SDL_FLIP_NONE);

    SDL_SetTextureAlphaMod(spr::rampUpTile->getTexture(), 255);
}

void drawBridgeShadows()
{
    int tile = 16 * zoomScale;
    for (auto p : bridgeShadowList)
    {
        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * p.x) - cameraX);
        dst.y = cameraH / 2 + zoomScale * ((16 * p.y) - cameraY);
        dst.w = tile;
        dst.h = tile;
        drawFillRect(dst, col::black, 80);
    }
}

void drawMulFogs()
{
    //시간대별 야간 틴트 색 — 키프레임 보간은 constVar:colors가 단일 출처(월드맵 Map과 공유).
    SDL_Color mulLightColor = mulCol::ambientMulColorAt(getHour() + getMin() / 60.0f);

    int mulFogCounter = 0;
    for (const auto& fog : mulFogList)
    {
        //LotEditor 전체공개 시 mulFogList=가시 전 타일 -> MAX_BATCH 초과 가능. 넘기 전 중간 플러시.
        if (mulFogCounter >= MAX_BATCH - 4)
        {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MUL);
            drawRectBatch(16, 16, rectColors, vertices, mulFogCounter, zoomScale);
            mulFogCounter = 0;
        }
        if (lightFogSet.find({ fog.x,fog.y }) != lightFogSet.end()) continue;

        int screenX = cameraW / 2 + zoomScale * ((16 * fog.x + 8) - cameraX) - (8 * zoomScale);
        int screenY = cameraH / 2 + zoomScale * ((16 * fog.y + 8) - cameraY) - (8 * zoomScale);
        vertices[mulFogCounter] = { screenX, screenY };


        rectColors[mulFogCounter] = mulLightColor;



        mulFogCounter++;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MUL);
    drawRectBatch(16, 16, rectColors, vertices, mulFogCounter, zoomScale);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void drawFogs()
{
    for (const auto& elem : gasList)
    {
        int tgtX = elem.x;
        int tgtY = elem.y;
        SDL_Rect dst;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;

        setZoom(zoomScale);
        TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());

        for (int j = 0; j < thisTile->gasVec.size(); j++)
        {
            int alpha = 255 * (1 - std::exp(-0.03 * thisTile->gasVec[j].gasVol));
            SDL_SetTextureAlphaMod(spr::steamEffect1->getTexture(), std::min(255, alpha));
            SDL_SetTextureBlendMode(spr::steamEffect1->getTexture(), SDL_BLENDMODE_BLEND);

            SDL_SetTextureColorMod(spr::steamEffect1->getTexture(),
                itemDex[thisTile->gasVec[j].gasCode].gasColorR,
                itemDex[thisTile->gasVec[j].gasCode].gasColorG,
                itemDex[thisTile->gasVec[j].gasCode].gasColorB);
            int sprIndex = 0;//1초에 5번해야됨, 600 10
            sprIndex += thisTile->randomVal;
            if (timer::timer600 % 60 < 10) sprIndex += 0;
            else if (timer::timer600 % 60 < 20) sprIndex += 1;
            else if (timer::timer600 % 60 < 30) sprIndex += 2;
            else if (timer::timer600 % 60 < 40) sprIndex += 3;
            else if (timer::timer600 % 60 < 50) sprIndex += 4;
            else sprIndex += 5;
            sprIndex += 2 * j;
            sprIndex = sprIndex % 6;
            drawSpriteCenter
            (
                spr::steamEffect1,
                sprIndex,
                dst.x + dst.w / 2,
                dst.y + dst.h / 2
            );
        }
        SDL_SetTextureAlphaMod(spr::steamEffect1->getTexture(), 255);
        setZoom(1.0);
    }

    int fogCounter = 0;
    for (const auto& elem : blackFogList)
    {
        vertices[fogCounter] = {
            cameraW / 2 + static_cast<int>(zoomScale * ((16 * elem.x + 8) - cameraX) - ((16 * zoomScale) / 2)),
            cameraH / 2 + static_cast<int>(zoomScale * ((16 * elem.y + 8) - cameraY) - ((16 * zoomScale) / 2))
        };
        rectColors[fogCounter] = { 0x16, 0x16, 0x16, 255 };
        fogCounter++;

    }

    for (const auto& elem : grayFogList)
    {
        vertices[fogCounter] = {
            cameraW / 2 + static_cast<int>(zoomScale * ((16 * elem.x + 8) - cameraX) - ((16 * zoomScale) / 2)),
            cameraH / 2 + static_cast<int>(zoomScale * ((16 * elem.y + 8) - cameraY) - ((16 * zoomScale) / 2))
        };
        rectColors[fogCounter] = { 0x16, 0x16, 0x16, 185 };
        fogCounter++;
    }

    drawRectBatch(16, 16, rectColors, vertices, fogCounter, zoomScale);

    int lightCounter = 0;
    for (const auto& elem : lightFogList)
    {
        int tgtX = elem.x;
        int tgtY = elem.y;
        int posX = cameraW / 2 + static_cast<int>(zoomScale * ((16 * elem.x + 8) - cameraX) - ((16 * zoomScale) / 2));
        int posY = cameraH / 2 + static_cast<int>(zoomScale * ((16 * elem.y + 8) - cameraY) - ((16 * zoomScale) / 2));
        TileData* thisTile = &World::ins()->getTile(tgtX, tgtY, PlayerZ());
        for (int i = 0; i < thisTile->lightVec.size(); i++)
        {
            //vertices[lightCounter] = { posX, posY };
            //rectColors[lightCounter] = { 0x16, 0x16, 0x16, 200 };
            //lightCounter++;

            vertices[lightCounter] = { posX, posY };
            rectColors[lightCounter] = { thisTile->lightVec[i].r, thisTile->lightVec[i].g, thisTile->lightVec[i].b, thisTile->lightVec[i].a };
            lightCounter++;
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    drawRectBatch(16, 16, rectColors, vertices, lightCounter, zoomScale);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void drawMarkers()
{
    auto drawEplsionText = [](auto* spr, int spriteIndex, int x, int y) {
        SDL_SetTextureBlendMode(spr->getTexture(), SDL_BLENDMODE_BLEND);
        SDL_SetTextureColorMod(spr->getTexture(), 255, 255, 255);
        SDL_SetTextureAlphaMod(spr->getTexture(), 0);

        drawSprite(spr, spriteIndex, x + zoomScale, y);
        drawSprite(spr, spriteIndex, x, y - zoomScale);
        drawSprite(spr, spriteIndex, x - zoomScale, y);
        drawSprite(spr, spriteIndex, x, y + zoomScale);

        SDL_SetTextureColorMod(spr->getTexture(), 255, 255, 255);
        SDL_SetTextureAlphaMod(spr->getTexture(), 255);
        drawSprite(spr, spriteIndex, x, y);
        SDL_SetTextureColorMod(spr->getTexture(), 255, 255, 255);
        };

    if (option::inputMethod == input::mouse)
    {
        if (isPlayerMoving == false && turnCycle == turn::playerInput)
        {
            if (checkCursor(&letterbox) == false 
                && checkCursor(&tab) == false 
                && checkCursor(&letterboxPopUpButton) == false 
                && checkCursor(&quickSlotRegion) == false
                && checkCursor(&minimapRegion) == false
                )
            {
                if (GUI::getLastGUI() == HUD::ins())
                {
                    int tgtX = getAbsMouseGrid().x;
                    int tgtY = getAbsMouseGrid().y;

                    dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
                    dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
                    dst.w = tileSize;
                    dst.h = tileSize;
                    setZoom(zoomScale);
                    drawSpriteCenter
                    (
                        spr::whiteMarker,
                        0,
                        dst.x + dst.w / 2,
                        dst.y + dst.h / 2
                    );
                    setZoom(1.0);
                }
            }
        }
    }

    for (int i = 1; i < aStarTrail.size(); i++)
    {
        int tgtX = aStarTrail[i].x;
        int tgtY = aStarTrail[i].y;
        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
        dst.w = tileSize;
        dst.h = tileSize;
        setZoom(zoomScale);
        drawSpriteCenter
        (
            spr::trail,
            0,
            dst.x + dst.w / 2,
            dst.y + dst.h / 2
        );
        setZoom(1.0);
    }

    // 게임패드 화이트마커 그리기
    if (option::inputMethod == input::gamepad)
    {
        if (PlayerPtr->getAniType() == aniFlag::null)
        {
            if (gamepadWhiteMarker.z == PlayerZ())
            {
                if (std::abs(gamepadWhiteMarker.x - PlayerX()) <= MARKER_LIMIT_DIST)
                {
                    if (std::abs(gamepadWhiteMarker.y - PlayerY()) <= MARKER_LIMIT_DIST)
                    {
                        int tgtX = gamepadWhiteMarker.x;
                        int tgtY = gamepadWhiteMarker.y;
                        dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
                        dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
                        dst.w = tileSize;
                        dst.h = tileSize;
                        setZoom(zoomScale);
                        drawSpriteCenter
                        (
                            spr::whiteMarker,
                            0,
                            dst.x + dst.w / 2,
                            dst.y + dst.h / 2
                        );
                        setZoom(1.0);
                    }
                }
            }
        }
    }
    return;
}

void drawDebug()
{
    if (debug::chunkLineDraw)
    {
        for (const auto& elem : tileList)
        {
            int tgtX = elem.x;
            int tgtY = elem.y;

            dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
            dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
            dst.w = tileSize;
            dst.h = tileSize;

            if (tgtX % CHUNK_SIZE_X == 0) drawLine(dst.x, dst.y, dst.x, dst.y + dst.h, col::red);
            else if (tgtX % CHUNK_SIZE_X == CHUNK_SIZE_X - 1)  drawLine(dst.x + dst.w, dst.y, dst.x + dst.w, dst.y + dst.h, col::red);

            if (tgtY % CHUNK_SIZE_Y == 0) drawLine(dst.x, dst.y, dst.x + dst.w, dst.y, col::red);
            else if (tgtY % CHUNK_SIZE_Y == CHUNK_SIZE_Y - 1)  drawLine(dst.x, dst.y + dst.h, dst.x + dst.w, dst.y + dst.h, col::red);

            if (std::abs(tgtX % CHUNK_SIZE_X) == CHUNK_SIZE_X / 2 && std::abs(tgtY % CHUNK_SIZE_Y) == CHUNK_SIZE_Y / 2)
            {
                setFontSize(20);

                drawTextCenter(L"CHUNK", dst.x + dst.w / 2, dst.y + dst.h / 2 - 12, col::red);

                int cx, cy;
                World::ins()->changeToChunkCoord(tgtX, tgtY, cx, cy);
                std::wstring chunkName = L"";
                chunkName += std::to_wstring(cx);
                chunkName += L",";
                chunkName += std::to_wstring(cy);
                chunkName += L",";
                chunkName += std::to_wstring(PlayerZ());
                drawTextCenter(chunkName, dst.x + dst.w / 2, dst.y + dst.h / 2 + 12, col::red);
            }
        }
    }
}
