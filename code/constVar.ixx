#include <SDL.h>

export module constVar;

import std;

export constexpr int DARK_VISION_HALF_W = 18; //��ҽþ�
export constexpr int DARK_VISION_HALF_H = 9; //��ҽþ�

export constexpr int GRAY_VISION_HALF_W = 23; //�ֺ� FOV ȸ������ �ٲٴ� �ִ� W
export constexpr int GRAY_VISION_HALF_H = 23; //�ֺ� FOV ȸ������ �ٲٴ� �ִ� H

export constexpr int CHUNK_SIZE_X = 13; //ûũ�� x����
export constexpr int CHUNK_SIZE_Y = 13; //ûũ�� y����
export constexpr int CHUNK_SIZE_Z = 1; //ûũ�� z����, ���� 1�� �̻���
export constexpr int MAX_FONT_SIZE = 32;
export constexpr int EX_INPUT_TEXT_MAX = 30;
export constexpr int EQUIP_ITEM_MAX = 10;
export constexpr int LOOT_ITEM_MAX = 9;
export constexpr int INVENTORY_ITEM_MAX = 9;
export constexpr int DMG_FLAG_SIZE = 3;
export constexpr int TALENT_SIZE = 19;
export constexpr int MAX_VEHICLE_SIZE = 31;
export constexpr int CHUNK_LOADING_RANGE = 5;
export constexpr int MINIMAP_DIAMETER = 41; //�̴ϸ��� ���� (Ȧ��)
export constexpr int SECTOR_SIZE = 400; // ������ �� ������ �� �ۿ��ϴ� ����
export constexpr int TOLERANCE_LSTICK = 10000; //LStick�� �� ���� �Ѿ�� ������ �Ͼ
export constexpr int TOLERANCE_HOLD_DEL_XY = 20; //�� �� �̻� ������ ��� Ȧ�� �̺�Ʈ�� �Ͼ�� ����

export constexpr int MARKER_LIMIT_DIST = 100; //�÷��̾�κ��� 100ĭ �̻� �������� ��Ŀ�� �׷����� ����

export constexpr std::array<int, 27> expTable =
{ 50, 100, 150, 200, 250, 300, 350, 400, 450,
550, 650, 750, 850, 950, 1050, 1150, 1250, 1350,
1500, 1650, 1800, 1950, 2100, 2250, 2400, 2550, 2750 };

export namespace col
{
    constexpr SDL_Color black = { 0x00, 0x00, 0x00 };
    constexpr SDL_Color yellow = { 0xff,0xff,0x00 };
    constexpr SDL_Color brown = { 0x5c,0x33,0x17 };
    constexpr SDL_Color gray = { 0x63,0x63,0x63 };
    constexpr SDL_Color green = { 0x00,0x6e,0x00 };
    constexpr SDL_Color blueberry = { 0x64,0x64,0xff };
    constexpr SDL_Color red = { 0xf9,0x29,0x29 };
    constexpr SDL_Color white = { 0xff,0xff,0xff };
    constexpr SDL_Color lightGray = { 0x96,0x96,0x96 };
    constexpr SDL_Color blue = { 0x21,0x4a,0xea };
    constexpr SDL_Color yellowGreen = { 0x3a, 0xf5, 0x43 };
    constexpr SDL_Color monaLisa = { 0xff,0x96,0x96 };
    constexpr SDL_Color bondiBlue = { 0x00,0x96,0xb4 };
    constexpr SDL_Color hotPink = { 0x8b,0x3a,0x62 };
    constexpr SDL_Color pink = { 0xfe,0x00,0xfe };
    constexpr SDL_Color skyBlue = { 0x00,0xf0,0xff };
    constexpr SDL_Color blueDart = { 0x4e,0x8e,0xd2 };
    constexpr SDL_Color orange = { 0xf2, 0x65, 0x22 };
    constexpr SDL_Color cyan = { 0x00,0xa3,0xd2 };
};

export namespace lowCol
{
    constexpr SDL_Color black = { 0x00,0x00,0x00 };
    constexpr SDL_Color white = { 0xff,0xff,0xff };
    constexpr SDL_Color red = { 0xd0,0x3f,0x3f };
    constexpr SDL_Color orange = { 0xd0,0x7a,0x3f };
    constexpr SDL_Color yellow = { 0xd0,0xc3,0x3f };
    constexpr SDL_Color green = { 0x75,0xd0,0x3f };
    constexpr SDL_Color mint = { 0x3f,0xd0,0x7f };
    constexpr SDL_Color skyBlue = { 0x3f,0xba,0xd0 };
    constexpr SDL_Color deepBlue = { 0x20,0x50,0xa8 };
    constexpr SDL_Color blue = { 0x2b,0x81,0xe8 };
    constexpr SDL_Color purple = { 0x43,0x3e,0x8e };
    constexpr SDL_Color pink = { 0xbe,0x3f,0xd0 };
    constexpr SDL_Color crimson = { 0xd0,0x3f,0x89 };
};

