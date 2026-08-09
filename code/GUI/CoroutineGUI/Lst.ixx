module;
#include <SDL3/SDL.h>

export module Lst;

import std;
import util;
import GUI;
import constVar;
import globalVar;
import drawText;
import checkCursor;
import drawSprite;
import ItemPocket;
import Player;
import textureVar;
import drawWindow;
import World;

//Lst는 Msg랑 다르게 coAnswer로 문자열이 아니라 선택한 입력지의 인덱스를 반환할것
//coAnswer 반환형 : 현재 선택한 목록의 인덱스 숫자 정수형, 예로 0번째 선택지를 고르면 L"0" 반환
export class Lst : public GUI
{
private:
	const int MAX_BTN = 9;
	int displayCount = MAX_BTN; //실제 표시되는 버튼 수 (옵션 수에 따라 동적)
	int extraLines = 0; //안내 문자열이 1줄 초과 시 추가 줄 수
	inline static Lst* ptr = nullptr;
	int lstScroll = 0; //스크롤
	std::wstring lstTitleText; //타이틀바에 표시되는 문구
	std::wstring lstText; //메시지 박스 상단에 표시되는 문구
	std::vector<std::wstring> lstOptionVec; //메시지 박스에 표시되는 문구

	SDL_Rect lstBase;    //이 윈도우의 전체 면적과 그려지는 위치
	SDL_Rect lstWindow;
	SDL_Rect lstScrollBox; //리스트 스크롤

