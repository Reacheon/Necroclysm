export module constVar:itemIDs;

export namespace itemID
{
    //sentinel — "미설정" / "이 레이어 안 건드림". LotBuilder 등에서 vector 기본값(0)과 의미 일치.
    constexpr int none = 0;

    constexpr int metalFrame = 48;
    constexpr int dirt = 109;
    constexpr int grass = 220;
    constexpr int blackAsphalt = 296;
    constexpr int yellowAsphalt = 377;
    constexpr int whiteAsphalt = 562;

    constexpr int railRL = 320;
    constexpr int railTB = 321;
    constexpr int railBR = 322;
    constexpr int railTR = 323;
    constexpr int railTL = 324;
    constexpr int railBL = 325;

    constexpr int water = 71;


    constexpr int railSwitchEN = 326;
    constexpr int railSwitchES = 327;
    constexpr int railSwitchNW = 328;
    constexpr int railSwitchNE = 329;
    constexpr int railSwitchWN = 330;
    constexpr int railSwitchWS = 331;
    constexpr int railSwitchSW = 332;
    constexpr int railSwitchSE = 333;

    constexpr int wideRailHTop = 399;
    constexpr int wideRailHMid = 400;
    constexpr int wideRailHBot = 401;

    constexpr int wideRailVLeft = 402;
    constexpr int wideRailVMid = 403;
    constexpr int wideRailVRight = 404;

    constexpr int shallowFreshWater = 225;
    constexpr int deepFreshWater = 226;

    constexpr int shallowSeaWater = 231;
    constexpr int deepSeaWater = 232;

    constexpr int sandFloor = 381;

    constexpr int pickaxe = 388;
    constexpr int fellingAxe = 391;

    constexpr int dirtWall = 302;
    constexpr int stoneWall = 397;
    constexpr int glassWall = 114;
    constexpr int wireFence = 376;

    constexpr int katana = 103;

    constexpr int minecart = 405;
    constexpr int minecartController = 406;

    constexpr int arrowQuiver = 408;
    constexpr int boltQuiver = 409;

    constexpr int gasoline = 433;
    constexpr int diesel = 434;
    constexpr int electricity = 45;

    constexpr int gasolineGeneratorR = 458;
    constexpr int gasolineGeneratorT = 459;
    constexpr int gasolineGeneratorL = 460;
    constexpr int gasolineGeneratorB = 461;

    constexpr int dieselGeneratorR = 463;
    constexpr int dieselGeneratorT = 464;
    constexpr int dieselGeneratorL = 465;
    constexpr int dieselGeneratorB = 466;

    constexpr int solarGeneratorR = 467;
    constexpr int solarGeneratorT = 468;
    constexpr int solarGeneratorL = 469;
    constexpr int solarGeneratorB = 470;

    constexpr int steamGenerator = 471;

    constexpr int powerBankR = 473;
    constexpr int powerBankL = 474;

    constexpr int copperCable = 480;
    constexpr int silverCable = 482;

    constexpr int bollardLight = 118;

    ///////////////////////차량 부품//////////////////////
    constexpr int vehicleControl = 99;
    constexpr int helicopterController = 311;
    constexpr int engineV2Gasoline = 100;
    constexpr int fuelTank10L = 101;
    constexpr int tire = 102;
    constexpr int vehicleWall = 119;
    constexpr int vehicleDoor = 120;
    constexpr int vehicleGlass = 121;
    constexpr int vehiclePassage = 122;
    constexpr int vehicleSeat = 123;
    constexpr int trunkDoor = 124;
    constexpr int headlight = 126;
    constexpr int tailLight = 127;
    constexpr int vehicleRoof = 128;
    constexpr int vehicleTurret = 129;
    constexpr int steelBumper = 130;
    constexpr int bicycleSaddle = 132;
    constexpr int bicycleHandlebar = 133;
    constexpr int bicyclePedal = 135;
    constexpr int shoppingBasket = 136;
    constexpr int steerableTire = 142;

    constexpr int leverRL = 149;
    constexpr int leverUD = 150;

    constexpr int tactSwitchRL = 151;
    constexpr int tactSwitchUD = 152;

    constexpr int pressureSwitchRL = 163;
    constexpr int pressureSwitchUD = 164;

    constexpr int transistorR = 155;
    constexpr int transistorU = 156;
    constexpr int transistorL = 157;
    constexpr int transistorD = 158;

