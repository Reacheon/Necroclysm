module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import globalVar;
import constVar;
import Point;
import GUI;
import drawText;
import drawPrimitive;
import drawSprite;
import textureVar;
import ItemData;
import EntityData;
import ItemPocket;
import ItemStack;
import Prop;
import Vehicle;
import World;

void LotEditor::computeLayout()
{
    int mw = 92, mh = 22;
    for (int i = 0; i < MODE_COUNT; ++i) modeTabRect_[i] = { 6 + i * (mw + 2), 4, mw, mh };
    int tw = 84, th = 22;
    for (int i = 0; i < TOOL_COUNT; ++i) toolBtnRect_[i] = { 6 + i * (tw + 2), 30, tw, th };
    palettePanelRect_ = { cameraW - PAL_W, TOPBAR_H, PAL_W, cameraH - TOPBAR_H - HINT_H };
    paletteGridRect_ = { palettePanelRect_.x + 6, palettePanelRect_.y + 22, palettePanelRect_.w - 12, palettePanelRect_.h - 26 };

    //차량 속성 패널(좌상단, 차량 모드+활성차량일 때만 표시/판정)
    int vpx = 6, vpy = TOPBAR_H + 6, vpw = 252, vph = 100;
    vehPanelRect_ = { vpx, vpy, vpw, vph };
    vehNameBtn_ = { vpx + 50, vpy + 4, vpw - 58, 22 }; //"Name" 라벨 오른쪽 값 입력칸만
    vehTypeBtn_ = { vpx + vpw - 92, vpy + 30, 86, 18 };
    vehDirBtn_ = { vpx + vpw - 92, vpy + 52, 86, 18 };
    vehDeselectBtn_ = { vpx + 4, vpy + 74, 110, 20 };
}

int LotEditor::modeTabAt(int mx, int my)
{
    for (int i = 0; i < MODE_COUNT; ++i)
        if (mx >= modeTabRect_[i].x && mx < modeTabRect_[i].x + modeTabRect_[i].w && my >= modeTabRect_[i].y && my < modeTabRect_[i].y + modeTabRect_[i].h) return i;
    return -1;
}

int LotEditor::toolBtnAt(int mx, int my)
{
    for (int i = 0; i < TOOL_COUNT; ++i)
        if (mx >= toolBtnRect_[i].x && mx < toolBtnRect_[i].x + toolBtnRect_[i].w && my >= toolBtnRect_[i].y && my < toolBtnRect_[i].y + toolBtnRect_[i].h) return i;
    return -1;
}

void LotEditor::tileToScreenCenter(int tx, int ty, int& sx, int& sy)
{
    sx = cameraW / 2 + (int)(zoomScale * (16 * tx + 8 - cameraX));
    sy = cameraH / 2 + (int)(zoomScale * (16 * ty + 8 - cameraY));
}

void LotEditor::drawGUI()
{
    if (getStateDraw() == false) return;
    computeLayout();
    drawTileGrid();
    drawChunkGrid();
    drawExportBox();
    drawActiveVehHighlight();
    drawPreview();
    drawTopBar();
    drawPalette();
    drawInspector();
    drawVehPanel();
    drawHintBar();
    drawContextMenu();
    drawTooltip();
}

