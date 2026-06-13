export module mapEditorMode;

import util;
import constVar;
import globalVar;
import World;
import Player;
import LotEditor;

// ════════════════════════════════════════════════════════════════════════
// mapEditorMode — 맵 에디터 모드 진입.
//
//   현재 월드를 싹 비우고 빈 캔버스로 리셋한 뒤 LotEditor를 자동으로 띄운다.
//     z == 0  : 흙(dirt) 표면
//     z <  0  : 기존 지하처럼 (dirt + dirtWall, 파내는 솔리드)
//     z >  0  : 공허(void)
//
//   빈 월드 생성은 mapEditorActive 플래그가 켜진 동안 World::createChunk가
//   절차생성(SectorPlan 블릿)을 우회하는 방식으로 이뤄진다. 플래그는 세션 동안
//   유지되므로 카메라를 멀리 옮겨도 새 청크는 계속 빈 캔버스로 생성된다.
//
//   현재 호출처: 디버그 콘솔 40번. 향후: 엔딩 후 메인메뉴 진입.
//   전제: 플레이어가 차량에 미탑승 (resetWorldForEditor가 차량을 전부 파괴).
// ════════════════════════════════════════════════════════════════════════

export void enterMapEditor()
{
    mapEditorActive = true; //이후 생성되는 청크는 빈 캔버스(z=0 dirt)로

    const Point3 cur{ PlayerX(), PlayerY(), PlayerZ() };
    const Point3 origin{ 13, 12, 0 }; //흙 원점 (청크 (0,0,0)의 중심)

    World::ins()->resetWorldForEditor(cur, origin);

    PlayerPtr->updateNearbyChunk(CHUNK_LOADING_RANGE); //주변 빈 dirt 청크 선행 로드
    PlayerPtr->updateVision();

    cameraX = PlayerPtr->getX(); //카메라를 플레이어(청크 중심)로 정렬 — main.cpp 초기화와 동일
    cameraY = PlayerPtr->getY();
    zoomScale = 2.0f; //맵 에디터 기본 줌

    new LotEditor(); //자동 활성화. 소멸은 GUI 베이스가 관리.
}