    constexpr int relayR = 165;
    constexpr int relayU = 166;
    constexpr int relayL = 167;
    constexpr int relayD = 168;

    constexpr int andGateR = 488;
    constexpr int andGateL = 489;

    constexpr int orGateR = 490;
    constexpr int orGateL = 491;

    constexpr int xorGateR = 492;
    constexpr int xorGateL = 493;

    constexpr int notGateR = 494;
    constexpr int notGateL = 495;

    constexpr int srLatchR = 496;
    constexpr int srLatchL = 497;

    constexpr int delayR = 498;
    constexpr int delayL = 499;

    constexpr int diodeR = 501;
    constexpr int diodeU = 502;
    constexpr int diodeL = 503;
    constexpr int diodeD = 504;

    constexpr int chargingPort = 159;

    constexpr int battery = 42;
    constexpr int batteryPack = 43;

    constexpr int minerHelmet = 393;

    ///////////////////////배관//////////////////////

    constexpr int pipe = 170;
    constexpr int transparentPipe = 171;
    constexpr int fluidTank = 172;
    constexpr int pumpR = 173;
    constexpr int pumpU = 174;
    constexpr int pumpL = 175;
    constexpr int pumpD = 176;
    constexpr int valveRL = 177;
    constexpr int valveUD = 178;
    constexpr int solenoidValveRL = 179;
    constexpr int solenoidValveUD = 180;

    constexpr int sprinklerRL = 181;
    constexpr int sprinklerUD = 182;

    constexpr int intakePipeR = 183;
    constexpr int intakePipeU = 184;
    constexpr int intakePipeL = 185;
    constexpr int intakePipeD = 186;

    constexpr int verticalPipe = 187;
    constexpr int verticalPipeRB = 188;
    constexpr int verticalPipeLB = 189;
    constexpr int verticalPipeRA = 190;
    constexpr int verticalPipeLA = 191;

    constexpr int woodenDoorH = 213; //수직일자통로(y축방향 통로)를 막는 수평으로 펼쳐진 나무문
    constexpr int woodenDoorV = 214; //수평일자통로(x축방향 통로)를 막는 수직으로 펼쳐진 나무문

    /////////////////////////////////////////////////

    constexpr int hoe = 139;
    constexpr int scythe = 140;
    constexpr int farmland = 110;
    constexpr int rice = 505;
    constexpr int wheat = 506;
    constexpr int potato = 507;
    constexpr int orange = 508;

    constexpr int strawHat = 509;
    constexpr int wateringCan = 510;

    constexpr int riceCrop = 511;
    constexpr int wheatCrop = 512;
    constexpr int potatoCrop = 513;

    constexpr int tomato = 514;
    constexpr int tomatoSeed = 515;
    constexpr int tomatoCrop = 516;

    constexpr int watermelon = 517;
    constexpr int watermelonSeed = 518;
    constexpr int watermelonCrop = 519;

    constexpr int carrot = 522;
    constexpr int carrotSeed = 523;
    constexpr int carrotCrop = 524;

    constexpr int cabbage = 525;
    constexpr int cabbageSeed = 526;
    constexpr int cabbageCrop = 527;

    constexpr int rawChicken = 528;
    constexpr int cacaoFruit = 529;
    constexpr int butter = 530;

    constexpr int campfire = 531;
    constexpr int electricOven = 532;
    constexpr int electricCooktop = 533;

    constexpr int cookingPot = 534;
    constexpr int fryingPan = 535;
    constexpr int ttukbaegi = 536;

    constexpr int eggFriedRice = 537;
    constexpr int woodenPlate = 538;
    constexpr int ceramicPlate = 539;

    constexpr int egg = 58;

    constexpr int scallion = 540;
    constexpr int onion = 541;
    constexpr int garlic = 542;

    constexpr int assaultRifle = 12;

    constexpr int altarOfRehylion = 543;

    constexpr int mutagen = 544;
    constexpr int autodoc = 545;
    
    constexpr int cbm_nervedrive = 546;
    constexpr int cbm_powerStorage = 547;
    constexpr int cbm_metabExchange = 548;

    constexpr int dyeAmpule = 556;

    constexpr int tshirt = 557; //EQUIP_SPR_GENDERED 적용 티셔츠. 착용 시 entityInfo.gender에 따라 T-SHIRT_MALE / T-SHIRT_FEMALE 스프라이트로 표시

