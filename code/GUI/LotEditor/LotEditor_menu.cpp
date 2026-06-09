module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import globalVar;
import constVar;
import Point;
import World;
import Prop;
import ItemData;
import ItemPocket;
import ItemStack;
import Vehicle;
import Entity;
import Player;
import drawText;
import drawPrimitive;
import log;

namespace
{
    constexpr int MENU_W = 176;
    constexpr int ROW_H = 22;
}

//대상(프롭/스택/몬스터/빈 타일)에 따라 동작 목록을 구성해 커서 위치에 연다.
void LotEditor::openEditorContextMenu(Point3 t)
{
    menuTile_ = t;
    menuPos_ = { (int)getMouseX(), (int)getMouseY() };
    menuItems_.clear();
    menuItems_.push_back({ L"Box corner A here", 0 });
    menuItems_.push_back({ L"Box corner B here", 1 });
    if (mode_ == EditMode::Vehicle)
    {
        if (TileVehicle(t.x, t.y, t.z) == nullptr) menuItems_.push_back({ L"New vehicle here", 10 });
        else menuItems_.push_back({ L"Delete vehicle", 11 });
    }
    else
    {
        menuItems_.push_back({ L"Clear tile", 2 });
        Prop* propPtr = TileProp(t.x, t.y, t.z);
        if (propPtr != nullptr)
        {
            menuItems_.push_back({ L"Rotate prop", 3 });
            menuItems_.push_back({ L"Delete prop", 4 });
            //컨테이너 프롭에 내용물이 있으면 비우기 제공(잘못 채운 거 비우고 다시 채우기용).
            if (propPtr->leadItem.pocketPtr != nullptr && propPtr->leadItem.pocketPtr->itemInfo.empty() == false)
                menuItems_.push_back({ L"Empty contents", 7 });
            //창문 저작: 커튼 달기/제거, 커튼/창문 개폐, 파손 토글(플래그 직접 조작 — export 플래그 직렬화 후 영속)
            if (propPtr->leadItem.checkFlag(itemFlag::WINDOW))
            {
                if (propPtr->leadItem.checkFlag(itemFlag::CURTAIN))
                {
                    menuItems_.push_back({ L"Remove curtain", 21 });
                    menuItems_.push_back({ L"Toggle curtain", 22 });
                }
                else menuItems_.push_back({ L"Add curtain", 20 });
                menuItems_.push_back({ L"Toggle window", 23 });
                menuItems_.push_back({ L"Cycle break", 24 });
            }
        }
        //차량 컨테이너 부품에 cargo가 들어있으면 비우기 제공.
        {
            int vkind = 0;
            ItemPocket* cargo = pocketAt(t, &vkind);
            if (vkind == 2 && cargo != nullptr && cargo->itemInfo.empty() == false) menuItems_.push_back({ L"Empty cargo", 8 });
        }
        if (TileItemStack(t.x, t.y, t.z) != nullptr) menuItems_.push_back({ L"Delete stack", 5 });
        Entity* e = TileEntity(t.x, t.y, t.z);
        if (e != nullptr && e != (Entity*)PlayerPtr) menuItems_.push_back({ L"Delete monster", 6 });
    }
    menuOpen_ = true;

    //화면 밖으로 안 나가게 클램프
    int h = (int)menuItems_.size() * ROW_H + 6;
    if (menuPos_.x + MENU_W > cameraW) menuPos_.x = cameraW - MENU_W - 2;
    if (menuPos_.y + h > cameraH) menuPos_.y = cameraH - h - 2;
    if (menuPos_.x < 0) menuPos_.x = 0;
    if (menuPos_.y < 0) menuPos_.y = 0;
}

