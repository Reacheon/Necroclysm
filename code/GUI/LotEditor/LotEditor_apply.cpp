module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import globalVar;
import constVar;
import Point;
import World;
import Player;
import Prop;
import ItemData;
import ItemPocket;
import ItemStack;
import Entity;
import Monster;
import Chunk;
import EntityData;
import Vehicle;
import log;

void LotEditor::ensureChunkAt(int x, int y, int z)
{
    //World::getTile()은 청크 누락 시 throw. 쓰기 전 반드시 청크 보장.
    int cx, cy;
    World::ins()->changeToChunkCoord(x, y, cx, cy);
    if (World::ins()->existChunk(cx, cy, z) == false) World::ins()->createChunk(cx, cy, z);
}

//카메라가 비추는 영역의 청크를 전부 생성. renderTile analyseRender가 getTile()로 해당 영역을
//  훑으므로(청크 없으면 throw), 자유 팬으로 미로드 영역에 들어가기 전에 미리 깔아둬야 크래시 방지.
void LotEditor::ensureVisibleChunks()
{
    double halfW = (cameraW / 2.0) / zoomScale;
    double halfH = (cameraH / 2.0) / zoomScale;
    int leftTileX = (int)std::floor((cameraX - halfW) / 16.0) - 2;
    int rightTileX = (int)std::ceil((cameraX + halfW) / 16.0) + 2;
    int topTileY = (int)std::floor((cameraY - halfH) / 16.0) - 2;
    int botTileY = (int)std::ceil((cameraY + halfH) / 16.0) + 2;
    int z = PlayerZ();
    int ca, da, cb, db;
    World::ins()->changeToChunkCoord(leftTileX, topTileY, ca, da);
    World::ins()->changeToChunkCoord(rightTileX, botTileY, cb, db);
    for (int cy = da; cy <= db; ++cy)
        for (int cx = ca; cx <= cb; ++cx)
            if (World::ins()->existChunk(cx, cy, z) == false) World::ins()->createChunk(cx, cy, z);
}

Point2 LotEditor::currentChunkXY()
{
    int cx, cy;
    World::ins()->changeToChunkCoord(PlayerX(), PlayerY(), cx, cy);
    return { cx, cy };
}

//커서 아래 타일이 속한 청크 좌표(카메라가 플레이어와 분리되어 박스 모서리는 커서 기준).
Point2 LotEditor::cursorChunkXY()
{
    Point2 g = getAbsMouseGrid();
    int cx, cy;
    World::ins()->changeToChunkCoord(g.x, g.y, cx, cy);
    return { cx, cy };
}

void LotEditor::setMode(EditMode m)
{
    if (mode_ != m) paletteScroll_ = 0; //모드 전환 시 스크롤 리셋(버킷 길이 달라 잔여 스크롤이 빈 화면 유발)
    mode_ = m;
}

Point3 LotEditor::cursorTile()
{
    Point2 g = getAbsMouseGrid();
    return { g.x, g.y, PlayerZ() };
}

//Z층 변경. 렌더 Z = PlayerZ() 이므로 플레이어를 제자리에서 위/아래로만 이동. 카메라는 안 건드림
//  (같은 X/Y 영역의 다른 층을 본다). 시야는 lotEditorActive로 renderTile에서 전체 공개.
void LotEditor::changeZ(int dz)
{
    if (dz == 0) return;
    int px = PlayerX(), py = PlayerY(), pz = PlayerZ();
    int nz = pz + dz;
    ensureChunkAt(px, py, nz);
    if (TileEntity(px, py, nz) != nullptr)
    {
        updateLog(L"[LotEditor] Z move blocked: entity there");
        return;
    }
    EntityPtrMove({ px, py, pz }, { px, py, nz });
    ensureVisibleChunks(); //새 Z층의 가시 영역 청크 확보
}

