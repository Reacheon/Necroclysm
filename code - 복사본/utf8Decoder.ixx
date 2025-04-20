export module utf8Decoder;

import std;
import prt;

//  @brief utf-8로 인코딩된 값을 유니코드로 바꾸는 함수
//  @param byte1 첫번째 바이트
//  @param byte2 두번째 바이트
//  @param byte3 세번째 바이트
//  @param byte4 네번째 바이트
//  @return 부호없는 4바이트의 유니코드 정수 반환
export unsigned long utf8Decoder(char byte1, char byte2, char byte3, char byte4)
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

	if (strByte == -1) { prt(L"[에러] utf8Decoder가 UTF-8 인코딩이 아닌 잘못된 입력값을 받았습니다.\n"); }

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
			//prt(L"%d번째 바이트의 비트들은 %d 이다.", i+1, bit[i][j] - 48);
		}
		//prt(L"\n");
	}

	//각 비트의 값을 decode에 붙혀넣는다. 반드시 48을 빼줄 것
	for (int i = startPoint; i < 8; i++)
	{
		//prt(L"decode의 %d번째에 %d를 입력했다.\n", i, bit[0][i] - 48);
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

	unsigned long unicode = 0;
	for (int i = 0; i < bitSize; i++)
	{
		if (decode[i] - 48 == 1)
		{
			unicode += std::pow(2, bitSize - i - 1);
		}
	}
	//prt(L"디코딩 결과값은 %d이다.\n", number);
	return unicode;
}