void LotEditor::drawTopBar()
{
    SDL_Rect bar = { 0, 0, cameraW, TOPBAR_H };
    drawFillRect(bar, SDL_Color{ 22, 24, 30, 255 }, 244);
    drawLine(0, TOPBAR_H, cameraW, TOPBAR_H, SDL_Color{ 74, 80, 102 }, 255);

    int mx = (int)getMouseX();
    int my = (int)getMouseY();

    const wchar_t* modeNames[MODE_COUNT] = { L"Floor", L"Wall", L"Prop", L"Item", L"Monster", L"Vehicle" };
    setFont(fontType::mainFont);
    for (int i = 0; i < MODE_COUNT; ++i)
    {
        SDL_Rect& rc = modeTabRect_[i];
        bool cur = ((int)mode_ == i);
        bool hover = (mx >= rc.x && mx < rc.x + rc.w && my >= rc.y && my < rc.y + rc.h);
        drawFillRect(rc, cur ? SDL_Color{ 58, 110, 180, 255 } : (hover ? SDL_Color{ 46, 52, 70, 255 } : SDL_Color{ 33, 36, 47, 255 }), 255);
        drawRect(rc, cur ? SDL_Color{ 130, 185, 255, 255 } : SDL_Color{ 62, 68, 88, 255 }, 255);
        setFontSize(15);
        drawTextCenter(modeNames[i], rc.x + rc.w / 2, rc.y + rc.h / 2 - 1, cur ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 200, 205, 215, 255 });
    }

    const wchar_t* toolNames[TOOL_COUNT] = { L"Brush", L"Rect", L"RectLine", L"Eyedrop", L"Eraser", L"Clear" };
    for (int i = 0; i < TOOL_COUNT; ++i)
    {
        SDL_Rect& rc = toolBtnRect_[i];
        bool cur = ((int)tool_ == i);
        bool hover = (mx >= rc.x && mx < rc.x + rc.w && my >= rc.y && my < rc.y + rc.h);
        drawFillRect(rc, cur ? SDL_Color{ 150, 110, 50, 255 } : (hover ? SDL_Color{ 46, 52, 70, 255 } : SDL_Color{ 33, 36, 47, 255 }), 255);
        drawRect(rc, cur ? SDL_Color{ 230, 180, 90, 255 } : SDL_Color{ 62, 68, 88, 255 }, 255);
        setFontSize(14);
        drawTextCenter(toolNames[i], rc.x + rc.w / 2, rc.y + rc.h / 2 - 1, cur ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 200, 205, 215, 255 });
    }

    //우측 readout(Z / zoom / 커서청크 / 선택 아이템)
    Point2 g = getAbsMouseGrid();
    int ccx, ccy;
    World::ins()->changeToChunkCoord(g.x, g.y, ccx, ccy);
    int sel = currentSelCode();
    std::wstring selName = (sel != itemID::none && sel >= 0 && sel < (int)itemDex.size() && mode_ != EditMode::Monster)
        ? itemDex[sel].name
        : (mode_ == EditMode::Monster && sel >= 0 && sel < (int)entityDex.size() ? entityDex[sel].name : L"(none)");

    std::wstring line1 = L"Z " + std::to_wstring(PlayerZ()) + L"   Zoom " + std::to_wstring((int)(zoomScale + 0.01f)) + L"x   Chunk (" + std::to_wstring(ccx) + L"," + std::to_wstring(ccy) + L")";
    //"#"는 drawText에서 컬러코드(#RRGGBB)로 오인되어 뒤 7글자를 먹어버리므로 리터럴 # 사용 금지
    std::wstring line2 = L"Sel " + std::to_wstring(sel) + L"  " + selName;

    setFont(fontType::mainFont);
    setFontSize(14);
    int w1 = getTextWidthWithoutColor(line1);
    int w2 = getTextWidthWithoutColor(line2);
    drawText(line1, cameraW - w1 - 10, 6, SDL_Color{ 200, 210, 230, 255 });
    drawText(line2, cameraW - w2 - 10, 30, SDL_Color{ 230, 220, 150, 255 });
}

