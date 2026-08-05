module World;

import std;
import util;
import constVar;
import globalVar;
import Chunk;

// ════════════════════════════════════════════════════════════════════════
// World::createChunk — 청크 1개 생성. z층별 디폴트 균일 채움만 수행.
//   z > 0 : 공허
//   z < 0 : 지하 (dirt + dirtWall, 파내는 솔리드)
//   z = 0 : 바다 (chunkFlag::seawater — Chunk 생성자가 균일 페인트)
// ════════════════════════════════════════════════════════════════════════

void World::createChunk(int chunkX, int chunkY, int chunkZ)
{
    chunkFlag inputFlag = chunkFlag::seawater;
    if (chunkZ > 0)      inputFlag = chunkFlag::none;
    else if (chunkZ < 0) inputFlag = chunkFlag::underground;

    chunkPtr[{chunkX, chunkY, chunkZ}] = std::make_unique<Chunk>(inputFlag);
}
