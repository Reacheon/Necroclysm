#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL.h>

export module readItemDex;

import std;
import util;
import constVar;
import globalVar;
import textureVar;

namespace csvItem
{
    constexpr int name = 0;
    constexpr int itemCode = 1;
    constexpr int sprIndex = 2;
    constexpr int tooltip = 3;
    constexpr int category = 4;
    constexpr int subcategory = 5;

    constexpr int flag = 6;
    constexpr int weight = 7;
    constexpr int volume = 8;
    constexpr int material = 9;

    constexpr int tileSprIndex = 10;
    constexpr int fixtureSprIndex = 11;
    constexpr int toolQuality = 12; // A_X 여기서 X는 레벨, 즉 1_3은 망치질 3단계
    constexpr int craftMinUnit = 13;
    constexpr int craftTime = 14;

    constexpr int recipe = 15;
    constexpr int recipeQualityNeed = 16;
    constexpr int recipeTalentNeed = 17;

    constexpr int disassy = 18;
    constexpr int disassyQualityNeed = 19;
    constexpr int disassyTalentNeed = 20;

    constexpr int repair = 21;
    constexpr int repairQualityNeed = 22;
    constexpr int repairTalentNeed = 23;

    constexpr int pocketMaxVolume = 24;
    constexpr int pocketMaxNumber = 25;
    constexpr int pocketOnlyItem = 26;

    constexpr int rPierce = 27;
    constexpr int rCut = 28;
    constexpr int rBash = 29;
    constexpr int SH = 30;
    constexpr int EV = 31;
    constexpr int rFire = 32;
    constexpr int rCold = 33;
    constexpr int rElec = 34;
    constexpr int rCorr = 35;
    constexpr int rRad = 36;
    constexpr int bionicIndex = 37;

    constexpr int durability = 38;

    constexpr int pierce = 39;
    constexpr int cut = 40;
    constexpr int bash = 41;

    constexpr int bulletPierce = 42;
    constexpr int bulletCut = 43;
    constexpr int bulletBash = 44;
    constexpr int bulletStoppingPower = 45;
    constexpr int bulletJam = 46;
    constexpr int bulletType = 47;
    constexpr int bulletRange = 48;

    constexpr int gunDmg = 49;
    constexpr int gunAccInit = 50;
    constexpr int gunAccSpd = 51;
    constexpr int gunAccMax = 52;
    constexpr int gunRebound = 53;
    constexpr int gunShotSpd = 54;
    constexpr int gunReloadSpd = 55;
    constexpr int gunJam = 56;
    constexpr int gunMod = 57;
    constexpr int gunBalance = 58;
    constexpr int gunRange = 59;

    constexpr int meleeAtkSpd = 60;
    constexpr int meleeAtkAcc = 61;
    constexpr int meleeBalance = 62;
    constexpr int meleeRange = 63;

    constexpr int modeTemplate = 64;

    constexpr int throwAtk = 65;
    constexpr int throwType = 66;
    constexpr int throwAtkAcc = 67;
    constexpr int throwBalance = 68;
    constexpr int throwRange = 69;
    constexpr int throwStoppingPower = 70;

    constexpr int bookIndex = 71;

    constexpr int equipSpr = 72;
    constexpr int equipPriority = 73;

    constexpr int leftWieldSpr = 74;
    constexpr int leftWieldPriority = 75;

    constexpr int rightWieldSpr = 76;
    constexpr int rightWieldPriority = 77;

    constexpr int tileConnectGroup = 78;
    constexpr int dirChangeItemCode = 79;

    constexpr int lightRange = 80;
    constexpr int lightIntensity = 81;
    constexpr int lightR = 82;
    constexpr int lightG = 83;
    constexpr int lightB = 84;
    constexpr int lightDelX = 85;
    constexpr int lightDelY = 86;

    constexpr int animeSize = 87;
    constexpr int animeFPS = 88;
    constexpr int randomPropSprSize = 89;
    constexpr int growthThreshold = 90;

    constexpr int molecularWeight = 91;
    constexpr int liqColorR = 92;
    constexpr int liqColorG = 93;
    constexpr int liqColorB = 94;
    constexpr int gasColorR = 95;
    constexpr int gasColorG = 96;
    constexpr int gasColorB = 97;
    constexpr int gasAlphaMax = 98;

    constexpr int propMaxHP = 99;
    constexpr int propRPierce = 100;
    constexpr int propRCut = 101;
    constexpr int propRBash = 102;


};