export enum class act
{
    null,       //�Ϲ� �����÷���
    blank,      //��ĭ
    status,     //����â
    ability,    //Ư���ɷ�
    inventory,  //�κ��丮
    bionic,     //���̿���
    talent,      //���
    runMode,    //�޸��� ���
    menu,       //�޴�

    identify,   //����
    vehicle,    //����
    alchemy,    //���ݼ�
    god, //�ž�
    map, //����

    closeDoor, //���ݱ�


    armor,  //���̵�Ƹ�
    cooking,    //�丮
    loot,       //�ݱ�
    pick,       //�ֱ�(���濡)
    wield,      //���
    equip,      //���
    pickSelect, //�ݱ�(����)
    selectAll,  //����(����) �ʿ����
    searching,  //�˻�
    sorting,    //����
    select,     //����
    eat,        //�Ա�
    apply,      //���
    selectMode, //���� ���
    droping,    //������
    throwing,   //������
    dirSelect,  //���⼱��
    coordSelect,//��ġ����
    mutation,   //��������
    craft,      //����
    construct,  //����
    open,       //���濭��
    test,       //�׽�Ʈ ���
    insert,     //������ �ֱ�(���濡 �ű��)
    reload,     //������ ����
    reloadBulletToMagazine, //źȯ ����
    unloadBulletFromMagazine, //źȯ �и�
    reloadMagazine, //źâ ����, �Ѱ� źâ ���ʿ� ������ ����� �ٸ�
    unloadMagazine, //źâ �и�
    reloadBulletToGun, //źȯ ����
    unloadBulletFromGun, //źȯ �и�

    turnLeft,//��ȸ��
    wait,//1�ϴ��
    turnRight,//��ȸ��
    startEngine,//���Žõ�
    stopEngine,//��������
    shiftGear,//����
    accel,//�׼�
    brake,//�극��ũ
    autoGear,//�ڵ����ӱ�

    collectiveLever,
    cyclicLever,
    rpmLever,
    tailRotorPedalL,
    tailRotorPedalR,

    confirm,//Ȯ��
    rotate,//ȸ��
    cancel,//���

    quickSlot,
    skillActive,
    skillToggle,

    inspect,

    //skillInfo,
};

export namespace humanCustom
{
    enum class skin
    {
        null,
        white,
        yellow,
        brown,
        black
    };

    enum class eyes
    {
        null,
        close,
        red,
        blue,
        black,
    };


    enum class scar
    {
        null,
        darkCircles
    };

    enum class beard
    {
        null,
        mustache
    };

    enum class hair
    {
        null,
        commaBlack,
        bob1Black,
        ponytail,
        middlePart,
    };


};

export namespace axis
{
    constexpr int x = 0;
    constexpr int y = 1;
    constexpr int z = 2;
};


export namespace mapFlag
{
    constexpr int floor = 0; //�ٴ��� ����
    constexpr int wall = 1; //���� ����
    constexpr int base = 2; //
    constexpr int opaque = 3; //�þ߸� ���� ���ع� ����
    constexpr int fov = 4; //���� fov ����
    constexpr int walkable = 5; //���� ���� ����

    // �� ����
    //�Ʒ� RGB���� ������ŭ �Ȱ��� ���� ���� {R,G,B} = {255,0,0}�� ���
    //{255,255,255}-{0,255,255}�ؼ� {255,0,0}, �ִ��� �׻� 255�̴�.
    constexpr int light = 6;
    constexpr int redLight = 7;
    constexpr int greenLight = 8;
    constexpr int blueLight = 9;
};

export enum entityIndex
{
    test = 1,
    player = 2,
    zombie = 3
};

export enum itemIndex
{
    glitchItem = 0,
    orb = 1,
};



export enum class fovFlag
{
    white,
    black,
    gray
};

export enum class weatherFlag
{
    sunny,
    cloudy,
    rain,
    storm,
    snow,
};


export namespace entityFlag
{
    constexpr int name = 0;
    constexpr int index = 1;
    constexpr int nameIndex = 2;
    constexpr int sprIndex = 3;
    constexpr int category = 4;
    constexpr int temperature = 5;
    constexpr int weight = 6;
    constexpr int volume = 7;
    constexpr int HD = 8;
    constexpr int SH = 9;
    constexpr int EV = 10;
    constexpr int rFire = 11;
    constexpr int rCold = 12;
    constexpr int rElec = 13;
    constexpr int rCorr = 14;
    constexpr int rRad = 15;
    constexpr int maxHP = 16;