//카메라 자유 팬(플레이어 이동 없음). 1타일 = 16 월드픽셀.
void LotEditor::panCameraTiles(int dx, int dy)
{
    cameraFix = false;
    cameraX += dx * 16;
    cameraY += dy * 16;
    ensureVisibleChunks();
}

int LotEditor::currentSelCode()
{
    switch (mode_)
    {
    case EditMode::Floor: return selFloor_;
    case EditMode::Wall: return selWall_;
    case EditMode::Prop: return selProp_;
    case EditMode::Item: return selItem_;
    case EditMode::Monster: return selMonster_;
    case EditMode::Vehicle: return selVeh_;
    default: return itemID::none;
    }
}

void LotEditor::setSelCode(int code)
{
    switch (mode_)
    {
    case EditMode::Floor: selFloor_ = code; break;
    case EditMode::Wall: selWall_ = code; break;
    case EditMode::Prop: selProp_ = code; break;
    case EditMode::Item: selItem_ = code; break;
    case EditMode::Monster: selMonster_ = code; break;
    case EditMode::Vehicle: selVeh_ = code; break;
    default: break;
    }
}

void LotEditor::applyFloor(Point3 t, int code)
{
    ensureChunkAt(t.x, t.y, t.z);
    setFloor(t, code);
}

void LotEditor::applyWall(Point3 t, int code)
{
    ensureChunkAt(t.x, t.y, t.z);
    if (code == itemID::none) { DestroyWall(t.x, t.y, t.z); return; }
    setWall(t, code);
}

void LotEditor::applyProp(Point3 t, int code)
{
    ensureChunkAt(t.x, t.y, t.z);
    if (TileProp(t.x, t.y, t.z) != nullptr) destroyProp(t);
    if (code != itemID::none) createProp(t, code);
}

void LotEditor::applyByMode(Point3 t)
{
    switch (mode_)
    {
    case EditMode::Floor: applyFloor(t, selFloor_); break;
    case EditMode::Wall: applyWall(t, selWall_); break;
    case EditMode::Prop: applyProp(t, selProp_); break;
    case EditMode::Monster: applyMonster(t); break;
    case EditMode::Item: applyItem(t); break;
    case EditMode::Vehicle: applyVehicle(t); break;
    default: break;
    }
}

void LotEditor::eraseAt(Point3 t)
{
    ensureChunkAt(t.x, t.y, t.z);
    switch (mode_)
    {
    case EditMode::Floor: setFloor(t, itemID::none); break;
    case EditMode::Wall: DestroyWall(t.x, t.y, t.z); break;
    case EditMode::Prop: if (TileProp(t.x, t.y, t.z) != nullptr) destroyProp(t); break;
    case EditMode::Item:
    {
        int kind = 0;
        ItemPocket* pk = pocketAt(t, &kind);
        if (pk != nullptr && pk->itemInfo.empty() == false)
        {
            pk->subtractItemIndex((int)pk->itemInfo.size() - 1, 1); //top 1개만 제거
            if (kind == 0) //바닥 스택만 비면 제거(프롭/차량 cargo 포켓은 유지)
            {
                ItemStack* s = TileItemStack(t.x, t.y, t.z);
                if (s != nullptr && s->getPocket()->getPocketNumber() == 0) destroyItemStack(t);
            }
        }
        break;
    }
    case EditMode::Monster: removeMonsterAt(t); break;
    case EditMode::Vehicle:
    {
        if (activeVeh_ != nullptr && activeVeh_->getGridZ() == t.z)
        {
            Point3 key = { t.x, t.y, t.z };
            auto it = activeVeh_->partInfo.find(key);
            if (it != activeVeh_->partInfo.end() && it->second != nullptr)
            {
                int n = (int)it->second->itemInfo.size();
                if (n > 1) { activeVeh_->erasePart(t.x, t.y, n - 1); activeVeh_->updateSpr(); } //장착부품 top 제거
                else //프레임만 남은 타일 -> 타일 자체를 차량에서 제거(erasePart는 프레임 타일을 못 없앰)
                {
                    activeVeh_->partInfo.erase(key);
                    TileVehicle(t.x, t.y, t.z) = nullptr;
                    if (activeVeh_->partInfo.empty())
                    {
                        World::ins()->destroyVehicle(activeVeh_); //빈 차량은 소멸(소멸자가 청크/타일 정리)
                        activeVeh_ = nullptr;
                    }
                    else activeVeh_->updateSpr();
                }
            }
        }
        break;
    }
    default: break;
    }
}

