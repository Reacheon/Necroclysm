module;
#include <SDL3/SDL.h>

export module WorldGenScreen;

import std;
import util;
import constVar;
import GUI;
import drawText;
import globalVar;
import procGen;

// ════════════════════════════════════════════════════════════════════════
// WorldGenScreen — 게임 시작 1회 절차생성 진행 화면
//   메인 스레드: 매 프레임 progress 스냅샷을 읽어 화면 갱신
//   워커 스레드(별도): generateWorld가 PNG 로드 → 도시 배치 → 도로망 순차 실행
//
//   레이아웃: 풀스크린 검은 배경 + 중앙 다운샘플 위성지도(1080×540) +
//            도시 빨간 점 + 도로 하늘색 라인 + 하단 phase 텍스트(점 . .. ...)
//
//   화면 표시는 워커가 뭉텅이로 push해도 1프레임당 25개 정도씩 부드럽게
//   페이싱(displayedCity/Road) — 자세한 건 step() 안의 catchUp 로직.
// ════════════════════════════════════════════════════════════════════════

namespace wgcfg
{
    inline constexpr SDL_Color BG          = {  10,  10,  14, 255 };
    inline constexpr SDL_Color CITY_DOT    = { 230,  60,  60, 255 };
    inline constexpr SDL_Color ROAD_LINE   = { 110, 200, 240, 255 };
    inline constexpr SDL_Color TEXT_MAIN   = { 235, 235, 230, 255 };
    inline constexpr SDL_Color TEXT_SUB    = { 150, 150, 155, 255 };
    inline constexpr SDL_Color MAP_BORDER  = {  60,  60,  70, 255 };

    //페이싱 — 60fps 기준 초당 도시 ~1500개. 3000개는 약 2초에 다 등장.
    inline constexpr double CITY_RATE_BASE     = 25.0;
    inline constexpr double CITY_DEFICIT_DIV   = 30.0;  // 0.5초 안에 따라잡기
    inline constexpr double CITY_FINAL_DIV     = 10.0;  // phase 진행되면 빨리 정리
    inline constexpr double ROAD_RATE_BASE     = 4.0;
    inline constexpr double ROAD_DEFICIT_DIV   = 30.0;
}

export class WorldGenScreen : public GUI
{
private:
    inline static WorldGenScreen* ptr = nullptr;

    std::shared_ptr<procGen::WorldGenProgress> progress;
    std::jthread worker;

    //미리보기 텍스처 — 워커가 섹터별로 채우는 RGBA를 메인이 점진 반영.
    //  최초 생성: previewReady가 true가 된 직후 1회.
    //  이후 갱신: progress->previewVersion이 lastUploadedPreviewVersion과
    //  다르면 SDL_UpdateTexture로 전체 픽셀 재업로드.
    //  미로드 영역은 alpha=0이라 BLEND 모드에서 자연스럽게 투명 표시됨.
    SDL_Texture* previewTex = nullptr;
    int          lastUploadedPreviewVersion = -1;

    //부드러운 등장 페이싱 카운터
    double displayedCityCount = 0.0;
    double displayedRoadCount = 0.0;

    //월드 생성 완료 후 startArea 후속 처리를 호출할 콜백
    std::function<void(procGen::WorldGenResult)> onCompleted;
    bool completedFired = false;

    //--- 좌표 변환 헬퍼: 픽셀 좌표(0..43200) → 스크린 좌표 ---
    SDL_Rect mapRect() const
    {
        // 화면에 들어가는 최대 1080x540 비례. 여백 80px.
        const int margin = 80;
        const int maxW = cameraW - margin * 2;
        const int maxH = cameraH - margin * 2 - 80; //하단 텍스트 영역 확보

        // 2:1 비율 유지. min(maxW, maxH*2) 가 가로 길이.
        int w = std::min(maxW, maxH * 2);
        int h = w / 2;
        return SDL_Rect{
            (cameraW - w) / 2,
            (cameraH - h) / 2 - 30, //텍스트 자리 위로 살짝 올리기
            w, h
        };
    }

    SDL_FPoint pixelToScreen(int px, int py) const
    {
        const SDL_Rect r = mapRect();
        const double fx = (double)px / (double)procGen::WORLD_PIXEL_W;
        const double fy = (double)py / (double)procGen::WORLD_PIXEL_H;
        return SDL_FPoint{
            (float)(r.x + fx * r.w),
            (float)(r.y + fy * r.h)
        };
    }

    SDL_FPoint tileToScreen(const Point3& t) const
    {
        // procGen 픽셀 베이스: 섹터 (-54,-27) 좌상단이 픽셀(0,0)
        constexpr int SECTOR_X_MIN = -54;
        constexpr int SECTOR_Y_MIN = -27;
        constexpr int PIXEL_PER_SECTOR = 400;
        constexpr int TILE_BASE_X =
            SECTOR_X_MIN * PIXEL_PER_SECTOR * procGen::TILES_PER_PIXEL;
        constexpr int TILE_BASE_Y =
            SECTOR_Y_MIN * PIXEL_PER_SECTOR * procGen::TILES_PER_PIXEL;

        const double pxd = (double)(t.x - TILE_BASE_X) / (double)procGen::TILES_PER_PIXEL;
        const double pyd = (double)(t.y - TILE_BASE_Y) / (double)procGen::TILES_PER_PIXEL;

        const SDL_Rect r = mapRect();
        const double fx = pxd / (double)procGen::WORLD_PIXEL_W;
        const double fy = pyd / (double)procGen::WORLD_PIXEL_H;
        return SDL_FPoint{
            (float)(r.x + fx * r.w),
            (float)(r.y + fy * r.h)
        };
    }

