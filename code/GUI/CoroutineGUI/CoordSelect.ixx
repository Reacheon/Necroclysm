
#include <SDL3/SDL.h>

export module CoordSelect;

import std;
import util;
import GUI;
import Player;
import World;
import checkCursor;
import constVar;
import globalVar;
import wrapVar;
import drawSprite;
import textureVar;
import drawText;
import log;
import ItemPocket;

//반환형 : wstring L"x좌표,y좌표,z좌표", ex) 3,5,1
export class CoordSelect : public GUI
{
private:
	inline static CoordSelect* ptr = nullptr;
	std::wstring telepathyStr = sysStr[175];
	int index = -1;
	std::wstring parameter = L"";
	std::vector<std::array<int, 2>> selectableCoord;
	
	bool advance = false; //좌표를 선택하고 확인 버튼을 한번 더 눌러야 진행되는 옵션2
	int advanceIconIndex = -1;

	CoordSelectFlag type = CoordSelectFlag::NONE;

public:
	CoordSelect(CoordSelectFlag inputType, std::wstring inputTelepathyStr) : GUI(true)
	{
		coAnswer.clear();
		type = inputType;
		telepathyStr = inputTelepathyStr;
		prt(L"CoordSelect : 생성자가 호출되었습니다..\n");
		ptr = this;
		tabType = tabFlag::closeWin;
	}
	CoordSelect(std::wstring inputTelepathyStr) : CoordSelect(CoordSelectFlag::NONE, inputTelepathyStr) { }


	//셀렉터블 코드는 상대좌표 기준
	CoordSelect(CoordSelectFlag inputType, std::wstring inputTelepathyStr, std::vector<std::array<int, 2>> inputSelectableCoord) : GUI(true)
	{
		coAnswer.clear();
		type = inputType;
		telepathyStr = inputTelepathyStr;
		prt(L"CoordSelect : 생성자가 호출되었습니다..\n");
		ptr = this;

		selectableCoord = inputSelectableCoord;
	}
	CoordSelect(std::wstring inputTelepathyStr, std::vector<std::array<int, 2>> inputSelectableCoord) : CoordSelect(CoordSelectFlag::NONE, inputTelepathyStr, inputSelectableCoord) { }

