module worldGen;

import std;
import util;

using namespace worldGrid;  // Terrain, PixelCostGrid, loadWorldGrid, transitionToMmap 등 unqualified 접근

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
namespace worldGen
{
    namespace
    {
        //--- forward declarations (Stepdown — 정의는 generateWorld 아래) ---
        void updatePatchPreview(const PixelCostGrid& grid, int patchX, int patchY, std::mutex& mtx, std::vector<std::uint32_t>& dstRGBA);
        constexpr std::uint32_t terrainPreviewColor(Terrain t) noexcept;
        constexpr std::uint32_t packPreviewRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept;
    }

    void generateWorld(std::uint64_t seed, WorldGenProgress& progress)
    {
        //--- Phase 1: PNG 로드 ---
        progress.phase.store(GenPhase::loadPng, std::memory_order_release);

        //미리보기 버퍼를 alpha=0(투명)으로 미리 alloc → 즉시 표시 가능 상태로 진입.
        //patchLoadSink에서 패치 1장씩 부분 갱신 → 위성이 점진적으로 그려지는 효과.
        {
            std::lock_guard<std::mutex> lk(progress.previewMtx);
            progress.previewRGBA.assign(
                static_cast<std::size_t>(PREVIEW_W) * PREVIEW_H, 0u);
        }
        progress.previewReady.store(true, std::memory_order_release);

        PixelCostGrid grid = loadWorldGrid([&](int loaded, int total, int sx, int sy, const PixelCostGrid& g)
        {
            progress.patchesLoadedDone .store(loaded, std::memory_order_relaxed);
            progress.patchesLoadedTotal.store(total , std::memory_order_relaxed);

            //방금 로드된 패치의 10×10 블록만 미리보기에 반영 + 버전 bump.
            updatePatchPreview(g, sx, sy, progress.previewMtx, progress.previewRGBA);
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

        //--- Phase 1~3 산출물 저장 ---
        progress.result = WorldGenResult{ std::move(cities), std::move(roads) };

        //--- mmap 진입 — 933MB heap → 디스크 임시 파일 + mmap → heap free ---
        //  성공 시: Phase 2 게임플레이는 worldGrid::worldPixel() 통해 lazy 페이지 폴트로 픽셀 접근.
        //  실패 시: heap grid 그대로 유지 (워커 스레드 종료 시 RAII로 free) — 폴백.
        //          worldPixel은 Sea 반환하므로 게임은 동작하나 색상 데이터 무효.
        if (transitionToMmap(grid))
        {
            grid.data.reset();   //933MB heap 즉시 회수
        }

        //  Phase 4 (prepareSpawn) + done 설정은 *호출자*(WorldGenScreen 워커)가 처리.
        //  worldGen 모듈은 Sector 모듈을 import할 수 없으므로 (Sector → worldGen 단방향 의존),
        //  스폰 주변 섹터 사전 절차생성은 호출자 측 책임.
    }

    namespace
    {
        //패치 ↔ 미리보기 블록 매핑 상수.
        //  43200 = 108 patch * 400px. 1080 / 108 = 10 → 패치 1장 = 미리보기 10×10.
        //  21600 =  54 patch * 400px.  540 /  54 = 10 → 정확히 정합.
        constexpr int PREVIEW_PER_PATCH = PATCH_PIXEL * PREVIEW_W / PixelCostGrid::W;  // = 10
        static_assert(PATCH_PIXEL * PREVIEW_W % PixelCostGrid::W == 0,
                      "패치 픽셀이 미리보기 해상도와 정합되지 않음 — 비율 어긋남");

        //방금 로드된 패치 1장(400×400 source) → 미리보기 10×10 블록 갱신.
        //  각 미리보기 픽셀은 40×40 source 블록의 center 1픽셀을 nearest 샘플.
        //  (평균을 안 쓰는 이유: CityCenter 같은 단일 픽셀 마커가 사라짐.)
        //  로컬 스택 버퍼에 먼저 계산 → 짧게 락 잡고 메인 버퍼에 복사. 락 시간 < 1µs.
        void updatePatchPreview(const PixelCostGrid& grid, int patchX, int patchY, std::mutex& mtx, std::vector<std::uint32_t>& dstRGBA)
        {
            const int sIdxX = patchX - PATCH_X_MIN;             // 0..107
            const int sIdxY = patchY - PATCH_Y_MIN;             // 0..53
            const int blkX0 = sIdxX * PREVIEW_PER_PATCH;        // preview x 시작
            const int blkY0 = sIdxY * PREVIEW_PER_PATCH;        // preview y 시작

            //로컬 버퍼에 색 미리 계산 (락 밖에서 grid 읽기 — grid는 워커 단일 소유라 안전)
            std::uint32_t local[PREVIEW_PER_PATCH * PREVIEW_PER_PATCH];
            for (int dy = 0; dy < PREVIEW_PER_PATCH; ++dy)
            {
                const int srcY = (blkY0 + dy) * (PixelCostGrid::H / PREVIEW_H)
                               + (PixelCostGrid::H / PREVIEW_H) / 2;
                for (int dx = 0; dx < PREVIEW_PER_PATCH; ++dx)
                {
                    const int srcX = (blkX0 + dx) * (PixelCostGrid::W / PREVIEW_W)
                                   + (PixelCostGrid::W / PREVIEW_W) / 2;
                    local[dy * PREVIEW_PER_PATCH + dx] = terrainPreviewColor(grid.at(srcX, srcY));
                }
            }

            //짧게 락 잡고 메인 버퍼에 줄단위 복사
            std::lock_guard<std::mutex> lk(mtx);
            for (int dy = 0; dy < PREVIEW_PER_PATCH; ++dy)
            {
                std::uint32_t* dstRow = dstRGBA.data()
                    + static_cast<std::size_t>(blkY0 + dy) * PREVIEW_W + blkX0;
                std::memcpy(dstRow, &local[dy * PREVIEW_PER_PATCH],
                            PREVIEW_PER_PATCH * sizeof(std::uint32_t));
            }
        }

        //Terrain → 미리보기 RGBA 색 매핑 (Map.ixx의 biomeColor 톤 참조).
        constexpr std::uint32_t terrainPreviewColor(Terrain t) noexcept
        {
            switch (t)
            {
            case Terrain::Land:        return packPreviewRGBA(0xc0, 0xd7, 0xa8);
            case Terrain::Sea:         return packPreviewRGBA(0x55, 0x84, 0xad);
            case Terrain::River:       return packPreviewRGBA(0x89, 0xb4, 0xc8);
            case Terrain::Lake:        return packPreviewRGBA(0x6f, 0x6a, 0xb8);   //#9384e5의 미리보기 톤 — 강과 구분되는 보라빛
            case Terrain::CityZone:    return packPreviewRGBA(0xe6, 0xe2, 0xda);
            case Terrain::CityCenter:  return packPreviewRGBA(0xff, 0x60, 0x60);
            case Terrain::CityRiver:   return packPreviewRGBA(0xa6, 0xc1, 0xea);   //#a2bfef의 미리보기 톤 — 도시 내 강 강조
            case Terrain::CitySea:     return packPreviewRGBA(0x73, 0x70, 0xb8);   //#6d6abd의 미리보기 톤 — 도시 내 해협 강조
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

        //little-endian RGBA32 = 0xAABBGGRR. SDL_PIXELFORMAT_RGBA32와 정합.
        constexpr std::uint32_t packPreviewRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
        {
            return (std::uint32_t(0xff) << 24)
                 | (std::uint32_t(b)    << 16)
                 | (std::uint32_t(g)    <<  8)
                 |  std::uint32_t(r);
        }
    }
}
