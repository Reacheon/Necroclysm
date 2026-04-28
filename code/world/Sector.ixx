export module Sector;

import std;
import constVar;

// 섹터 한 장의 픽셀별 지형 데이터를 저장
// 1픽셀 = 50타일 영역. 섹터 PNG의 원본 픽셀(400x400)을 그대로 보관
// 청크 생성 시 해당 청크 중심이 위치한 픽셀의 지형을 조회해 기본 타일을 깔음
export class Sector
{
private:
    std::array<std::array<chunkFlag, PIXEL_PER_SECTOR>, PIXEL_PER_SECTOR> pixelData{};

public:
    Sector()
    {
        for (auto& row : pixelData) row.fill(chunkFlag::none);
    }

    void set(int localPixelX, int localPixelY, chunkFlag f)
    {
        if (localPixelX < 0 || localPixelX >= PIXEL_PER_SECTOR) return;
        if (localPixelY < 0 || localPixelY >= PIXEL_PER_SECTOR) return;
        pixelData[localPixelY][localPixelX] = f;
    }

    chunkFlag get(int localPixelX, int localPixelY) const
    {
        if (localPixelX < 0 || localPixelX >= PIXEL_PER_SECTOR) return chunkFlag::none;
        if (localPixelY < 0 || localPixelY >= PIXEL_PER_SECTOR) return chunkFlag::none;
        return pixelData[localPixelY][localPixelX];
    }
};

// 좌표 변환 헬퍼들 (픽셀-섹터-타일 간 변환)
// 음수 좌표 floor division을 일관되게 처리

export int sectorFromTile(int tile)
{
    return (tile >= 0) ? (tile / TILE_PER_SECTOR)
                       : ((tile - (TILE_PER_SECTOR - 1)) / TILE_PER_SECTOR);
}

export int sectorFromPixel(int pixel)
{
    return (pixel >= 0) ? (pixel / PIXEL_PER_SECTOR)
                        : ((pixel - (PIXEL_PER_SECTOR - 1)) / PIXEL_PER_SECTOR);
}

export int pixelFromTile(int tile)
{
    return (tile >= 0) ? (tile / TILE_PER_PIXEL)
                       : ((tile - (TILE_PER_PIXEL - 1)) / TILE_PER_PIXEL);
}

export int localPixelFromTile(int tile)
{
    int sec = sectorFromTile(tile);
    int localTile = tile - sec * TILE_PER_SECTOR;
    return localTile / TILE_PER_PIXEL;
}

export int tileFromPixel(int pixel)
{
    return pixel * TILE_PER_PIXEL;
}

export int pixelCenterTile(int pixel)
{
    return pixel * TILE_PER_PIXEL + TILE_PER_PIXEL / 2;
}
