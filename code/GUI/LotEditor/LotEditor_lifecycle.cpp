module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import GUI;
import globalVar;
import constVar;
import Point;
import World;
import Player;
import log;

LotEditor::LotEditor() : GUI(false)
{
    errorBox(ptr != nullptr, L"More than one LotEditor instance was generated.");
    ptr = this;

    saveHostState();
    buildPalettes();
    changeXY(0, 0, false);

    cameraFix = false;
    drawHUD = false; //HUD(미니맵/레터박스/탭) 숨김. 로그는 renderLog가 별도로 그려서 유지됨.
    lotEditorActive = true; //renderTile이 시야 전체공개+플레이어 숨김
    GUI::deactDrawAll(this); //alwaysDraw 아닌 다른 GUI(맵 등) 숨김

    //기본 export 박스 = 현재 플레이어가 선 청크(1x1)
    Point2 c = currentChunkXY();
    boxChunkA_ = { c.x, c.y, PlayerZ() };
    boxChunkB_ = { c.x, c.y, PlayerZ() };
    boxSet_ = true;

    ensureVisibleChunks();
    updateLog(L"activated. WASD pan, Q/E z-level, wheel zoom, G grid, Enter export, Esc exit.");
}

LotEditor::~LotEditor()
{
    restoreHostState();
    ptr = nullptr;
}

void LotEditor::saveHostState()
{
    savedCameraFix_ = cameraFix;
    savedCameraX_ = cameraX;
    savedCameraY_ = cameraY;
    savedZoom_ = zoomScale;
    savedPlayerPos_ = { PlayerX(), PlayerY(), PlayerZ() };
    gridWasOn_ = debug::chunkLineDraw;
    savedDrawHUD_ = drawHUD;
}

void LotEditor::restoreHostState()
{
    if (vehNameEdit_) stopVehNameEdit(); //이름 입력 중이었으면 SDL 텍스트입력/ exInput 해제
    //플레이어를 원래 위치로 복귀(점유 시 스킵)
    int fx = PlayerX(), fy = PlayerY(), fz = PlayerZ();
    if (fx != savedPlayerPos_.x || fy != savedPlayerPos_.y || fz != savedPlayerPos_.z)
    {
        ensureChunkAt(savedPlayerPos_.x, savedPlayerPos_.y, savedPlayerPos_.z);
        if (TileEntity(savedPlayerPos_.x, savedPlayerPos_.y, savedPlayerPos_.z) == nullptr)
        {
            EntityPtrMove({ fx, fy, fz }, savedPlayerPos_);
            PlayerPtr->updateVision();
            PlayerPtr->updateMinimap();
        }
    }
    cameraFix = savedCameraFix_;
    cameraX = savedCameraX_;
    cameraY = savedCameraY_;
    zoomScale = savedZoom_;
    debug::chunkLineDraw = gridWasOn_;
    drawHUD = savedDrawHUD_;
    lotEditorActive = false;
    lotEditorHoverVeh = nullptr;
    PlayerPtr->updateVision(); //복귀 후 플레이어 시야 재계산
    GUI::actDrawAll(this);
}

void LotEditor::changeXY(int inputX, int inputY, bool center)
{
    //전체화면 오버레이라 위치는 의미 없음(애니 베이스용으로만 보관)
    x = inputX;
    y = inputY;
}

void LotEditor::step()
{
    //매 스텝 카메라 고정 해제 유지(자유 팬). 턴 진행 함수는 절대 호출하지 않는다.
    cameraFix = false;
    ensureVisibleChunks(); //창 크기 변경 등으로 넓어진 영역 대비(렌더 전 청크 보장)

    //커서가 올라간 차량 기록 -> Vehicle_drawSelf가 그 차량 천장을 반투명하게(편집 중 내부 보기).
    Point2 g = getAbsMouseGrid();
    int hcx, hcy;
    World::ins()->changeToChunkCoord(g.x, g.y, hcx, hcy);
    if (World::ins()->existChunk(hcx, hcy, PlayerZ())) lotEditorHoverVeh = (void*)TileVehicle(g.x, g.y, PlayerZ());
    else lotEditorHoverVeh = nullptr;
}
