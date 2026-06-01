module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import globalVar;
import constVar;
import Point;
import GUI;

void LotEditor::clickDownGUI()
{
    if (getStateInput() == false) return;
    //좌클릭만 도구 적용/선택. 우/중클릭은 여기서 배치하지 않음(turnCycleLoop가 전 버튼에 clickDown을
    //  호출하므로 가드 없으면 우클릭에도 타일이 배치되는 버그가 난다).
    if (event.button.button != SDL_BUTTON_LEFT) return;
    computeLayout();

    if (menuOpen_) { contextMenuClick(); return; }

    int mx = (int)getMouseX();
    int my = (int)getMouseY();

    //차량 속성 패널 버튼(차량 모드 + 활성차량)
    if (mode_ == EditMode::Vehicle && activeVeh_ != nullptr)
    {
        auto in = [&](const SDL_Rect& r) { return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h; };
        if (in(vehPanelRect_))
        {
            if (in(vehNameBtn_)) { if (vehNameEdit_ == false) startVehNameEdit(); }
            else if (in(vehTypeBtn_)) cycleVehType();
            else if (in(vehDirBtn_)) rotateActiveVeh();
            else if (in(vehDeselectBtn_)) { if (vehNameEdit_) stopVehNameEdit(); activeVeh_ = nullptr; }
            else if (vehNameEdit_) confirmVehName();
            return;
        }
    }
    if (vehNameEdit_) { confirmVehName(); return; } //패널 밖 클릭 = 이름 확정

    int mi = modeTabAt(mx, my);
    if (mi >= 0) { setMode((EditMode)mi); return; }
    int ti = toolBtnAt(mx, my);
    if (ti >= 0) { tool_ = (EditTool)ti; return; }
    if (mouseOverPalette()) { paletteClick(); return; }
    if (my < TOPBAR_H || my > cameraH - HINT_H) return; //상단바/힌트바 영역은 페인트 안 함

    Point3 t = cursorTile();
    if (altEyedrop_) { eyedropAt(t); return; }
    if (tool_ == EditTool::RectFill || tool_ == EditTool::RectOutline || tool_ == EditTool::Clear)
    {
        rectActive_ = true;
        rectAnchor_ = { t.x, t.y };
        return;
    }
    lastPainted_ = { t.x, t.y };
    applyToolAt(t);
}

void LotEditor::clickMotionGUI(int dx, int dy)
{
    if (getStateInput() == false) return;
    //motion 이벤트 중 event.button.button은 motion.state 하위바이트와 정렬 -> 단일버튼 드래그 판정(HUD와 동일).
    if (event.button.button == SDL_BUTTON_MIDDLE)
    {
        cameraFix = false;
        cameraX -= (int)((getMouseX() - prevMouseX4Motion) / 2.0f);
        cameraY -= (int)((getMouseY() - prevMouseY4Motion) / 2.0f);
        ensureVisibleChunks();
        return;
    }
    if (event.button.button == SDL_BUTTON_LEFT)
    {
        int my = (int)getMouseY();
        if (my < TOPBAR_H || my > cameraH - HINT_H) return;
        if (mouseOverPalette()) return;
        if (rectActive_) return;
        if (tool_ == EditTool::Brush || tool_ == EditTool::Eraser)
        {
            Point3 t = cursorTile();
            if (t.x != lastPainted_.x || t.y != lastPainted_.y)
            {
                lastPainted_ = { t.x, t.y };
                if (altEyedrop_) eyedropAt(t);
                else applyToolAt(t);
            }
        }
    }
}

void LotEditor::clickUpGUI()
{
    if (getStateInput() == false) return;
    lastPainted_ = { -2000000000, -2000000000 };
    if (rectActive_)
    {
        Point3 t = cursorTile();
        if (tool_ == EditTool::Clear) clearRect(rectAnchor_, { t.x, t.y });
        else rasterRect(rectAnchor_, { t.x, t.y }, tool_ == EditTool::RectOutline);
        rectActive_ = false;
    }
}

void LotEditor::clickRightGUI()
{
    if (getStateInput() == false) return;
    computeLayout();
    int mx = (int)getMouseX();
    int my = (int)getMouseY();
    if (my < TOPBAR_H || mouseOverPalette()) return; //패널 위 우클릭은 무시
    openEditorContextMenu(cursorTile());
}

