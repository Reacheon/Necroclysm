export module constVar:constants;

import std;

export constexpr int DARK_VISION_RADIUS = 13;

export constexpr double MAX_ZOOM = 5.0;
export constexpr double MIN_ZOOM = 2.0;

export constexpr int CHUNK_SIZE_X = 24; //청크의 x길이 (건물 양자화 그리드와 정합: 24타일 = 1픽셀 / 2 = 최소 건물 footprint)
export constexpr int CHUNK_SIZE_Y = 24; //청크의 y길이 (건물 양자화 그리드와 정합: 24타일 = 1픽셀 / 2 = 최소 건물 footprint)
export constexpr int CHUNK_SIZE_Z = 1; //청크의 z길이, 현재 1로 미사용됨
export constexpr int MAX_FONT_SIZE = 50;
export constexpr int EX_INPUT_TEXT_MAX = 30;
export constexpr int EQUIP_ITEM_MAX = 10;
export constexpr int LOOT_ITEM_MAX = 9;
export constexpr int INVENTORY_ITEM_MAX = 9;
export constexpr int DMG_FLAG_SIZE = 3;
export constexpr int TALENT_SIZE = 20;

// 24칸이 한 청크니까 차량의 왼쪽끝(코어)이 청크의 오른쪽끝에 있어도 최대 2청크만 걸침 → 3*3 분석으로 충분.
// (청크가 16일 땐 31차량이 3청크 걸쳐 5*5 분석이 필요했으나, 24청크 전환으로 여유 확보)
export constexpr int MAX_VEHICLE_SIZE = 31;


export constexpr int CHUNK_LOADING_RANGE = 5;
export constexpr int MINIMAP_DIAMETER = 41; //미니맵의 지름 (홀수)
export constexpr int MINIMAP_TILE_PX = 6;   //미니맵에서 타일 1개의 픽셀 크기 (HUD draw가 zoom 6.0로 그리던 것과 동일한 화면 점유)
export constexpr int NAVIMAP_WIDTH = 99;//167;
export constexpr int NAVIMAP_HEIGHT = 58;//99;
// 월드 생성 좌표계 (픽셀 ↔ 청크 정합 정렬)
// 패치 PNG는 400x400 픽셀, 픽셀 1칸 = 24타일 (= 24 × 1 → 픽셀 1개 = 1×1 청크 = 정확히 1청크)
// 1패치 = 400*24 = 9600타일 변길이 = 9600/24 = 400청크 변길이
//
// ★ 본 블록의 모든 상수는 월드 좌표계의 단일 진리원천(SSOT).
//   worldGrid / worldWrap / Sector / CityPlan 등 모듈은 본 상수를 import해 사용 —
//   같은 의미의 별도 상수를 모듈 내에 두지 말 것 (이중 상수 드리프트 방지).
export constexpr int TILE_PER_PIXEL = 24;     // 픽셀 1칸이 표현하는 타일 길이 (청크 크기와 동일 — 1픽셀 = 1청크 완전 정합)
export constexpr int PIXEL_PER_PATCH = 400;   // 패치 PNG 한 변의 픽셀 수
export constexpr int TILE_PER_PATCH = TILE_PER_PIXEL * PIXEL_PER_PATCH; // 9600

// 월드 픽셀 해상도 (위성 PNG 그리드 차원 — 108×54 패치)
export constexpr int WORLD_PIXEL_W = 43200;   // = 108 patch × 400 px
export constexpr int WORLD_PIXEL_H = 21600;   // =  54 patch × 400 px

// 패치 격자 좌표 범위 (worldPatch-XXX.png 파일명 그리드)
export constexpr int PATCH_X_MIN = -54;
export constexpr int PATCH_X_MAX =  53;
export constexpr int PATCH_Y_MIN = -27;
export constexpr int PATCH_Y_MAX =  26;

// 파생: 월드 전체 타일 차원
export constexpr int WORLD_TILE_W  = WORLD_PIXEL_W * TILE_PER_PIXEL;       // 1,036,800
export constexpr int WORLD_TILE_H  = WORLD_PIXEL_H * TILE_PER_PIXEL;       //   518,400
export constexpr int WORLD_CHUNK_W = WORLD_TILE_W / CHUNK_SIZE_X;          //    43,200

// 파생: 픽셀 좌표 (0,0) ↔ 타일 좌표 변환 원점 (좌상단 패치 기준)
export constexpr int TILE_BASE_X = PATCH_X_MIN * PIXEL_PER_PATCH * TILE_PER_PIXEL; // -518,400
export constexpr int TILE_BASE_Y = PATCH_Y_MIN * PIXEL_PER_PATCH * TILE_PER_PIXEL; // -259,200

// 렌더 X 공간 (= WORLD_TILE_W × 16) — Coord 저장 좌표계 단위 (1 타일 = 16 렌더 유닛)
export constexpr int RENDER_X_SPAN = WORLD_TILE_W * 16;
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