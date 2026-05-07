module;
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <compressapi.h>
#pragma comment(lib, "Cabinet.lib")  //compressapi 링크 — vcxproj 수정 회피

module procGen;

import std;
import util;

//============================================================
// 월드 그리드 캐시 — 933MB 디코드 결과를 압축 저장 / 압축 해제 로드.
//   PNG 5832장의 mtime+filesize FNV-1a 해시로 stale 검증.
//   해시 mismatch / 파일 손상 / 헤더 불일치 시 PNG 디코드 fallback 후 캐시 재기록.
//   압축은 Windows 내장 compressapi (XPRESS_HUFF, Cabinet.lib).
//   16MB 청크 단위 — 단일 거대 버퍼 회피, 청크별 진행 표시도 가능한 구조.
//============================================================
namespace procGen
{
    namespace
    {
        //캐시 파일 포맷.
        constexpr std::uint32_t kCacheMagic    = 0x4757434EU;         //'NCWG' little-endian
        constexpr std::uint32_t kCacheVersion  = 3;                   //v3: CitySea 추가로 Terrain enum 바이트 값 재배치(이스탄불·홍콩식 도시 내 해협).
        constexpr std::size_t   kChunkBytes    = 16ULL * 1024 * 1024; //16MB / 청크
        //신뢰 못 할 헤더 값에 대한 sanity 상한 — chunk당 최대 32MB로 제한.
        constexpr std::size_t   kMaxChunkBytes = 32ULL * 1024 * 1024;

        struct CacheHeader
        {
            std::uint32_t magic;
            std::uint32_t version;
            std::uint32_t width;
            std::uint32_t height;
            std::uint64_t pngHash;
            std::uint64_t totalBytes;
            std::uint32_t numChunks;
            std::uint32_t reserved;
        };
        static_assert(sizeof(CacheHeader) == 40, "CacheHeader 패킹 불일치 — 컴파일러 정렬 확인");

        std::filesystem::path cacheFilePath()
        {
            return std::filesystem::path("map") / "worldPatch.cache";
        }

        std::string buildPatchPathLocal(int sx, int sy)
        {
            //buildPatchPath와 완전 동일한 규칙 — fingerprint가 실제 로더와 같은
            //파일 집합을 보도록 3자리 0-padding 사용.
            int number = PATCH_NUMBER_BIAS + sx + 108 * sy;
            return std::format("map/worldPatch-{:03d}.png", number);
        }

        //5832장 PNG의 mtime+size를 FNV-1a 64비트로 폴딩 — stale 검증용.
        //  파일 부재 시 0 누적 → 부재 자체가 해시에 반영되어 PNG 추가/삭제도 감지.
        std::uint64_t computePngFingerprint()
        {
            constexpr std::uint64_t kFnvOffset = 0xcbf29ce484222325ULL;
            constexpr std::uint64_t kFnvPrime  = 0x100000001b3ULL;
            std::uint64_t h = kFnvOffset;

            auto roll = [&](const void* data, std::size_t n) noexcept
            {
                const std::uint8_t* p = static_cast<const std::uint8_t*>(data);
                for (std::size_t i = 0; i < n; ++i)
                {
                    h ^= p[i];
                    h *= kFnvPrime;
                }
            };

            for (int sy = PATCH_Y_MIN; sy <= PATCH_Y_MAX; ++sy)
            {
                for (int sx = PATCH_X_MIN; sx <= PATCH_X_MAX; ++sx)
                {
                    std::string path = buildPatchPathLocal(sx, sy);
                    std::error_code ec;

                    auto sz = std::filesystem::file_size(path, ec);
                    std::uint64_t sizeVal = ec ? 0ULL : static_cast<std::uint64_t>(sz);
                    roll(&sizeVal, sizeof(sizeVal));

                    auto mt = std::filesystem::last_write_time(path, ec);
                    std::int64_t timeVal = ec
                        ? 0LL
                        : static_cast<std::int64_t>(mt.time_since_epoch().count());
                    roll(&timeVal, sizeof(timeVal));
                }
            }
            return h;
        }

        //캐시 hit 후 미리보기 콜백 일괄 재생 — GUI 미리보기 RGBA가 채워지도록.
        //  메인 GUI는 previewVersion만 보고 다음 프레임에 한 번에 텍스처 갱신함.
        void replayPatchPreview(const PixelCostGrid& grid, PatchLoadSink& onPatch)
        {
            if (!onPatch) return;
            const int totalPatches =
                (PATCH_Y_MAX - PATCH_Y_MIN + 1) *
                (PATCH_X_MAX - PATCH_X_MIN + 1);
            int done = 0;
            for (int sy = PATCH_Y_MIN; sy <= PATCH_Y_MAX; ++sy)
            {
                for (int sx = PATCH_X_MIN; sx <= PATCH_X_MAX; ++sx)
                {
                    ++done;
                    onPatch(done, totalPatches, sx, sy, grid);
                }
            }
        }