    constexpr std::array<int, 10> indexPart = { 17, 23, 29, 35, 41, 47, 53, 59, 65, 71 };
    constexpr std::array<int, 10> rPiercePart = { 18, 24, 30, 36, 42, 48, 54, 60, 66, 72 };
    constexpr std::array<int, 10> rCutPart = { 19, 25, 31, 37, 43, 49, 55, 61, 67, 73 };
    constexpr std::array<int, 10> rBashPart = { 20, 26, 32, 38, 44, 50, 56, 62, 68, 74 };
    constexpr std::array<int, 10> encMaxPart = { 21, 27, 33, 39, 45, 51, 57, 63, 69, 75 };
    constexpr std::array<int, 10> accPart = { 22, 28, 34, 40, 46, 52, 58, 64, 70, 76 };

    constexpr int strength = 77;
    constexpr int intelligence = 78;
    constexpr int dexterity = 79;
    constexpr int corpseItemIndex = 80;
    constexpr int downImageIndex = 81;
    constexpr int corpseImageIndex = 82;
    constexpr int portraitImageIndex = 83;
    constexpr int atk = 84;
    constexpr int atkType = 85;
    constexpr int atkSpd = 86;
    constexpr int movSpd = 87;
    constexpr int flySpd = 88;
    constexpr int swimSpd = 89;
    constexpr int unique = 90;
    constexpr int regen = 91;
    constexpr int blood = 92;
    constexpr int sight = 93;
    constexpr int smell = 94;
};

export namespace dmgFlag
{
    constexpr int partIndex = 0;
    constexpr int dmg = 1;
    constexpr int type = 2;
};


/*
* ������ 0���� �����̰� 0���� 0�� �Ǹ� �����
* �������� ���ϸ� 1�� �̻��� �����ε����� �����Ǵµ� �̶� 0������ ���� �������� ��, ��Ÿ �������� ��ҷε� HP�� �����
*/
export namespace partType
{
    constexpr unsigned __int8 null = 0;
    constexpr unsigned __int8 head = 1;
    constexpr unsigned __int8 torso = 2;
    constexpr unsigned __int8 larm = 3;
    constexpr unsigned __int8 rarm = 4;
    constexpr unsigned __int8 lleg = 5;
    constexpr unsigned __int8 rleg = 6;
    constexpr unsigned __int8 tail = 7;

    constexpr unsigned __int8 turret = 8;
    constexpr unsigned __int8 camera = 9;
    constexpr unsigned __int8 body = 10;
    constexpr unsigned __int8 lCaterpillar = 11;
    constexpr unsigned __int8 rCaterpillar = 12;
};

export namespace wound
{
    constexpr int pierce = 0; //�����
    constexpr int cut = 1; //���ܻ�
    constexpr int bash = 2; //Ÿ�ڻ�
    constexpr int fracture = 3; //���� : �ǰݽ� Ÿ�ڻ� ����Ʈ�� ���� ��� �߻�, �θ����θ� ġ�� ���� 20% ���� �ս�
    constexpr int burn = 4; //ȭ�� : ġ�� �Ұ���, �ǻ翡�� ������
    constexpr int frostbite = 5; //���� : ������ ���� ���� ���� ���ݾ� �����
    constexpr int shock = 6; //��ũ : �������� ���, �Ӹ����� �߻���
};

export namespace talentFlag
{
    constexpr int fighting = 0;
    constexpr int dodging = 1;
    constexpr int stealth = 2;
    constexpr int throwing = 3;
    constexpr int unarmedCombat = 4;
    constexpr int piercingWeapon = 5;
    constexpr int cuttingWeapon = 6;
    constexpr int bashingWeapon = 7;
    constexpr int archery = 8;
    constexpr int gun = 9;
    constexpr int electronics = 10;
    constexpr int chemistry = 11;
    constexpr int mechanics = 12;
    constexpr int computer = 13;
    constexpr int medicine = 14;
    constexpr int cooking = 15;
    constexpr int fabrication = 16;
    constexpr int social = 17;
    constexpr int architecture = 18;
};

export enum class msgFlag
{
    deact,
    normal,
    input,
};

