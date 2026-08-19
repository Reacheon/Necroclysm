export module constVar:itemIDs;

export namespace itemID
{
    constexpr int none = 0;

    constexpr int metalFrame = 45;
    constexpr int dirt = 95;
    constexpr int grass = 191;
    constexpr int blackAsphalt = 251;
    constexpr int yellowAsphalt = 316;
    constexpr int whiteAsphalt = 488;

    constexpr int railRL = 265;
    constexpr int railTB = 266;
    constexpr int railBR = 267;
    constexpr int railTR = 268;
    constexpr int railTL = 269;
    constexpr int railBL = 270;

    constexpr int water = 61;


    //철도 분기기 : {R/L}Turn=분기방향(우/좌), {R/U/L/D}Enter=진입방향. dc는 진입방향 R->U->L->D (CCW)
    constexpr int railSwitchRTurnREnter = 271; //우회전, 진입 우
    constexpr int railSwitchRTurnUEnter = 272; //우회전, 진입 상
    constexpr int railSwitchRTurnLEnter = 273; //우회전, 진입 좌
    constexpr int railSwitchRTurnDEnter = 274; //우회전, 진입 하
    constexpr int railSwitchLTurnREnter = 275; //좌회전, 진입 우
    constexpr int railSwitchLTurnUEnter = 276; //좌회전, 진입 상
    constexpr int railSwitchLTurnLEnter = 277; //좌회전, 진입 좌
    constexpr int railSwitchLTurnDEnter = 278; //좌회전, 진입 하

    constexpr int wideRailHTop = 338;
    constexpr int wideRailHMid = 339;
    constexpr int wideRailHBot = 340;

    constexpr int wideRailVLeft = 341;
    constexpr int wideRailVMid = 342;
    constexpr int wideRailVRight = 343;

    constexpr int shallowFreshWater = 192;
    constexpr int deepFreshWater = 193;

    constexpr int shallowSeaWater = 194;
    constexpr int deepSeaWater = 195;

    constexpr int sandFloor = 320;

    constexpr int pickaxe = 327;
    constexpr int fellingAxe = 330;

    constexpr int dirtWall = 257;
    constexpr int stoneWall = 336;
    constexpr int glassWall = 98;
    constexpr int wireFence = 315;

    constexpr int katana = 89;

    constexpr int minecart = 344;
    constexpr int minecartController = 345;

    constexpr int arrowQuiver = 347;
    constexpr int boltQuiver = 348;

    constexpr int gasoline = 372;
    constexpr int diesel = 373;
    constexpr int electricity = 42;

    constexpr int gasolineGeneratorR = 396;
    constexpr int gasolineGeneratorT = 397;
    constexpr int gasolineGeneratorL = 398;
    constexpr int gasolineGeneratorB = 399;

    constexpr int dieselGeneratorR = 401;
    constexpr int dieselGeneratorT = 402;
    constexpr int dieselGeneratorL = 403;
    constexpr int dieselGeneratorB = 404;

    constexpr int solarGeneratorR = 405;
    constexpr int solarGeneratorT = 406;
    constexpr int solarGeneratorL = 407;
    constexpr int solarGeneratorB = 408;

    constexpr int steamGeneratorR = 171;
    constexpr int steamGeneratorT = 172;
    constexpr int steamGeneratorL = 173;
    constexpr int steamGeneratorB = 174;

    constexpr int powerBankR = 187;
    constexpr int powerBankT = 188;
    constexpr int powerBankL = 189;
    constexpr int powerBankB = 190;

    constexpr int copperCable = 418;
    constexpr int silverCable = 420;

    constexpr int bollardLight = 102;

