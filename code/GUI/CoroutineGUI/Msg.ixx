#include <SDL3/SDL.h>


export module Msg;

import std;
import util;
import globalVar;
import wrapVar;
import constVar;
import errorBox;
import textureVar;
import drawText;
import checkCursor;
import drawSprite;
import World;
import Player;
import log;
import Ani;
import Sticker;
import Sprite;
import GUI;
import ItemStack;
import ItemPocket;
import drawWindow;

//이 msg 클래스는 처음 만들어진 기초 GUI이며 모든 GUI들은 msg를 기반으로 만들어짐
//msgIndex
//0 : null
//1 : 루팅 아이템 갯수 선택
//2 : 양손 비었을 때 드는 손 선택
//3 : 양손 꽉 찼을 때 드는 손 선택
//4 : 루팅 아이템 검색어
//5 : 이큅 루팅 아이템 갯수 선택
//6 : 이큅 루팅 아이템 검색어
export class Msg : public GUI
{
private:
	inline static Msg* ptr = nullptr;
	int msgCursor = -1; //키보드입력일 때 사용되는 커서
	msgFlag msgType = msgFlag::deact; // 메시지의 타입, -1이면 비활성화, 0이면 일반 메시지, 1이면 입력 메시지 박스
	std::wstring msgTitleText; //메시지 박스 하단의 옵션(선택 확인 취소 등)
	std::wstring msgText; //메시지 박스 상단에 표시되는 문구
	std::vector<std::wstring> msgOptionVec; //메시지 박스에 표시되는 문구

	SDL_Rect msgBase;//이 윈도우의 전체 면적과 그려지는 위치
	std::vector<std::vector<SDL_Rect>> msgBtn;
	SDL_Rect msgInputBox;

	tabFlag prevTabType;//메시지 창을 열기 전의 탭 타입(닫을 때 원래대로 돌아감)

public:
	Msg(msgFlag type, std::wstring inputTitle, std::wstring inputText, std::vector<std::wstring> option) : GUI(true)
	{
		prt(L"Msg : 생성자가 호출되었습니다.\n");
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;


		coAnswer = L""; //코루틴 객체 초기화

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		msgType = type;
		msgTitleText = inputTitle;
		msgText = inputText;
		msgOptionVec = option;

		exInputText.clear();

		if (type == msgFlag::input)
		{
			exInput = true;
			exInputText.clear();
			exInputCursor = 0;
			exInputIndex = 0;
			exInputEditing = false;
		}


		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);

