module;

#include <SDL3/SDL.h>

export module textureVar;

import std;
import Sprite;

export namespace texture
{
	SDL_Texture* minimap = nullptr;
	SDL_Texture* worldmap = nullptr;
	SDL_Texture* navimap = nullptr;
	SDL_Texture* mainGaugeWhiteShadow = nullptr;
	SDL_Texture* hpGaugeWhiteShadow = nullptr;
	SDL_Texture* circuitInfo = nullptr;

	std::array<SDL_Texture*, 10> shoreSpline = { nullptr, };
	// 47타일의 오토타일 스프라이트 시트
	// 384px*288px의 파일, 한 오토타일 당 48px*48px
	// 한 행에 8개의 타일로 총 6개의 행으로 구성 (마지막 칸은 공백)
	// 나중에는 복수 갯수로 확장해서 경계의 다양성을 추구할 예정
	// #3899ff -> 물,   #5b4940 -> 땅
	// 비트마스크 값 오름차순, 첫번째는 흙채우기 없는 정사각형 물타일, 마지막 타일은 주변이 흙으로 둘러쌓인 물 타일
}

export namespace spr
{
	std::unordered_map<std::wstring, Sprite*> spriteMapper;

	Sprite* charsetHero = nullptr;
	Sprite* defaultMonster = nullptr;
	Sprite* effectBash1 = nullptr;
	Sprite* effectCut1 = nullptr;
	Sprite* effectCut2 = nullptr;
	Sprite* effectPierce1 = nullptr;
	Sprite* effectBite1 = nullptr;
	Sprite* effectClaw1 = nullptr;
	Sprite* icon13 = nullptr;
	Sprite* icon48 = nullptr;
	Sprite* batteryGauge = nullptr;
	Sprite* ecliptic = nullptr;
	Sprite* weather = nullptr;
	Sprite* weatherCloud = nullptr;
	Sprite* itemset = nullptr;
	Sprite* colorPaletteOption = nullptr; //16x16 타일. 염색앰플 등 색상 선택 UI용 컬러 칩 시트
	Sprite* windowArrow = nullptr;
	Sprite* whiteMarker = nullptr;
	Sprite* yellowMarker = nullptr;
	Sprite* itemCursorLong = nullptr;
	Sprite* itemCursorShort = nullptr;
	Sprite* lootBagArrow = nullptr;
	Sprite* icon16 = nullptr;
	Sprite* guideBtn = nullptr;
	Sprite* buttons = nullptr;
	Sprite* buttonsPressed = nullptr;
	Sprite* menuPopUp = nullptr;
	Sprite* letterboxBtnMarker = nullptr;
	Sprite* proficIcon = nullptr;
	Sprite* proficIconGold = nullptr;
	Sprite* bionicSkeleton = nullptr;
	Sprite* bionicSlotGauge = nullptr;
	Sprite* mutationIcon = nullptr;
	Sprite* staminaGauge = nullptr;
	Sprite* segment = nullptr;
	Sprite* tileset = nullptr;
	Sprite* propset = nullptr;
	Sprite* vehset = nullptr; //차량 부품 전용 아틀라스(과거엔 propset 공용 -> 혼동 방지 위해 분리). 부품은 vehSprIndex로 색인
	Sprite* mapset1by1 = nullptr; //월드맵 1x1 청크 심볼 (48px 셀 = 3x3 청크, 심볼 art는 중앙 16px). 건물·도로·산 1칸
	Sprite* mapset2by2 = nullptr; //월드맵 2x2/2x1/1x2 청크 심볼 (64px 셀 = 4x4 청크). 공원·마트·학교·경찰서·소방서·산 2x2
	Sprite* icon32 = nullptr;
	Sprite* ring24 = nullptr;
	Sprite* bloodM = nullptr;
	Sprite* aimMarker = nullptr;
	Sprite* aimMarkerTmp = nullptr;
	Sprite* aimAtkTypeMarker = nullptr;
	Sprite* tab = nullptr;
	Sprite* aimLRChange = nullptr;
	Sprite* epsilonFont = nullptr;
	Sprite* loadingAnime = nullptr;
	Sprite* buildCursor = nullptr;
	Sprite* msgChoiceBtn = nullptr;
	Sprite* itemSlotBtn = nullptr;
	Sprite* itemSlotPocketArrow = nullptr;
	Sprite* lstSelectBox = nullptr;
	Sprite* beardMustacheBlack = nullptr;
	// 헤어/눈/피부 스프라이트는 각각 image/charset/body/hair/, body/eyes/, body/skin/ 재귀 로드 +
	// palette/hair.tsv / palette/eyes.tsv / palette/skin.tsv 팔레트 스왑으로
	// spr::spriteMapper[L"<stem>_<색>"]에 등록됨
	// (예: L"HAIR_BOB_BLACK", L"EYES_OPEN_BLUE", L"SKIN_MALE_LIGHT"). 전용 포인터 없음.
	Sprite* shadow = nullptr;
	Sprite* vehicleHUD = nullptr;
	Sprite* dashboard = nullptr;
	Sprite* vehicleHUDParts = nullptr;
	Sprite* vehicleHUDSteeringWheel = nullptr;
	Sprite* vehicleActCursor = nullptr;
	Sprite* dirMarker = nullptr;
	Sprite* windowArrow2 = nullptr;
	Sprite* mapHereMarker = nullptr;
	Sprite* screenRain = nullptr;
	Sprite* screenSnow = nullptr;
	Sprite* symbolSunny = nullptr;
	Sprite* symbolCloudy = nullptr;
	Sprite* symbolMoon = nullptr;
	Sprite* symbolRain = nullptr;
	Sprite* symbolStorm = nullptr;
	Sprite* symbolSnow = nullptr;
	Sprite* tailPedalL = nullptr;
	Sprite* tailPedalR = nullptr;
	Sprite* trainBrake = nullptr;
	Sprite* mainRotor = nullptr;
	Sprite* singleQuickSlot = nullptr;
	Sprite* topQuickSlotBtn = nullptr;
	Sprite* skillSet = nullptr;
	Sprite* fireStorm = nullptr;
	Sprite* flameSet = nullptr;
	Sprite* gasEffect1 = nullptr;
	Sprite* steamEffect1 = nullptr;
	Sprite* trail = nullptr;
	Sprite* statusEffectRect = nullptr;
	Sprite* statusIcon = nullptr;
	Sprite* minimapEdge = nullptr;
	Sprite* bulletset = nullptr;
	Sprite* aimMarkerWhite = nullptr;
	Sprite* hornCoverRed = nullptr;
	Sprite* mapMagnifyIcon = nullptr;
	Sprite* staminaGaugeCircle = nullptr;
	Sprite* speechBubble = nullptr;
	Sprite* particle = nullptr;
	Sprite* footprint = nullptr;
	Sprite* craftItemRect = nullptr;
	Sprite* msgBox = nullptr;
	Sprite* skillRect = nullptr;
	Sprite* gridMarker = nullptr;
	Sprite* tabBoxAim = nullptr;