export enum class itemFlag
{
    TWOHANDED,
    CANEQUIP,
    NONSTACK,
    GUN,
    MAGAZINE,
    AMMO,
    BOOKMARK1,
    BOOKMARK2,
    BOOKMARK3,
    BOOKMARK4,
    BOOKMARK5,
    BOOKMARK6,
    POWERED,//���°���
    VFRAME,//���������� : ��ġ ����
    RAIDARMOR,//�Ƹ������� : ��ġ ����
    WHITEFILTER,
    GRAYFILTER,//�˻� �� ���� GUI���� �������� ȸ������ ǥ���ϰ� ����
    BLACKFILTER,//���� GUI���� ���ش� �������� ���������� ǥ���ϰ� ����(��ǥ��)
    COORDCRAFT,//��ǥ����, ���๰�̳� ���� ��
    ALCHEMYTOOL,//���ݼ����� ��� ������ ����
    ALCHEMYMATERIAL, //���ݼ��� ��� ������ ���
    LIQUID, //��ü ��
    GAS, // ��ü ��
    VPART, //������ǰ
    TRANSPARENT_WALL, //����

    VPART_WALL_CONNECT,//�������� �ֺ��� ����Ǵ� 16Ÿ��
    VPART_DIR_DEPEND,//���⿡ �����Ͽ� 16������ �����ϴ� ����
    VPART_DOOR_OPEN,
    VPART_DOOR_CLOSE,

    VEH_ROOF,

    LIGHT_ON,
    LIGHT_OFF,

    TIRE_NORMAL,
    TIRE_STEER,

    PROP, //��ġ ������ ������
    PROP_BIG,//48px�� �ƴ϶� 80px*80px�� giantVehicleset ������
    PROP_WALKABLE,//�̵����� ����
    PROP_BLOCKER,//�þ߹��� ����
    PROP_DEPTH_LOWER,//���� ��ġ��(&��ƼƼ)��� �������� �ʰ� �ٴڿ� �򸮴� ��ġ��


    PIPE_CNCT_RIGHT,
    PIPE_CNCT_TOP,
    PIPE_CNCT_LEFT,
    PIPE_CNCT_BOT,

    WIRE_CNCT_RIGHT,
    WIRE_CNCT_TOP,
    WIRE_CNCT_LEFT,
    WIRE_CNCT_BOT,

    CONVEYOR_CNCT_RIGHT,
    CONVEYOR_CNCT_TOP,
    CONVEYOR_CNCT_LEFT,
    CONVEYOR_CNCT_BOT,

    WIRE,
    PIPE,
    CONVEYOR,

    RAIL,

    RAIL_CNCT_TOP,
    RAIL_CNCT_BOT,
    RAIL_CNCT_LEFT,
    RAIL_CNCT_RIGHT,

    //������ȯ�⿡�� ������ ����
    RAIL_INPUT_TOP,
    RAIL_INPUT_BOT,
    RAIL_INPUT_LEFT,
    RAIL_INPUT_RIGHT,

    RAIL_BUFFER,

    NOT_RECIPE, //���չ��� ���� �߰����� ����(ȸ���� ��ġ������ �ߺ� ��ġ����)

    /// ���⼭���� ���� �߰��ؾߵ�
    TREE, //����
    PLANT, //�Ĺ�
    PLANT_SEASON_DEPENDENT, //���������Ĺ�
    PLANT_GROWTH_DEPENDENT, //���������Ĺ�
    MUSHROOM, //����
    FLOOR, //�ٴ�Ÿ��
    WALL, //��
    CEIL, //õ��
    WATER_SHALLOW, //������
    WATER_DEEP, //������
    FRESHWATER, //���
    SEAWATER, //�ؼ�
    TILE_SEASON, //������ ���� ���ϴ� Ÿ��
    DOOR,
    UPSTAIR,
    DOWNSTAIR,
    SIGN,//����ǥ���ǰ��� ������ �����ϴ� ��ü


    DOOR_CLOSE,
    DOOR_OPEN,

    TRAIN_WHEEL, //��������

    PROP_GAS_OBSTACLE_ON,
    PROP_GAS_OBSTACLE_OFF,
    WALL_GAS_PERMEABLE,
};

export namespace toolQuality
{
    constexpr int none = 0;
    constexpr int screwDriving = 1;
    constexpr int drilling = 2;
    constexpr int welding = 3;
    constexpr int soldering = 4;
    constexpr int cutting = 5;
    constexpr int sawing = 6;
    constexpr int hammering = 7;
    constexpr int digging = 8;
    constexpr int sewing = 9;
    constexpr int distillation = 10;
    constexpr int boiling = 11;
    constexpr int frying = 12;
    constexpr int roasting = 13;
    constexpr int boltTurning = 14;
    constexpr int woodCutting = 15;
    constexpr int metalCutting = 16;
    constexpr int metalDrilling = 17;
    constexpr int refrigeration = 18;
    constexpr int heating = 19;
};

export enum class itemAct
{
    pick = 0,
    wield = 1,
    drop = 2,
    pitch = 3,