void LotEditor::drawTileGrid()
{
    if (tileGridOn_ == false) return;
    SDL_Rect worldRect = { 0, TOPBAR_H, cameraW - PAL_W, cameraH - TOPBAR_H - HINT_H };
    double halfW = (cameraW / 2.0) / zoomScale;
    double halfH = (cameraH / 2.0) / zoomScale;
    int leftTileX = (int)std::floor((cameraX - halfW) / 16.0) - 1;
    int rightTileX = (int)std::ceil((cameraX + halfW) / 16.0) + 1;
    int topTileY = (int)std::floor((cameraY - halfH) / 16.0) - 1;
    int botTileY = (int)std::ceil((cameraY + halfH) / 16.0) + 1;
    SDL_Color g = { 255, 255, 255, 255 };
    auto sX = [&](int tc) { return cameraW / 2 + (int)(zoomScale * (16 * tc - cameraX)); };
    auto sY = [&](int tc) { return cameraH / 2 + (int)(zoomScale * (16 * tc - cameraY)); };
    for (int tx = leftTileX; tx <= rightTileX; ++tx)
    {
        int sx = sX(tx);
        if (sx < worldRect.x || sx > worldRect.x + worldRect.w) continue;
        //흰색 옅게 1px(회색은 거의 안 보였고, 2px은 너무 두꺼움).
        drawLine(sx, worldRect.y, sx, worldRect.y + worldRect.h, g, 72);
    }
    for (int ty = topTileY; ty <= botTileY; ++ty)
    {
        int sy = sY(ty);
        if (sy < worldRect.y || sy > worldRect.y + worldRect.h) continue;
        drawLine(worldRect.x, sy, worldRect.x + worldRect.w, sy, g, 72);
    }
}

void LotEditor::drawChunkGrid()
{
    if (gridOn_ == false) return;

    SDL_Rect worldRect = { 0, TOPBAR_H, cameraW - PAL_W, cameraH - TOPBAR_H - HINT_H };

    //화면에 보이는 타일 코너 X/Y 범위 역산
    double halfW = (cameraW / 2.0) / zoomScale;
    double halfH = (cameraH / 2.0) / zoomScale;
    int leftTileX = (int)std::floor((cameraX - halfW) / 16.0) - 1;
    int rightTileX = (int)std::ceil((cameraX + halfW) / 16.0) + 1;
    int topTileY = (int)std::floor((cameraY - halfH) / 16.0) - 1;
    int botTileY = (int)std::ceil((cameraY + halfH) / 16.0) + 1;

    SDL_Color lineCol = { 80, 200, 220, 255 };

    auto screenX = [&](int tileCornerX) { return cameraW / 2 + (int)(zoomScale * (16 * tileCornerX - cameraX)); };
    auto screenY = [&](int tileCornerY) { return cameraH / 2 + (int)(zoomScale * (16 * tileCornerY - cameraY)); };

    int firstCX = (int)std::floor(leftTileX / 24.0) * 24;
    for (int tx = firstCX; tx <= rightTileX; tx += CHUNK_SIZE_X)
    {
        int sx = screenX(tx);
        if (sx < worldRect.x || sx > worldRect.x + worldRect.w) continue;
        drawLine(sx, worldRect.y, sx, worldRect.y + worldRect.h, lineCol, 150);
    }
    int firstCY = (int)std::floor(topTileY / 24.0) * 24;
    for (int ty = firstCY; ty <= botTileY; ty += CHUNK_SIZE_Y)
    {
        int sy = screenY(ty);
        if (sy < worldRect.y || sy > worldRect.y + worldRect.h) continue;
        drawLine(worldRect.x, sy, worldRect.x + worldRect.w, sy, lineCol, 150);
    }

    //청크 좌표 라벨 - 각 청크 우상단 코너 안쪽에 작게
    setFont(fontType::pixel);
    setFontSize(12);
    for (int cyTile = firstCY; cyTile <= botTileY; cyTile += CHUNK_SIZE_Y)
    {
        for (int cxTile = firstCX; cxTile <= rightTileX; cxTile += CHUNK_SIZE_X)
        {
            int cx, cy;
            World::ins()->changeToChunkCoord(cxTile, cyTile, cx, cy);
            int lblX = screenX(cxTile + CHUNK_SIZE_X) - 4;
            int lblY = screenY(cyTile) + 2;
            std::wstring lbl = std::to_wstring(cx) + L"," + std::to_wstring(cy);
            int lw = getTextWidthWithoutColor(lbl);
            if (lblX - lw < worldRect.x || lblX > worldRect.x + worldRect.w) continue;
            if (lblY < worldRect.y || lblY + 12 > worldRect.y + worldRect.h) continue;
            drawText(lbl, lblX - lw, lblY, SDL_Color{ 120, 220, 235, 220 });
        }
    }
}

