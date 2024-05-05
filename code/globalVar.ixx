#include <SDL.h>
#include <SDL_ttf.h>

#define 낮 { 0xFF,0xF8,0xED }
#define 황혼 { 0x79,0x87,0xff}
#define 노을 { 0xFF,0xB6,0x76 }

export module globalVar;

import std;
import util;
import constVar;
import ItemData;
import EntityData;
import SkillData;
import Ani;
import AlchemyData;

/////////////////////////////////////////////////////////////////
export namespace actSet
{
    std::vector<act> null = { act::status, act::inventory, act::bionic, act::talent, act::runMode, act::mutation, act::test, act::identify, act::craft, act::alchemy, act::god, act::map };
    std::vector<act> lootPart = { act::pick, act::wield, act::equip, act::eat };
    std::vector<act> vehicle = { act::turnLeft, act::wait, act::turnRight, act::startEngine, act::shiftGear,act::brake, act::accel, act::test };
    std::vector<act> helicopter = { act::collectiveLever, act::wait, act::cyclicLever, act::startEngine, act::rpmLever, act::tailRotorPedalL, act::tailRotorPedalR };
    std::vector<act> train = { act::rpmLever, act::wait, act::brake, act::startEngine, act::shiftGear, act::blank,act::blank };

    std::vector<act> bionicActive= { act::bionicActive, act::bionicToggle, act::bionicQuickSlot, act::bionicInfo };
    std::vector<act> mutationActive = { act::mutationActive, act::mutationToggle, act::mutationQuickSlot, act::mutationInfo };
};
export namespace option
{
    std::wstring language = L"Korean";// 또는 "English"
};

export std::vector<std::wstring> sysStr;

export namespace timer
{
    int timer600 = 0;
    int cursorHightlight = 0;
};

/////////////////////////////////////////////////////////////////
export SDL_Window* window;//게임의 메인 윈도우
export SDL_Renderer* renderer;//게임의 메인 렌더러

auto aniUSetComp = [](Ani* a, Ani* b) -> bool {

    if (a->getAniPriority() == b->getAniPriority()) return a < b;
    else if (a->getAniPriority() > b->getAniPriority()) return true;
    else return false;
    };
export std::set<Ani*, decltype(aniUSetComp)> aniUSet;//애니메이션 저장 해시셋, 해당 애니메이션의 우선도 순서대로 정렬됨
//export std::unordered_map<std::wstring, SDL_Texture*> textCache;// drawText들의 문자열 저장 캐시
//export std::unordered_map<std::wstring, SDL_Texture*> textOutlineCache;// drawText들의 문자열 저장 캐시
export turn turnCycle = turn::playerInput;//0:플레이어 입력_1:플레이어 애니메이션 재생_2:모든 엔티티 AI 작동(하나라도 false 반환시 3으로, 없으면 0으로)_3:엔티티 애니메이션 재생
export bool quit = false;// true일 경우 게임을 종료시킴
export bool stopLog = false; // 로그를 멈춘다. 시간이 지나도 사라지지 않음
export float timeGift = 0; // 유저의 행동에 의해 엔티티들에게 주어지는 시간


export std::vector<EntityData> entityDex; // Entity DB
export std::vector<ItemData> itemDex;// 아이템 DB
export std::vector<AlchemyData> alchemyDex; //연금술 조합 DB
export std::vector<SkillData> skillDex; //스킬 DB

export std::vector<std::wstring> itemTooltip;
export std::vector<std::wstring> entityTooltip;

export std::vector<std::array<int, effectDexWidth>> effectDex; // 이펙트 DB