    wear = 4,
    takeOff = 5,
    repair = 6,
    disasm = 7,

    apply = 8,
    eat = 9,
    read = 10,
    reload = 11,
    unload = 12,
    open = 13,
};


export enum class playerSpriteCategory
{
    oneHanded = 0,
    twoHanded = 2,
    sitOneHanded = 4,
    sitTwoHanded = 6,
    runOneHanded = 8,
    runTwoHanded = 10,
};

export enum class btn
{
    keypad6 = 0,
    keypad9 = 1,
    keypad8 = 2,
    keypad7 = 3,
    keypad4 = 4,
    keypad1 = 5,
    keypad2 = 6,
    keypad3 = 7,
    keypad5 = 8,
    a = 9,
    b = 10,
    x = 11,
    y = 12,
    l1 = 13,
    r1 = 14,
    l2 = 15,
    r2 = 16,
};



export enum class storageType
{
    null,//�ӽÿ�
    equip,//���
    stack,//����
    pocket,//�������� ���� �ָӴ�
    temp,//�ӽÿ�
    recipe,//������(�÷��̾��)
};

export enum class aniFlag
{
    null,
    move,
    atk,
    drop,
    winUnfoldOpen,
    winUnfoldClose,
    winSlipOpen,
    winSlipClose,
    throwing,
    popUpLetterbox, //GUI�� �����ϴ� HUD ���� �ִϸ��̼�
    popDownLetterbox,
    popUpSingleLetterbox,
    shotSingle,
    shotBurst,
    shotAuto,
    shotArrow,
    propRush,
    trainRush,
    quickSlotPopLeft,
    quickSlotPopRight,
    fireStorm,
};

export enum class input
{
    mouse,
    touch,
    keyboard,
    gamepad
};

export enum class msgOption
{
    okay,
    cancel,
    yes,
    no,
    confirm,
    left,
    right
};

export enum class tabFlag
{
    autoAtk,
    closeWin,
    back,
    confirm,
    aim,
};

export enum class sortFlag
{
    null,
    weightDescend,
    weightAscend,
    volumeDescend,
    volumeAscend,
    equip,
};

export namespace equip
{
    constexpr int none = 0;
    constexpr int normal = 1;
    constexpr int left = 2;
    constexpr int right = 3;
    constexpr int both = 4;
};

export enum class turn
{
    playerInput,
    playerAnime,
    VehicleAI,
    VehicleAnime,
    monsterAI,
    monsterAnime,
};

export namespace keyIcon
{
    constexpr int blankRect = 0;

    constexpr int keyboard_1 = 1;
    constexpr int keyboard_2 = 2;
    constexpr int keyboard_3 = 3;
    constexpr int keyboard_4 = 4;
    constexpr int keyboard_5 = 5;
    constexpr int keyboard_6 = 6;
    constexpr int keyboard_7 = 7;
    constexpr int keyboard_8 = 8;
    constexpr int keyboard_9 = 9;
    constexpr int keyboard_0 = 10;

    constexpr int keyboard_Num1 = 11;
    constexpr int keyboard_Num2 = 12;
    constexpr int keyboard_Num3 = 13;
    constexpr int keyboard_Num4 = 14;
    constexpr int keyboard_Num5 = 15;
    constexpr int keyboard_Num6 = 16;
    constexpr int keyboard_Num7 = 17;
    constexpr int keyboard_Num8 = 18;
    constexpr int keyboard_Num9 = 19;
    constexpr int keyboard_Num0 = 20;

    constexpr int keyboard_F1 = 21;
    constexpr int keyboard_F2 = 22;
    constexpr int keyboard_F3 = 23;
    constexpr int keyboard_F4 = 24;
    constexpr int keyboard_F5 = 25;
    constexpr int keyboard_F6 = 26;
    constexpr int keyboard_F7 = 27;
    constexpr int keyboard_F8 = 28;
    constexpr int keyboard_F9 = 29;
    constexpr int keyboard_F10 = 30;
    constexpr int keyboard_F11 = 31;
    constexpr int keyboard_F12 = 32;

    constexpr int keyboard_H = 33;
    constexpr int keyboard_J = 34;
    constexpr int keyboard_K = 35;
    constexpr int keyboard_L = 36;
    constexpr int keyboard_Y = 37;
    constexpr int keyboard_U = 38;
    constexpr int keyboard_B = 39;
    constexpr int keyboard_N = 40;
    constexpr int keyboard_A = 41;
    constexpr int keyboard_S = 42;
    constexpr int keyboard_Z = 43;
    constexpr int keyboard_X = 44;