    ///////////////////////차량 부품//////////////////////
    constexpr int vehicleControl = 85;
    constexpr int helicopterController = 260;
    constexpr int engineV2Gasoline = 86;
    constexpr int fuelTank10L = 87;
    constexpr int tire = 88;
    constexpr int vehicleWall = 103;
    constexpr int vehicleDoor = 104;
    constexpr int vehicleGlass = 105;
    constexpr int vehiclePassage = 106;
    constexpr int vehicleSeat = 107;
    constexpr int trunkDoor = 108;
    constexpr int headlight = 110;
    constexpr int tailLight = 111;
    constexpr int vehicleRoof = 112;
    constexpr int vehicleTurret = 113;
    constexpr int steelBumper = 114;
    constexpr int bicycleSaddle = 116;
    constexpr int bicycleHandlebar = 117;
    constexpr int bicyclePedal = 119;
    constexpr int shoppingBasket = 120;
    constexpr int steerableTire = 126;

    constexpr int leverRL = 128;
    constexpr int leverUD = 129;

    constexpr int tactSwitchRL = 130;
    constexpr int tactSwitchUD = 131;

    constexpr int pressureSwitchRL = 139;
    constexpr int pressureSwitchUD = 140;

    constexpr int transistorR = 134;
    constexpr int transistorU = 135;
    constexpr int transistorL = 136;
    constexpr int transistorD = 137;

    constexpr int relayR = 141;
    constexpr int relayU = 142;
    constexpr int relayL = 143;
    constexpr int relayD = 144;

    constexpr int andGateR = 533;
    constexpr int andGateU = 534;
    constexpr int andGateL = 535;
    constexpr int andGateD = 536;

    constexpr int orGateR = 537;
    constexpr int orGateU = 538;
    constexpr int orGateL = 539;
    constexpr int orGateD = 540;

    constexpr int xorGateR = 541;
    constexpr int xorGateU = 542;
    constexpr int xorGateL = 543;
    constexpr int xorGateD = 544;

    constexpr int notGateR = 545;
    constexpr int notGateU = 546;
    constexpr int notGateL = 547;
    constexpr int notGateD = 548;

    constexpr int srLatchR = 426;
    constexpr int srLatchU = 427;
    constexpr int srLatchL = 428;
    constexpr int srLatchD = 429;

    constexpr int delayR = 409;
    constexpr int delayU = 410;
    constexpr int delayL = 411;
    constexpr int delayD = 412;

    constexpr int diodeR = 431;
    constexpr int diodeU = 432;
    constexpr int diodeL = 433;
    constexpr int diodeD = 434;

    constexpr int chargingPort = 138;

    constexpr int battery = 40;
    constexpr int batteryPack = 41;

    constexpr int minerHelmet = 332;

    ///////////////////////배관//////////////////////

    constexpr int pipe = 145;
    constexpr int transparentPipe = 146;
    constexpr int fluidTank = 147;
    constexpr int pumpR = 148;
    constexpr int pumpU = 149;
    constexpr int pumpL = 150;
    constexpr int pumpD = 151;
    constexpr int valveRL = 152;
    constexpr int valveUD = 153;
    constexpr int solenoidValveRL = 154;
    constexpr int solenoidValveUD = 155;

    constexpr int sprinklerRL = 156;
    constexpr int sprinklerUD = 157;

    constexpr int intakePipeR = 158;
    constexpr int intakePipeU = 159;
    constexpr int intakePipeL = 160;
    constexpr int intakePipeD = 161;

    constexpr int verticalPipe = 162;
    //아랫층(below) 연결 엘보 : 평면 방향 R/U/L/D
    constexpr int verticalElbowRB = 163;
    constexpr int verticalElbowUB = 164;
    constexpr int verticalElbowLB = 165;
    constexpr int verticalElbowDB = 166;
    //윗층(above) 연결 엘보 : 평면 방향 R/U/L/D
    constexpr int verticalElbowRA = 167;
    constexpr int verticalElbowUA = 168;
    constexpr int verticalElbowLA = 169;
    constexpr int verticalElbowDA = 170;

    constexpr int woodenDoorH = 184; //수직일자통로(y축방향 통로)를 막는 수평으로 펼쳐진 나무문
    constexpr int woodenDoorV = 185; //수평일자통로(x축방향 통로)를 막는 수직으로 펼쳐진 나무문

    /////////////////////////////////////////////////

