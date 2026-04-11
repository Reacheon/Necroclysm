module SkillRegistry;

import std;
import SkillBehavior;
import SkillFireStorm;
import SkillRoll;
import SkillLeap;
import SkillSelfHeal;
import SkillHealOther;
import SkillPurify;
import SkillFocusHeal;
import SkillSuperRegen;
import SkillLivingWard;
import SkillNervedrive;
import SkillPowerStorage;
import SkillMetabExchange;
import SkillInfravision;

void SkillRegistry::init()
{
	registerSkill(std::make_unique<SkillFireStorm>());
	registerSkill(std::make_unique<SkillRoll>());
	registerSkill(std::make_unique<SkillLeap>());
	registerSkill(std::make_unique<SkillSelfHeal>());
	registerSkill(std::make_unique<SkillHealOther>());
	registerSkill(std::make_unique<SkillPurify>());
	registerSkill(std::make_unique<SkillFocusHeal>());
	registerSkill(std::make_unique<SkillSuperRegen>());
	registerSkill(std::make_unique<SkillLivingWard>());
	registerSkill(std::make_unique<SkillNervedrive>());
	registerSkill(std::make_unique<SkillPowerStorage>());
	registerSkill(std::make_unique<SkillMetabExchange>());
	registerSkill(std::make_unique<SkillInfravision>());
}