void LotEditor::drawPreview()
{
    if (menuOpen_) return; //컨텍스트 메뉴 열려있으면 타일 커서 프리뷰 생략(메뉴 선택 중)
    int my = (int)getMouseY();
    if (my < TOPBAR_H || my > cameraH - HINT_H) return;
    if (mouseOverPalette()) return;
    //차량 속성 패널 위에선 커서 프리뷰(하늘색 사각형) 생략
    if (mode_ == EditMode::Vehicle && activeVeh_ != nullptr)
    {
        int mx = (int)getMouseX();
        if (mx >= vehPanelRect_.x && mx < vehPanelRect_.x + vehPanelRect_.w && my >= vehPanelRect_.y && my < vehPanelRect_.y + vehPanelRect_.h) return;
    }

    int ts = (int)(16 * zoomScale);
    SDL_Color c = { 0, 255, 0, 255 };
    switch (mode_)
    {
    case EditMode::Wall: c = { 80, 160, 255, 255 }; break;
    case EditMode::Prop: c = { 255, 220, 60, 255 }; break;
    case EditMode::Item: c = { 255, 130, 255, 255 }; break;
    case EditMode::Monster: c = { 255, 80, 80, 255 }; break;
    case EditMode::Vehicle: c = { 130, 255, 255, 255 }; break;
    default: break;
    }
    if (tool_ == EditTool::Eraser || altEyedrop_) c = { 255, 60, 60, 255 };

    Point2 g = getAbsMouseGrid();
    if (rectActive_)
    {
        int x0 = std::min(rectAnchor_.x, g.x), x1 = std::max(rectAnchor_.x, g.x);
        int y0 = std::min(rectAnchor_.y, g.y), y1 = std::max(rectAnchor_.y, g.y);
        int sx0, sy0, sx1, sy1;
        tileToScreenCenter(x0, y0, sx0, sy0);
        tileToScreenCenter(x1, y1, sx1, sy1);
        SDL_Rect r = { sx0 - ts / 2, sy0 - ts / 2, (sx1 - sx0) + ts, (sy1 - sy0) + ts };
        drawRect(r, c, 255);
    }
    else
    {
        int sx, sy;
        tileToScreenCenter(g.x, g.y, sx, sy);
        SDL_Rect r = { sx - ts / 2, sy - ts / 2, ts, ts };
        drawRect(r, c, 255);
    }
}

void LotEditor::drawExportBox()
{
    if (boxSet_ == false) return;
    int caX = std::min(boxChunkA_.x, boxChunkB_.x);
    int cbX = std::max(boxChunkA_.x, boxChunkB_.x);
    int caY = std::min(boxChunkA_.y, boxChunkB_.y);
    int cbY = std::max(boxChunkA_.y, boxChunkB_.y);
    int tx0 = caX * CHUNK_SIZE_X;
    int ty0 = caY * CHUNK_SIZE_Y;
    int tx1 = (cbX + 1) * CHUNK_SIZE_X;
    int ty1 = (cbY + 1) * CHUNK_SIZE_Y;
    int sx0 = cameraW / 2 + (int)(zoomScale * (16 * tx0 - cameraX));
    int sy0 = cameraH / 2 + (int)(zoomScale * (16 * ty0 - cameraY));
    int sx1 = cameraW / 2 + (int)(zoomScale * (16 * tx1 - cameraX));
    int sy1 = cameraH / 2 + (int)(zoomScale * (16 * ty1 - cameraY));
    SDL_Rect r = { sx0, sy0, sx1 - sx0, sy1 - sy0 };
    drawRect(r, SDL_Color{ 255, 120, 40, 255 }, 255);
    SDL_Rect r2 = { sx0 - 1, sy0 - 1, sx1 - sx0 + 2, sy1 - sy0 + 2 };
    drawRect(r2, SDL_Color{ 255, 120, 40, 160 }, 255);
}