    constexpr int hoe = 123;
    constexpr int scythe = 124;
    constexpr int farmland = 96;
    constexpr int rice = 435;
    constexpr int wheat = 436;
    constexpr int potato = 437;
    constexpr int orange = 438;

    constexpr int strawHat = 439;
    constexpr int wateringCan = 440;

    constexpr int riceCrop = 441;
    constexpr int wheatCrop = 442;
    constexpr int potatoCrop = 443;

    constexpr int tomato = 444;
    constexpr int tomatoSeed = 445;
    constexpr int tomatoCrop = 446;

    constexpr int watermelon = 447;
    constexpr int watermelonSeed = 448;
    constexpr int watermelonCrop = 449;

    constexpr int carrot = 450;
    constexpr int carrotSeed = 451;
    constexpr int carrotCrop = 452;

    constexpr int cabbage = 453;
    constexpr int cabbageSeed = 454;
    constexpr int cabbageCrop = 455;

    constexpr int rawChicken = 456;
    constexpr int cacaoFruit = 457;
    constexpr int butter = 458;

    constexpr int campfire = 459;
    constexpr int electricOven = 460;
    constexpr int electricCooktop = 461;

    constexpr int cookingPot = 462;
    constexpr int fryingPan = 463;
    constexpr int ttukbaegi = 464;

    constexpr int eggFriedRice = 465;
    constexpr int woodenPlate = 466;
    constexpr int ceramicPlate = 467;

    constexpr int egg = 51;

    constexpr int scallion = 468;
    constexpr int onion = 469;
    constexpr int garlic = 470;

    constexpr int assaultRifle = 12;

    constexpr int mutagen = 471;
    constexpr int autodoc = 472;
    
    constexpr int cbm_nervedrive = 473;
    constexpr int cbm_powerStorage = 474;
    constexpr int cbm_metabExchange = 475;

    constexpr int dyeAmpule = 482;

    constexpr int tshirt = 483; //EQUIP_SPR_GENDERED 적용 티셔츠. 착용 시 entityInfo.gender에 따라 T-SHIRT_MALE / T-SHIRT_FEMALE 스프라이트로 표시

    //속옷 3종. 외피보다 낮은 priority로 안쪽에 그려짐. 성별 무관 단일 스프라이트.
    constexpr int bra = 484;
    constexpr int panties = 485;
    constexpr int briefs = 486;
    constexpr int concreteWall = 487;

    ///////////////////////나무//////////////////////

    constexpr int oakTree = 99;
    constexpr int mapleTree = 100;
    constexpr int spruceTree = 101;
    constexpr int birchTree = 199;
    constexpr int cherryTree = 200;
    constexpr int pineTree = 201;
    constexpr int ginkgoTree = 202;
    constexpr int jungleTree = 203;
    constexpr int umbrellaAcaciaTree = 204;
    constexpr int palmTree = 205;
    constexpr int bamboo = 206;
    constexpr int appleTree = 207;
    constexpr int orangeTree = 208;
    constexpr int persimmonTree = 209;
    constexpr int lemonTree = 210;
    constexpr int peachTree = 211;
    constexpr int figTree = 212;
    constexpr int pearTree = 213;
    constexpr int juniperTree = 214;
    constexpr int magnoliaTree = 215;
    constexpr int bananaTree = 216;
    constexpr int willowTree = 217;
    constexpr int zelkovaTree = 528; //느티나무

    constexpr int treeStump = 238;
    constexpr int paver = 489;
    constexpr int rampUp = 529;
    constexpr int rampDown = 530;
    constexpr int guardrail = 531;
    constexpr int pillarWall = 532;

    ///////////////////////아스팔트 변형 (페인트용 절반·대각·화살표) //////////////////////
    //   whiteAsphalt(562) / yellowAsphalt(377) 의 시각 변형 타일들. 도로 페인팅에서
    //   floorOverlay 가 베이스 색으로 동일 처리.

