#include <SDL.h>
#include <SDL_ttf.h>

export module drawText;

import std;
import util;
import globalVar;
import constVar;

static int s_fontSize = 16;
static int s_fontGap = 4;

//@brief 텍스트 스캔, 제어문자를 포함해 스캔할 경우(exConChar = true) 더 빠름
export void queryTextSize(std::wstring text, int* w, int* h, bool isColorCodeText)
{
	if (isColorCodeText == true)
	{
		for (int i = 0; i < text.size(); i++)
		{
			if (text[i] == '#')
			{
				text = text.erase(i, 7);
				i--;
			}
			else if (text[i] == '\n')
			{
				text = text.erase(i, 1);
				i--;
			}
		}
	}
	Uint16* unicode = new Uint16[text.size() + 1]();
	for (int i = 0; i < text.size(); i++) { unicode[i] = text[i]; }
	unicode[text.size()] = 0;
	TTF_SizeUNICODE(mainFont[s_fontSize], unicode, w, h);
	delete[] unicode;
}

//@brief 텍스트 너비 스캔, 제어문자를 포함해 스캔할 경우(exConChar = true) 더 빠름 
export int queryTextWidth(std::wstring text, bool isColorCodeText)
{
	int widthVar = -1;
	queryTextSize(text, &widthVar, nullptr, isColorCodeText);
	return widthVar;
}

export int queryTextWidthColorCode(std::wstring text) { return queryTextWidth(text, true); }
export int queryTextWidth(std::wstring text) { return queryTextWidth(text,false); }

//@brief 텍스트 높이 스캔, 제어문자를 포함해 스캔할 경우(exConChar = true) 더 빠름 
export int queryTextHeight(std::wstring text, bool isColorCodeText)
{
	int heightVar = -1;
	queryTextSize(text, nullptr, &heightVar, isColorCodeText);
	return heightVar;
}

export std::wstring eraseColorCodeText(std::wstring text)
{
	for (int i = text.size() - 1; i >= 0; i--)
	{
		if (text[i] == UNI::HASH)
		{
			text = text.erase(i, 7);
		}
		else if (text[i] == '\n')
		{
			text = text.erase(i, 1);
		}
	}
	return text;
}

//  @brief 입력한 텍스트의 가로 너비가 일정 크기(픽셀) 이상일 경우 텍스트를 분해, wstring이 들어있는 사이즈 2의 배열을 반환함
export std::array<std::wstring, 2> textSplitter(std::wstring text, int widthLimit)
{
	static std::unordered_map<std::wstring, std::array<std::wstring, 2>> splitCache;
	std::wstring originalText = text;
	// 캐시에 값이 있으면 캐시된 값을 반환
	auto splitCached = splitCache.find(originalText);
	if (splitCached != splitCache.end()) { return splitCached->second; }

	int textNoColorWidth = 0;
	int hashWidth = 0; // #FFFFFF 7글자 사이즈
	int hashNumber = 0; // #FFFFFF 커서 i가 현재까지 발견한 #의 숫자
	queryTextSize(text, &textNoColorWidth, nullptr, true);
	queryTextSize(L"#FFFFFF", &hashWidth, nullptr, false);

	if (textNoColorWidth > widthLimit) // 만약 노컬러가 일정 크기 이상이면 분할 시작
	{
		for (int i = 0; i < text.size(); i++)
		{
			int fragWidth;
			queryTextSize(text.substr(0, i + 1), &fragWidth, nullptr, false);
			if (fragWidth - (hashNumber * hashWidth) > widthLimit)
			{
				std::wstring lastHash = L"";
				std::wstring return1 = text.substr(0, i);
				std::wstring return2 = text.erase(0, i);

				for (int j = 0; j < return1.size(); j++)
				{
					if (return1[j] == '\n')
					{
						return2 = return1.substr(j + 1) + return2;
						return1 = return1.erase(j);
						break;
					}
				}

				if (return2[0] != UNI::HASH)//return2의 첫 문자가 #가 아니면
				{
					for (int j = return1.size(); j >= 0; j--)//return1의 뒤부터 순회
					{
						if (j <= (return1.size() - 1) - 6) //단 # 오른쪽에 여분 6칸의 자리가 있어야 하는 j 조건
						{
							if (return1[j] == UNI::HASH)
							{
								lastHash = return1.substr(j, 7);
								break;
							}
						}
					}
				}
				std::array<std::wstring, 2> result = { return1, lastHash + return2 }; // return1, lastHash + return2가 최종 결과라고 가정
				splitCache[originalText] = result;
				return result;
			}

			if (text[i] == UNI::HASH) { hashNumber++; }
		}
	}
	else //노컬러가 일정 크기 이하면 라인피드가 있는지만 검사하고 잘라냄
	{
		for (int i = 0; i < text.size(); i++)
		{
			if (text[i] == '\n') //라인피드 발생
			{
				std::wstring return1 = text.substr(0, i);
				std::wstring return2 = text.erase(0, i + 1);
				std::wstring lastHash;
				if (return2[0] != UNI::HASH)//return2의 첫 문자가 #가 아니면
				{
					for (int j = return1.size(); j >= 0; j--)//return1의 뒤부터 순회
					{
						if (j <= (return1.size() - 1) - 6) //단 # 오른쪽에 여분 6칸의 자리가 있어야 하는 j 조건
						{
							if (return1[j] == UNI::HASH)
							{
								lastHash = return1.substr(j, 7);
								break;
							}
						}
					}
				}
				std::array<std::wstring, 2> result = { return1, lastHash + return2 }; // return1, lastHash + return2가 최종 결과라고 가정
				splitCache[originalText] = result;
				return result;
			}
		}
		std::array<std::wstring, 2> result = { text, L"" };
		splitCache[originalText] = result;
		return result; //라인피드도 없고 문자열 너비도 작을 경우
	}
}

