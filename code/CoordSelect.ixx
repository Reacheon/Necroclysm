#include <SDL.h>

export module CoordSelect;

import std;
import util;
import GUI;
import Player;
import World;
import checkCursor;
import constVar;
import globalVar;
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
	std::wstring telepathyStr = L"좌표를 선택해주세요.";
	int index = -1;
	std::wstring parameter = L"";
	std::vector<std::array<int, 2>> selectableCoord;
	
	bool advance = false; //좌표를 선택하고 확인 버튼을 한번 더 눌러야 진행되는 옵션
	int advanceIconIndex = -1;
	
public:
	CoordSelect(std::wstring inputTelepathyStr) : GUI(true)
	{
		telepathyStr = inputTelepathyStr;
		prt(L"CoordSelect : 생성자가 호출되었습니다..\n");
		ptr = this;
	}

	CoordSelect(std::wstring inputTelepathyStr, std::vector<std::array<int, 2>> inputSelectableCoord) : GUI(true)
	{
		selectableCoord = inputSelectableCoord;
		telepathyStr = inputTelepathyStr;
		prt(L"CoordSelect : 생성자가 호출되었습니다..\n");
		ptr = this;
	}

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
		int markerIndex = 0;
		if (timer::timer600 % 30 < 5) { markerIndex = 0; }
		else if (timer::timer600 % 30 < 10) { markerIndex = 1; }
		else if (timer::timer600 % 30 < 15) { markerIndex = 2; }
		else if (timer::timer600 % 30 < 20) { markerIndex = 3; }
		else if (timer::timer600 % 30 < 25) { markerIndex = 4; }

		//플레이어 주변의 타일을 체크해 선택이 가능한 좌표만 표시
		for (int i = 0; i < selectableCoord.size(); i++)
		{
			int revX = Player::ins()->getGridX() - selectableCoord[i][axis::x];
			int revY = Player::ins()->getGridY() - selectableCoord[i][axis::y];

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

		if (checkCursor(&letterbox) == false && checkCursor(&tab) == false)
		{
			int revX, revY, revGridX, revGridY;
			if (inputType == input::touch)
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

			setZoom(zoomScale);
			drawSpriteCenter
			(
				spr::blueMarker,
				markerIndex,
				(cameraW / 2) + (int)(16.0 * zoomScale * revGridX),
				(cameraH / 2) + (int)(16.0 * zoomScale * revGridY)
			);
			setZoom(1.0);
		}

		//사념파
		drawSpriteCenter(spr::floatLog, 0, cameraW / 2, 165);
		drawTextCenter(L"#FFFFFF" + telepathyStr, cameraW / 2, 165);
	}
	void clickUpGUI()
	{
		if (checkCursor(&tab) == true)
		{
			UIType = act::null;
			tabType = tabFlag::autoAtk;
			delete this;
		}
		else
		{
			if (checkCursor(&letterbox) == false && checkCursor(&tab) == false && checkCursor(&letterboxPopUpButton) == false)
			{
				int revX, revY, revGridX, revGridY;
				if (inputType == input::touch)
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

				prt(L"[CoordSelect] 절대좌표 (%d,%d) 타일을 터치했다.\n", wtoi(xStr.c_str()), wtoi(yStr.c_str()));


				if (selectableCoord.size() > 0)
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

					if (World::ins()->getTile(throwingX, throwingY, throwingZ).fov == fovFlag::white)
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
	void step() { }
};