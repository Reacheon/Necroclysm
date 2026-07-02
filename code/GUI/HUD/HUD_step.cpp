import HUD;
import constVar;
import globalVar;
import World;
import ItemData;
import Sleep;
import statusEffect;

void HUD::step()
{
    tabType = tabFlag::attackNearby;

    // 도구 장착 시 탭 변경
    {
        std::vector<ItemData>& equipInfo = PlayerEquip()->itemInfo;
        for (const ItemData& eqItem : equipInfo)
        {
            if (eqItem.equipState == equipHandFlag::both)
            {
                if (eqItem.itemCode == itemID::hoe) { tabType = tabFlag::till; break; }
                else if (eqItem.itemCode == itemID::wateringCan) { tabType = tabFlag::water; break; }
            }
        }
    }

    if (GUI::getLastGUI() == this)
    {
        gamepadStep();
        mouseStep();
    }
    if (doPopUpSingleHUD == true)
    {
        executePopUpSingle();
        doPopUpSingleHUD = false;
    }
    else if (doPopDownHUD == true)
    {
        executePopDown();
        doPopDownHUD = false;
    }

    static int hungryDisplayTimer = 0;
    static int thirstDisplayTimer = 0;
    static int fatigueDisplayTimer = 0;
    auto& statInfo = PlayerInfo().statusEffectVec;

    double currentHunger = hunger;
    double currentThirst = thirst;
    double currentFatigue = fatigue;

    // 배고픔 상태 관리 (높을수록 나쁨)
    if (fakeHunger > currentHunger && checkStatusEffect(statInfo, statusEffectFlag::hungry) == false)
    {
        hungryDisplayTimer = 0;
        statInfo.push_back({ statusEffectFlag::hungry, -1 });
    }

    if (fakeHunger >= PLAYER_HUNGRY_PERCENT && checkStatusEffect(statInfo, statusEffectFlag::hungry) == false)
    {
        hungryDisplayTimer = 0;
        statInfo.push_back({ statusEffectFlag::hungry, -1 });
    }
    else if (fakeHunger < PLAYER_HUNGRY_PERCENT && checkStatusEffect(statInfo, statusEffectFlag::hungry) == true && std::abs(fakeHunger - currentHunger) < 0.01)
    {
        if (hungryDisplayTimer == 0) hungryDisplayTimer = 180;
        hungryDisplayTimer--;
        if (hungryDisplayTimer == 0)
        {
            eraseStatusEffect(statInfo, statusEffectFlag::hungry);
        }
    }

    // 갈증 상태 관리
    if (fakeThirst > currentThirst && checkStatusEffect(statInfo, statusEffectFlag::dehydrated) == false)
    {
        thirstDisplayTimer = 0;
        statInfo.push_back({ statusEffectFlag::dehydrated, -1 });
    }

    if (fakeThirst >= PLAYER_THIRSTY_PERCENT && checkStatusEffect(statInfo, statusEffectFlag::dehydrated) == false)
    {
        thirstDisplayTimer = 0;
        statInfo.push_back({ statusEffectFlag::dehydrated, -1 });
    }
    else if (fakeThirst < PLAYER_THIRSTY_PERCENT && checkStatusEffect(statInfo, statusEffectFlag::dehydrated) == true && std::abs(fakeThirst - currentThirst) < 0.01)
    {
        if (thirstDisplayTimer == 0) thirstDisplayTimer = 180;
        thirstDisplayTimer--;
        if (thirstDisplayTimer == 0)
        {
            eraseStatusEffect(statInfo, statusEffectFlag::dehydrated);
        }
    }

    if(getLastGUI() == Sleep::ins() && checkStatusEffect(statInfo, statusEffectFlag::tired) == false)
    {
        fatigueDisplayTimer = 0;
        statInfo.push_back({ statusEffectFlag::tired, -1 });
    }

    if (fakeFatigue >= PLAYER_TIRED_PERCENT && checkStatusEffect(statInfo, statusEffectFlag::tired) == false)
    {
        fatigueDisplayTimer = 0;
        statInfo.push_back({ statusEffectFlag::tired, -1 });
    }
    else if (fakeFatigue < PLAYER_TIRED_PERCENT && checkStatusEffect(statInfo, statusEffectFlag::tired) == true && std::abs(fakeFatigue - currentFatigue) < 0.01 && getLastGUI() != Sleep::ins())
    {
        if (fatigueDisplayTimer == 0) fatigueDisplayTimer = 180;
        fatigueDisplayTimer--;
        if (fatigueDisplayTimer == 0)
        {
            eraseStatusEffect(statInfo, statusEffectFlag::tired);
        }
    }
}