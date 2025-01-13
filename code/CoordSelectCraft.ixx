#include <SDL.h>


export module CoordSelectCraft;

import std;
import util;
import GUI;
import Player;
import World;
import Loot;
import checkCursor;
import constVar;
import globalVar;
import drawSprite;
import textureVar;
import drawText;
import drawWindow;
import log;
import ItemPocket;

//반환형 : wstring L"x좌표,y좌표,회전으로 변경된 itemCode", ex) 3,5,167
export class CoordSelectCraft : public GUI
{
private:
	inline static CoordSelectCraft* ptr = nullptr;
	std::wstring telepathyStr = sysStr[175];
	int index = -1;
	std::wstring parameter = L"";
	std::vector<std::array<int, 2>> selectableCoord;

	bool advance = false; //좌표를 선택하고 확인 버튼을 한번 더 눌러야 진행되는 옵션
	int advanceIconIndex = -1;

	bool targetSelect = false;
	int targetGridX = 0;
	int targetGridY = 0;
	int rotatedItemCode = 0;

	SDL_Rect selectBox = { 270, 400, 180, 52 };
	SDL_Rect confirmBtn = { selectBox.x + 8, selectBox.y + 11, 51, 30 };
	SDL_Rect rotateBtn = { selectBox.x + 8 + 57 * 1, selectBox.y + 11, 51, 30 };
	SDL_Rect cancelBtn = { selectBox.x + 8 + 57 * 2, selectBox.y + 11, 51, 30 };

public:

	CoordSelectCraft(int tgtItemCode, std::wstring inputTelepathyStr, std::vector<std::array<int, 2>> inputSelectableCoord) : GUI(true)
	{
		selectableCoord = inputSelectableCoord;
		telepathyStr = inputTelepathyStr;
		rotatedItemCode = tgtItemCode;
		prt(L"CoordSelectCraft : 생성자가 호출되었습니다..\n");
		ptr = this;
	}