    //속옷 3종. 외피보다 낮은 priority로 안쪽에 그려짐. 성별 무관 단일 스프라이트.
    constexpr int bra = 558;
    constexpr int panties = 559;
    constexpr int briefs = 560;
    constexpr int concreteWall = 561;

    ///////////////////////나무//////////////////////

    constexpr int oakTree = 115;
    constexpr int mapleTree = 116;
    constexpr int spruceTree = 117;
    constexpr int birchTree = 236;
    constexpr int cherryTree = 237;
    constexpr int pineTree = 238;
    constexpr int ginkgoTree = 239;
    constexpr int jungleTree = 240;
    constexpr int umbrellaAcaciaTree = 241;
    constexpr int palmTree = 242;
    constexpr int bamboo = 243;
    constexpr int appleTree = 244;
    constexpr int orangeTree = 245;
    constexpr int persimmonTree = 246;
    constexpr int lemonTree = 247;
    constexpr int peachTree = 248;
    constexpr int figTree = 249;
    constexpr int pearTree = 250;
    constexpr int juniperTree = 251;
    constexpr int magnoliaTree = 252;
    constexpr int bananaTree = 253;
    constexpr int willowTree = 254;
    constexpr int zelkovaTree = 602; //느티나무

    constexpr int treeStump = 282;
    constexpr int paver = 563;
    constexpr int rampUp = 603;
    constexpr int rampDown = 604;
    constexpr int guardrail = 605;
    constexpr int pillarWall = 606;

    ///////////////////////아스팔트 변형 (페인트용 절반·대각·화살표) //////////////////////
    //   whiteAsphalt(562) / yellowAsphalt(377) 의 시각 변형 타일들. 도로 페인팅에서
    //   직사각/대각 줄무늬, 차선 화살표 등을 표현. 색상은 베이스와 동일 — Map.ixx
    //   floorOverlay 가 베이스 색으로 동일 처리.

    // ── White Asphalt 절반 채움 (4면 axis + 4 대각) ──
    constexpr int whiteAsphaltLeftHalf   = 564;  // ◧
    constexpr int whiteAsphaltRightHalf  = 565;  // ◨
    constexpr int whiteAsphaltTopHalf    = 566;  // ⬒
    constexpr int whiteAsphaltBottomHalf = 567;  // ⬓
    constexpr int whiteAsphaltDiagUL     = 568;  // ◩  (/-cut, upper-left filled)
    constexpr int whiteAsphaltDiagLR     = 569;  // ◪  (/-cut, lower-right filled)
    constexpr int whiteAsphaltDiagUR     = 570;  //     (\-cut, upper-right filled)
    constexpr int whiteAsphaltDiagLL     = 571;  //     (\-cut, lower-left filled)

    // ── White Asphalt 쿼터(1/4) 채움 — 가운데 1/4 사각형만 흰색, 가는 횡단보도용 ──
    constexpr int whiteAsphaltRightQuarter  = 596;  // ▕  (우측 쿼터만 채움)
    constexpr int whiteAsphaltTopQuarter    = 597;  // ▔  (상단 쿼터만 채움)
    constexpr int whiteAsphaltLeftQuarter   = 598;  // ▏  (좌측 쿼터만 채움)
    constexpr int whiteAsphaltBottomQuarter = 599;  // ▁  (하단 쿼터만 채움)

    // ── White Asphalt 화살표 (8방향) ──
    constexpr int whiteAsphaltArrowR  = 572;  // →
    constexpr int whiteAsphaltArrowUR = 573;  // ↗
    constexpr int whiteAsphaltArrowU  = 574;  // ↑
    constexpr int whiteAsphaltArrowUL = 575;  // ↖
    constexpr int whiteAsphaltArrowL  = 576;  // ←
    constexpr int whiteAsphaltArrowLL = 577;  // ↙
    constexpr int whiteAsphaltArrowD  = 578;  // ↓
    constexpr int whiteAsphaltArrowLR = 579;  // ↘

    constexpr int whiteAsphaltBowtiePattern = 600; //▶◀
    constexpr int whiteAsphaltHourglassPattern = 601; //⧗

