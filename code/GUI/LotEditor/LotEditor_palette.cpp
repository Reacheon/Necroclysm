module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import globalVar;
import constVar;
import ItemData;
import EntityData;
import textureVar;
import drawSprite;
import drawPrimitive;
import drawText;

namespace
{
    constexpr int CELL = 40; //팔레트 셀 크기(스프라이트 2배=32px가 들어갈 만큼 크게)
}

//생성자에서 1회 스캔. itemDex 인덱스 == itemCode 이므로 인덱스를 그대로 코드로 보관.
void LotEditor::buildPalettes()
{
    bucketFloor_.clear();
    bucketWall_.clear();
    bucketProp_.clear();
    bucketMonster_.clear();
    bucketItem_.clear();
    bucketVeh_.clear();
    for (int i = 0; i < (int)itemDex.size(); ++i)
    {
        //스프라이트 미배정 플레이스홀더는 팔레트에서 숨김 - 아이템/차량은 sprIndex==1, 프롭은 propSprIndex==1이 기본 미배정값.
        const bool itemUnassigned = (itemDex[i].itemSprIndex == 1);
        const bool propUnassigned = (itemDex[i].propSprIndex == 1);

        //차량 버킷은 category로 전부 수집(VPART 플래그 없는 통로/트렁크/문 등 포함, 방향변형도 포함).
        //  NOT_RECIPE 필터보다 먼저 - 차량은 방향별 부품을 직접 골라야 하므로 변형을 다 보여줌.
        if (i >= 1 && itemDex[i].category == itemCategory::vehicles && !itemUnassigned) bucketVeh_.push_back(i);
        //회전된 중복 설치물(NOT_RECIPE)은 나머지 팔레트에서 제외 - 대표 1개만, 변형은 R키/우클릭 회전(Craft와 동일).
        if (itemDex[i].checkFlag(itemFlag::NOT_RECIPE)) continue;
        if (itemDex[i].checkFlag(itemFlag::FLOOR)) bucketFloor_.push_back(i);
        else if (itemDex[i].checkFlag(itemFlag::WALL)) bucketWall_.push_back(i);
        else if (itemDex[i].checkFlag(itemFlag::PROP) && !propUnassigned) bucketProp_.push_back(i);
        //Item 버킷: 바닥/벽 아닌 아이템(프롭 포함 - 줍는 아이템으로도 배치 가능).
        //  NO_ITEM_FORM(나무/Ramp 등 월드 전용)은 ItemStack으로 인스턴스화 안 되므로 제외 - Prop으로 설치할 것.
        if (i >= 1 && !itemDex[i].checkFlag(itemFlag::FLOOR) && !itemDex[i].checkFlag(itemFlag::WALL) && !itemDex[i].checkFlag(itemFlag::NO_ITEM_FORM) && !itemUnassigned) bucketItem_.push_back(i);
    }
    for (int i = 1; i < (int)entityDex.size(); ++i) bucketMonster_.push_back(i); //0번은 더미
}

//현재 모드에 해당하는 팔레트 버킷. 팔레트가 없는 모드는 nullptr(클릭이 월드로 전달됨).
std::vector<int>* LotEditor::currentBucket()
{
    switch (mode_)
    {
    case EditMode::Floor: return &bucketFloor_;
    case EditMode::Wall: return &bucketWall_;
    case EditMode::Prop: return &bucketProp_;
    case EditMode::Monster: return &bucketMonster_;
    case EditMode::Item: return &bucketItem_;
    case EditMode::Vehicle: return &bucketVeh_;
    default: return nullptr;
    }
}

int LotEditor::paletteCols()
{
    int c = paletteGridRect_.w / CELL;
    return c < 1 ? 1 : c;
}

//스크롤 가능한 최대 행 오프셋(이 이상 내려도 빈 화면 - 휠 클램프용).
int LotEditor::paletteMaxScroll()
{
    std::vector<int>* b = currentBucket();
    if (b == nullptr) return 0;
    int cols = paletteCols();
    int rowsVisible = paletteGridRect_.h / CELL;
    int totalRows = ((int)b->size() + cols - 1) / cols;
    int m = totalRows - rowsVisible;
    return m < 0 ? 0 : m;
}

bool LotEditor::mouseOverPalette()
{
    if (currentBucket() == nullptr) return false;
    float mx = getMouseX();
    float my = getMouseY();
    return mx >= palettePanelRect_.x && mx < palettePanelRect_.x + palettePanelRect_.w
        && my >= palettePanelRect_.y && my < palettePanelRect_.y + palettePanelRect_.h;
}

