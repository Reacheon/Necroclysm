module procGen;

import std;
import util;

//============================================================
// generateWorld — 월드 생성 워커 스레드 진입점.
//   메인 스레드에서 WorldGenScreen GUI를 띄운 뒤 std::jthread로 호출.
//   각 단계 시작/끝에 progress.phase를 갱신, 도시/도로는 콜백으로 누적.
//   PNG 로드 직후 미리보기 RGBA를 계산해 progress.previewRGBA에 저장.
//   메인 스레드는 매 프레임 progress의 atomic/스냅샷만 읽음.
//
//   prt() 등 std::wprintf 계열은 스레드 안전이 보장되지 않으므로 워커에서는
//   호출하지 않음. 로그는 각 phase 함수 내부의 prt가 처리.
//============================================================
namespace procGen
{
    namespace
    {
        //Terrain → 미리보기 RGBA 색 매핑 (Map.ixx의 biomeColor 톤 참조).
        //  little-endian RGBA32 = 0xAABBGGRR. SDL_PIXELFORMAT_RGBA32와 정합.
        constexpr std::uint32_t packPreviewRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
        {
            return (std::uint32_t(0xff) << 24)
                 | (std::uint32_t(b)    << 16)
                 | (std::uint32_t(g)    <<  8)
                 |  std::uint32_t(r);
        }

        constexpr std::uint32_t terrainPreviewColor(Terrain t) noexcept
        {
            switch (t)
            {
            case Terrain::Land:        return packPreviewRGBA(0xc0, 0xd7, 0xa8);
            case Terrain::Sea:         return packPreviewRGBA(0x55, 0x84, 0xad);
            case Terrain::FreshWater:  return packPreviewRGBA(0x89, 0xb4, 0xc8);
            case Terrain::Bridge:      return packPreviewRGBA(0xc8, 0xc3, 0xbc);
            case Terrain::CityZone:    return packPreviewRGBA(0xe6, 0xe2, 0xda);
            case Terrain::CityCenter:  return packPreviewRGBA(0xff, 0x60, 0x60);
            case Terrain::Mountain:    return packPreviewRGBA(0x8a, 0x6a, 0x52);
            case Terrain::Polar:       return packPreviewRGBA(0xf2, 0xf6, 0xff);
            case Terrain::Tundra:      return packPreviewRGBA(0x8e, 0xc6, 0xcd);
            case Terrain::Subarctic:   return packPreviewRGBA(0x6e, 0x9b, 0xc8);
            case Terrain::Monsoon:               return packPreviewRGBA(0x96, 0xa3, 0x55);
            case Terrain::InsularRainforest:     return packPreviewRGBA(0x35, 0x77, 0x49);
            case Terrain::Desert:                return packPreviewRGBA(0xe8, 0xd9, 0x7a);
            case Terrain::ContinentalRainforest: return packPreviewRGBA(0x1f, 0x4a, 0x1a);
            }
            return packPreviewRGBA(0x10, 0x10, 0x10);
        }

        //섹터 ↔ 미리보기 블록 매핑 상수.
        //  43200 = 108 sector * 400px. 1080 / 108 = 10 → 섹터 1장 = 미리보기 10×10.
        //  21600 =  54 sector * 400px.  540 /  54 = 10 → 정확히 정합.
        constexpr int SECTOR_X_MIN_LOCAL  = -54;
        constexpr int SECTOR_Y_MIN_LOCAL  = -27;
        constexpr int SECTOR_PIXEL_LOCAL  = 400;
        constexpr int PREVIEW_PER_SECTOR  = SECTOR_PIXEL_LOCAL * PREVIEW_W / PixelCostGrid::W;  // = 10
        static_assert(SECTOR_PIXEL_LOCAL * PREVIEW_W % PixelCostGrid::W == 0,
                      "섹터 픽셀이 미리보기 해상도와 정합되지 않음 — 비율 어긋남");

