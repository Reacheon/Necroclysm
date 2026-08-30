module;
#include <SDL3/SDL.h>

export module Map;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import constVar;
import WorldData;

export class Map : public GUI
{
private:
	inline static Map* ptr = nullptr;
	SDL_Rect mapBase;
	int mapCursor = -1;
	int mapScroll = 0;
public:
	Map() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"중복된 GUI 인스턴스가 생성되었다.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		currentWorld.reset();
		SDL_DestroyTexture(texture::worldmap);
		texture::worldmap = nullptr;
		static std::uint64_t attempt = 10000;
		attempt++;
		currentWorld = std::make_unique<WorldData>(getSeed() ^ (attempt * 0x9E3779B97F4A7C15ULL));

		if (texture::worldmap == nullptr)
		{
			if (currentWorld != nullptr)
			{
				texture::worldmap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WORLD_DATA_SIZE, WORLD_DATA_SIZE);
				SDL_SetTextureScaleMode(texture::worldmap, SDL_SCALEMODE_NEAREST);

				SDL_SetRenderTarget(renderer, texture::worldmap);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);
				for (int y = 0; y < WORLD_DATA_SIZE; y++)
				{
					for (int x = 0; x < WORLD_DATA_SIZE; x++)
					{

						//노이즈맵 표시
						//Uint8 bright = (Uint8)((currentWorld->noiseMap[x][y] + 1.0f) * 0.5f * 255);
						//drawPoint(x, y, SDL_Color{ bright, bright, bright });

						if (currentWorld->getProphecy(x, y, 0) == chunkType::deepSea) drawPoint(x, y, { 0x36,0x58,0xc3 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::shallowSea) drawPoint(x, y, { 0x53,0xa6,0xcf });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::dirt) drawPoint(x, y, { 0x7a,0xd4,0x33 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::beach) drawPoint(x, y, { 0xec,0xec,0xc3 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::mountain) drawPoint(x, y, { 0x84,0x74,0x66 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::forest) drawPoint(x, y, { 0x38,0xa6,0x41 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::river) drawPoint(x, y, { 0x53,0xa6,0xcf });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::lake) drawPoint(x, y, { 0x53,0xa6,0xcf });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::snow) drawPoint(x, y, { 0xf2,0xf6,0xf7 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::desert) drawPoint(x, y, { 0xee,0xea,0x8b });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::city) drawPoint(x, y, { 0xa2,0xa2,0xa2 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::volcanicLand) drawPoint(x, y, { 0x4d,0x42,0x42 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::volcano) drawPoint(x, y, { 0xc3,0x2e,0x2e });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::jungle) drawPoint(x, y, { 0x18,0x4d,0x1d });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::cityRoad) drawPoint(x, y, { 0x0,0x0,0x0 });
						else
						{
							if (currentWorld->buildingID.contains({ x, y, 0 }))
							{
								int h = (currentWorld->buildingID.at({ x, y, 0 }) * 137) % 360; //황금각
								int s = 70, v = 95;
								int r = 0, g = 0, b = 0;
								HSV2RGB(h, s, v, r, g, b);
								drawPoint(x, y, { (Uint8)r, (Uint8)g, (Uint8)b });
							}
							else drawPoint(x, y, { 0xff,0x00,0xff });
						}

						//Uint8 bright = (Uint8)((currentWorld->filledHeightMap[x][y]- currentWorld->heightMap[x][y] + 1.0f) * 0.5f * 255);
						//drawPoint(x, y, SDL_Color{ bright, bright, bright });
					}
				}
				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
				SDL_SetRenderTarget(renderer, frameTarget);
			}
		}
	}
	~Map()
	{
		ptr = nullptr;
	}
	static Map* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		mapBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			mapBase.x += inputX;
			mapBase.y += inputY;
		}
		else
		{
			mapBase.x += inputX - mapBase.w / 2;
			mapBase.y += inputY - mapBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - mapBase.w / 2;
			y = inputY - mapBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		drawFillRect(SDL_FRect{ 0,0,static_cast<float>(cameraW),static_cast<float>(cameraH) }, col::black, 255);


		if (texture::worldmap != nullptr)
		{
			drawTexture(texture::worldmap, 0, 0);
		}


		//탭 버튼 그리기
		{
			SDL_Color btnColor = { 0x00, 0x00, 0x00 };
			if (checkCursor(&tab))
			{
				if (click == false) { btnColor = lowCol::blue; }
				else { btnColor = lowCol::deepBlue; }
			}

			drawStadium(tab.x, tab.y, tab.w, tab.h, btnColor, 150, 5);
			setZoom(1.5);
			drawSpriteCenter(spr::icon48, 182, tab.x + 90, tab.y + 78);
			setZoom(1.0);
			setFontSize(22);
			drawTextCenter(sysStr[21], tab.x + 90, tab.y + 150);
			drawSpriteCenter(spr::keyboardButtons, keyboardIndex::tab + SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_TAB], tab.x + 164, tab.y + 8);
		}

	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::null);
		}
		else
		{
		}
	}

	void step()
	{
		tabType = tabFlag::back;
	}
};
