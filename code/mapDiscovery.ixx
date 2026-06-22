export module mapDiscovery;

import std;
import constVar;
import worldWrap;

// ════════════════════════════════════════════════════════════════════════
// mapDiscovery — 월드맵 "전장의 구름" 탐험 상태 (청크=픽셀 해상도).
//
//   타일 단위 fovFlag(TileData)는 로드된 청크에만 존재하고 디스크 저장도 안 되므로
//   광역 월드맵에는 못 쓴다. 여기서는 "플레이어가 가본 청크"를 픽셀(=청크) 좌표
//   집합으로 따로 들고 있는다. 희소(sparse) 집합이라 탐험한 면적에 비례해서만
//   메모리를 쓴다 (전 세계 9억 청크를 비트맵으로 들 필요 없음).
//
//   설계 모델 — 지형(해안선/강/산)은 위성지도라 항상 보이고, 인공물(도로/도시/
//   건물/라벨)만 이 집합으로 게이트한다:
//     discovered == true  → 가본 곳(회색 구름): 인공물 표시, 지형 약간 밝게.
//     discovered == false → 미발견(검은 구름): 지형 윤곽만, 인공물 숨김.
//
//   마킹은 Player::updateVision 단일 길목에서 호출 — 시야 갱신 = 플레이어가 그
//   위치를 "인지"한 시점. (세이브/로드 영속화는 후속 — 현재는 세션 내 유지.)
// ════════════════════════════════════════════════════════════════════════

namespace mapDiscovery
{
    //플레이어 맵 발견 반경(원형, 청크). 시야 갱신 때 이 원 안 청크를 "탐험됨"으로 영구 기록 —
    //  한 번 들어오면 떠나도 도로·도시·지형이 풀밝기로 남는다(Map.ixx tierBright는 발견/미발견
    //  2상태라 별도 시야 티어 없음). 단일 발견 반경 튜닝 포인트(직경 = 2*R 청크).
    export constexpr int SIGHT_CHUNK_RADIUS = 36;

    //픽셀(=청크) 좌표 → 64비트 키. px∈[0,WORLD_PIXEL_W), py∈[0,WORLD_PIXEL_H) 가정(둘 다 16비트 초과 X).
    std::uint64_t keyOf(int px, int py)
    {
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(px)) << 32)
             |  static_cast<std::uint64_t>(static_cast<std::uint32_t>(py));
    }

    std::unordered_set<std::uint64_t>& store()
    {
        static std::unordered_set<std::uint64_t> s;
        return s;
    }

    //타일 좌표(플레이어 위치) 주변 청크들을 발견 처리. cx,cy = 타일 좌표.
    //  타일→픽셀 변환은 Map의 tilePixelIX/IY와 동일 규약: (t - TILE_BASE)/TILE_PER_PIXEL.
    export void markAroundTile(int cx, int cy)
    {
        const int px = (cx - TILE_BASE_X) / TILE_PER_PIXEL;
        const int py = (cy - TILE_BASE_Y) / TILE_PER_PIXEL;
        auto& s = store();
        constexpr int R  = SIGHT_CHUNK_RADIUS;
        constexpr int R2 = R * R;
        for (int dy = -R; dy <= R; ++dy)
        {
            const int wy = py + dy;
            if (wy < 0 || wy >= WORLD_PIXEL_H) continue;   // Y는 wrap 안 함(극지 경계)
            for (int dx = -R; dx <= R; ++dx)
            {
                if (dx * dx + dy * dy > R2) continue;       // 원형 — 사각 박스 아님
                s.insert(keyOf(worldWrap::wrapPixelX(px + dx), wy));
            }
        }
    }

    //픽셀(=청크) 좌표가 발견됐는지. px는 내부에서 wrap, py는 범위 밖이면 false.
    export bool discovered(int px, int py)
    {
        if (py < 0 || py >= WORLD_PIXEL_H) return false;
        return store().contains(keyOf(worldWrap::wrapPixelX(px), py));
    }

    //새 월드 생성 시 호출 — 이전 월드의 발견 상태 제거.
    export void reset() { store().clear(); }

    //발견한 청크 수(디버그/HUD용).
    export std::size_t count() { return store().size(); }
}
