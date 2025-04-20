#include <SDL.h>


export module Maint;

import std;
import util;
import GUI;
import globalVar;
import wrapVar;
import drawText;
import checkCursor;
import drawSprite;
import ItemPocket;
import Player;
import textureVar;
import drawWindow;
import World;
import Vehicle;

export enum class maintFlag
{
	repair,
	detach,
};


//Maint는 Msg랑 다르게 coAnswer로 문자열이 아니라 선택한 입력지의 인덱스를 반환할것
//coAnswer 반환형 : 현재 선택한 목록의 인덱스 숫자 정수형, 예로 0번째 선택지를 고르면 L"0" 반환
export class Maint : public GUI
{
private:
	maintFlag maintMode;
	const int MAX_BTN = 9;
	inline static Maint* ptr = nullptr;
	int maintIndex = -1;
	int maintCursor = -1; //키보드입력일 때 사용되는 커서
	int maintScroll = 0; //스크롤
	std::wstring maintTitleText; //메시지 박스 하단의 옵션(선택 확인 취소 등)
	std::wstring maintText; //메시지 박스 상단에 표시되는 문구
	std::vector<std::wstring> maintOptionVec; //메시지 박스에 표시되는 문구
	std::wstring maintParameter = L""; //메시지를 열 때 전달되는 매개변수

	SDL_Rect maintBase;//이 윈도우의 전체 면적과 그려지는 위치
	SDL_Rect maintTitle;
	SDL_Rect maintWindow;
	SDL_Rect maintScrollBox; //리스트 스크롤

	std::vector<SDL_Rect> maintBtn;
	SDL_Rect maintInputBox;

	tabFlag prevTabType;//메시지 창을 열기 전의 탭 타입(닫을 때 원래대로 돌아감)

