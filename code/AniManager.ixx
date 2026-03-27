export module AniManager;

import std;
import constVar;
import Ani;

export class AniManager
{
private:
	std::set<Ani*, bool(*)(Ani*, Ani*)> aniSet;

public:
	AniManager() : aniSet(
		[](Ani* a, Ani* b) -> bool {
			if (a->getAniPriority() == b->getAniPriority())
				return a < b;
			else
				return a->getAniPriority() > b->getAniPriority();
		}
	) {}

	//애니메이션을 추가한다. 단 턴을 넘기지는 않는다.
	void add(Ani* tgtPtr, aniFlag inputType)
	{
		aniSet.insert(tgtPtr);
		tgtPtr->setAniType(inputType);
	}

	//애니메이션을 제거한다. (shutdown 등 강제 종료용)
	void remove(Ani* tgtPtr)
	{
		auto it = aniSet.find(tgtPtr);
		if (it != aniSet.end())
			aniSet.erase(it);
	}

	//해당 Ani가 현재 애니메이션 중인지 확인
	bool contains(Ani* tgtPtr)
	{
		return aniSet.find(tgtPtr) != aniSet.end();
	}

	bool empty() { return aniSet.empty(); }
	int size() { return (int)aniSet.size(); }

	auto begin() { return aniSet.begin(); }
	auto end() { return aniSet.end(); }
	auto erase(std::set<Ani*, bool(*)(Ani*, Ani*)>::iterator it) { return aniSet.erase(it); }
};
