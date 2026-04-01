// ============================================================
// 스킬 추가 가이드
// ============================================================
// 1. 이 파일을 복사하여 SkillXXX.ixx를 만든다.
// 2. 모듈명, 클래스명, 생성자의 메타데이터, getSkillCode()를 수정한다.
// 3. execute()에 스킬 로직을 구현한다. (Corouter 반환 → co_await 사용 가능)
// 4. SkillRegistry.cpp에 import + registerSkill 한 줄 추가.
// 5. vcxproj / vcxproj.filters에 파일 등록.
//
// [주의사항]
// - execute() 끝과 모든 co_return 직전에 currentUsingSkill = -1 리셋
// - 플레이어 전용 처리(카메라 등)는 caster == static_cast<Entity*>(PlayerPtr) 체크
// - 애니메이션은 aniFlag enum에 추가 후 Entity_runAnimation.cpp에 구현
// ============================================================

export module SkillSample;

import std;
import util;
import constVar;
import SkillData;
import SkillBehavior;
import globalVar;
import World;
import Entity;

export class SkillSample : public SkillBehavior
{
public:
	SkillSample()
	{
		name = L"Sample Skill";
		iconIndex = 0;
		descript = L"This is a sample skill for reference.";
		src = skillSrc::GENERAL;   // GENERAL, BIONIC, MUTATION, MAGIC
		type = skillType::ACTIVE;  // ACTIVE, PASSIVE, TOGGLE
		reqStat = L"";             // STR, INT, DEX 또는 빈 문자열
		// reqProfic = { 1 };      // 필요 숙련도 인덱스 목록
		// skillRank = L"F";       // 기본값 F
		// maxSkillLevel = 5;      // 기본값 5
		// maxCooldown = 30.0f;    // 기본값 30.0
		// energyPerAct = 0.0f;
		// energyPerTurn = 0.0f;
		// energyPerDay = 0.0f;
	}

	int getSkillCode() const override { return -1; } // 고유 코드로 변경할 것

	// [선택] 스킬 고유 사용 조건 (기본: true 반환)
	// bool canUse(Entity* caster, const SkillData& data) const override { return true; }

	// [필수] 스킬 실행
	Corouter execute(Entity* caster, SkillData& data) override
	{
		// 여기에 스킬 로직 구현
		// CoordSelect, LstEx 등 코루틴 GUI 사용 가능:
		//   new CoordSelect(...);
		//   co_await std::suspend_always();
		//   if (coAnswer.empty()) { currentUsingSkill = -1; co_return; }

		currentUsingSkill = -1;
		co_return;
	}

	// [선택] 패시브: 매 턴 호출
	// void onTurnTick(Entity* caster, const SkillData& data) override { }

	// [선택] 패시브: 스탯 보정
	// void modifyStats(Entity* caster, const SkillData& data) override { }

	// [선택] 자동 발동 조건 (HP 25% 이하 등)
	// bool shouldAutoTrigger(Entity* caster, const SkillData& data) const override { return false; }

	// [선택] 토글 온/오프 훅
	// void onToggleOn(Entity* caster, const SkillData& data) override { }
	// void onToggleOff(Entity* caster, const SkillData& data) override { }
};