    //--- phase별 표시 텍스트 ---
    static const wchar_t* phaseLabel(procGen::GenPhase ph)
    {
        switch (ph)
        {
        case procGen::GenPhase::idle:      return L"Initializing";
        case procGen::GenPhase::loadPng:   return L"Loading satellite imagery";
        case procGen::GenPhase::placeCity: return L"Placing cities";
        case procGen::GenPhase::buildRoad: return L"Building road network";
        case procGen::GenPhase::done:      return L"Finalizing world";
        }
        return L"";
    }

public:
    //onWorldReady 콜백은 done 시점에 메인 스레드에서 호출됨(WorldGenResult 결과 인계).
    WorldGenScreen(std::uint64_t seed, std::function<void(procGen::WorldGenResult)> onWorldReady)
        : GUI(false)
        , progress(std::make_shared<procGen::WorldGenProgress>())
        , onCompleted(std::move(onWorldReady))
    {
        errorBox(ptr != nullptr, L"More than one WorldGenScreen instance was generated.");
        ptr = this;
        x = 0; y = 0;

        //오픈 애니메이션 없이 즉시 표시. 입력은 차단(생성 중에 메뉴 못 열게).
        deactInput();

        //워커 스레드 기동 — shared_ptr 캡처로 수명 안전(jthread 소멸자가 join)
        auto progPtr = progress;
        worker = std::jthread([seed, progPtr]
        {
            procGen::generateWorld(seed, *progPtr);
        });
    }

    ~WorldGenScreen() override
    {
        //jthread 소멸자가 join까지 보장 — 화면이 죽기 전에 워커는 이미 done 상태
        if (previewTex) { SDL_DestroyTexture(previewTex); previewTex = nullptr; }
        ptr = nullptr;
    }

    static WorldGenScreen* ins() { return ptr; }
    bool hasFinished() const { return completedFired; }

    void changeXY(int /*ix*/, int /*iy*/, bool /*center*/) override { x = 0; y = 0; }

