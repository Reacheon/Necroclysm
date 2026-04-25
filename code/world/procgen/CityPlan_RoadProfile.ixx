export module CityPlan.RoadProfile;

import std;
import CityPlan;

// ──────────────────────────────────────────────────────────
// CityPlan.RoadProfile — 도로 단면 프로파일 카탈로그
//
//   RoadProfile 은 데이터다. 코드 수정 없이 프로파일 추가만으로
//   새 도로 종류를 도입할 수 있어야 한다 (CDDA 스파게티 방지).
//
//   카탈로그는 싱글톤. 게임 시작 시 1회 registerDefaultRoadProfiles()
//   호출 → 이후 Planner/Renderer 가 ProfileId 로 조회.
//
//   기본 프로파일 4종 (현재 3-tier 시스템과 호환):
//     neighborhood2Lane   (9 폭) — 동네 2차선,           collector 호환
//     arterial4Lane       (17 폭) — 4차선 간선,          arterial   호환
//     boulevard6Lane      (23 폭) — 중앙분리대 6차선,    highway    호환
//     scrambleBoulevard   (23 폭) — 시부야형 스크램블 교차로 진입 도로
//
//   Phase 0 에서는 정의·등록만. 실제 사용은 Phase 1-2 부터.
// ──────────────────────────────────────────────────────────

export class RoadProfileCatalog
{
public:
    static RoadProfileCatalog& ins()
    {
        static RoadProfileCatalog instance;
        return instance;
    }

    // 프로파일 등록. lane width 합과 totalWidth 일치 검증.
    // 불일치 시 ID는 정상 발급되지만 std::cerr 로 경고 — 디버깅용.
    ProfileId registerProfile(RoadProfile p)
    {
        int sum = 0;
        for (const auto& l : p.lanes) sum += l.width;
        if (sum != p.totalWidth)
        {
            std::cerr << "[RoadProfile] '" << p.name
                      << "' lane width sum (" << sum
                      << ") != totalWidth (" << p.totalWidth << ")\n";
        }
        p.id = (ProfileId)profiles_.size();
        profiles_.push_back(std::move(p));
        return profiles_.back().id;
    }

    const RoadProfile* get(ProfileId id) const
    {
        if (id == INVALID_PROFILE || id >= profiles_.size()) return nullptr;
        return &profiles_[id];
    }

    const RoadProfile* findByName(const std::string& name) const
    {
        for (const auto& p : profiles_) if (p.name == name) return &p;
        return nullptr;
    }

    std::size_t size() const { return profiles_.size(); }

    // 테스트/리셋용. 일반적으로 호출하지 않음.
    void clear() { profiles_.clear(); }

private:
    std::vector<RoadProfile> profiles_;
};

// ──────────────── 기본 프로파일 ID 묶음 ────────────────
// registerDefaultRoadProfiles() 의 반환값. Planner 가 자주 쓰는
// 표준 프로파일 ID를 한 번에 들고 다닐 수 있게 모아 반환.
export struct DefaultProfileIds
{
    ProfileId neighborhood2Lane = INVALID_PROFILE;
    ProfileId arterial4Lane     = INVALID_PROFILE;
    ProfileId boulevard6Lane    = INVALID_PROFILE;
    ProfileId scrambleBoulevard = INVALID_PROFILE;
};

