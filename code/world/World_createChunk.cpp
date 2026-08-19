module World;

import std;
import util;
import constVar;
import globalVar;
import Chunk;

void World::createChunk(int chunkX, int chunkY, int chunkZ)
{
    chunkFlag inputFlag = chunkFlag::seawater;
    if (chunkZ > 0)      inputFlag = chunkFlag::none;
    else if (chunkZ < 0) inputFlag = chunkFlag::underground;

    chunkPtr[{chunkX, chunkY, chunkZ}] = std::make_unique<Chunk>(inputFlag);
}