void LotEditor::eyedropAt(Point3 t)
{
    switch (mode_)
    {
    case EditMode::Floor:
    {
        int f = TileFloor(t.x, t.y, t.z);
        if (f != itemID::none) selFloor_ = f;
        break;
    }
    case EditMode::Wall:
    {
        int w = TileWall(t.x, t.y, t.z);
        if (w != itemID::none) selWall_ = w;
        break;
    }
    case EditMode::Prop:
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr) selProp_ = p->leadItem.itemCode;
        break;
    }
    case EditMode::Monster:
    {
        Entity* e = TileEntity(t.x, t.y, t.z);
        if (e != nullptr && e != (Entity*)PlayerPtr) selMonster_ = e->entityInfo.entityCode;
        break;
    }
    case EditMode::Item:
    {
        ItemPocket* pk = pocketAt(t);
        if (pk != nullptr && pk->itemInfo.empty() == false) selItem_ = pk->itemInfo.back().itemCode;
        break;
    }
    case EditMode::Vehicle:
    {
        Vehicle* v = TileVehicle(t.x, t.y, t.z);
        if (v != nullptr)
        {
            auto it = v->partInfo.find({ t.x, t.y, v->getGridZ() });
            if (it != v->partInfo.end() && it->second != nullptr && it->second->itemInfo.empty() == false)
                selVeh_ = it->second->itemInfo.back().itemCode;
        }
        break;
    }
    default: break;
    }
}

void LotEditor::applyItem(Point3 t)
{
    if (selItem_ == itemID::none) return;
    ensureChunkAt(t.x, t.y, t.z);
    ItemPocket* pk = pocketAt(t); //컨테이너 프롭 / 차량 cargo / 바닥 스택
    if (pk != nullptr) { pk->addItemFromDex(selItem_, 1); return; }
    //컨테이너 없음: 차량 타일이면 바닥 스택 만들지 말고 안내(컨테이너 부품 필요).
    if (TileVehicle(t.x, t.y, t.z) != nullptr) { updateLog(L"[LotEditor] No container part on this vehicle tile"); return; }
    addItemToTile(t, selItem_, 1); //스택 없으면 생성 후 추가(검증된 헬퍼)
}

//타일의 편집 대상 포켓: 컨테이너 프롭(1) > 차량 cargo 부품(2) > 바닥 스택(0) 순. 없으면 nullptr.
ItemPocket* LotEditor::pocketAt(Point3 t, int* outKind)
{
    if (outKind != nullptr) *outKind = 0;
    Prop* p = TileProp(t.x, t.y, t.z);
    if (p != nullptr && p->leadItem.pocketPtr != nullptr)
    {
        if (outKind != nullptr) *outKind = 1;
        return p->leadItem.pocketPtr.get();
    }
    //차량 컨테이너 부품(바구니/연료탱크 등) - 부품 ItemData의 pocketPtr가 cargo 저장소.
    Vehicle* v = TileVehicle(t.x, t.y, t.z);
    if (v != nullptr)
    {
        auto it = v->partInfo.find({ t.x, t.y, v->getGridZ() });
        if (it != v->partInfo.end() && it->second != nullptr)
        {
            for (ItemData& part : it->second->itemInfo)
            {
                if (part.pocketMaxVolume > 0 && part.pocketPtr != nullptr)
                {
                    if (outKind != nullptr) *outKind = 2;
                    return part.pocketPtr.get();
                }
            }
        }
    }
    ItemStack* s = TileItemStack(t.x, t.y, t.z);
    return s != nullptr ? s->getPocket() : nullptr;
}

