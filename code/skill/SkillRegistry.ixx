export module SkillRegistry;

import std;
import SkillBehavior;

export class SkillRegistry
{
	static inline std::unordered_map<int, std::unique_ptr<SkillBehavior>> behaviors;

public:
	static void registerSkill(std::unique_ptr<SkillBehavior> skill)
	{
		int code = skill->getSkillCode();
		behaviors[code] = std::move(skill);
	}

	static SkillBehavior* get(int skillCode)
	{
		auto it = behaviors.find(skillCode);
		return (it != behaviors.end()) ? it->second.get() : nullptr;
	}

	static void init();
};
