module;
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

module procGen;

import std;
import util;

//============================================================
// mmap 픽셀 그리드 — Phase 1 후 진입, Phase 2 게임플레이 픽셀 접근.
//   세션 임시 파일 (map/worldPixels.bin) → CreateFileMapping → MapViewOfFile.
//   접근 패턴: 플레이어 주변 페이지(4KB)만 OS가 lazy 로드, 콜드 페이지 자동 evict.
//============================================================
namespace procGen
{
    namespace
    {
        constexpr std::size_t kMmapBytes =
            static_cast<std::size_t>(PixelCostGrid::W) * PixelCostGrid::H;

        std::filesystem::path mmapFilePath()
        {
            return std::filesystem::path("map") / "worldPixels.bin";
        }

        //전역 mmap 상태. shutdown 시까지 유지.
        struct MmapState
        {
            HANDLE         hFile   = INVALID_HANDLE_VALUE;
            HANDLE         hMap    = nullptr;
            const Terrain* base    = nullptr;

            bool active() const noexcept { return base != nullptr; }
        };

        MmapState& state() noexcept
        {
            static MmapState s;
            return s;
        }

        //디스크에 933MB 압축 없이 기록. 실패 시 false.
        bool writeUncompressedFile(const PixelCostGrid& grid, const std::filesystem::path& path)
        {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);

            std::ofstream f(path, std::ios::binary | std::ios::trunc);
            if (!f) return false;

            const char* src = reinterpret_cast<const char*>(grid.data.get());
            //64MB 청크로 나눠 쓰기 — ofstream::write는 size_t 인자라 933MB 한 번에 가능하지만
            //일부 환경에서 4GB 미만이라도 큰 단일 write가 실패하는 경우 회피.
            constexpr std::size_t kChunk = 64ULL * 1024 * 1024;
            std::size_t off = 0;
            while (off < kMmapBytes)
            {
                const std::size_t cur = std::min(kChunk, kMmapBytes - off);
                f.write(src + off, static_cast<std::streamsize>(cur));
                if (!f) return false;
                off += cur;
            }
            f.flush();
            return static_cast<bool>(f);
        }

        //파일을 read-only mmap 진입. 실패 시 모든 핸들 정리 후 false.
        bool openMmap(const std::filesystem::path& path, MmapState& s)
        {
            const std::wstring wpath = path.wstring();

            s.hFile = CreateFileW(
                wpath.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);
            if (s.hFile == INVALID_HANDLE_VALUE) return false;

            s.hMap = CreateFileMappingW(
                s.hFile,
                nullptr,
                PAGE_READONLY,
                0, 0,                //크기 0 = 파일 전체
                nullptr);
            if (!s.hMap)
            {
                CloseHandle(s.hFile);
                s.hFile = INVALID_HANDLE_VALUE;
                return false;
            }

            void* view = MapViewOfFile(s.hMap, FILE_MAP_READ, 0, 0, 0);
            if (!view)
            {
                CloseHandle(s.hMap);
                CloseHandle(s.hFile);
                s.hMap = nullptr;
                s.hFile = INVALID_HANDLE_VALUE;
                return false;
            }

            s.base = static_cast<const Terrain*>(view);
            return true;
        }

        void closeMmap(MmapState& s) noexcept
        {
            if (s.base)   { UnmapViewOfFile(s.base); s.base = nullptr; }
            if (s.hMap)   { CloseHandle(s.hMap);     s.hMap = nullptr; }
            if (s.hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(s.hFile);
                s.hFile = INVALID_HANDLE_VALUE;
            }
        }
    }

    bool transitionToMmap(const PixelCostGrid& heapGrid)
    {
        const __int64 tStart = getNanoTimer();

        //이미 mmap 진입 상태면 한 번 닫고 재진입 (월드 재생성 케이스 대비).
        closeMmap(state());

        const auto path = mmapFilePath();

        //--- 1. 디스크 기록 ---
        if (!writeUncompressedFile(heapGrid, path))
        {
            prt(L"[procGen] transitionToMmap: write failed\n");
            return false;
        }
        const __int64 tWrote = getNanoTimer();

        //--- 2. mmap 진입 ---
        if (!openMmap(path, state()))
        {
            prt(L"[procGen] transitionToMmap: openMmap failed\n");
            //손상된 파일이면 다음 시도 위해 삭제.
            std::error_code ec;
            std::filesystem::remove(path, ec);
            return false;
        }
        const __int64 tMapped = getNanoTimer();

        const double writeMs  = (tWrote  - tStart ) / 1.0e6;
        const double mapMs    = (tMapped - tWrote ) / 1.0e6;
        const double totalMs  = (tMapped - tStart ) / 1.0e6;
        prt(L"[procGen] transitionToMmap: ok\n");
        prt(L"  write 933MB    : %8.2f ms\n", writeMs);
        prt(L"  CreateFileMap  : %8.2f ms\n", mapMs);
        prt(L"  total          : %8.2f ms\n", totalMs);

        return true;
    }

    void shutdownWorldPixelMmap() noexcept
    {
        closeMmap(state());
        std::error_code ec;
        std::filesystem::remove(mmapFilePath(), ec);
    }

    Terrain worldPixel(int px, int py) noexcept
    {
        const MmapState& s = state();
        if (!s.base) return Terrain::Sea;
        // Y는 양극(보이지않는 벽)이므로 범위 밖 = Sea. X는 시암 wrap.
        if (py < 0 || py >= PixelCostGrid::H) return Terrain::Sea;
        px = worldWrap::wrapPixelX(px);
        return s.base[static_cast<std::size_t>(py) * PixelCostGrid::W + px];
    }

    bool worldPixelMmapActive() noexcept
    {
        return state().active();
    }
}