void LotEditor::applyMonster(Point3 t)
{
    ensureChunkAt(t.x, t.y, t.z);
    Entity* e = TileEntity(t.x, t.y, t.z);
    if (e == (Entity*)PlayerPtr) return; //플레이어(앵커) 타일엔 배치 금지
    if (e != nullptr) removeMonsterAt(t);
    if (selMonster_ > 0) createMonster(t, selMonster_);
}

//에디터용 몬스터 제거: death()의 시체/드롭 없이 chunk monsterSet에서 빼고 EntityPtr만 해제.
void LotEditor::removeMonsterAt(Point3 t)
{
    Entity* e = TileEntity(t.x, t.y, t.z);
    if (e == nullptr || e == (Entity*)PlayerPtr) return;
    Monster* m = static_cast<Monster*>(e); //플레이어 외 타일 엔티티는 Monster.
    int cx, cy;
    World::ins()->changeToChunkCoord(t.x, t.y, cx, cy);
    World::ins()->getChunk(cx, cy, t.z).eraseMonster(m);
    World::ins()->getTile(t.x, t.y, t.z).EntityPtr.reset();
}

//차량 편집 디스패치. 선택된 게 VFRAME이면 프레임(새 차량/확장), VPART면 부품 추가.
//  편집 범위 = activeVeh_ 한 대. 다른 차량 클릭 시 그 차량으로 활성 전환.
void LotEditor::applyVehicle(Point3 t)
{
    ensureChunkAt(t.x, t.y, t.z);
    Vehicle* vAt = TileVehicle(t.x, t.y, t.z);
    bool isFrame = itemDex[selVeh_].checkFlag(itemFlag::VFRAME);

    //새 차량 시작 대기 중: 빈 타일 + 프레임이면 생성.
    if (newVehPending_)
    {
        if (vAt != nullptr) { updateLog(L"[LotEditor] Tile already has a vehicle"); return; }
        if (isFrame == false) { updateLog(L"[LotEditor] New vehicle needs a frame selected"); return; }
        Vehicle* v = World::ins()->createVehicle(t.x, t.y, t.z, selVeh_);
        activeVeh_ = v;
        newVehPending_ = false;
        v->updateSpr();
        return;
    }

    //다른 차량을 클릭하면 그 차량을 활성(편집 범위)으로 전환.
    if (vAt != nullptr && vAt != activeVeh_)
    {
        activeVeh_ = vAt;
        return;
    }

    if (activeVeh_ == nullptr) { updateLog(L"[LotEditor] Right-click New vehicle, or click a vehicle to select"); return; }
    if (activeVeh_->getGridZ() != t.z) { updateLog(L"[LotEditor] Wrong Z layer for this vehicle (Q/E)"); return; }

    if (isFrame)
    {
        if (activeVeh_->hasFrame(t.x, t.y)) return; //이미 프레임 있음
        //extendPart는 인접 프레임 없거나 벽 위면 errorBox -> 미리 가드해 조용히 로그.
        bool adj = activeVeh_->hasFrame(t.x + 1, t.y) || activeVeh_->hasFrame(t.x - 1, t.y)
            || activeVeh_->hasFrame(t.x, t.y + 1) || activeVeh_->hasFrame(t.x, t.y - 1);
        if (adj == false) { updateLog(L"[LotEditor] Frame must be adjacent (right-click = new vehicle)"); return; }
        if (ExistWall(t.x, t.y, t.z)) { updateLog(L"[LotEditor] Can't extend frame onto a wall"); return; }
        activeVeh_->extendPart(t.x, t.y, selVeh_);
        activeVeh_->updateSpr();
    }
    else //VPART
    {
        if (activeVeh_->hasFrame(t.x, t.y)) { activeVeh_->addPart(t.x, t.y, selVeh_); activeVeh_->updateSpr(); }
        else updateLog(L"[LotEditor] Parts go on frame tiles only");
    }
}

