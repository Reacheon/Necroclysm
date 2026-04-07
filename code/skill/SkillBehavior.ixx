export module SkillBehavior;

import std;
import util;
import constVar;
import SkillData;
import Entity;

export class SkillBehavior
{
public:
	virtual ~SkillBehavior() = default;

	// 정적 메타데이터 (서브클래스 생성자에서 설정)
	std::wstring name = L"UNNAMED SKILL";
	int iconIndex = 0;
	std::wstring descript = L"";
	std::wstring abstract = L"";
	skillSrc src = skillSrc::GENERAL;
	skillType type = skillType::ACTIVE;
	float energyPerAct = 0.0f;
	float energyPerTurn = 0.0f;
	float energyPerDay = 0.0f;
	std::wstring reqStat = L"";
	std::vector<int> reqProfic;
	int maxSkillLevel = 5;
	float maxCooldown = 30.0f;
	std::wstring skillRank = L"F";
	humanPartFlag bodyPart = humanPartFlag::torso; // 돌연변이/바이오닉 장착 부위

	// 랭크 문자열을 난이도 숫자로 변환 (F=1, E=2, D=3, C=4, B=5, A=6, S=7)
	int getRankDifficulty() const
	{
		if (skillRank == L"F") return 1;
		if (skillRank == L"E") return 2;
		if (skillRank == L"D") return 3;
		if (skillRank == L"C") return 4;
		if (skillRank == L"B") return 5;
		if (skillRank == L"A") return 6;
		if (skillRank == L"S") return 7;
		return 1;
	}

	// 실패율 계산: failRate = 15*난이도 - 5 - (25/6)*평균숙련도
	// F(Lv0)=10%, S(Lv0)=100%, S(Lv24)=0%
	// reqProfic이 비어있으면 실패율 0% (실패 없음)
	int calcFailRate(Entity* caster) const
	{
		if (reqProfic.empty()) return 0;

		float avgLevel = 0.0f;
		for (int idx : reqProfic)
			avgLevel += caster->getProficLevel(idx);
		avgLevel /= static_cast<float>(reqProfic.size());

		int rawFail = 15 * getRankDifficulty() - 5 - static_cast<int>(avgLevel * 25.0f / 6.0f);
		return std::clamp(rawFail, 0, 100);
	}

	// 스킬 코드 (SkillData.skillCode와 매칭)
	virtual int getSkillCode() const = 0;

	// 사용 가능 여부 (쿨다운/자원과 별개로, 스킬 고유 조건)
	virtual bool canUse(Entity* caster, const SkillData& data) const { return true; }

	// 스킬 실행 코루틴 (CoordSelect, LstEx 등 자유롭게 co_await 가능)
	virtual Corouter execute(Entity* caster, SkillData& data) = 0;

	// 패시브: 매 턴 호출
	virtual void onTurnTick(Entity* caster, const SkillData& data) {}

	// 패시브: 스탯 보정
	virtual void modifyStats(Entity* caster, const SkillData& data) {}

	// 자동 발동 조건 (HP 25% 이하 등)
	virtual bool shouldAutoTrigger(Entity* caster, const SkillData& data) const { return false; }

	// 토글 온/오프 훅
	virtual void onToggleOn(Entity* caster, const SkillData& data) {}
	virtual void onToggleOff(Entity* caster, const SkillData& data) {}
};