    // ── White Asphalt 절반 채움 (4면 axis + 4 대각) ──
    constexpr int whiteAsphaltLeftHalf = 490;  // ◧
    constexpr int whiteAsphaltRightHalf = 491;  // ◨
    constexpr int whiteAsphaltTopHalf = 492;  // ⬒
    constexpr int whiteAsphaltBottomHalf = 493;  // ⬓
    constexpr int whiteAsphaltDiagUL = 494;  // ◩  (/-cut, upper-left filled)
    constexpr int whiteAsphaltDiagLR = 495;  // ◪  (/-cut, lower-right filled)
    constexpr int whiteAsphaltDiagUR = 496;  //     (\-cut, upper-right filled)
    constexpr int whiteAsphaltDiagLL = 497;  //     (\-cut, lower-left filled)

    constexpr int whiteAsphaltRightQuarter = 522;  // ▕  (우측 쿼터만 채움)
    constexpr int whiteAsphaltTopQuarter = 523;  // ▔  (상단 쿼터만 채움)
    constexpr int whiteAsphaltLeftQuarter = 524;  // ▏  (좌측 쿼터만 채움)
    constexpr int whiteAsphaltBottomQuarter = 525;  // ▁  (하단 쿼터만 채움)

    // ── White Asphalt 화살표 (8방향) ──
    constexpr int whiteAsphaltArrowR = 498;  // →
    constexpr int whiteAsphaltArrowUR = 499;  // ↗
    constexpr int whiteAsphaltArrowU = 500;  // ↑
    constexpr int whiteAsphaltArrowUL = 501;  // ↖
    constexpr int whiteAsphaltArrowL = 502;  // ←
    constexpr int whiteAsphaltArrowLL = 503;  // ↙
    constexpr int whiteAsphaltArrowD = 504;  // ↓
    constexpr int whiteAsphaltArrowLR = 505;  // ↘

    constexpr int whiteAsphaltBowtiePattern = 526; //▶◀
    constexpr int whiteAsphaltHourglassPattern = 527; //⧗

    // ── Yellow Asphalt 절반 채움 ──
    constexpr int yellowAsphaltLeftHalf = 506;  // ◧
    constexpr int yellowAsphaltRightHalf = 507;  // ◨
    constexpr int yellowAsphaltTopHalf = 508;  // ⬒
    constexpr int yellowAsphaltBottomHalf = 509;  // ⬓
    constexpr int yellowAsphaltDiagUL = 510;  // ◩
    constexpr int yellowAsphaltDiagLR = 511;  // ◪
    constexpr int yellowAsphaltDiagUR = 512;  //     (\-cut, UR filled)
    constexpr int yellowAsphaltDiagLL = 513;  //     (\-cut, LL filled)

    // ── Yellow Asphalt 화살표 ──
    constexpr int yellowAsphaltArrowR = 514;  // →
    constexpr int yellowAsphaltArrowUR = 515;  // ↗
    constexpr int yellowAsphaltArrowU = 516;  // ↑
    constexpr int yellowAsphaltArrowUL = 517;  // ↖
    constexpr int yellowAsphaltArrowL = 518;  // ←
    constexpr int yellowAsphaltArrowLL = 519;  // ↙
    constexpr int yellowAsphaltArrowD = 520;  // ↓
    constexpr int yellowAsphaltArrowLR = 521;  // ↘

    //── 의류·소품 ──
    constexpr int backpack = 2;            //배낭
    constexpr int lead = 11;               //납
    constexpr int leadFragment = 25;       //납 조각
    constexpr int shoes = 91;             //신발
    constexpr int jeans = 92;             //청바지
    constexpr int blackTshirt = 93;       //검은색 티셔츠
    constexpr int fieldJacket = 246;       //야상자켓
    constexpr int hanbokSet = 329;         //한복세트

