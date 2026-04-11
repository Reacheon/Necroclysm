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
import log;
import turnWait;

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
	for (auto& sd : PlayerInfo().skillList)
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

	// 토글 타입: execute 대신 토글 상태 전환
	if (behavior->type == skillType::TOGGLE)
	{
		skillDataPtr->toggle = !skillDataPtr->toggle;
		if (skillDataPtr->toggle)
			behavior->onToggleOn(caster, *skillDataPtr);
		else
			behavior->onToggleOff(caster, *skillDataPtr);
		return;
	}

	// 실패율 판정 (reqProfic이 있는 스킬만 실패 가능)
	if (!behavior->reqProfic.empty())
	{
		int failRate = behavior->calcFailRate(caster);
		if (failRate > 0 && randomRange(1, 100) <= failRate)
		{
			updateLog(L"You fail to use " + behavior->name + L".");
			turnWait(1.0);
			return;
		}
	}

	currentUsingSkill = skillCode;
	Corouter::start(behavior->execute(caster, *skillDataPtr));
}