		prevTabType = tabType;
		tabType = tabFlag::closeWin;
	}
	~Msg()
	{
		prt(L"Msg : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		if (msgType == msgFlag::input)
		{
			exInput = false;
			exInputCursor = 0;
			exInputEditing = false;
			exInputIndex = -1;
		}

		tabType = prevTabType;


	}
	static Msg* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		msgBase = { 0, 0, 300, 236 };

		if (center == false)
		{
			msgBase.x += inputX;
			msgBase.y += inputY;
		}
		else
		{
			msgBase.x += inputX - msgBase.w / 2;
			msgBase.y += inputY - msgBase.h / 2;
		}
		/*
		*  236 59
		*/
		msgBtn =
		{
			{ },
			{ {msgBase.x + 36 ,msgBase.y + msgBase.h - 54, 90 , 46} },
			{ {msgBase.x + 36 ,msgBase.y + msgBase.h - 54, 90 , 46}, {msgBase.x + msgBase.w - 126 ,msgBase.y + msgBase.h - 54, 90 , 46} },
			{ {msgBase.x + 7,msgBase.y + msgBase.h - 54, 90 , 46}, {msgBase.x + 105,msgBase.y + msgBase.h - 54, 90 ,46}, {msgBase.x + 203,msgBase.y + msgBase.h - 54, 90 , 46} }
		};
		msgInputBox = { msgBase.x + 50,msgBase.y + 36 + 60, 200, 40 };


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - msgBase.w / 2;
			y = inputY - msgBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		const bool* state = SDL_GetKeyboardState(nullptr);
		if (getFoldRatio() == 1.0)
		{
			setWindowAlpha(230);
			//drawWindow(&msgBase, msgTitleText, 0);
			drawSprite(spr::msgBox, msgBase.x, msgBase.y);

			setFontSize(22);
			renderTextCenter(msgTitleText, msgBase.x+150, msgBase.y+34);

			drawSpriteCenter(spr::icon32, 1,msgBase.x + 150- queryTextWidth(msgTitleText) / 2.0 -24, msgBase.y + 34);
			

			resetWindowAlpha();

			//윈도우 박스 그리기
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			setFontSize(16);

			int numberStart;
			int targetItemCode = 0;
			int firstTextEnd = 0;
			int lastTextStart = 0;
			for (int i = 0; i < msgText.size(); i++)
			{
				if (msgText[i] == UNI::LEFT_PARENTHESIS)
					if (i + 1 < msgText.size() && msgText[i + 1] == UNI::PERCENT_SIGN)
						if (i + 2 < msgText.size() && msgText[i + 2] == UNI::i)
							if (i + 3 < msgText.size() && msgText[i + 3] == UNI::t)
								if (i + 4 < msgText.size() && msgText[i + 4] == UNI::e)
									if (i + 5 < msgText.size() && msgText[i + 5] == UNI::m)
										if (i + 6 < msgText.size())
										{
											for (int j = i + 6; j < msgText.size(); j++)
											{
												if (msgText[j] == UNI::RIGHT_PARENTHESIS)
												{
													targetItemCode = wtoi(msgText.substr(i + 6, j - (i + 6)));
													firstTextEnd = i - 1;
													lastTextStart = j + 1;
													goto loopEnd;
												}
											}
										}
			}
		loopEnd:

			if (exInput == true) renderTextWidth(msgText, msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60 - 40 + 14, true, 240, -1);
			else if (targetItemCode != 0)
			{
				renderTextWidth(msgText.substr(0, firstTextEnd + 1), msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60 - 47, true, 240, -1);



				int pivotX = msgBase.x + 22;
				int pivotY = msgBase.y + 86;
				SDL_Rect background = { pivotX - 11, pivotY - 4,275,44 };
				drawStadium(background.x, background.y, background.w, background.h, col::white, 20, 5);


				SDL_Rect iconBox = { pivotX,pivotY,36,36 };
				drawWindow(&iconBox);
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, itemDex[targetItemCode].sprIndex, pivotX + 18, pivotY + 18);
				setZoom(1.0);
				setFontSize(12);
				renderText(itemDex[targetItemCode].name, pivotX + 50, pivotY + 6);

				if (lastTextStart <= msgText.size() - 1)
				{
					setFontSize(16);
					renderTextWidth(msgText.substr(lastTextStart), msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60 - 40 + 83, true, 240, -1);
				}
			}
			else
			{
				renderTextWidth(msgText, msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60, true, 240, -1);
			}


			//////////////////////////////////////////////////////////////////버튼///////////////////////////////////////////////////////////////////////


			//버튼 흰색 뚜껑
			drawLine(msgBase.x + 1, msgBase.y + msgBase.h - 63, msgBase.x + msgBase.w - 1, msgBase.y + msgBase.h - 63, { 0x63,0x63,0x63 });

			for (int i = 0; i < msgOptionVec.size(); i++)
			{
				int btnColorSprIndex = 0;
				if (checkCursor(&msgBtn[msgOptionVec.size()][i]) == true)
				{
					if (click == true) btnColorSprIndex = 2;
					else btnColorSprIndex = 1;
				}
				drawSprite(spr::msgChoiceBtn, btnColorSprIndex, msgBtn[msgOptionVec.size()][i].x, msgBtn[msgOptionVec.size()][i].y);
			}

			//버튼 글자들
			setFontSize(16);
			for (int i = 0; i < msgOptionVec.size(); i++)
			{
				renderTextCenter(msgOptionVec[i], msgBtn[msgOptionVec.size()][i].x + msgBtn[msgOptionVec.size()][i].w / 2, msgBtn[msgOptionVec.size()][i].y + msgBtn[msgOptionVec.size()][i].h / 2);
			}

			if (exInput == true)
			{
				//좌측 아이템 아이콘과 숫자 그리기
				bool lootItemExist = false;
				//메시지 입력 박스
				SDL_Rect fMsgInputBox = msgInputBox; // 아이템 존재로 최종보정된 사각형
				fMsgInputBox.x = msgInputBox.x + 30 * lootItemExist;
				drawRect(fMsgInputBox, col::white);

				const unsigned __int16 maxTextWidth = 170;
				SDL_Point inputTextPoint = { fMsgInputBox.x + 10, msgInputBox.y + 9 };
				setFontSize(16);

				std::wstring exInputTextCut = exInputText;
				while (queryTextWidth(exInputTextCut, false) > maxTextWidth)
				{
					exInputTextCut = exInputTextCut.substr(1);
				}
				renderText(exInputTextCut, inputTextPoint.x, inputTextPoint.y);
				std::wstring cursorText = exInputTextCut.substr(0, exInputCursor + exInputEditing);
				if (timer::timer600 % 30 <= 15 && exInput == true)
				{
					int textWidth = queryTextWidth(cursorText, false);
					int textHeight = queryTextHeight(cursorText, false);
					drawLine(inputTextPoint.x + textWidth - 1, inputTextPoint.y, inputTextPoint.x + textWidth - 1, inputTextPoint.y + textHeight, col::white);
				}

			}
		}
		else
		{
			SDL_Rect vRect = msgBase;

			int type = 1;
			switch (type)
			{
			case 0:
				vRect.w = msgBase.w * getFoldRatio();
				vRect.h = msgBase.h * getFoldRatio();
				break;
			case 1:
				vRect.x = msgBase.x + msgBase.w * (1 - getFoldRatio()) / 2;
				vRect.w = msgBase.w * getFoldRatio();
				break;
			}
			drawStadium(vRect.x, vRect.y, vRect.w, vRect.h, { 0,0,0 }, 230, 5);
		}

	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		for (int i = 0; i < msgOptionVec.size(); i++)
		{
			if (checkCursor(&msgBtn[msgOptionVec.size()][i]))
			{
				coAnswer = msgOptionVec[i];
				close(aniFlag::null);
			}
		}

		if (checkCursor(&tab))
		{
			coAnswer = L"";
			close(aniFlag::null);
			return;
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

