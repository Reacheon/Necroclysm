module;
#include <SDL3_image/SDL_image.h>

module World;

import std;
import util;
import Sector;
import constVar;

// 섹터 PNG를 로드해 픽셀별 지형을 Sector에 채움
// 1픽셀 = 50타일. 청크와는 완전 분리
void World::createSector(int sectorX, int sectorY, int sectorZ)
{
	if (sectorZ == 0)
	{
		// 섹터 범위 체크: 기존 PNG 라인업 보존
		if ((sectorY <= 26 && sectorY >= -27) && (sectorX <= 53 && sectorX >= -54))
		{
			std::string filePath = "map/worldSector-";
			int number = 2971 + sectorX + 108 * sectorY;
			if (number < 100) filePath += "0";
			filePath += std::to_string(number);
			filePath += ".png";
			std::wstring wPath(filePath.begin(), filePath.end());

			SDL_Surface* refSector = IMG_Load(filePath.c_str());

			if (!refSector)
			{
				std::wstring sdlErr = stringToWstring(std::string(SDL_GetError()));
				std::wstring cwd = std::filesystem::current_path().wstring();

				std::wstring msg =
					L"IMG_Load 실패\n"
					L"  SDL_GetError : " + sdlErr +
					L"\n  시도한 경로   : " + wPath +
					L"\n  현재 CWD      : " + cwd + L'\n';

				prt(L"%ls", msg.c_str());
			}

			errorBox(refSector == NULL, L"섹터의 파일 읽기가 실패하였습니다. :"
				+ std::to_wstring(sectorX) + L"," + std::to_wstring(sectorY) + L"," + std::to_wstring(sectorZ));

			Uint32* pixels = (Uint32*)refSector->pixels;

			auto babySector = std::make_unique<Sector>();

			auto isSameCol = [](SDL_Color col1, SDL_Color col2) -> bool
				{
					return col1.r == col2.r && col1.g == col2.g && col1.b == col2.b;
				};

			for (int px = 0; px < PIXEL_PER_SECTOR; px++)
			{
				for (int py = 0; py < PIXEL_PER_SECTOR; py++)
				{
					Uint32 pixel = pixels[(py * refSector->w) + px];
					SDL_Color pixelCol;
					SDL_GetRGB(pixel,
						SDL_GetPixelFormatDetails(refSector->format),
						SDL_GetSurfacePalette(refSector),
						&pixelCol.r, &pixelCol.g, &pixelCol.b);

					chunkFlag targetFlag = chunkFlag::none;

					if      (isSameCol(pixelCol, chunkCol::seawater)) targetFlag = chunkFlag::seawater;
					else if (isSameCol(pixelCol, chunkCol::land))     targetFlag = chunkFlag::dirt;
					else if (isSameCol(pixelCol, chunkCol::city))     targetFlag = chunkFlag::city;
					else if (isSameCol(pixelCol, chunkCol::river))    targetFlag = chunkFlag::freshwater;
					else if (isSameCol(pixelCol, chunkCol::bridge))   targetFlag = chunkFlag::bridge;

					babySector->set(px, py, targetFlag);
				}
			}

			sectorMap[{ sectorX, sectorY, sectorZ }] = std::move(babySector);

			SDL_DestroySurface(refSector);
		}
	}

	isSectorCreated.insert({ sectorX,sectorY,sectorZ });
}
