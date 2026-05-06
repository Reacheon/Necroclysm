export module worldGenState;

import std;
import util;
import procGen;
import WorldGenScreen;

//============================================================
// 월드 생성 진행 상태 — main 루프에서 플레이어 의존 코드 가드용.
//   true 동안 turnCycleLoop / stepEvent / renderTile 등은 스킵.
//   WorldGenScreen->step()만 별도로 매 프레임 호출됨.
//
//   기본 게임 시작은 startArea()가 직접 처리(소형 홈베이스 + Player + HUD).
//   3000개 도시 + 도로망 절차생성은 디버그 콘솔에서 startWorldGen()을 수동
//   호출해야 시작됨 — 실행 직후 자동 진입으로 인한 장시간 대기 방지.
//
//   별도 모듈인 이유: debugConsole → startArea → HUD → debugConsole 순환
//   의존성 회피. 본 모듈은 procGen + WorldGenScreen만 import.
//============================================================
export bool worldGenInProgress = false;
export std::optional<procGen::WorldGenResult> worldGenResult;

//월드 절차생성에 사용된 시드. 0 = 미생성 상태.
//  Sector procGenerate / CityLayout BCP 등 모든 후속 절차생성이 이 시드를 참조.
//  결정론 보장 — 같은 시드면 세이브/로드 후에도 같은 plan 재생성.
export std::uint64_t worldSeed = 0;

//Phase 1 종료 후 자동 텔레포트 대상. 한반도 서울 픽셀(36840, 6293) 센터 (48타일/px).
//  Phase 4가 이 좌표 주변 9 섹터를 사전 절차생성하므로 텔레포트 즉시 진입 가능.
//  향후 캐릭터 생성 화면에서 선택 도시 좌표로 대체 예정.
//  (Point3 ctor가 constexpr 아니라 inline const로 정의 — module interface에서 안전.)
export inline const Point3 SPAWN_DEFAULT{ 731544, -216312, 0 };

//Phase 1~4 완료 후 호출되는 후처리 콜백. main.cpp가 부팅 시 설정.
//  내용: 타이틀/startArea 청크 wipe → SPAWN_DEFAULT로 텔레포트.
//  worldGenState 자체는 World/Teleport import 안 함 (의존성 cycle 회피) — 콜백 주입으로 우회.
export std::function<void()> onWorldGenComplete;

//WorldGenScreen 띄우고 워커 스레드에서 generateWorld 실행. 디버그 콘솔에서 수동 호출.
//완료 시점에는 결과만 worldGenResult에 저장 — Player/HUD는 이미 존재하므로 재초기화 X.
//  Phase 4까지 끝나면 onWorldGenComplete 콜백 호출 → wipe + 텔레포트.
export void startWorldGen()
{
    if (worldGenInProgress) return;
    worldGenInProgress = true;

    std::random_device rd;
    worldSeed =
        (static_cast<std::uint64_t>(rd()) << 32) | static_cast<std::uint64_t>(rd());

    new WorldGenScreen(worldSeed, SPAWN_DEFAULT, [](procGen::WorldGenResult result)
    {
        worldGenResult = std::move(result);
        worldGenInProgress = false;

        //타이틀 청크 wipe + SPAWN_DEFAULT 텔레포트.
        if (onWorldGenComplete) onWorldGenComplete();
    });
}
