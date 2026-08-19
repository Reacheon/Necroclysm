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
	// 스킬 고유 ID. SkillData.skillId와 매칭. 접두어 규칙:
	//   SKILL_xxx → GENERAL/MAGIC, BION_xxx → BIONIC, MUT_xxx → MUTATION
	std::wstring id = L"";
	std::wstring name = L"미확인 스킬";
	int iconIndex = 0;
	std::wstring descript = L"";
	std::wstring abstract = L"";
	skillSrc src = skillSrc::GENERAL;
	skillType type = skillType::ACTIVE;
	// GENERAL 스킬의 탭 분류 (weapon/survival/action). 다른 src는 src에서 자동 유도되므로 설정 불필요.
	skillCategory category = skillCategory::action;
	float energyPerAct = 0.0f;
	float energyPerTurn = 0.0f;
	float energyPerDay = 0.0f;
	std::wstring reqStat = L"";
	// 실패율에 영향을 주는 참조 스킬 ID 목록 (예: Roll/Leap → SKILL_ATHLETICS).
	// 보통 1개, 드물게 2개 (DCSS 이중학파식). 비어있으면 자기 랭크만으로 실패율 결정.
	std::vector<std::wstring> refSkills;
	int maxSkillLevel = 5;
	float maxCooldown = 30.0f;
	// 습득/설치 시점의 시작 랭크. 현재 랭크는 SkillData.skillRank에서 성장함.
	// 일반/기도술 스킬은 F 시작, 바이오닉은 이 값이 곧 부품 등급(설치 후 고정).
	std::wstring skillRank = L"F";

	//돌연변이 외형 메타 (src == MUTATION인 경우에만 사용)
	//mutLayer == none이면 외형 없음 (적외선시각 등)
	mutDrawLayer mutLayer = mutDrawLayer::none;
	std::wstring mutSprBaseName = L"";              //예: L"MUT_RABBIT_EARS". 색 접미사가 뒤에 자동 부착됨
	mutColorSource mutColorSrc = mutColorSource::none; //색 접미사 소스 (fur/horn/none)
	int mutDrawPriority = 0;                        //같은 레이어 내 그리기 순서 (오름차순)

	// 스킬 GUI 탭 분류: BIONIC/MUTATION/MAGIC/GOD는 src에서 유도, GENERAL은 category 필드 사용
	skillCategory getCategory() const
	{
		switch (src)
		{
		case skillSrc::BIONIC: return skillCategory::bionic;
		case skillSrc::MUTATION: return skillCategory::mutation;
		case skillSrc::MAGIC:
		case skillSrc::GOD: return skillCategory::divinity;
		default: return category;
		}
	}

	// 실패율 계산 (숙련도-스킬 통합 모델): 자기 랭크가 기본을 만들고 참조 스킬 랭크가 보정한다.
	// 참조 랭크가 낮아도 자기 랭크만 충분히 높으면 쓸 만한 수준이 되도록 참조 비중은 보조적.
	int calcFailRate(Entity* caster, const SkillData& data) const
	{
		constexpr int BASE_FAIL = 20; // 자기 F랭크·참조 F 기준 실패율
		constexpr int OWN_STEP = 3;   // 자기 랭크 1단계당 감소량 (F→S 최대 -18)
		constexpr int REF_STEP = 1;   // 참조 스킬 평균 1단계당 감소량 (F→S 최대 -6)

		int fail = BASE_FAIL - OWN_STEP * (rankDifficulty(data.skillRank) - 1);

		if (!refSkills.empty())
		{
			float refAvg = 0.0f;
			for (const auto& refId : refSkills)
			{
				int refDiff = 1; // 미습득 참조 스킬은 F랭크 취급
				for (const auto& sd : caster->entityInfo.skillList)
					if (sd.skillId == refId) { refDiff = rankDifficulty(sd.skillRank); break; }
				refAvg += static_cast<float>(refDiff - 1);
			}
			refAvg /= static_cast<float>(refSkills.size());
			fail -= static_cast<int>(REF_STEP * refAvg);
		}

		return std::clamp(fail, 0, 100);
	}

	// 사용 가능 여부 (쿨다운/자원과 별개로, 스킬 고유 조건)
	virtual bool canUse(Entity* caster, const SkillData& data) const { return true; }

	// 스킬 실행 코루틴 (CoordSelect, LstEx 등 자유롭게 co_await 가능)
	virtual Corouter execute(Entity* caster, SkillData& data) = 0;

	// 패시브/토글: 매 턴 호출
	virtual void onTurnTick(Entity* caster, SkillData& data) {}

	// 패시브: 스탯 보정
	virtual void modifyStats(Entity* caster, const SkillData& data) {}

	// 자동 발동 조건 (HP 25% 이하 등)
	virtual bool shouldAutoTrigger(Entity* caster, const SkillData& data) const { return false; }

	// 토글 온/오프 훅
	virtual void onToggleOn(Entity* caster, const SkillData& data) {}
	virtual void onToggleOff(Entity* caster, const SkillData& data) {}
};