    //── 가구·건물·계단 ──
    constexpr int refrigerator = 82;       //냉장고
    constexpr int woodenFence = 177;       //나무 울타리
    constexpr int traditionalLamp = 182;   //전통등
    constexpr int tileFloor = 247;         //타일 바닥
    constexpr int trail = 248;             //오솔길
    constexpr int bed = 249;               //침대
    constexpr int bookshelf = 250;         //책장
    constexpr int woodenSign = 252;        //나무 표지판
    constexpr int upwardStairs = 253;      //올라가는 계단
    constexpr int downwardStairs = 254;    //내려가는 계단
    constexpr int wasteContainerWall = 314;//폐컨테이너벽
    constexpr int scrapMetalPile = 279;    //고철더미

    //── 꽃 (조경) ──
    constexpr int chrysanthemum = 227;     //국화
    constexpr int tulip = 228;             //튤립
    constexpr int forsythia = 229;         //개나리
    constexpr int freesia = 230;           //프리지아
    constexpr int azalea = 231;            //진달래
    constexpr int lily = 232;              //백합
    constexpr int lavender = 233;          //라벤더

    //── 차량·열차·헬기 부품 ──
    constexpr int motorcycleWindshield = 118; //바이크 윈드실드
    constexpr int shoppingCart = 121;      //쇼핑카트
    constexpr int trainControl = 262;      //열차 조종장치
    constexpr int helicopterRotor = 263;   //헬기 로터
    constexpr int tailRotor = 264;         //테일 로터
    constexpr int woodenCart = 317;        //나무 수레
    constexpr int foldingWagon = 318;      //접이식 왜건

    //── 무기·도구 ──
    constexpr int crossbow = 321;          //석궁
    constexpr int bow = 322;               //활
    constexpr int bolt = 323;              //볼트
    constexpr int arrow = 324;             //화살
    constexpr int fishingRod = 333;        //낚시대
    constexpr int shovel = 334;            //삽

    //── 용기 ──
    constexpr int plasticBottle = 312;     //페트병
    constexpr int largePlasticBottle = 413;//큰 페트병
    constexpr int gasCan = 414;            //연료통
    constexpr int milkJug = 415;           //우유통
    constexpr int glassBottle = 416;       //유리병

    //── 탄약·의료·기타 ──
    constexpr int ammoBox = 366;           //탄통
    constexpr int firstAidKit = 390;       //구급상자
    constexpr int healingOintment = 383;   //재생 연고
    constexpr int toothpaste = 384;        //치약
    constexpr int povidoneIodine = 388;    //포비돈 요오드
    constexpr int craftingItem = 391;      //조합 중인 아이템

    //── 무기 ──
    constexpr int tripleSword = 3;         //삼중검
    constexpr int revolver = 4;            //리볼버
    constexpr int rifleBayonet = 18;       //라이플용 대검
    constexpr int pipeSpear = 23;          //파이프창
    constexpr int policeShield = 24;       //경찰방패
    constexpr int longsword = 325;         //롱소드
    constexpr int kiteShield = 326;        //카이트 실드
    constexpr int crusaderHelmet = 328;    //십자군 헬멧
    constexpr int shotgun = 349;           //샷건
    constexpr int pistol = 353;            //권총
    constexpr int smg = 354;               //SMG
    constexpr int sniperRifle = 356;       //스나이퍼 라이플

    //── 탄약 ──
    constexpr int magnum357Round = 5;          //.357 매그넘 탄
    constexpr int nato556Round = 15;           //5.56 NATO 탄
    constexpr int nato556Tracer = 16;          //5.56 NATO 예광탄
    constexpr int nato556AP = 17;              //5.56 NATO 철갑탄
    constexpr int slugRound = 350;             //슬러그탄
    constexpr int buckshot = 351;              //벅샷
    constexpr int dragonsBreath = 352;         //드래곤브레스
    constexpr int parabellum9mm = 355;         //9mm 파라벨룸
    constexpr int magnum357Rubber = 357;       //.357 매그넘 고무탄
    constexpr int magnum357Blank = 358;        //.357 매그넘 공포탄
    constexpr int magnum357Shotshell = 359;    //.357 매그넘 샷셀
    constexpr int magnum357AP = 360;           //.357 매그넘 철갑탄
    constexpr int magnum357HotLoad = 361;      //.357 매그넘 핫로드
    constexpr int nato556Blank = 362;          //5.56 NATO 공포탄
    constexpr int bmg50Ball = 363;             //.50 BMG 보통탄
    constexpr int bmg50Tracer = 364;           //.50 BMG 예광탄
    constexpr int bmg50AP = 365;               //.50 BMG 철갑탄

