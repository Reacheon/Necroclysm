export module constVar:enums;

export enum class act
{
    null,       //일반 게임플레이
    blank,      //빈칸
    status,     //상태창
    ability,    //특수능력
    equipment,  //인벤토리
    bionic,     //바이오닉
    profic,      //재능
    runMode,    //달리기 모드
    skill,      //스킬
    quest,      //퀘스트
    menu,       //메뉴

    identify,   //감정
    vehicle,    //차량
    god, //신앙
    map, //지도

    closeDoor, //문닫기


    armor,  //레이드아머
    cooking,    //요리
    loot,       //줍기
    pick,       //넣기(가방에)
    wield,      //들기
    equip,      //장비
    pickSelect, //줍기(선택)
    selectAll,  //선택(전부) 필요없음
    searching,  //검색
    sorting,    //정렬
    select,     //선택
    eat,        //먹기
    drink,     //마시기
    apply,      //사용
    selectMode, //선택 모드
    droping,    //버리기
    throwing,   //던지기
    dirSelect,  //방향선택
    coordSelect,//위치선택
    mutation,   //돌연변이
    craft,      //조합
    construct,  //건축
    open,       //가방열기
    test,       //테스트 기능
    reloadBulletToMagazine, //탄환 장전
    unloadBulletFromMagazine, //탄환 분리
    reloadMagazine, //탄창 장전, 총과 탄창 양쪽에 있으며 기능이 다름
    unloadMagazine, //탄창 분리
    reloadBulletToGun, //탄환 장전
    unloadBulletFromGun, //탄환 분리

    turnLeft,//좌회전
    wait,//1턴대기
    turnRight,//우회전
    startEngine,//엔신시동
    stopEngine,//엔진끄기
    shiftGear,//기어변경
    accel,//액셀
    brake,//브레이크
    autoGear,//자동변속기

    collectiveLever,
    cyclicLever,
    rpmLever,
    tailRotorPedalL,
    tailRotorPedalR,

    confirm,//확인
    rotate,//회전
    cancel,//취소

    quickSlot,
    skillActive,
    skillToggle,

    inspect,
    unbox,
    pull,

    climb,
    swim,
    ride,

    phone,
    message,
    camera,
    internet,

    settings,
    saveAndQuit,

    toggleOn,
    toggleOff,

    vehicleRepair,
    vehicleDetach,

    headlight,

    drawLiquid,

    sleep,

    drinkFloorWater, //바닥물 마시기

    dump, //쏟기 (포켓 내부에 있는 아이템을 바닥에 쏟음)

    propCarry, //프롭 들기
    propInstall, //프롭 설치

    propTurnOn, //프롭 켜기
    propTurnOff, //프롭 끄기

    connectPlusZ, //위층의 타일과 연결
    connectMinusZ, //아래층의 타일과 연결

    toggleCrossCable,

    insertBattery, //전자기기 배터리 장착
    removeBattery, //전자기기 배터리 탈착

    hideWire,
    showWire,

    plant, //씨앗을 심는 행동(벼나 밀, 감자같은 것들)

    extractSeed, //과일에서 씨앗을 여러개 추출함(원래 과일은 사라짐)

    pray, //기도(제단 상호작용)

    dye, //염색(염색 앰플 사용)

    closeWindow, //창문 닫기
    closeCurtain, //커튼 닫기
    tearCurtain, //커튼 뜯기
    breakWindow, //창문 깨트리기
};

export namespace humanCustom
{
    //피부/눈 색상은 wstring(EntityData.skinColor / eyeColor)으로 저장. 각각 palette/skin.tsv,
    //palette/eyes.tsv 헤더와 매칭. 빈 문자열이면 해당 부위 없음 (해골/특수 종 대응).
    //성별(EntityData.gender)도 wstring으로 저장하며 image/charset/body/skin/SKIN_<gender>.png stem과 매칭.
    //enum을 쓰지 않는 이유는 헤어와 동일: 팔레트 컬럼 / PNG 한 장 추가만으로 확장이 끝나도록 하기 위함.

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

    //머리카락은 wstring(EntityData.hairStyle)으로 저장. image/charset/body/hair/*.png의 stem과 매칭.
    //빈 문자열이면 헤어 없음. enum을 쓰지 않는 이유: 헤어 추가를 PNG 한 장 드롭으로 끝내기 위함.

    enum class horn
    {
        null,
        coverRed,
    };

    //돌연변이 털/뿔 색상은 wstring으로 저장 (palette/fur.tsv, palette/horn.tsv의 색상명과 매칭).
    //enum을 쓰지 않는 이유: 색상 추가를 TSV 한 파일 수정으로 끝내기 위함.
};