export int readItemDex(const wchar_t* file)
{
    std::wifstream in(file);
    std::wstring str;
    std::wstring strFragment;//표 한 칸의 문자열이 저장되는 객체, 매번 초기화됨

    if (in.is_open())
    {
        in.imbue(std::locale(in.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
        in.seekg(0, std::ios::end);
        long long size = in.tellg();
        str.resize(size);
        in.seekg(0, std::ios::beg);
        in.read(&str[0], size);
        in.close();
        //읽기 종료

        //배열의 가로 사이즈를 구한다.
        int csvWidth = 0;
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == UNI::TAB || str[i] == UNI::LF)//해당 문자가 쉼표(44)거나 라인피드(10)일 경우
            {
                csvWidth++;
                if (str[i] == UNI::LF)
                {
                    //prt(L"이 csv 파일의 가로사이즈는 %d이다!\n", csvWidth);
                    break;
                }
            }
        }

        // 3,5,7 형태의 문자열을 벡터에 순서대로 3->5->7 순서대로 입력해주는 람다함수
        auto valsToVec = [&](std::wstring strFragment, auto& container) {
            int val;
            for (int j = 0; j < strFragment.size(); j++)
            {
                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                {
                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                    val = std::stoi(strFragment.substr(0, j));
                    strFragment.erase(0, j + 1);
                    j = 0;

                    container.push_back(val);
                }
            }
            };


        // 3x3,5x6,2x6 형태의 문자열을 pair 형태로 벡터에 순서대로 입력해주는 람다함수
        auto pairsToVec = [&](std::wstring strFragment, auto& container) {
            using T1 = decltype(container[0].first);
            using T2 = decltype(container[0].second);

            for (int j = 0; j < strFragment.size(); j++)
            {
                if (strFragment[j] == UNI::ASTERISK)
                {
                    container.push_back({ static_cast<T1>(std::stoi(strFragment.substr(0, j))), 0 });
                    strFragment.erase(0, j + 1);
                    j = 0;
                }

                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                {
                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                    container.back().second = static_cast<T2>(std::stoi(strFragment.substr(0, j)));
                    strFragment.erase(0, j + 1);
                    j = 0;
                }
            }
            };


        int startPoint = -1;
        int endPoint = -1;
        int arrayCounter = 0;

        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == UNI::TAB || str[i] == 10)//ASCII로 44(,)와 또는 라인피드(\n)와 같을 때
            {
                if (i == str.size() - 1) { i++; }//마지막 문자면 endP-startP 보정을 위해 i를 1 더함.
                endPoint = i;

                strFragment = str.substr(startPoint, endPoint - startPoint);

                if (arrayCounter / csvWidth == itemDex.size() + 1) // 만약 벡터 크기(상하)가 부족하면 1칸 늘림
                {
                    ItemData newItem;
                    itemDex.push_back(newItem);
                }

                if (arrayCounter / (csvWidth) != 0)
                {
                    switch (arrayCounter % (csvWidth))
                    {
                        case csvItem::name:
                            itemDex[arrayCounter / (csvWidth)-1].name = strFragment;
                            break;
                        case csvItem::itemCode:
                            itemDex[arrayCounter / (csvWidth)-1].itemCode = wtoi(strFragment.c_str());
                            break;
                        case csvItem::sprIndex:
                            itemDex[arrayCounter / (csvWidth)-1].sprIndex = wtoi(strFragment.c_str());
                            break;
                        case csvItem::tooltip:
                            itemTooltip.push_back(strFragment);
                            itemDex[arrayCounter / (csvWidth)-1].tooltipIndex = itemTooltip.size()-1;
                            break;
                        case csvItem::category:
                            if (strFragment == L"EQUIPMENT") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::equipment; }
                            else if (strFragment == L"WEAPON") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::weapon; }
                            else if (strFragment == L"TOOL") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::tool; }
                            else if (strFragment == L"CONSUMABLE") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::consumable; }
                            else if (strFragment == L"VEHICLE") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::vehicle; }
                            else if (strFragment == L"BIONIC") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::bionic; }
                            else if (strFragment == L"STRUCTURE") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::structure; }
                            else if (strFragment == L"MATERIAL") { itemDex[arrayCounter / (csvWidth)-1].category = itemCategory::material; }       
                            else { errorBox("error in readItemDex.ixx, csvItem::category"); }

                            break;
                        case csvItem::subcategory:
                            if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::equipment)
                            {
                                if (strFragment == L"CLOTHING") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::equipment_clothing; }
                                else if (strFragment == L"HAT") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::equipment_hat; }
                                else if (strFragment == L"GLOVES") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::equipment_gloves; }
                                else if (strFragment == L"SHOES") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::equipment_shoes; }
                                else if (strFragment == L"ACCESSORY") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::equipment_accessory; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(equipment)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::weapon)
                            {
                                if (strFragment == L"PIERCING") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::weapon_piercing; }
                                else if (strFragment == L"CUTTING") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::weapon_cutting; }
                                else if (strFragment == L"BASHING") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::weapon_bashing; }
                                else if (strFragment == L"SHOOTING") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::weapon_shooting; }
                                else if (strFragment == L"THROWING") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::weapon_throwing; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(weapon)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::tool)
                            {
                                if (strFragment == L"HAND") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::tool_hand; }
                                else if (strFragment == L"POWER") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::tool_power; }
                                else if (strFragment == L"CONTAINER") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::tool_container; }
                                else if (strFragment == L"DEVICE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::tool_device; }
                                else if (strFragment == L"DOCUMENT") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::tool_document; }
                                else if (strFragment == L"ETC") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::tool_etc; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(tool)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::consumable)
                            {
                                if (strFragment == L"FOOD") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::consumable_food; }
                                else if (strFragment == L"MEDICINE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::consumable_medicine; }
                                else if (strFragment == L"AMMO") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::consumable_ammo; }
                                else if (strFragment == L"FUEL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::consumable_fuel; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(consumable)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::vehicle)
                            {
                                if (strFragment == L"FRAME") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::vehicle_frame; }
                                else if (strFragment == L"ENGINE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::vehicle_engine; }
                                else if (strFragment == L"EXTERIOR") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::vehicle_exterior; }
                                else if (strFragment == L"TRANSPORT") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::vehicle_transport; }
                                else if (strFragment == L"ENERGY") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::vehicle_energy; }
                                else if (strFragment == L"DEVICE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::vehicle_device; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(container)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::bionic)
                            {
                                if (strFragment == L"CORE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::bionic_core; }
                                else if (strFragment == L"ACTIVE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::bionic_active; }
                                else if (strFragment == L"PASSIVE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::bionic_passive; }
                                else if (strFragment == L"TOGGLE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::bionic_toggle; }
                                else if (strFragment == L"GENERATOR") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::bionic_generator; }
                                else if (strFragment == L"STORAGE") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::bionic_storage; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(container)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::material)
                            {
                                if (strFragment == L"CHEMICAL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::material_chemical; }
                                else if (strFragment == L"BIOLOGICAL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::material_biological; }
                                else if (strFragment == L"MECHANICAL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::material_mechanical; }
                                else if (strFragment == L"ELECTRICAL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::material_electrical; }
                                else if (strFragment == L"ETC") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::material_etc; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(material)"); }
                            }
                            else if (itemDex[arrayCounter / (csvWidth)-1].category == itemCategory::structure)
                            {
                                if (strFragment == L"WALL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::structure_wall; }
                                else if (strFragment == L"FLOOR") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::structure_floor; }
                                else if (strFragment == L"CEIL") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::structure_ceil; }
                                else if (strFragment == L"PROP") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::structure_prop; }
                                else if (strFragment == L"ELECTRIC") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::structure_electric; }
                                else if (strFragment == L"PNEUMATIC") { itemDex[arrayCounter / (csvWidth)-1].subcategory = itemSubcategory::structure_pneumatic; }
                                else { errorBox("error in readItemDex.ixx, csvItem::subcategory(structure)"); }
                            }
                            else { errorBox("error in readItemDex.ixx, itemCategory"); }
                            break;
                        case csvItem::weight:
                            itemDex[arrayCounter / (csvWidth)-1].weight = wtoi(strFragment.c_str());
                            break;
                        case csvItem::volume:
                            itemDex[arrayCounter / (csvWidth)-1].volume = wtoi(strFragment.c_str());
                            break;
                        case csvItem::material:
                            break;
                        case csvItem::flag:
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    if (strFragment.substr(0, j) == L"TWOHANDED") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TWOHANDED);
                                    else if (strFragment.substr(0, j) == L"CANEQUIP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CANEQUIP);
                                    else if (strFragment.substr(0, j) == L"NONSTACK") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::NONSTACK);
                                    else if (strFragment.substr(0, j) == L"GUN") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::GUN);
                                    else if (strFragment.substr(0, j) == L"MAGAZINE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::MAGAZINE);
                                    else if (strFragment.substr(0, j) == L"AMMO") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::AMMO);
                                    else if (strFragment.substr(0, j) == L"POWERED") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::POWERED);
                                    else if (strFragment.substr(0, j) == L"RAIDARMOR") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIDARMOR);
                                    else if (strFragment.substr(0, j) == L"COORDCRAFT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::COORDCRAFT);
                                    else if (strFragment.substr(0, j) == L"VFRAME") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VFRAME);
                                    else if (strFragment.substr(0, j) == L"ALCHEMYTOOL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::ALCHEMYTOOL);
                                    else if (strFragment.substr(0, j) == L"ALCHEMYMATERIAL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::ALCHEMYMATERIAL);
                                    else if (strFragment.substr(0, j) == L"LIQUID") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::LIQUID);
                                    else if (strFragment.substr(0, j) == L"GAS") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::GAS);
                                    else if (strFragment.substr(0, j) == L"VPART") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VPART);
                                    else if (strFragment.substr(0, j) == L"TRANSPARENT_WALL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TRANSPARENT_WALL);
                                    else if (strFragment.substr(0, j) == L"PROP_BIG") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP_BIG);
                                    else if (strFragment.substr(0, j) == L"VEH_ROOF") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VEH_ROOF);
                                    else if (strFragment.substr(0, j) == L"PROP_WALKABLE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP_WALKABLE);
                                    else if (strFragment.substr(0, j) == L"PROP_BLOCKER") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP_BLOCKER);
                                    else if (strFragment.substr(0, j) == L"PROP_DEPTH_LOWER") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP_DEPTH_LOWER);
                                    else if (strFragment.substr(0, j) == L"LIGHT_ON") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::LIGHT_ON);
                                    else if (strFragment.substr(0, j) == L"LIGHT_OFF") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::LIGHT_OFF);
                                    else if (strFragment.substr(0, j) == L"VPART_WALL_CONNECT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VPART_WALL_CONNECT);
                                    else if (strFragment.substr(0, j) == L"VPART_DIR_DEPEND") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VPART_DIR_DEPEND);
                                    else if (strFragment.substr(0, j) == L"VPART_DOOR_OPEN") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VPART_DOOR_OPEN);
                                    else if (strFragment.substr(0, j) == L"VPART_DOOR_CLOSE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::VPART_DOOR_CLOSE);

                                    else if (strFragment.substr(0, j) == L"PIPE_CNCT_RIGHT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PIPE_CNCT_RIGHT);
                                    else if (strFragment.substr(0, j) == L"PIPE_CNCT_TOP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PIPE_CNCT_TOP);
                                    else if (strFragment.substr(0, j) == L"PIPE_CNCT_LEFT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PIPE_CNCT_LEFT);
                                    else if (strFragment.substr(0, j) == L"PIPE_CNCT_BOT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PIPE_CNCT_BOT);

                                    else if (strFragment.substr(0, j) == L"WIRE_CNCT_RIGHT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WIRE_CNCT_RIGHT);
                                    else if (strFragment.substr(0, j) == L"WIRE_CNCT_TOP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WIRE_CNCT_TOP);
                                    else if (strFragment.substr(0, j) == L"WIRE_CNCT_LEFT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WIRE_CNCT_LEFT);
                                    else if (strFragment.substr(0, j) == L"WIRE_CNCT_BOT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WIRE_CNCT_BOT);

                                    else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_RIGHT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CONVEYOR_CNCT_RIGHT);
                                    else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_TOP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CONVEYOR_CNCT_TOP);
                                    else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_LEFT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CONVEYOR_CNCT_LEFT);
                                    else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_BOT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CONVEYOR_CNCT_BOT);

                                    else if (strFragment.substr(0, j) == L"TIRE_NORMAL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TIRE_NORMAL);
                                    else if (strFragment.substr(0, j) == L"TIRE_STEER") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TIRE_STEER);

                                    else if (strFragment.substr(0, j) == L"PROP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP);

                                    else if (strFragment.substr(0, j) == L"WIRE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WIRE);
                                    else if (strFragment.substr(0, j) == L"PIPE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PIPE);
                                    else if (strFragment.substr(0, j) == L"CONVEYOR") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CONVEYOR);

                                    else if (strFragment.substr(0, j) == L"RAIL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL);

                                    else if (strFragment.substr(0, j) == L"RAIL_CNCT_TOP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_CNCT_TOP);
                                    else if (strFragment.substr(0, j) == L"RAIL_CNCT_BOT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_CNCT_BOT);
                                    else if (strFragment.substr(0, j) == L"RAIL_CNCT_LEFT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_CNCT_LEFT);
                                    else if (strFragment.substr(0, j) == L"RAIL_CNCT_RIGHT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_CNCT_RIGHT);

                                    else if (strFragment.substr(0, j) == L"RAIL_INPUT_TOP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_INPUT_TOP);
                                    else if (strFragment.substr(0, j) == L"RAIL_INPUT_BOT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_INPUT_BOT);
                                    else if (strFragment.substr(0, j) == L"RAIL_INPUT_LEFT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_INPUT_LEFT);
                                    else if (strFragment.substr(0, j) == L"RAIL_INPUT_RIGHT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_INPUT_RIGHT);

                                    else if (strFragment.substr(0, j) == L"RAIL_BUFFER") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::RAIL_BUFFER);

                                    else if (strFragment.substr(0, j) == L"NOT_RECIPE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::NOT_RECIPE);
                                    
                                    else if (strFragment.substr(0, j) == L"TREE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TREE);

                                    else if (strFragment.substr(0, j) == L"PLANT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PLANT);
                                    else if (strFragment.substr(0, j) == L"PLANT_SEASON_DEPENDENT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PLANT_SEASON_DEPENDENT);
                                    else if (strFragment.substr(0, j) == L"PLANT_GROWTH_DEPENDENT") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PLANT_GROWTH_DEPENDENT);
                                    else if (strFragment.substr(0, j) == L"MUSHROOM") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::MUSHROOM);
                                    else if (strFragment.substr(0, j) == L"FLOOR") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::FLOOR);
                                    else if (strFragment.substr(0, j) == L"WALL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WALL);
                                    else if (strFragment.substr(0, j) == L"CEIL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CEIL);
                                    else if (strFragment.substr(0, j) == L"PROP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP);
                                    else if (strFragment.substr(0, j) == L"WATER_SHALLOW") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WATER_SHALLOW);
                                    else if (strFragment.substr(0, j) == L"WATER_DEEP") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WATER_DEEP);
                                    else if (strFragment.substr(0, j) == L"FRESHWATER") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::FRESHWATER);
                                    else if (strFragment.substr(0, j) == L"SEAWATER") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::SEAWATER);
                                    else if (strFragment.substr(0, j) == L"TILE_SEASON") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TILE_SEASON);

                                    else if (strFragment.substr(0, j) == L"DOOR") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::DOOR);
                                    else if (strFragment.substr(0, j) == L"UPSTAIR") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::UPSTAIR);
                                    else if (strFragment.substr(0, j) == L"DOWNSTAIR") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::DOWNSTAIR);
                                    else if (strFragment.substr(0, j) == L"SIGN") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::SIGN);
                                    else if (strFragment.substr(0, j) == L"DOOR_CLOSE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::DOOR_CLOSE);
                                    else if (strFragment.substr(0, j) == L"DOOR_OEPN") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::DOOR_OPEN);
                                    else if (strFragment.substr(0, j) == L"TRAIN_WHEEL") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::TRAIN_WHEEL);

                                    else if (strFragment.substr(0, j) == L"PROP_GAS_OBSTACLE_ON") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP_GAS_OBSTACLE_ON);
                                    else if (strFragment.substr(0, j) == L"PROP_GAS_OBSTACLE_OFF") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::PROP_GAS_OBSTACLE_OFF);
                                    else if (strFragment.substr(0, j) == L"WALL_GAS_PERMEABLE") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::WALL_GAS_PERMEABLE);

                                    else if (strFragment.substr(0, j) == L"POCKET") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::POCKET);
                                    else if (strFragment.substr(0, j) == L"CAN_CLIMB") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CAN_CLIMB);
                                    else if (strFragment.substr(0, j) == L"SPR_TH_WEAPON") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::SPR_TH_WEAPON);
                                    else if (strFragment.substr(0, j) == L"NO_HAIR_HELMET") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::NO_HAIR_HELMET);
                                    else if (strFragment.substr(0, j) == L"BOW") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::BOW);
                                    else if (strFragment.substr(0, j) == L"CROSSBOW") itemDex[arrayCounter / (csvWidth)-1].flag.push_back(itemFlag::CROSSBOW);
                                    else
                                    {
                                        errorBox(L"error in readItemDex.ixx, csvItem::flag, unknown itemFlag defined " + strFragment.substr(0, j));
                                    }

                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                }
                            }
                            break;
                        case csvItem::toolQuality:
                        {
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    itemDex[arrayCounter / (csvWidth)-1].toolQuality.push_back(wtoi(strFragment.substr(0, j).c_str()));
                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                }
                            }
                            break;
                        }
                        case csvItem::craftMinUnit:
                        {
                            itemDex[arrayCounter / (csvWidth)-1].craftMinUnit = wtoi(strFragment.c_str());
                            break;
                        }
                        case csvItem::craftTime:
                        {
                            itemDex[arrayCounter / (csvWidth)-1].craftTime = wtoi(strFragment.c_str());
                            break;
                        }
                        case csvItem::recipe:
                        {
                            pairsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].recipe);
                            break;
                        }
                        case csvItem::recipeQualityNeed:
                        {
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    itemDex[arrayCounter / (csvWidth)-1].recipeQualityNeed.push_back(wtoi(strFragment.substr(0, j).c_str()));
                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                }
                            }
                            break;
                        }
                        case csvItem::recipeTalentNeed:
                        {
                            pairsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].recipeTalentNeed);
                            break;
                        }
                        case csvItem::disassy:
                        {
                            pairsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].disassy);
                            break;
                        }
                        case csvItem::disassyQualityNeed:
                        {
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    itemDex[arrayCounter / (csvWidth)-1].disassyQualityNeed.push_back(wtoi(strFragment.substr(0, j).c_str()));
                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                }
                            }
                            break;
                        }
                        case csvItem::disassyTalentNeed:
                        {
                            pairsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].disassyTalentNeed);
                            break;
                        }
                        case csvItem::repair:
                        {
                            pairsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].repair);
                            break;
                        }
                        case csvItem::repairQualityNeed:
                        {
                            for (int j = 0; j < strFragment.size(); j++)
                            {
                                if (strFragment[j] == UNI::COMMA || j == strFragment.size() - 1)
                                {
                                    if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                    itemDex[arrayCounter / (csvWidth)-1].repairQualityNeed.push_back(wtoi(strFragment.c_str()));
                                    strFragment.erase(0, j + 1);
                                    j = 0;
                                }
                            }
                            break;
                        }
                        case csvItem::repairTalentNeed:
                        {
                            pairsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].repairTalentNeed);
                            break;
                        }
                        case csvItem::pocketMaxVolume:
                            itemDex[arrayCounter / (csvWidth)-1].pocketMaxVolume = wtoi(strFragment.c_str());
                            break;
                        case csvItem::pocketMaxNumber:
                            itemDex[arrayCounter / (csvWidth)-1].pocketMaxNumber = wtoi(strFragment.c_str());
                            break;
                        case csvItem::pocketOnlyItem:
                        {
                            valsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].pocketOnlyItem);
                            break;
                        }
                        case csvItem::rPierce:
                        {
                            itemDex[arrayCounter / (csvWidth)-1].rPierce = wtoi(strFragment.c_str());
                            break;
                        }
                        case csvItem::rCut:
                        {
                            itemDex[arrayCounter / (csvWidth)-1].rCut = wtoi(strFragment.c_str());
                            break;
                        }
                        case csvItem::rBash:
                        {
                            itemDex[arrayCounter / (csvWidth)-1].rBash = wtoi(strFragment.c_str());
                            break;
                        }
                        case csvItem::SH:
                            itemDex[arrayCounter / (csvWidth)-1].sh = wtoi(strFragment.c_str());
                            break;
                        case csvItem::EV:
                            itemDex[arrayCounter / (csvWidth)-1].ev = wtoi(strFragment.c_str());
                            break;
                        case csvItem::rFire:
                            itemDex[arrayCounter / (csvWidth)-1].rFire = wtoi(strFragment.c_str());
                            break;
                        case csvItem::rCold:
                            itemDex[arrayCounter / (csvWidth)-1].rCold = wtoi(strFragment.c_str());
                            break;
                        case csvItem::rElec:
                            itemDex[arrayCounter / (csvWidth)-1].rElec = wtoi(strFragment.c_str());
                            break;
                        case csvItem::rCorr:
                            itemDex[arrayCounter / (csvWidth)-1].rCorr = wtoi(strFragment.c_str());
                            break;
                        case csvItem::rRad:
                            itemDex[arrayCounter / (csvWidth)-1].rRad = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bionicIndex:
                            itemDex[arrayCounter / (csvWidth)-1].bionicIndex = wtoi(strFragment.c_str());
                            break;
                        case csvItem::durability:
                            itemDex[arrayCounter / (csvWidth)-1].durablility = wtoi(strFragment.c_str());
                            break;
                        case csvItem::pierce:
                            itemDex[arrayCounter / (csvWidth)-1].pierce = wtoi(strFragment.c_str());
                            break;
                        case csvItem::cut:
                            itemDex[arrayCounter / (csvWidth)-1].cut = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bash:
                            itemDex[arrayCounter / (csvWidth)-1].bash = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bulletPierce:
                            itemDex[arrayCounter / (csvWidth)-1].bulletPierce = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bulletCut:
                            itemDex[arrayCounter / (csvWidth)-1].bulletCut = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bulletBash:
                            itemDex[arrayCounter / (csvWidth)-1].bulletBash = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bulletStoppingPower:
                            itemDex[arrayCounter / (csvWidth)-1].bulletStoppingPower = wtof(strFragment.c_str());
                            break;
                        case csvItem::bulletJam:
                            itemDex[arrayCounter / (csvWidth)-1].bulletJam = wtoi(strFragment.c_str());
                            break;
                        case csvItem::bulletType:
                            if (strFragment == L"NORMAL") { itemDex[arrayCounter / (csvWidth)-1].bulletType = bulletFlag::normal; }
                            else if (strFragment == L"TRACER") { itemDex[arrayCounter / (csvWidth)-1].bulletType = bulletFlag::tracer; }
                            else if (strFragment == L"AP") { itemDex[arrayCounter / (csvWidth)-1].bulletType = bulletFlag::ap; }
                            else { errorBox("error in readItemDex.ixx, csvItem::bulletType"); }
                            break;
                        case csvItem::bulletRange:
                            itemDex[arrayCounter / (csvWidth)-1].bulletRange = wtoi(strFragment.c_str());
                            break;

                        case csvItem::gunDmg:
                            itemDex[arrayCounter / (csvWidth)-1].gunDmg = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunAccInit:
                            itemDex[arrayCounter / (csvWidth)-1].gunAccInit = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunAccSpd:
                            itemDex[arrayCounter / (csvWidth)-1].gunAccSpd = wtoi(strFragment.c_str());
                            break;
                        case csvItem::gunAccMax:
                            itemDex[arrayCounter / (csvWidth)-1].gunAccMax = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunRebound:
                            itemDex[arrayCounter / (csvWidth)-1].gunRebound = wtoi(strFragment.c_str());
                            break;
                        case csvItem::gunShotSpd:
                            itemDex[arrayCounter / (csvWidth)-1].gunShotSpd = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunReloadSpd:
                            itemDex[arrayCounter / (csvWidth)-1].gunReloadSpd = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunJam:
                            itemDex[arrayCounter / (csvWidth)-1].gunJam = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunMod:
                        {
                            valsToVec(strFragment, itemDex[arrayCounter / (csvWidth)-1].gunMod);
                            break;
                        }
                        case csvItem::gunBalance:
                            itemDex[arrayCounter / (csvWidth)-1].gunBalance = wtof(strFragment.c_str());
                            break;
                        case csvItem::gunRange:
                            itemDex[arrayCounter / (csvWidth)-1].gunRange = wtoi(strFragment.c_str());
                            break;


                        case csvItem::meleeAtkSpd:
                            itemDex[arrayCounter / (csvWidth)-1].meleeAtkSpd = wtof(strFragment.c_str());
                            break;
                        case csvItem::meleeAtkAcc:
                            itemDex[arrayCounter / (csvWidth)-1].meleeAtkAcc = wtof(strFragment.c_str());
                            break;
                        case csvItem::meleeBalance:
                            itemDex[arrayCounter / (csvWidth)-1].meleeBalance = wtof(strFragment.c_str());
                            break;
                        case csvItem::meleeRange:
                            itemDex[arrayCounter / (csvWidth)-1].meleeRange = wtoi(strFragment.c_str());
                            break;


                        case csvItem::modeTemplate:
                            itemDex[arrayCounter / (csvWidth)-1].modeTemplate = wtoi(strFragment.c_str());
                            if (itemDex[arrayCounter / (csvWidth)-1].modeTemplate == 1)
                            {
                                itemDex[arrayCounter / (csvWidth)-1].modeCurrent = weaponMode::semiAutoMode;
                            }
                            break;

                        case csvItem::throwAtk:
                            itemDex[arrayCounter / (csvWidth)-1].throwAtk = wtoi(strFragment.c_str());
                            break;
                        case csvItem::throwType:
                            itemDex[arrayCounter / (csvWidth)-1].throwType = wtoi(strFragment.c_str());
                            break;
                        case csvItem::throwAtkAcc:
                            itemDex[arrayCounter / (csvWidth)-1].throwAtkAcc = wtof(strFragment.c_str());
                            break;
                        case csvItem::throwBalance:
                            itemDex[arrayCounter / (csvWidth)-1].throwBalance = wtof(strFragment.c_str());
                            break;
                        case csvItem::throwRange:
                            itemDex[arrayCounter / (csvWidth)-1].throwRange = wtoi(strFragment.c_str());
                            break;
                        case csvItem::throwStoppingPower:
                            itemDex[arrayCounter / (csvWidth)-1].throwStoppingPower = wtof(strFragment.c_str());
                            break;
                        case csvItem::bookIndex:
                            itemDex[arrayCounter / (csvWidth)-1].bookIndex = wtoi(strFragment.c_str());
                            break;
                        case csvItem::fixtureSprIndex:
                            itemDex[arrayCounter / (csvWidth)-1].propSprIndex = wtoi(strFragment.c_str());
                            break;

                        case csvItem::equipSpr:
                            errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 equip 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                            itemDex[arrayCounter / (csvWidth)-1].equipSpr = spr::spriteMapper[strFragment.c_str()];
                            break;
                        case csvItem::equipPriority:
                            itemDex[arrayCounter / (csvWidth)-1].equipPriority = wtoi(strFragment.c_str());
                            break;

                        case csvItem::leftWieldSpr:
                            errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 leftWield 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                            itemDex[arrayCounter / (csvWidth)-1].leftWieldSpr = spr::spriteMapper[strFragment.c_str()];
                            break;
                        case csvItem::leftWieldPriority:
                            itemDex[arrayCounter / (csvWidth)-1].leftWieldPriority = wtoi(strFragment.c_str());
                            break;

                        case csvItem::rightWieldSpr:
                            errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 rightWield 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                            itemDex[arrayCounter / (csvWidth)-1].rightWieldSpr = spr::spriteMapper[strFragment.c_str()];
                            break;
                        case csvItem::rightWieldPriority:
                            itemDex[arrayCounter / (csvWidth)-1].rightWieldPriority = wtoi(strFragment.c_str());
                            break;
                        ///////////////////////////////////////////
                        case csvItem::tileSprIndex:
                            itemDex[arrayCounter / (csvWidth)-1].tileSprIndex = wtoi(strFragment.c_str());
                            break;
                        case csvItem::tileConnectGroup:
                            itemDex[arrayCounter / (csvWidth)-1].tileConnectGroup = wtoi(strFragment.c_str());
                            break;
                        case csvItem::dirChangeItemCode:
                            itemDex[arrayCounter / (csvWidth)-1].dirChangeItemCode = wtoi(strFragment.c_str());
                            break;

                        case csvItem::lightRange:
                            itemDex[arrayCounter / (csvWidth)-1].lightRange = wtoi(strFragment.c_str());
                            break;
                        case csvItem::lightIntensity:
                            itemDex[arrayCounter / (csvWidth)-1].lightIntensity = wtoi(strFragment.c_str());
                            break;
                        case csvItem::lightR:
                            itemDex[arrayCounter / (csvWidth)-1].lightR = wtoi(strFragment.c_str());
                            break;
                        case csvItem::lightG:
                            itemDex[arrayCounter / (csvWidth)-1].lightG = wtoi(strFragment.c_str());
                            break;
                        case csvItem::lightB:
                            itemDex[arrayCounter / (csvWidth)-1].lightB = wtoi(strFragment.c_str());
                            break;
                        case csvItem::lightDelX:
                            itemDex[arrayCounter / (csvWidth)-1].lightDelX = wtoi(strFragment.c_str());
                            break;
                        case csvItem::lightDelY:
                            itemDex[arrayCounter / (csvWidth)-1].lightDelY = wtoi(strFragment.c_str());
                            break;

                        case csvItem::animeSize:
                            itemDex[arrayCounter / (csvWidth)-1].animeSize = wtoi(strFragment.c_str());
                            break;
                        case csvItem::animeFPS:
                            itemDex[arrayCounter / (csvWidth)-1].animeFPS = wtoi(strFragment.c_str());
                            break;
                        case csvItem::randomPropSprSize:
                            itemDex[arrayCounter / (csvWidth)-1].randomPropSprSize = wtoi(strFragment.c_str());
                            break;
                        case csvItem::growthThreshold:
                            break;
                        case csvItem::molecularWeight:
                            itemDex[arrayCounter / (csvWidth)-1].molecularWeight = wtoi(strFragment.c_str());
                            break;
                        case csvItem::liqColorR:
                            itemDex[arrayCounter / (csvWidth)-1].liqColorR = wtoi(strFragment.c_str());
                            break;
                        case csvItem::liqColorG:
                            itemDex[arrayCounter / (csvWidth)-1].liqColorG = wtoi(strFragment.c_str());
                            break;
                        case csvItem::liqColorB:
                            itemDex[arrayCounter / (csvWidth)-1].liqColorB = wtoi(strFragment.c_str());
                            break;
                        case csvItem::gasColorR:
                            itemDex[arrayCounter / (csvWidth)-1].gasColorR = wtoi(strFragment.c_str());
                            break;
                        case csvItem::gasColorG:
                            itemDex[arrayCounter / (csvWidth)-1].gasColorG = wtoi(strFragment.c_str());
                            break;
                        case csvItem::gasColorB:
                            itemDex[arrayCounter / (csvWidth)-1].gasColorB = wtoi(strFragment.c_str());
                            break;
                        case csvItem::gasAlphaMax:
                            itemDex[arrayCounter / (csvWidth)-1].gasAlphaMax = wtoi(strFragment.c_str());
                            break;
                        case csvItem::propMaxHP:
                            itemDex[arrayCounter / (csvWidth)-1].propMaxHP = wtoi(strFragment.c_str());
                            break;
                        case csvItem::propRPierce:
                            itemDex[arrayCounter / (csvWidth)-1].propRPierce = wtoi(strFragment.c_str());
                            break;
                        case csvItem::propRCut:
                            itemDex[arrayCounter / (csvWidth)-1].propRCut = wtoi(strFragment.c_str());
                            break;
                        case csvItem::propRBash:
                            itemDex[arrayCounter / (csvWidth)-1].propRBash = wtoi(strFragment.c_str());
                            break;
                        default:
                            prt(L"readItemDex.ixx에서 오류 발생. csv의 잘못된 장소를 읽었다.\n");
                            break;
                    }

                    //prt(L"[문자열] %ws ", strFragment.c_str());
                    //prt(L"를 (%d,%d)에 입력했다.\n", arrayCounter / (csvWidth)-1, arrayCounter % (csvWidth));
                }

                arrayCounter++;

                startPoint = -1;
                endPoint = -1;
                strFragment.clear();

                if (i != str.size() - 1)//만약 다음 글자가 연속으로 쉼표면 여백이므로 건너뜀
                {
                    while (str[i + 1] == UNI::TAB || str[i + 1] == 10)
                    {
                        arrayCounter++;
                        i++;
                    }
                }
            }
            else
            {
                if (startPoint == -1)
                {
                    startPoint = i;
                }
            }
        }
        //itemDex.erase(itemDex.begin());//0번째 라벨 삭제
        return 1;
    }
    else//읽기 실패
    {

        return 0;
    }
}