	std::vector<SDL_Rect> lstBtn;
public:
	Lst(std::wstring inputTitle, std::wstring inputText, std::vector<std::wstring> option, bool useCorouter = true) : GUI(useCorouter)
	{
		coAnswer.clear();
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		dbgPrt(L"Lst 객체가 생성되었다.\n");
		errorBox(ptr != nullptr, L"중복된 GUI 인스턴스가 생성되었다.");
		ptr = this;

		lstTitleText = inputTitle;
		lstText = inputText;
		lstOptionVec = option;

		//옵션 수에 따라 표시 버튼 수 결정
		displayCount = myMin(MAX_BTN, (int)lstOptionVec.size());
		if (displayCount < 1) displayCount = 1;

		//안내 문자열 줄 수 계산 (1줄 초과분만큼 높이 확장)
		setFontSize(20);
		int lineCount = queryLineCount(lstText, 420 - 15);
		extraLines = myMax(0, lineCount - 1);

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);


		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}

	~Lst()
	{
		dbgPrt(L"Lst : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		exInput = false;
		exInputCursor = 0;
		exInputEditing = false;
		exInputIndex = -1;
	}
	static Lst* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		int lineH = extraLines * 24; //추가 줄에 의한 높이 확장
		int dynamicH = 150 + displayCount * 50 + lineH;
		lstBase = { 0, 0, 420, dynamicH };
		lstWindow = { 0, 54, 420, dynamicH - 54 };

		if (center == false)
		{
			lstBase.x += inputX;
			lstBase.y += inputY;
		}
		else
		{
			lstBase.x += inputX - lstBase.w / 2;
			lstBase.y += inputY - lstBase.h / 2;
		}

		lstWindow.x = lstBase.x;
		lstWindow.y = lstBase.y + 54;


		bool needScroll = (int)lstOptionVec.size() > displayCount;
		int btnOffsetX = needScroll ? 27 : (lstBase.w - 360) / 2;
		lstBtn.resize(displayCount);
		for (int i = 0; i < displayCount; i++) lstBtn[i] = { lstWindow.x + btnOffsetX, lstWindow.y + 62 + lineH + 50 * i, 360, 44 };

		lstScrollBox = { lstBase.x + 403, lstWindow.y + 62 + lineH, 3, displayCount * 50 - 5 };

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - lstBase.w / 2;
			y = inputY - lstBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) return;

		if (getFoldRatio() == 1.0)
		{
			setFont(fontType::mainFont);
			drawWindow(&lstBase, lstTitleText, 0);

			SDL_Rect topWindow = { lstBase.x + 1, lstBase.y + 35, lstBase.w - 2, 66 + extraLines * 24 };
			SDL_Rect botWindow = { lstBase.x + 1, lstBase.y + lstBase.h - 25, lstBase.w - 2, 24 };
			drawFillRect(topWindow, col::black, 255);
			drawFillRect(botWindow, col::black, 255);

			setFontSize(20);
			setFont(fontType::mainFont);
			drawTextCenterWidth(lstText, lstWindow.x + lstWindow.w / 2, lstBase.y + 45 + 22, lstBase.w - 15, -1);

			//선택지 버튼 그리기
			int hoverCursor = -1;

			for (int i = 0; i < displayCount; i++)
			{
				int currentItemIndex = lstScroll + i;
				if (currentItemIndex < 0 || currentItemIndex >= lstOptionVec.size()) continue;

				//마우스 호버/클릭 상태 감지
				bool isHover = checkCursor(&lstBtn[i]);
				bool isClick = isHover && click;
				if (isHover) hoverCursor = currentItemIndex;

				//버튼 배경
				if (isClick)      drawFillRect(lstBtn[i], SDL_Color{ 25, 40, 120 }, 255);
				else if (isHover) drawFillRect(lstBtn[i], SDL_Color{ 35, 55, 150 }, 255);
				else              drawFillRect(lstBtn[i], SDL_Color{ 0, 0, 0 }, 180);

				drawRect(lstBtn[i], col::gray);

				//텍스트 표시
				setFontSize(21);
				setFont(fontType::mainFont);
				drawText(lstOptionVec[currentItemIndex], lstBtn[i].x + 14, lstBtn[i].y + 8);
			}

			//하단 호버 인덱스 표시
			setFontSize(15);
			std::wstring hoverText = L"-";
			if (hoverCursor != -1) hoverText = std::to_wstring(hoverCursor + 1);
			drawTextCenter(hoverText + L"/" + std::to_wstring(lstOptionVec.size()), lstWindow.x + lstWindow.w - 45, lstBase.y + lstBase.h - 26 + 12);

			//스크롤바 (스크롤 필요할 때만 표시)
			if (lstOptionVec.size() > displayCount)
			{
				drawFillRect(lstScrollBox, { 120, 120, 120 });
				SDL_Rect inScrollBox = lstScrollBox;
				inScrollBox.h = lstScrollBox.h * myMin(1.0, (float)displayCount / (float)lstOptionVec.size());
				if (inScrollBox.h < 8) inScrollBox.h = 8;

				if (!lstOptionVec.empty()) inScrollBox.y = lstScrollBox.y + lstScrollBox.h * ((float)lstScroll / (float)lstOptionVec.size());
				else inScrollBox.y = lstScrollBox.y;

				if (inScrollBox.y < lstScrollBox.y) inScrollBox.y = lstScrollBox.y;
				if (inScrollBox.y + inScrollBox.h > lstScrollBox.y + lstScrollBox.h)
					inScrollBox.y = lstScrollBox.y + lstScrollBox.h - inScrollBox.h;

				drawFillRect(inScrollBox, col::white);
			}
		}
		else
		{
			SDL_Rect vRect = lstBase;
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
			drawWindow(&vRect);
		}
	}

	void clickUpGUI()
	{
		if (getStateInput() == false) return;

		for (int i = 0; i < displayCount; i++)
		{
			if (checkCursor(&lstBtn[i]))
			{
				int selectedIndex = lstScroll + i;

				if (selectedIndex >= 0 && selectedIndex < lstOptionVec.size())
				{
					coAnswer = std::to_wstring(selectedIndex);
					close(aniFlag::null);
					return;
				}
			}
		}

		if (checkCursor(&tab))
		{
			close(aniFlag::null);
			return;
		}
	}

	void mouseWheel()
	{
		if (checkCursor(&lstBase))
		{
			if (event.wheel.y > 0 && lstScroll > 0) lstScroll -= 1;
			else if (event.wheel.y < 0 && lstScroll + displayCount < lstOptionVec.size()) lstScroll += 1;
		}
	}

	void step()
	{
		tabType = tabFlag::back;

		if (lstOptionVec.empty() || lstOptionVec.size() <= displayCount) lstScroll = 0;
		else
		{
			if (lstScroll < 0) lstScroll = 0;
			int maxScroll = (int)lstOptionVec.size() - displayCount;
			if (lstScroll > maxScroll) lstScroll = maxScroll;
		}
	}
};