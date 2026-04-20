export module GodBehavior;

import std;
import constVar;

export class GodBehavior
{
public:
	virtual ~GodBehavior() = default;

	// 메타데이터 (서브클래스 생성자에서 설정)
	std::wstring name = L"UNNAMED GOD";
	std::wstring title = L"";
	std::wstring playerTitle = L"Follower";
	std::wstring descript = L"";
	std::vector<std::wstring> prohibitions;
	std::vector<std::wstring> pietyGains;
	int iconIndex = 0;
	int altarItemCode = 0;

	// 신앙도 단계 경계값 (0~200, 6단계)
	// rank 0: 0~29, rank 1: 30~49, rank 2: 50~74,
	// rank 3: 75~119, rank 4: 120~159, rank 5: 160~200
	std::array<int, 5> pietyThresholds = { 30, 50, 75, 120, 160 };

	// 이 신의 고유 ID
	virtual godFlag getGodFlag() const = 0;

	// 신앙도 → 현재 단계(0~5) 계산
	int getPietyRank(int piety) const
	{
		for (int i = 4; i >= 0; --i)
		{
			if (piety >= pietyThresholds[i]) return i + 1;
		}
		return 0;
	}

	// ─── 규율(Conduct) ───

	// 행동이 이 신의 규율에 위반되는지 확인한다.
	// 반환값: 신앙도 변동량 (음수 = 감소, 0 = 해당없음, 양수 = 보상)
	virtual int checkConduct(conductType conduct) const { return 0; }

	// ─── 신앙도 변동 ───

	// 신앙도 단계가 변할 때 호출된다. 스킬 부여/회수를 여기서 처리.
	// oldRank에서 newRank로 변경됨.
	virtual void onPietyRankChange(int oldRank, int newRank) {}

	// 매 턴마다 호출 (신앙도 자연 감소, 기도 효과 등)
	virtual void onTurnTick(int& piety) {}

	// ─── 배교 & 징벌 ───

	// 배교 시 초기 참회(penance) 값
	virtual int getInitialPenance() const { return 25; }

	// 징벌 효과 실행 (참회 중 확률적으로 호출됨)
	virtual void executeWrath() {}

	// ─── 입교 & 탈퇴 ───

	// 신을 믿기 시작할 때 호출
	virtual void onJoin() {}

	// 신을 떠날 때 호출 (스킬 회수 등)
	virtual void onLeave() {}

	// ─── 스킬 부여 헬퍼 ───

	// 특정 신앙도 단계에서 부여할 스킬 ID 목록
	// key: pietyRank, value: 해당 단계에서 부여되는 skillId 목록
	std::map<int, std::vector<std::wstring>> rankSkills;
};
