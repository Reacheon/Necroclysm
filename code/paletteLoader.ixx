module;
#include <SDL3/SDL.h>

export module paletteLoader;

import std;
import constVar;

// ===== 팔레트 TSV 로더 (공유 모듈) =====
// 팔레트 파일 형식 (palette/fur.tsv, palette/hair.tsv, palette/horn.tsv):
//   첫 줄: slot<TAB>COLOR1<TAB>COLOR2...  (첫 열은 slot placeholder)
//   이후 줄: <슬롯번호><TAB><hex>... (6자리 RGB hex, 알파 없음, 모두 불투명 처리)
//   색상명(헤더)이 spriteMapper 키 접미사 및 EntityData의 색상 필드 값이 됨.
//
// 이 모듈은 textureLoader(텍스쳐 생성)와 염색앰플 UI(색상 선택지 생성) 양쪽에서 쓰인다.

export struct PaletteTable
{
	std::vector<std::wstring> colorNames;                 // 헤더 순서대로 (예: GRAY, WHITE, BLACK, ...)
	std::map<std::wstring, std::vector<SDL_Color>> table; // colorName -> 슬롯 배열
};

// TSV 팔레트 로드. 파일 없거나 형식 오류면 빈 테이블 반환.
export PaletteTable loadPaletteTable(const std::string& path)
{
	PaletteTable p;
	std::ifstream f(path);
	if (!f.is_open()) return p;

	std::string line;
	bool headerParsed = false;
	while (std::getline(f, line))
	{
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty()) continue;

		std::vector<std::string> cols;
		std::stringstream ss(line);
		std::string cell;
		while (std::getline(ss, cell, '\t')) cols.push_back(cell);
		if (cols.size() < 2) continue;

		if (!headerParsed)
		{
			headerParsed = true;
			for (size_t i = 1; i < cols.size(); i++)
			{
				std::wstring name(cols[i].begin(), cols[i].end());
				p.colorNames.push_back(name);
				p.table[name] = {};
			}
			continue;
		}

		for (size_t i = 1; i < cols.size() && (i - 1) < p.colorNames.size(); i++)
		{
			unsigned int rgb = 0;
			try { rgb = std::stoul(cols[i], nullptr, 16); }
			catch (...) { continue; }
			SDL_Color c{
				(Uint8)((rgb >> 16) & 0xFF),
				(Uint8)((rgb >> 8) & 0xFF),
				(Uint8)(rgb & 0xFF),
				255
			};
			p.table[p.colorNames[i - 1]].push_back(c);
		}
	}
	return p;
}

// ===== 색상 키 → UI 메타데이터 =====
// TSV 헤더 키는 UPPER_SNAKE (예: L"ASH_GRAY").
// 이를 타이틀 케이스 UI 표시명(L"Ash Gray")으로 변환하는 헬퍼.
// 새 색상 추가 시 별도 테이블 수정 불필요 — 키 이름 규칙만 지키면 자동 변환.
export std::wstring paletteKeyToDisplayName(const std::wstring& key)
{
	std::wstring out;
	out.reserve(key.size());
	bool capNext = true;
	for (wchar_t c : key)
	{
		if (c == L'_') { out.push_back(L' '); capNext = true; continue; }
		if (capNext) { out.push_back((wchar_t)std::towupper(c)); capNext = false; }
		else { out.push_back((wchar_t)std::towlower(c)); }
	}
	return out;
}

// TSV 키 → colorPaletteOption.png의 sprIndex.
// colorPaletteOption.png 타일 레이아웃과 1:1 매핑이므로 PNG 타일 추가 시 이 맵에도 반드시 항목 추가.
// 알려지지 않은 키는 COLOR_EMPTY(0) 반환 → 빈 칩으로 표시됨.
export int paletteKeyToSprIndex(const std::wstring& key)
{
	static const std::unordered_map<std::wstring, int> m = {
		{ L"GRAY",     COLOR_GRAY     },
		{ L"WHITE",    COLOR_WHITE    },
		{ L"BLACK",    COLOR_BLACK    },
		{ L"ASH_GRAY", COLOR_ASH_GRAY },
		{ L"CREAM",    COLOR_CREAM    },
		{ L"ORANGE",   COLOR_ORANGE   },
		{ L"BROWN",    COLOR_BROWN    },
		{ L"KHAKI",    COLOR_KHAKI    },
		{ L"RED",      COLOR_RED      },
		{ L"SKY",      COLOR_SKY      },
		{ L"PURPLE",   COLOR_PURPLE   },
		{ L"GREEN",    COLOR_GREEN    },
		{ L"AMBER",    COLOR_AMBER    },
	};
	auto it = m.find(key);
	return (it != m.end()) ? it->second : (int)COLOR_EMPTY;
}