void LotEditor::drawActiveVehHighlight()
{
    if (mode_ != EditMode::Vehicle || activeVeh_ == nullptr) return;
    int ts = (int)(16 * zoomScale);
    SDL_Color c = { 120, 255, 255, 235 };
    //합쳐진 footprint의 외곽선만(직교다각형): 이웃이 차량 아닌 변만 그림 -> 분리된 칸이 아니라 한 대로 보임.
    for (auto& kv : activeVeh_->partInfo)
    {
        const Point3& key = kv.first;
        if (key.z != PlayerZ()) continue;
        int sx, sy;
        tileToScreenCenter(key.x, key.y, sx, sy);
        int x0 = sx - ts / 2, y0 = sy - ts / 2, x1 = sx + ts / 2, y1 = sy + ts / 2;
        if (activeVeh_->hasFrame(key.x, key.y - 1) == false) drawLine(x0, y0, x1, y0, c, 235);
        if (activeVeh_->hasFrame(key.x, key.y + 1) == false) drawLine(x0, y1, x1, y1, c, 235);
        if (activeVeh_->hasFrame(key.x - 1, key.y) == false) drawLine(x0, y0, x0, y1, c, 235);
        if (activeVeh_->hasFrame(key.x + 1, key.y) == false) drawLine(x1, y0, x1, y1, c, 235);
    }
}

void LotEditor::drawVehPanel()
{
    if (mode_ != EditMode::Vehicle || activeVeh_ == nullptr) return;
    drawFillRect(vehPanelRect_, SDL_Color{ 18, 26, 30, 255 }, 238);
    drawRect(vehPanelRect_, SDL_Color{ 90, 150, 150, 255 }, 255);

    int px = vehPanelRect_.x;
    int py = vehPanelRect_.y;
    setFont(fontType::mainFont);
    setFontSize(15);

    //이름: "Name" 라벨은 입력칸 밖, 값만 사각형 안 - 라벨까지 지워질 것 같은 착각 방지.
    drawText(L"Name", px + 8, py + 6, SDL_Color{ 210, 230, 230, 255 });
    drawFillRect(vehNameBtn_, vehNameEdit_ ? SDL_Color{ 40, 64, 72, 255 } : SDL_Color{ 30, 40, 46, 255 }, 255);
    drawRect(vehNameBtn_, SDL_Color{ 100, 140, 150, 255 }, 255);
    std::wstring nameShown = vehNameEdit_ ? exInputText : activeVeh_->name;
    if (vehNameEdit_ && (SDL_GetTicks() / 400) % 2 == 0) nameShown += L"_";
    drawText(nameShown, vehNameBtn_.x + 6, vehNameBtn_.y + 2, SDL_Color{ 235, 245, 240, 255 });

    const wchar_t* tn[6] = { L"none", L"car", L"heli", L"minecart", L"train", L"ship" };
    int ti = (int)activeVeh_->vehType;
    if (ti < 0 || ti > 5) ti = 0;
    drawText(L"Type: " + std::wstring(tn[ti]), px + 8, py + 30, SDL_Color{ 210, 230, 230, 255 });
    drawFillRect(vehTypeBtn_, SDL_Color{ 50, 80, 90, 255 }, 255);
    drawRect(vehTypeBtn_, SDL_Color{ 120, 170, 180, 255 }, 255);
    drawTextCenter(L"cycle", vehTypeBtn_.x + vehTypeBtn_.w / 2, vehTypeBtn_.y + vehTypeBtn_.h / 2 - 1, SDL_Color{ 235, 235, 240, 255 });

    //방향을 방위 풀네임(East/North/West/South, dir0=East, index 증가=반시계)으로 표기 - 숫자보다 직관적. 90도 회전만 가능해 실제로는 4방위만 표시됨.
    const wchar_t* dn[16] = { L"East", L"East-northeast", L"Northeast", L"North-northeast", L"North", L"North-northwest", L"Northwest", L"West-northwest", L"West", L"West-southwest", L"Southwest", L"South-southwest", L"South", L"South-southeast", L"Southeast", L"East-southeast" };
    int di = (int)activeVeh_->bodyDir;
    drawText(L"Dir: " + std::wstring((di >= 0 && di < 16) ? dn[di] : L"?"), px + 8, py + 52, SDL_Color{ 210, 230, 230, 255 });
    drawFillRect(vehDirBtn_, SDL_Color{ 50, 80, 90, 255 }, 255);
    drawRect(vehDirBtn_, SDL_Color{ 120, 170, 180, 255 }, 255);
    drawTextCenter(L"rotate", vehDirBtn_.x + vehDirBtn_.w / 2, vehDirBtn_.y + vehDirBtn_.h / 2 - 1, SDL_Color{ 235, 235, 240, 255 });

    drawFillRect(vehDeselectBtn_, SDL_Color{ 70, 50, 50, 255 }, 255);
    drawRect(vehDeselectBtn_, SDL_Color{ 170, 120, 120, 255 }, 255);
    drawTextCenter(L"Deselect", vehDeselectBtn_.x + vehDeselectBtn_.w / 2, vehDeselectBtn_.y + vehDeselectBtn_.h / 2 - 1, SDL_Color{ 240, 230, 230, 255 });
}

