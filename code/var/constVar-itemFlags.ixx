export module constVar:itemFlags;

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
    POWERED,//동력공구
    VFRAME,//차량프레임 : 설치 가능
    RAIDARMOR,//아머프레임 : 설치 가능
    WHITEFILTER,
    GRAYFILTER,//검색 및 조합 GUI에서 아이템을 회색으로 표시하게 만듬
    BLACKFILTER,//조합 GUI에서 미해당 아이템을 검은색으로 표시하게 만듬(미표시)
    COORDCRAFT,//좌표조합, 건축물이나 차량 등
    LIQUID, //액체 상
    GAS, // 기체 상
    VPART, //차량부품
    TRANSPARENT_WALL, //투명벽

    VPART_WALL_CONNECT,//차벽같이 주변에 연결되는 16타일
    VPART_DIR_DEPEND,//방향에 의존하여 16방향이 존재하는 프롭
    VPART_DOOR_OPEN,
    VPART_DOOR_CLOSE,

    VEH_ROOF,

    LIGHT_ON,
    LIGHT_OFF,

    TIRE_NORMAL,
    TIRE_STEER,

    PROP, //설치 가능한 아이템
    PROP_BIG,//48px가 아니라 80px*80px의 giantVehicleset 참조함
    PROP_WALKABLE,//이동가능 프롭
    PROP_BLOCKER,//시야방해 프롭
    PROP_DEPTH_LOWER,//기존 설치물(&엔티티)들과 겹쳐지지 않고 바닥에 깔리는 설치물

    CABLE_CNCT_RIGHT,
    CABLE_CNCT_UP,
    CABLE_CNCT_LEFT,
    CABLE_CNCT_DOWN,
    CABLE_Z_ASCEND, //위층의 현재 타일과 연결된 케이블
    CABLE_Z_DESCEND, //아래층의 현재 타일과 연결된 케이블

    PIPE_CNCT_RIGHT,
    PIPE_CNCT_UP,
    PIPE_CNCT_LEFT,
    PIPE_CNCT_DOWN,
    PIPE_CNCT_ABOVE, //위층의 현재 타일과 연결된 파이프
    PIPE_CNCT_BELOW, //아래층의 현재 타일과 연결된 파이프

    FLUID_CIRCUIT,

    CONVEYOR_CNCT_RIGHT,
    CONVEYOR_CNCT_UP,
    CONVEYOR_CNCT_LEFT,
    CONVEYOR_CNCT_DOWN,

    CABLE,
    PIPE,
    CONVEYOR,

    RAIL,
    RAIL_CNCT_UP,
    RAIL_CNCT_DOWN,
    RAIL_CNCT_LEFT,
    RAIL_CNCT_RIGHT,

    //선로전환기에서 고정된 레일
    RAIL_INPUT_UP,
    RAIL_INPUT_DOWN,
    RAIL_INPUT_LEFT,
    RAIL_INPUT_RIGHT,

    RAIL_BUFFER,

    NOT_RECIPE, //조합법에 따로 추가되지 않음(회전된 설치물같은 중복 설치물들)

    /// 여기서부터 새로 추가해야됨
    TREE, //나무
    PLANT, //식물
    PLANT_SEASON_DEPENDENT, //계절의존식물
    PLANT_GROWTH_DEPENDENT, //성장의존식물
    MUSHROOM, //버섯
    FLOOR, //바닥타일
    WALL, //벽
    CEIL, //천장
    WATER_SHALLOW, //얕은물
    WATER_DEEP, //깊은물
    FRESHWATER, //담수
    SEAWATER, //해수
    TILE_SEASON, //계절에 따라 변하는 타일
    DOOR,
    UPSTAIR,
    DOWNSTAIR,
    SIGN,//나무표지판같이 문구를 저장하는 객체


    DOOR_CLOSE,
    DOOR_OPEN,

    TRAIN_WHEEL, //열차바퀴

    PROP_GAS_OBSTACLE_ON,
    PROP_GAS_OBSTACLE_OFF,
    WALL_GAS_PERMEABLE,

    POCKET,
    CAN_CLIMB,

    SPR_TH_WEAPON,
    NO_HAIR_HELMET,//머리카락이 안보이는 헬멧

    BOW,
    CROSSBOW,

    STUMP,

    TOGGLE_ON,
    TOGGLE_OFF,
    HAS_TOGGLE_SPRITE,

    CANCRAFT,//조합할 수 있는 아이템인지

    HEADLIGHT,
    SHIELD,
    CONTAINER_LIQ,

    VPART_NOT_WALKABLE,

    ENGINE_GASOLINE,
    ENGINE_DIESEL,
    ENGINE_ELECTRIC,

    CAN_EAT,
    CAN_DRINK,

    CONTAINER_FLEX, //내부에 들어있는 아이템에 따라 부피가 더해짐

    WIELD_NORMAL_DISPLAY,//이게 있으면 손에 들면 보임(무기같은 이미 이미지가 있는 장비 제외)

    LIQ_COL_RED,
    LIQ_COL_BLUE,
    LIQ_COL_YELLOW,
    LIQ_COL_WHITE,
    LIQ_COL_GRAY,
    LIQ_COL_BLACK,

    CONTAINER_TRANSPARENT,//투명 용기
    CONTAINER_TRANSLUCENT,//반투명 용기

    PROP_POWER_OFF,//가솔린 발전기나 무선 장치 등등
    PROP_POWER_ON,
    PROP_NEXT_TURN_POWER_OFF, //택트스위치 다음 턴 종료 플래그

    CIRCUIT, //전자회로 관련 부품

    CABLE_BEHIND, //케이블이 프롭 뒤에 그려짐

    VOLTAGE_SOURCE, //전력 공급원(발전기 등)
    VOLTAGE_OUTPUT_RIGHT,
    VOLTAGE_OUTPUT_UP,
    VOLTAGE_OUTPUT_LEFT,
    VOLTAGE_OUTPUT_DOWN,

    FORCE_LOAD, //BFS에 포함되기만 해도 loadSet에 포함되는 플래그(2개 이상의 경로 가짐)

    CROSSED_CABLE,

    POWERED_BY_BATTERY, //광부헬멧같이 내부에 장착된 배터리로 동작하는 전자기기들

    HIDE_WIRE, //전자회로의 전선을 보이지 않게 하는 플래그

    CAN_PLANT, //씨앗같이 밭에 심을 수 있는 아이템들

    CROP, //매턴 성장이 발생하는 농작물 종류들
    
    SEED_FRUIT, //씨앗을 얻을 수 있는 과일
};
