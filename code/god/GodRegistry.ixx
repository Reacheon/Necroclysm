export module GodRegistry;

import std;
import constVar;
import GodBehavior;

export class GodRegistry
{
	static inline std::unordered_map<int, std::unique_ptr<GodBehavior>> gods;

public:
	static void registerGod(std::unique_ptr<GodBehavior> god)
	{
		int code = static_cast<int>(god->getGodFlag());
		gods[code] = std::move(god);
	}

	static GodBehavior* get(godFlag flag)
	{
		auto it = gods.find(static_cast<int>(flag));
		return (it != gods.end()) ? it->second.get() : nullptr;
	}

	static void init();
};