    constexpr int keyboard_Tab = 45;
    constexpr int keyboard_Enter = 46;
    constexpr int keyboard_PgUp = 47;
    constexpr int keyboard_PgDn = 48;

    constexpr int pad_X = 49;
    constexpr int pad_Y = 50;
    constexpr int pad_A = 51;
    constexpr int pad_B = 52;
    constexpr int pad_L = 53;
    constexpr int pad_R = 54;
    constexpr int pad_LStick = 55;
    constexpr int pad_ZL = 56;
    constexpr int pad_ZR = 57;

    constexpr int pad_Right = 58;
    constexpr int pad_Up = 59;
    constexpr int pad_Left = 60;
    constexpr int pad_Down = 61;

    constexpr int keyboard_C = 62;
    constexpr int keyboard_V = 63;

    constexpr int keyboard_LShift = 64;
    constexpr int keyboard_RShift = 65;

    constexpr int duelSense_L1 = 66;
    constexpr int duelSense_L2 = 67;
    constexpr int duelSense_R1 = 68;
    constexpr int duelSense_R2 = 69;

    constexpr int duelSense_X = 70;
    constexpr int duelSense_CIR = 71;
    constexpr int duelSense_RECT = 72;
    constexpr int duelSense_TRI = 73;
    constexpr int duelSense_LStick = 74;
    constexpr int duelSense_RStick = 75;
  
    constexpr int duelSense_RIGHT = 77;
    constexpr int duelSense_TOP = 78;
    constexpr int duelSense_LEFT = 79;
    constexpr int duelSense_BOT = 80;
    constexpr int duelSense_OPTIONS = 81;
    constexpr int duelSense_SHARE = 82;

    ////////

    constexpr int joyCon_L = 88;
    constexpr int joyCon_ZL = 89;
    constexpr int joyCon_R = 90;
    constexpr int joyCon_ZR = 91;

    constexpr int joyCon_A = 92;
    constexpr int joyCon_B = 93;
    constexpr int joyCon_X = 94;
    constexpr int joyCon_Y = 95;
    constexpr int joyCon_LStick = 96;
    constexpr int joyCon_RStick = 97;

    constexpr int joyCon_RIGHT = 99;
    constexpr int joyCon_TOP = 100;
    constexpr int joyCon_LEFT = 101;
    constexpr int joyCon_BOT = 102;
    constexpr int joyCon_PLUS = 103;
    constexpr int joycon_MINUS = 104;
};

export namespace sprInf
{
    constexpr int walk = 0;
    constexpr int run = 6;
    constexpr int sit = 12;
    constexpr int crawl = 18;
};

export enum class itemCategory
{
    weapon,
    equipment,
    tool,
    consumable,
    bionic,
    vehicle,
    structure,
    material,
};

export enum class itemSubcategory
{
    weapon_piercing,
    weapon_cutting,
    weapon_bashing,
    weapon_shooting,
    weapon_throwing,

    equipment_clothing,
    equipment_hat,
    equipment_gloves,
    equipment_shoes,
    equipment_accessory,

    tool_hand,
    tool_power,
    tool_container,
    tool_device,
    tool_document,
    tool_etc,

    consumable_food,
    consumable_medicine,
    consumable_ammo,
    consumable_fuel,
    consumable_etc,

    vehicle_frame,//������
    vehicle_engine,//���� : ���ָ�, ����, �������
    vehicle_exterior,//���� : ����, ����, Ʈ��ũ��
    vehicle_transport,//���� : Ʈ��ũ, ����
    vehicle_energy,//������ : ���͸�, ������
    vehicle_device,//��ġ : �����, ������ġ ��

    bionic_core,//�ھ�
    bionic_active,//�ߵ���
    bionic_passive,//���Ӱ�
    bionic_toggle,//��ȯ��
    bionic_generator,//�����
    bionic_storage,//�����

    structure_wall,
    structure_floor,
    structure_ceil,
    structure_prop,
    structure_electric,
    structure_pneumatic,

    material_chemical,
    material_biological,
    material_mechanical,
    material_electrical,
    material_etc,
};

export enum class godFlag
{
    none,
    teshub,
    buddha,
    jesus,
    amaterasu,
    yudi,
    ra,
};

export namespace entityCategory
{
    constexpr int none = 0;
    constexpr int human = 1;
    constexpr int zombie = 2;
    constexpr int robot = 3;
};