    // ── Yellow Asphalt 절반 채움 ──
    constexpr int yellowAsphaltLeftHalf   = 580;  // ◧
    constexpr int yellowAsphaltRightHalf  = 581;  // ◨
    constexpr int yellowAsphaltTopHalf    = 582;  // ⬒
    constexpr int yellowAsphaltBottomHalf = 583;  // ⬓
    constexpr int yellowAsphaltDiagUL     = 584;  // ◩
    constexpr int yellowAsphaltDiagLR     = 585;  // ◪
    constexpr int yellowAsphaltDiagUR     = 586;  //     (\-cut, UR filled)
    constexpr int yellowAsphaltDiagLL     = 587;  //     (\-cut, LL filled)

    // ── Yellow Asphalt 화살표 ──
    constexpr int yellowAsphaltArrowR  = 588;  // →
    constexpr int yellowAsphaltArrowUR = 589;  // ↗
    constexpr int yellowAsphaltArrowU  = 590;  // ↑
    constexpr int yellowAsphaltArrowUL = 591;  // ↖
    constexpr int yellowAsphaltArrowL  = 592;  // ←
    constexpr int yellowAsphaltArrowLL = 593;  // ↙
    constexpr int yellowAsphaltArrowD  = 594;  // ↓
    constexpr int yellowAsphaltArrowLR = 595;  // ↘

    ///////////////////////매직넘버 치환용 추가 상수 (itemDex.tsv 기준)//////////////////////
    //  startArea/Lot/HUD/Craft 등에서 raw int로 쓰이던 itemCode를 명명. 코드값은 itemDex.tsv 권위.

    //── 의류·소품 ──
    constexpr int backpack = 2;            //배낭
    constexpr int lead = 11;               //납
    constexpr int leadFragment = 25;       //납 조각
    constexpr int shoes = 105;             //신발
    constexpr int jeans = 106;             //청바지
    constexpr int blackTshirt = 107;       //검은색 티셔츠
    constexpr int fieldJacket = 290;       //야상자켓
    constexpr int hanbokSet = 390;         //한복세트

    //── 가구·건물·계단 ──
    constexpr int refrigerator = 96;       //냉장고
    constexpr int woodenFence = 206;       //나무 울타리
    constexpr int traditionalLamp = 211;   //전통등
    constexpr int tileFloor = 292;         //타일 바닥
    constexpr int trail = 293;             //오솔길
    constexpr int bed = 294;               //침대
    constexpr int bookshelf = 295;         //책장
    constexpr int woodenSign = 297;        //나무 표지판
    constexpr int upwardStairs = 298;      //올라가는 계단
    constexpr int downwardStairs = 299;    //내려가는 계단
    constexpr int wasteContainerWall = 375;//폐컨테이너벽
    constexpr int scrapMetalPile = 338;    //고철더미

    //── 꽃 (조경) ──
    constexpr int chrysanthemum = 265;     //국화
    constexpr int tulip = 266;             //튤립
    constexpr int forsythia = 267;         //개나리
    constexpr int freesia = 268;           //프리지아
    constexpr int azalea = 269;            //진달래
    constexpr int lily = 270;              //백합
    constexpr int lavender = 271;          //라벤더

    //── 차량·열차·헬기 부품 ──
    constexpr int motorcycleWindshield = 134; //바이크 윈드실드
    constexpr int shoppingCart = 137;      //쇼핑카트
    constexpr int trainControl = 313;      //열차 조종장치
    constexpr int helicopterRotor = 314;   //헬기 로터
    constexpr int tailRotor = 315;         //테일 로터
    constexpr int woodenCart = 378;        //나무 수레
    constexpr int foldingWagon = 379;      //접이식 왜건

    //── 무기·도구 ──
    constexpr int crossbow = 382;          //석궁
    constexpr int bow = 383;               //활
    constexpr int bolt = 384;              //볼트
    constexpr int arrow = 385;             //화살
    constexpr int fishingRod = 394;        //낚시대
    constexpr int shovel = 395;            //삽

    //── 용기 ──
    constexpr int plasticBottle = 373;     //페트병
    constexpr int largePlasticBottle = 475;//큰 페트병
    constexpr int gasCan = 476;            //연료통
    constexpr int milkJug = 477;           //우유통
    constexpr int glassBottle = 478;       //유리병