    //── 탄창 ──
    constexpr int rifleMagazine = 13;          //라이플 탄창
    constexpr int rifleDrumMagazine = 14;      //라이플 드럼 탄창
    constexpr int magazine9mm = 367;           //9mm 탄창
    constexpr int magazine9mmExtended = 368;   //9mm 대용량 탄창
    constexpr int sniperBmg50Magazine = 369;   //스나이퍼 .50 BMG 탄창

    //── 의료 ──
    constexpr int smallTube = 381;         //소형 튜브
    constexpr int tube = 382;              //튜브
    constexpr int splint = 385;            //부목
    constexpr int bandage = 386;           //붕대
    constexpr int medicineBottle = 387;    //약병
    constexpr int adhesiveBandage = 389;   //반창고

    //── 전자부품 (미설치 원자재) ──
    constexpr int ne555 = 1;               //NE555
    constexpr int electricSwitch = 53;     //스위치 (switch는 예약어 → electricSwitch)
    constexpr int fuseCartridge = 54;      //퓨즈 카트리지
    constexpr int transistor = 55;         //트랜지스터
    constexpr int relay = 56;              //릴레이
    constexpr int inductor = 57;           //인덕터
    constexpr int capacitor = 58;          //커패시터
    constexpr int supercapacitor = 59;     //슈퍼 커패시터
    constexpr int resistor = 60;           //저항

    //── 화학·도구 ──
    constexpr int distiller = 70;          //증류기
    constexpr int fermentedEthanol = 74;   //발효 에탄올
    constexpr int ethanol = 75;            //에탄올
    constexpr int benzene = 77;            //벤젠
    constexpr int nipper = 392;            //니퍼
    constexpr int plier = 393;             //플라이어
    constexpr int multimeter = 394;        //멀티미터

    //── 식량 ──
    constexpr int bread = 380;             //빵

    constexpr int test = 0;                //테스트 아이템 (압축 후에도 0 고정)
    constexpr int log = 331;               //통나무 (Prop 파괴 시 드롭)
    constexpr int dirtItem = 335;          //흙 아이템 (흙벽 파괴 드롭). 바닥 dirt(109)와 다름
    constexpr int stone = 337;             //돌 아이템 (돌벽 파괴 드롭). rockWall(397)과 다름

    ///////////////////////////////////////////////////////////////////////////////

    constexpr int woodenStool = 549;          //스툴 나무 의자

    constexpr int woodenChairR = 550;         //목재 의자
    constexpr int woodenChairU = 551;
    constexpr int woodenChairL = 552;
    constexpr int woodenChairD = 553;

    constexpr int cardboardBox = 554;         //골판지 상자
    constexpr int woodenTable = 555;          //나무 테이블
    constexpr int steelTable = 556;           //철제 테이블
    constexpr int drawer = 557;               //서랍
    constexpr int wardrobe = 558;             //장롱
    constexpr int openWardrobe = 559;         //개방형 장롱
    constexpr int roundWoodenTable = 560;     //원형 목재 테이블

    constexpr int schoolChairR = 561;         //학교 의자
    constexpr int schoolChairU = 562;
    constexpr int schoolChairL = 563;
    constexpr int schoolChairD = 564;

    constexpr int officeChairR = 565;         //사무용 의자
    constexpr int officeChairU = 566;
    constexpr int officeChairL = 567;
    constexpr int officeChairD = 568;

    constexpr int headrestChairR = 569;       //헤드레스트 의자
    constexpr int headrestChairU = 570;
    constexpr int headrestChairL = 571;
    constexpr int headrestChairD = 572;