export std::map<std::wstring, void*, std::greater<std::wstring>> StickerList; // 화면에 고정되는 텍스처 객체에 대한 맵
export int cameraW = 304; // 카메라의 Width
export int cameraH = 244; // 카메라의 Height
export bool cameraFix = true; //카메라를 플레이어에 고정
export int cameraX = 0; // 카메라의 X 좌표(좌측상단)
export int cameraY = 0; // 카메라의 Y 좌표(좌측상단)
//여기가 체크포인트
export float zoomScale = 3.0; // 줌 배율, 2.0부터 시작
export SDL_Event event; // SDL 이벤트
export act UIType = act::null; // 0 : 일반, 1 :루팅, 2: 인벤토리
export bool click = false; // 현재 플레이어가 화면을 누르고 있는지 나타냄
export SDL_Point clickTile = { 0,0 }; // 현재 플레이어가 터치하고 있는 타일(그리드 단위 절대좌표)
export std::array<int, 3> lootTile = { 0,0,0 };
////////////////////////////////////////////////////////////////////////////////////////////////////////
export int detailScroll = 0; //상단 디테일의 스크롤
export input inputType = input::mouse; //조작방식 설정
export SDL_Point clickDownPoint = { 0,0 }; //다운 이벤트를 실행한 좌표
export SDL_Point clickUpPoint = { 0,0 }; //업 이벤트를 실행한 좌표
export bool deactClickUp = false; //true일 경우 클릭업 함수를 실행하지 않음
export bool cursorMotionLock = false; //스크롤 행동시 마우스를 옮겼을 때 버튼들의 색변화 방지
export std::vector<act> barAct = actSet::null; //하단에 표시되는 행동 리스트
export int dxClickStack = 0; //x 좌표의 이동값
export int dyClickStack = 0; //y 좌표의 이동값
export int dtClickStackStart = 0; //클릭 시간 측정 시작한 시간
export int dtClickStack = -1; //총 시간, 단 측정 중이지 않을 때에는 값이 -1
export TTF_Font* mainFont[maxFontSize] = { nullptr, }; //메인 폰트의 사이즈(언어 설정이 바뀌면 다른 폰트로 바뀜)
export TTF_Font* outlineFont[maxFontSize] = { nullptr, }; //메인 폰트의 사이즈(언어 설정이 바뀌면 다른 폰트로 바뀜)
export int selectMode = 0; // 선택모드 0일 경우 전체선택, 1이면 정밀선택
/////////////////////////////////////////////////////////////////
export bool exInput = false; // exInput(외부 텍스트 입력)가 작동 중인지의 여부
export std::wstring exInputText = L""; // 메시지 박스에 입력한 문자열
export int exInputCursor = 0; // 메시지 박스에서 현재 가리키는 커서
export bool exInputEditing = false; // 현재 입력이 수정 중인지, 예로 한글 완성 전에는 true임, 이 값을 바탕으로 그리는 위치도 변동됨
export int exInputIndex = -1; // -1은 미선택, 0부터 할당, 0은 아이템 선택 숫자 입력
////////////////////////////////////////////////////////////////
//HUD 관련 전역변수
export tabFlag tabType = tabFlag::autoAtk;
export SDL_Rect letterbox = { 0, 0, 0, 0 };
export SDL_Rect barButton[35] = { 0, 0, 0, 0 };
export SDL_Rect letterboxInButton[35] = { 0, 0, 0, 0 };
export SDL_Rect letterboxPopUpButton = { 0, 0, 0, 0 };
export int letterboxPopUpRelY = 0;
export SDL_Rect tab = { 0, 0, 0, 0 };
export bool doPopUpSingleHUD = false;
export bool doPopDownHUD = false;

export int barActCursor = -1; //키보드 입력 시에 사용되는 레터박스 커서, 비활성은 -1

export SDL_Color mainLightColor = 낮;
export int mainLightBright = 20;
export int mainLightSight = 0;

/////////////////////코루틴 관련 변수///////////////////////////////
export Corouter* coFunc;
export std::wstring coAnswer = L"";
export bool coTurnSkip = false; //true일 경우 플레이어 턴에 도달시 coFunc에 할당된 코루틴 함수를 실행시킴, 즉 코루틴함수 도중에 턴을 스킵하는 기능을 할 수 있음

/////////////////////////////////////////////////////////////////
export bool isLeftHanded = false;

//fac, tile, wall, ceil(따로 추가적으로 객체없음)
export std::vector<int> canBuildListWall = { 1, 2 }; //나무벽, 흙벽
export std::vector<int> canBuildListFloor = { 1 }; //나무바닥
export std::vector<int> canBuildListCeil = { 1 }; //나무천장
export std::vector<int> canBuildListFac = { 1, 2, 3 }; //차량 철 프레임, 의자, 장롱

export void* availableRecipe;

export bool vehicleMode = false;
export void* ctrlVeh = nullptr;

export std::map < dir16, std::unordered_map<std::array<int, 2>, std::array<int, 2>, decltype(arrayHasher2)>> coordTransform;//좌표변환

export namespace dur
{
	__int64 turnCycle = 0;
	__int64 stepEvent = 0;
	__int64 renderTile = 0;
	__int64 renderSticker = 0;
	__int64 renderUI = 0;
	__int64 renderLog = 0;

    __int64 tile = 0;
    __int64 corpse = 0;
    __int64 item = 0;
    __int64 entity = 0;
    __int64 damage = 0;
    __int64 fog = 0;
    __int64 marker = 0;
}

export int extraCameraLength = 0;

export __int64 nextCameraX = 0;
export __int64 nextCameraY = 0;

export __int64 delCameraX = 0;
export __int64 delCameraY = 0;

export std::set<std::array<int, 2>> extraRenderSet;

export godFlag playerGod = godFlag::none;
export int godPiety = 0;

export std::vector<void*> extraRenderVehList;
export std::vector<void*> extraRenderEntityList;