export namespace bodyTemplateFlag
{
    constexpr int none = 0;
    //��� ������ �ı��Ǹ� ���
    constexpr int human = 1;
    //�Ӹ� ���� ���� ������ �޴ٸ� �����ٸ�
    //�Ӹ��� ���� ������ ���, �� ������ ���ݷ� 30% ����, �ٸ� ������ �̼� 30% ����
    constexpr int zombie = 2;
    //�Ӹ� ���� ���� ������ �޴ٸ� �����ٸ�
    //���� ������ ���, �� ������ ���ݷ� 30% ����, �ٸ� ������ �̼� 30% ����, �Ӹ� ������ �þ� 0
    constexpr int tank = 3;
    //��ž ��ü �¹��ѱ˵� �칫�ѱ˵�
    //��ü ������ ���, ��ž ������ ��ݺҰ�, �˵� ������ ���� �̵��ӵ� 30% ����
};

export namespace partsFlag
{
    constexpr int index = 0;
    constexpr int hp = 1;
    constexpr int acc = 2;
    constexpr int maxHP = 3;
};

export namespace bulletFlag
{
    constexpr int normal = 0;
    constexpr int tracer = 1;
    constexpr int ap = 2;
};

export namespace weaponMode
{
    constexpr int none = 0;
    constexpr int safeMode = 1;
    constexpr int semiAutoMode = 2;
    constexpr int burstMode = 3;
    constexpr int autoMode = 4;
};

//���� �������� ����
export namespace dmgType
{
    constexpr int none = 0; //���Ӽ� ����
    constexpr int pierce = 1;//����(����)
    constexpr int cut = 2;//����(����)
    constexpr int bash = 3;//Ÿ��(����)
};

//�÷��̾ ���� �� �ִ� ������ Ÿ��, Aim�� 5���� ����(����, ����, Ÿ��, ���, ��ô)
export enum class atkType
{
    pierce,
    cut,
    bash,
    shot,
    throwing,
};

export namespace UNI
{
    constexpr int NUL = 0;   // Null char
    constexpr int SOH = 1;   // Start of Heading
    constexpr int STX = 2;   // Start of Text
    constexpr int ETX = 3;   // End of Text
    constexpr int EOT = 4;   // End of Transmission
    constexpr int ENQ = 5;   // Enquiry
    constexpr int ACK = 6;   // Acknowledgment
    constexpr int BEL = 7;   // Bell
    constexpr int BACKSPACE = 8;    // Back Space
    constexpr int TAB = 9;   // Horizontal Tab
    constexpr int LF = 10;   // Line Feed
    constexpr int VT = 11;   // Vertical Tab
    constexpr int FF = 12;   // Form Feed
    constexpr int CR = 13;   // Carriage Return
    constexpr int SO = 14;   // Shift Out / X-On
    constexpr int SI = 15;   // Shift In / X-Off
    constexpr int DLE = 16;  // Data Line Escape
    constexpr int DC1 = 17;  // Device Control 1 (oft. XON)
    constexpr int DC2 = 18;  // Device Control 2
    constexpr int DC3 = 19;  // Device Control 3 (oft. XOFF)
    constexpr int DC4 = 20;  // Device Control 4
    constexpr int NAK = 21;  // Negative Acknowledgement
    constexpr int SYN = 22;  // Synchronous Idle
    constexpr int ETB = 23;  // End of Transmit Block
    constexpr int CAN = 24;  // Cancel
    constexpr int EM = 25;   // End of Medium
    constexpr int SUB = 26;  // Substitute
    constexpr int ESC = 27;  // Escape
    constexpr int FS = 28;   // File Separator
    constexpr int GS = 29;   // Group Separator
    constexpr int RS = 30;   // Record Separator
    constexpr int US = 31;   // Unit Separator
    constexpr int SPACE = 32;
    constexpr int EXCLAMATION_MARK = 33;  // '!'
    constexpr int DOUBLE_QUOTES = 34;  // '"'
    constexpr int HASH = 35;  // '#'
    constexpr int DOLLAR_SIGN = 36;  // '$'
    constexpr int PERCENT_SIGN = 37;  // '%'
    constexpr int AMPERSAND = 38;  // '&'
    constexpr int APOSTROPHE = 39;  // '''
    constexpr int LEFT_PARENTHESIS = 40;  // '('
    constexpr int RIGHT_PARENTHESIS = 41;  // ')'
    constexpr int ASTERISK = 42;  // '*'
    constexpr int PLUS_SIGN = 43;  // '+'
    constexpr int COMMA = 44;  // ','
    constexpr int MINUS_SIGN = 45;  // '-'
    constexpr int PERIOD = 46;  // '.'
    constexpr int SLASH = 47;  // '/'
    constexpr int ZERO = 48;
    constexpr int ONE = 49;
    constexpr int TWO = 50;
    constexpr int THREE = 51;
    constexpr int FOUR = 52;
    constexpr int FIVE = 53;
    constexpr int SIX = 54;
    constexpr int SEVEN = 55;
    constexpr int EIGHT = 56;
    constexpr int NINE = 57;
    constexpr int COLON = 58;  // ':'
    constexpr int SEMICOLON = 59;  // ';'
    constexpr int LESS_THAN_SIGN = 60;  // '<'
    constexpr int EQUAL_SIGN = 61;  // '='
    constexpr int GREATER_THAN_SIGN = 62;  // '>'
    constexpr int QUESTION_MARK = 63;  // '?'
    constexpr int AT_SIGN = 64;  // '@'
    constexpr int A = 65;
    constexpr int B = 66;
    constexpr int C = 67;
    constexpr int D = 68;
    constexpr int E = 69;
    constexpr int F = 70;
    constexpr int G = 71;
    constexpr int H = 72;
    constexpr int I = 73;
    constexpr int J = 74;
    constexpr int K = 75;
    constexpr int L = 76;
    constexpr int M = 77;
    constexpr int N = 78;
    constexpr int O = 79;
    constexpr int P = 80;
    constexpr int Q = 81;
    constexpr int R = 82;
    constexpr int S = 83;
    constexpr int T = 84;
    constexpr int U = 85;
    constexpr int V = 86;
    constexpr int W = 87;
    constexpr int X = 88;
    constexpr int Y = 89;
    constexpr int Z = 90;
    constexpr int LEFT_SQUARE_BRACKET = 91;  // '['
    constexpr int BACKSLASH = 92;  // '\'
    constexpr int RIGHT_SQUARE_BRACKET = 93;  // ']'
    constexpr int CARET = 94;  // '^'
    constexpr int UNDERSCORE = 95;  // '_'
    constexpr int GRAVE_ACCENT = 96;  // '`'
    constexpr int a = 97;
    constexpr int b = 98;
    constexpr int c = 99;
    constexpr int d = 100;
    constexpr int e = 101;
    constexpr int f = 102;
    constexpr int g = 103;
    constexpr int h = 104;
    constexpr int i = 105;
    constexpr int j = 106;
    constexpr int k = 107;
    constexpr int l = 108;
    constexpr int m = 109;
    constexpr int n = 110;
    constexpr int o = 111;
    constexpr int p = 112;
    constexpr int q = 113;
    constexpr int r = 114;
    constexpr int s = 115;
    constexpr int t = 116;
    constexpr int u = 117;
    constexpr int v = 118;
    constexpr int w = 119;
    constexpr int x = 120;
    constexpr int y = 121;
    constexpr int z = 122;
    constexpr int LEFT_CURLY_BRACKET = 123;  // '{'
    constexpr int VERTICAL_BAR = 124;  // '|'
    constexpr int RIGHT_CURLY_BRACKET = 125;  // '}'
    constexpr int TILDE = 126;  // '~'
    constexpr int DEL = 127;   // Delete
};