        //성공 시 grid 채우고 true. 실패 시 grid는 빈 상태로 두고 false.
        bool tryLoadFromCache(std::uint64_t expectedHash, PixelCostGrid& grid)
        {
            std::ifstream f(cacheFilePath(), std::ios::binary);
            if (!f) return false;

            CacheHeader hdr{};
            f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
            if (!f) return false;

            if (hdr.magic   != kCacheMagic)                      return false;
            if (hdr.version != kCacheVersion)                    return false;
            if (hdr.width   != static_cast<std::uint32_t>(PixelCostGrid::W)) return false;
            if (hdr.height  != static_cast<std::uint32_t>(PixelCostGrid::H)) return false;
            if (hdr.pngHash != expectedHash)                     return false;

            const std::uint64_t expectedTotal =
                static_cast<std::uint64_t>(PixelCostGrid::W) * PixelCostGrid::H;
            if (hdr.totalBytes != expectedTotal)                 return false;
            if (hdr.numChunks  == 0)                             return false;

            //검증 통과 후에만 alloc — miss 시 933MB alloc 비용 회피.
            grid.data = std::make_unique<Terrain[]>(static_cast<std::size_t>(expectedTotal));

            DECOMPRESSOR_HANDLE dec = nullptr;
            if (!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS_HUFF, nullptr, &dec))
            {
                grid.data.reset();
                return false;
            }

            std::uint8_t* dst = reinterpret_cast<std::uint8_t*>(grid.data.get());
            std::uint64_t writeOff = 0;
            std::vector<std::uint8_t> buf;

            auto fail = [&](DECOMPRESSOR_HANDLE h)
            {
                CloseDecompressor(h);
                grid.data.reset();
                return false;
            };

            for (std::uint32_t i = 0; i < hdr.numChunks; ++i)
            {
                std::uint32_t cmpSize = 0;
                std::uint32_t dstSize = 0;
                f.read(reinterpret_cast<char*>(&cmpSize), 4);
                f.read(reinterpret_cast<char*>(&dstSize), 4);
                if (!f) return fail(dec);

                //손상된 헤더가 거대 alloc 유발하지 않도록 sanity 검사.
                if (cmpSize == 0 || cmpSize > kMaxChunkBytes)            return fail(dec);
                if (dstSize == 0 || dstSize > kMaxChunkBytes)            return fail(dec);
                if (writeOff + dstSize > expectedTotal)                  return fail(dec);

                buf.resize(cmpSize);
                f.read(reinterpret_cast<char*>(buf.data()), cmpSize);
                if (!f) return fail(dec);

                SIZE_T outSize = 0;
                BOOL ok = Decompress(
                    dec,
                    buf.data(), cmpSize,
                    dst + writeOff, dstSize,
                    &outSize);
                if (!ok || outSize != dstSize) return fail(dec);
                writeOff += dstSize;
            }
            CloseDecompressor(dec);

            if (writeOff != expectedTotal)
            {
                grid.data.reset();
                return false;
            }
            return true;
        }

