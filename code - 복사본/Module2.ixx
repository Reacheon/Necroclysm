#include <SDL.h>
#include <SDL_ttf.h>

#define �� { 0xFF,0xF8,0xED }
#define Ȳȥ { 0x79,0x87,0xff}
#define ���� { 0xFF,0xB6,0x76 }

export module globalVar;

import std;
import util;
import constVar;
import ItemData;
import EntityData;
import Ani;
import AlchemyData;

/////////////////////////////////////////////////////////////////
export namespace actSet
{
    std::vector<act> null = { act::status, act::ability, act::inventory, act::bionic, act::talent, act::runMode, act::test, act::mutation, act::identify, act::craft, act::alchemy, act::god, act::map };
    std::vector<act> lootPart = { act::pick, act::wield, act::equip, act::eat };
    std::vector<act> vehicle = { act::turnLeft, act::wait, act::turnRight, act::startEngine, act::shiftGear,act::brake, act::accel, act::test };
    std::vector<act> helicopter = { act::collectiveLever, act::wait, act::cyclicLever, act::startEngine, act::rpmLever, act::tailRotorPedalL, act::tailRotorPedalR };
    std::vector<act> train = { act::rpmLever, act::wait, act::brake, act::startEngine, act::blank,act::blank,act::blank };
};
export namespace option
{
    std::wstring language = L"Korean";// �Ǵ� "English"
};

export std::vector<std::wstring> sysStr;

export namespace timer
{
    int timer600 = 0;
    int cursorHightlight = 0;
};

/////////////////////////////////////////////////////////////////
export SDL_Window* window;//������ ���� ������
export SDL_Renderer* renderer;//������ ���� ������

auto aniUSetComp = [](Ani* a, Ani* b) -> bool {

    if (a->getAniPriority() == b->getAniPriority()) return a < b;
    else if (a->getAniPriority() > b->getAniPriority()) return true;
    else return false;
    };
export std::set<Ani*, decltype(aniUSetComp)> aniUSet;//�ִϸ��̼� ���� �ؽü�, �ش� �ִϸ��̼��� �켱�� ������� ���ĵ�
//export std::unordered_map<std::wstring, SDL_Texture*> textCache;// drawText���� ���ڿ� ���� ĳ��
//export std::unordered_map<std::wstring, SDL_Texture*> textOutlineCache;// drawText���� ���ڿ� ���� ĳ��
export turn turnCycle = turn::playerInput;//0:�÷��̾� �Է�_1:�÷��̾� �ִϸ��̼� ���_2:��� ��ƼƼ AI �۵�(�ϳ��� false ��ȯ�� 3����, ������ 0����)_3:��ƼƼ �ִϸ��̼� ���
export bool quit = false;// true�� ��� ������ �����Ŵ
export bool stopLog = false; // �α׸� �����. �ð��� ������ ������� ����
export float timeGift = 0; // ������ �ൿ�� ���� ��ƼƼ�鿡�� �־����� �ð�


export std::vector<EntityData> entityDex; // Entity DB
export std::vector<ItemData> itemDex;// ������ DB
export std::vector<AlchemyData> alchemyDex; //���ݼ� ���� DB
export std::vector<std::wstring> itemTooltip;
export std::vector<std::wstring> entityTooltip;

export std::vector<std::array<int, effectDexWidth>> effectDex; // ����Ʈ DB