	Point3 maintCoor;

public:
	Maint(std::wstring inputTitle, std::wstring inputText, Point3 inputCoor, maintFlag inputMode) : GUI(true)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		prt(L"Maint 객체가 생성되었다.\n");
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		maintTitleText = inputTitle;
		maintText = inputText;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);

		maintCoor = inputCoor;
		maintMode = inputMode;

		prevTabType = tabType;
		tabType = tabFlag::back;

	}

	~Maint()
	{
		prt(L"Maint : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		exInput = false;
		exInputCursor = 0;
		exInputEditing = false;
		exInputIndex = -1;

		tabType = tabFlag::autoAtk;

	}
	static Maint* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		maintBase = { 0, 0, 280, 393 };
		maintTitle = { 140 - 65,0,130,30 };
		maintWindow = { 0,36,280,357 };

		if (center == false)
		{
			maintBase.x += inputX;
			maintBase.y += inputY;
			maintTitle.x += inputX;
			maintTitle.y += inputY;
			maintWindow.x += inputX;
			maintWindow.y += inputY;
		}
		else
		{
			maintBase.x += inputX - maintBase.w / 2;
			maintBase.y += inputY - maintBase.h / 2;
			maintTitle.x += inputX - maintBase.w / 2;
			maintTitle.y += inputY - maintBase.h / 2;
			maintWindow.x += inputX - maintBase.w / 2;
			maintWindow.y += inputY - maintBase.h / 2;
		}

		maintBtn.clear();
		for (int i = 0; i < MAX_BTN; i++)
		{
			SDL_Rect tempMaintBtn = { maintWindow.x + 22 ,maintWindow.y + 48 + 32 * i, 240, 29 };
			maintBtn.push_back(tempMaintBtn);
		}

		maintInputBox = { maintWindow.x + 50,maintWindow.y + 60, 200, 40 };

		maintScrollBox = { maintWindow.x + 271,maintWindow.y + 48, 2, 285 };

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - maintBase.w / 2;
			y = inputY - maintBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) return;

		Vehicle* vPtr = (Vehicle*)World::ins()->getTile(maintCoor.x, maintCoor.y, maintCoor.z).VehiclePtr;
		if (!vPtr) return;
		auto partIter = vPtr->partInfo.find({ maintCoor.x, maintCoor.y });
		if (partIter == vPtr->partInfo.end() || !partIter->second) return;
		std::vector<ItemData>& items = partIter->second->itemInfo;

		if (getFoldRatio() == 1.0)
		{
			if (maintMode == maintFlag::repair) drawWindow(&maintBase, maintTitleText, 86);
			else drawWindow(&maintBase, maintTitleText, 87);

			SDL_Rect topWindow = { maintBase.x + 1, maintBase.y + 30, 278, 44 };
			SDL_Rect botWindow = { maintBase.x + 1, maintBase.y + maintBase.h - 17, 278, 16 };
			drawFillRect(topWindow, col::black, 180);
			drawFillRect(botWindow, col::black, 180);

			setFontSize(14);
			drawTextCenter(col2Str(col::white) + maintText, maintWindow.x + maintWindow.w / 2, maintBase.y + 30 + 22);

			//선택지 버튼 그리기
			int hoverCursor = -1;

			for (int i = 0; i < MAX_BTN; i++)
			{
				int currentItemIndex = maintScroll + i;

				if (currentItemIndex >= 0 && currentItemIndex < items.size())
				{
					int selectBoxIndex = 0;

					if (checkCursor(&maintBtn[i]))
					{
						hoverCursor = currentItemIndex;
						if (click == true) selectBoxIndex = 2;
						else selectBoxIndex = 1;
					}

					drawSprite(spr::lstSelectBox, selectBoxIndex, maintBtn[i].x, maintBtn[i].y);

					setZoom(2.0);
					drawSpriteCenter(spr::itemset, items[currentItemIndex].sprIndex, maintBtn[i].x + 15, maintBtn[i].y + 14);
					setZoom(1.0);

					setFontSize(14);
					drawText(col2Str(col::white) + items[currentItemIndex].name, maintBtn[i].x + 36, maintBtn[i].y + 5);

					drawRect({ maintBtn[i].x + 230, maintBtn[i].y + 3, 7, 23 }, col::lightGray);
					drawFillRect({ maintBtn[i].x + 230 + 2, maintBtn[i].y + 3 + 2, 3, 19 }, lowCol::green);
				}
			}

			setFontSize(10);
			std::wstring hoverText = L"-";
			if (hoverCursor != -1) hoverText = std::to_wstring(hoverCursor + 1);
			drawTextCenter(col2Str(col::white) + hoverText + L"/" + std::to_wstring(items.size()), maintWindow.x + maintWindow.w - 30, maintBase.y + maintBase.h - 17 + 8);

			// 아이템 스크롤 그리기
			if (items.size() > MAX_BTN)
			{
				drawFillRect(maintScrollBox, { 120,120,120 });

				SDL_Rect inScrollBox = maintScrollBox;
				inScrollBox.h = maintScrollBox.h * myMin(1.0, (float)MAX_BTN / (float)items.size());
				if (inScrollBox.h < 5) inScrollBox.h = 5;

				if (!items.empty()) inScrollBox.y = maintScrollBox.y + maintScrollBox.h * ((float)maintScroll / (float)items.size());
				else inScrollBox.y = maintScrollBox.y;

				if (inScrollBox.y < maintScrollBox.y) inScrollBox.y = maintScrollBox.y;
				if (inScrollBox.y + inScrollBox.h > maintScrollBox.y + maintScrollBox.h) inScrollBox.y = maintScrollBox.y + maintScrollBox.h - inScrollBox.h;

				drawFillRect(inScrollBox, col::white);
			}
		}
		else
		{
			SDL_Rect vRect = maintBase;
			int type = 1;
			switch (type)
			{
			case 0:
				vRect.w = vRect.w * getFoldRatio();
				vRect.h = vRect.h * getFoldRatio();
				break;
			case 1:
				vRect.x = vRect.x + vRect.w * (1 - getFoldRatio()) / 2;
				vRect.w = vRect.w * getFoldRatio();
				break;
			}
			// drawStadium(vRect.x, vRect.y, vRect.w, vRect.h, { 0,0,0 }, 230, 5);
			drawWindow(&vRect);
		}
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		for (int i = 0; i < maintBtn.size(); i++)
		{
			if (checkCursor(&maintBtn[i]))
			{
				coAnswer = std::to_wstring(maintScroll + i);
				close(aniFlag::null);
			}
		}

		if (checkCursor(&tab))
		{
			coAnswer = std::to_wstring(-1);
			close(aniFlag::null);
			return;
		}
	}
	void clickMotionGUI(int dx, int dy) {}
	void clickDownGUI() {}
	void clickRightGUI() {}
	void clickHoldGUI() {}
	void mouseWheel() 
	{
		Vehicle* vPtr = (Vehicle*)World::ins()->getTile(maintCoor.x, maintCoor.y, maintCoor.z).VehiclePtr;
		std::vector<ItemData>& items = vPtr->partInfo[{maintCoor.x, maintCoor.y}]->itemInfo;

		if (checkCursor(&maintBase))
		{
			if (event.wheel.y > 0 && maintScroll > 0) maintScroll -= 1;
			else if (event.wheel.y < 0 && maintScroll + MAX_BTN < items.size()) maintScroll += 1;
		}
	}
	void gamepadBtnDown() {}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}
	void step() 
	{
		Vehicle* vPtr = (Vehicle*)World::ins()->getTile(maintCoor.x, maintCoor.y, maintCoor.z).VehiclePtr;
		std::vector<ItemData>& items = vPtr->partInfo[{maintCoor.x, maintCoor.y}]->itemInfo;

		if (items.empty() || items.size() <= MAX_BTN) maintScroll = 0;
		else
		{
			if (maintScroll < 0) maintScroll = 0;
			int maxScroll = (int)items.size() - MAX_BTN;
			if (maintScroll > maxScroll) maintScroll = maxScroll;
		}
	}
};