        //캐시 기록. 실패해도 throw 안 함 — 다음 실행에서 PNG 디코드 후 재시도.
        //  부분 기록된 손상 파일이 남으면 다음 로드 때 헤더/sanity 검사로 reject되어 PNG fallback.
        void writeCache(std::uint64_t pngHash, const PixelCostGrid& grid)
        {
            std::error_code ec;
            std::filesystem::create_directories(cacheFilePath().parent_path(), ec);

            std::ofstream f(cacheFilePath(), std::ios::binary | std::ios::trunc);
            if (!f) return;

            const std::uint64_t total =
                static_cast<std::uint64_t>(PixelCostGrid::W) * PixelCostGrid::H;
            const std::uint32_t numChunks =
                static_cast<std::uint32_t>((total + kChunkBytes - 1) / kChunkBytes);

            CacheHeader hdr{};
            hdr.magic      = kCacheMagic;
            hdr.version    = kCacheVersion;
            hdr.width      = static_cast<std::uint32_t>(PixelCostGrid::W);
            hdr.height     = static_cast<std::uint32_t>(PixelCostGrid::H);
            hdr.pngHash    = pngHash;
            hdr.totalBytes = total;
            hdr.numChunks  = numChunks;
            hdr.reserved   = 0;
            f.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
            if (!f) return;

            COMPRESSOR_HANDLE cmp = nullptr;
            if (!CreateCompressor(COMPRESS_ALGORITHM_XPRESS_HUFF, nullptr, &cmp)) return;

            const std::uint8_t* src = reinterpret_cast<const std::uint8_t*>(grid.data.get());
            std::vector<std::uint8_t> outBuf(kChunkBytes + 1024); //엔트로피 높은 청크는 약간 늘어날 수 있음.

            std::uint64_t off = 0;
            std::uint64_t totalCompressed = 0;
            while (off < total)
            {
                const std::size_t cur =
                    static_cast<std::size_t>(std::min<std::uint64_t>(kChunkBytes, total - off));

                SIZE_T outSize = 0;
                BOOL ok = Compress(cmp, src + off, cur, outBuf.data(), outBuf.size(), &outSize);
                if (!ok)
                {
                    //버퍼 부족이면 정확한 필요 크기 질의 후 재시도.
                    SIZE_T need = 0;
                    Compress(cmp, src + off, cur, nullptr, 0, &need);
                    if (need == 0) { CloseCompressor(cmp); return; }
                    outBuf.resize(need);
                    ok = Compress(cmp, src + off, cur, outBuf.data(), outBuf.size(), &outSize);
                    if (!ok) { CloseCompressor(cmp); return; }
                }

                std::uint32_t cmpSize = static_cast<std::uint32_t>(outSize);
                std::uint32_t dstSize = static_cast<std::uint32_t>(cur);
                f.write(reinterpret_cast<const char*>(&cmpSize), 4);
                f.write(reinterpret_cast<const char*>(&dstSize), 4);
                f.write(reinterpret_cast<const char*>(outBuf.data()), cmpSize);
                if (!f) { CloseCompressor(cmp); return; }

                totalCompressed += cmpSize;
                off += cur;
            }
            CloseCompressor(cmp);

            const double srcMB = static_cast<double>(total)           / (1024.0 * 1024.0);
            const double cmpMB = static_cast<double>(totalCompressed) / (1024.0 * 1024.0);
            prt(L"  cache write : %d chunks, %.1f MB -> %.1f MB (%.1f%% of orig)\n",
                static_cast<int>(numChunks), srcMB, cmpMB,
                srcMB > 0.0 ? 100.0 * cmpMB / srcMB : 0.0);
        }
    }

    //공개 진입점 — 캐시 우선 로드. miss 시 PNG 디코드 + 캐시 기록.
    //캐시 hit 시에도 onPatch 콜백을 5832회 재생해 미리보기 RGBA를 채운다.
    PixelCostGrid loadWorldGrid(PatchLoadSink onPatch)
    {
        const __int64 tStart = getNanoTimer();
        const std::uint64_t pngHash = computePngFingerprint();
        const __int64 tHash = getNanoTimer();
        const double hashMs = (tHash - tStart) / 1.0e6;

        //--- 캐시 시도 ---
        PixelCostGrid grid;
        if (tryLoadFromCache(pngHash, grid))
        {
            const __int64 tDecomp = getNanoTimer();

            //미리보기 갱신 — 5832회 콜백 (GUI는 다음 프레임에 한 번에 반영).
            replayPatchPreview(grid, onPatch);

            const __int64 tEnd = getNanoTimer();
            prt(L"[procGen] loadWorldGrid: cache HIT\n");
            prt(L"  png fingerprint: %8.2f ms\n", hashMs);
            prt(L"  decompress     : %8.2f ms\n", (tDecomp - tHash ) / 1.0e6);
            prt(L"  preview replay : %8.2f ms\n", (tEnd    - tDecomp) / 1.0e6);
            prt(L"  total          : %8.2f ms (%.2f s)\n",
                (tEnd - tStart) / 1.0e6, (tEnd - tStart) / 1.0e9);
            return grid;
        }

        //--- 캐시 miss → PNG 디코드 → 캐시 기록 ---
        prt(L"[procGen] loadWorldGrid: cache MISS — PNG decode + write\n");
        prt(L"  png fingerprint: %8.2f ms\n", hashMs);

        grid = loadWorldGridFromPng(onPatch);

        const __int64 tBeforeWrite = getNanoTimer();
        writeCache(pngHash, grid);
        const __int64 tAfterWrite = getNanoTimer();
        prt(L"  cache write    : %8.2f ms\n", (tAfterWrite - tBeforeWrite) / 1.0e6);

        return grid;
    }
}
