export module GodRehylion;

import std;
import constVar;
import GodBehavior;

export class GodRehylion : public GodBehavior
{
public:
	GodRehylion()
	{
		name = L"Rehylion";
		title = L"the Healer";
		playerTitle = L"Vessel";
		descript = L"A god of healing and medicine, devoted to mending the wounded and guiding the undead to their rightful rest. Followers of Rehylion regard healing others as the greatest virtue.";
		iconIndex = 0;
		altarItemCode = itemID::altarOfRehylion;

		prohibitions = {
			L"Attacking friendly creatures",
			L"Using tools of evil origin",
			L"Necromutation"
		};
		pietyGains = {
			L"Praying (once per day)",
			L"Performing medical treatments",
			L"Waking from restful sleep",
			L"Laying undead to rest",
			L"Converting others through healing"
		};

		rankSkills[0] = { 45 };          // 생체결계 (패시브, 입교 즉시)
		rankSkills[1] = { 40, 41 };      // 자가치료, 타인치료
		rankSkills[2] = { 42 };      // 정화
		rankSkills[3] = { 43 };      // 집중치료
		rankSkills[4] = { 44 };      // 초재생
	}

	godFlag getGodFlag() const override { return godFlag::rehylion; }

	int checkConduct(conductType conduct) const override
	{
		switch (conduct)
		{
		case conductType::USE_NECROMANCY:	return -15;
		case conductType::ATTACK_ALLY:		return -10;
		case conductType::USE_EVIL_ITEM:	return -5;
		case conductType::KILL_EVIL:		return 3;
		default: return 0;
		}
	}

	void onTurnTick(int& piety) override
	{
		// 신앙도 자연 감소: 340턴마다 1씩
		// 턴 카운터는 호출측에서 관리, 여기서는 감소 로직만
	}

	void onJoin() override
	{
		// 입교 시 초기 신앙도 15에서 시작 (호출측에서 설정)
		// rank 0이므로 스킬 부여 없음
	}

	void onLeave() override
	{
		// 모든 신 스킬 회수 (호출측의 GodService에서 처리)
	}

	void onPietyRankChange(int oldRank, int newRank) override
	{
		// 스킬 부여/회수는 GodService에서 rankSkills를 참조하여 처리
	}

	int getInitialPenance() const override { return 25; }

	void executeWrath() override
	{
		// TODO: 징벌 효과 구현
		// 예: 일시적으로 치유 효과 반감, 상태이상 부여 등
	}
};
