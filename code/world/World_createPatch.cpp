module;
#include <SDL3_image/SDL_image.h>

module World;

import std;
import util;
import Patch;
import constVar;

// 패치 PNG를 로드해 픽셀별 지형을 Patch에 채움
// 1픽셀 = 50타일. 청크와는 완전 분리
void World::createPatch(int patchX, int patchY, int patchZ)
{
	if (patchZ == 0)
	{
		// 패치 범위 체크: 기존 PNG 라인업 보존
		if ((patchY <= 26 && patchY >= -27) && (patchX <= 53 && patchX >= -54))
		{
			int number = 2971 + patchX + 108 * patchY;
			std::string filePath = std::format("map/worldPatch-{:03d}.png", number);
			std::wstring wPath(filePath.begin(), filePath.end());

			SDL_Surface* refPatch = IMG_Load(filePath.c_str());

			if (!refPatch)
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

			errorBox(refPatch == NULL, L"패치의 파일 읽기가 실패하였습니다. :"
				+ std::to_wstring(patchX) + L"," + std::to_wstring(patchY) + L"," + std::to_wstring(patchZ));

			Uint32* pixels = (Uint32*)refPatch->pixels;

			auto babyPatch = std::make_unique<Patch>();

			auto isSameCol = [](SDL_Color col1, SDL_Color col2) -> bool
				{
					return col1.r == col2.r && col1.g == col2.g && col1.b == col2.b;
				};

			for (int px = 0; px < PIXEL_PER_PATCH; px++)
			{
				for (int py = 0; py < PIXEL_PER_PATCH; py++)
				{
					Uint32 pixel = pixels[(py * refPatch->w) + px];
					SDL_Color pixelCol;
					SDL_GetRGB(pixel,
						SDL_GetPixelFormatDetails(refPatch->format),
						SDL_GetSurfacePalette(refPatch),
						&pixelCol.r, &pixelCol.g, &pixelCol.b);

					chunkFlag targetFlag = chunkFlag::none;

					if      (isSameCol(pixelCol, pngPatchPixelCol::seawater)) targetFlag = chunkFlag::seawater;
					else if (isSameCol(pixelCol, pngPatchPixelCol::land))     targetFlag = chunkFlag::dirt;
					else if (isSameCol(pixelCol, pngPatchPixelCol::city))     targetFlag = chunkFlag::city;
					else if (isSameCol(pixelCol, pngPatchPixelCol::river))    targetFlag = chunkFlag::freshwater;
					else if (isSameCol(pixelCol, pngPatchPixelCol::bridge))   targetFlag = chunkFlag::bridge;
					else if (isSameCol(pixelCol, pngPatchPixelCol::mountain))     targetFlag = chunkFlag::dirt;

					babyPatch->set(px, py, targetFlag);
				}
			}

			patchMap[{ patchX, patchY, patchZ }] = std::move(babyPatch);

			SDL_DestroySurface(refPatch);
		}
	}

	isPatchCreated.insert({ patchX,patchY,patchZ });
}
