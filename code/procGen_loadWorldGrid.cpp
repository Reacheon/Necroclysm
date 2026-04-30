module;
#include <SDL3_image/SDL_image.h>

module procGen;

import std;
import util;

//============================================================
// 월드 PNG 로딩 — 5832개 섹터 PNG를 읽어 43200px×21600px Terrain 그리드 구성
//   섹터 번호 공식 / 색 매핑은 World::createSector와 동일.
//   나중에 createSector도 procGen 네임스페이스로 옮길 예정임(레거시 코드)
//   순수 블랙박스: 외부 상태 무관, grid만 반환.
//============================================================
namespace procGen
{
    namespace
    {
        constexpr int SECTOR_X_MIN  = -54;
        constexpr int SECTOR_X_MAX  =  53;
        constexpr int SECTOR_Y_MIN  = -27;
        constexpr int SECTOR_Y_MAX  =  26;
        constexpr int SECTOR_PIXEL  = 400;     //섹터 1장의 픽셀 변 (400x400)
        constexpr int NUMBER_BIAS   = 2971;    //number = NUMBER_BIAS + sectorX + 108*sectorY

        //RGB 24비트 패킹 — switch 비교 한 번으로 컬러 매칭
        constexpr std::uint32_t packRGB(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
        {
            return (std::uint32_t(r) << 16) | (std::uint32_t(g) << 8) | std::uint32_t(b);
        }

        constexpr std::uint32_t COL_SEAWATER    = packRGB(0x16, 0x21, 0xff);
        constexpr std::uint32_t COL_FRESHWATER  = packRGB(0x9d, 0xa2, 0xfb);
        constexpr std::uint32_t COL_CITY        = packRGB(0xa2, 0xa2, 0xa2);
        constexpr std::uint32_t COL_CITY_CENTER = packRGB(0xff, 0x00, 0x00);
        constexpr std::uint32_t COL_LAND        = packRGB(0x59, 0xc6, 0x82);
        constexpr std::uint32_t COL_BRIDGE      = packRGB(0x77, 0x77, 0x77);
        constexpr std::uint32_t COL_MOUNTAIN    = packRGB(0xc4, 0x65, 0x48);

        constexpr std::uint32_t COL_POLAR = packRGB(0xff, 0xff, 0xff);
        constexpr std::uint32_t COL_TUNDRA = packRGB(0x56, 0xb9, 0xc2);
        constexpr std::uint32_t COL_SUBARCTIC = packRGB(0x56, 0x7f, 0xc2);
        constexpr std::uint32_t COL_MONSOON = packRGB(0x85, 0x8f, 0x3f);
        constexpr std::uint32_t COL_SABANNA = packRGB(0x11, 0x58, 0x2c);

        constexpr Terrain colorToTerrain(std::uint32_t rgb) noexcept
        {
            switch (rgb)
            {
            case COL_SEAWATER:    return Terrain::Sea;
            case COL_FRESHWATER:  return Terrain::FreshWater;
            case COL_CITY:        return Terrain::CityZone;
            case COL_CITY_CENTER: return Terrain::CityCenter;
            case COL_LAND:        return Terrain::Land;
            case COL_BRIDGE:      return Terrain::Bridge;
            case COL_MOUNTAIN:    return Terrain::Mountain;

            case COL_POLAR: return Terrain::Polar;
            case COL_TUNDRA: return Terrain::Tundra;
            case COL_SUBARCTIC: return Terrain::Subarctic;
            case COL_MONSOON: return Terrain::Monsoon;
            case COL_SABANNA: return Terrain::Sabanna;

            default:           return Terrain::Sea;  //매칭 실패는 보수적으로 Sea
            }
        }

        std::string buildSectorPath(int sectorX, int sectorY)
        {
            int number = NUMBER_BIAS + sectorX + 108 * sectorY;
            std::string path = "map/worldSector-";
            if (number < 100) path += "0";  //createSector 기존 네이밍 규칙 보존
            path += std::to_string(number);
            path += ".png";
            return path;
        }

        //섹터 1장을 글로벌 그리드의 정해진 위치로 직접 디코드. 실패 시 false (Sea 디폴트 유지).
        bool blitSectorInto(int sectorX, int sectorY, Terrain* dst, int dstStride)
        {
            std::string path = buildSectorPath(sectorX, sectorY);
            SDL_Surface* surf = IMG_Load(path.c_str());
            if (!surf) return false;

            const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surf->format);
            //32bpp 가정 — 8/24bpp로 들어오면 uint32* 캐스팅이 픽셀 4개를 1개로 묶어 통째로 Sea가 되는 사일런트 버그 방지
            errorBox(fmt->bytes_per_pixel != 4,
                L"worldSector PNG가 32bpp가 아님: " + std::to_wstring(sectorX) + L"," + std::to_wstring(sectorY));
            const SDL_Palette* pal = SDL_GetSurfacePalette(surf);
            const std::uint32_t* pixels = static_cast<const std::uint32_t*>(surf->pixels);
            const int srcStride = surf->w;

            //글로벌 그리드 좌상단 기준의 이 섹터 시작 픽셀 좌표
            const int baseX = (sectorX - SECTOR_X_MIN) * SECTOR_PIXEL;
            const int baseY = (sectorY - SECTOR_Y_MIN) * SECTOR_PIXEL;

            for (int py = 0; py < SECTOR_PIXEL; ++py)
            {
                Terrain* row = dst + static_cast<std::size_t>(baseY + py) * dstStride + baseX;
                const std::uint32_t* srcRow = pixels + static_cast<std::size_t>(py) * srcStride;
                for (int px = 0; px < SECTOR_PIXEL; ++px)
                {
                    std::uint8_t r, g, b;
                    SDL_GetRGB(srcRow[px], fmt, pal, &r, &g, &b);
                    row[px] = colorToTerrain(packRGB(r, g, b));
                }
            }

            SDL_DestroySurface(surf);
            return true;
        }
    }