export std::map<std::wstring, void*, std::greater<std::wstring>> StickerList; // ȭ�鿡 �����Ǵ� �ؽ�ó ��ü�� ���� ��
export int cameraW = 304; // ī�޶��� Width
export int cameraH = 244; // ī�޶��� Height
export bool cameraFix = true; //ī�޶� �÷��̾ ����
export int cameraX = 0; // ī�޶��� X ��ǥ(�������)
export int cameraY = 0; // ī�޶��� Y ��ǥ(�������)
//���Ⱑ üũ����Ʈ
export float zoomScale = 3.0; // �� ����, 2.0���� ����
export SDL_Event event; // SDL �̺�Ʈ
export act UIType = act::null; // 0 : �Ϲ�, 1 :����, 2: �κ��丮
export bool click = false; // ���� �÷��̾ ȭ���� ������ �ִ��� ��Ÿ��
export SDL_Point clickTile = { 0,0 }; // ���� �÷��̾ ��ġ�ϰ� �ִ� Ÿ��(�׸��� ���� ������ǥ)
export std::array<int, 3> lootTile = { 0,0,0 };
////////////////////////////////////////////////////////////////////////////////////////////////////////
export int detailScroll = 0; //��� �������� ��ũ��
export input inputType = input::mouse; //���۹�� ����
export SDL_Point clickDownPoint = { 0,0 }; //�ٿ� �̺�Ʈ�� ������ ��ǥ
export SDL_Point clickUpPoint = { 0,0 }; //�� �̺�Ʈ�� ������ ��ǥ
export bool deactClickUp = false; //true�� ��� Ŭ���� �Լ��� �������� ����
export bool cursorMotionLock = false; //��ũ�� �ൿ�� ���콺�� �Ű��� �� ��ư���� ����ȭ ����
export std::vector<act> barAct = actSet::null; //�ϴܿ� ǥ�õǴ� �ൿ ����Ʈ
export int dxClickStack = 0; //x ��ǥ�� �̵���
export int dyClickStack = 0; //y ��ǥ�� �̵���
export int dtClickStackStart = 0; //Ŭ�� �ð� ���� ������ �ð�
export int dtClickStack = -1; //�� �ð�, �� ���� ������ ���� ������ ���� -1
export TTF_Font* mainFont[maxFontSize] = { nullptr, }; //���� ��Ʈ�� ������(��� ������ �ٲ�� �ٸ� ��Ʈ�� �ٲ�)
export TTF_Font* outlineFont[maxFontSize] = { nullptr, }; //���� ��Ʈ�� ������(��� ������ �ٲ�� �ٸ� ��Ʈ�� �ٲ�)
export int selectMode = 0; // ���ø�� 0�� ��� ��ü����, 1�̸� ���м���
/////////////////////////////////////////////////////////////////
export bool exInput = false; // exInput(�ܺ� �ؽ�Ʈ �Է�)�� �۵� �������� ����
export std::wstring exInputText = L""; // �޽��� �ڽ��� �Է��� ���ڿ�
export int exInputCursor = 0; // �޽��� �ڽ����� ���� ����Ű�� Ŀ��
export bool exInputEditing = false; // ���� �Է��� ���� ������, ���� �ѱ� �ϼ� ������ true��, �� ���� �������� �׸��� ��ġ�� ������
export int exInputIndex = -1; // -1�� �̼���, 0���� �Ҵ�, 0�� ������ ���� ���� �Է�
////////////////////////////////////////////////////////////////
//HUD ���� ��������
export tabFlag tabType = tabFlag::autoAtk;
export SDL_Rect letterbox = { 0, 0, 0, 0 };
export SDL_Rect barButton[35] = { 0, 0, 0, 0 };
export SDL_Rect letterboxInButton[35] = { 0, 0, 0, 0 };
export SDL_Rect letterboxPopUpButton = { 0, 0, 0, 0 };
export int letterboxPopUpRelY = 0;
export SDL_Rect tab = { 0, 0, 0, 0 };
export bool doPopUpSingleHUD = false;
export bool doPopDownHUD = false;

export int barActCursor = -1; //Ű���� �Է� �ÿ� ���Ǵ� ���͹ڽ� Ŀ��, ��Ȱ���� -1

export SDL_Color mainLightColor = ��;
export int mainLightBright = 20;
export int mainLightSight = 0;

/////////////////////�ڷ�ƾ ���� ����///////////////////////////////
export Corouter* coFunc;
export std::wstring coAnswer = L"";
export bool coTurnSkip = false; //true�� ��� �÷��̾� �Ͽ� ���޽� coFunc�� �Ҵ�� �ڷ�ƾ �Լ��� �����Ŵ, �� �ڷ�ƾ�Լ� ���߿� ���� ��ŵ�ϴ� ����� �� �� ����

/////////////////////////////////////////////////////////////////
export bool isLeftHanded = false;

//fac, tile, wall, ceil(���� �߰������� ��ü����)
export std::vector<int> canBuildListWall = { 1, 2 }; //������, �뺮
export std::vector<int> canBuildListFloor = { 1 }; //�����ٴ�
export std::vector<int> canBuildListCeil = { 1 }; //����õ��
export std::vector<int> canBuildListFac = { 1, 2, 3 }; //���� ö ������, ����, ���

export void* availableRecipe;

export bool vehicleMode = false;
export void* ctrlVeh = nullptr;

export std::map < dir16, std::unordered_map<std::array<int, 2>, std::array<int, 2>, decltype(arrayHasher2)>> coordTransform;//��ǥ��ȯ

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