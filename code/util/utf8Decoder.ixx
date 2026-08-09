export module utf8Decoder;

import std;
import dbgPrt;

//  @brief utf-8로 인코딩된 값을 유니코드로 바꾸는 함수
//  @param byte1 첫번째 바이트
//  @param byte2 두번째 바이트
//  @param byte3 세번째 바이트
//  @param byte4 네번째 바이트
//  @return 부호없는 4바이트의 유니코드 정수 반환
export std::uint32_t utf8Decoder(char byte1, char byte2, char byte3, char byte4)
{
	int strByte = -1;
	std::string bit[4];
	bit[0] = std::bitset<8>(byte1).to_string();
	bit[1] = std::bitset<8>(byte2).to_string();
	bit[2] = std::bitset<8>(byte3).to_string();
	bit[3] = std::bitset<8>(byte4).to_string();

	//앞의 비트를 보고 입력된 UTF-8이 몇 바이트인지 체크함(예로 한글은 3바이트)
	//아스키 숫자로 저장되기에 48을 빼서 0과 1로 변환해주어야 함
	if (bit[0][0] - 48 == 0) { strByte = 1; } //0xxxxxxx
	else if (bit[0][0] - 48 == 1)
	{
		if (bit[0][1] - 48 == 1)
		{
			if (bit[0][2] - 48 == 0) { strByte = 2; } //110xxxxx
			else if (bit[0][2] - 48 == 1)
			{
				if (bit[0][3] - 48 == 0) { strByte = 3; } //1110xxxx
				else if (bit[0][3] - 48 == 1)
				{
					if (bit[0][4] - 48 == 0) { strByte = 4; } //11110xxx
				}

			}
		}
	}

	if (strByte == -1) { dbgPrt(L"[에러] utf8Decoder가 UTF-8 인코딩이 아닌 잘못된 입력값을 받았습니다.\n"); }

	//decode에는 완성한 값이 한 칸당 유니코드 숫자로 들어감
	std::string decode;
	int startPoint = 1; // 첫번째 바이트에서 읽기 시작하는 지점
	int bitSize = 7; //바이트 당 할당된 비트(고정 문자 제외된 값임)
	switch (strByte)
	{
		default:
			startPoint = 1;
			bitSize = 7;
			for (int i = 0; i < bitSize; i++) { decode += "0"; }
			break;
		case 2:
			startPoint = 3;
			bitSize = 11;
			for (int i = 0; i < bitSize; i++) { decode += "0"; }
			break;
		case 3:
			startPoint = 4;
			bitSize = 16;
			for (int i = 0; i < bitSize; i++) { decode += "0"; }
			break;
		case 4:
			startPoint = 5;
			bitSize = 21;
			for (int i = 0; i < bitSize; i++) { decode += "0"; }
			break;
	}


	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			//dbgPrt(L"%d번째 바이트의 비트들은 %d 이다.", i+1, bit[i][j] - 48);
		}
		//dbgPrt(L"\n");
	}

	//각 비트의 값을 decode에 붙혀넣는다. 반드시 48을 빼줄 것
	for (int i = startPoint; i < 8; i++)
	{
		//dbgPrt(L"decode의 %d번째에 %d를 입력했다.\n", i, bit[0][i] - 48);
		decode[i - startPoint] += bit[0][i] - 48;
		int lastMax0 = i - startPoint;
		if (i == 7)
		{
			if (strByte == 1) { break; }
			for (int j = 2; j < 8; j++)
			{
				decode[lastMax0 + 1 + (j - 2)] += bit[1][j] - 48;
				int lastMax1 = lastMax0 + 1 + (j - 2);
				if (j == 7)
				{
					if (strByte == 2) { break; }
					for (int k = 2; k < 8; k++)
					{
						decode[lastMax1 + 1 + (k - 2)] += bit[2][k] - 48;
						int lastMax2 = lastMax1 + 1 + (k - 2);
						if (k == 7)
						{
							if (strByte == 3) { break; }
							for (int l = 2; l < 8; l++)
							{
								decode[lastMax2 + 1 + (l - 2)] += bit[3][l] - 48;
							}
						}
					}
				}
			}
		}
	}

	std::uint32_t unicode = 0;
	for (int i = 0; i < bitSize; i++)
	{
		if (decode[i] - 48 == 1)
		{
			unicode += std::pow(2, bitSize - i - 1);
		}
	}
	//dbgPrt(L"디코딩 결과값은 %d이다.\n", number);
	return unicode;
}