    //@brief 작업 폴더의 map 경로에 존재하는 위성사진 png들을 지형 데이터(색)을 포함하는 그리드로 반환한다. 순수 블랙박스 함수이다.
    //@return 위성사진의 지형데이터(Uint8)가 유니크포인터로 저장된 933MB가량의 대형 구조체
    PixelCostGrid loadWorldGrid()
    {
        const __int64 tStart = getNanoTimer();

        //--- 1. alloc + Sea 디폴트 fill ---
        PixelCostGrid grid;
        const std::size_t total =
            static_cast<std::size_t>(PixelCostGrid::W) * PixelCostGrid::H;
        grid.data = std::make_unique<Terrain[]>(total);
        std::fill_n(grid.data.get(), total, Terrain::Sea);

        const __int64 tAlloc = getNanoTimer();

        //--- 2. 섹터 PNG 5832장 디코드 ---
        int loadOk = 0;
        int loadFail = 0;
        for (int sy = SECTOR_Y_MIN; sy <= SECTOR_Y_MAX; ++sy)
        {
            for (int sx = SECTOR_X_MIN; sx <= SECTOR_X_MAX; ++sx)
            {
                if (blitSectorInto(sx, sy, grid.data.get(), PixelCostGrid::W)) ++loadOk;
                else ++loadFail;
            }
        }

        const __int64 tLoaded = getNanoTimer();

        //--- 3. 디버그 히스토그램 (Terrain 분포 검증) ---
        constexpr int TERRAIN_COUNT = 12;
        std::array<std::uint64_t, TERRAIN_COUNT> hist{};
        for (std::size_t i = 0; i < total; ++i)
        {
            ++hist[static_cast<std::size_t>(grid.data[i])];
        }

        const __int64 tHist = getNanoTimer();

        //--- 4. 리포트 ---
        const double allocMs = (tAlloc  - tStart ) / 1.0e6;
        const double loadMs  = (tLoaded - tAlloc ) / 1.0e6;
        const double histMs  = (tHist   - tLoaded) / 1.0e6;
        const double totalMs = (tHist   - tStart ) / 1.0e6;
        const double memMB   = static_cast<double>(total) / (1024.0 * 1024.0);

        prt(L"[procGen] loadWorldGrid done\n");
        prt(L"  alloc+fill : %8.2f ms\n", allocMs);
        prt(L"  sector load: %8.2f ms  (ok=%d fail=%d / %d)\n",
            loadMs, loadOk, loadFail, loadOk + loadFail);
        prt(L"  histogram  : %8.2f ms\n", histMs);
        prt(L"  total      : %8.2f ms  (%.2f s)\n", totalMs, totalMs / 1000.0);
        prt(L"  memory     : %8.1f MB\n", memMB);

        if (loadFail > 0)
        {
            const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
            prt(warn, L"  [WARN] %d sector PNG(s) missing/failed - filled with Sea\n", loadFail);
        }

        const wchar_t* const names[TERRAIN_COUNT] = {
            L"Land", L"Sea", L"FreshWater", L"Bridge", L"CityZone", L"CityCenter",
            L"Mountain", L"Polar", L"Tundra", L"Subarctic", L"Monsoon", L"Sabanna"
        };
        prt(L"  pixel terrain distribution:\n");
        for (int i = 0; i < TERRAIN_COUNT; ++i)
        {
            const double pct = 100.0 * static_cast<double>(hist[i])
                                     / static_cast<double>(total);
            prt(L"    %-10ls: %12llu  (%6.2f%%)\n", names[i], hist[i], pct);
        }

        return grid;
    }
}
