module;
#include <SDL3/SDL.h>

export module WorldGenScreen;

import std;
import util;
import constVar;
import GUI;
import drawText;
import globalVar;
import worldGen;
import worldGrid;
import Sector;

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

    std::shared_ptr<worldGen::WorldGenProgress> progress;
    std::jthread worker;

    //미리보기 텍스처 — 워커가 패치별로 채우는 RGBA를 메인이 점진 반영.
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
    std::function<void(worldGen::WorldGenResult)> onCompleted;
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
        const double fx = (double)px / (double)worldGrid::WORLD_PIXEL_W;
        const double fy = (double)py / (double)worldGrid::WORLD_PIXEL_H;
        return SDL_FPoint{
            (float)(r.x + fx * r.w),
            (float)(r.y + fy * r.h)
        };
    }

    SDL_FPoint tileToScreen(const Point3& t) const
    {
        // worldGrid 픽셀 베이스: 패치 (-54,-27) 좌상단이 픽셀(0,0)
        constexpr int PATCH_X_MIN = -54;
        constexpr int PATCH_Y_MIN = -27;
        constexpr int PIXEL_PER_PATCH = 400;
        constexpr int TILE_BASE_X =
            PATCH_X_MIN * PIXEL_PER_PATCH * worldGrid::TILES_PER_PIXEL;
        constexpr int TILE_BASE_Y =
            PATCH_Y_MIN * PIXEL_PER_PATCH * worldGrid::TILES_PER_PIXEL;

        const double pxd = (double)(t.x - TILE_BASE_X) / (double)worldGrid::TILES_PER_PIXEL;
        const double pyd = (double)(t.y - TILE_BASE_Y) / (double)worldGrid::TILES_PER_PIXEL;

        const SDL_Rect r = mapRect();
        const double fx = pxd / (double)worldGrid::WORLD_PIXEL_W;
        const double fy = pyd / (double)worldGrid::WORLD_PIXEL_H;
        return SDL_FPoint{
            (float)(r.x + fx * r.w),
            (float)(r.y + fy * r.h)
        };
    }

    //--- phase별 표시 텍스트 ---
    static const wchar_t* phaseLabel(worldGen::GenPhase ph)
    {
        switch (ph)
        {
        case worldGen::GenPhase::idle:         return L"Initializing";
        case worldGen::GenPhase::loadPng:      return L"Loading satellite imagery";
        case worldGen::GenPhase::placeCity:    return L"Placing cities";
        case worldGen::GenPhase::buildRoad:    return L"Building road network";
        case worldGen::GenPhase::prepareSpawn: return L"Preparing spawn area";
        case worldGen::GenPhase::done:         return L"Finalizing world";
        }
        return L"";
    }

    //--- 좌측 하단 로딩 로드맵 ---------------------------------------------
    //   4단계 진행도를 원-라인-원 형태로 표시. 활성 단계는 흰색 + 두 줄
    //   회전 스피너(머리/꼬리 페이드 아크)로 강조.
    void drawRoadmap(worldGen::GenPhase ph) const
    {
        // 단계 번호: 1=loadPng, 2=placeCity, 3=buildRoad, 4=prepareSpawn
        int activeStep = 0;
        bool stepDone[4] = { false, false, false, false };
        switch (ph)
        {
        case worldGen::GenPhase::idle:         activeStep = 0; break;
        case worldGen::GenPhase::loadPng:      activeStep = 1; break;
        case worldGen::GenPhase::placeCity:    activeStep = 2; stepDone[0] = true; break;
        case worldGen::GenPhase::buildRoad:    activeStep = 3; stepDone[0] = stepDone[1] = true; break;
        case worldGen::GenPhase::prepareSpawn: activeStep = 4; stepDone[0] = stepDone[1] = stepDone[2] = true; break;
        case worldGen::GenPhase::done:         stepDone[0] = stepDone[1] = stepDone[2] = stepDone[3] = true; break;
        }

        constexpr SDL_Color C_PENDING = { 105, 105, 110, 255 };
        constexpr SDL_Color C_DONE    = { 175, 175, 180, 255 };
        constexpr SDL_Color C_ACTIVE  = { 245, 245, 240, 255 };

        constexpr int R_OUTER = 17;   // 본 링 바깥 반지름
        constexpr int R_INNER = 15;   // 본 링 안쪽 (두께 2px)
        constexpr int SPACING = 74;   // 단계 간 세로 간격 (원 중심 기준)
        const int cx       = 64;
        const int firstCy  = cameraH - 300;   // 최상단 원 중심 Y (4단계로 확장하면서 위로)
        const int textX    = cx + 36;

        static const wchar_t* labels[4] = {
            L"Loading satellite imagery",
            L"Placing cities",
            L"Building road network",
            L"Preparing spawn area",
        };

        // 단계 사이를 잇는 도트 라인 (3px 점, 4px 간격)
        for (int i = 0; i < 3; ++i)
        {
            const int yA = firstCy +  i      * SPACING + R_OUTER + 4;
            const int yB = firstCy + (i + 1) * SPACING - R_OUTER - 4;
            const SDL_Color lineCol = stepDone[i] ? C_DONE : C_PENDING;
            for (int y = yA; y <= yB; y += 5)
            {
                drawLine(cx, y, cx, std::min(yB, y + 2), lineCol);
            }
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // 각 단계 그리기
        for (int i = 0; i < 4; ++i)
        {
            const int cy = firstCy + i * SPACING;
            const bool isActive = (activeStep == (i + 1));
            const SDL_Color col = isActive ? C_ACTIVE : (stepDone[i] ? C_DONE : C_PENDING);

            // 두께 2px 링 (스캔라인 방식 — 외곽-내곽 사이만 채움)
            for (int dy = -R_OUTER; dy <= R_OUTER; ++dy)
            {
                const int oxSpan = (int)std::sqrt((float)(R_OUTER * R_OUTER - dy * dy));
                const int absdy  = (dy < 0) ? -dy : dy;
                if (absdy <= R_INNER)
                {
                    const int ixSpan = (int)std::sqrt((float)(R_INNER * R_INNER - dy * dy));
                    drawLine(cx - oxSpan, cy + dy, cx - ixSpan, cy + dy, col);
                    drawLine(cx + ixSpan + 1, cy + dy, cx + oxSpan, cy + dy, col);
                }
                else
                {
                    drawLine(cx - oxSpan, cy + dy, cx + oxSpan, cy + dy, col);
                }
            }

            // 숫자 — 원 중심에 정확히 정렬
            setFont(fontType::mainFontBold);
            setFontSize(18);
            const std::wstring numStr = std::to_wstring(i + 1);
            drawTextCenter(numStr, cx, cy, col);

            // 라벨 — 원 우측, 세로 중앙
            setFont(fontType::mainFont);
            setFontSize(15);
            drawTextCenter(labels[i], textX + queryTextWidth(labels[i]) / 2, cy, col);

            // 활성 단계: 두 개의 회전 아크 스피너
            //   본 링 바깥(R_OUTER+2.5 ~ R_OUTER+4.5)에 75도짜리 호 두 개를
            //   180도 간격으로 띄우고, 각 호는 머리에서 꼬리 방향으로 페이드.
            //   라디얼 라인을 촘촘히 쌓아서 2px 두께를 만든다.
            if (isActive)
            {
                constexpr float PI    = 3.14159265f;
                constexpr float TWOPI = 6.28318531f;
                const float t   = (float)SDL_GetTicks() / 1000.0f;
                const float rot = std::fmod(t * 3.0f, TWOPI);   // ≈ 0.48 회전/초
                constexpr float ARC = 1.30f;                    // ≈ 75도

                const float rIn  = (float)R_OUTER + 2.5f;
                const float rOut = (float)R_OUTER + 4.5f;

                constexpr int N = 36;  // 호당 라디얼 분할
                for (int k = 0; k < 2; ++k)
                {
                    const float head = rot + (float)k * PI;
                    for (int s = 0; s < N; ++s)
                    {
                        const float frac = (float)s / (float)(N - 1);  // 0=머리, 1=꼬리
                        const float ang  = head - frac * ARC;
                        // 머리 가까이는 밝고, 꼬리는 빠르게 페이드
                        const float fade = 1.0f - frac;
                        const Uint8 a = (Uint8)(245.0f * fade * fade);
                        if (a < 6) continue;
                        const float c = std::cos(ang), si = std::sin(ang);
                        drawLine(
                            (int)std::round((float)cx + rIn  * c),
                            (int)std::round((float)cy + rIn  * si),
                            (int)std::round((float)cx + rOut * c),
                            (int)std::round((float)cy + rOut * si),
                            C_ACTIVE, a);
                    }
                    // 머리 끝에 살짝 굵은 글로우 점 — 회전 방향성을 강조
                    const float c = std::cos(head), si = std::sin(head);
                    const float rMid = (rIn + rOut) * 0.5f;
                    drawFillCircle(
                        (int)std::round((float)cx + rMid * c),
                        (int)std::round((float)cy + rMid * si),
                        2, C_ACTIVE, 220);
                }
            }
        }
    }

public:
    //onWorldReady 콜백은 done 시점에 메인 스레드에서 호출됨(WorldGenResult 결과 인계).
    //  spawnTile은 Phase 4에서 사전 절차생성할 섹터 윈도우의 중심 (보통 SPAWN_DEFAULT).
    WorldGenScreen(std::uint64_t seed, Point3 spawnTile, std::function<void(worldGen::WorldGenResult)> onWorldReady)
        : GUI(false)
        , progress(std::make_shared<worldGen::WorldGenProgress>())
        , onCompleted(std::move(onWorldReady))
    {
        errorBox(ptr != nullptr, L"More than one WorldGenScreen instance was generated.");
        ptr = this;
        x = 0; y = 0;

        //오픈 애니메이션 없이 즉시 표시. 입력은 차단(생성 중에 메뉴 못 열게).
        deactInput();

        //워커 스레드 기동 — shared_ptr 캡처로 수명 안전(jthread 소멸자가 join)
        //  Phase 1~3은 worldGen::generateWorld가 처리.
        //  Phase 4 (prepareSpawn)는 본 워커가 generateWorld 후 직접 처리 — Sector 모듈 의존성을
        //  worldGen 모듈에 넣지 않기 위함 (Sector → worldGrid 단방향 유지).
        auto progPtr = progress;
        worker = std::jthread([seed, spawnTile, progPtr]
        {
            //--- Phase 1~3: PNG 로드 + 도시 + 도로망 ---
            worldGen::generateWorld(seed, *progPtr);

            //--- Phase 4: 스폰 주변 9 섹터 사전 절차생성 (동기) ---
            progPtr->phase.store(worldGen::GenPhase::prepareSpawn, std::memory_order_release);
            const SectorCoord cur = sectorFromTile(spawnTile);
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    SectorCache::ins().getOrCompute(
                        SectorCoord{ cur.x + dx, cur.y + dy, cur.z }, seed);
                }
            }

            //--- Done ---
            progPtr->phase.store(worldGen::GenPhase::done, std::memory_order_release);
            progPtr->done .store(true,                    std::memory_order_release);
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
                    worldGrid::PREVIEW_W, worldGrid::PREVIEW_H);
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
                        worldGrid::PREVIEW_W * 4);
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
            if (ph == worldGen::GenPhase::buildRoad || ph == worldGen::GenPhase::done)
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
                case worldGen::CityTier::T1: s = 3; break;
                case worldGen::CityTier::T2: s = 2; break;
                case worldGen::CityTier::T3: s = 1; break;
                }
                drawFillRect(SDL_Rect{ (int)sp.x - s/2, (int)sp.y - s/2, s, s },
                             wgcfg::CITY_DOT);
            }
        }

        //--- 하단 phase 텍스트 + 점 . .. ... 애니메이션 ---
        const auto ph = progress ? progress->phase.load(std::memory_order_acquire)
                                 : worldGen::GenPhase::idle;
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
        case worldGen::GenPhase::loadPng:
        {
            const int loaded = progress ? progress->patchesLoadedDone .load() : 0;
            const int total  = progress ? progress->patchesLoadedTotal.load() : 0;
            if (total > 0)
                sub << L"patches " << loaded << L" / " << total;
            break;
        }
        case worldGen::GenPhase::placeCity:
            sub << L"cities " << (int)displayedCityCount;
            break;
        case worldGen::GenPhase::buildRoad:
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

        //--- 좌측 하단 로딩 로드맵 ---
        drawRoadmap(ph);
    }
};