	~CoordSelectCraft()
	{
		prt(L"CoordSelectCraft : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;
		tabType = tabFlag::autoAtk;
		UIType = act::null;
	}
	static CoordSelectCraft* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center) {}
	void drawGUI()
	{
		//사념파
		drawSpriteCenter(spr::floatLog, 0, cameraW / 2, 165);
		drawTextCenter(L"#FFFFFF" + telepathyStr, cameraW / 2, 165);

		if (targetSelect == false)
		{
			int markerIndex = 0;
			if (timer::timer600 % 30 < 5) { markerIndex = 0; }
			else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
			else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
			else if (timer::timer600 % 30 < 20) { markerIndex = 3; }
			else if (timer::timer600 % 30 < 25) { markerIndex = 4; }

			//플레이어 주변의 타일을 체크해 선택이 가능한 좌표만 표시
			for (int i = 0; i < selectableCoord.size(); i++)
			{
				int revX = Player::ins()->getGridX() + selectableCoord[i][axis::x];
				int revY = Player::ins()->getGridY() + selectableCoord[i][axis::y];

				setZoom(zoomScale);
				SDL_SetTextureAlphaMod(spr::yellowMarker->getTexture(), 120);
				drawSpriteCenter
				(
					spr::yellowMarker,
					markerIndex,
					cameraW / 2 + zoomScale * ((16 * selectableCoord[i][axis::x] + 8) - cameraX),
					cameraH / 2 + zoomScale * ((16 * selectableCoord[i][axis::y] + 8) - cameraY)
				);
				SDL_SetTextureAlphaMod(spr::yellowMarker->getTexture(), 255);
				setZoom(1.0);
			}


			if (checkCursor(&letterbox) == false && checkCursor(&tab) == false)
			{
				int revX, revY, revGridX, revGridY;
				if (option::inputMethod == input::touch)
				{
					revX = event.tfinger.x * cameraW - (cameraW / 2);
					revY = event.tfinger.y * cameraH - (cameraH / 2);
				}
				else
				{
					revX = event.motion.x - (cameraW / 2);
					revY = event.motion.y - (cameraH / 2);
				}
				revX += sgn(revX) * (8 * zoomScale);
				revGridX = revX / (16 * zoomScale);
				revY += sgn(revY) * (8 * zoomScale);
				revGridY = revY / (16 * zoomScale);

				if (itemDex[rotatedItemCode].checkFlag(itemFlag::PROP_BIG)) revGridY -= 1;

				setZoom(zoomScale);
				SDL_SetTextureColorMod(spr::propset->getTexture(), 0, 255, 0);
				SDL_SetTextureAlphaMod(spr::propset->getTexture(), 120);
				drawSpriteCenter
				(
					spr::propset,
					itemDex[rotatedItemCode].propSprIndex,
					(cameraW / 2) + (int)(16.0 * zoomScale * revGridX),
					(cameraH / 2) + (int)(16.0 * zoomScale * revGridY)
				);
				SDL_SetTextureColorMod(spr::propset->getTexture(), 255, 255, 255);
				SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
				setZoom(1.0);
			}


		}
		else//타겟 선택이 완료된 후
		{
			setZoom(zoomScale);
			SDL_SetTextureColorMod(spr::propset->getTexture(), 0, 255, 0);
			SDL_SetTextureAlphaMod(spr::propset->getTexture(), 120);
			drawSpriteCenter
			(
				spr::propset,
				itemDex[rotatedItemCode].propSprIndex,
				(cameraW / 2),
				(cameraH / 2) + itemDex[rotatedItemCode].checkFlag(itemFlag::PROP_BIG)*zoomScale * (-16 * 1 + 8)
			);
			SDL_SetTextureColorMod(spr::propset->getTexture(), 255, 255, 255);
			SDL_SetTextureAlphaMod(spr::propset->getTexture(), 255);
			setZoom(1.0);

			drawEdgeWindow(270, 400, 180, 52, 16, dir16::dir2);

			auto drawBtn = [](int iconIndex, SDL_Rect targetBtn)
				{
					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					if (checkCursor(&targetBtn))
					{
						if (click == false) { btnColor = lowCol::blue; }
						else { btnColor = lowCol::deepBlue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}

					drawFillRect(targetBtn, btnColor);
					drawRect(targetBtn, outlineColor);

					drawSpriteCenter(spr::icon16, iconIndex, targetBtn.x + targetBtn.w / 2, targetBtn.y + targetBtn.h / 2);
				};

			drawBtn(41, confirmBtn);
			
			if (itemDex[rotatedItemCode].dirChangeItemCode != 0)
			{
				drawBtn(42, rotateBtn);
			}
			else
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };

				drawFillRect(rotateBtn, btnColor);
				drawRect(rotateBtn, outlineColor);

				drawSpriteCenter(spr::icon16, 42, rotateBtn.x + rotateBtn.w / 2, rotateBtn.y + rotateBtn.h / 2);
				drawFillRect(rotateBtn, btnColor, 150);
			}

			drawBtn(43, cancelBtn);
		}
	}
	void clickUpGUI()
	{
		if (checkCursor(&tab) == true)
		{
			tabType = tabFlag::autoAtk;
			coAnswer.clear();
			cameraFix = true;
			delete this;
		}
		else
		{
			if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false)
			{
				if (targetSelect == false)//타겟 선택 전
				{
					int revX, revY, revGridX, revGridY;
					if (option::inputMethod == input::touch)
					{
						revX = event.tfinger.x * cameraW - (cameraW / 2);
						revY = event.tfinger.y * cameraH - (cameraH / 2);
					}
					else
					{
						revX = event.motion.x - (cameraW / 2);
						revY = event.motion.y - (cameraH / 2);
					}
					revX += sgn(revX) * (8 * zoomScale);
					revGridX = revX / (16 * zoomScale);
					revY += sgn(revY) * (8 * zoomScale);
					revGridY = revY / (16 * zoomScale);

					//상대좌표를 절대좌표로 변환
					int throwingX = Player::ins()->getGridX() + revGridX;
					int throwingY = Player::ins()->getGridY() + revGridY;
					int throwingZ = Player::ins()->getGridZ();
					std::wstring xStr = std::to_wstring(throwingX);
					std::wstring yStr = std::to_wstring(throwingY);
					std::wstring zStr = std::to_wstring(throwingZ);

					prt(L"절대좌표 (%d,%d) 타일을 터치했다.\n", wtoi(xStr.c_str()), wtoi(yStr.c_str()));


					if (selectableCoord.size() > 0)//선택좌표가 있는 경우일 때
					{
						for (int i = 0; i < selectableCoord.size(); i++)
						{
							if (selectableCoord[i][axis::x] == throwingX && selectableCoord[i][axis::y] == throwingY)
							{
								targetSelect = true;
								targetGridX = throwingX;
								targetGridY = throwingY;
								cameraFix = false;
								cameraX = 16 * targetGridX + 8;
								cameraY = 16 * targetGridY + 8;
								break;
							}
							lowCol::deepBlue;

							if (i == selectableCoord.size() - 1)
							{
								prt(L"해당 좌표는 선택할 수 없다.\n");
							}
						}
					}
					else//선택좌표가 없으면 그냥 시야가 흰색이면 선택 OK
					{

						if (World::ins()->getTile(throwingX, throwingY, throwingZ).fov == fovFlag::white)
						{
							targetSelect = true;
							targetGridX = throwingX;
							targetGridY = throwingY;
							cameraFix = false;
							cameraX = 16 * targetGridX + 8;
							cameraY = 16 * targetGridY + 8;
						}
						else
						{
							prt(L"해당 좌표는 시야에 보이지 않는다.\n");
						}
					}
				}
				else
				{
					if (checkCursor(&confirmBtn))
					{
						coAnswer = std::to_wstring(targetGridX) + L"," + std::to_wstring(targetGridY) + L"," + std::to_wstring(rotatedItemCode);
						cameraFix = true;
						delete this;
					}
					else if (checkCursor(&rotateBtn))
					{
						if (itemDex[rotatedItemCode].dirChangeItemCode != 0)
						{
							rotatedItemCode = itemDex[rotatedItemCode].dirChangeItemCode;
						}
					}
					else if (checkCursor(&cancelBtn))
					{
						coAnswer.clear();
						cameraFix = true;
						delete this;
					}
				}
			}
		}
	}
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};