	~CoordSelect()
	{
		prt(L"CoordSelect : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;
		tabType = tabFlag::autoAtk;
		UIType = act::null;

	}

	static CoordSelect* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center) {}
	void drawGUI()
	{
		bool displaySelectableCursor = true;
		bool yellowFullRectCursor = false;

		if (type == CoordSelectFlag::FIRESTORM)
		{
			displaySelectableCursor = false;
			yellowFullRectCursor = true;
		}

		int markerIndex = 0;
		if (timer::timer600 % 30 < 5) { markerIndex = 0; }
		else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
		else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
		else if (timer::timer600 % 30 < 20) { markerIndex = 3; }
		else if (timer::timer600 % 30 < 25) { markerIndex = 4; }

		if (displaySelectableCursor)
		{
			//플레이어 주변의 타일을 체크해 선택이 가능한 좌표만 옐로커서로 표시
			for (int i = 0; i < selectableCoord.size(); i++)
			{
				int revX = PlayerX() - selectableCoord[i][axis::x];
				int revY = PlayerY() - selectableCoord[i][axis::y];

				setZoom(zoomScale);
				drawSpriteCenter
				(
					spr::yellowMarker,
					markerIndex,
					cameraW / 2 + zoomScale * ((16 * revX + 8) - cameraX),
					cameraH / 2 + zoomScale * ((16 * revY + 8) - cameraY)
				);
				setZoom(1.0);
			}
		}


		if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&quickSlotRegion) == false)
		{
			int revX, revY, revGridX, revGridY;
			if (option::inputMethod == input::touch)
			{
				revX = event.tfinger.x * cameraW - (cameraW / 2);
				revY = event.tfinger.y * cameraH - (cameraH / 2);
			}
			else
			{
				revX = getMouseX() - (cameraW / 2);
				revY = getMouseY() - (cameraH / 2);
			}
			revX += sgn(revX) * (8 * zoomScale);
			revGridX = revX / (16 * zoomScale);
			revY += sgn(revY) * (8 * zoomScale);
			revGridY = revY / (16 * zoomScale);

			setZoom(zoomScale);

			auto drawGridRect = [](int inputRevX, int inputRevY, SDL_Color inputCol, Uint8 inputAlpha)
				{
					double tileSize = 16 * zoomScale;
					SDL_Rect dst;
					dst.x = cameraW / 2 + zoomScale * ((16 * inputRevX)) - ((16 * zoomScale) / 2);
					dst.y = cameraH / 2 + zoomScale * ((16 * inputRevY)) - ((16 * zoomScale) / 2);
					dst.w = tileSize;
					dst.h = tileSize;
					drawFillRect(dst, inputCol, inputAlpha);
				};

			if (yellowFullRectCursor)
			{
				for (int dx = -2; dx <= 2; dx++)
				{
					for (int dy = -2; dy <= 2; dy++)
					{
						if (std::abs(dx) <= 1 && std::abs(dy) <= 1) drawGridRect(revGridX + dx, revGridY + dy, col::yellow, 120);
						else drawGridRect(revGridX + dx, revGridY + dy, col::yellow, 60);
					}
				}
			}
			else
			{
				drawSpriteCenter
				(
					spr::whiteMarker,
					markerIndex,
					(cameraW / 2) + (int)(16.0 * zoomScale * revGridX),
					(cameraH / 2) + (int)(16.0 * zoomScale * revGridY)
				);
			}


			setZoom(1.0);
		}

		//사념파
		drawSpriteCenter(spr::floatLog, 0, cameraW / 2 + 20, 105);
		renderTextCenter(telepathyStr, cameraW / 2 + 20, 105);
	}
	void clickUpGUI()
	{
		if (checkCursor(&tab) == true)
		{
			tabType = tabFlag::autoAtk;
			close(aniFlag::null);
		}
		else
		{
			if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false && checkCursor(&quickSlotRegion) == false)
			{
				//상대좌표를 절대좌표로 변환
				int throwingX = getAbsMouseGrid().x;
				int throwingY = getAbsMouseGrid().y;
				int throwingZ = PlayerZ();
				std::wstring xStr = std::to_wstring(throwingX);
				std::wstring yStr = std::to_wstring(throwingY);
				std::wstring zStr = std::to_wstring(throwingZ);

				prt(L"[CoordSelect] 절대좌표 (%d,%d) 타일을 터치했다.\n", wtoi(xStr.c_str()), wtoi(yStr.c_str()));

				if (rangeSet.size() > 0)
				{
					Point2 targetPoint(throwingX, throwingY);
					if (rangeSet.find(targetPoint) != rangeSet.end())
					{
						coAnswer = xStr + L"," + yStr + L"," + zStr;
						prt(L"[CoordSelect] coAnswer의 값은 %ls이다.\n", coAnswer.c_str());
						delete this;
					}
					else
					{
						prt(L"[CoordSelect] 해당 좌표는 선택할 수 없다.\n");
					}
				}
				else if (selectableCoord.size() > 0)
				{
					for (int i = 0; i < selectableCoord.size(); i++)
					{
						if (selectableCoord[i][axis::x] == throwingX && selectableCoord[i][axis::y] == throwingY)
						{
							coAnswer = xStr + L"," + yStr + L"," + zStr;
							prt(L"[CoordSelect] coAnswer의 값은 %ls이다.\n", coAnswer.c_str());
							delete this;
							break;
						}

						if (i == selectableCoord.size() - 1)
						{
							prt(L"[CoordSelect] 해당 좌표는 선택할 수 없다.\n");
						}
					}
				}
				else
				{

					if (TileFov(throwingX, throwingY, throwingZ) == fovFlag::white)
					{
						coAnswer = xStr + L"," + yStr + L"," + zStr;
						prt(L"[CoordSelect] coAnswer의 값은 %ls이다.\n", coAnswer.c_str());
						delete this;
					}
					else
					{
						prt(L"[CoordSelect] 해당 좌표는 시야에 보이지 않는다.\n");
					}
				}
			}
		}
	}
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() {}
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};