//돌연변이 외형이 그려지는 레이어. SkillBehavior.mutLayer로 사용됨.
export enum class mutDrawLayer
{
    none,       //외형 없는 돌연변이 (적외선시각 등)
    underEyes,  //skin 위, eyes 아래 (전신 털, 꼬리)
    aboveEquip, //모든 장비 위 (주둥이, 귀, 뿔)
};

//돌연변이 스프라이트 색상 소스. resolveMutSprite에서 접미사 결정에 사용.
export enum class mutColorSource
{
    none,  //접미사 안 붙임 (예: MUT_RAT_TAIL.png)
    fur,   //entityInfo.furColor 사용
    horn,  //entityInfo.hornColor 사용
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

export enum class msgFlag
{
    deact,
    normal,
    input,
};

export enum class walkFlag
{
    walk,
    run,
    crouch,
    crawl,
    wade,
    swim,
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
    null,//임시용
    equip,//장비
    stack,//스택
    pocket,//아이템의 내부 주머니
    temp,//임시용
    recipe,//레시피(플레이어용)
};

export enum class aniFlag
{
    null,
    move,
    atk,
    felling,
    miningWall,
    diggingWall,
    drop,
    winUnfoldOpen,
    winUnfoldClose,
    winSlipOpen,
    winSlipClose,
    throwing,
    popUpLetterbox, //GUI에 존재하는 HUD 전용 애니메이션
    popDownLetterbox,
    popUpSingleLetterbox,

    shotSingle,

    shotBurst,
    shotAuto,
    shotArrow,
    propRush,
    minecartRush,
    quickSlotPopLeft,
    quickSlotPopRight,

    fireStorm,
    roll,
    leap,

    treeFalling,

    entityThrow,

    faint,
    dropInventory,

    propTurnOnOff, //프롭 켜기/끄기
    changePropDelay, //딜레이 조정

    tilling, //밭갈기(이미 갈아져 있으면 평탄화)
    watering, //밭 물주기
    harvesting, //작물 수확
};

export enum class input
{
    mouse,
    touch,
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
    attackNearby,
    closeWin,
    back,
    confirm,
    aim,
    till,
    water,

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

export enum class equipHandFlag
{
    none,
    normal,
    left,
    right,
    both,
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

export enum class itemCategory
{
    equipment,
    foods,
    tools,
    tech,
    consumables,
    vehicles,
    structures,
    materials,
};

export enum class itemSubcategory
{
    equipment_melee,
    equipment_ranged,
    equipment_firearms,
    equipment_throwing,
    equipment_clothing,

    foods_cooked,       // 요리 (김치찌개, 스테이크, 스파게티, 케밥 등)
    foods_processed,    // 가공식품 (편의점 샌드위치, 빵 등)
    foods_preserved,    // 보존식품 (갈치통조림, 건빵 등)
    foods_drinks,       // 음료 (물, 콜라)
    foods_ingredients,  // 재료 (달걀, 식용유, 소금 등)

    consumable_medicine,
    consumable_ammo,
    consumable_fuel,
    consumable_etc,

    tools_hand,          // 수공구 (망치, 톱 등)
    tools_power,         // 전동공구 (전동드릴, 그라인더 등)
    tools_containers,    // 컨테이너 (배낭, 비커 등)
    tools_etc,          // 기타 도구 (나침반, 지도 등)

    tech_bionics,        // 바이오닉
    tech_powerArmor,    // 파워아머

    vehicle_frames,//프레임
    vehicle_power,//파워 : 가솔린, 디젤, 전기모터
    vehicle_exteriors,//외장 : 바퀴, 차문, 트렁크문
    vehicle_parts,//부품 : 기타

    structure_walls,
    structure_floors,
    structure_props,