void LotEditor::drawContextMenu()
{
    if (menuOpen_ == false) return;
    int n = (int)menuItems_.size();
    int h = n * ROW_H + 6;
    SDL_Rect bg = { menuPos_.x, menuPos_.y, MENU_W, h };
    drawFillRect(bg, SDL_Color{ 20, 22, 30, 255 }, 246);
    drawRect(bg, SDL_Color{ 120, 130, 160, 255 }, 255);

    float mx = getMouseX();
    float my = getMouseY();
    setFont(fontType::mainFont);
    setFontSize(15);
    for (int i = 0; i < n; ++i)
    {
        int ry = menuPos_.y + 3 + i * ROW_H;
        bool hover = (mx >= menuPos_.x && mx < menuPos_.x + MENU_W && my >= ry && my < ry + ROW_H);
        if (hover)
        {
            SDL_Rect hr = { menuPos_.x + 2, ry, MENU_W - 4, ROW_H };
            drawFillRect(hr, SDL_Color{ 60, 84, 130, 255 }, 255);
        }
        drawText(menuItems_[i].first, menuPos_.x + 10, ry + 3, SDL_Color{ 232, 234, 240, 255 });
    }
}

void LotEditor::contextMenuClick()
{
    int mx = (int)getMouseX();
    int my = (int)getMouseY();
    int n = (int)menuItems_.size();
    for (int i = 0; i < n; ++i)
    {
        int ry = menuPos_.y + 3 + i * ROW_H;
        if (mx >= menuPos_.x && mx < menuPos_.x + MENU_W && my >= ry && my < ry + ROW_H)
        {
            executeMenuAction(menuItems_[i].second);
            break;
        }
    }
    menuOpen_ = false;
}