void LotEditor::drawInspector()
{
    int my = (int)getMouseY();
    bool inWorld = (my >= TOPBAR_H && my <= cameraH - HINT_H && mouseOverPalette() == false);
    if (inWorld == false) return;

    Point3 t = cursorTile();
    ItemPocket* pk = nullptr;
    std::wstring header;
    SDL_Color headCol = { 150, 220, 255, 255 };

    if (mode_ == EditMode::Vehicle)
    {
        Vehicle* v = TileVehicle(t.x, t.y, t.z);
        if (v == nullptr) return; //차량 없는 타일은 인스펙터 생략
        auto vit = v->partInfo.find({ t.x, t.y, v->getGridZ() });
        if (vit != v->partInfo.end()) pk = vit->second.get();
        header = L"VEHICLE \"" + v->name + L"\"  parts";
        headCol = { 130, 255, 225, 255 };
    }
    else
    {
        int kind = 0;
        pk = pocketAt(t, &kind);
        if (kind == 1) { Prop* prp = TileProp(t.x, t.y, t.z); header = L"PROP: " + std::wstring(prp != nullptr ? itemDex[prp->leadItem.itemCode].name : L"?"); headCol = { 255, 210, 120, 255 }; }
        else if (kind == 2) { header = L"VEHICLE CARGO"; headCol = { 130, 255, 225, 255 }; }
        else header = L"GROUND STACK";
        bool show = (pk != nullptr && pk->itemInfo.empty() == false) || (mode_ == EditMode::Item);
        if (show == false) return;
    }

    int n = (pk != nullptr) ? (int)pk->itemInfo.size() : 0;
    header += L"   (" + std::to_wstring(t.x) + L"," + std::to_wstring(t.y) + L"," + std::to_wstring(t.z) + L")";

    int rowH = 24;
    int headerH = 24;
    int w = 320;
    int rows = std::max(1, n);
    int h = headerH + rows * rowH + 6;
    int px = 6;
    int py = cameraH - HINT_H - h - 4;
    if (py < TOPBAR_H + 4) { py = TOPBAR_H + 4; }

    SDL_Rect panel = { px, py, w, h };
    drawFillRect(panel, SDL_Color{ 16, 18, 24, 255 }, 235);
    drawRect(panel, SDL_Color{ 90, 100, 130, 255 }, 255);

    setFont(fontType::mainFont);
    setFontSize(15);
    drawText(header, px + 8, py + 4, headCol);

    if (n == 0)
    {
        setFontSize(14);
        std::wstring msg = (mode_ == EditMode::Vehicle) ? L"(frame only - pick a part and click)" : L"(empty - left-click to add selected item)";
        drawText(msg, px + 10, py + headerH + 4, SDL_Color{ 150, 155, 170, 255 });
        return;
    }

    for (int i = 0; i < n; ++i)
    {
        ItemData& it = pk->itemInfo[i];
        int ry = py + headerH + i * rowH;
        SDL_Rect iconBg = { px + 6, ry + 2, 20, 20 };
        drawFillRect(iconBg, SDL_Color{ 8, 9, 12, 255 }, 255);
        setZoom(1.25);
        drawSpriteCenter(spr::itemset, it.getSprIndex(), px + 16, ry + 12);
        setZoom(1.0);

        std::wstring nm = (it.itemCode >= 0 && it.itemCode < (int)itemDex.size()) ? itemDex[it.itemCode].name : L"?";
        setFontSize(15);
        drawText(nm, px + 34, ry + 4, SDL_Color{ 235, 235, 240, 255 });
        std::wstring cnt = L"x" + std::to_wstring((int)it.number);
        int cw = getTextWidthWithoutColor(cnt);
        drawText(cnt, px + w - cw - 10, ry + 4, SDL_Color{ 255, 230, 150, 255 });
    }
}

