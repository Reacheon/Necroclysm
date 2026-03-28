export module useSkill;

import std;
import util;
import constVar;
import globalVar;
import Player;
import Entity;
import SkillData;
import SkillBehavior;
import SkillRegistry;

export void useSkill(int skillCode)
{
	PlayerPtr->deactAStarDst();
	if (turnCycle != turn::playerInput) return;

	auto* behavior = SkillRegistry::get(skillCode);
	if (!behavior)
	{
		if (skillCode == 0 || skillCode == 1) return; // 빈 슬롯
		std::wstring errorMsg = replaceStr(L"Player used an unknown skill: %d", L"%d", std::to_wstring(skillCode));
		errorBox(errorMsg);
		return;
	}

	// skillList에서 해당 스킬 데이터 찾기
	SkillData* skillDataPtr = nullptr;
	for (auto& sd : PlayerPtr->entityInfo.skillList)
	{
		if (sd.skillCode == skillCode)
		{
			skillDataPtr = &sd;
			break;
		}
	}
	if (!skillDataPtr) return;

	Entity* caster = static_cast<Entity*>(PlayerPtr);
	if (!behavior->canUse(caster, *skillDataPtr)) return;

	currentUsingSkill = skillCode;
	Corouter::start(behavior->execute(caster, *skillDataPtr));
}
