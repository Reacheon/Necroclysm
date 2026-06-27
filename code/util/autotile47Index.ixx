export module autotile47Index;

//@brief 8 이웃 마스크 → 47-piece 블롭 오토타일(GameMaker autotile47 컨벤션) 인덱스(0~46).
//  각 bool = "그 방향 이웃이 *이 타일과 다른 쪽 지형*인가"(= 자기 자신이 아닌 배경/대비 지형).
//  예) shoreSpline: 자기=물, bool=land(이웃이 육지). 산맥 오토타일: 자기=산, bool=非산.
//  배치(인덱스→8×6 그리드 셀, cellX=idx%8, cellY=idx/8)는 image/autotile47/* 및
//  image/spline/shoreSpline*.png와 동일. 셀 47은 미사용(공백).
//
//  인덱스 매핑 (47 = 16 + 16 + 2 + 8 + 4 + 1):
//    0..15  — 변 0 + 외각 코너 16조합 (NW=1, NE=2, SE=4, SW=8 raw 비트 합)
//    16..19 — L-Edge(W) + 외각 NE/SE 0/1조합
//    20..23 — T-Edge(N) + 외각 SE/SW
//    24..27 — R-Edge(E) + 외각 SW/NW
//    28..31 — B-Edge(S) + 외각 NW/NE
//    32 — L+R (수직 일자통로) / 33 — T+B (수평 일자통로)
//    34..41 — 변 2 인접(T+L, T+R, R+B, L+B) + 외각 코너 0/1
//    42..45 — 변 3 (데드엔드 4가지) / 46 — 변 4 (둘러쌓임)
//
//  외각 코너는 *양변 모두 대비지형*일 때만 raw 비트 의미. 양변이 자기지형인 코너는
//  prefab에서 자동 처리(raw 무관), 한쪽 변만 대비지형인 코너는 변에 흡수(raw 무관).
//  이 마스킹 룰로 raw 8비트 → 47 인덱스 압축.
export int autotile47Index(bool n, bool e, bool s, bool w, bool nw, bool ne, bool sw, bool se)
{
	const bool oNW = nw && !n && !w;
	const bool oNE = ne && !n && !e;
	const bool oSW = sw && !s && !w;
	const bool oSE = se && !s && !e;
	const int edges = (n ? 1 : 0) + (e ? 1 : 0) + (s ? 1 : 0) + (w ? 1 : 0);

	if (edges == 0)
	{
		//   변 0: 외각 코너 16조합. NW=1, NE=2, SE=4, SW=8 비트 합.
		return (oNW ? 1 : 0) | (oNE ? 2 : 0) | (oSE ? 4 : 0) | (oSW ? 8 : 0);
	}
	if (edges == 4) return 46;
	if (edges == 1)
	{
		//   변 1 + 외각 코너 (변 인접 코너 2개는 한쪽 자기지형이라 흡수, 나머지 2개만 외각 가능).
		if (w) return 16 + ((oNE ? 1 : 0) | (oSE ? 2 : 0));
		if (n) return 20 + ((oSE ? 1 : 0) | (oSW ? 2 : 0));
		if (e) return 24 + ((oSW ? 1 : 0) | (oNW ? 2 : 0));
		return 28 + ((oNW ? 1 : 0) | (oNE ? 2 : 0));   // s
	}
	if (edges == 2)
	{
		if (w && e) return 32;   // L+R 수직 통로
		if (n && s) return 33;   // T+B 수평 통로
		//   변 2 인접: 양변 자기지형 코너 1개(자동 처리), 양변 대비지형 코너 1개(외각 가능).
		if (n && w) return oSE ? 35 : 34;   // T+L, 외각 SE
		if (n && e) return oSW ? 37 : 36;   // T+R, 외각 SW
		if (e && s) return oNW ? 39 : 38;   // R+B, 외각 NW
		return oNE ? 41 : 40;               // L+B, 외각 NE
	}
	//   edges == 3: 데드엔드. 양변 자기지형 코너 2개 자동 처리, 외각 가능 0개.
	if (!s) return 42;   // L+T+R
	if (!e) return 43;   // L+T+B
	if (!n) return 44;   // L+B+R
	return 45;           // T+R+B
}
