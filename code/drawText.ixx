module;
#include <SDL3_ttf/SDL_ttf.h>
export module drawText;
import std;
import util;
import globalVar;
import constVar;

export enum class fontType
{
    mainFont,
    pixel,
};

export TTF_Font* mainFont[MAX_FONT_SIZE] = { nullptr, };

export TTF_Font* pixelFont[MAX_FONT_SIZE] = { nullptr, };

static TTF_Font** s_currentFont = nullptr;
static int  s_fontSize = 15;
static int  s_fontGap = 4;
static bool s_useSolidRender = true;
static constexpr size_t MAX_CACHE_SIZE = 5000;

// 폰트 설정 함수들
export void setFontSize(int val) { s_fontSize = val; }
export void setFontGap(int val) { s_fontGap = val; }
void setFontSolidRender(bool useSolid) { s_useSolidRender = useSolid; }
export void setFont(fontType inputFont)
{ 
    setFontSolidRender(false);
    if (inputFont == fontType::pixel)
    {
        s_currentFont = pixelFont;
        setFontSolidRender(true);
    }
    else if (inputFont == fontType::mainFont) s_currentFont = mainFont;
    else errorBox(L"setFont: 알 수 없는 폰트타입을 인자로 넣었다.");
}

struct TextCacheKey
{
    std::wstring text;
    int fontSize;
    SDL_Color color;
    TTF_Font** fontArray;
    bool solidRender;

    bool operator==(const TextCacheKey& other) const {
        return text == other.text &&
            fontSize == other.fontSize &&
            fontArray == other.fontArray &&
            solidRender == other.solidRender &&
            color.r == other.color.r &&
            color.g == other.color.g &&
            color.b == other.color.b &&
            color.a == other.color.a;
    }
};

struct TextCacheKeyHasher
{
    size_t operator()(const TextCacheKey& key) const
    {
        size_t h1 = std::hash<std::wstring>{}(key.text);
        size_t h2 = std::hash<int>{}(key.fontSize);
        size_t h3 = std::hash<void*>{}((void*)key.fontArray);

        size_t hash = h1;
        hash ^= h2 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= h3 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= key.solidRender + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= key.color.r + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= key.color.g + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= key.color.b + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= key.color.a + 0x9e3779b9 + (hash << 6) + (hash >> 2);

        return hash;
    }
};

struct CachedTexture
{
    SDL_Texture* texture;
    float width, height;
    mutable std::list<TextCacheKey>::iterator lru_it;
};

static std::unordered_map<TextCacheKey, CachedTexture, TextCacheKeyHasher> textureCache;
static std::unordered_map<std::wstring, std::array<std::wstring, 2>> splitCache; // 키: text + L"|" + widthLimit
static std::list<TextCacheKey> lruList;

static std::string toUTF8(const std::wstring& w)
{
    return utf8Encoder(w); //util 모듈을 통해 노출, <codecvt>(C++26 제거 예정) 대체
}

// 컬러코드 파싱 함수
static SDL_Color parseColorCode(const std::wstring& colorCode)
{
    SDL_Color color = { 255, 255, 255, 255 }; // 기본값: 흰색

    if (colorCode.size() != 7 || colorCode[0] != L'#') return color;

    auto hexToInt = [](wchar_t c) -> int {
        if (c >= L'0' && c <= L'9') return c - L'0';
        if (c >= L'A' && c <= L'F') return c - L'A' + 10;
        if (c >= L'a' && c <= L'f') return c - L'a' + 10;
        return 15; // 기본값
        };

    color.r = hexToInt(colorCode[1]) * 16 + hexToInt(colorCode[2]);
    color.g = hexToInt(colorCode[3]) * 16 + hexToInt(colorCode[4]);
    color.b = hexToInt(colorCode[5]) * 16 + hexToInt(colorCode[6]);

    return color;
}

// 컬러코드 제거된 텍스트와 실제 표시 너비 계산
export std::wstring removeColorCodes(const std::wstring& text)
{
    std::wstring result;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == L'#' && i + 6 < text.size()) {
            i += 6; // #RRGGBB 건너뛰기
        }
        else if (text[i] != L'\n') {
            result += text[i];
        }
    }
    return result;
}

// 컬러코드를 제외한 텍스트 너비 계산
export int getTextWidthWithoutColor(const std::wstring& text)
{
    std::wstring cleanText = removeColorCodes(text);
    std::string utf8 = toUTF8(cleanText);

    int w, h;
    TTF_GetStringSize(s_currentFont[s_fontSize], utf8.c_str(), 0, &w, &h);
    return w;
}

