#include <SDL.h>


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

//�� msg Ŭ������ ó�� ������� ���� GUI�̸� ��� GUI���� msg�� ������� �������
//msgIndex
//0 : null
//1 : ���� ������ ���� ����
//2 : ��� ����� �� ��� �� ����
//3 : ��� �� á�� �� ��� �� ����
//4 : ���� ������ �˻���
//5 : ��Ţ ���� ������ ���� ����
//6 : ��Ţ ���� ������ �˻���
export class Msg : public GUI
{
private:
	inline static Msg* ptr = nullptr;
	int msgCursor = -1; //Ű�����Է��� �� ���Ǵ� Ŀ��
	msgFlag msgType = msgFlag::deact; // �޽����� Ÿ��, -1�̸� ��Ȱ��ȭ, 0�̸� �Ϲ� �޽���, 1�̸� �Է� �޽��� �ڽ�
	std::wstring msgTitleText; //�޽��� �ڽ� �ϴ��� �ɼ�(���� Ȯ�� ��� ��)
	std::wstring msgText; //�޽��� �ڽ� ��ܿ� ǥ�õǴ� ����
	std::vector<std::wstring> msgOptionVec; //�޽��� �ڽ��� ǥ�õǴ� ����

	SDL_Rect msgBase;//�� �������� ��ü ������ �׷����� ��ġ
	std::vector<std::vector<SDL_Rect>> msgBtn;
	SDL_Rect msgInputBox;

	tabFlag prevTabType;//�޽��� â�� ���� ���� �� Ÿ��(���� �� ������� ���ư�)

public:
	Msg(msgFlag type, std::wstring inputTitle, std::wstring inputText, std::vector<std::wstring> option) : GUI(true)
	{
		prt(L"Msg : �����ڰ� ȣ��Ǿ����ϴ�.\n");
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;


		coAnswer = L""; //�ڷ�ƾ ��ü �ʱ�ȭ

		//�޼��� �ڽ� ������
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

		if (option::inputMethod == input::keyboard) { msgCursor = 0; }

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);

		prevTabType = tabType;
		tabType = tabFlag::closeWin;
	}
	~Msg()
	{
		prt(L"Msg : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
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

		const Uint8* state = SDL_GetKeyboardState(NULL);
		if (getFoldRatio() == 1.0)
		{
			setWindowAlpha(230);
			drawWindow(&msgBase, msgTitleText, 0);
			resetWindowAlpha();

			//������ �ڽ� �׸���
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

			if (exInput == true) drawTextWidth(msgText, msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60 - 40, true, 240, -1);
			else if (targetItemCode != 0)
			{
				drawTextWidth(msgText.substr(0, firstTextEnd + 1), msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60 - 47, true, 240, -1);



				int pivotX = msgBase.x + 22;
				int pivotY = msgBase.y + 86;
				SDL_Rect background = { pivotX - 11, pivotY - 4,275,44 };
				drawStadium(background.x, background.y, background.w, background.h, col::white, 20, 5);


				SDL_Rect iconBox = { pivotX,pivotY,36,36 };
				drawWindow(&iconBox);
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, itemDex[targetItemCode].sprIndex, pivotX + 18, pivotY + 18);
				setZoom(1.0);
				setFontSize(13);
				drawText(col2Str(col::white) + itemDex[targetItemCode].name, pivotX + 50, pivotY + 6);

				if (lastTextStart <= msgText.size() - 1)
				{
					setFontSize(16);
					drawTextWidth(msgText.substr(lastTextStart), msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60 - 40 + 83, true, 240, -1);
				}
			}
			else
			{
				drawTextWidth(msgText, msgBase.x + msgBase.w / 2, msgBase.y + 36 + 60, true, 240, -1);
			}


			//////////////////////////////////////////////////////////////////��ư///////////////////////////////////////////////////////////////////////


			//��ư ��� �Ѳ�
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

			//��ư ���ڵ�
			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			setFontSize(16);
			for (int i = 0; i < msgOptionVec.size(); i++)
			{
				drawTextCenter(msgOptionVec[i], msgBtn[msgOptionVec.size()][i].x + msgBtn[msgOptionVec.size()][i].w / 2, msgBtn[msgOptionVec.size()][i].y + msgBtn[msgOptionVec.size()][i].h / 2);
			}

			if (exInput == true)
			{
				//���� ������ �����ܰ� ���� �׸���
				bool lootItemExist = false;
				//�޽��� �Է� �ڽ�
				SDL_Rect fMsgInputBox = msgInputBox; // ������ ����� ���������� �簢��
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
				drawText(col2Str(col::white) + exInputTextCut, inputTextPoint.x, inputTextPoint.y);
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
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};