void LotEditor::drawPalette()
{
    hoverPaletteIdx_ = -1;
    std::vector<int>* bp = currentBucket();
    if (bp == nullptr) return;
    std::vector<int>& bucket = *bp;

    drawFillRect(palettePanelRect_, SDL_Color{ 24, 26, 34, 255 }, 238);
    drawRect(palettePanelRect_, SDL_Color{ 74, 80, 102, 255 }, 255);

    std::wstring title =
        (mode_ == EditMode::Floor) ? L"FLOOR" :
        (mode_ == EditMode::Wall) ? L"WALL" :
        (mode_ == EditMode::Prop) ? L"PROP" :
        (mode_ == EditMode::Monster) ? L"MONSTER" :
        (mode_ == EditMode::Item) ? L"ITEM" : L"VEHICLE";

    setFont(fontType::mainFont);
    setFontSize(15);
    drawText(title + L"  " + std::to_wstring((int)bucket.size()), palettePanelRect_.x + 8, palettePanelRect_.y + 3, SDL_Color{ 235, 235, 240, 255 });

    int cols = paletteCols();
    int rowsVisible = paletteGridRect_.h / CELL;
    int firstIndex = paletteScroll_ * cols;
    int sel = currentSelCode();
    float mx = getMouseX();
    float my = getMouseY();

    for (int r = 0; r < rowsVisible; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int idx = firstIndex + r * cols + c;
            if (idx < 0 || idx >= (int)bucket.size()) continue;
            int code = bucket[idx];
            int cellX = paletteGridRect_.x + c * CELL;
            int cellY = paletteGridRect_.y + r * CELL;
            int cx = cellX + CELL / 2;
            int cy = cellY + CELL / 2;
            SDL_Rect cell = { cellX + 1, cellY + 1, CELL - 2, CELL - 2 };

            bool hover = (mx >= cell.x && mx < cell.x + cell.w && my >= cell.y && my < cell.y + cell.h);
            if (hover) hoverPaletteIdx_ = idx;

            drawFillRect(cell, hover ? SDL_Color{ 52, 60, 82, 255 } : SDL_Color{ 12, 13, 18, 255 }, 255);

            if (mode_ == EditMode::Floor || mode_ == EditMode::Wall)
            {
                setZoom(2.0);
                //고립(이웃에 동류 없음) 변형을 대표 이미지로 - 연결된 벽보다 명확함.
                int sprIdx = itemDex[code].tileSprIndex;
                if (itemDex[code].tileConnectGroup != -1) sprIdx += connectGroupExtraIndex(false, false, false, false);
                drawSpriteCenter(spr::tileset, sprIdx, cx, cy);
            }
            else if (mode_ == EditMode::Prop)
            {
                //나무는 48px 풀스프라이트라 셀을 넘쳐 이웃을 가림 -> 1/2배(2.0->1.0)로 픽셀손실 없이 축소.
                setZoom(itemDex[code].checkFlag(itemFlag::TREE) ? 1.0f : 2.0f);
                //고립(이웃에 동류 없음) 변형을 대표 이미지로 - 타일과 동일 처리.
                int sprIdx = itemDex[code].propSprIndex;
                if (itemDex[code].tileConnectGroup != -1) sprIdx += connectGroupExtraIndex(false, false, false, false);
                drawSpriteCenter(spr::propset, sprIdx, cx, cy);
            }
            else if (mode_ == EditMode::Item || mode_ == EditMode::Vehicle)
            {
                setZoom(2.0);
                drawSpriteCenter(spr::itemset, itemDex[code].getSprIndex(), cx, cy);
            }
            else if (mode_ == EditMode::Monster && entityDex[code].entitySpr != nullptr)
            {
                setZoom(2.0);
                drawSpriteCenter(entityDex[code].entitySpr, 0, cx, cy);
            }
            setZoom(1.0);

            if (code == sel) drawRect(cell, SDL_Color{ 255, 220, 40, 255 }, 255);
            else if (hover) drawRect(cell, SDL_Color{ 150, 170, 210, 255 }, 255);
        }
    }

    //스크롤 힌트(위/아래로 더 있을 때 작은 삼각 표시)
    int totalRows = ((int)bucket.size() + cols - 1) / cols;
    if (paletteScroll_ > 0)
        drawText(L"^", palettePanelRect_.x + palettePanelRect_.w - 16, palettePanelRect_.y + 3, SDL_Color{ 180, 200, 240, 255 });
    if (paletteScroll_ + rowsVisible < totalRows)
        drawText(L"v", palettePanelRect_.x + palettePanelRect_.w - 16, palettePanelRect_.y + palettePanelRect_.h - 18, SDL_Color{ 180, 200, 240, 255 });
}

void LotEditor::paletteClick()
{
    std::vector<int>* bp = currentBucket();
    if (bp == nullptr) return;
    std::vector<int>& bucket = *bp;
    int cols = paletteCols();
    int localX = (int)getMouseX() - paletteGridRect_.x;
    int localY = (int)getMouseY() - paletteGridRect_.y;
    if (localX < 0 || localY < 0) return;
    int c = localX / CELL;
    int r = localY / CELL;
    if (c >= cols) return;
    int idx = paletteScroll_ * cols + r * cols + c;
    if (idx >= 0 && idx < (int)bucket.size()) setSelCode(bucket[idx]);
}