// 컬러코드를 제외한 텍스트 높이 계산
export int getTextHeightWithoutColor(const std::wstring& text)
{
    std::wstring cleanText = removeColorCodes(text);
    std::string utf8 = toUTF8(cleanText);

    int w, h;
    TTF_GetStringSize(s_currentFont[s_fontSize], utf8.c_str(), 0, &w, &h);
    return h;
}

static CachedTexture* getCachedTexture(const std::wstring& text, int fontSize, SDL_Color color)
{
    // 색상 정규화
    if (color.a == 0) color.a = 255;

    TextCacheKey key{ text, fontSize, color, s_currentFont, s_useSolidRender };

    auto it = textureCache.find(key);
    if (it != textureCache.end()) {
        lruList.erase(it->second.lru_it);
        lruList.push_front(key);
        it->second.lru_it = lruList.begin();
        return &it->second;
    }

    std::string utf8 = toUTF8(text);
    SDL_Surface* surf = s_useSolidRender
        ? TTF_RenderText_Solid(s_currentFont[fontSize], utf8.c_str(), 0, color)
        : TTF_RenderText_Blended(s_currentFont[fontSize], utf8.c_str(), 0, color);

    if (!surf) return nullptr;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

    // 픽셀 폰트용 설정
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);  // 추가!

    float fw, fh;
    SDL_GetTextureSize(tex, &fw, &fh);
    SDL_DestroySurface(surf);

    if (textureCache.size() >= MAX_CACHE_SIZE) {
        auto lru_key = lruList.back();
        auto lru_it = textureCache.find(lru_key);
        if (lru_it != textureCache.end()) {  // 안전성 체크 추가
            SDL_DestroyTexture(lru_it->second.texture);
            textureCache.erase(lru_it);
        }
        lruList.pop_back();
    }

    lruList.push_front(key);
    auto [insert_it, success] = textureCache.emplace(key, CachedTexture{ tex, fw, fh, lruList.begin() });

    return &insert_it->second;
}

// 멀티컬러 텍스트 렌더링 (내부 구현)
static void drawMultiColorTextInternal(const std::wstring& text, int x, int y, SDL_Color defaultColor, bool center)
{
    // 줄바꿈 처리
    size_t newlinePos = text.find(L'\n');
    if (newlinePos != std::wstring::npos) {
        std::wstring line1 = text.substr(0, newlinePos);
        std::wstring line2 = text.substr(newlinePos + 1);

        drawMultiColorTextInternal(line1, x, y, defaultColor, center);
        int centerCorrection = center ? getTextHeightWithoutColor(L"A") / 2 : 0;
        drawMultiColorTextInternal(line2, x, y + s_fontSize + s_fontGap + centerCorrection, defaultColor, center);
        return;
    }

    // Center 처리를 위한 전체 너비/높이 계산
    int totalWidth = 0;
    int totalHeight = 0;
    if (center) {
        totalWidth = getTextWidthWithoutColor(text);
        totalHeight = getTextHeightWithoutColor(text);
        y -= totalHeight / 2;
    }

    int currentX = center ? x - totalWidth / 2 : x;
    SDL_Color currentColor = defaultColor;
    std::wstring remainingText = text;

    // 첫 번째 문자가 컬러코드가 아니면 기본 색상 추가
    if (remainingText.empty() || remainingText[0] != L'#') {
        // 현재 렌더러 색상을 컬러코드로 변환하여 추가
        wchar_t colorStr[8];
        swprintf(colorStr, 8, L"#%02X%02X%02X", defaultColor.r, defaultColor.g, defaultColor.b);
        remainingText = std::wstring(colorStr) + remainingText;
    }

    while (!remainingText.empty()) {
        size_t colorPos = remainingText.find(L'#');

        if (colorPos == 0 && remainingText.size() >= 7) {
            // 컬러코드 파싱
            std::wstring colorCode = remainingText.substr(0, 7);
            currentColor = parseColorCode(colorCode);
            remainingText = remainingText.substr(7);

            // 다음 컬러코드나 문자열 끝까지 찾기
            size_t nextColorPos = remainingText.find(L'#');
            std::wstring textSegment = (nextColorPos != std::wstring::npos) ?
                remainingText.substr(0, nextColorPos) : remainingText;

            if (!textSegment.empty()) {
                // 텍스트 세그먼트 렌더링
                CachedTexture* cached = getCachedTexture(textSegment, s_fontSize, currentColor);
                if (cached) {
                    SDL_FRect dst = { float(currentX), float(y), cached->width, cached->height };
                    SDL_RenderTexture(renderer, cached->texture, nullptr, &dst);
                    currentX += static_cast<int>(cached->width);
                }

                remainingText = (nextColorPos != std::wstring::npos) ?
                    remainingText.substr(nextColorPos) : L"";
            }
        }
        else {
            // 컬러코드가 없는 경우 전체 텍스트 렌더링
            CachedTexture* cached = getCachedTexture(remainingText, s_fontSize, currentColor);
            if (cached) {
                SDL_FRect dst = { float(currentX), float(y), cached->width, cached->height };
                SDL_RenderTexture(renderer, cached->texture, nullptr, &dst);
            }
            break;
        }
    }
}

