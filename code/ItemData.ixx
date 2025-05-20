export module ItemData;

import std;
import util;
import constVar;
import Light;

//__int8 : -128~127
//__int16 : -32768 ~32767
//__int32 : –2,147,483,648 ~2,147,483,647

export struct ItemData_Base
{
    std::wstring name = L"DEFAULT ITEM";
    unsigned __int16 itemCode = 1;
    unsigned __int16 sprIndex = 1;
    unsigned __int16 tooltipIndex = 0;
    itemCategory category = itemCategory::material;
    itemSubcategory subcategory = itemSubcategory::material_etc;
    unsigned __int32 weight = 1000;
    unsigned __int32 volume = 1000;
    std::vector<unsigned __int16> material;
    std::vector<itemFlag> flag;
    std::vector<unsigned __int8> toolQuality;
    unsigned __int8 craftMinUnit = 1;
    unsigned __int16 craftTime = 2;
    std::vector<std::pair<unsigned __int16, unsigned __int32>> recipe;
    std::vector<unsigned __int8> recipeQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> recipeProficNeed;
    std::vector<std::pair<unsigned __int16, unsigned __int32>> disassy;
    std::vector<unsigned __int8> disassyQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> disassyProficNeed;
    std::vector<std::pair<unsigned __int16, unsigned __int32>> repair;
    std::vector<unsigned __int8> repairQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> repairProficNeed;
    unsigned __int32 pocketMaxVolume = 0;
    unsigned __int32 pocketMaxNumber = 0;
    std::vector<unsigned __int16> pocketOnlyItem;
    __int16 rPierce = 0;
    __int16 rCut = 0;
    __int16 rBash = 0;
    std::vector<std::pair<unsigned __int8, __int16>> enc;
    __int16 sh = 0;
    __int16 ev = 0;
    __int16 rFire = 0;
    __int16 rCold = 0;
    __int16 rElec = 0;
    __int16 rCorr = 0;
    __int16 rRad = 0;
    unsigned __int16 bionicIndex = static_cast<unsigned __int16>(-1);
    __int16 durablility = 100;
    __int16 pierce = 0;
    __int16 cut = 0;
    __int16 bash = 0;
    __int16 bulletPierce = 1;
    __int16 bulletCut = 1;
    __int16 bulletBash = 1;
    float bulletStoppingPower = 0.0f;
    unsigned __int16 bulletJam = 1;
    unsigned __int8 bulletType = 0;
    unsigned __int8 bulletRange = 1;
    float gunDmg = 0.00f;
    float gunAccInit = 0.5f;
    unsigned __int8 gunAccSpd = 100;
    float gunAccMax = 0.99f;
    unsigned __int8 gunRebound = 5;
    float gunShotSpd = 1.0f;
    float gunReloadSpd = 1.0f;
    float gunJam = 0.001f;
    std::vector<unsigned __int16> gunMod;
    float gunBalance = 0.2f;
    unsigned __int8 gunRange = 1;
    float meleeAtkSpd = 0.5f;
    float meleeAtkAcc = 0.5f;
    float meleeBalance = 0.5f;
    unsigned __int8 meleeRange = 1;
    unsigned __int8 modeTemplate = 0;
    __int16 throwAtk = 1;
    unsigned __int8 throwType = 3; //0:무속성, 1:관통, 2:절단, 3:타격
    float throwAtkAcc = 0.3f;
    float throwBalance = 0.3f;
    unsigned __int8 throwRange = 5;
    float throwStoppingPower = 0.005f;
    unsigned __int16 bookIndex = 0; //책 파일의 이름명, 0이면 작동하지 않음(책이 아닌걸로 취급), 1이면 1.txt를 읽어냄
    unsigned __int16 propSprIndex = 0; //아이템아이콘 말고 실제 Vehicle로 설치되었을 때의 이미지 인덱스
    void* equipSpr = nullptr;
    void* equipSprToggleOn = nullptr;
    unsigned __int32 equipPriority = 0;
    void* leftWieldSpr = nullptr;
    unsigned __int32 leftWieldPriority = 0;
    void* rightWieldSpr = nullptr;
    unsigned __int32 rightWieldPriority = 0;
    unsigned __int16 tileSprIndex = 0;
    __int8 tileConnectGroup = -1; //타일이 그려질 때 주변과 연결 여부 -1:비활성, 0:자신하고만, 1:자연벽, 2:인공벽
    unsigned __int8 lightRange = 0;
    unsigned __int8 lightIntensity = 0;
    unsigned __int8 lightR = 0;
    unsigned __int8 lightG = 0;
    unsigned __int8 lightB = 0;
    __int8 lightDelX = 0;
    __int8 lightDelY = 0;
    __int8 animeSize = 1;
    __int16 animeFPS = 0;
    __int8 randomPropSprSize = 1;
    unsigned __int16 molecularWeight = 1;
    unsigned __int8 liqColorR = 255;
    unsigned __int8 liqColorG = 255;
    unsigned __int8 liqColorB = 255;
    unsigned __int8 gasColorR = 255;
    unsigned __int8 gasColorG = 255;
    unsigned __int8 gasColorB = 255;
    unsigned __int8 gasAlphaMax = 0; //가스연기의 짙은 정도, 만약 무색의 기체의 경우 값은 0
    __int16 propMaxHP = 100; //나무같은 프롭의 HP나 방어상성 설정
    __int8 propRPierce = 0;
    __int8 propRCut = 0;
    __int8 propRBash = 0;

    __int32 codeID = 0; //랜덤으로 결정되는 ID값, Alchemy가 열릴 때 등... 일시적임(0인 경우 미할당)
};


export struct ItemData : public ItemData_Base
{
    ///////////////////비데이터베이스 변수////////////////////
    unsigned __int32 pocketVolume = 0;
    unsigned __int32 number = 1;
    unsigned __int32 lootSelect = 0;
    unsigned __int32 reactSelect = 0;

    std::unique_ptr<ItemPocket> pocketPtr;
    equipHandFlag equipState = equipHandFlag::none;
    unsigned __int8 modeCurrent = 0;

    unsigned __int16 unfinishItemCode = 0;
    unsigned __int8 unfinishPercent = 100;

    unsigned __int8 extraSprIndexSingle = 0; //벽의 연결로 추가되는 sprIndex(일단은 차량만 구현)
    unsigned __int8 extraSprIndex16 = 0; //16단위로 움직이는 추가 sprIndex

    unsigned __int16 dirChangeItemCode = 0;
    dir16 propDir16 = dir16::dir0;

    std::unique_ptr<Light> lightPtr;
    __int16 propHP = 100;
    float propFakeHP = 100.0f;

    bool checkFlag(itemFlag inputFlag) const;

    void addFlag(itemFlag inputFlag);

    void eraseFlag(itemFlag inputFlag);

    ItemData cloneForTransfer(int transferNumber) const;

    bool itemOverlay(const ItemData& tgtItem) const;

    ItemData();
    virtual ~ItemData();
    ItemData(ItemData&&) noexcept;
    ItemData& operator=(ItemData&&) noexcept = default;
    ItemData(const ItemData&) = delete;
    ItemData& operator=(const ItemData&) = delete;

    bool isPocketOnlyItem(int inputCode);
};

export ItemData cloneFromItemDex(ItemData& inputData, int transferNumber);