        //방금 로드된 섹터 1장(400×400 source) → 미리보기 10×10 블록 갱신.
        //  각 미리보기 픽셀은 40×40 source 블록의 center 1픽셀을 nearest 샘플.
        //  (평균을 안 쓰는 이유: CityCenter 같은 단일 픽셀 마커가 사라짐.)
        //  로컬 스택 버퍼에 먼저 계산 → 짧게 락 잡고 메인 버퍼에 복사. 락 시간 < 1µs.
        void updateSectorPreview(const PixelCostGrid& grid,
                                 int sectorX, int sectorY,
                                 std::mutex& mtx,
                                 std::vector<std::uint32_t>& dstRGBA)
        {
            const int sIdxX = sectorX - SECTOR_X_MIN_LOCAL;     // 0..107
            const int sIdxY = sectorY - SECTOR_Y_MIN_LOCAL;     // 0..53
            const int blkX0 = sIdxX * PREVIEW_PER_SECTOR;       // preview x 시작
            const int blkY0 = sIdxY * PREVIEW_PER_SECTOR;       // preview y 시작

            //로컬 버퍼에 색 미리 계산 (락 밖에서 grid 읽기 — grid는 워커 단일 소유라 안전)
            std::uint32_t local[PREVIEW_PER_SECTOR * PREVIEW_PER_SECTOR];
            for (int dy = 0; dy < PREVIEW_PER_SECTOR; ++dy)
            {
                const int srcY = (blkY0 + dy) * (PixelCostGrid::H / PREVIEW_H)
                               + (PixelCostGrid::H / PREVIEW_H) / 2;
                for (int dx = 0; dx < PREVIEW_PER_SECTOR; ++dx)
                {
                    const int srcX = (blkX0 + dx) * (PixelCostGrid::W / PREVIEW_W)
                                   + (PixelCostGrid::W / PREVIEW_W) / 2;
                    local[dy * PREVIEW_PER_SECTOR + dx] = terrainPreviewColor(grid.at(srcX, srcY));
                }
            }

            //짧게 락 잡고 메인 버퍼에 줄단위 복사
            std::lock_guard<std::mutex> lk(mtx);
            for (int dy = 0; dy < PREVIEW_PER_SECTOR; ++dy)
            {
                std::uint32_t* dstRow = dstRGBA.data()
                    + static_cast<std::size_t>(blkY0 + dy) * PREVIEW_W + blkX0;
                std::memcpy(dstRow, &local[dy * PREVIEW_PER_SECTOR],
                            PREVIEW_PER_SECTOR * sizeof(std::uint32_t));
            }
        }
    }

    void generateWorld(std::uint64_t seed, WorldGenProgress& progress)
    {
        //--- Phase 1: PNG 로드 ---
        progress.phase.store(GenPhase::loadPng, std::memory_order_release);

        //미리보기 버퍼를 alpha=0(투명)으로 미리 alloc → 즉시 표시 가능 상태로 진입.
        //sectorLoadSink에서 섹터 1장씩 부분 갱신 → 위성이 점진적으로 그려지는 효과.
        {
            std::lock_guard<std::mutex> lk(progress.previewMtx);
            progress.previewRGBA.assign(
                static_cast<std::size_t>(PREVIEW_W) * PREVIEW_H, 0u);
        }
        progress.previewReady.store(true, std::memory_order_release);

        PixelCostGrid grid = loadWorldGrid([&](int loaded, int total, int sx, int sy, const PixelCostGrid& g)
        {
            progress.sectorsLoadedDone .store(loaded, std::memory_order_relaxed);
            progress.sectorsLoadedTotal.store(total , std::memory_order_relaxed);

            //방금 로드된 섹터의 10×10 블록만 미리보기에 반영 + 버전 bump.
            updateSectorPreview(g, sx, sy, progress.previewMtx, progress.previewRGBA);
            progress.previewVersion.fetch_add(1, std::memory_order_release);
        });

        //--- Phase 2: 도시 배치 ---
        progress.phase.store(GenPhase::placeCity, std::memory_order_release);

        std::vector<CityNode> cities = placeCities(seed, grid, [&](const CityNode& c)
        {
            std::lock_guard<std::mutex> lk(progress.citiesMtx);
            progress.citiesSnap.push_back(c);
        });

        //--- Phase 3: 도로망 ---
        progress.phase.store(GenPhase::buildRoad, std::memory_order_release);

        std::vector<RoadPolyLine> roads = buildRoadNetwork(seed, grid, cities, [&](const RoadPolyLine& r)
        {
            std::lock_guard<std::mutex> lk(progress.roadsMtx);
            progress.roadsSnap.push_back(r);
        });

        //도로망 튜닝용 디버그 가드. true면 done을 set하지 않고 return → WorldGenScreen이
        //닫히지 않고 도시+도로가 그려진 상태로 멈춤(QUIT으로만 종료). 튜닝 끝나면 제거.
        constexpr bool kDebugStopAfterRoads = true;
        if constexpr (kDebugStopAfterRoads)
        {
            return;
        }

        //--- 결과 저장 후 done ---
        progress.result = WorldGenResult{ std::move(cities), std::move(roads) };
        progress.phase.store(GenPhase::done, std::memory_order_release);
        progress.done .store(true,           std::memory_order_release);
    }
}