//주의할 점!!!! 색이 달라도 캐싱은 같은 걸로 취급함!!!!!!!!!!!!!
export void drawTextEx(std::wstring text, int x, int y, bool center)
{
	static std::unordered_map<std::wstring, SDL_Texture*> textCache;
	std::wstring cacheKey = text + L"_" + std::to_wstring(s_fontSize);

	SDL_Color color;
	SDL_Texture* texture;
	SDL_Surface* surface;
	SDL_Rect dst;
	SDL_GetRenderDrawColor(renderer, &color.r, &color.g, &color.b, &color.a);

	if (text[0] != UNI::HASH)
	{
		std::wstring instantColor = L"#000000";
		instantColor[1] = color.r / 16 + 48;
		instantColor[2] = color.r % 16 + 48;
		instantColor[3] = color.g / 16 + 48;
		instantColor[4] = color.g % 16 + 48;
		instantColor[5] = color.b / 16 + 48;
		instantColor[6] = color.b % 16 + 48;
		text = instantColor + text;
	}

	std::wstring originText = text;

	for (int i = 1; i < text.size(); i++)//라인피드 스캔
	{
		if (text[i] == '\n')
		{
			drawTextEx(text.substr(0, i), x, y, center);
			int centerCorrection = 0;
			if (center == true) { centerCorrection = queryTextHeight(L"A", false) / 2; }
			drawTextEx(text.erase(0, i + 1), x, y + s_fontSize + s_fontGap + centerCorrection, center);
			return;
		}
	}

	for (int i = 1; i < text.size(); i++)//컬러코드 스캔
	{
		if (text[i] == UNI::HASH) //첫번째 샵 이외의 또다른 샵을 발견했을 경우
		{
			int textWidth;
			queryTextSize(text.substr(0, i), &textWidth, nullptr, true);

			//만약 Center가 실행되면 첫글자색까지만 Center가 발생하기에 수동으로 문자열의 길이를 구한 후에
			//center=false로 만든 후 수동으로 문자열의 길이의 절반만큼 매개변수에서 감해서 Center 구현
			int totalWidth = 0;
			if (center == true)
			{
				for (int j = 0; j < text.size(); j++)
				{
					if (j == text.size() - 1 || text[j] == '\n')
					{
						totalWidth = queryTextWidth(text.substr(0, j), true);
						break;
					}
				}
			}

			drawTextEx(text.substr(0, i), x - totalWidth / 2, y, false);
			drawTextEx(text.erase(0, i), x + textWidth - totalWidth / 2, y, false);
			return;
		}

		if (i == text.size() - 1) //샵 문자가 전체에서 1개만 있을 경우
		{
			int hashColor[3] = { 0, 0, 0 };
			int temp = 0;
			for (int k = 0; k < 3; k++)
			{
				switch (text[1 + (2 * k)])
				{
					default: temp = 15; break;
					case 48: temp = 0; break;
					case 49: temp = 1; break;
					case 50: temp = 2; break;
					case 51: temp = 3; break;
					case 52: temp = 4; break;
					case 53: temp = 5; break;
					case 54: temp = 6; break;
					case 55: temp = 7; break;
					case 56: temp = 8; break;
					case 57: temp = 9; break;
					case 65: temp = 10; break;
					case 66: temp = 11; break;
					case 67: temp = 12; break;
					case 68: temp = 13; break;
					case 69: temp = 14; break;
					case 70: temp = 15; break;
				}
				hashColor[k] += 16 * temp;

				switch (text[2 + (2 * k)])
				{
					default: temp = 15; break;
					case 48: temp = 0; break;
					case 49: temp = 1; break;
					case 50: temp = 2; break;
					case 51: temp = 3; break;
					case 52: temp = 4; break;
					case 53: temp = 5; break;
					case 54: temp = 6; break;
					case 55: temp = 7; break;
					case 56: temp = 8; break;
					case 57: temp = 9; break;
					case 65: temp = 10; break;
					case 66: temp = 11; break;
					case 67: temp = 12; break;
					case 68: temp = 13; break;
					case 69: temp = 14; break;
					case 70: temp = 15; break;
				}
				hashColor[k] += temp;
			}
			SDL_SetRenderDrawColor(renderer, hashColor[0], hashColor[1], hashColor[2], 0xff);
			SDL_GetRenderDrawColor(renderer, &color.r, &color.g, &color.b, &color.a);
			text = text.substr(7);
		}
	}

	if (textCache.find(cacheKey) != textCache.end())//캐시 매개변수가 true고 텍스쳐가 캐시에 이미 있을 경우
	{
		texture = textCache[cacheKey];
	}
	else//캐시가 false이거나 캐시에 텍스쳐가 없을 경우
	{
		Uint16* unicode = new Uint16[text.size() + 1]();
		for (int i = 0; i < text.size(); i++) { unicode[i] = text[i]; }
		unicode[text.size()] = 0;
		surface = TTF_RenderUNICODE_Blended(mainFont[s_fontSize], unicode, color);//병목 1/3
		texture = SDL_CreateTextureFromSurface(renderer, surface); //병목 2/3
		SDL_FreeSurface(surface);
		textCache[cacheKey] = texture;
		delete[] unicode;
	}

	SDL_QueryTexture(texture, nullptr, nullptr, &dst.w, &dst.h);
	if (center == false) { dst = { x, y, dst.w, dst.h }; }
	else { dst = { x - dst.w / 2, y - (dst.h) / 2 - 1, dst.w, dst.h }; }
	SDL_RenderCopy(renderer, texture, nullptr, &dst);
}

