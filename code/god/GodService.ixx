// ============================================================
// GodService - 신 시스템 서비스 레이어
// ============================================================
// 전역 상태(playerGod, godPiety)를 기반으로
// conduct 디스패치, 신앙도 관리, 스킬 부여/회수를 담당한다.
//
// 사용법:
//   GodService::joinGod(godFlag::rehylion);    // 입교
//   GodService::reportConduct(conductType::USE_NECROMANCY); // 규율 체크
//   GodService::tickTurn();                     // 매 턴 호출
//   GodService::leaveGod();                     // 배교
// ============================================================

export module GodService;

import std;
import constVar;
import globalVar;
import GodBehavior;
import GodRegistry;
import Entity;
import Player;

export class GodService
{
	static inline int penanceCounter = 0;	//참회 카운터
	static inline int turnAccum = 0;		//턴 누적 (자연 감소용)

	static constexpr int INITIAL_PIETY = 15;
	static constexpr int MAX_PIETY = 200;
	static constexpr int PIETY_DECAY_INTERVAL = 340; //자연 감소 간격

public:
	// ─── 입교 ───
	static void joinGod(godFlag god)
	{
		if (playerGod != godFlag::none) leaveGod();

		playerGod = god;
		godPiety = INITIAL_PIETY;
		penanceCounter = 0;
		turnAccum = 0;

		auto* behavior = GodRegistry::get(god);
		if (behavior) behavior->onJoin();

		//초기 신앙도 단계에 해당하는 스킬 부여
		updateSkillsForRank(0, getPietyRank());
	}

	// ─── 배교 ───
	static void leaveGod()
	{
		auto* behavior = GodRegistry::get(playerGod);
		if (!behavior) return;

		//모든 신 스킬 회수
		removeAllGodSkills();

		penanceCounter = behavior->getInitialPenance();
		behavior->onLeave();

		playerGod = godFlag::none;
		godPiety = 0;
		turnAccum = 0;
	}

	// ─── 규율(Conduct) 보고 ───
	// 게임 로직의 각 이벤트 포인트에서 호출
	static void reportConduct(conductType conduct)
	{
		auto* behavior = GodRegistry::get(playerGod);
		if (!behavior) return;

		int delta = behavior->checkConduct(conduct);
		if (delta != 0) changePiety(delta);
	}

	// ─── 매 턴 호출 ───
	static void tickTurn()
	{
		//참회 중이면 징벌 처리
		if (penanceCounter > 0)
		{
			--penanceCounter;
			// TODO: 확률적으로 executeWrath() 호출
			return;
		}

		auto* behavior = GodRegistry::get(playerGod);
		if (!behavior) return;

		behavior->onTurnTick(godPiety);

		//신앙도 자연 감소
		++turnAccum;
		if (turnAccum >= PIETY_DECAY_INTERVAL)
		{
			turnAccum = 0;
			changePiety(-1);
		}
	}

	// ─── 신앙도 변동 ───
	static void changePiety(int delta)
	{
		int oldRank = getPietyRank();
		godPiety = std::clamp(godPiety + delta, 0, MAX_PIETY);
		int newRank = getPietyRank();

		if (oldRank != newRank)
		{
			auto* behavior = GodRegistry::get(playerGod);
			if (behavior)
			{
				behavior->onPietyRankChange(oldRank, newRank);
				updateSkillsForRank(oldRank, newRank);
			}
		}
	}

	// ─── 조회 ───
	static int getPietyRank()
	{
		auto* behavior = GodRegistry::get(playerGod);
		return behavior ? behavior->getPietyRank(godPiety) : 0;
	}

	static bool isUnderPenance() { return penanceCounter > 0; }
	static int getPenance() { return penanceCounter; }

	static GodBehavior* getCurrentGod()
	{
		return GodRegistry::get(playerGod);
	}

private:
	// 신앙도 단계 변경에 따른 스킬 부여/회수
	static void updateSkillsForRank(int oldRank, int newRank)
	{
		auto* behavior = GodRegistry::get(playerGod);
		if (!behavior || !PlayerPtr) return;

		Entity* player = static_cast<Entity*>(PlayerPtr);

		if (newRank > oldRank)
		{
			//단계 상승: 새 단계까지의 스킬 부여
			for (int r = oldRank + 1; r <= newRank; ++r)
			{
				auto it = behavior->rankSkills.find(r);
				if (it != behavior->rankSkills.end())
				{
					for (int skillCode : it->second)
						player->addSkill(skillCode);
				}
			}
		}
		else if (newRank < oldRank)
		{
			//단계 하락: 잃은 단계의 스킬 회수
			for (int r = oldRank; r > newRank; --r)
			{
				auto it = behavior->rankSkills.find(r);
				if (it != behavior->rankSkills.end())
				{
					for (int skillCode : it->second)
						player->removeSkill(skillCode);
				}
			}
		}
	}

	// 모든 신 스킬 회수
	static void removeAllGodSkills()
	{
		auto* behavior = GodRegistry::get(playerGod);
		if (!behavior || !PlayerPtr) return;

		Entity* player = static_cast<Entity*>(PlayerPtr);
		for (auto& [rank, skills] : behavior->rankSkills)
		{
			for (int skillCode : skills)
				player->removeSkill(skillCode);
		}
	}
};
