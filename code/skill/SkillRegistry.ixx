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

	// 등록된 모든 스킬 코드를 오름차순으로 반환. 디버그 리스팅 용도.
	static std::vector<int> getAllCodes()
	{
		std::vector<int> codes;
		codes.reserve(behaviors.size());
		for (const auto& [code, _] : behaviors) codes.push_back(code);
		std::sort(codes.begin(), codes.end());
		return codes;
	}

	static void init();
};