    //── 탄약·의료·기타 ──
    constexpr int ammoBox = 427;           //탄통
    constexpr int firstAidKit = 452;       //구급상자
    constexpr int healingOintment = 444;   //재생 연고
    constexpr int toothpaste = 445;        //치약
    constexpr int povidoneIodine = 450;    //포비돈 요오드
    constexpr int craftingItem = 453;      //조합 중인 아이템

    ///////////////////////시작 컨테이너/디버그 적재용 추가 상수 (itemDex.tsv 기준)//////////////////////
    //  startArea의 구급상자/냉장고/탄통/디버그 적재 목록에서 raw int로 쓰이던 itemCode 명명.

    //── 무기 ──
    constexpr int tripleSword = 3;         //삼중검
    constexpr int revolver = 4;            //리볼버
    constexpr int rifleBayonet = 18;       //라이플용 대검
    constexpr int pipeSpear = 23;          //파이프창
    constexpr int policeShield = 24;       //경찰방패
    constexpr int longsword = 386;         //롱소드
    constexpr int kiteShield = 387;        //카이트 실드
    constexpr int crusaderHelmet = 389;    //십자군 헬멧
    constexpr int shotgun = 410;           //샷건
    constexpr int pistol = 414;            //권총
    constexpr int smg = 415;               //SMG
    constexpr int sniperRifle = 417;       //스나이퍼 라이플

    //── 탄약 ──
    constexpr int magnum357Round = 5;          //.357 매그넘 탄
    constexpr int nato556Round = 15;           //5.56 NATO 탄
    constexpr int nato556Tracer = 16;          //5.56 NATO 예광탄
    constexpr int nato556AP = 17;              //5.56 NATO 철갑탄
    constexpr int slugRound = 411;             //슬러그탄
    constexpr int buckshot = 412;              //벅샷
    constexpr int dragonsBreath = 413;         //드래곤브레스
    constexpr int parabellum9mm = 416;         //9mm 파라벨룸
    constexpr int magnum357Rubber = 418;       //.357 매그넘 고무탄
    constexpr int magnum357Blank = 419;        //.357 매그넘 공포탄
    constexpr int magnum357Shotshell = 420;    //.357 매그넘 샷셀
    constexpr int magnum357AP = 421;           //.357 매그넘 철갑탄
    constexpr int magnum357HotLoad = 422;      //.357 매그넘 핫로드
    constexpr int nato556Blank = 423;          //5.56 NATO 공포탄
    constexpr int bmg50Ball = 424;             //.50 BMG 보통탄
    constexpr int bmg50Tracer = 425;           //.50 BMG 예광탄
    constexpr int bmg50AP = 426;               //.50 BMG 철갑탄

    //── 탄창 ──
    constexpr int rifleMagazine = 13;          //라이플 탄창
    constexpr int rifleDrumMagazine = 14;      //라이플 드럼 탄창
    constexpr int magazine9mm = 428;           //9mm 탄창
    constexpr int magazine9mmExtended = 429;   //9mm 대용량 탄창
    constexpr int sniperBmg50Magazine = 430;   //스나이퍼 .50 BMG 탄창

    //── 의료 ──
    constexpr int smallTube = 442;         //소형 튜브
    constexpr int tube = 443;              //튜브
    constexpr int splint = 446;            //부목
    constexpr int bandage = 447;           //붕대
    constexpr int medicineBottle = 449;    //약병
    constexpr int adhesiveBandage = 451;   //반창고

    //── 전자부품 (미설치 원자재) ──
    constexpr int ne555 = 1;               //NE555
    constexpr int electricSwitch = 60;     //스위치 (switch는 예약어 → electricSwitch)
    constexpr int fuseCartridge = 61;      //퓨즈 카트리지
    constexpr int transistor = 62;         //트랜지스터
    constexpr int relay = 63;              //릴레이
    constexpr int inductor = 64;           //인덕터
    constexpr int capacitor = 65;          //커패시터
    constexpr int supercapacitor = 66;     //슈퍼 커패시터
    constexpr int resistor = 67;           //저항

    //── 화학·도구 ──
    constexpr int distiller = 82;          //증류기
    constexpr int fermentedEthanol = 88;   //발효 에탄올
    constexpr int ethanol = 89;            //에탄올
    constexpr int benzene = 91;            //벤젠
    constexpr int nipper = 454;            //니퍼
    constexpr int plier = 455;             //플라이어
    constexpr int multimeter = 456;        //멀티미터

    //── 식량 ──
    constexpr int bread = 441;             //빵

};