export module SkillRegistry;

import std;
import SkillBehavior;
import util;

export class SkillRegistry
{
	static inline std::unordered_map<std::wstring, std::unique_ptr<SkillBehavior>> behaviors;

public:
	static void registerSkill(std::unique_ptr<SkillBehavior> skill)
	{
		std::wstring id = skill->id;
		errorBox(id.empty(), L"[registerSkill] ID가 정의되지 않은 스킬이 등록되었다.");
		errorBox(behaviors.contains(id), L"[registerSkill] 이미 ID가 등록된 스킬이 또 추가되었다. 두 스킬이 중복된 ID를 가지고 있다.");
		behaviors[id] = std::move(skill);
	}

	static SkillBehavior* get(const std::wstring& skillId)
	{
		auto it = behaviors.find(skillId);
		return (it != behaviors.end()) ? it->second.get() : nullptr;
	}

	static std::vector<std::wstring> getAllIds()
	{
		std::vector<std::wstring> ids;
		ids.reserve(behaviors.size());
		for (const auto& [id, _] : behaviors) ids.push_back(id);
		std::sort(ids.begin(), ids.end());
		return ids;
	}

	static void init();
};
