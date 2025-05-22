#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cmath>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <array>

export module drawText;

import util;
import globalVar;
import constVar;

static int  s_fontSize = 15;
static int  s_fontGap = 4;
static bool solidDraw = true;

static std::string toUTF8(const std::wstring& w)
{
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;
    return cvt.to_bytes(w);
}

export void setSolidText() { solidDraw = true; }
export void disableSolidText() { solidDraw = false; }

export void queryTextSize(std::wstring text, int* w, int* h, bool isColorCodeText)
{
    if (isColorCodeText)
    {
        for (std::size_t i = 0; i < text.size(); )
        {
            if (text[i] == L'#') { text.erase(i, 7); }
            else if (text[i] == L'\n') { text.erase(i, 1); }
            else { ++i; }
        }
    }

    std::string utf8 = toUTF8(text);
    TTF_Text* txt = TTF_CreateText(nullptr, mainFont[s_fontSize], utf8.c_str(), 0);
    if (!txt) { if (w) *w = 0; if (h) *h = 0; return; }

    TTF_GetTextSize(txt, w, h);
    TTF_DestroyText(txt);
}

export int queryTextWidth(std::wstring text, bool isColorCodeText)
{
    int w = -1;
    queryTextSize(std::move(text), &w, nullptr, isColorCodeText);
    return w;
}

export int queryTextWidthColorCode(std::wstring text) { return queryTextWidth(std::move(text), true); }
export int queryTextWidth(std::wstring text) { return queryTextWidth(std::move(text), false); }

export int queryTextHeight(std::wstring text, bool isColorCodeText)
{
    int h = -1;
    queryTextSize(std::move(text), nullptr, &h, isColorCodeText);
    return h;
}

export std::wstring eraseColorCodeText(std::wstring text)
{
    for (int i = static_cast<int>(text.size()) - 1; i >= 0; --i)
    {
        if (text[i] == UNI::HASH) { text.erase(i, 7); }
        else if (text[i] == L'\n') { text.erase(i, 1); }
    }
    return text;
}

export std::array<std::wstring, 2> textSplitter(std::wstring text, int widthLimit)
{
    static std::unordered_map<std::wstring, std::array<std::wstring, 2>> cache;
    auto it = cache.find(text);
    if (it != cache.end()) return it->second;

    int plainWidth = queryTextWidth(text, true);
    int hashWidth = queryTextWidth(L"#FFFFFF", false);

    if (plainWidth > widthLimit)
    {
        int hashCnt = 0;
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            int frag;
            queryTextSize(text.substr(0, i + 1), &frag, nullptr, false);
            if (frag - hashCnt * hashWidth > widthLimit)
            {
                std::wstring part1 = text.substr(0, i);
                std::wstring part2 = text.substr(i);

                auto lf = part1.find(L'\n');
                if (lf != std::wstring::npos)
                {
                    part2 = part1.substr(lf + 1) + part2;
                    part1.erase(lf);
                }

                std::wstring lastHash;
                if (part2[0] != UNI::HASH)
                {
                    for (int j = static_cast<int>(part1.size()) - 7; j >= 0; --j)
                        if (part1[j] == UNI::HASH) { lastHash = part1.substr(j, 7); break; }
                }
                return cache[text] = { part1, lastHash + part2 };
            }
            if (text[i] == UNI::HASH) ++hashCnt;
        }
    }
    else
    {
        auto lf = text.find(L'\n');
        if (lf != std::wstring::npos)
        {
            std::wstring part1 = text.substr(0, lf);
            std::wstring part2 = text.substr(lf + 1);
            std::wstring lastHash;
            if (part2[0] != UNI::HASH)
            {
                for (int j = static_cast<int>(part1.size()) - 7; j >= 0; --j)
                    if (part1[j] == UNI::HASH) { lastHash = part1.substr(j, 7); break; }
            }
            return cache[text] = { part1, lastHash + part2 };
        }
    }
    return cache[text] = { text, L"" };
}

export void drawTextEx(std::wstring text, int x, int y, bool center)
{
    SDL_Color color; SDL_GetRenderDrawColor(renderer, &color.r, &color.g, &color.b, &color.a);

    int totalWidth = 0;
    if (center) totalWidth = queryTextWidth(text, true);

    int cursorX = x - (center ? totalWidth / 2 : 0);

    for (std::size_t i = 0; i < text.size();)
    {
        SDL_Color renderCol = color;
        if (text[i] == UNI::HASH && i + 6 < text.size())
        {
            auto hex2 = [](wchar_t c)->int {
                if (c >= L'0' && c <= L'9') return c - L'0';
                if (c >= L'A' && c <= L'F') return 10 + (c - L'A');
                return 15;
                };
            renderCol.r = Uint8(hex2(text[i + 1]) * 16 + hex2(text[i + 2]));
            renderCol.g = Uint8(hex2(text[i + 3]) * 16 + hex2(text[i + 4]));
            renderCol.b = Uint8(hex2(text[i + 5]) * 16 + hex2(text[i + 6]));
            i += 7;
        }

        std::size_t nextHash = text.find(UNI::HASH, i);
        std::wstring segment = text.substr(i, nextHash - i);

        std::string utf8 = toUTF8(segment);
        SDL_Surface* surf = solidDraw ?
            TTF_RenderText_Solid(mainFont[s_fontSize], utf8.c_str(), 0, renderCol) :
            TTF_RenderText_Blended(mainFont[s_fontSize], utf8.c_str(), 0, renderCol);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);

        float w, h; SDL_GetTextureSize(tex, &w, &h);
        SDL_FRect dst = { float(cursorX), float(y) - (center ? h / 2 + 1 : 0), w, h };
        SDL_RenderTexture(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);

        cursorX += w;

        if (nextHash == std::wstring::npos) break;
        i = nextHash;
    }
}

export void drawText(std::wstring t, int x, int y) { drawTextEx(t, x, y, false); }
export void drawTextCenter(std::wstring t, int x, int y) { drawTextEx(t, x, y, true); }

export int drawTextWidth(std::wstring text, int x, int y, bool center, int width, int height, int limit)
{
    auto pair = textSplitter(text, width);
    if (height == -1) height = s_fontSize + s_fontGap;
    if (limit <= 0) return 0;
    if (pair[1].empty()) { drawTextEx(text, x, y, center); return 1; }
    drawTextEx(pair[0], x, y, center);
    return 1 + drawTextWidth(pair[1], x, y + height, center, width, height, limit - 1);
}

export int drawTextWidth(std::wstring text, int x, int y, bool center, int width, int height)
{
    return drawTextWidth(std::move(text), x, y, center, width, height, 999);
}

export void setFontSize(int v) { s_fontSize = v; }
export void setFontGap(int v) { s_fontGap = v; }
