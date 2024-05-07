#include <SDL.h>

export module Lst;

import std;
import util;
import GUI;
import globalVar;
import drawPrimitive;
import drawText;
import checkCursor;
import drawSprite;
import ItemPocket;
import Player;
import textureVar;
import drawWindow;

//Lst�� Msg�� �ٸ��� coAnswer�� ���ڿ��� �ƴ϶� ������ �Է����� �ε����� ��ȯ�Ұ�
//coAnswer ��ȯ�� : ���� ������ ����� �ε��� ���� ������, ���� 0��° �������� ���� L"0" ��ȯ
export class Lst : public GUI
{
private:
	const int btnSize = 9;
	inline static Lst* ptr = nullptr;
	int lstIndex = -1;
	int lstCursor = -1; //Ű�����Է��� �� ���Ǵ� Ŀ��
	int lstScroll = 0; //��ũ��
	std::wstring lstTitleText; //�޽��� �ڽ� �ϴ��� �ɼ�(���� Ȯ�� ��� ��)
	std::wstring lstText; //�޽��� �ڽ� ��ܿ� ǥ�õǴ� ����
	std::vector<std::wstring> lstOptionVec; //�޽��� �ڽ��� ǥ�õǴ� ����
	std::wstring lstParameter = L""; //�޽����� �� �� ���޵Ǵ� �Ű�����

	SDL_Rect lstBase;//�� �������� ��ü ������ �׷����� ��ġ
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

		if (inputType == input::keyboard) { lstCursor = 0; }

		deactInput();
		deactDraw();
		setAniType(aniFlag::winUnfoldOpen);
		aniUSet.insert(this);
		turnCycle = turn::playerAnime;

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
		lstTitle = { 140 - 65,0,130,30 };
		lstWindow = { 0,36,280,357 };

		if (center == false)
		{
			lstBase.x += inputX;
			lstBase.y += inputY;
			lstTitle.x += inputX;
			lstTitle.y += inputY;
			lstWindow.x += inputX;
			lstWindow.y += inputY;
		}
		else
		{
			lstBase.x += inputX - lstBase.w / 2;
			lstBase.y += inputY - lstBase.h / 2;
			lstTitle.x += inputX - lstBase.w / 2;
			lstTitle.y += inputY - lstBase.h / 2;
			lstWindow.x += inputX - lstBase.w / 2;
			lstWindow.y += inputY - lstBase.h / 2;
		}

		lstBtn.clear();
		for (int i = 0; i < btnSize; i++)
		{
			SDL_Rect tempLstBtn = { lstWindow.x + 22 ,lstWindow.y + 48 + 32 * i, 240, 29 };
			lstBtn.push_back(tempLstBtn);
		}

		lstInputBox = { lstWindow.x + 50,lstWindow.y + 60, 200, 40 };

		lstScrollBox = { lstWindow.x + 271,lstWindow.y + 48, 2, 285 };

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
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&lstBase, lstTitleText, 0);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
			SDL_Rect topWindow = { lstBase.x + 1, lstBase.y + 30, 278, 44 };
			SDL_Rect botWindow = { lstBase.x + 1, lstBase.y + lstBase.h - 17, 278, 16 };
			SDL_RenderFillRect(renderer, &topWindow);
			SDL_RenderFillRect(renderer, &botWindow);

			setFontSize(10);
			drawTextCenter(L"#FFFFFF172/282", lstWindow.x + lstWindow.w - 30, lstWindow.y + lstWindow.h - 9);

			SDL_SetRenderDrawColor(renderer, lowCol::white.r, lowCol::white.g, lowCol::white.b, 0xff);
			setFontSize(14);
			int hCorrection = 0;
			drawTextCenter(lstText, lstWindow.x + lstWindow.w / 2, lstWindow.y + 14);

			//������ ��ư �׸���	
			for (int i = 0; i < myMin(btnSize, (int)lstOptionVec.size() - lstScroll); i++)
			{
				int selectBoxIndex = 0;
				if (checkCursor(&lstBtn[i + lstScroll]))
				{
					if (click == true) { selectBoxIndex = 2; }
					else { selectBoxIndex = 1; }
				}
				drawSprite(spr::lstSelectBox, selectBoxIndex, lstBtn[i + lstScroll].x, lstBtn[i + lstScroll].y);
				setFontSize(14);
				drawText(col2Str(col::white) + lstOptionVec[i + lstScroll], lstBtn[i + lstScroll].x + 12, lstBtn[i + lstScroll].y + 5);
			}

			// ������ ��ũ�� �׸���
			SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
			SDL_RenderFillRect(renderer, &lstScrollBox);
		}
		else
		{
			SDL_Rect vRect = lstBase;
			int a = 6;
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
			drawStadium(vRect.x, vRect.y, vRect.w, vRect.h, { 0,0,0 }, 230, 5);
		}
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		for (int i = 0; i < lstBtn.size(); i++)
		{
			if (checkCursor(&lstBtn[i]))
			{
				coAnswer = std::to_wstring(lstScroll + i);
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
	void clickMotionGUI(int dx, int dy)
	{
		//if (checkCursor(&lootArea))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // ���ӻ��, �۾������� ��ũ�� �ӵ��� ������
				//lootScroll = initLootScroll + dy / scrollAccelConst;
				if (abs(dy / scrollAccelConst) >= 1)
				{
					deactClickUp = true;
					cursorMotionLock = true;
				}
			}
		}
	}
	void clickDownGUI() { }
	void step() { }
};

