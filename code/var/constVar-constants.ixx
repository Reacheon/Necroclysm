export module constVar:constants;

import std;

export constexpr int DARK_VISION_RADIUS = 13;

export constexpr double MAX_ZOOM = 5.0;
export constexpr double MIN_ZOOM = 2.0;

export constexpr int CHUNK_SIZE_X = 13; //청크의 x길이
export constexpr int CHUNK_SIZE_Y = 13; //청크의 y길이
export constexpr int CHUNK_SIZE_Z = 1; //청크의 z길이, 현재 1로 미사용됨
export constexpr int MAX_FONT_SIZE = 50;
export constexpr int EX_INPUT_TEXT_MAX = 30;
export constexpr int EQUIP_ITEM_MAX = 10;
export constexpr int LOOT_ITEM_MAX = 9;
export constexpr int INVENTORY_ITEM_MAX = 9;
export constexpr int DMG_FLAG_SIZE = 3;
export constexpr int TALENT_SIZE = 20;

// 13칸이 한 청크니까 걸칠 수 있는 정도로, 차량의 왼쪽끝(코어)이 청크의 오른쪽끝 그러면... 7청크*7청크 분석해야한다.
// ●|○○○○○○○○○○○○○|○○○○○○○○○○○○○|○○○○×××××××××|
export constexpr int MAX_VEHICLE_SIZE = 31;


export constexpr int CHUNK_LOADING_RANGE = 5;
export constexpr int MINIMAP_DIAMETER = 41; //미니맵의 지름 (홀수)
export constexpr int NAVIMAP_WIDTH = 99;//167;
export constexpr int NAVIMAP_HEIGHT = 58;//99;
export constexpr int SECTOR_SIZE = 400; // 절차적 맵 생성할 때 작용하는 범위
export constexpr int TOLERANCE_LSTICK = 10000; //LStick이 이 값을 넘어야 판정이 일어남
export constexpr int TOLERANCE_HOLD_DEL_XY = 20; //이 값 이상 움직일 경우 홀드 이벤트가 일어나지 않음

export constexpr int MARKER_LIMIT_DIST = 100; //플레이어로부터 100칸 이상 떨어지면 마커가 그려지지 않음

export constexpr int MAX_PROFIC_LEVEL = 27;

export constexpr int SKILL_GUI_MAX = 7;
export constexpr int QUICK_SLOT_MAX = 8;

export constexpr int CRAFT_MAX_ROW = 4;

export constexpr int MAX_ENC = 10; //최대 방해도
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


export constexpr int PLAYER_MAX_CALORIE = 17280;//10일 버팀
export constexpr int PLAYER_HUNGRY_CALORIE = 4320 * 3;
export constexpr int PLAYER_VERY_HUNGRY_CALORIE = 4320 * 2;
export constexpr int PLAYER_STARVE_CALORIE = 4320;

export constexpr float HUNGRY_SPPED = 1.2;


export constexpr int PLAYER_MAX_HYDRATION = 8640;//4일 버팀
export constexpr int PLAYER_THIRSTY_HYDRATION = 2160 * 3;
export constexpr int PLAYER_VERY_THIRSTY_HYDRATION = 2160 * 2;
export constexpr int PLAYER_DEHYDRATION_HYDRATION = 2160 * 1;

export constexpr float THIRST_SPEED = 1.5;


export constexpr int PLAYER_MAX_FATIGUE = 4320;//최대 3일 안 자고 버팀
export constexpr int PLAYER_TIRED_FATIGUE = 3360;//16시간 지나면 피곤해짐
export constexpr int PLAYER_VERY_TIRED_FATIGUE = 2400;
export constexpr int PLAYER_EXHAUSTED_FATIGUE = 1440;

export constexpr float FATIGUE_SPEED = 1.0;

export constexpr int MAX_BATCH = 4096;

export constexpr int MAX_CIRCUIT_LOOP_COUNT = 50; //전자회로가 트랜지스터 게이트로 인해 중복 업데이트될 때의 한계값