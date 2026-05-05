module;
#include <SDL3_image/SDL_image.h>

module procGen;

import std;
import util;

//============================================================
// 월드 PNG 로딩 (내부 백엔드) — 5832개 패치 PNG를 읽어 43200×21600 Terrain 그리드 구성.
//   패치 번호 공식 / 색 매핑은 World::createPatch와 동일.
//   순수 블랙박스: 외부 상태 무관, grid만 반환.
//   공개 진입점은 procGen_worldGridCache.cpp의 loadWorldGrid가 담당. 이 함수는
//   캐시 miss 시에만 호출되므로 export하지 않음.
//============================================================
namespace procGen
{
    namespace
    {
        constexpr int PATCH_X_MIN  = -54;
        constexpr int PATCH_X_MAX  =  53;
        constexpr int PATCH_Y_MIN  = -27;
        constexpr int PATCH_Y_MAX  =  26;
        constexpr int PATCH_PIXEL  = 400;     //패치 1장의 픽셀 변 (400×400)
        constexpr int NUMBER_BIAS  = 2971;    //number = NUMBER_BIAS + patchX + 108*patchY

        //RGB 24비트 패킹 — switch 비교 한 번으로 컬러 매칭.
        constexpr std::uint32_t packRGB(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
        {
            return (std::uint32_t(r) << 16) | (std::uint32_t(g) << 8) | std::uint32_t(b);
        }

        constexpr std::uint32_t COL_SEAWATER    = packRGB(0x16, 0x21, 0xff);
        constexpr std::uint32_t COL_RIVER       = packRGB(0x9d, 0xa2, 0xfb);   //강 — 폭 1~2px, 도로 가로지름 허용
        constexpr std::uint32_t COL_LAKE        = packRGB(0x93, 0x84, 0xe5);   //호수 — 도로 거의 차단
        constexpr std::uint32_t COL_CITY        = packRGB(0xa2, 0xa2, 0xa2);
        constexpr std::uint32_t COL_CITY_CENTER = packRGB(0xff, 0x00, 0x00);
        constexpr std::uint32_t COL_CITY_RIVER  = packRGB(0xa2, 0xbf, 0xef);   //도시 내 강 — CityZone과 함께 도시 범위 산정
        constexpr std::uint32_t COL_CITY_SEA    = packRGB(0x6d, 0x6a, 0xbd);   //도시 내 바다(소금물) — 이스탄불/홍콩식 해협. 도시 범위 포함.
        constexpr std::uint32_t COL_LAND        = packRGB(0x59, 0xc6, 0x82);
        constexpr std::uint32_t COL_MOUNTAIN    = packRGB(0xc4, 0x65, 0x48);

        constexpr std::uint32_t COL_POLAR = packRGB(0xff, 0xff, 0xff);
        constexpr std::uint32_t COL_TUNDRA = packRGB(0x56, 0xb9, 0xc2);
        constexpr std::uint32_t COL_SUBARCTIC = packRGB(0x56, 0x7f, 0xc2);
        constexpr std::uint32_t COL_MONSOON = packRGB(0x85, 0x8f, 0x3f);
        constexpr std::uint32_t COL_INSULAR_RAINFOREST = packRGB(0x1a, 0x6c, 0x25); //해양/도서성 — 동남아 군도(옛 Sabanna 자리)
        constexpr std::uint32_t COL_DESERT = packRGB(0xd3, 0xc6, 0x37);
        constexpr std::uint32_t COL_CONT_RAINFOREST = packRGB(0x11, 0x58, 0x2c);    //대륙성 — 아마존/콩고(옛 Sabanna 색상 재할당)

        constexpr Terrain colorToTerrain(std::uint32_t rgb) noexcept
        {
            switch (rgb)
            {
            case COL_SEAWATER:    return Terrain::Sea;
            case COL_RIVER:       return Terrain::River;
            case COL_LAKE:        return Terrain::Lake;
            case COL_CITY:        return Terrain::CityZone;
            case COL_CITY_CENTER: return Terrain::CityCenter;
            case COL_CITY_RIVER:  return Terrain::CityRiver;
            case COL_CITY_SEA:    return Terrain::CitySea;
            case COL_LAND:        return Terrain::Land;
            case COL_MOUNTAIN:    return Terrain::Mountain;

            case COL_POLAR: return Terrain::Polar;
            case COL_TUNDRA: return Terrain::Tundra;
            case COL_SUBARCTIC: return Terrain::Subarctic;
            case COL_MONSOON: return Terrain::Monsoon;
            case COL_INSULAR_RAINFOREST: return Terrain::InsularRainforest;
            case COL_DESERT:  return Terrain::Desert;
            case COL_CONT_RAINFOREST: return Terrain::ContinentalRainforest;

            default:           return Terrain::Sea;  //매칭 실패는 보수적으로 Sea
            }
        }

        std::string buildPatchPath(int patchX, int patchY)
        {
            //파일명은 3자리 0-padding (worldPatch-001.png ~ worldPatch-5832.png).
            //과거 1자리 패딩 분기는 number 1~9에서 -01.png를 만들어 sy=-27 끝줄 9장이
            //통째로 누락되던 버그가 있어 폐기.
            int number = NUMBER_BIAS + patchX + 108 * patchY;
            return std::format("map/worldPatch-{:03d}.png", number);
        }

        //패치 1장을 글로벌 그리드의 정해진 위치로 직접 디코드. 실패 시 false(Sea 디폴트 유지).
        bool blitPatchInto(int patchX, int patchY, Terrain* dst, int dstStride)
        {
            std::string path = buildPatchPath(patchX, patchY);
            SDL_Surface* surf = IMG_Load(path.c_str());
            if (!surf) return false;

            const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surf->format);
            //32bpp 가정. 8/24bpp가 들어오면 uint32* 캐스팅이 픽셀 4개를 1개로 묶어
            //통째로 Sea가 되는 사일런트 버그 방지.
            errorBox(fmt->bytes_per_pixel != 4,
                L"worldPatch PNG가 32bpp가 아님: " + std::to_wstring(patchX) + L"," + std::to_wstring(patchY));
            const SDL_Palette* pal = SDL_GetSurfacePalette(surf);
            const std::uint32_t* pixels = static_cast<const std::uint32_t*>(surf->pixels);
            const int srcStride = surf->w;

            //글로벌 그리드 좌상단 기준의 이 패치 시작 픽셀 좌표
            const int baseX = (patchX - PATCH_X_MIN) * PATCH_PIXEL;
            const int baseY = (patchY - PATCH_Y_MIN) * PATCH_PIXEL;

            for (int py = 0; py < PATCH_PIXEL; ++py)
            {
                Terrain* row = dst + static_cast<std::size_t>(baseY + py) * dstStride + baseX;
                const std::uint32_t* srcRow = pixels + static_cast<std::size_t>(py) * srcStride;
                for (int px = 0; px < PATCH_PIXEL; ++px)
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

    //PNG 5832장을 디코드해 933MB 그리드 구성. onPatch default no-op이면 출력 영향 없음.
    //grid는 콜백 시점까지의 부분 로드 상태가 그대로 노출됨(미리보기 점진 갱신용).
    PixelCostGrid loadWorldGridFromPng(PatchLoadSink onPatch)
    {
        const __int64 tStart = getNanoTimer();

        //--- 1. alloc + Sea 디폴트 fill ---
        PixelCostGrid grid;
        const std::size_t total =
            static_cast<std::size_t>(PixelCostGrid::W) * PixelCostGrid::H;
        grid.data = std::make_unique<Terrain[]>(total);
        std::fill_n(grid.data.get(), total, Terrain::Sea);

        const __int64 tAlloc = getNanoTimer();

        //--- 2. 패치 PNG 5832장 디코드 ---
        int loadOk = 0;
        int loadFail = 0;
        const int totalPatches =
            (PATCH_Y_MAX - PATCH_Y_MIN + 1) * (PATCH_X_MAX - PATCH_X_MIN + 1);
        int donePatches = 0;
        for (int sy = PATCH_Y_MIN; sy <= PATCH_Y_MAX; ++sy)
        {
            for (int sx = PATCH_X_MIN; sx <= PATCH_X_MAX; ++sx)
            {
                if (blitPatchInto(sx, sy, grid.data.get(), PixelCostGrid::W)) ++loadOk;
                else ++loadFail;
                ++donePatches;
                if (onPatch) onPatch(donePatches, totalPatches, sx, sy, grid);
            }
        }

        const __int64 tLoaded = getNanoTimer();

        //--- 3. 디버그 히스토그램 (Terrain 분포 검증) ---
        constexpr int TERRAIN_COUNT = 16;
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

        prt(L"[procGen] loadWorldGridFromPng done\n");
        prt(L"  alloc+fill : %8.2f ms\n", allocMs);
        prt(L"  patch load : %8.2f ms  (ok=%d fail=%d / %d)\n",
            loadMs, loadOk, loadFail, loadOk + loadFail);
        prt(L"  histogram  : %8.2f ms\n", histMs);
        prt(L"  total      : %8.2f ms  (%.2f s)\n", totalMs, totalMs / 1000.0);
        prt(L"  memory     : %8.1f MB\n", memMB);

        if (loadFail > 0)
        {
            const SDL_Color warn{ 0xff, 0x60, 0x60, 0xff };
            prt(warn, L"  [WARN] %d patch PNG(s) missing/failed - filled with Sea\n", loadFail);
        }

        const wchar_t* const names[TERRAIN_COUNT] = {
            L"Land", L"Sea", L"River", L"Lake", L"CityZone", L"CityCenter", L"CityRiver", L"CitySea",
            L"Mountain", L"Polar", L"Tundra", L"Subarctic", L"Monsoon", L"InsularRainforest",
            L"Desert", L"ContinentalRainforest"
        };
        prt(L"  pixel terrain distribution:\n");
        for (int i = 0; i < TERRAIN_COUNT; ++i)
        {
            const double pct = 100.0 * static_cast<double>(hist[i])
                                     / static_cast<double>(total);
            prt(L"    %-22ls: %12llu  (%6.2f%%)\n", names[i], hist[i], pct);
        }

        return grid;
    }
}
