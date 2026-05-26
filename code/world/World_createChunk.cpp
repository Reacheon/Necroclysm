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
//     - SectorPlan에서 읽어 TileData에 복사하는 *블릿만*
//
//   왜 이 책임 분리인가:
//     절차생성은 sector 광역 스케일에서만 의미 있음. 청크는 너무 작아 어떤 결정도 못 내림.
//     모든 procgen 변경은 Sector_procGenerate.cpp 한 곳만 수정 → 청크 코드 손대지 않음.
//
//   Z층 분기:
//     SectorPlan은 sc.z=0 단일 인스턴스에 모든 데이터 보유 (dense tiles + sparse
//     skyTiles + sparse props). chunkZ가 무엇이든 같은 (x,y,0) sector 조회.
//     - chunkZ == 0: dense PaintCell 블릿 + skyTiles(z=0은 비어있음) + props 필터
//     - chunkZ != 0: 빈 청크 + skyTiles(chunkZ 필터) + props(chunkZ 필터)
//     이 정책으로 z=±1 sector 14.7M 평면 안 할당 (다리 deck은 sparse가 충분).
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

    //--- 2) Phase 2 진입 후: Sector 데이터 블릿 ---
    if (worldGrid::worldPixelMmapActive())
    {
        // chunkX는 음수/W 초과(render-space)일 수 있으므로 wrap 후 sector 계산.
        const int wrapChunkX        = worldWrap::wrapChunkX(chunkX);
        const int chunkOriginTileX  = wrapChunkX * CHUNK_SIZE_X;
        const int chunkOriginTileY  = chunkY * CHUNK_SIZE_Y;

        //모든 z층 데이터는 (x,y,0) sector 단일 인스턴스에 모임.
        const SectorCoord sc = sectorFromTile(
            Point3{ chunkOriginTileX, chunkOriginTileY, 0 });
        const SectorPlan& sp = SectorCache::ins().getOrCompute(sc, worldSeed);

        //── 2a) chunkZ == 0: dense PaintCell 블릿 ──
        if (chunkZ == 0)
        {
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
                    tile.randomVal = c.randomVal;
                    tile.walkable  = (c.flags & TILE_FLAG_WALKABLE) != 0;
                    tile.hasSnow   = (c.flags & TILE_FLAG_HAS_SNOW) != 0;
                    tile.blocker   = (c.flags & TILE_FLAG_BLOCKER ) != 0;
                    tile.isWet     = (c.flags & TILE_FLAG_IS_WET  ) != 0;
                }
            }
        }

        //── 2b) sparse skyTiles 적용 — 본 z층(sc.z=0) 외 타일들 ──
        for (const SectorSkyTile& st : sp.skyTiles)
        {
            if (st.pos.z != chunkZ) continue;
            const int localX = st.pos.x - chunkOriginTileX;
            const int localY = st.pos.y - chunkOriginTileY;
            if (localX < 0 || localX >= CHUNK_SIZE_X) continue;
            if (localY < 0 || localY >= CHUNK_SIZE_Y) continue;

            TileData& tile = chunk->getChunkTile(localX, localY);
            if (st.floor != itemID::none) tile.floor = st.floor;
            if (st.wall  != itemID::none) tile.wall  = st.wall;
            tile.walkable = (st.flags & TILE_FLAG_WALKABLE) != 0;
        }

        chunkPtr[{wrapChunkX, chunkY, chunkZ}] = std::move(chunk);

        //── 2c) sparse props 생성 — chunk가 chunkPtr에 들어간 뒤 createProp 가능 ──
        for (const SectorProp& p : sp.props)
        {
            if (p.pos.z != chunkZ) continue;
            const int localX = p.pos.x - chunkOriginTileX;
            const int localY = p.pos.y - chunkOriginTileY;
            if (localX < 0 || localX >= CHUNK_SIZE_X) continue;
            if (localY < 0 || localY >= CHUNK_SIZE_Y) continue;
            createProp(p.pos, p.itemId);
        }
        return;
    }

    chunkPtr[{worldWrap::wrapChunkX(chunkX), chunkY, chunkZ}] = std::move(chunk);
}
