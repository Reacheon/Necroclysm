export module Teleport;

import std;
import util;
import constVar;
import globalVar;
import World;
import Player;

// ════════════════════════════════════════════════════════════════════════
// ════════════════════════════════════════════════════════════════════════

namespace teleport
{
    //목적지 주변 3×3 청크 선행 생성. EntityPtrMove의 getTile이 throw 안 하도록.
    void ensureChunksAround(Point3 dst)
    {
        int tcx, tcy;
        World::ins()->changeToChunkCoord(dst.x, dst.y, tcx, tcy);
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (!World::ins()->existChunk(tcx + dx, tcy + dy, dst.z))
                {
                    World::ins()->createChunk(tcx + dx, tcy + dy, dst.z);
                }
            }
        }
    }
}

//플레이어를 dst로 텔레포트. 목적지 청크 동기 생성 후 이동.
export void teleportPlayer(Point3 dst)
{
    //--- 1) 목적지 청크 선행 생성 ---
    teleport::ensureChunksAround(dst);

    //--- 2) 이동 (EntityPtrMove → setGrid → updateNearbyChunk) ---
    EntityPtrMove(Point3{ PlayerX(), PlayerY(), PlayerZ() }, dst);

    //--- 3) 시야 갱신 ---
    PlayerPtr->updateVision(PlayerInfo().eyeSight);
}