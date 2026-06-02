module;
#include <SDL3/SDL.h>

module LotEditor;

import std;
import util;
import globalVar;
import constVar;
import Point;
import World;
import TileData;
import Prop;
import ItemData;
import ItemPocket;
import ItemStack;
import Vehicle;
import Entity;
import EntityData;
import Player;
import log;

void LotEditor::doExport()
{
    int caX = std::min(boxChunkA_.x, boxChunkB_.x);
    int cbX = std::max(boxChunkA_.x, boxChunkB_.x);
    int caY = std::min(boxChunkA_.y, boxChunkB_.y);
    int cbY = std::max(boxChunkA_.y, boxChunkB_.y);
    int caZ = std::min(boxChunkA_.z, boxChunkB_.z);
    int cbZ = std::max(boxChunkA_.z, boxChunkB_.z);
    int chunkW = cbX - caX + 1;
    int chunkH = cbY - caY + 1;
    int originX = caX * CHUNK_SIZE_X;
    int originY = caY * CHUNK_SIZE_Y;
    int tileW = chunkW * CHUNK_SIZE_X;
    int tileH = chunkH * CHUNK_SIZE_Y;

    std::vector<std::wstring> lines;
    auto nm = [&](int code) -> std::wstring
    {
        if (code >= 0 && code < (int)itemDex.size()) return itemDex[code].name;
        return L"?";
    };
    //포켓 내용물을 {{code,count},...} + 이름 주석으로 직렬화(1단계만; 중첩은 손실).
    auto serializePocket = [&](ItemPocket* pk, std::wstring& codes, std::wstring& names)
    {
        codes = L"{ ";
        names.clear();
        bool first = true;
        for (ItemData& it : pk->itemInfo)
        {
            if (first == false) { codes += L", "; names += L", "; }
            codes += L"{" + std::to_wstring(it.itemCode) + L"," + std::to_wstring((int)it.number) + L"}";
            names += nm(it.itemCode) + L" x" + std::to_wstring((int)it.number);
            first = false;
        }
        codes += L" }";
    };

    lines.push_back(L"// ==== LotEditor export ====");
    lines.push_back(L"// footprint: " + std::to_wstring(chunkW) + L" x " + std::to_wstring(chunkH) + L" chunk(s)  (" + std::to_wstring(tileW) + L" x " + std::to_wstring(tileH) + L" tiles)");
    lines.push_back(L"// origin chunk (" + std::to_wstring(caX) + L"," + std::to_wstring(caY) + L"," + std::to_wstring(caZ) + L") -> world tile origin (" + std::to_wstring(originX) + L"," + std::to_wstring(originY) + L"," + std::to_wstring(caZ) + L")");
    lines.push_back(L"// default orientation: north-facing (door on south). codes = itemID / entityCode.");
    lines.push_back(L"LotBuilder b(" + std::to_wstring(chunkW) + L" * TILE_PER_PIXEL, " + std::to_wstring(chunkH) + L" * TILE_PER_PIXEL);");

    int cFloor = 0, cWall = 0, cProp = 0, cMonster = 0, cStack = 0, cContents = 0, cVeh = 0;
    for (int z = caZ; z <= cbZ; ++z)
    {
        int lz = z - caZ;
        std::vector<std::wstring> plane;
        for (int ty = 0; ty < tileH; ++ty)
        {
            for (int tx = 0; tx < tileW; ++tx)
            {
                TileData* tile = World::ins()->tryGetTile(originX + tx, originY + ty, z);
                if (tile == nullptr) continue;
                if (tile->floor != itemID::none)
                {
                    plane.push_back(L"b.setFloor(" + std::to_wstring(tx) + L", " + std::to_wstring(ty) + L", " + std::to_wstring(lz) + L", " + std::to_wstring(tile->floor) + L");  // " + nm(tile->floor));
                    cFloor++;
                }
                if (tile->wall != itemID::none)
                {
                    plane.push_back(L"b.setWall(" + std::to_wstring(tx) + L", " + std::to_wstring(ty) + L", " + std::to_wstring(lz) + L", " + std::to_wstring(tile->wall) + L");  // " + nm(tile->wall));
                    cWall++;
                }
                Prop* p = tile->PropPtr.get();
                if (p != nullptr)
                {
                    plane.push_back(L"b.setProp(" + std::to_wstring(tx) + L", " + std::to_wstring(ty) + L", " + std::to_wstring(lz) + L", " + std::to_wstring(p->leadItem.itemCode) + L");  // " + nm(p->leadItem.itemCode));
                    cProp++;
                }
                Entity* ent = tile->EntityPtr.get();
                if (ent != nullptr && ent != (Entity*)PlayerPtr)
                {
                    int ec = ent->entityInfo.entityCode;
                    std::wstring en = (ec >= 0 && ec < (int)entityDex.size()) ? entityDex[ec].name : L"?";
                    plane.push_back(L"b.addMonster(" + std::to_wstring(tx) + L", " + std::to_wstring(ty) + L", " + std::to_wstring(lz) + L", " + std::to_wstring(ec) + L");  // " + en);
                    cMonster++;
                }
                ItemStack* st = tile->ItemStackPtr.get();
                if (st != nullptr && st->getPocket() != nullptr && st->getPocket()->itemInfo.empty() == false)
                {
                    std::wstring codes, names;
                    serializePocket(st->getPocket(), codes, names);
                    plane.push_back(L"b.addItemStack(" + std::to_wstring(tx) + L", " + std::to_wstring(ty) + L", " + std::to_wstring(lz) + L", " + codes + L");  // " + names);
                    cStack++;
                }
                if (p != nullptr && p->leadItem.pocketPtr != nullptr && p->leadItem.pocketPtr->itemInfo.empty() == false)
                {
                    std::wstring codes, names;
                    serializePocket(p->leadItem.pocketPtr.get(), codes, names);
                    plane.push_back(L"b.addPropContents(" + std::to_wstring(tx) + L", " + std::to_wstring(ty) + L", " + std::to_wstring(lz) + L", " + codes + L");  // " + names);
                    cContents++;
                }
            }
        }
        if (plane.empty() == false)
        {
            lines.push_back(L"// ---- z = " + std::to_wstring(lz) + L" ----");
            for (auto& s : plane) lines.push_back(s);
        }
    }
    //차량 캡처: 박스 내 anchor 차량을 partInfo(절대좌표 키) 기준으로 직렬화. 부품+cargo+dir+type+name.
    //  LotBuilder 직변환은 아니고 LLM이 addVehicle/extendPart/addPart/addCargo로 옮길 수 있는 덤프.
    {
        std::unordered_set<Vehicle*> vseen;
        std::vector<std::wstring> vlines;
        for (int z = caZ; z <= cbZ; ++z)
            for (int ty = 0; ty < tileH; ++ty)
                for (int tx = 0; tx < tileW; ++tx)
                {
                    Vehicle* v = TileVehicle(originX + tx, originY + ty, z);
                    if (v == nullptr || vseen.count(v) > 0) continue;
                    vseen.insert(v);
                    int ax = v->getGridX(), ay = v->getGridY(), az = v->getGridZ();
                    if (ax < originX || ax >= originX + tileW || ay < originY || ay >= originY + tileH || az < caZ || az > cbZ) continue; //anchor 박스 밖 -> 그 박스에서 캡처
                    ++cVeh;
                    vlines.push_back(L"// vehicle \"" + v->name + L"\"  vehType=" + std::to_wstring((int)v->vehType) + L"  bodyDir=" + std::to_wstring((int)v->bodyDir) + L"  anchorLocal(" + std::to_wstring(ax - originX) + L"," + std::to_wstring(ay - originY) + L"," + std::to_wstring(az - caZ) + L")");
                    for (auto& kv : v->partInfo)
                    {
                        const Point3& key = kv.first;
                        ItemPocket* pocket = kv.second.get();
                        if (pocket == nullptr) continue;
                        int lx = key.x - originX, ly = key.y - originY, lz = key.z - caZ;
                        std::wstring parts;
                        bool first = true;
                        for (ItemData& part : pocket->itemInfo)
                        {
                            if (first == false) parts += L", ";
                            parts += std::to_wstring(part.itemCode) + L"(" + nm(part.itemCode) + L")";
                            first = false;
                        }
                        vlines.push_back(L"//   (" + std::to_wstring(lx) + L"," + std::to_wstring(ly) + L"," + std::to_wstring(lz) + L"): " + parts);
                        for (ItemData& part : pocket->itemInfo)
                        {
                            if (part.pocketPtr != nullptr && part.pocketPtr->itemInfo.empty() == false)
                            {
                                std::wstring ccodes, cnames;
                                serializePocket(part.pocketPtr.get(), ccodes, cnames);
                                vlines.push_back(L"//      cargo in " + std::to_wstring(part.itemCode) + L"(" + nm(part.itemCode) + L"): " + ccodes + L"  // " + cnames);
                            }
                        }
                    }
                }
        if (vlines.empty() == false)
        {
            lines.push_back(L"// ---- vehicles ----");
            for (auto& s : vlines) lines.push_back(s);
        }
    }

    lines.push_back(L"// counts: floors " + std::to_wstring(cFloor) + L", walls " + std::to_wstring(cWall) + L", props " + std::to_wstring(cProp) + L", monsters " + std::to_wstring(cMonster) + L", stacks " + std::to_wstring(cStack) + L", propContents " + std::to_wstring(cContents) + L", vehicles " + std::to_wstring(cVeh));

    //콘솔 출력
    for (auto& s : lines) prt(L"%ls\n", s.c_str());

    //파일 출력(./lotExport/lot_<n>.txt). 내용은 ASCII(코드+영문 이름+C++)라 narrow 변환 안전.
    std::string dir = "lotExport";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    int n = 0;
    std::string path;
    do
    {
        path = dir + "/lot_" + std::to_string(n) + ".txt";
        n++;
    } while (std::filesystem::exists(path) && n < 100000);

    std::ofstream out(path, std::ios::binary);
    if (out.is_open())
    {
        for (auto& wline : lines)
        {
            std::string s;
            for (wchar_t ch : wline) s += (ch >= 0 && ch < 128) ? (char)ch : '?';
            out << s << "\r\n";
        }
        out.close();
        std::wstring wpath(path.begin(), path.end());
        prt(L"export written: %ls (%d lines)\n", wpath.c_str(), (int)lines.size());
    }
    else
    {
        prt(L"file write failed\n");
    }
}