// 기본 단색 텍스트 렌더링 (기존 함수들)
export void drawText(std::wstring text, int x, int y, SDL_Color inputCol)
{
    // 알파값 명시적 설정
    if (inputCol.a == 0) inputCol.a = 255;

    if (text.find(L'#') != std::wstring::npos) {
        drawMultiColorTextInternal(text, x, y, inputCol, false);
        return;
    }

    CachedTexture* cached = getCachedTexture(text, s_fontSize, inputCol);
    if (!cached) return;

    // 정수 좌표로 렌더링
    SDL_FRect dst = {
        std::round(float(x)),
        std::round(float(y)),
        std::round(cached->width),
        std::round(cached->height)
    };
    SDL_RenderTexture(renderer, cached->texture, nullptr, &dst);
}

export void drawText(std::wstring text, int x, int y) {
    drawText(text, x, y, col::white);
}

export void drawTextCenter(std::wstring text, int x, int y, SDL_Color inputCol)
{
    if (inputCol.a == 0) inputCol.a = 255;

    if (text.find(L'#') != std::wstring::npos) {
        drawMultiColorTextInternal(text, x, y, inputCol, true);
        return;
    }

    CachedTexture* cached = getCachedTexture(text, s_fontSize, inputCol);
    if (!cached) return;

    SDL_FRect dst = {
        std::round(float(x) - cached->width / 2.0f),
        std::round(float(y) - cached->height / 2.0f),
        std::round(cached->width),
        std::round(cached->height)
    };
    SDL_RenderTexture(renderer, cached->texture, nullptr, &dst);
}

export void drawTextCenter(std::wstring text, int x, int y) {
    drawTextCenter(text, x, y, col::white);
}

//텍스트를 scale배로 중앙 그리기 — 텍스처를 NEAREST로 확대/축소(픽셀폰트 격자 보존).
//  현재 setFont/setFontSize 상태를 그대로 사용 → 한 폰트크기(예: 픽셀폰트 12)를 줌 배율로
//  키우는 용도(월드맵 도시 라벨). 단색·단일행 전용(멀티컬러/줄바꿈 미지원).
export void drawTextCenterScaled(std::wstring text, int x, int y, float scale, SDL_Color inputCol)
{
    if (inputCol.a == 0) inputCol.a = 255;
    CachedTexture* cached = getCachedTexture(text, s_fontSize, inputCol);
    if (!cached) return;

    SDL_FRect dst = {
        std::round(float(x) - cached->width  * scale / 2.0f),
        std::round(float(y) - cached->height * scale / 2.0f),
        std::round(cached->width  * scale),
        std::round(cached->height * scale)
    };
    SDL_RenderTexture(renderer, cached->texture, nullptr, &dst);
}

export void drawTextOutline(std::wstring text, int x, int y, SDL_Color inputCol)
{
    drawText(text, x - 1, y, col::black);
    drawText(text, x + 1, y, col::black);
    drawText(text, x, y - 1, col::black);
    drawText(text, x, y + 1, col::black);
    drawText(text, x - 1, y - 1, col::black);
    drawText(text, x + 1, y - 1, col::black);
    drawText(text, x - 1, y + 1, col::black);
    drawText(text, x + 1, y + 1, col::black);
    drawText(text, x, y, inputCol);
}

export void drawTextOutlineCenter(std::wstring text, int x, int y, SDL_Color inputCol)
{
    drawTextCenter(text, x - 1, y, col::black);
    drawTextCenter(text, x + 1, y, col::black);
    drawTextCenter(text, x, y - 1, col::black);
    drawTextCenter(text, x, y + 1, col::black);
    drawTextCenter(text, x - 1, y - 1, col::black);
    drawTextCenter(text, x + 1, y - 1, col::black);
    drawTextCenter(text, x - 1, y + 1, col::black);
    drawTextCenter(text, x + 1, y + 1, col::black);
    drawTextCenter(text, x, y, inputCol);
}

