export module SkillRegistry;

import std;
import SkillBehavior;

export class SkillRegistry
{
	static inline std::unordered_map<std::wstring, std::unique_ptr<SkillBehavior>> behaviors;

public:
	static void registerSkill(std::unique_ptr<SkillBehavior> skill)
	{
		std::wstring id = skill->id;
		// 빈 ID 또는 중복 등록은 버그. 개발 중 즉시 발견되도록 abort.
		if (id.empty())
		{
			std::wprintf(L"[SkillRegistry] Skill with empty id registered.\n");
			std::abort();
		}
		if (behaviors.contains(id))
		{
			std::wprintf(L"[SkillRegistry] Duplicate skill id: %ls\n", id.c_str());
			std::abort();
		}
		behaviors[id] = std::move(skill);
	}

	static SkillBehavior* get(const std::wstring& skillId)
	{
		auto it = behaviors.find(skillId);
		return (it != behaviors.end()) ? it->second.get() : nullptr;
	}

	// 등록된 모든 스킬 ID를 이름 오름차순으로 반환. 디버그 리스팅 용도.
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