void LotEditor::keyDownGUI()
{
    //차량 이름 입력 중엔 키를 텍스트 편집으로만 쓰고 나머지 동작 억제.
    if (vehNameEdit_)
    {
        switch (event.key.key)
        {
        case SDLK_BACKSPACE:
            if (exInputCursor > 0 && exInputText.empty() == false) { exInputText.erase(exInputCursor - 1, 1); exInputCursor--; }
            break;
        case SDLK_RETURN: case SDLK_KP_ENTER: confirmVehName(); break;
        case SDLK_ESCAPE: stopVehNameEdit(); break;
        }
        return;
    }
    //이동/Z는 키리피트가 동작하도록 keyDown에서 처리. WASD = 카메라 팬(플레이어 안 움직임), Q/E = Z층.
    switch (event.key.key)
    {
    case SDLK_W: case SDLK_UP: panCameraTiles(0, -1); break;
    case SDLK_S: case SDLK_DOWN: panCameraTiles(0, 1); break;
    case SDLK_A: case SDLK_LEFT: panCameraTiles(-1, 0); break;
    case SDLK_D: case SDLK_RIGHT: panCameraTiles(1, 0); break;
    case SDLK_Q: case SDLK_PAGEDOWN: changeZ(-1); break;
    case SDLK_E: case SDLK_PAGEUP: changeZ(1); break;
    case SDLK_LALT: case SDLK_RALT: altEyedrop_ = true; break;
    }
}

void LotEditor::keyUpGUI()
{
    if (vehNameEdit_) return; //이름 입력 중엔 단축키 억제
    switch (event.key.key)
    {
    case SDLK_1: setMode(EditMode::Floor); break;
    case SDLK_2: setMode(EditMode::Wall); break;
    case SDLK_3: setMode(EditMode::Prop); break;
    case SDLK_4: setMode(EditMode::Item); break;
    case SDLK_5: setMode(EditMode::Monster); break;
    case SDLK_6: setMode(EditMode::Vehicle); break;
    case SDLK_TAB: tool_ = (EditTool)(((int)tool_ + 1) % TOOL_COUNT); break;
    case SDLK_G: gridOn_ = !gridOn_; break;
    case SDLK_T: tileGridOn_ = !tileGridOn_; break;
    case SDLK_R: rotateSelectedProp(); break;
    case SDLK_N: if (mode_ == EditMode::Vehicle) startNewVehicle(); break;
    case SDLK_RETURN: doExport(); break;
    case SDLK_LEFTBRACKET:
    {
        Point2 c = cursorChunkXY();
        boxChunkA_ = { c.x, c.y, PlayerZ() };
        boxSet_ = true;
        break;
    }
    case SDLK_RIGHTBRACKET:
    {
        Point2 c = cursorChunkXY();
        boxChunkB_ = { c.x, c.y, PlayerZ() };
        boxSet_ = true;
        break;
    }
    case SDLK_LALT: case SDLK_RALT: altEyedrop_ = false; break;
    case SDLK_ESCAPE:
        if (menuOpen_) menuOpen_ = false;
        else close(aniFlag::winUnfoldClose);
        break;
    }
}

void LotEditor::mouseWheel()
{
    computeLayout();
    if (mouseOverPalette())
    {
        if (event.wheel.y > 0) paletteScroll_ -= 1;
        else if (event.wheel.y < 0) paletteScroll_ += 1;
        int maxS = paletteMaxScroll();
        if (paletteScroll_ > maxS) paletteScroll_ = maxS; //끝 이상으로 못 내림(빈 화면 방지)
        if (paletteScroll_ < 0) paletteScroll_ = 0;
        return;
    }
    //LotEditor에선 1.0x ~ 8.0x 허용(기본 게임은 2.0~5.0). 종료 시 savedZoom_로 복원.
    if (event.wheel.y > 0) { zoomScale += 1; if (zoomScale > 8.0f) zoomScale = 8.0f; }
    else if (event.wheel.y < 0) { zoomScale -= 1; if (zoomScale < 1.0f) zoomScale = 1.0f; }
    ensureVisibleChunks(); //줌아웃으로 더 넓어진 가시 영역 청크 확보
}