export enum class gearFlag
{
    park,
    reverse,
    neutral,
    drive,
};

export enum class chunkFlag
{
    none,
    freshwater,//���
    seawater,//�ؼ�
    meadow,//�ʿ�
    underground,//����
    //����
    //����
};

export namespace chunkCol
{
    SDL_Color seawater = { 22, 33, 255 };
};

export namespace tileFloorFlag
{
    int none = 0;
    int seawater = 220;
    int grass = 221;
};


export enum class seasonFlag
{
    spring,
    summer,
    autumn,
    winter,
};

export enum class vehFlag
{
    none,
    car,
    heli,
    train,
};


export namespace itemVIPCode
{
    int railRL = 320;
    int railTB = 321;
    int railBR = 322;
    int railTR = 323;
    int railTL = 324;
    int railBL = 325;

    int railSwitchEN = 326;
    int railSwitchES = 327;
    int railSwitchNW = 328;
    int railSwitchNE = 329;
    int railSwitchWN = 330;
    int railSwitchWS = 331;
    int railSwitchSW = 332;
    int railSwitchSE = 333;
};

export enum class skillSrc
{
    NONE,
    BIONIC,
    MUTATION,
    MARTIAL_ART,
    DIVINE_POWER,
    MAGIC,
};

export enum class skillType
{
    ACTIVE,
    PASSIVE,
    TOGGLE,
};

export enum class quickSlotFlag
{
    NONE,
    SKILL,
    ITEM,
};

export enum class CoordSelectFlag
{
    NONE,
    SINGLE_TARGET_SKILL,
    FIRESTORM,
};

export enum class flameFlag
{
    NONE,
    SMALL,
    NORMAL,
    BIG,
};

export enum class gasFlag
{
    NONE,
    SMALL,
    NORMAL,
    BIG,
};