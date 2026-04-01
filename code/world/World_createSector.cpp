module;
#include <SDL3_image/SDL_image.h>

module World;

import std;
import util;
import Mapmaker;
import constVar;

void World::createSector(int sectorX, int sectorY, int sectorZ)
{
	if (sectorZ == 0)
	{
		if ((sectorY <= 26 && sectorY >= -27) && (sectorX <= 53 && sectorX >= -54))
		{
			std::string filePath = "map/worldSector-";
			int number = 2971 + sectorX + 108 * sectorY;
			if (number < 100) filePath += "0";
			filePath += std::to_string(number);
			filePath += ".png";
			std::wstring wPath(filePath.begin(), filePath.end());
			//std::wprintf(L"[World] Sector : %ls의 파일을 읽어내었다.\n", wPath.c_str());
			SDL_Surface* refSector = IMG_Load(filePath.c_str());

			if (!refSector)   // 디버깅: 로드 실패 이유 출력
			{
				// SDL_GetError() → std::string → std::wstring
				std::wstring sdlErr = stringToWstring(std::string(SDL_GetError()));

				// 실행 중의 작업 디렉터리 (char→wstring)
				std::wstring cwd = std::filesystem::current_path().wstring();

				std::wstring msg =
					L"IMG_Load 실패\n"
					L"  SDL_GetError : " + sdlErr +
					L"\n  시도한 경로   : " + wPath +
					L"\n  현재 CWD      : " + cwd + L'\n';

				prt(L"%ls", msg.c_str());               // 콘솔/디버그 출력
			}

			errorBox(refSector == NULL, L"섹터의 파일 읽기가 실패하였습니다. :" + std::to_wstring(sectorX) + L"," + std::to_wstring(sectorY) + L"," + std::to_wstring(sectorZ));
			Uint32* pixels = (Uint32*)refSector->pixels;


			for (int x = 0; x < 400; x++)
			{
				for (int y = 0; y < 400; y++)
				{
					chunkFlag targetFlag = chunkFlag::none;

					Uint32 pixel = pixels[(y * refSector->w) + x];
					SDL_Color pixelCol;
					SDL_GetRGB(pixel,
						SDL_GetPixelFormatDetails(refSector->format),
						SDL_GetSurfacePalette(refSector),
						&pixelCol.r, &pixelCol.g, &pixelCol.b);

					auto isSameCol = [](SDL_Color col1, SDL_Color col2)->bool
						{
							if (col1.r == col2.r)
							{
								if (col1.g == col2.g)
								{
									if (col1.b == col2.b)
									{
										return true;
									}
								}
							}

							return false;
						};

					if (isSameCol(pixelCol, chunkCol::seawater)) targetFlag = chunkFlag::seawater;
					else if (isSameCol(pixelCol, chunkCol::land)) targetFlag = chunkFlag::dirt;
					else if (isSameCol(pixelCol, chunkCol::city)) targetFlag = chunkFlag::dirt;
					else if (isSameCol(pixelCol, chunkCol::river)) targetFlag = chunkFlag::seawater;

					//섹터 좌표로 청크 좌표 구하기
					int chunkOriginX, chunkOriginY;
					chunkOriginX = 400 * sectorX;
					chunkOriginY = 400 * sectorY;

					Mapmaker::ins()->addProphecy(chunkOriginX + x, chunkOriginY + y, sectorZ, targetFlag);
				}
			}

			SDL_DestroySurface(refSector);
		}
	}

	isSectorCreated.insert({ sectorX,sectorY,sectorZ });
}