// ──────────────── 기본 프로파일 등록 ────────────────
// 게임 시작 시 1회 호출.
//   현재 CityGen 의 3-tier (9/17/23) 와 같은 폭으로 매핑하여
//   Phase 1에서 기존 출력을 IR 경로로 흘려도 시각적 회귀가 없게 한다.
//
//   Lane 배치 규약: 외측 인도 → 안쪽 중앙선 → 반대편 인도 (대칭).
export inline DefaultProfileIds registerDefaultRoadProfiles()
{
    auto& cat = RoadProfileCatalog::ins();
    DefaultProfileIds ids;

    // 프로파일 레이아웃 규약 (Phase 2):
    //   사이드워크는 프로파일 밖 — Renderer 가 도로 외측에 별도 ring 으로 추가.
    //   lanes 배열은 "도로 표면(asphalt)" 단면만 기술한다.
    //
    //   legacy CityGen.Paint 의 stripe 위치와 1:1 매칭하도록 lane 분할:
    //     collector(9):  off 0      → 노란 점선
    //     arterial(17):  off 0      → 노란 실선
    //                    off ±4     → 흰 점선 (4차선 분리)
    //     highway(23):   off 0,±1   → 노란 실선 (3타일 분리대)
    //                    off ±5     → 흰 점선

    // ────────── neighborhood2Lane (9 폭) ──────────
    // off 범위 -4..+4. 중앙 1타일 = 노란 점선, 좌우 4타일씩 = 주행.
    {
        RoadProfile p;
        p.name           = "neighborhood2Lane";
        p.totalWidth     = 9;
        p.lanes = {
            { LaneKind::travelLane,       4 },
            { LaneKind::centerlineDashed, 1 },
            { LaneKind::travelLane,       4 },
        };
        p.crosswalkStyle = CrosswalkStyle::standard;
        p.divided        = false;
        p.materialTier   = 1;
        ids.neighborhood2Lane = cat.registerProfile(std::move(p));
    }

    // ────────── arterial4Lane (17 폭) ──────────
    // off 범위 -8..+8. 분포:
    //   off 0       : centerlineSolid (1)
    //   off ±1..±3  : travelLane     (3 each side, 안쪽 차선)
    //   off ±4      : laneStripe     (1 each side, 흰 점선)
    //   off ±5..±8  : travelLane     (4 each side, 바깥 차선)
    {
        RoadProfile p;
        p.name           = "arterial4Lane";
        p.totalWidth     = 17;
        p.lanes = {
            { LaneKind::travelLane,      4 },  // off -8..-5
            { LaneKind::laneStripe,      1 },  // off -4
            { LaneKind::travelLane,      3 },  // off -3..-1
            { LaneKind::centerlineSolid, 1 },  // off 0
            { LaneKind::travelLane,      3 },  // off +1..+3
            { LaneKind::laneStripe,      1 },  // off +4
            { LaneKind::travelLane,      4 },  // off +5..+8
        };
        p.crosswalkStyle = CrosswalkStyle::standard;
        p.divided        = false;
        p.materialTier   = 2;
        ids.arterial4Lane = cat.registerProfile(std::move(p));
    }

    // ────────── boulevard6Lane (23 폭) ──────────
    // off 범위 -11..+11. 분포:
    //   off -1..+1  : medianStrip    (3, 노란 실선으로 채움)
    //   off ±2..±4  : travelLane     (3 each side, 안쪽 차선)
    //   off ±5      : laneStripe     (1 each side)
    //   off ±6..±11 : travelLane     (6 each side, 바깥 차선)
    {
        RoadProfile p;
        p.name           = "boulevard6Lane";
        p.totalWidth     = 23;
        p.lanes = {
            { LaneKind::travelLane,  6 },  // off -11..-6
            { LaneKind::laneStripe,  1 },  // off -5
            { LaneKind::travelLane,  3 },  // off -4..-2
            { LaneKind::medianStrip, 3 },  // off -1..+1
            { LaneKind::travelLane,  3 },  // off +2..+4
            { LaneKind::laneStripe,  1 },  // off +5
            { LaneKind::travelLane,  6 },  // off +6..+11
        };
        p.crosswalkStyle = CrosswalkStyle::standard;
        p.divided        = true;
        p.materialTier   = 3;
        ids.boulevard6Lane = cat.registerProfile(std::move(p));
    }

    // ────────── scrambleBoulevard (23 폭) ──────────
    // boulevard6Lane 와 lane 분할 동일. 교차로 양식만 scramble.
    // Phase 4 도심 노드에 이 프로파일을 부여하면 Renderer 가 X자 + 직각
    // 횡단보도를 그린다.
    {
        RoadProfile p;
        p.name           = "scrambleBoulevard";
        p.totalWidth     = 23;
        p.lanes = {
            { LaneKind::travelLane,  6 },
            { LaneKind::laneStripe,  1 },
            { LaneKind::travelLane,  3 },
            { LaneKind::medianStrip, 3 },
            { LaneKind::travelLane,  3 },
            { LaneKind::laneStripe,  1 },
            { LaneKind::travelLane,  6 },
        };
        p.crosswalkStyle = CrosswalkStyle::scramble;
        p.divided        = true;
        p.materialTier   = 3;
        ids.scrambleBoulevard = cat.registerProfile(std::move(p));
    }

    return ids;
}

// ──────────────── 기본 프로파일 ID 전역 조회 ────────────────
// 처음 호출 시 registerDefaultRoadProfiles() 1회 실행 후 그 결과를 캐시.
// static-local 이라 thread-safe (C++11+).
//
// 의도:
//   procgen 의 어느 지점에서든 표준 4종 프로파일 ID 를 즉시 얻을 수 있게.
//   별도의 init 호출 순서를 걱정하지 않아도 됨.
export inline const DefaultProfileIds& defaultProfileIds()
{
    static const DefaultProfileIds ids = registerDefaultRoadProfiles();
    return ids;
}

// ──────────────── 조회 헬퍼 ────────────────
// ProfileId 로 lane 배열을 빠르게 얻는다.
export inline const std::vector<LaneSpec>* lanesOf(ProfileId id)
{
    const RoadProfile* p = RoadProfileCatalog::ins().get(id);
    return p ? &p->lanes : nullptr;
}

export inline int totalWidthOf(ProfileId id)
{
    const RoadProfile* p = RoadProfileCatalog::ins().get(id);
    return p ? p->totalWidth : 0;
}

export inline CrosswalkStyle crosswalkStyleOf(ProfileId id)
{
    const RoadProfile* p = RoadProfileCatalog::ins().get(id);
    return p ? p->crosswalkStyle : CrosswalkStyle::none;
}