// 기존 함수는 그대로 두고, 문자열을 받는 오버로드 추가
export std::wstring utf8Decoder(const char* utf8Str)
{
	if (!utf8Str) return L"";

	std::wstring result;
	size_t len = std::strlen(utf8Str);

	for (size_t i = 0; i < len; )
	{
		char byte1 = utf8Str[i];
		char byte2 = (i + 1 < len) ? utf8Str[i + 1] : 0;
		char byte3 = (i + 2 < len) ? utf8Str[i + 2] : 0;
		char byte4 = (i + 3 < len) ? utf8Str[i + 3] : 0;

		std::uint32_t unicode = utf8Decoder(byte1, byte2, byte3, byte4);

		if (unicode != 0)
		{
			result += static_cast<wchar_t>(unicode);
		}

		// UTF-8 바이트 수만큼 인덱스 증가
		if ((byte1 & 0x80) == 0) i += 1;      // ASCII (0xxxxxxx)
		else if ((byte1 & 0xE0) == 0xC0) i += 2; // 2바이트 (110xxxxx)
		else if ((byte1 & 0xF0) == 0xE0) i += 3; // 3바이트 (1110xxxx)
		else if ((byte1 & 0xF8) == 0xF0) i += 4; // 4바이트 (11110xxx)
		else i += 1; // 오류 시 1바이트만 건너뛰기
	}

	return result;
}

// std::string을 받는 버전도 추가
export std::wstring utf8Decoder(const std::string& utf8Str)
{
	return utf8Decoder(utf8Str.c_str());
}

//============================================================
// utf8Encoder : std::wstring(또는 wstring_view) → UTF-8 std::string.
//   <codecvt>/std::wstring_convert(C++26 제거 예정) 대체 — 자체 구현, 플랫폼 무관.
//   sizeof(wchar_t)에 따라:
//     Windows(2바이트, UTF-16): surrogate pair → 코드포인트 → UTF-8
//     Linux/macOS(4바이트, UTF-32): wchar_t = 코드포인트 그대로
//   잘못된 surrogate / 범위 외 코드포인트(>0x10FFFF)는 U+FFFD로 대체.
//============================================================
export std::string utf8Encoder(std::wstring_view wide)
{
	std::string out;
	out.reserve(wide.size() * 2); //대략 한글 평균 3바이트, ASCII 1바이트

	auto appendCodepoint = [&out](std::uint32_t cp)
	{
		if (cp <= 0x7F)
		{
			out.push_back(static_cast<char>(cp));
		}
		else if (cp <= 0x7FF)
		{
			out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
			out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
		}
		else if (cp <= 0xFFFF)
		{
			out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
			out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
			out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
		}
		else if (cp <= 0x10FFFF)
		{
			out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
			out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
			out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
			out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
		}
		//범위 외(>0x10FFFF)는 무시 — appendCodepoint에서 받지 않도록 호출측에서 가드
	};

	for (std::size_t i = 0; i < wide.size(); ++i)
	{
		std::uint32_t cp = 0;

		if constexpr (sizeof(wchar_t) == 2)
		{
			//UTF-16: surrogate pair 디코드
			std::uint32_t hi = static_cast<std::uint16_t>(wide[i]);
			if (hi >= 0xD800 && hi <= 0xDBFF && i + 1 < wide.size())
			{
				std::uint32_t lo = static_cast<std::uint16_t>(wide[i + 1]);
				if (lo >= 0xDC00 && lo <= 0xDFFF)
				{
					cp = 0x10000 + ((hi - 0xD800) << 10) + (lo - 0xDC00);
					++i; //low surrogate 소비
				}
				else
				{
					cp = 0xFFFD; //orphan high surrogate
				}
			}
			else if (hi >= 0xDC00 && hi <= 0xDFFF)
			{
				cp = 0xFFFD; //orphan low surrogate
			}
			else
			{
				cp = hi;
			}
		}
		else
		{
			//UTF-32: wchar_t가 코드포인트 그 자체
			cp = static_cast<std::uint32_t>(wide[i]);
			if (cp > 0x10FFFF) cp = 0xFFFD;
		}

		appendCodepoint(cp);
	}

	return out;
}