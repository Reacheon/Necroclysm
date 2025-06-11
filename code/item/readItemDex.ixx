#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <SDL3/SDL.h>

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
    constexpr int calorie = 12;
    constexpr int hydration = 13;
    constexpr int hydrationPerML = 14;

    constexpr int toolQuality = 15; // A_X 여기서 X는 레벨, 즉 1_3은 망치질 3단계
    constexpr int craftMinUnit = 16;
    constexpr int craftTime = 17;
    constexpr int recipe = 18;
    constexpr int recipeQualityNeed = 19;
    constexpr int recipeProficNeed = 20;
    constexpr int disassy = 21;
    constexpr int disassyQualityNeed = 22;
    constexpr int disassyProficNeed = 23;
    constexpr int repair = 24;
    constexpr int repairQualityNeed = 25;
    constexpr int repairProficNeed = 26;
    constexpr int pocketMaxVolume = 27;
    constexpr int pocketMaxNumber = 28;
    constexpr int pocketOnlyItem = 29;

    // Torso (몸통)
    constexpr int rPierceTorso = 30;
    constexpr int rCutTorso = 31;
    constexpr int rBashTorso = 32;
    constexpr int encTorso = 33;

    // Head (머리)
    constexpr int rPierceHead = 34;
    constexpr int rCutHead = 35;
    constexpr int rBashHead = 36;
    constexpr int encHead = 37;

    // Left Arm (왼팔)
    constexpr int rPierceLArm = 38;
    constexpr int rCutLArm = 39;
    constexpr int rBashLArm = 40;
    constexpr int encLArm = 41;

    // Right Arm (오른팔)
    constexpr int rPierceRArm = 42;
    constexpr int rCutRArm = 43;
    constexpr int rBashRArm = 44;
    constexpr int encRArm = 45;

    // Left Leg (왼다리)
    constexpr int rPierceLLeg = 46;
    constexpr int rCutLLeg = 47;
    constexpr int rBashLLeg = 48;
    constexpr int encLLeg = 49;

    // Right Leg (오른다리)
    constexpr int rPierceRLeg = 50;
    constexpr int rCutRLeg = 51;
    constexpr int rBashRLeg = 52;
    constexpr int encRLeg = 53;

    constexpr int SH = 54;
    constexpr int EV = 55;
    constexpr int rFire = 56;
    constexpr int rCold = 57;
    constexpr int rElec = 58;
    constexpr int rCorr = 59;
    constexpr int rRad = 60;
    constexpr int bionicIndex = 61;
    constexpr int durability = 62;
    constexpr int pierce = 63;
    constexpr int cut = 64;
    constexpr int bash = 65;
    constexpr int bulletPierce = 66;
    constexpr int bulletCut = 67;
    constexpr int bulletBash = 68;
    constexpr int bulletStoppingPower = 69;
    constexpr int bulletJam = 70;
    constexpr int bulletType = 71;
    constexpr int bulletRange = 72;
    constexpr int gunDmg = 73;
    constexpr int gunAccInit = 74;
    constexpr int gunAccSpd = 75;
    constexpr int gunAccMax = 76;
    constexpr int gunRebound = 77;
    constexpr int gunShotSpd = 78;
    constexpr int gunReloadSpd = 79;
    constexpr int gunJam = 80;
    constexpr int gunMod = 81;
    constexpr int gunBalance = 82;
    constexpr int gunRange = 83;
    constexpr int meleeAtkSpd = 84;
    constexpr int meleeAtkAcc = 85;
    constexpr int meleeBalance = 86;
    constexpr int meleeRange = 87;
    constexpr int modeTemplate = 88;
    constexpr int throwAtk = 89;
    constexpr int throwType = 90;
    constexpr int throwAtkAcc = 91;
    constexpr int throwBalance = 92;
    constexpr int throwRange = 93;
    constexpr int throwStoppingPower = 94;
    constexpr int bookIndex = 95;
    constexpr int equipSpr = 96;
    constexpr int equipPriority = 97;
    constexpr int flipEquipSpr = 98;
    constexpr int flipEquipPriority = 99;
    constexpr int leftWieldSpr = 100;
    constexpr int leftWieldPriority = 101;
    constexpr int rightWieldSpr = 102;
    constexpr int rightWieldPriority = 103;
    constexpr int tileConnectGroup = 104;
    constexpr int dirChangeItemCode = 105;
    constexpr int lightRange = 106;
    constexpr int lightIntensity = 107;
    constexpr int lightR = 108;
    constexpr int lightG = 109;
    constexpr int lightB = 110;
    constexpr int lightDelX = 111;
    constexpr int lightDelY = 112;
    constexpr int animeSize = 113;
    constexpr int animeFPS = 114;
    constexpr int randomPropSprSize = 115;
    constexpr int growthThreshold = 116;
    constexpr int molecularWeight = 117;
    constexpr int liqColorR = 118;
    constexpr int liqColorG = 119;
    constexpr int liqColorB = 120;
    constexpr int gasColorR = 121;
    constexpr int gasColorG = 122;
    constexpr int gasColorB = 123;
    constexpr int gasAlphaMax = 124;
    constexpr int propMaxHP = 125;
    constexpr int propRPierce = 126;
    constexpr int propRCut = 127;
    constexpr int propRBash = 128;
    constexpr int propDrawPriority = 129;
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
            if (str[i] == UNI::COMMA || str[i] == UNI::LF)//해당 문자가 쉼표(44)거나 라인피드(10)일 경우
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
                if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
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

                if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
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
            if (str[i] == UNI::COMMA || str[i] == 10)//ASCII로 44(,)와 또는 라인피드(\n)와 같을 때
            {
                if (i == str.size() - 1) { i++; }//마지막 문자면 endP-startP 보정을 위해 i를 1 더함.
                endPoint = i;

                strFragment = str.substr(startPoint, endPoint - startPoint);

                if (arrayCounter / csvWidth == itemDex.size() + 1) // 만약 벡터 크기(상하)가 부족하면 1칸 늘림
                {
                    ItemData newItem;
                    itemDex.push_back(std::move(newItem));
                }

                if (arrayCounter / (csvWidth) != 0)
                {
                    const int tgtIndex = arrayCounter / (csvWidth)-1;
                    const int columnIndex = arrayCounter % csvWidth;

                    switch (columnIndex)
                    {
                    case csvItem::name:
                        itemDex[tgtIndex].name = strFragment;
                        break;
                    case csvItem::itemCode:
                        itemDex[tgtIndex].itemCode = wtoi(strFragment.c_str());
                        break;
                    case csvItem::sprIndex:
                        itemDex[tgtIndex].sprIndex = wtoi(strFragment.c_str());
                        break;
                    case csvItem::tooltip:
                        itemTooltip.push_back(strFragment);
                        itemDex[tgtIndex].tooltipIndex = itemTooltip.size() - 1;
                        break;
                    case csvItem::category:
                        if (strFragment == L"EQUIPMENT") { itemDex[tgtIndex].category = itemCategory::equipment; }
                        else if (strFragment == L"WEAPON") { itemDex[tgtIndex].category = itemCategory::weapon; }
                        else if (strFragment == L"TOOL") { itemDex[tgtIndex].category = itemCategory::tool; }
                        else if (strFragment == L"CONSUMABLE") { itemDex[tgtIndex].category = itemCategory::consumable; }
                        else if (strFragment == L"VEHICLE") { itemDex[tgtIndex].category = itemCategory::vehicle; }
                        else if (strFragment == L"BIONIC") { itemDex[tgtIndex].category = itemCategory::bionic; }
                        else if (strFragment == L"STRUCTURE") { itemDex[tgtIndex].category = itemCategory::structure; }
                        else if (strFragment == L"MATERIAL") { itemDex[tgtIndex].category = itemCategory::material; }
                        else { errorBox(L"error in readItemDex.ixx, csvItem::category"); }

                        break;
                    case csvItem::subcategory:
                        if (itemDex[tgtIndex].category == itemCategory::equipment)
                        {
                            if (strFragment == L"CLOTHING") { itemDex[tgtIndex].subcategory = itemSubcategory::equipment_clothing; }
                            else if (strFragment == L"HAT") { itemDex[tgtIndex].subcategory = itemSubcategory::equipment_hat; }
                            else if (strFragment == L"GLOVES") { itemDex[tgtIndex].subcategory = itemSubcategory::equipment_gloves; }
                            else if (strFragment == L"SHOES") { itemDex[tgtIndex].subcategory = itemSubcategory::equipment_shoes; }
                            else if (strFragment == L"ACCESSORY") { itemDex[tgtIndex].subcategory = itemSubcategory::equipment_accessory; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(equipment)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::weapon)
                        {
                            if (strFragment == L"PIERCING") { itemDex[tgtIndex].subcategory = itemSubcategory::weapon_piercing; }
                            else if (strFragment == L"CUTTING") { itemDex[tgtIndex].subcategory = itemSubcategory::weapon_cutting; }
                            else if (strFragment == L"BASHING") { itemDex[tgtIndex].subcategory = itemSubcategory::weapon_bashing; }
                            else if (strFragment == L"SHOOTING") { itemDex[tgtIndex].subcategory = itemSubcategory::weapon_shooting; }
                            else if (strFragment == L"THROWING") { itemDex[tgtIndex].subcategory = itemSubcategory::weapon_throwing; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(weapon)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::tool)
                        {
                            if (strFragment == L"HAND") { itemDex[tgtIndex].subcategory = itemSubcategory::tool_hand; }
                            else if (strFragment == L"POWER") { itemDex[tgtIndex].subcategory = itemSubcategory::tool_power; }
                            else if (strFragment == L"CONTAINER") { itemDex[tgtIndex].subcategory = itemSubcategory::tool_container; }
                            else if (strFragment == L"DEVICE") { itemDex[tgtIndex].subcategory = itemSubcategory::tool_device; }
                            else if (strFragment == L"DOCUMENT") { itemDex[tgtIndex].subcategory = itemSubcategory::tool_document; }
                            else if (strFragment == L"ETC") { itemDex[tgtIndex].subcategory = itemSubcategory::tool_etc; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(tool)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::consumable)
                        {
                            if (strFragment == L"FOOD") { itemDex[tgtIndex].subcategory = itemSubcategory::consumable_food; }
                            else if (strFragment == L"MEDICINE") { itemDex[tgtIndex].subcategory = itemSubcategory::consumable_medicine; }
                            else if (strFragment == L"AMMO") { itemDex[tgtIndex].subcategory = itemSubcategory::consumable_ammo; }
                            else if (strFragment == L"FUEL") { itemDex[tgtIndex].subcategory = itemSubcategory::consumable_fuel; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(consumable)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::vehicle)
                        {
                            if (strFragment == L"FRAME") { itemDex[tgtIndex].subcategory = itemSubcategory::vehicle_frame; }
                            else if (strFragment == L"ENGINE") { itemDex[tgtIndex].subcategory = itemSubcategory::vehicle_engine; }
                            else if (strFragment == L"EXTERIOR") { itemDex[tgtIndex].subcategory = itemSubcategory::vehicle_exterior; }
                            else if (strFragment == L"TRANSPORT") { itemDex[tgtIndex].subcategory = itemSubcategory::vehicle_transport; }
                            else if (strFragment == L"ENERGY") { itemDex[tgtIndex].subcategory = itemSubcategory::vehicle_energy; }
                            else if (strFragment == L"DEVICE") { itemDex[tgtIndex].subcategory = itemSubcategory::vehicle_device; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(container)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::bionic)
                        {
                            if (strFragment == L"CORE") { itemDex[tgtIndex].subcategory = itemSubcategory::bionic_core; }
                            else if (strFragment == L"ACTIVE") { itemDex[tgtIndex].subcategory = itemSubcategory::bionic_active; }
                            else if (strFragment == L"PASSIVE") { itemDex[tgtIndex].subcategory = itemSubcategory::bionic_passive; }
                            else if (strFragment == L"TOGGLE") { itemDex[tgtIndex].subcategory = itemSubcategory::bionic_toggle; }
                            else if (strFragment == L"GENERATOR") { itemDex[tgtIndex].subcategory = itemSubcategory::bionic_generator; }
                            else if (strFragment == L"STORAGE") { itemDex[tgtIndex].subcategory = itemSubcategory::bionic_storage; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(container)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::material)
                        {
                            if (strFragment == L"CHEMICAL") { itemDex[tgtIndex].subcategory = itemSubcategory::material_chemical; }
                            else if (strFragment == L"BIOLOGICAL") { itemDex[tgtIndex].subcategory = itemSubcategory::material_biological; }
                            else if (strFragment == L"MECHANICAL") { itemDex[tgtIndex].subcategory = itemSubcategory::material_mechanical; }
                            else if (strFragment == L"ELECTRICAL") { itemDex[tgtIndex].subcategory = itemSubcategory::material_electrical; }
                            else if (strFragment == L"ETC") { itemDex[tgtIndex].subcategory = itemSubcategory::material_etc; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(material)"); }
                        }
                        else if (itemDex[tgtIndex].category == itemCategory::structure)
                        {
                            if (strFragment == L"WALL") { itemDex[tgtIndex].subcategory = itemSubcategory::structure_wall; }
                            else if (strFragment == L"FLOOR") { itemDex[tgtIndex].subcategory = itemSubcategory::structure_floor; }
                            else if (strFragment == L"CEIL") { itemDex[tgtIndex].subcategory = itemSubcategory::structure_ceil; }
                            else if (strFragment == L"PROP") { itemDex[tgtIndex].subcategory = itemSubcategory::structure_prop; }
                            else if (strFragment == L"ELECTRIC") { itemDex[tgtIndex].subcategory = itemSubcategory::structure_electric; }
                            else if (strFragment == L"PNEUMATIC") { itemDex[tgtIndex].subcategory = itemSubcategory::structure_pneumatic; }
                            else { errorBox(L"error in readItemDex.ixx, csvItem::subcategory(structure)"); }
                        }
                        else { errorBox(L"error in readItemDex.ixx, itemCategory"); }
                        break;
                    case csvItem::weight:
                        itemDex[tgtIndex].weight = wtoi(strFragment.c_str());
                        break;
                    case csvItem::volume:
                        itemDex[tgtIndex].volume = wtoi(strFragment.c_str());
                        break;
                    case csvItem::material:
                        break;
                    case csvItem::flag:
                        for (int j = 0; j < strFragment.size(); j++)
                        {
                            if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
                            {
                                if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                if (strFragment.substr(0, j) == L"TWOHANDED") itemDex[tgtIndex].flag.push_back(itemFlag::TWOHANDED);
                                else if (strFragment.substr(0, j) == L"CANEQUIP") itemDex[tgtIndex].flag.push_back(itemFlag::CANEQUIP);
                                else if (strFragment.substr(0, j) == L"NONSTACK") itemDex[tgtIndex].flag.push_back(itemFlag::NONSTACK);
                                else if (strFragment.substr(0, j) == L"GUN") itemDex[tgtIndex].flag.push_back(itemFlag::GUN);
                                else if (strFragment.substr(0, j) == L"MAGAZINE") itemDex[tgtIndex].flag.push_back(itemFlag::MAGAZINE);
                                else if (strFragment.substr(0, j) == L"AMMO") itemDex[tgtIndex].flag.push_back(itemFlag::AMMO);
                                else if (strFragment.substr(0, j) == L"POWERED") itemDex[tgtIndex].flag.push_back(itemFlag::POWERED);
                                else if (strFragment.substr(0, j) == L"RAIDARMOR") itemDex[tgtIndex].flag.push_back(itemFlag::RAIDARMOR);
                                else if (strFragment.substr(0, j) == L"COORDCRAFT") itemDex[tgtIndex].flag.push_back(itemFlag::COORDCRAFT);
                                else if (strFragment.substr(0, j) == L"VFRAME") itemDex[tgtIndex].flag.push_back(itemFlag::VFRAME);
                                else if (strFragment.substr(0, j) == L"ALCHEMYTOOL") itemDex[tgtIndex].flag.push_back(itemFlag::ALCHEMYTOOL);
                                else if (strFragment.substr(0, j) == L"ALCHEMYMATERIAL") itemDex[tgtIndex].flag.push_back(itemFlag::ALCHEMYMATERIAL);
                                else if (strFragment.substr(0, j) == L"LIQUID") itemDex[tgtIndex].flag.push_back(itemFlag::LIQUID);
                                else if (strFragment.substr(0, j) == L"GAS") itemDex[tgtIndex].flag.push_back(itemFlag::GAS);
                                else if (strFragment.substr(0, j) == L"VPART") itemDex[tgtIndex].flag.push_back(itemFlag::VPART);
                                else if (strFragment.substr(0, j) == L"TRANSPARENT_WALL") itemDex[tgtIndex].flag.push_back(itemFlag::TRANSPARENT_WALL);
                                else if (strFragment.substr(0, j) == L"PROP_BIG") itemDex[tgtIndex].flag.push_back(itemFlag::PROP_BIG);
                                else if (strFragment.substr(0, j) == L"VEH_ROOF") itemDex[tgtIndex].flag.push_back(itemFlag::VEH_ROOF);
                                else if (strFragment.substr(0, j) == L"PROP_WALKABLE") itemDex[tgtIndex].flag.push_back(itemFlag::PROP_WALKABLE);
                                else if (strFragment.substr(0, j) == L"PROP_BLOCKER") itemDex[tgtIndex].flag.push_back(itemFlag::PROP_BLOCKER);
                                else if (strFragment.substr(0, j) == L"PROP_DEPTH_LOWER") itemDex[tgtIndex].flag.push_back(itemFlag::PROP_DEPTH_LOWER);
                                else if (strFragment.substr(0, j) == L"LIGHT_ON") itemDex[tgtIndex].flag.push_back(itemFlag::LIGHT_ON);
                                else if (strFragment.substr(0, j) == L"LIGHT_OFF") itemDex[tgtIndex].flag.push_back(itemFlag::LIGHT_OFF);
                                else if (strFragment.substr(0, j) == L"VPART_WALL_CONNECT") itemDex[tgtIndex].flag.push_back(itemFlag::VPART_WALL_CONNECT);
                                else if (strFragment.substr(0, j) == L"VPART_DIR_DEPEND") itemDex[tgtIndex].flag.push_back(itemFlag::VPART_DIR_DEPEND);
                                else if (strFragment.substr(0, j) == L"VPART_DOOR_OPEN") itemDex[tgtIndex].flag.push_back(itemFlag::VPART_DOOR_OPEN);
                                else if (strFragment.substr(0, j) == L"VPART_DOOR_CLOSE") itemDex[tgtIndex].flag.push_back(itemFlag::VPART_DOOR_CLOSE);

                                else if (strFragment.substr(0, j) == L"PIPE_CNCT_RIGHT") itemDex[tgtIndex].flag.push_back(itemFlag::PIPE_CNCT_RIGHT);
                                else if (strFragment.substr(0, j) == L"PIPE_CNCT_TOP") itemDex[tgtIndex].flag.push_back(itemFlag::PIPE_CNCT_TOP);
                                else if (strFragment.substr(0, j) == L"PIPE_CNCT_LEFT") itemDex[tgtIndex].flag.push_back(itemFlag::PIPE_CNCT_LEFT);
                                else if (strFragment.substr(0, j) == L"PIPE_CNCT_BOT") itemDex[tgtIndex].flag.push_back(itemFlag::PIPE_CNCT_BOT);

                                else if (strFragment.substr(0, j) == L"WIRE_CNCT_RIGHT") itemDex[tgtIndex].flag.push_back(itemFlag::WIRE_CNCT_RIGHT);
                                else if (strFragment.substr(0, j) == L"WIRE_CNCT_TOP") itemDex[tgtIndex].flag.push_back(itemFlag::WIRE_CNCT_TOP);
                                else if (strFragment.substr(0, j) == L"WIRE_CNCT_LEFT") itemDex[tgtIndex].flag.push_back(itemFlag::WIRE_CNCT_LEFT);
                                else if (strFragment.substr(0, j) == L"WIRE_CNCT_BOT") itemDex[tgtIndex].flag.push_back(itemFlag::WIRE_CNCT_BOT);

                                else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_RIGHT") itemDex[tgtIndex].flag.push_back(itemFlag::CONVEYOR_CNCT_RIGHT);
                                else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_TOP") itemDex[tgtIndex].flag.push_back(itemFlag::CONVEYOR_CNCT_TOP);
                                else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_LEFT") itemDex[tgtIndex].flag.push_back(itemFlag::CONVEYOR_CNCT_LEFT);
                                else if (strFragment.substr(0, j) == L"CONVEYOR_CNCT_BOT") itemDex[tgtIndex].flag.push_back(itemFlag::CONVEYOR_CNCT_BOT);

                                else if (strFragment.substr(0, j) == L"TIRE_NORMAL") itemDex[tgtIndex].flag.push_back(itemFlag::TIRE_NORMAL);
                                else if (strFragment.substr(0, j) == L"TIRE_STEER") itemDex[tgtIndex].flag.push_back(itemFlag::TIRE_STEER);

                                else if (strFragment.substr(0, j) == L"PROP") itemDex[tgtIndex].flag.push_back(itemFlag::PROP);

                                else if (strFragment.substr(0, j) == L"WIRE") itemDex[tgtIndex].flag.push_back(itemFlag::WIRE);
                                else if (strFragment.substr(0, j) == L"PIPE") itemDex[tgtIndex].flag.push_back(itemFlag::PIPE);
                                else if (strFragment.substr(0, j) == L"CONVEYOR") itemDex[tgtIndex].flag.push_back(itemFlag::CONVEYOR);

                                else if (strFragment.substr(0, j) == L"RAIL") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL);

                                else if (strFragment.substr(0, j) == L"RAIL_CNCT_TOP") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_CNCT_TOP);
                                else if (strFragment.substr(0, j) == L"RAIL_CNCT_BOT") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_CNCT_BOT);
                                else if (strFragment.substr(0, j) == L"RAIL_CNCT_LEFT") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_CNCT_LEFT);
                                else if (strFragment.substr(0, j) == L"RAIL_CNCT_RIGHT") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_CNCT_RIGHT);

                                else if (strFragment.substr(0, j) == L"RAIL_INPUT_TOP") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_INPUT_TOP);
                                else if (strFragment.substr(0, j) == L"RAIL_INPUT_BOT") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_INPUT_BOT);
                                else if (strFragment.substr(0, j) == L"RAIL_INPUT_LEFT") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_INPUT_LEFT);
                                else if (strFragment.substr(0, j) == L"RAIL_INPUT_RIGHT") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_INPUT_RIGHT);

                                else if (strFragment.substr(0, j) == L"RAIL_BUFFER") itemDex[tgtIndex].flag.push_back(itemFlag::RAIL_BUFFER);

                                else if (strFragment.substr(0, j) == L"NOT_RECIPE") itemDex[tgtIndex].flag.push_back(itemFlag::NOT_RECIPE);

                                else if (strFragment.substr(0, j) == L"TREE") itemDex[tgtIndex].flag.push_back(itemFlag::TREE);

                                else if (strFragment.substr(0, j) == L"PLANT") itemDex[tgtIndex].flag.push_back(itemFlag::PLANT);
                                else if (strFragment.substr(0, j) == L"PLANT_SEASON_DEPENDENT") itemDex[tgtIndex].flag.push_back(itemFlag::PLANT_SEASON_DEPENDENT);
                                else if (strFragment.substr(0, j) == L"PLANT_GROWTH_DEPENDENT") itemDex[tgtIndex].flag.push_back(itemFlag::PLANT_GROWTH_DEPENDENT);
                                else if (strFragment.substr(0, j) == L"MUSHROOM") itemDex[tgtIndex].flag.push_back(itemFlag::MUSHROOM);
                                else if (strFragment.substr(0, j) == L"FLOOR") itemDex[tgtIndex].flag.push_back(itemFlag::FLOOR);
                                else if (strFragment.substr(0, j) == L"WALL") itemDex[tgtIndex].flag.push_back(itemFlag::WALL);
                                else if (strFragment.substr(0, j) == L"CEIL") itemDex[tgtIndex].flag.push_back(itemFlag::CEIL);
                                else if (strFragment.substr(0, j) == L"PROP") itemDex[tgtIndex].flag.push_back(itemFlag::PROP);
                                else if (strFragment.substr(0, j) == L"WATER_SHALLOW") itemDex[tgtIndex].flag.push_back(itemFlag::WATER_SHALLOW);
                                else if (strFragment.substr(0, j) == L"WATER_DEEP") itemDex[tgtIndex].flag.push_back(itemFlag::WATER_DEEP);
                                else if (strFragment.substr(0, j) == L"FRESHWATER") itemDex[tgtIndex].flag.push_back(itemFlag::FRESHWATER);
                                else if (strFragment.substr(0, j) == L"SEAWATER") itemDex[tgtIndex].flag.push_back(itemFlag::SEAWATER);
                                else if (strFragment.substr(0, j) == L"TILE_SEASON") itemDex[tgtIndex].flag.push_back(itemFlag::TILE_SEASON);

                                else if (strFragment.substr(0, j) == L"DOOR") itemDex[tgtIndex].flag.push_back(itemFlag::DOOR);
                                else if (strFragment.substr(0, j) == L"UPSTAIR") itemDex[tgtIndex].flag.push_back(itemFlag::UPSTAIR);
                                else if (strFragment.substr(0, j) == L"DOWNSTAIR") itemDex[tgtIndex].flag.push_back(itemFlag::DOWNSTAIR);
                                else if (strFragment.substr(0, j) == L"SIGN") itemDex[tgtIndex].flag.push_back(itemFlag::SIGN);
                                else if (strFragment.substr(0, j) == L"DOOR_CLOSE") itemDex[tgtIndex].flag.push_back(itemFlag::DOOR_CLOSE);
                                else if (strFragment.substr(0, j) == L"DOOR_OEPN") itemDex[tgtIndex].flag.push_back(itemFlag::DOOR_OPEN);
                                else if (strFragment.substr(0, j) == L"TRAIN_WHEEL") itemDex[tgtIndex].flag.push_back(itemFlag::TRAIN_WHEEL);

                                else if (strFragment.substr(0, j) == L"PROP_GAS_OBSTACLE_ON") itemDex[tgtIndex].flag.push_back(itemFlag::PROP_GAS_OBSTACLE_ON);
                                else if (strFragment.substr(0, j) == L"PROP_GAS_OBSTACLE_OFF") itemDex[tgtIndex].flag.push_back(itemFlag::PROP_GAS_OBSTACLE_OFF);
                                else if (strFragment.substr(0, j) == L"WALL_GAS_PERMEABLE") itemDex[tgtIndex].flag.push_back(itemFlag::WALL_GAS_PERMEABLE);

                                else if (strFragment.substr(0, j) == L"POCKET") itemDex[tgtIndex].flag.push_back(itemFlag::POCKET);
                                else if (strFragment.substr(0, j) == L"CAN_CLIMB") itemDex[tgtIndex].flag.push_back(itemFlag::CAN_CLIMB);
                                else if (strFragment.substr(0, j) == L"SPR_TH_WEAPON") itemDex[tgtIndex].flag.push_back(itemFlag::SPR_TH_WEAPON);
                                else if (strFragment.substr(0, j) == L"NO_HAIR_HELMET") itemDex[tgtIndex].flag.push_back(itemFlag::NO_HAIR_HELMET);
                                else if (strFragment.substr(0, j) == L"BOW") itemDex[tgtIndex].flag.push_back(itemFlag::BOW);
                                else if (strFragment.substr(0, j) == L"CROSSBOW") itemDex[tgtIndex].flag.push_back(itemFlag::CROSSBOW);
                                else if (strFragment.substr(0, j) == L"TOGGLE_ON") itemDex[tgtIndex].flag.push_back(itemFlag::TOGGLE_ON);
                                else if (strFragment.substr(0, j) == L"TOGGLE_OFF") itemDex[tgtIndex].flag.push_back(itemFlag::TOGGLE_OFF);
                                else if (strFragment.substr(0, j) == L"HAS_TOGGLE_SPRITE") itemDex[tgtIndex].flag.push_back(itemFlag::HAS_TOGGLE_SPRITE);
                                else if (strFragment.substr(0, j) == L"CANCRAFT") itemDex[tgtIndex].flag.push_back(itemFlag::CANCRAFT);
                                else if (strFragment.substr(0, j) == L"HEADLIGHT") itemDex[tgtIndex].flag.push_back(itemFlag::HEADLIGHT);
                                else if (strFragment.substr(0, j) == L"SHIELD") itemDex[tgtIndex].flag.push_back(itemFlag::SHIELD);
                                else if (strFragment.substr(0, j) == L"CONTAINER_LIQ") itemDex[tgtIndex].flag.push_back(itemFlag::CONTAINER_LIQ);
                                else if (strFragment.substr(0, j) == L"VPART_NOT_WALKABLE") itemDex[tgtIndex].flag.push_back(itemFlag::VPART_NOT_WALKABLE);
                                else if (strFragment.substr(0, j) == L"ENGINE_GASOLINE") itemDex[tgtIndex].flag.push_back(itemFlag::ENGINE_GASOLINE);
                                else if (strFragment.substr(0, j) == L"ENGINE_DISEL") itemDex[tgtIndex].flag.push_back(itemFlag::ENGINE_DISEL);
                                else if (strFragment.substr(0, j) == L"ENGINE_ELECTRIC") itemDex[tgtIndex].flag.push_back(itemFlag::ENGINE_ELECTRIC);
                                else if (strFragment.substr(0, j) == L"CAN_EAT") itemDex[tgtIndex].flag.push_back(itemFlag::CAN_EAT);
                                else if (strFragment.substr(0, j) == L"CAN_DRINK") itemDex[tgtIndex].flag.push_back(itemFlag::CAN_DRINK);
                                else
                                {
                                    errorBox(L"error in readItemDex.ixx, csvItem::flag, unknown itemFlag defined " + strFragment.substr(0, j));
                                }

                                strFragment.erase(0, j + 1);
                                j = 0;
                            }
                        }
                        break;
                    case csvItem::calorie:
                        itemDex[tgtIndex].calorie = wtoi(strFragment.c_str());
                        break;
                    case csvItem::hydration:
                        itemDex[tgtIndex].hydration = wtoi(strFragment.c_str());    
                        break;
                    case csvItem::hydrationPerML:
                        itemDex[tgtIndex].hydrationPerML = wtof(strFragment.c_str());
                        break;
                    case csvItem::toolQuality:
                    {
                        for (int j = 0; j < strFragment.size(); j++)
                        {
                            if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
                            {
                                if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                itemDex[tgtIndex].toolQuality.push_back(wtoi(strFragment.substr(0, j).c_str()));
                                strFragment.erase(0, j + 1);
                                j = 0;
                            }
                        }
                        break;
                    }
                    case csvItem::craftMinUnit:
                    {
                        itemDex[tgtIndex].craftMinUnit = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::craftTime:
                    {
                        itemDex[tgtIndex].craftTime = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::recipe:
                    {
                        pairsToVec(strFragment, itemDex[tgtIndex].recipe);
                        break;
                    }
                    case csvItem::recipeQualityNeed:
                    {
                        for (int j = 0; j < strFragment.size(); j++)
                        {
                            if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
                            {
                                if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                itemDex[tgtIndex].recipeQualityNeed.push_back(wtoi(strFragment.substr(0, j).c_str()));
                                strFragment.erase(0, j + 1);
                                j = 0;
                            }
                        }
                        break;
                    }
                    case csvItem::recipeProficNeed:
                    {
                        pairsToVec(strFragment, itemDex[tgtIndex].recipeProficNeed);
                        break;
                    }
                    case csvItem::disassy:
                    {
                        pairsToVec(strFragment, itemDex[tgtIndex].disassy);
                        break;
                    }
                    case csvItem::disassyQualityNeed:
                    {
                        for (int j = 0; j < strFragment.size(); j++)
                        {
                            if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
                            {
                                if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                itemDex[tgtIndex].disassyQualityNeed.push_back(wtoi(strFragment.substr(0, j).c_str()));
                                strFragment.erase(0, j + 1);
                                j = 0;
                            }
                        }
                        break;
                    }
                    case csvItem::disassyProficNeed:
                    {
                        pairsToVec(strFragment, itemDex[tgtIndex].disassyProficNeed);
                        break;
                    }
                    case csvItem::repair:
                    {
                        pairsToVec(strFragment, itemDex[tgtIndex].repair);
                        break;
                    }
                    case csvItem::repairQualityNeed:
                    {
                        for (int j = 0; j < strFragment.size(); j++)
                        {
                            if (strFragment[j] == UNI::SEMICOLON || j == strFragment.size() - 1)
                            {
                                if (j == strFragment.size() - 1) { j++; } //마지막이면 j값을 1 더하여 보정

                                itemDex[tgtIndex].repairQualityNeed.push_back(wtoi(strFragment.c_str()));
                                strFragment.erase(0, j + 1);
                                j = 0;
                            }
                        }
                        break;
                    }
                    case csvItem::repairProficNeed:
                    {
                        pairsToVec(strFragment, itemDex[tgtIndex].repairProficNeed);
                        break;
                    }
                    case csvItem::pocketMaxVolume:
                        itemDex[tgtIndex].pocketMaxVolume = wtoi(strFragment.c_str());
                        break;
                    case csvItem::pocketMaxNumber:
                        itemDex[tgtIndex].pocketMaxNumber = wtoi(strFragment.c_str());
                        break;
                    case csvItem::pocketOnlyItem:
                    {
                        valsToVec(strFragment, itemDex[tgtIndex].pocketOnlyItem);
                        break;
                    }
                    case csvItem::rPierceTorso:
                    {
                        itemDex[tgtIndex].rPierceTorso = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rCutTorso:
                    {
                        itemDex[tgtIndex].rCutTorso = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rBashTorso:
                    {
                        itemDex[tgtIndex].rBashTorso = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::encTorso:
                    {
                        itemDex[tgtIndex].encTorso = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rPierceHead:
                    {
                        itemDex[tgtIndex].rPierceHead = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rCutHead:
                    {
                        itemDex[tgtIndex].rCutHead = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rBashHead:
                    {
                        itemDex[tgtIndex].rBashHead = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::encHead:
                    {
                        itemDex[tgtIndex].encHead = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rPierceLArm:
                    {
                        itemDex[tgtIndex].rPierceLArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rCutLArm:
                    {
                        itemDex[tgtIndex].rCutLArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rBashLArm:
                    {
                        itemDex[tgtIndex].rBashLArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::encLArm:
                    {
                        itemDex[tgtIndex].encLArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rPierceRArm:
                    {
                        itemDex[tgtIndex].rPierceRArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rCutRArm:
                    {
                        itemDex[tgtIndex].rCutRArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rBashRArm:
                    {
                        itemDex[tgtIndex].rBashRArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::encRArm:
                    {
                        itemDex[tgtIndex].encRArm = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rPierceLLeg:
                    {
                        itemDex[tgtIndex].rPierceLLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rCutLLeg:
                    {
                        itemDex[tgtIndex].rCutLLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rBashLLeg:
                    {
                        itemDex[tgtIndex].rBashLLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::encLLeg:
                    {
                        itemDex[tgtIndex].encLLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rPierceRLeg:
                    {
                        itemDex[tgtIndex].rPierceRLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rCutRLeg:
                    {
                        itemDex[tgtIndex].rCutRLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::rBashRLeg:
                    {
                        itemDex[tgtIndex].rBashRLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::encRLeg:
                    {
                        itemDex[tgtIndex].encRLeg = wtoi(strFragment.c_str());
                        break;
                    }
                    case csvItem::SH:
                        itemDex[tgtIndex].sh = wtoi(strFragment.c_str());
                        break;
                    case csvItem::EV:
                        itemDex[tgtIndex].ev = wtoi(strFragment.c_str());
                        break;
                    case csvItem::rFire:
                        itemDex[tgtIndex].rFire = wtoi(strFragment.c_str());
                        break;
                    case csvItem::rCold:
                        itemDex[tgtIndex].rCold = wtoi(strFragment.c_str());
                        break;
                    case csvItem::rElec:
                        itemDex[tgtIndex].rElec = wtoi(strFragment.c_str());
                        break;
                    case csvItem::rCorr:
                        itemDex[tgtIndex].rCorr = wtoi(strFragment.c_str());
                        break;
                    case csvItem::rRad:
                        itemDex[tgtIndex].rRad = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bionicIndex:
                        itemDex[tgtIndex].bionicIndex = wtoi(strFragment.c_str());
                        break;
                    case csvItem::durability:
                        itemDex[tgtIndex].durablility = wtoi(strFragment.c_str());
                        break;
                    case csvItem::pierce:
                        itemDex[tgtIndex].pierce = wtoi(strFragment.c_str());
                        break;
                    case csvItem::cut:
                        itemDex[tgtIndex].cut = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bash:
                        itemDex[tgtIndex].bash = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bulletPierce:
                        itemDex[tgtIndex].bulletPierce = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bulletCut:
                        itemDex[tgtIndex].bulletCut = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bulletBash:
                        itemDex[tgtIndex].bulletBash = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bulletStoppingPower:
                        itemDex[tgtIndex].bulletStoppingPower = wtof(strFragment.c_str());
                        break;
                    case csvItem::bulletJam:
                        itemDex[tgtIndex].bulletJam = wtoi(strFragment.c_str());
                        break;
                    case csvItem::bulletType:
                        if (strFragment == L"NORMAL") { itemDex[tgtIndex].bulletType = bulletFlag::normal; }
                        else if (strFragment == L"TRACER") { itemDex[tgtIndex].bulletType = bulletFlag::tracer; }
                        else if (strFragment == L"AP") { itemDex[tgtIndex].bulletType = bulletFlag::ap; }
                        else { errorBox(L"error in readItemDex.ixx, csvItem::bulletType"); }
                        break;
                    case csvItem::bulletRange:
                        itemDex[tgtIndex].bulletRange = wtoi(strFragment.c_str());
                        break;

                    case csvItem::gunDmg:
                        itemDex[tgtIndex].gunDmg = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunAccInit:
                        itemDex[tgtIndex].gunAccInit = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunAccSpd:
                        itemDex[tgtIndex].gunAccSpd = wtoi(strFragment.c_str());
                        break;
                    case csvItem::gunAccMax:
                        itemDex[tgtIndex].gunAccMax = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunRebound:
                        itemDex[tgtIndex].gunRebound = wtoi(strFragment.c_str());
                        break;
                    case csvItem::gunShotSpd:
                        itemDex[tgtIndex].gunShotSpd = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunReloadSpd:
                        itemDex[tgtIndex].gunReloadSpd = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunJam:
                        itemDex[tgtIndex].gunJam = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunMod:
                    {
                        valsToVec(strFragment, itemDex[tgtIndex].gunMod);
                        break;
                    }
                    case csvItem::gunBalance:
                        itemDex[tgtIndex].gunBalance = wtof(strFragment.c_str());
                        break;
                    case csvItem::gunRange:
                        itemDex[tgtIndex].gunRange = wtoi(strFragment.c_str());
                        break;


                    case csvItem::meleeAtkSpd:
                        itemDex[tgtIndex].meleeAtkSpd = wtof(strFragment.c_str());
                        break;
                    case csvItem::meleeAtkAcc:
                        itemDex[tgtIndex].meleeAtkAcc = wtof(strFragment.c_str());
                        break;
                    case csvItem::meleeBalance:
                        itemDex[tgtIndex].meleeBalance = wtof(strFragment.c_str());
                        break;
                    case csvItem::meleeRange:
                        itemDex[tgtIndex].meleeRange = wtoi(strFragment.c_str());
                        break;


                    case csvItem::modeTemplate:
                        itemDex[tgtIndex].modeTemplate = wtoi(strFragment.c_str());
                        if (itemDex[tgtIndex].modeTemplate == 1)
                        {
                            itemDex[tgtIndex].modeCurrent = weaponMode::semiAutoMode;
                        }
                        break;

                    case csvItem::throwAtk:
                        itemDex[tgtIndex].throwAtk = wtoi(strFragment.c_str());
                        break;
                    case csvItem::throwType:
                        itemDex[tgtIndex].throwType = wtoi(strFragment.c_str());
                        break;
                    case csvItem::throwAtkAcc:
                        itemDex[tgtIndex].throwAtkAcc = wtof(strFragment.c_str());
                        break;
                    case csvItem::throwBalance:
                        itemDex[tgtIndex].throwBalance = wtof(strFragment.c_str());
                        break;
                    case csvItem::throwRange:
                        itemDex[tgtIndex].throwRange = wtoi(strFragment.c_str());
                        break;
                    case csvItem::throwStoppingPower:
                        itemDex[tgtIndex].throwStoppingPower = wtof(strFragment.c_str());
                        break;
                    case csvItem::bookIndex:
                        itemDex[tgtIndex].bookIndex = wtoi(strFragment.c_str());
                        break;
                    case csvItem::fixtureSprIndex:
                        itemDex[tgtIndex].propSprIndex = wtoi(strFragment.c_str());
                        break;

                    case csvItem::equipSpr:
                        errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 equip 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                        itemDex[tgtIndex].equipSpr = spr::spriteMapper[strFragment.c_str()];

                        if (itemDex[tgtIndex].checkFlag(itemFlag::HAS_TOGGLE_SPRITE))
                        {
                            itemDex[tgtIndex].equipSprToggleOn = spr::spriteMapper[(strFragment + L"On").c_str()];
                        }
                        break;
                    case csvItem::equipPriority:
                        itemDex[tgtIndex].equipPriority = wtoi(strFragment.c_str());
                        break;
                    case csvItem::flipEquipSpr:
                        errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 equip 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                        itemDex[tgtIndex].flipEquipSpr = spr::spriteMapper[strFragment.c_str()];

                        if (itemDex[tgtIndex].checkFlag(itemFlag::HAS_TOGGLE_SPRITE))
                        {
                            itemDex[tgtIndex].flipEquipSprToggleOn = spr::spriteMapper[(strFragment + L"On").c_str()];
                        }
                        break;
                    case csvItem::flipEquipPriority:
                        itemDex[tgtIndex].flipEquipPriority = wtoi(strFragment.c_str());
                        break;

                    case csvItem::leftWieldSpr:
                        errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 leftWield 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                        itemDex[tgtIndex].leftWieldSpr = spr::spriteMapper[strFragment.c_str()];
                        break;
                    case csvItem::leftWieldPriority:
                        itemDex[tgtIndex].leftWieldPriority = wtoi(strFragment.c_str());
                        break;

                    case csvItem::rightWieldSpr:
                        errorBox(spr::spriteMapper.find(strFragment) == spr::spriteMapper.end(), L"이 아이템의 rightWield 이미지 파일이 spr::spriteMapper에 없음 : " + strFragment);
                        itemDex[tgtIndex].rightWieldSpr = spr::spriteMapper[strFragment.c_str()];
                        break;
                    case csvItem::rightWieldPriority:
                        itemDex[tgtIndex].rightWieldPriority = wtoi(strFragment.c_str());
                        break;
                        ///////////////////////////////////////////
                    case csvItem::tileSprIndex:
                        itemDex[tgtIndex].tileSprIndex = wtoi(strFragment.c_str());
                        break;
                    case csvItem::tileConnectGroup:
                        itemDex[tgtIndex].tileConnectGroup = wtoi(strFragment.c_str());
                        break;
                    case csvItem::dirChangeItemCode:
                        itemDex[tgtIndex].dirChangeItemCode = wtoi(strFragment.c_str());
                        break;

                    case csvItem::lightRange:
                        itemDex[tgtIndex].lightRange = wtoi(strFragment.c_str());
                        break;
                    case csvItem::lightIntensity:
                        itemDex[tgtIndex].lightIntensity = wtoi(strFragment.c_str());
                        break;
                    case csvItem::lightR:
                        itemDex[tgtIndex].lightR = wtoi(strFragment.c_str());
                        break;
                    case csvItem::lightG:
                        itemDex[tgtIndex].lightG = wtoi(strFragment.c_str());
                        break;
                    case csvItem::lightB:
                        itemDex[tgtIndex].lightB = wtoi(strFragment.c_str());
                        break;
                    case csvItem::lightDelX:
                        itemDex[tgtIndex].lightDelX = wtoi(strFragment.c_str());
                        break;
                    case csvItem::lightDelY:
                        itemDex[tgtIndex].lightDelY = wtoi(strFragment.c_str());
                        break;

                    case csvItem::animeSize:
                        itemDex[tgtIndex].animeSize = wtoi(strFragment.c_str());
                        break;
                    case csvItem::animeFPS:
                        itemDex[tgtIndex].animeFPS = wtoi(strFragment.c_str());
                        break;
                    case csvItem::randomPropSprSize:
                        itemDex[tgtIndex].randomPropSprSize = wtoi(strFragment.c_str());
                        break;
                    case csvItem::growthThreshold:
                        break;
                    case csvItem::molecularWeight:
                        itemDex[tgtIndex].molecularWeight = wtoi(strFragment.c_str());
                        break;
                    case csvItem::liqColorR:
                        itemDex[tgtIndex].liqColorR = wtoi(strFragment.c_str());
                        break;
                    case csvItem::liqColorG:
                        itemDex[tgtIndex].liqColorG = wtoi(strFragment.c_str());
                        break;
                    case csvItem::liqColorB:
                        itemDex[tgtIndex].liqColorB = wtoi(strFragment.c_str());
                        break;
                    case csvItem::gasColorR:
                        itemDex[tgtIndex].gasColorR = wtoi(strFragment.c_str());
                        break;
                    case csvItem::gasColorG:
                        itemDex[tgtIndex].gasColorG = wtoi(strFragment.c_str());
                        break;
                    case csvItem::gasColorB:
                        itemDex[tgtIndex].gasColorB = wtoi(strFragment.c_str());
                        break;
                    case csvItem::gasAlphaMax:
                        itemDex[tgtIndex].gasAlphaMax = wtoi(strFragment.c_str());
                        break;
                    case csvItem::propMaxHP:
                        itemDex[tgtIndex].propMaxHP = wtoi(strFragment.c_str());
                        break;
                    case csvItem::propRPierce:
                        itemDex[tgtIndex].propRPierce = wtoi(strFragment.c_str());
                        break;
                    case csvItem::propRCut:
                        itemDex[tgtIndex].propRCut = wtoi(strFragment.c_str());
                        break;
                    case csvItem::propRBash:
                        itemDex[tgtIndex].propRBash = wtoi(strFragment.c_str());
                        break;
                    case csvItem::propDrawPriority:
                        itemDex[tgtIndex].propDrawPriority = wtoi(strFragment.c_str());
                        break;
                    default:
                        prt(L"readItemDex.ixx에서 오류 발생. csv의 잘못된 장소를 읽었다.\n");
                        break;
                    }

                    //prt(L"[문자열] %ws ", strFragment.c_str());
                    //\prt(L"를 (%d,%d)에 입력했다.\n", tgtIndex, arrayCounter % (csvWidth));
                }

                arrayCounter++;

                startPoint = -1;
                endPoint = -1;
                strFragment.clear();

                if (i != str.size() - 1)//만약 다음 글자가 연속으로 쉼표면 여백이므로 건너뜀
                {
                    while (str[i + 1] == UNI::COMMA || str[i + 1] == 10)
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