export void drawTextOuline(std::wstring text, int x, int y) {
    drawTextOutline(text, x, y, col::white);
}

export void drawTextOutlineCenter(std::wstring text, int x, int y) {
    drawTextOutlineCenter(text, x, y, col::white);
}

// 텍스트 너비 자동 줄바꿈 기능
export std::array<std::wstring, 2> textSplitter(std::wstring text, int widthLimit)
{
    static constexpr size_t MAX_SPLIT_CACHE_SIZE = 1000;
    static std::list<std::wstring> splitLruList;

    std::wstring cacheKey = text + L"|" + std::to_wstring(widthLimit);

    // 캐시 확인
    auto splitCached = splitCache.find(cacheKey);
    if (splitCached != splitCache.end()) {
        // LRU 업데이트: 해당 키를 리스트에서 제거하고 맨 앞으로 이동
        auto lruIt = std::find(splitLruList.begin(), splitLruList.end(), cacheKey);
        if (lruIt != splitLruList.end()) {
            splitLruList.erase(lruIt);
        }
        splitLruList.push_front(cacheKey);
        return splitCached->second;
    }

    std::array<std::wstring, 2> result;
    int textNoColorWidth = getTextWidthWithoutColor(text);

    if (textNoColorWidth > widthLimit) {
        // 컬러코드를 고려한 분할 로직
        std::wstring lastColorCode = L"";
        size_t lastSpaceInOriginal = std::wstring::npos; // 원본 text에서 마지막 공백 위치 추적

        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == L'#' && i + 6 < text.size()) {
                lastColorCode = text.substr(i, 7);
                i += 6;
                continue;
            }

            if (text[i] == L' ') {
                lastSpaceInOriginal = i;
            }

            std::wstring testText = removeColorCodes(text.substr(0, i + 1));
            int testWidth = getTextWidthWithoutColor(testText);

            if (testWidth > widthLimit) {
                // 단어 단위 줄바꿈: 마지막 공백 위치로 backtrack
                size_t splitPos;
                if (lastSpaceInOriginal != std::wstring::npos && lastSpaceInOriginal > 0) {
                    splitPos = lastSpaceInOriginal + 1; // 공백 다음에서 분할
                }
                else {
                    splitPos = i; // fallback: 공백이 없으면 글자 단위로 분할
                }

                std::wstring return1 = text.substr(0, splitPos);
                std::wstring return2 = text.substr(splitPos);

                // 줄바꿈 문자 처리
                size_t newlinePos = return1.find(L'\n');
                if (newlinePos != std::wstring::npos) {
                    return2 = return1.substr(newlinePos + 1) + return2;
                    return1 = return1.substr(0, newlinePos);
                }

                // 두 번째 줄에 컬러코드가 없으면 마지막 컬러코드 추가
                if (!return2.empty() && return2[0] != L'#' && !lastColorCode.empty()) {
                    return2 = lastColorCode + return2;
                }

                result = { return1, return2 };
                break;
            }
        }

        // 반복문이 끝났는데 result가 설정되지 않은 경우
        if (result[0].empty() && result[1].empty()) {
            result = { text, L"" };
        }
    }
    else {
        // 너비가 제한보다 작으면 줄바꿈 문자만 확인
        size_t newlinePos = text.find(L'\n');
        if (newlinePos != std::wstring::npos) {
            std::wstring return1 = text.substr(0, newlinePos);
            std::wstring return2 = text.substr(newlinePos + 1);
            result = { return1, return2 };
        }
        else {
            result = { text, L"" };
        }
    }

    // 캐시 크기 관리
    if (splitCache.size() >= MAX_SPLIT_CACHE_SIZE) {
        // 가장 오래된 항목 제거
        auto lru_key = splitLruList.back();
        splitCache.erase(lru_key);
        splitLruList.pop_back();
    }

    // 새 결과를 캐시에 추가
    splitCache[cacheKey] = result;
    splitLruList.push_front(cacheKey);

    return result;
}

