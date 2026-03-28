module SkillRegistry;

import std;
import SkillBehavior;
import SkillFireStorm;
import SkillRoll;
import SkillLeap;

void SkillRegistry::init()
{
	registerSkill(std::make_unique<SkillFireStorm>());
	registerSkill(std::make_unique<SkillRoll>());
	registerSkill(std::make_unique<SkillLeap>());
}
