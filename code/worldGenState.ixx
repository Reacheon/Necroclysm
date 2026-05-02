export module worldGenState;

import std;
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

//WorldGenScreen 띄우고 워커 스레드에서 generateWorld 실행. 디버그 콘솔에서 수동 호출.
//완료 시점에는 결과만 worldGenResult에 저장 — Player/HUD는 이미 존재하므로 재초기화 X.
export void startWorldGen()
{
    if (worldGenInProgress) return;
    worldGenInProgress = true;

    std::random_device rd;
    const std::uint64_t seed =
        (static_cast<std::uint64_t>(rd()) << 32) | static_cast<std::uint64_t>(rd());

    new WorldGenScreen(seed, [](procGen::WorldGenResult result)
    {
        worldGenResult = std::move(result);
        worldGenInProgress = false;
    });
}