export void drawText(std::wstring text, int x, int y) { drawTextEx(text, x, y, false); }

export void drawTextCenter(std::wstring text, int x, int y) { drawTextEx(text, x, y, true); };

//@brief 일정 너비 이상이면 라인피드를 함.. height == -1일 경우에는 정해진 값대로 라인피드, \n를 입력하여 강제 라인피드 가능
//@return 만들어진 줄의 수, 3줄짜리 글이면 3을 반환
export int drawTextWidth(std::wstring text, int x, int y, bool center, int width, int height, int lineLimit) //입력한 너비를 벗어나면 자동으로 줄바꿈해서 재귀 호출
{
	std::array<std::wstring, 2> textPair = textSplitter(text, width);
	//두번째에서 잗공하지않는 원인은 뒤로 나온 문자열이 @를 포함하고 있어도 너비가 좁아 drawTextEx만 실행되기 떄문이다.

	//prt(L"첫번째 문자열 : %ws\n", textPair[0].c_str());
	//prt(L"두번째 문자열 : %ws\n", textPair[1].c_str());
	if (height == -1) { height = s_fontSize + s_fontGap; }

	if (lineLimit <= 0) { return 0; }
	else if (textPair[1] == L"")
	{
		drawTextEx(text, x, y, center);
		return 1;
	}
	else {
		drawTextEx(textPair[0], x, y, center);
		return 1 + drawTextWidth(textPair[1], x, y + height, center, width, height, lineLimit - 1);
	}
}

export int drawTextWidth(std::wstring text, int x, int y, bool center, int width, int height) //입력한 너비를 벗어나면 자동으로 줄바꿈해서 재귀 호출
{
	return drawTextWidth(text, x, y, center, width, height, 999);
}

export void setFontSize(int val) { s_fontSize = val; }

export void setFontGap(int val) { s_fontGap = val; }

export void drawTextShadow(std::wstring text, int x, int y)
{
	drawText(col2Str(col::black)+text.substr(7), x + 1, y + 1);
	drawText(text, x, y);
}

export void drawTextShadowAll(std::wstring text, int x, int y)
{
	drawText(col2Str(col::black) + text.substr(7), x + 1, y);
	drawText(col2Str(col::black) + text.substr(7), x - 1, y);
	drawText(col2Str(col::black) + text.substr(7), x, y+1);
	drawText(col2Str(col::black) + text.substr(7), x, y-1);
	drawText(text, x, y);
}
