#include <SDL.h>

export module God;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;

export class God : public GUI
{
private:
	inline static God* ptr = nullptr;
	SDL_Rect godBase;
	godFlag thisGod = godFlag::none;
	int delayR2 = 0;
	SDL_Rect convertBtn;
public:
	God(godFlag inputGod) : GUI(false)
	{
		thisGod = inputGod;
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//�޼��� �ڽ� ������
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~God()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static God* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		godBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			godBase.x += inputX;
			godBase.y += inputY;
		}
		else
		{
			godBase.x += inputX - godBase.w / 2;
			godBase.y += inputY - godBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - godBase.w / 2;
			y = inputY - godBase.h / 2;
		}

		convertBtn = { godBase.x + 235, godBase.y + 277, 180, 52 };
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&godBase, sysStr[149], 44);

			if (thisGod == godFlag::buddha)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::propset, 1059, godBase.x + godBase.w / 2, godBase.y + 103);
				setZoom(1.0);

				setFontSize(14);
				drawTextCenter(col2Str(lowCol::yellow) + L"��Ÿ�� �˴ٸ�Ÿ (***...)", godBase.x + godBase.w / 2, godBase.y + 144);

				setFontSize(12);
				std::wstring godDescript = L"���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�. ";
				drawTextWidth(col2Str(col::lightGray) + godDescript, godBase.x + 12, godBase.y + 163, false, 631, 14);
			}
			else if (thisGod == godFlag::jesus)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::propset, 1061, godBase.x + godBase.w / 2, godBase.y + 103);
				setZoom(1.0);

				setFontSize(14);
				drawTextCenter(col2Str(lowCol::yellow) + L"���ڷ��� ���� (***...)", godBase.x + godBase.w / 2, godBase.y + 144);

				setFontSize(12);
				std::wstring godDescript = L"���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�. ";
				drawTextWidth(col2Str(col::lightGray) + godDescript, godBase.x + 12, godBase.y + 163, false, 631, 14);
			}
			else if (thisGod == godFlag::amaterasu)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::propset, 1057, godBase.x + godBase.w / 2, godBase.y + 103);
				setZoom(1.0);

				setFontSize(14);
				drawTextCenter(col2Str(lowCol::yellow) + L"�¾�� �Ƹ��׶� (***...)", godBase.x + godBase.w / 2, godBase.y + 144);

				setFontSize(12);
				std::wstring godDescript = L"���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�. ";
				drawTextWidth(col2Str(col::lightGray) + godDescript, godBase.x + 12, godBase.y + 163, false, 631, 14);
			}
			else if (thisGod == godFlag::yudi)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::propset, 1056, godBase.x + godBase.w / 2, godBase.y + 103);
				setZoom(1.0);

				setFontSize(14);
				drawTextCenter(col2Str(lowCol::yellow) + L"��Ȳ���� (***...)", godBase.x + godBase.w / 2, godBase.y + 144);

				setFontSize(12);
				std::wstring godDescript = L"���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�. ";
				drawTextWidth(col2Str(col::lightGray) + godDescript, godBase.x + 12, godBase.y + 163, false, 631, 14);
			}
			else if (thisGod == godFlag::ra)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::propset, 1064, godBase.x + godBase.w / 2, godBase.y + 103);
				setZoom(1.0);

				setFontSize(14);
				drawTextCenter(col2Str(lowCol::yellow) + L"�¾�� �� (***...)", godBase.x + godBase.w / 2, godBase.y + 144);

				setFontSize(12);
				std::wstring godDescript = L"���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�. ";
				drawTextWidth(col2Str(col::lightGray) + godDescript, godBase.x + 12, godBase.y + 163, false, 631, 14);
			}
			else
			{
				setZoom(3.0);
				drawSpriteCenter(spr::propset, 1063, godBase.x + godBase.w / 2, godBase.y + 103);
				setZoom(1.0);

				setFontSize(14);
				drawTextCenter(col2Str(lowCol::yellow) + L"�� �� ���� �� (******)", godBase.x + godBase.w/2, godBase.y + 144);

				setFontSize(12);
				std::wstring godDescript = L"���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�.���⿡ �Ű� ������ ���� ������ ���ϴ�. ";
				drawTextWidth(col2Str(col::lightGray) + godDescript, godBase.x + 12, godBase.y + 163, false, 631, 14);
			}

			if (playerGod == thisGod)
			{
				auto drawPower = [](int pivotX, int pivotY)
					{

						drawEdgeWindow(pivotX, pivotY, 96, 122, 16);
						setFontSize(14);
						drawText(col2Str(lowCol::yellow) + L"��1", pivotX + 4, pivotY + 4);

						SDL_Rect proficRect = { pivotX + 31, pivotY + 5, 34,34 };
						drawRect(proficRect, col::gray);

						setFontSize(14);
						drawTextCenter(col2Str(col::white) + L"�Ǵ� �̸�", pivotX + 47, pivotY + 48);

						setFontSize(9);
						drawTextWidth(col2Str(col::lightGray) + L"���⿡ �Ǵɿ� ���� ������ ���ϴ�. �Ǵɿ� ���� ������ ���ϴ�.", pivotX + 5, pivotY + 60, false, 90, 11);

						drawTextCenter(col2Str(lowCol::red) + L"�žӵ� -3", pivotX + 47, pivotY + 111);
					};
				drawPower(godBase.x + 10 + 107 * 0, godBase.y + 242);
				drawPower(godBase.x + 10 + 107 * 1, godBase.y + 242);
				drawPower(godBase.x + 10 + 107 * 2, godBase.y + 242);
				drawPower(godBase.x + 10 + 107 * 3, godBase.y + 242);
				drawPower(godBase.x + 10 + 107 * 4, godBase.y + 242);
				drawPower(godBase.x + 10 + 107 * 5, godBase.y + 242);
			}
			else
			{
				setFontSize(18);
				SDL_Color btnCol = col::black;
				if (checkCursor(&convertBtn))
				{
					if (click) btnCol = lowCol::deepBlue;
					else btnCol = lowCol::blue;
				}

				drawFillRect(convertBtn,btnCol);
				drawEdgeWindow(convertBtn.x, convertBtn.y, convertBtn.w, convertBtn.h, 16);
				drawTextCenter(col2Str(col::yellow) + L"�����ϱ�", convertBtn.x + 90, convertBtn.y + 26);
			}
		}
		else
		{
			SDL_Rect vRect = godBase;
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
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else if (checkCursor(&convertBtn))
		{
			if (thisGod != playerGod) playerGod = thisGod;
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
	void step() 
	{
		std::wprintf(L"delayR2�� ���� %d�̴�.\n", delayR2);
		if (SDL_NumJoysticks() > 0)
		{
			if (GUI::getLastGUI() == this)
			{
				if (delayR2 <= 0 && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 10000)
				{
					prt(L"���� ����Ǿ���.\n");
					close(aniFlag::winUnfoldClose);
					delayR2 = 20;
				}
				else delayR2--;
			}
		}
	}
};