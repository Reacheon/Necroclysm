module;
#include <SDL3/SDL.h>

export module LstEx;

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
import Sprite;

//LstEx 옵션 구조체 : 스프라이트 인덱스, 이름, 출처(노란색 우측상단 표시)
export struct LstExOption {
	int sprIndex = 0;
	std::wstring name;
	std::wstring source; //출처 표시 (우측상단 노란색 작은 글씨)
};

//확장형 리스트 GUI
//기존 Lst에 아이템 스프라이트 표시 + 출처(source) 표시 기능을 추가한 코루틴 GUI
//coAnswer 반환형 : 선택한 목록의 인덱스 정수형 (Lst와 동일), 예로 0번째 선택지를 고르면 L"0" 반환
export class LstEx : public GUI
{
private:
	const int MAX_BTN = 9;
	int displayCount = MAX_BTN; //실제 표시되는 버튼 수 (옵션 수에 따라 동적)
	int extraLines = 0; //안내 문자열이 1줄 초과 시 추가 줄 수
	inline static LstEx* ptr = nullptr;
	int lstScroll = 0;
	std::wstring lstTitleText;
	std::wstring lstText;
	std::vector<LstExOption> lstOptionVec;

	Sprite* sprSet = nullptr; //왼쪽에 표시할 스프라이트셋 (nullptr이면 스프라이트 미표시)
	float sprZoom = 2.0f;     //스프라이트 줌 배율 (16px 실질 콘텐츠 기준, 2.0 = 32px 표시)
	bool showSource = true;   //출처 표시 여부

	SDL_Rect lstBase;
	SDL_Rect lstWindow;
	SDL_Rect lstScrollBox;
	std::vector<SDL_Rect> lstBtn;

public:
	LstEx(std::wstring inputTitle, std::wstring inputText, std::vector<LstExOption> options, Sprite* inputSprSet = nullptr, bool inputShowSource = true) : GUI(true)
	{
		coAnswer.clear();
		dbgPrt(L"LstEx 객체가 생성되었다.\n");
		errorBox(ptr != nullptr, L"중복된 GUI 인스턴스가 생성되었다.");
		ptr = this;

		lstTitleText = inputTitle;
		lstText = inputText;
		lstOptionVec = std::move(options);
		sprSet = inputSprSet;
		showSource = inputShowSource;

		//옵션 수에 따라 표시 버튼 수 결정
		displayCount = myMin(MAX_BTN, (int)lstOptionVec.size());
		if (displayCount < 1) displayCount = 1;

		//안내 문자열 줄 수 계산 (1줄 초과분만큼 높이 확장)
		setFontSize(20);
		int lineCount = queryLineCount(lstText, 500 - 15);
		extraLines = myMax(0, lineCount - 1);

		changeXY(cameraW / 2, cameraH / 2, true);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}

	~LstEx()
	{
		dbgPrt(L"LstEx : 소멸자가 호출되었습니다.\n");
		ptr = nullptr;

		exInput = false;
		exInputCursor = 0;
		exInputEditing = false;
		exInputIndex = -1;
	}

	static LstEx* ins() { return ptr; }

	void changeXY(int inputX, int inputY, bool center)
	{
		int lineH = extraLines * 24; //추가 줄에 의한 높이 확장
		int dynamicH = 150 + displayCount * 50 + lineH;
		lstBase = { 0, 0, 500, dynamicH };
		lstWindow = { 0, 54, 500, dynamicH - 54 };

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
		int btnOffsetX = needScroll ? (481 - 450) / 2 : (lstBase.w - 450) / 2;
		lstBtn.resize(displayCount);
		for (int i = 0; i < displayCount; i++)
			lstBtn[i] = { lstWindow.x + btnOffsetX, lstWindow.y + 62 + lineH + 50 * i, 450, 44 };

		lstScrollBox = { lstWindow.x + 481, lstWindow.y + 62 + lineH, 3, displayCount * 50 - 5 };

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

				//버튼 배경 (보류 옵션임 투명 회색)
				//if (isClick)      drawFillRect(lstBtn[i], SDL_Color{ 25, 40, 120 }, 230);
				//else if (isHover) drawFillRect(lstBtn[i], SDL_Color{ 35, 55, 150 }, 200);
				//else              drawFillRect(lstBtn[i], SDL_Color{ 30, 30, 35 }, 180);

				drawRect(lstBtn[i], col::gray);

				const LstExOption& opt = lstOptionVec[currentItemIndex];

				//스프라이트 표시
				int textStartX = lstBtn[i].x + 14;
				if (sprSet != nullptr)
				{
					setZoom(sprZoom);
					drawSpriteCenter(sprSet, opt.sprIndex, lstBtn[i].x + 24, lstBtn[i].y + 22);
					setZoom(1.0f);
					textStartX = lstBtn[i].x + 48;
				}

				//아이템 이름 표시
				setFontSize(21);
				setFont(fontType::mainFont);
				drawText(opt.name, textStartX, lstBtn[i].y + 10);

				//출처 표시 (우측상단, 노란색, 작은 폰트)
				if (showSource && !opt.source.empty())
				{
					setFontSize(13);
					int sourceTextW = queryTextWidth(opt.source);
					drawText(opt.source, lstBtn[i].x + lstBtn[i].w - sourceTextW - 4, lstBtn[i].y + 4, col::yellow);
				}
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
			//펼치기/접기 애니메이션
			SDL_Rect vRect = lstBase;
			vRect.x = vRect.x + vRect.w * (1 - getFoldRatio()) / 2;
			vRect.w = vRect.w * getFoldRatio();
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