void LotEditor::drawTooltip()
{
    if (hoverPaletteIdx_ < 0) return;
    std::vector<int>* bp = currentBucket();
    if (bp == nullptr || hoverPaletteIdx_ >= (int)bp->size()) return;
    int code = (*bp)[hoverPaletteIdx_];
    std::wstring name;
    if (mode_ == EditMode::Monster) name = (code >= 0 && code < (int)entityDex.size()) ? entityDex[code].name : L"?";
    else name = (code >= 0 && code < (int)itemDex.size()) ? itemDex[code].name : L"?";
    name += L"  #" + std::to_wstring(code);

    setFont(fontType::mainFont);
    setFontSize(14);
    int tw = getTextWidthWithoutColor(name) + 14;
    int th = 22;
    int mx = (int)getMouseX();
    int my = (int)getMouseY();
    int bx = mx - tw - 8;
    if (bx < 0) bx = mx + 16;
    int by = my + 6;
    if (by + th > cameraH) by = my - th - 6;
    SDL_Rect box = { bx, by, tw, th };
    drawFillRect(box, SDL_Color{ 8, 9, 13, 255 }, 245);
    drawRect(box, SDL_Color{ 130, 150, 190, 255 }, 255);
    drawText(name, bx + 7, by + 4, SDL_Color{ 240, 240, 245, 255 });
}

void LotEditor::drawHintBar()
{
    SDL_Rect bar = { 0, cameraH - HINT_H, cameraW, HINT_H };
    drawFillRect(bar, SDL_Color{ 18, 20, 26, 255 }, 244);
    drawLine(0, cameraH - HINT_H, cameraW, cameraH - HINT_H, SDL_Color{ 60, 66, 84 }, 255);
    setFont(fontType::mainFont);
    setFontSize(13);
    drawText(L"1-6 mode   Tab tool   WASD pan   Q/E z-level   Alt=eyedrop   [/]=box-corner   Enter=export   G chunk-grid   T tile-grid   Esc exit",
        8, cameraH - HINT_H + 3, SDL_Color{ 165, 172, 188, 255 });
}
