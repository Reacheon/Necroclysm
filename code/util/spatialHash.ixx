export module spatialHash;

import std;

//============================================================
// 균등 격자 공간 해시 — int 인덱스를 (px, py) 좌표 기반 셀에 저장.
//   삽입 O(1), 반경 질의는 검색반경/셀크기 만큼의 셀만 훑음.
//   좌표 의미(픽셀/타일/미터)는 호출자가 정함 — 이 모듈은 정수쌍만 본다.
//
//   사용 패턴:
//     SpatialHash h(worldW, worldH, cellSize);
//     for (...) h.insert(itemIdx, x, y);
//     h.forEachInRadius(qx, qy, R, [&](int idx) { ... });
//
//   주의: 도시 충돌 검사 같은 도메인 로직은 forEachInRadius 람다 내부에서 직접 수행.
//============================================================

//@brief 주변에 가까운 점들 찾을 때 미리 구역별로 나눠서 캐시해놓고 빨리 찾는 기능
export struct SpatialHash
{
    int                           cellSize;
    int                           gridW;
    int                           gridH;
    std::vector<std::vector<int>> cells;

    SpatialHash(int worldW, int worldH, int cellSize_)
        : cellSize(cellSize_)
        , gridW((worldW + cellSize_ - 1) / cellSize_)
        , gridH((worldH + cellSize_ - 1) / cellSize_)
        , cells(static_cast<std::size_t>(gridW) * gridH)
    {}

    std::size_t cellIdx(int cx, int cy) const noexcept
    {
        return static_cast<std::size_t>(cy) * gridW + cx;
    }

    void insert(int idx, int px, int py)
    {
        const int cx = std::clamp(px / cellSize, 0, gridW - 1);
        const int cy = std::clamp(py / cellSize, 0, gridH - 1);
        cells[cellIdx(cx, cy)].push_back(idx);
    }

    template <class F>
    void forEachInRadius(int px, int py, int searchR, F&& fn) const
    {
        const int searchCells = (searchR + cellSize - 1) / cellSize;
        const int cxC = std::clamp(px / cellSize, 0, gridW - 1);
        const int cyC = std::clamp(py / cellSize, 0, gridH - 1);
        const int x0  = std::max(0,         cxC - searchCells);
        const int x1  = std::min(gridW - 1, cxC + searchCells);
        const int y0  = std::max(0,         cyC - searchCells);
        const int y1  = std::min(gridH - 1, cyC + searchCells);
        for (int cy = y0; cy <= y1; ++cy)
            for (int cx = x0; cx <= x1; ++cx)
                for (int i : cells[cellIdx(cx, cy)])
                    fn(i);
    }
};