void LotEditor::startNewVehicle()
{
    newVehPending_ = true;
    activeVeh_ = nullptr;
    updateLog(L"[LotEditor] New vehicle: click an empty tile with a frame");
}

void LotEditor::cycleVehType()
{
    if (activeVeh_ == nullptr) return;
    activeVeh_->vehType = (vehFlag)(((int)activeVeh_->vehType + 1) % 6); //none/car/heli/minecart/train/ship
}

void LotEditor::rotateActiveVeh()
{
    if (activeVeh_ == nullptr) return;
    activeVeh_->rotate((dir16)(((int)activeVeh_->bodyDir + 4) % 16)); //CCW 90도(=4*22.5)
}

//차량 이름 텍스트 입력 시작/확정/종료 - Msg GUI와 동일한 exInput 패턴.
void LotEditor::startVehNameEdit()
{
    if (activeVeh_ == nullptr) return;
    exInput = true;
    exInputText = activeVeh_->name;
    exInputCursor = (int)exInputText.size();
    exInputEditing = false;
    exInputIndex = 0;
    SDL_StartTextInput(window);
    vehNameEdit_ = true;
}

void LotEditor::confirmVehName()
{
    if (vehNameEdit_ == false) return;
    if (activeVeh_ != nullptr && exInputText.empty() == false) activeVeh_->name = exInputText;
    stopVehNameEdit();
}

void LotEditor::stopVehNameEdit()
{
    exInput = false;
    exInputCursor = 0;
    exInputEditing = false;
    exInputIndex = -1;
    SDL_StopTextInput(window);
    vehNameEdit_ = false;
}

void LotEditor::applyToolAt(Point3 t)
{
    if (altEyedrop_) { eyedropAt(t); return; }
    switch (tool_)
    {
    case EditTool::Brush: applyByMode(t); break;
    case EditTool::Eraser: eraseAt(t); break;
    case EditTool::Eyedropper: eyedropAt(t); break;
    default: break; //Rect/Clear: clickUp에서 처리
    }
}

void LotEditor::rasterRect(Point2 a, Point2 b, bool outline)
{
    int x0 = std::min(a.x, b.x), x1 = std::max(a.x, b.x);
    int y0 = std::min(a.y, b.y), y1 = std::max(a.y, b.y);
    int z = PlayerZ();
    for (int ty = y0; ty <= y1; ++ty)
        for (int tx = x0; tx <= x1; ++tx)
        {
            bool border = (tx == x0 || tx == x1 || ty == y0 || ty == y1);
            if (outline == false || border) applyByMode({ tx, ty, z });
        }
}

void LotEditor::clearRect(Point2 a, Point2 b)
{
    int x0 = std::min(a.x, b.x), x1 = std::max(a.x, b.x);
    int y0 = std::min(a.y, b.y), y1 = std::max(a.y, b.y);
    int z = PlayerZ();
    for (int ty = y0; ty <= y1; ++ty)
        for (int tx = x0; tx <= x1; ++tx)
        {
            ensureChunkAt(tx, ty, z);
            setFloor({ tx, ty, z }, itemID::none);
            DestroyWall(tx, ty, z);
            if (TileProp(tx, ty, z) != nullptr) destroyProp({ tx, ty, z });
            if (TileItemStack(tx, ty, z) != nullptr) destroyItemStack({ tx, ty, z });
            removeMonsterAt({ tx, ty, z });
        }
}

void LotEditor::rotateSelectedProp()
{
    if (selProp_ == itemID::none) return;
    int r = itemDex[selProp_].rotatedCCW90ItemCode;
    if (r != 0) selProp_ = r;
}