	Sprite* youDied = nullptr;
	Sprite* gameOverOptionRect = nullptr;
	Sprite* gameOverOptionMarker = nullptr;

	Sprite* statusEffectGaugeCircle = nullptr;

	Sprite* thoughtBubble = nullptr;
	Sprite* waveFoam = nullptr;
	
	Sprite* seaFoam = nullptr;
	Sprite* waterFoam = nullptr;

	Sprite* itemBackgroundRect = nullptr;

	Sprite* newWindowArrow = nullptr;

	Sprite* icon28 = nullptr;

	Sprite* downRightLetterbox = nullptr;
	Sprite* mainGauge = nullptr;

	Sprite* bodyShape = nullptr;
	Sprite* hpGauge = nullptr;

	Sprite* icon24 = nullptr;

	Sprite* gamepadInstruction = nullptr;
	Sprite* gamepadButtons = nullptr;
	Sprite* keyboardButtons = nullptr;
	Sprite* icon80 = nullptr;

	Sprite* gearStick = nullptr;

	Sprite* coordCraftBox = nullptr;
	Sprite* coordCraftMarker = nullptr;
	Sprite* fluxArrow = nullptr;

	Sprite* sprinkler33 = nullptr;
	Sprite* sprinkler55 = nullptr;

	Sprite* logBackground = nullptr;

	Sprite* floatGuideLog = nullptr;

	Sprite* fryingPan = nullptr;
	Sprite* cookingPot = nullptr;
	
	Sprite* bodyPartEncLine = nullptr;

	Sprite* btnGuideBackground = nullptr;
	Sprite* statusPortraitBackground = nullptr;

	Sprite* rampUpTile = nullptr;

}