    constexpr int bluePlasticChair = 573;     //청색 플라스틱 의자
    constexpr int redPlasticChair = 574;      //적색 플라스틱 의자
    constexpr int cabinet = 575;              //캐비닛
    constexpr int safe = 576;                 //금고
    constexpr int window = 577;               //창문
    constexpr int displayRefrigerator = 578;  //쇼케이스 냉장고
    constexpr int checkoutCounter = 579;      //계산대
    constexpr int coffeeMachine = 580;        //커피 머신
    constexpr int flowerPot = 581;            //화분
    constexpr int telephone = 582;            //전화기
    constexpr int monitor = 583;              //모니터
    constexpr int shelf = 584;                //선반
    constexpr int computer = 585;             //컴퓨터
    constexpr int printer = 586;              //프린터기
    constexpr int centrifuge = 587;           //원심분리기
    constexpr int electronicScale = 588;      //전자 저울
    constexpr int operatingTable = 589;       //수술대
    constexpr int dentalChair = 590;          //치과 의자
    constexpr int drum = 591;                 //드럼통
    constexpr int microscope = 592;           //현미경
    constexpr int vacuumCleaner = 593;        //청소기
    constexpr int autoclave = 594;            //오토클레이브
    constexpr int atm = 595;                  //ATM
    constexpr int tissueDispenser = 596;      //휴지 디스펜서
    constexpr int ivStand = 597;              //링거 스탠드
    constexpr int urinal = 598;               //소변기
    constexpr int toilet = 599;               //좌변기
    constexpr int washbasin = 600;            //세면대
    constexpr int bathtub = 601;              //욕조
    constexpr int shower = 602;               //샤워기

    constexpr int glassDoorH = 603;           //유리문 (↔, 수직통로 차단)
    constexpr int glassDoorV = 604;           //유리문 (↕, 수평통로 차단)

    constexpr int ventilator = 605;           //인공호흡기

    constexpr int blackSofaR = 606;           //검은색 소파
    constexpr int blackSofaU = 607;
    constexpr int blackSofaL = 608;
    constexpr int blackSofaD = 609;

    constexpr int greenSofaR = 610;           //녹색 소파
    constexpr int greenSofaU = 611;
    constexpr int greenSofaL = 612;
    constexpr int greenSofaD = 613;

    constexpr int redSofaR = 614;             //적색 소파
    constexpr int redSofaU = 615;
    constexpr int redSofaL = 616;
    constexpr int redSofaD = 617;

    constexpr int beigeSofaR = 618;           //베이지색 소파
    constexpr int beigeSofaU = 619;
    constexpr int beigeSofaL = 620;
    constexpr int beigeSofaD = 621;

    constexpr int cinemaSeatR = 622;          //영화관 의자
    constexpr int cinemaSeatU = 623;
    constexpr int cinemaSeatL = 624;
    constexpr int cinemaSeatD = 625;

    constexpr int ctScannerR = 626;           //CT 기계
    constexpr int ctScannerU = 627;
    constexpr int ctScannerL = 628;
    constexpr int ctScannerD = 629;

    constexpr int mriScannerR = 630;          //MRI 기계
    constexpr int mriScannerU = 631;
    constexpr int mriScannerL = 632;
    constexpr int mriScannerD = 633;

    constexpr int examinationTableR = 634;    //검사 테이블
    constexpr int examinationTableU = 635;
    constexpr int examinationTableL = 636;
    constexpr int examinationTableD = 637;

    constexpr int whiteboard = 638;           //화이트보드
    constexpr int humidifier = 639;           //가습기
    constexpr int controlPanel = 640; //제어반(공장)

    constexpr int mechanicalWinch = 641;      //기계식 윈치(인접 롤업도어 개폐)
    constexpr int rollupDoorH = 642;          //롤업도어 (↔, 좌우 체인)
    constexpr int rollupDoorV = 643;          //롤업도어 (↕, 상하 체인)

    constexpr int polishedCeramicTile = 644;  //광택 세라믹 타일
    constexpr int mop = 645;                  //대걸레
    constexpr int iceCreamFreezer = 646;      //아이스크림 냉동고

};