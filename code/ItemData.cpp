import ItemData;
import ItemPocket;
import std;
import util;
import constVar;

ItemData::ItemData() = default;
ItemData::~ItemData() = default;
ItemData::ItemData(ItemData&&) noexcept = default;


bool ItemData::checkFlag(itemFlag inputFlag) const
{
    if (std::find(flag.begin(), flag.end(), inputFlag) == flag.end()) { return false; }
    else { return true; }
}

void ItemData::addFlag(itemFlag inputFlag)
{
    if (!checkFlag(inputFlag)) { flag.push_back(inputFlag); }
}

void ItemData::eraseFlag(itemFlag inputFlag)
{
    std::erase(flag, inputFlag);
}

ItemData ItemData::cloneForTransfer(int transferNumber) const
{
    ItemData newItem;

    static_cast<ItemDataBase&>(newItem) = static_cast<const ItemDataBase&>(*this);

    newItem.number = transferNumber;
    newItem.lootSelect = 0;
    newItem.pocketVolume = 0;
    newItem.reactSelect = 0;
    newItem.equipState = equipHandFlag::none;
    newItem.modeCurrent = 0;
    newItem.unfinishItemCode = 0;
    newItem.unfinishPercent = 100;
    newItem.propHP = newItem.propMaxHP;
    newItem.propFakeHP = static_cast<float>(newItem.propMaxHP);

    newItem.eraseFlag(itemFlag::GRAYFILTER);
    // if (destinationType == storageType::equip) { newItem.equipState = equipHandFlag::normal; }

    return newItem;
}

ItemData cloneFromItemDex(ItemData& inputData, int transferNumber)
{
    ItemData newItem;

    errorBox(transferNumber <= 0, L"Number to add must be positive in addItemFromDex.");

    ItemData& itemTemplate = inputData;

    newItem = itemTemplate.cloneForTransfer(1);
    newItem.codeID = genItemID();

    // 해당 아이템이 포켓을 가질 경우
    if (itemTemplate.pocketMaxVolume > 0 || itemTemplate.pocketMaxNumber > 0)
    {
        errorBox(transferNumber >= 2, L"cloneFromItemDex에서 포켓이 있는 아이템을 2개 이상 복사하려고 하였다.");
        newItem.pocketPtr = std::make_unique<ItemPocket>(storageType::pocket);
    }
    else
    {
        newItem.number = transferNumber;
    }

    return newItem;
}

bool ItemData::isPocketOnlyItem(int inputCode)
{
    for (int i = 0; i < pocketOnlyItem.size(); i++)
    {
        if (pocketOnlyItem[i] == inputCode) return true;
    }
    return false;
}


bool ItemData::itemOverlay(const ItemData& tgtItem) const
{
    if (checkFlag(itemFlag::NONSTACK) || tgtItem.checkFlag(itemFlag::NONSTACK)) { return false; }

    if (lightPtr != nullptr || pocketPtr != nullptr) return false;

    if (name != tgtItem.name) { return false; }
    if (itemCode != tgtItem.itemCode) { return false; }
    if (sprIndex != tgtItem.sprIndex) { return false; }
    if (tooltipIndex != tgtItem.tooltipIndex) { return false; }
    if (category != tgtItem.category) { return false; }
    if (subcategory != tgtItem.subcategory) { return false; }
    if (weight != tgtItem.weight) { return false; }
    if (volume != tgtItem.volume) { return false; }
    if (material != tgtItem.material) { return false; }
    if (toolQuality != tgtItem.toolQuality) { return false; }
    if (recipe != tgtItem.recipe) { return false; }
    if (recipeQualityNeed != tgtItem.recipeQualityNeed) { return false; }
    if (recipeProficNeed != tgtItem.recipeProficNeed) { return false; }
    if (disassy != tgtItem.disassy) { return false; }
    if (disassyQualityNeed != tgtItem.disassyQualityNeed) { return false; }
    if (repair != tgtItem.repair) { return false; }
    if (repairQualityNeed != tgtItem.repairQualityNeed) { return false; }
    if (repairProficNeed != tgtItem.repairProficNeed) { return false; }
    if (pocketMaxVolume != tgtItem.pocketMaxVolume) { return false; }
    if (pocketOnlyItem != tgtItem.pocketOnlyItem) { return false; }

    //방어 계열 변수
    if (rPierce != tgtItem.rPierce) { return false; }
    if (rCut != tgtItem.rCut) { return false; }
    if (rBash != tgtItem.rBash) { return false; }
    if (enc != tgtItem.enc) { return false; }
    if (sh != tgtItem.sh) { return false; }
    if (ev != tgtItem.ev) { return false; }
    if (rFire != tgtItem.rFire) { return false; }
    if (rCold != tgtItem.rCold) { return false; }
    if (rElec != tgtItem.rElec) { return false; }
    if (rCorr != tgtItem.rCorr) { return false; }
    if (rRad != tgtItem.rRad) { return false; }
    if (bionicIndex != tgtItem.bionicIndex) { return false; }

    if (pocketVolume != tgtItem.pocketVolume) { return false; }
    //if (number != tgtItem.number) { return false; }
    //if (select != tgtItem.select) { return false; }
    if (equipState != tgtItem.equipState) { return false; }

    return true;
}

