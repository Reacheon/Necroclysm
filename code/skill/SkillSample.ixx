// ============================================================
// 스킬 추가 가이드
// ============================================================
// 1. 이 파일을 복사하여 SkillXXX.ixx를 만든다.
// 2. 모듈명, 클래스명, 생성자의 메타데이터(특히 id)를 수정한다.
//    id 접두어 규칙: SKILL_xxx(일반/마법), BION_xxx(바이오닉), MUT_xxx(돌연변이)
// 3. execute()에 스킬 로직을 구현한다. (Corouter 반환 → co_await 사용 가능)
// 4. SkillRegistry.cpp에 import + registerSkill 한 줄 추가.
// 5. vcxproj / vcxproj.filters에 파일 등록.
//
// [주의사항]
// - execute() 끝과 모든 co_return 직전에 currentUsingSkill.clear() 리셋
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
		id = L"SKILL_SAMPLE";          // 고유 ID로 변경할 것 (SKILL_/BION_/MUT_ 접두어)
		name = L"샘플 스킬";
		iconIndex = 0;
		descript = L"";
		src = skillSrc::GENERAL;   // GENERAL, BIONIC, MUTATION, MAGIC
		category = skillCategory::action; // GENERAL 전용 탭 분류: weapon(무기술)/survival(생존)/action(행동)
		type = skillType::ACTIVE;  // ACTIVE, PASSIVE, TOGGLE
		reqStat = L"";             // STR, INT, DEX 또는 빈 문자열
		// refSkills = { skillRefCode::athletics }; // 실패율에 영향을 주는 참조 스킬 ID (보통 1개, 드물게 2개)
		// skillRank = L"F";       // 시작 랭크. 기본값 F (바이오닉은 부품 등급으로 사용)
		// maxSkillLevel = 5;      // 기본값 5
		// maxCooldown = 30.0f;    // 기본값 30.0
		// energyPerAct = 0.0f;
		// energyPerTurn = 0.0f;
		// energyPerDay = 0.0f;
	}

	// [선택] 스킬 고유 사용 조건 (기본: true 반환)
	// bool canUse(Entity* caster, const SkillData& data) const override { return true; }

	// [필수] 스킬 실행
	Corouter execute(Entity* caster, SkillData& data) override
	{
		// 여기에 스킬 로직 구현
		// CoordSelect, LstEx 등 코루틴 GUI 사용 가능:
		//   new CoordSelect(...);
		//   co_await std::suspend_always();
		//   if (coAnswer.empty()) { currentUsingSkill.clear(); co_return; }

		currentUsingSkill.clear();
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
