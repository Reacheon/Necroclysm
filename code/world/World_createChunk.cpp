module World;

import std;
import util;
import constVar;
import Chunk;
import TileData;
import worldGrid;
import Sector;
import worldSession;

// ════════════════════════════════════════════════════════════════════════
// World::createChunk — 청크 1개 생성 + (Phase 2 진입 후) Sector 데이터 *블릿*.
//
//   본 함수는 *순수 소비자*다:
//     - 절차생성 결정 X
//     - 픽셀 분석 X
//     - 노이즈·워프 X
//     - Terrain enum 매핑 X
//     - SectorPlan.tiles[] 배열에서 PaintCell 16×16개 읽어 TileData에 복사하는 *블릿만*
//
//   왜 이 책임 분리인가:
//     절차생성은 sector 광역 스케일에서만 의미 있음. 청크는 너무 작아 어떤 결정도 못 내림.
//     모든 procgen 변경은 Sector_procGenerate.cpp 한 곳만 수정 → 청크 코드 손대지 않음.
//
//   Phase 2 미진입(타이틀):
//     mmap 비활성 → SectorCache::getOrCompute 호출 안 함.
//     chunkFlag::seawater 디폴트로 균일 페인트 (Chunk 생성자가 처리).
// ════════════════════════════════════════════════════════════════════════

void World::createChunk(int chunkX, int chunkY, int chunkZ)
{
    //--- 1) chunkFlag 디폴트 (Phase 2 미진입 시 보임) ---
    chunkFlag inputFlag = chunkFlag::seawater;
    if (chunkZ > 0)      inputFlag = chunkFlag::none;
    else if (chunkZ < 0) inputFlag = chunkFlag::underground;

    auto chunk = std::make_unique<Chunk>(inputFlag);

    //--- 2) Phase 2 진입 후: Sector PaintCell 블릿 ---
    //   16×16 = 256 타일을 SectorPlan.tiles에서 직접 복사. 결정 0, 매핑 0.
    if (chunkZ == 0 && worldGrid::worldPixelMmapActive())
    {
        // chunkX는 음수/W 초과(render-space)일 수 있으므로 wrap 후 sector 계산.
        // 그렇지 않으면 sectorFromTile이 음수 sectorX를 반환해 캐시 키 충돌 발생.
        const int wrapChunkX        = worldWrap::wrapChunkX(chunkX);
        const int chunkOriginTileX  = wrapChunkX * CHUNK_SIZE_X;
        const int chunkOriginTileY  = chunkY * CHUNK_SIZE_Y;

        const SectorCoord sc = sectorFromTile(
            Point3{ chunkOriginTileX, chunkOriginTileY, chunkZ });
        const SectorPlan& sp = SectorCache::ins().getOrCompute(sc, worldSeed);

        //sector 내부 로컬 타일 좌표 — 청크 좌상단 기준.
        const int localTileBaseX = chunkOriginTileX - sc.x * SectorCoord::TILES;
        const int localTileBaseY = chunkOriginTileY - sc.y * SectorCoord::TILES;

        for (int y = 0; y < CHUNK_SIZE_Y; ++y)
        {
            const std::size_t rowBase =
                static_cast<std::size_t>(localTileBaseY + y) * SectorCoord::TILES
                + localTileBaseX;

            for (int x = 0; x < CHUNK_SIZE_X; ++x)
            {
                const PaintCell& c = sp.tiles[rowBase + x];

                TileData& tile = chunk->getChunkTile(x, y);
                tile.floor     = c.floor;
                tile.wall      = c.wall;
                tile.ceil      = c.ceil;
                tile.randomVal = c.randomVal;
                tile.walkable  = (c.flags & TILE_FLAG_WALKABLE) != 0;
                tile.hasSnow   = (c.flags & TILE_FLAG_HAS_SNOW) != 0;
                tile.blocker   = (c.flags & TILE_FLAG_BLOCKER ) != 0;
                tile.isWet     = (c.flags & TILE_FLAG_IS_WET  ) != 0;
            }
        }
    }

    chunkPtr[{worldWrap::wrapChunkX(chunkX), chunkY, chunkZ}] = std::move(chunk);
}
