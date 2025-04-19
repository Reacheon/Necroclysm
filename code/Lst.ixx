#include <SDL.h>

export module Lst;

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

//Lst�� Msg�� �ٸ��� coAnswer�� ���ڿ��� �ƴ϶� ������ �Է����� �ε����� ��ȯ�Ұ�
//coAnswer ��ȯ�� : ���� ������ ����� �ε��� ���� ������, ���� 0��° �������� ���� L"0" ��ȯ
export class Lst : public GUI
{
private:
	const int MAX_BTN = 9;
	inline static Lst* ptr = nullptr;
	int lstIndex = -1;
	int lstCursor = -1; //Ű�����Է��� �� ���Ǵ� Ŀ��
	int lstScroll = 0; //��ũ��
	std::wstring lstTitleText; //�޽��� �ڽ� �ϴ��� �ɼ�(���� Ȯ�� ��� ��)
	std::wstring lstText; //�޽��� �ڽ� ��ܿ� ǥ�õǴ� ����
	std::vector<std::wstring> lstOptionVec; //�޽��� �ڽ��� ǥ�õǴ� ����
	std::wstring lstParameter = L""; //�޽����� �� �� ���޵Ǵ� �Ű�����

	SDL_Rect lstBase;    //�� �������� ��ü ������ �׷����� ��ġ
	SDL_Rect lstTitle;
	SDL_Rect lstWindow;
	SDL_Rect lstScrollBox; //����Ʈ ��ũ��

	std::vector<SDL_Rect> lstBtn;
	SDL_Rect lstInputBox;

	tabFlag prevTabType;//�޽��� â�� ���� ���� �� Ÿ��(���� �� ������� ���ư�)
public:
	Lst(std::wstring inputTitle, std::wstring inputText, std::vector<std::wstring> option) : GUI(true)
	{
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		prt(L"Lst ��ü�� �����Ǿ���.\n");
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//�޼��� �ڽ� ������
		changeXY(cameraW / 2, cameraH / 2, true);

		lstTitleText = inputTitle;
		lstText = inputText;
		lstOptionVec = option;


		lstBtn.resize(MAX_BTN);
		for (int i = 0; i < MAX_BTN; i++) lstBtn[i] = { lstWindow.x + 22 ,lstWindow.y + 48 + 32 * i, 240, 29 };


		lstScrollBox = { lstWindow.x + 271, lstWindow.y + 48, 2, 285 };
		lstInputBox = { lstWindow.x + 50, lstWindow.y + 60, 200, 40 };


		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);

		prevTabType = tabType;
		tabType = tabFlag::back;
	}

	~Lst()
	{
		prt(L"Lst : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
		ptr = nullptr;

		exInput = false;
		exInputCursor = 0;
		exInputEditing = false;
		exInputIndex = -1;

		tabType = prevTabType;
	}
	static Lst* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		lstBase = { 0, 0, 280, 393 };
		lstWindow = { 0,36,280,357 };

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
		lstWindow.y = lstBase.y + 36;


		lstBtn.resize(MAX_BTN);
		for (int i = 0; i < MAX_BTN; i++) lstBtn[i] = { lstWindow.x + 22 ,lstWindow.y + 48 + 32 * i, 240, 29 };

		lstScrollBox = { lstWindow.x + 271,lstWindow.y + 48, 2, 285 };
		lstInputBox = { lstWindow.x + 50,lstWindow.y + 60, 200, 40 };

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
			drawWindow(&lstBase, lstTitleText, 0);

			SDL_Rect topWindow = { lstBase.x + 1, lstBase.y + 30, 278, 44 };
			SDL_Rect botWindow = { lstBase.x + 1, lstBase.y + lstBase.h - 17, 278, 16 };
			drawFillRect(topWindow, col::black, 180);
			drawFillRect(botWindow, col::black, 180);

			setFontSize(14);
			drawTextCenter(col2Str(col::white) + lstText, lstWindow.x + lstWindow.w / 2, lstBase.y + 30 + 22);

			//������ ��ư �׸���
			int hoverCursor = -1;

			for (int i = 0; i < MAX_BTN; i++)
			{
				int currentItemIndex = lstScroll + i;

				if (currentItemIndex >= 0 && currentItemIndex < lstOptionVec.size())
				{
					int selectBoxIndex = 0;

					if (checkCursor(&lstBtn[i]))
					{
						hoverCursor = currentItemIndex;
						if (click == true) selectBoxIndex = 2;
						else selectBoxIndex = 1;
					}

					drawSprite(spr::lstSelectBox, selectBoxIndex, lstBtn[i].x, lstBtn[i].y);

					setFontSize(14);
					drawText(col2Str(col::white) + lstOptionVec[currentItemIndex], lstBtn[i].x + 12, lstBtn[i].y + 5);
				}
			}

			setFontSize(10);
			std::wstring hoverText = L"-";
			if (hoverCursor != -1) hoverText = std::to_wstring(hoverCursor + 1);
			drawTextCenter(col2Str(col::white) + hoverText + L"/" + std::to_wstring(lstOptionVec.size()), lstWindow.x + lstWindow.w - 30, lstBase.y + lstBase.h - 17 + 8);

			// ������ ��ũ�� �׸���
			if (lstOptionVec.size() > MAX_BTN)
			{
				drawFillRect(lstScrollBox, { 120, 120, 120 });

				SDL_Rect inScrollBox = lstScrollBox;

				inScrollBox.h = lstScrollBox.h * myMin(1.0, (float)MAX_BTN / (float)lstOptionVec.size());
				if (inScrollBox.h < 5) inScrollBox.h = 5;

				if (!lstOptionVec.empty()) inScrollBox.y = lstScrollBox.y + lstScrollBox.h * ((float)lstScroll / (float)lstOptionVec.size());
				else inScrollBox.y = lstScrollBox.y;

				if (inScrollBox.y < lstScrollBox.y) inScrollBox.y = lstScrollBox.y;
				if (inScrollBox.y + inScrollBox.h > lstScrollBox.y + lstScrollBox.h) inScrollBox.y = lstScrollBox.y + lstScrollBox.h - inScrollBox.h;

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

		for (int i = 0; i < MAX_BTN; i++)
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
			coAnswer = std::to_wstring(-1);
			close(aniFlag::null);
			return;
		}
	}

	void clickMotionGUI(int dx, int dy)	{}

	void clickDownGUI() {}
	void clickRightGUI() {}
	void clickHoldGUI() {}

	void mouseWheel()
	{
		if (checkCursor(&lstBase))
		{
			if (event.wheel.y > 0 && lstScroll > 0) lstScroll -= 1;
			else if (event.wheel.y < 0 && lstScroll + MAX_BTN < lstOptionVec.size()) lstScroll += 1;
		}
	}

	void gamepadBtnDown() {}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}

	void step()
	{
		if (lstOptionVec.empty() || lstOptionVec.size() <= MAX_BTN) lstScroll = 0;
		else 
		{
			if (lstScroll < 0) lstScroll = 0;
			int maxScroll = (int)lstOptionVec.size() - MAX_BTN;
			if (lstScroll > maxScroll) lstScroll = maxScroll;
		}
	}
};