// 너비 제한 텍스트 렌더링
export int drawTextWidth(std::wstring text, int x, int y, int width, int height, int lineLimit)
{
    std::array<std::wstring, 2> textPair = textSplitter(text, width);

    if (height == -1) {
        height = s_fontSize + s_fontGap;
    }

    if (lineLimit <= 0) {
        return 0;
    }
    else if (textPair[1] == L"") {
        drawText(text, x, y);
        return 1;
    }
    else {
        drawText(textPair[0], x, y);
        return 1 + drawTextWidth(textPair[1], x, y + height, width, height, lineLimit - 1);
    }
}

export int drawTextWidth(std::wstring text, int x, int y, int width, int height)
{
    return drawTextWidth(text, x, y, width, height, 999);
}

export int drawTextCenterWidth(std::wstring text, int x, int y, int width, int height, int lineLimit)
{
    std::array<std::wstring, 2> textPair = textSplitter(text, width);

    if (height == -1) {
        height = s_fontSize + s_fontGap;
    }

    if (lineLimit <= 0) {
        return 0;
    }
    else if (textPair[1] == L"") {
        drawTextCenter(text, x, y);
        return 1;
    }
    else {
        drawTextCenter(textPair[0], x, y);
        return 1 + drawTextCenterWidth(textPair[1], x, y + height, width, height, lineLimit - 1);
    }
}

export int drawTextCenterWidth(std::wstring text, int x, int y, int width, int height)
{
    return drawTextCenterWidth(text, x, y, width, height, 999);
}

//그리지 않고 줄 수만 계산
export int queryLineCount(const std::wstring& text, int width)
{
    std::wstring remaining = text;
    int count = 0;
    while (!remaining.empty())
    {
        auto pair = textSplitter(remaining, width);
        count++;
        if (pair[1].empty()) break;
        remaining = pair[1];
    }
    return count;
}

// Cache cleanup function - call this when shutting down
export void clearTextCache()
{
    for (auto& pair : textureCache) {
        SDL_DestroyTexture(pair.second.texture);
    }
    textureCache.clear();
    splitCache.clear();
    lruList.clear();
}

// 텍스트 가로/세로 측정 함수
export int queryTextWidth(const std::wstring& text, bool hasColor = false) {
    if (hasColor) return getTextWidthWithoutColor(text);
    if (auto ct = getCachedTexture(text, s_fontSize, col::white)) return (int)ct->width;
    return 0;
}
export int queryTextHeight(const std::wstring& text, bool hasColor = false) {
    if (hasColor) return getTextHeightWithoutColor(text);
    if (auto ct = getCachedTexture(text, s_fontSize, col::white)) return (int)ct->height;
    return 0;
}

//@brief 캐시없이 바로 텍스트를 출력하는 함수, 프레임이나 디버깅 시간 표시 용도로 쓸 것
export void drawTextDirect(const std::wstring& text, int x, int y, SDL_Color col = col::white)
{
    std::string utf8 = toUTF8(text);

    SDL_Surface* surf = s_useSolidRender
        ? TTF_RenderText_Solid(s_currentFont[s_fontSize], utf8.c_str(), 0, col)
        : TTF_RenderText_Blended(s_currentFont[s_fontSize], utf8.c_str(), 0, col);

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);
    float w, h;
    SDL_GetTextureSize(tex, &w, &h);
    SDL_FRect dst = { float(x), float(y), w, h };
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

//@brief 텍스트를 maxWidth 픽셀 내에서 단어 단위로 분할 (공백 기준, 단어 중간 끊김 방지)
export std::array<std::wstring, 2> wordSplitter(const std::wstring& text, int maxWidth)
{
    std::wstring line;
    int lastSpace = -1;

    for (int i = 0; i < text.size(); i++)
    {
        line += text[i];
        if (text[i] == L' ') lastSpace = i;

        if (queryTextWidth(removeColorCodes(line)) >= maxWidth)
        {
            if (lastSpace != -1)
            {
                return { text.substr(0, lastSpace), text.substr(lastSpace + 1) };
            }
            else
            {
                return { text.substr(0, i), text.substr(i) };
            }
        }
    }
    return { text, L"" };
}

// wordSplitter를 두 번 적용해 최대 3줄로 분할. 3번째 줄은 남은 전체(여전히 maxWidth 초과 가능).
export std::array<std::wstring, 3> wordSplitter3(const std::wstring& text, int maxWidth)
{
    auto first = wordSplitter(text, maxWidth); // { 1줄째, 나머지 }
    if (queryTextWidth(removeColorCodes(first[1])) < maxWidth)
    {
        return { first[0], first[1], L"" };
    }

    auto second = wordSplitter(first[1], maxWidth); // { 2줄째, 3줄째 }
    return { first[0], second[0], second[1] };
}