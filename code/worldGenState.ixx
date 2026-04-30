export module worldGenState;

import std;
import procGen;
import WorldGenScreen;

//============================================================
// 월드 생성 진행 상태 — main 루프에서 플레이어 의존 코드 가드용
//   true 동안 turnCycleLoop / stepEvent / renderTile 등은 스킵.
//   WorldGenScreen->step()만 별도로 매 프레임 호출됨.
//
//   기본 게임 시작은 startArea()가 직접 처리(소형 홈베이스 + Player + HUD).
//   3000개 도시 + 도로망 절차생성은 디버그 콘솔에서 startWorldGen()을
//   수동 호출해야 시작됨 — exe 키자마자 20분 대기 방지.
//
//   별도 모듈인 이유: debugConsole → startArea → HUD → debugConsole
//   순환 의존성 회피. 본 모듈은 procGen + WorldGenScreen만 import해서
//   사이클에 끼지 않음.
//============================================================
export bool worldGenInProgress = false;
export std::optional<procGen::WorldGenResult> worldGenResult;

//WorldGenScreen 띄우고 워커 스레드에서 generateWorld 실행. 디버그 콘솔에서 수동 호출.
//완료 시점에는 결과만 worldGenResult에 저장 — Player/HUD는 이미 존재하므로 재초기화 X.
export void startWorldGen()
{
    if (worldGenInProgress) return;        //이미 진행 중이면 무시
    worldGenInProgress = true;

    //시드는 일단 고정. 추후 옵션화 예정.
    constexpr std::uint64_t kSeed = 0xC0FFEE'1234ULL;

    new WorldGenScreen(kSeed, [](procGen::WorldGenResult result)
    {
        worldGenResult = std::move(result);
        worldGenInProgress = false;
    });
}
