export module cityLayout;

import std;
import worldGrid;

// ════════════════════════════════════════════════════════════════════════
// cityLayout — 도시 픽셀 footprint 분해 (스텁).
//
//   책임 (현재 축소판):
//     - CityRect: 도시를 구성하는 직사각형(픽셀 좌표, 4×4 이상 보장)
//     - decomposeClusterToRects: 사전배치 도시의 임의 모양 클러스터를 4×4 이상
//                                직사각형들로 분해
//
//   BCP/CityLayout/CityRoadSegment/CityEntryPoint/CityBridge/CityBlock/Dir4 등
//   도시 내부 절차생성 데이터는 모두 제거 — 향후 sector 단계에서 lazy 재구현 예정.
//   현재 placeCities 만 사용. (이전엔 buildCityLayouts/buildRoadNetwork 가 입력으로 사용했음.)
//
//   픽셀 좌표(1px=48타일). raw 좌표 사용 — X 시암 wrap은 호출자가 처리.
// ════════════════════════════════════════════════════════════════════════

export namespace cityLayout
{
    // ─── 직사각형 (픽셀 좌표) ─────────────────────────────────────────────
    // w/h는 항상 ≥ 4 (계획서 보장).
    struct CityRect
    {
        int px = 0, py = 0;   // 좌상단 픽셀 좌표 (raw, X wrap 미적용)
        int w  = 0, h  = 0;   // 폭/높이 픽셀, ≥ 4

        constexpr int x1() const noexcept { return px + w; }  // exclusive
        constexpr int y1() const noexcept { return py + h; }
    };

    // ─── 클러스터 → 직사각형 분해 ─────────────────────────────────────────
    // PNG 클러스터링 결과(임의 모양의 City* 픽셀 집합)를 4×4 이상 직사각형들로 분해.
    //
    //   입력: bbox 안의 inMask[(py-py0)*W + (px-px0)] = (그 픽셀이 클러스터 소속이면 true)
    //         W = bboxW, H = bboxH.
    //         (px0, py0) = bbox 좌상단 raw 픽셀 좌표.
    //   출력: 클러스터를 완전히 덮는 (오버랩 없는) 4×4+ 직사각형 리스트.
    //         분해 실패(어떤 4×4 직사각형도 못 찾을 만큼 좁은 영역 잔재) 시 빈 리스트.
    //
    //   알고리즘: 그리디 max-rect — 매 라운드 남은 영역에서 가장 큰 직사각형 추출(히스토그램).
    //   계획서가 "전부 최소 4×4로 분해 가능"을 보장하므로 빈 리스트 = PNG 마킹 문제 신호.
    std::vector<CityRect> decomposeClusterToRects(const std::uint8_t* inMask, int bboxPxX, int bboxPxY, int bboxW, int bboxH, int minSize = 4);
}