    void step() override
    {
        if (!progress) return;

        //--- 1) 미리보기 텍스처 — 최초 생성 + 점진 업데이트 ---
        //   최초 1회: 빈 STREAMING 텍스처 생성 + BLEND 모드 (alpha=0 영역 투명).
        //   이후: previewVersion 변할 때만 SDL_UpdateTexture로 전체 재업로드.
        if (progress->previewReady.load(std::memory_order_acquire))
        {
            if (!previewTex)
            {
                previewTex = SDL_CreateTexture(renderer,
                    SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_STREAMING,
                    procGen::PREVIEW_W, procGen::PREVIEW_H);
                if (previewTex)
                {
                    SDL_SetTextureBlendMode(previewTex, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureScaleMode(previewTex, SDL_SCALEMODE_LINEAR);
                }
            }

            const int curVer = progress->previewVersion.load(std::memory_order_acquire);
            if (previewTex && curVer != lastUploadedPreviewVersion)
            {
                std::lock_guard<std::mutex> lk(progress->previewMtx);
                if (!progress->previewRGBA.empty())
                {
                    SDL_UpdateTexture(previewTex, nullptr,
                        progress->previewRGBA.data(),
                        procGen::PREVIEW_W * 4);
                    lastUploadedPreviewVersion = curVer;
                }
            }
        }

        //--- 2) 도시/도로 부드러운 등장 페이싱 ---
        const auto ph = progress->phase.load(std::memory_order_acquire);

        int cityActual = 0;
        {
            std::lock_guard<std::mutex> lk(progress->citiesMtx);
            cityActual = (int)progress->citiesSnap.size();
        }
        {
            const double deficit = (double)cityActual - displayedCityCount;
            double rate = wgcfg::CITY_RATE_BASE;
            if (deficit > 0)
                rate = std::max(rate, deficit / wgcfg::CITY_DEFICIT_DIV);
            //phase가 도시 단계를 지났으면 잔여분 빠르게 정리
            if (ph == procGen::GenPhase::buildRoad || ph == procGen::GenPhase::done)
                rate = std::max(rate, deficit / wgcfg::CITY_FINAL_DIV);
            displayedCityCount = std::min((double)cityActual, displayedCityCount + rate);
        }

        int roadActual = 0;
        {
            std::lock_guard<std::mutex> lk(progress->roadsMtx);
            roadActual = (int)progress->roadsSnap.size();
        }
        {
            const double deficit = (double)roadActual - displayedRoadCount;
            double rate = wgcfg::ROAD_RATE_BASE;
            if (deficit > 0)
                rate = std::max(rate, deficit / wgcfg::ROAD_DEFICIT_DIV);
            displayedRoadCount = std::min((double)roadActual, displayedRoadCount + rate);
        }

        //--- 3) 완료 감지 → 결과 인계 → 자기 자신 close ---
        if (!completedFired
            && progress->done.load(std::memory_order_acquire)
            && displayedCityCount >= cityActual
            && displayedRoadCount >= roadActual)
        {
            completedFired = true;

            //결과 인계 — onCompleted는 startArea_post를 부르는 람다일 수 있음.
            //  result는 std::optional이라 has_value로 가드.
            if (progress->result.has_value() && onCompleted)
            {
                onCompleted(std::move(*progress->result));
            }

            //입력 차단 풀기 + GUI 제거
            close(aniFlag::null);
        }
    }

    void drawGUI() override
    {
        if (getStateDraw() == false) return;

        // 풀스크린 검정 배경
        drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, wgcfg::BG);

        const SDL_Rect r = mapRect();

        //--- 미리보기 텍스처 (미로드 영역은 alpha=0이라 BG가 그대로 비침) ---
        if (previewTex)
        {
            SDL_FRect dst{ (float)r.x, (float)r.y, (float)r.w, (float)r.h };
            SDL_RenderTexture(renderer, previewTex, nullptr, &dst);
        }
        drawRect(r, wgcfg::MAP_BORDER);

        //--- 도로 하늘색 라인 ---
        if (progress)
        {
            std::lock_guard<std::mutex> lk(progress->roadsMtx);
            const int n = std::min((int)progress->roadsSnap.size(), (int)displayedRoadCount);
            SDL_SetRenderDrawColor(renderer,
                wgcfg::ROAD_LINE.r, wgcfg::ROAD_LINE.g, wgcfg::ROAD_LINE.b, wgcfg::ROAD_LINE.a);
            for (int i = 0; i < n; ++i)
            {
                const auto& v = progress->roadsSnap[i].verts;
                for (std::size_t k = 1; k < v.size(); ++k)
                {
                    SDL_FPoint a = tileToScreen(v[k - 1]);
                    SDL_FPoint b = tileToScreen(v[k]);
                    SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
                }
            }
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        }

        //--- 도시 빨간 점 ---
        if (progress)
        {
            std::lock_guard<std::mutex> lk(progress->citiesMtx);
            const int n = std::min((int)progress->citiesSnap.size(), (int)displayedCityCount);
            for (int i = 0; i < n; ++i)
            {
                const SDL_FPoint sp = tileToScreen(progress->citiesSnap[i].center);
                //티어별 점 크기 차등 — T1=3, T2=2, T3=1
                int s = 1;
                switch (progress->citiesSnap[i].tier)
                {
                case procGen::CityTier::T1: s = 3; break;
                case procGen::CityTier::T2: s = 2; break;
                case procGen::CityTier::T3: s = 1; break;
                }
                drawFillRect(SDL_Rect{ (int)sp.x - s/2, (int)sp.y - s/2, s, s },
                             wgcfg::CITY_DOT);
            }
        }

        //--- 하단 phase 텍스트 + 점 . .. ... 애니메이션 ---
        const auto ph = progress ? progress->phase.load(std::memory_order_acquire)
                                 : procGen::GenPhase::idle;
        const wchar_t* lbl = phaseLabel(ph);

        const int dotCount = (int)((SDL_GetTicks() / 400) % 3) + 1;
        std::wstring dots(dotCount, L'.');

        const int textY = r.y + r.h + 32;
        const int subY  = textY + 30;

        setFont(fontType::mainFontBold);
        setFontSize(22);
        // 텍스트는 본문 + 점. 본문은 중앙 정렬, 점은 그 오른쪽에 고정 폭(점 3자리만큼 미리 자리 잡음).
        const std::wstring full = std::wstring(lbl) + L" ...";
        const int fullW = queryTextWidth(full);
        const int leftX = (cameraW - fullW) / 2;

        drawText(lbl, leftX, textY, wgcfg::TEXT_MAIN);
        const int lblW = queryTextWidth(lbl);
        drawText(L" " + dots, leftX + lblW, textY, wgcfg::TEXT_MAIN);

        //--- 서브 정보 — phase별 진행 카운트 ---
        setFont(fontType::mainFont);
        setFontSize(14);
        std::wostringstream sub;
        switch (ph)
        {
        case procGen::GenPhase::loadPng:
        {
            const int loaded = progress ? progress->sectorsLoadedDone .load() : 0;
            const int total  = progress ? progress->sectorsLoadedTotal.load() : 0;
            if (total > 0)
                sub << L"sectors " << loaded << L" / " << total;
            break;
        }
        case procGen::GenPhase::placeCity:
            sub << L"cities " << (int)displayedCityCount;
            break;
        case procGen::GenPhase::buildRoad:
            sub << L"roads " << (int)displayedRoadCount;
            break;
        default:
            break;
        }
        const std::wstring subStr = sub.str();
        if (!subStr.empty())
        {
            const int subW = queryTextWidth(subStr);
            drawText(subStr, (cameraW - subW) / 2, subY, wgcfg::TEXT_SUB);
        }
    }
};
