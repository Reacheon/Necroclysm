export module ItemData;

import std;
import util;
import constVar;

//__int8 : -128~127
//__int16 : -32768 ~32767
//__int32 : –2,147,483,648 ~2,147,483,647

class ItemPocket;

export struct ItemData
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
    std::vector<std::pair<unsigned __int8, unsigned __int8>> recipeTalentNeed;


    std::vector<std::pair<unsigned __int16, unsigned __int32>> disassy;
    std::vector<unsigned __int8> disassyQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> disassyTalentNeed;

    std::vector<std::pair<unsigned __int16, unsigned __int32>> repair;
    std::vector<unsigned __int8> repairQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> repairTalentNeed;


    unsigned __int32 pocketMaxVolume = 0;
    unsigned __int32 pocketMaxNumber = 0;
    std::vector<unsigned __int16> pocketOnlyItem;

    std::vector<std::pair<unsigned __int8, __int16>> rPierce;
    std::vector<std::pair<unsigned __int8, __int16>> rCut;
    std::vector<std::pair<unsigned __int8, __int16>> rBash;
    std::vector<std::pair<unsigned __int8, __int16>> enc;
    __int16 sh = 0;
    __int16 ev = 0;
    __int16 rFire = 0;
    __int16 rCold = 0;
    __int16 rElec = 0;
    __int16 rCorr = 0;
    __int16 rRad = 0;
    unsigned __int16 bionicIndex = -1;

    __int16 durablility = 10;

    __int16 pierce = 0;
    __int16 cut = 0;
    __int16 bash = 0;

    __int16 bulletPierce = 1;
    __int16 bulletCut = 1;
    __int16 bulletBash = 1;
    float bulletStoppingPower = 0;
    unsigned __int16 bulletJam = 1;
    unsigned __int8 bulletType = 0;
    unsigned __int8 bulletRange = 1;

    float gunDmg = 0.00;
    float gunAccInit = 0.5;
    unsigned __int8 gunAccSpd = 100;
    float gunAccMax = 0.99;
    unsigned __int8 gunRebound = 5;
    float gunShotSpd = 1.0;
    float gunReloadSpd = 1.0;
    float gunJam = 0.001;
    std::vector<unsigned __int16> gunMod;
    float gunBalance = 0.2;
    unsigned __int8 gunRange = 1;

    float meleeAtkSpd = 0.5;
    float meleeAtkAcc = 0.5;
    float meleeBalance = 0.5;
    unsigned __int8 meleeRange = 1;

    unsigned __int8 modeTemplate = 0;

    __int16 throwAtk = 1;
    unsigned __int8 throwType = 3; //0:무속성, 1:관통, 2:절단, 3:타격
    float throwAtkAcc = 0.3;
    float throwBalance = 0.3;
    unsigned __int8 throwRange = 5;
    float throwStoppingPower = 0.005;

    unsigned __int16 bookIndex = 0; //책 파일의 이름명, 0이면 작동하지 않음(책이 아닌걸로 취급), 1이면 1.txt를 읽어냄

    unsigned __int16 propSprIndex = 0; //아이템아이콘 말고 실제 Vehicle로 설치되었을 때의 이미지 인덱스

    void* equipSpr = nullptr;
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

    //liqColorR	liqColorG	liqColorB	gasColorR	gasColorG	gasColorB	gasAlphaMax	vaporP

    unsigned __int16 molecularWeight = 1;
    unsigned __int8 liqColorR = 255;
    unsigned __int8 liqColorG = 255;
    unsigned __int8 liqColorB = 255;
    unsigned __int8 gasColorR = 255;
    unsigned __int8 gasColorG = 255;
    unsigned __int8 gasColorB = 255;
    unsigned __int8 gasAlphaMax = 0; //가스연기의 짙은 정도, 만약 무색의 기체의 경우 값은 0

    ///////////////////비데이터베이스 변수////////////////////

    unsigned __int32 pocketVolume = 0;
    unsigned __int32 number = 1;
    unsigned __int32 lootSelect = 0;
    unsigned __int32 reactSelect = 0;
    void* pocketPtr = nullptr;
    unsigned __int8 equipState = 0;
    unsigned __int8 modeCurrent = 0;

    unsigned __int16 unfinishItemCode = 0;
    unsigned __int8 unfinishPercent = 100;

    unsigned __int8 extraSprIndexSingle = 0; //벽의 연결로 추가되는 sprIndex(일단은 차량만 구현)
    unsigned __int8 extraSprIndex16 = 0; //16단위로 움직이는 추가 sprIndex

    unsigned __int16 dirChangeItemCode = 0;
    
    dir16 propDir16 = dir16::dir0;

    void* lightPtr = nullptr;

    bool checkFlag(itemFlag inputFlag)
    {
        if (std::find(flag.begin(), flag.end(), inputFlag) == flag.end()) { return false; }
        else { return true; }
    }

    //중복 비허용
    void addFlag(itemFlag inputFlag)
    {
        if (checkFlag(inputFlag) == false) { flag.push_back(inputFlag); }
    }

    void eraseFlag(itemFlag inputFlag)
    {
        if (checkFlag(inputFlag) == true) flag.erase(std::find(flag.begin(), flag.end(), inputFlag));
    }
};

export bool itemOverlay(ItemData& a, ItemData& b)
{
    if (a.checkFlag(itemFlag::NONSTACK) == true || b.checkFlag(itemFlag::NONSTACK) == true) { return false; }

    //////////////////////////////////////////////////////////////////////////

    if (a.name != b.name) { return false; }

    if (a.itemCode != b.itemCode) { return false; }
    if (a.sprIndex != b.sprIndex) { return false; }
    if (a.tooltipIndex != b.tooltipIndex) { return false; }
    if (a.category != b.category) { return false; }
    if (a.subcategory != b.subcategory) { return false; }
    if (a.weight != b.weight) { return false; }
    if (a.volume != b.volume) { return false; }
    if (a.material != b.material) { return false; }

    if (a.toolQuality != b.toolQuality) { return false; }

    if (a.recipe != b.recipe) { return false; }
    if (a.recipeQualityNeed != b.recipeQualityNeed) { return false; }
    if (a.recipeTalentNeed != b.recipeTalentNeed) { return false; }

    if (a.disassy != b.disassy) { return false; }
    if (a.disassyQualityNeed != b.disassyQualityNeed) { return false; }

    if (a.pocketMaxVolume != b.pocketMaxVolume) { return false; }
    if (a.pocketOnlyItem != b.pocketOnlyItem) { return false; }

    //방어 계열 변수
    if (a.rPierce != b.rPierce) { return false; }
    if (a.rCut != b.rCut) { return false; }
    if (a.rBash != b.rBash) { return false; }
    if (a.enc != b.enc) { return false; }
    if (a.sh != b.sh) { return false; }
    if (a.ev != b.ev) { return false; }
    if (a.rFire != b.rFire) { return false; }
    if (a.rCold != b.rCold) { return false; }
    if (a.rElec != b.rElec) { return false; }
    if (a.rCorr != b.rCorr) { return false; }
    if (a.rRad != b.rRad) { return false; }

    if (a.bionicIndex != b.bionicIndex) { return false; }

    if (a.pocketVolume != b.pocketVolume) { return false; }
    //if (a.number != b.number) { return false; }
    //if (a.select != b.select) { return false; }
    if (a.pocketPtr != b.pocketPtr) { return false; }
    if (a.equipState != b.equipState) { return false; }

    return true;
}
