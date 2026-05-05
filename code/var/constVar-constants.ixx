export module constVar:constants;

import std;

export constexpr int DARK_VISION_RADIUS = 13;

export constexpr double MAX_ZOOM = 5.0;
export constexpr double MIN_ZOOM = 2.0;

export constexpr int CHUNK_SIZE_X = 16; //청크의 x길이 (2의 거듭제곱)
export constexpr int CHUNK_SIZE_Y = 16; //청크의 y길이 (2의 거듭제곱)
export constexpr int CHUNK_SIZE_Z = 1; //청크의 z길이, 현재 1로 미사용됨
export constexpr int MAX_FONT_SIZE = 50;
export constexpr int EX_INPUT_TEXT_MAX = 30;
export constexpr int EQUIP_ITEM_MAX = 10;
export constexpr int LOOT_ITEM_MAX = 9;
export constexpr int INVENTORY_ITEM_MAX = 9;
export constexpr int DMG_FLAG_SIZE = 3;
export constexpr int TALENT_SIZE = 20;

// 16칸이 한 청크니까 걸칠 수 있는 정도로, 차량의 왼쪽끝(코어)이 청크의 오른쪽끝 그러면... 5청크*5청크 분석해야한다.
// ●|○○○○○○○○○○○○○○○○|○○○○○○○○○○○○○○○×|
export constexpr int MAX_VEHICLE_SIZE = 31;


export constexpr int CHUNK_LOADING_RANGE = 5;
export constexpr int MINIMAP_DIAMETER = 41; //미니맵의 지름 (홀수)
export constexpr int NAVIMAP_WIDTH = 99;//167;
export constexpr int NAVIMAP_HEIGHT = 58;//99;
// 월드 생성 좌표계 (완전 디커플링 — 픽셀과 청크는 무관)
// 패치 PNG는 400x400 픽셀, 픽셀 하나당 50타일이 실세계 영역을 표현
// 1패치 = 400*50 = 20000타일 변길이 = 20000/16 = 1250청크 변길이
export constexpr int TILE_PER_PIXEL = 50;     // 픽셀 1칸이 표현하는 타일 길이
export constexpr int PIXEL_PER_PATCH = 400;   // 패치 PNG 한 변의 픽셀 수
export constexpr int TILE_PER_PATCH = TILE_PER_PIXEL * PIXEL_PER_PATCH; // 20000
export constexpr int TOLERANCE_LSTICK = 10000; //LStick이 이 값을 넘어야 판정이 일어남
export constexpr int TOLERANCE_HOLD_DEL_XY = 20; //이 값 이상 움직일 경우 홀드 이벤트가 일어나지 않음

export constexpr int MARKER_LIMIT_DIST = 100; //플레이어로부터 100칸 이상 떨어지면 마커가 그려지지 않음

export constexpr int MAX_PROFIC_LEVEL = 27;

export constexpr int SKILL_GUI_MAX = 7;
export constexpr int QUICK_SLOT_MAX = 8;

export constexpr int CRAFT_MAX_ROW = 4;

export constexpr int MAX_ENC = 100; //최대 방해도
export constexpr int PART_MAX_HP = 100;

export constexpr int START_YEAR = 2099;
export constexpr int START_MONTH = 7;
export constexpr int START_DAY = 15;
export constexpr int START_HOUR = 12;
export constexpr int START_MINUTE = 0;

export constexpr std::array<int, 27> expTable =
{ 50, 100, 150, 200, 250, 300, 350, 400, 450,
550, 650, 750, 850, 950, 1050, 1150, 1250, 1350,
1500, 1650, 1800, 1950, 2100, 2250, 2400, 2550, 2750 };

export constexpr int CHAR_TEXTURE_WIDTH = 288;
export constexpr int CHAR_TEXTURE_HEIGHT = 384;


// 허기/갈증/피로: double 퍼센트 (0.0% = 최상, 100.0% = 사망)
// 임계값 (낮을수록 좋음)
export constexpr double PLAYER_HUNGRY_PERCENT       = 25.0; // 배고픔
export constexpr double PLAYER_VERY_HUNGRY_PERCENT  = 50.0; // 굶주림
export constexpr double PLAYER_STARVE_PERCENT       = 75.0; // 영양실조

export constexpr double PLAYER_THIRSTY_PERCENT          = 25.0; // 목마름
export constexpr double PLAYER_VERY_THIRSTY_PERCENT     = 50.0; // 심한 갈증
export constexpr double PLAYER_DEHYDRATION_PERCENT      = 75.0; // 탈수

export constexpr double PLAYER_TIRED_PERCENT        = 25.0; // 피곤함
export constexpr double PLAYER_VERY_TIRED_PERCENT   = 50.0; // 심한 피로
export constexpr double PLAYER_EXHAUSTED_PERCENT    = 75.0; // 탈진

// 분당 증가 속도 (%/분), 기존 실제 단위 기반 역산
export constexpr double HUNGER_SPEED  = 1.2 / 17280.0 * 100.0; // ≈0.00694%/분 (10일)
export constexpr double THIRST_SPEED  = 1.5 / 8640.0  * 100.0; // ≈0.01736%/분 (4일)
export constexpr double FATIGUE_SPEED = 1.0 / 4320.0  * 100.0; // ≈0.02315%/분 (3일)

// 아이템 칼로리/수분→퍼센트 변환용 상수 (기존 최대값 보존)
export constexpr double CALORIE_TO_PERCENT   = 100.0 / 17280.0;
export constexpr double HYDRATION_TO_PERCENT = 100.0 / 8640.0;

export constexpr int MAX_BATCH = 4096;

export constexpr int MAX_CIRCUIT_LOOP_COUNT = 50; //전자회로가 트랜지스터 게이트로 인해 중복 업데이트될 때의 한계값