void LotEditor::executeMenuAction(int id)
{
    Point3 t = menuTile_;
    switch (id)
    {
    case 0:
    {
        int cx, cy;
        World::ins()->changeToChunkCoord(t.x, t.y, cx, cy);
        boxChunkA_ = { cx, cy, t.z };
        boxSet_ = true;
        break;
    }
    case 1:
    {
        int cx, cy;
        World::ins()->changeToChunkCoord(t.x, t.y, cx, cy);
        boxChunkB_ = { cx, cy, t.z };
        boxSet_ = true;
        break;
    }
    case 2: clearRect({ t.x, t.y }, { t.x, t.y }); break;
    case 3:
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr)
        {
            int r = itemDex[p->leadItem.itemCode].rotatedCCW90ItemCode;
            if (r != 0) { destroyProp(t); createProp(t, r); }
        }
        break;
    }
    case 4: if (TileProp(t.x, t.y, t.z) != nullptr) destroyProp(t); break;
    case 5: if (TileItemStack(t.x, t.y, t.z) != nullptr) destroyItemStack(t); break;
    case 6: removeMonsterAt(t); break;
    case 7:
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr && p->leadItem.pocketPtr != nullptr) p->leadItem.pocketPtr->itemInfo.clear();
        break;
    }
    case 8: //차량 cargo 비우기
    {
        int vkind = 0;
        ItemPocket* cargo = pocketAt(t, &vkind);
        if (vkind == 2 && cargo != nullptr) cargo->itemInfo.clear();
        break;
    }
    case 10: //새 차량(선택된 프레임으로). 이름/타입/방향 설정 패널은 후속.
    {
        int frame = itemDex[selVeh_].checkFlag(itemFlag::VFRAME) ? selVeh_ : itemID::metalFrame;
        ensureChunkAt(t.x, t.y, t.z);
        if (TileVehicle(t.x, t.y, t.z) == nullptr)
        {
            Vehicle* v = World::ins()->createVehicle(t.x, t.y, t.z, frame);
            v->updateSpr();
            activeVeh_ = v;
            newVehPending_ = false;
        }
        break;
    }
    case 11: //차량 전체 삭제(소멸자가 청크/타일 정리)
    {
        Vehicle* v = TileVehicle(t.x, t.y, t.z);
        if (v != nullptr)
        {
            if (v == activeVeh_) activeVeh_ = nullptr;
            World::ins()->destroyVehicle(v);
        }
        break;
    }
    case 20: //커튼 달기 (닫힌 상태로 시작 = 시야 차단)
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr && p->leadItem.checkFlag(itemFlag::WINDOW))
        {
            p->leadItem.addFlag(itemFlag::CURTAIN);
            p->leadItem.eraseFlag(itemFlag::CURTAIN_OPEN);
            p->leadItem.eraseFlag(itemFlag::WINDOW_OPEN);
            p->leadItem.eraseFlag(itemFlag::WINDOW_BROKEN);
            p->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
            p->leadItem.addFlag(itemFlag::PROP_BLOCKER);
        }
        break;
    }
    case 21: //커튼 제거
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr)
        {
            p->leadItem.eraseFlag(itemFlag::CURTAIN);
            p->leadItem.eraseFlag(itemFlag::CURTAIN_OPEN);
            p->leadItem.eraseFlag(itemFlag::PROP_BLOCKER);
        }
        break;
    }
    case 22: //커튼 개폐 토글 (닫으면 안쪽 창문도 닫힘)
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr && p->leadItem.checkFlag(itemFlag::CURTAIN))
        {
            if (p->leadItem.checkFlag(itemFlag::CURTAIN_OPEN))
            {
                p->leadItem.eraseFlag(itemFlag::CURTAIN_OPEN);
                p->leadItem.eraseFlag(itemFlag::WINDOW_OPEN);
                p->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
                p->leadItem.addFlag(itemFlag::PROP_BLOCKER);
            }
            else
            {
                p->leadItem.addFlag(itemFlag::CURTAIN_OPEN);
                p->leadItem.eraseFlag(itemFlag::PROP_BLOCKER);
            }
        }
        break;
    }
    case 23: //창문 개폐 토글 (커튼이 닫혀 있으면 먼저 커튼을 열어야 함)
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr && p->leadItem.checkFlag(itemFlag::WINDOW) && p->leadItem.checkFlag(itemFlag::WINDOW_BROKEN) == false)
        {
            if (p->leadItem.checkFlag(itemFlag::CURTAIN) && p->leadItem.checkFlag(itemFlag::CURTAIN_OPEN) == false) break;
            if (p->leadItem.checkFlag(itemFlag::WINDOW_OPEN))
            {
                p->leadItem.eraseFlag(itemFlag::WINDOW_OPEN);
                p->leadItem.eraseFlag(itemFlag::PROP_WALKABLE);
            }
            else
            {
                p->leadItem.addFlag(itemFlag::WINDOW_OPEN);
                p->leadItem.addFlag(itemFlag::PROP_WALKABLE);
            }
        }
        break;
    }
    case 24: //파손 단계 순환: 멀쩡 → 깨짐(149) → 창틀만(150) → 멀쩡 (파손 시 커튼 자동 해체)
    {
        Prop* p = TileProp(t.x, t.y, t.z);
        if (p != nullptr && p->leadItem.checkFlag(itemFlag::WINDOW))
        {
            ItemData& w = p->leadItem;
            if (w.checkFlag(itemFlag::WINDOW_FRAME))
            {
                //창틀만 → 멀쩡(닫힌 창문)으로 복구
                w.eraseFlag(itemFlag::WINDOW_FRAME);
                w.eraseFlag(itemFlag::WINDOW_OPEN);
                w.eraseFlag(itemFlag::PROP_WALKABLE);
            }
            else if (w.checkFlag(itemFlag::WINDOW_BROKEN))
            {
                //깨짐 → 창틀만
                w.eraseFlag(itemFlag::WINDOW_BROKEN);
                w.addFlag(itemFlag::WINDOW_FRAME);
            }
            else
            {
                //멀쩡 → 깨짐(커튼 자동 해체)
                w.eraseFlag(itemFlag::CURTAIN);
                w.eraseFlag(itemFlag::CURTAIN_OPEN);
                w.eraseFlag(itemFlag::WINDOW_OPEN);
                w.addFlag(itemFlag::WINDOW_BROKEN);
                w.eraseFlag(itemFlag::PROP_BLOCKER);
                w.addFlag(itemFlag::PROP_WALKABLE);
            }
        }
        break;
    }
    }
}
