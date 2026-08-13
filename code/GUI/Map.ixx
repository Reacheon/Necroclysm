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
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::dirt) drawPoint(x, y, { 0x7a,0xd4,0x33 });
						else if (currentWorld->getProphecy(x, y, 0) == chunkType::beach) drawPoint(x, y, { 0xec,0xec,0xc3 });
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
