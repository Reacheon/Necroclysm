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
