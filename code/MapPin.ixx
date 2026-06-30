module;
#include <SDL3/SDL.h>

export module MapPin;

import std;

// ════════════════════════════════════════════════════════════════════════
// MapPin — 플레이어가 월드맵에 찍는 항법 마커(웨이포인트).
//
//   마커는 "색상별로 하나"다 — 색마다 마커 1개(최대 MAP_PIN_COLOR_COUNT개 동시 존재).
//   같은 색을 다시 찍으면 그 색 마커가 새 자리로 이동한다(슬롯 = 색 인덱스, std::optional).
//   좌표는 타일 단위(찍은 청크의 중심 타일)로 보관 — 플레이어 좌표 패널·미니맵 방위 계산과
//   같은 단위라 변환이 단순하고 표시가 일관된다. x는 청크 중심 타일이라 tileToPixelX로 되돌리면
//   정확히 그 청크 셀 중심이 나온다(렌더는 Map.ixx가 plr 마커와 동일하게 sX/sY로 환산).
//
//   세이브/로드 시스템이 아직 없어 세션 글로벌로만 존속한다(activeCities와 동일 수명 정책 —
//   새 월드/재시작 시 리셋). 구조는 그대로 직렬화 가능.
//
//   색은 4가지(주황 제외 — 주황은 메인퀘스트 마커 전용 예약색).
// ════════════════════════════════════════════════════════════════════════

export struct MapPin
{
    int x = 0;   // 타일 좌표 (찍힌 청크의 중심 타일). 색은 슬롯 인덱스로 암묵 표현.
    int y = 0;
    int z = 0;
};

//핀 색 팔레트 — 서로 뚜렷이 구분되는 채도 높은 4색. 주황은 메인퀘스트 마커 예약색이라 제외.
//  무채색도 피함(지형 회색·도로와 충돌해 식별성 저하).
export inline constexpr SDL_Color MAP_PIN_PALETTE[] = {
    { 230,  64,  72, 255 },   // red
    {  84, 200,  96, 255 },   // green
    {  74, 156, 240, 255 },   // blue
    { 178, 108, 238, 255 },   // violet
};
export inline constexpr int MAP_PIN_COLOR_COUNT = static_cast<int>(sizeof(MAP_PIN_PALETTE) / sizeof(MAP_PIN_PALETTE[0]));

//색상별 단일 마커 슬롯 — 인덱스 = 색. nullopt = 그 색 마커 없음.
export inline std::array<std::optional<MapPin>, MAP_PIN_COLOR_COUNT> mapPins;

export inline SDL_Color mapPinColor(int idx)
{
    if (idx < 0 || idx >= MAP_PIN_COLOR_COUNT) idx = 0;
    return MAP_PIN_PALETTE[idx];
}

//그 색 마커가 (위치 무관) 존재하는가 — 메뉴 버튼 활성 표시 + 토글(존재하면 클릭=취소) 판정용.
export inline bool mapPinExists(int color)
{
    return color >= 0 && color < MAP_PIN_COLOR_COUNT && mapPins[color].has_value();
}

//그 색 마커 설치/이동 — 색당 1개라 항상 이 좌표로 교체.
export inline void setMapPin(int color, int x, int y, int z)
{
    if (color >= 0 && color < MAP_PIN_COLOR_COUNT) mapPins[color] = MapPin{ x, y, z };
}

//그 색 마커 제거 — 컨텍스트 메뉴에서 같은 색 버튼 재클릭(토글) 시 사용.
export inline void clearMapPin(int color)
{
    if (color >= 0 && color < MAP_PIN_COLOR_COUNT) mapPins[color].reset();
}