    material_metals,       // 금속류 (철, 납, 아연 등)
    material_organic,      // 유기물 (가죽, 나무, 시체 등)
    material_components,   // 부품류 (파이프, 전선, 회로 등)
    material_chemicals,    // 화학물질 (산, 연료, 화약 등)
    material_etc,
};

export enum class godFlag
{
    none,
    rehylion,
};

//신이 감시하는 행동 유형
export enum class conductType
{
    KILL_NEUTRAL,       //중립 몬스터 살해
    KILL_HOLY,          //신성한 존재 살해
    KILL_EVIL,          //사악한 존재 살해
    USE_NECROMANCY,     //네크로맨시 사용
    USE_EVIL_ITEM,      //사악한 아이템 사용
    ATTACK_ALLY,        //아군 공격
    SELF_MUTATE,        //자발적 변이
};

//입은 데미지의 종류
export enum class dmgFlag
{
    none,
    pierce,
    cut,
    bash,
    fire,
    ice,
    elec,
    corr,
    rad
};

export enum class humanPartFlag
{
    head,
    torso,
    lArm,
    rArm,
    lLeg,
    rLeg,
};

//플레이어가 취할 수 있는 공격의 타입, Aim의 5가지 종류(관통, 절단, 타격, 사격, 투척)
export enum class atkType
{
    pierce,
    cut,
    bash,
    shot,
    throwing,
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
    freshwater,//담수
    seawater,//해수
    meadow,//초원
    underground,//지하
    dirt,//흙
    city,//도시 영역 (지면 = 흙)
    bridge,//다리 (지면 = 흙)
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
    minecart,
    train,
    ship,
};

export enum class fluidType
{
    NONE,
    WATER,          // 물
    SEAWATER,       // 바닷물
    POLLUTED_WATER, // 오염수
    OIL,            // 석유
    LAVA,           // 용암
    HOT_SPRING,     // 온천수
};

export enum class skillSrc
{
    GENERAL,
    BIONIC,
    MUTATION,
    MAGIC,
    GOD,
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

//image/UI/GUI/colorPaletteOption.png (16x16 타일)의 sprIndex 매핑.
//염색앰플 등 "색상만 선택하는 UI"에서 LstExOption.sprIndex로 사용.
export enum colorPaletteSprIndex
{
    COLOR_EMPTY     = 0,
    COLOR_GRAY      = 1,
    COLOR_WHITE     = 2,
    COLOR_BLACK     = 3,
    COLOR_ASH_GRAY  = 4,
    COLOR_CREAM     = 5,
    COLOR_ORANGE    = 6,
    COLOR_BROWN     = 7,
    COLOR_KHAKI     = 8,
    COLOR_RED       = 9,
    COLOR_SKY       = 10,
    COLOR_PURPLE    = 11,
    COLOR_GREEN     = 12,
    COLOR_AMBER     = 13,
};

export enum charSprIndex
{
    WALK = 0,
    WALK_2H = 3,

    RUN = 6,
    RUN_2H = 9,

    CROUCH = 12,
    CROUCH_2H = 15,

    CRAWL = 18,

    SIT = 21,
    HOVER = 22,
    SUPINE = 23,

    DASH = 24,

    LATK1 = 28,
    LATK2 = 29,
    RATK1 = 30,
    RATK2 = 31,
    AIM_RIFLE = 32,
    AIM_RIFLE_CROUCH = 33,
    LCAST = 34,
    LCAST_CROUCH = 35,

    RCAST = 36,
    RCAST_CROUCH = 37,
    BCAST = 38,
    BCAST_CROUCH = 39,
    MINING1 = 40,
    MINING2 = 41,

    LAND = 42,
    AIM_PISTOL = 43,
    AIM_PISTOL_CROUCH = 44,
    AIM_PISTOL_CRAWL = 45,

    CRAFT1 = 46,
    CRAFT2 = 47,
};

export enum class dmgAniFlag
{
    none,
    dodged,
    blocked,
};

export enum class relationFlag
{
    neutral,
    hostile,
    friendly,
};

export enum class ridingFlag
{
    none,
    horse,
    wyvern,
    dolphin,
};

export enum class particleFlag
{
    parabolic,
    leaf,
};

export enum class creatureType
{
    human,
    animal,
    undead,
};

//월드맵(Map.ixx) 청크 심볼 식별자. 건물 Lot 1종 = 1값 (mapSymbolOf로 Lot→심볼 매핑).
//  실제 스프라이트(아틀라스·인덱스·footprint 오프셋·변형)는 Map.ixx resolveSymbol이 결정 —
//  여기는 "무엇인가"만 들고, "어떻게 그릴지"는 렌더러가 안다. mountain은 Lot이 아니라
//  worldGrid::Terrain::Mountain에서 직접 파생(렌더러 전용)이지만 enum 일관성 위해 포함.
export enum class MapSymbol
{
    none,
    //1x1 (mapset1by1)
    apartment, bank, house, warehouse, cafe, cinema, junkShop, animalHospital,
    pharmacy, restaurant, stationeryStore, hardwareStore, bookstore,
    patrolStation, convenienceStore, bicycleShop, temple, church, cathedral,
    skyscraper, gasStation, shoppingArcade,
    postOffice, autoShop, clothingStore, jewelryStore, laundromat, gardenShop,
    //2x1 / 1x2 (mapset2by2 — footprint 방향에 따라 wide/tall 스프라이트 분기)
    policeStation, fireStation, hotel, hospital, library,
    //2x2 (mapset2by2)
    park, hypermarket, school, parkingLot,
    //terrain 파생 (Lot 아님 — 렌더러가 Mountain 청크에 직접 부여)
    mountain,
};