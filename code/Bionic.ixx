#include <SDL.h>

export module Bionic;

import std;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import util;


export class Bionic : public GUI
{
private:
	inline static Bionic* ptr = nullptr;
	SDL_Rect bionicBase;
	SDL_Rect bionicDataRect;

	int bionicCursor = -1;
public:
	Bionic() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);

		int bionicCursor = -1, bionicScroll = 0;
	}
	~Bionic()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Bionic* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		bionicBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			bionicBase.x += inputX;
			bionicBase.y += inputY;
		}
		else
		{
			bionicBase.x += inputX - bionicBase.w / 2;
			bionicBase.y += inputY - bionicBase.h / 2;
		}

		bionicDataRect = { bionicBase.x + 16,bionicBase.y + 52,200,300 };

		if (center == false) 
		{ 
			x = inputX;
			y = inputY;
		}
		else 
		{
			x = inputX - bionicBase.w / 2;
			y = inputY - bionicBase.h / 2;
		}
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&bionicBase, sysStr[6], 8);


			SDL_SetRenderDrawColor(renderer, col::gray.r, col::gray.g, col::gray.b, 0xff);

			SDL_Rect whiteRectData = { bionicBase.x, bionicBase.y + 30 - 1 , 220 , bionicBase.h - 30 + 1 };
			drawRect(whiteRectData, col::gray);

			SDL_Rect whiteRectActive = { whiteRectData.x + whiteRectData.w - 1, bionicBase.y + 30 - 1,241,bionicBase.h - 30 + 1 };
			drawRect(whiteRectActive, col::gray);

			SDL_Rect whiteRectPassive = { whiteRectActive.x + whiteRectActive.w - 1, bionicBase.y + 30 - 1,191,bionicBase.h - 30 + 1 };
			drawRect(whiteRectPassive, col::gray);

			setFontSize(10);
			drawText(L"#FFFFFF바이오닉 인터페이스 : (INTERFACE_NAME)", bionicBase.x + 20, bionicBase.y + 36 + 14);

			drawTextCenter(L"#FFFFFF전력 소모량", bionicBase.x + 169, bionicBase.y + 36 + 54);
			drawTextCenter(col2Str(lowCol::red) + L"132.5 KJ/turn", bionicBase.x + 169, bionicBase.y + 36 + 54 + 14);

			drawTextCenter(L"#FFFFFF전력 생산량", bionicBase.x + 169, bionicBase.y + 36 + 54 + 34 * 1);
			drawTextCenter(col2Str(lowCol::yellow) + L"132.5 KJ/turn", bionicBase.x + 169, bionicBase.y + 36 + 54 + 14 + 34 * 1);

			drawTextCenter(L"#FFFFFF총 전력 증감", bionicBase.x + 169, bionicBase.y + 36 + 54 + 34 * 2);
			drawTextCenter(col2Str(lowCol::green) + L"+132.5 KJ/turn", bionicBase.x + 169, bionicBase.y + 36 + 54 + 14 + 34 * 2);

			drawTextCenter(L"#FFFFFF[슬롯]", whiteRectData.x + whiteRectData.w / 2, whiteRectData.y + 220 - 56);

			int skeletonSprIndex;
			if (timer::timer600 % 60 < 10) { skeletonSprIndex = 0; }
			else if (timer::timer600 % 60 < 20) { skeletonSprIndex = 1; }
			else if (timer::timer600 % 60 < 30) { skeletonSprIndex = 2; }
			else if (timer::timer600 % 60 < 40) { skeletonSprIndex = 3; }
			else if (timer::timer600 % 60 < 50) { skeletonSprIndex = 4; }
			else { skeletonSprIndex = 5; }
			drawSpriteCenter(spr::bionicSkeleton, skeletonSprIndex, whiteRectData.x + whiteRectData.w / 2, whiteRectData.y + 220);

			setFontSize(8);
			drawSprite(spr::bionicSlotGauge, 0, whiteRectData.x + whiteRectData.w / 2 + 7, whiteRectData.y + 220 - 33);
			drawText(L"#FFFFFF상체 4/15", whiteRectData.x + whiteRectData.w / 2 + 7 + 12, whiteRectData.y + 220 - 33 - 12);

			drawSprite(spr::bionicSlotGauge, 7, whiteRectData.x + whiteRectData.w / 2 + 15, whiteRectData.y + 220 - 10);
			drawText(L"#FFFFFF왼팔 8/15", whiteRectData.x + whiteRectData.w / 2 + 15 + 12, whiteRectData.y + 220 - 10 - 12);

			drawSprite(spr::bionicSlotGauge, 2, whiteRectData.x + whiteRectData.w / 2 + 7, whiteRectData.y + 220 + 18);
			drawText(L"#FFFFFF왼다리 10/15", whiteRectData.x + whiteRectData.w / 2 + 7 + 12, whiteRectData.y + 220 + 18 - 12);

			setFlip(SDL_FLIP_HORIZONTAL);

			drawSprite(spr::bionicSlotGauge, 0, whiteRectData.x + whiteRectData.w / 2 - 68, whiteRectData.y + 220 - 39);
			drawText(L"#FFFFFF머리 4/15", whiteRectData.x + whiteRectData.w / 2 - 68, whiteRectData.y + 220 - 39 - 12);

			drawSprite(spr::bionicSlotGauge, 3, whiteRectData.x + whiteRectData.w / 2 - 78, whiteRectData.y + 220 - 13);
			drawText(L"#FFFFFF오른팔 4/15", whiteRectData.x + whiteRectData.w / 2 - 78, whiteRectData.y + 220 - 13 - 12);

			drawSprite(spr::bionicSlotGauge, 11, whiteRectData.x + whiteRectData.w / 2 - 72, whiteRectData.y + 220 + 13);
			drawText(L"#FFFFFF오른다리 12/15", whiteRectData.x + whiteRectData.w / 2 - 72, whiteRectData.y + 220 + 13 - 12);

			setFlip(SDL_FLIP_NONE);

			setFontSize(10);
			drawText(L"#FFFFFF* 인터페이스 부가효과 없음", bionicBase.x + 20, bionicBase.y + 36 + 14 + 260);

			//drawTextCenter(L"#FFFFFF[액티브]", whiteRectActive.x + whiteRectActive.w / 2, whiteRectActive.y + 15);
			//drawStadium(whiteRectActive.x + 6, whiteRectActive.y + 25, whiteRectActive.w - 12, 16, col::black, 200, 5);
			//setFontSize(10);
			//drawTextCenter(L"#FFFFFF상태", whiteRectActive.x + 6 + 20, whiteRectActive.y + 25 + 1 + 7);
			//drawTextCenter(L"#FFFFFF이름", whiteRectActive.x + 6 + 20 + 72, whiteRectActive.y + 25 + 1 + 7);
			//setFontSize(8);
			//drawTextCenter(L"#FFFFFFKJ/act", whiteRectActive.x + 6 + 20 + 146, whiteRectActive.y + 25 + 1 + 7);
			//drawTextCenter(L"#FFFFFFKJ/turn", whiteRectActive.x + 6 + 20 + 186, whiteRectActive.y + 25 + 1 + 7);
			//{
			//	drawStadium(whiteRectActive.x + 6, whiteRectActive.y + 25 + 19 * 1, whiteRectActive.w - 12, 20, col::black, 200, 5);
			//	setFontSize(10);
			//	drawTextCenter(col2Str(lowCol::green) + L"ON", whiteRectActive.x + 6 + 20, whiteRectActive.y + 25 + 1 + 9 + 19 * 1);
			//	drawTextCenter(col2Str(lowCol::green) + L"대기증류장치", whiteRectActive.x + 6 + 20 + 72, whiteRectActive.y + 25 + 1 + 9 + 19 * 1);
			//	setFontSize(8);
			//	drawTextCenter(col2Str(lowCol::green) + L"-12.5", whiteRectActive.x + 6 + 20 + 146, whiteRectActive.y + 25 + 1 + 9 + 19 * 1);
			//	drawTextCenter(col2Str(lowCol::green) + L"-1.2", whiteRectActive.x + 6 + 20 + 186, whiteRectActive.y + 25 + 1 + 9 + 19 * 1);
			//}

			drawTextCenter(L"#FFFFFF[액티브]", whiteRectActive.x + whiteRectActive.w / 2, whiteRectActive.y + 15);
			
			for(int i=0; i<13; i++) //13개까지 그릴 수 있음
			{
				SDL_Rect pivotRect = { whiteRectActive.x + 6, whiteRectActive.y + 25 + 24*i, whiteRectActive.w - 14, 22 };

				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				if (checkCursor(&pivotRect))
				{
					if (click == false) { btnColor = lowCol::blue; }
					else { btnColor = lowCol::deepBlue; }
				}
				else if (bionicCursor == i) btnColor = lowCol::blue;
				//else if (bionicCursor == i)
				//{
				//	btnColor = lowCol::blue;
				//}

				drawFillRect(pivotRect.x, pivotRect.y, pivotRect.w, pivotRect.h, btnColor);
				drawRect(pivotRect.x + 4, pivotRect.y + 2, 18, 18, col::white); //아이콘 테두리
				drawSprite(spr::mutationIcon, 17, pivotRect.x + 5, pivotRect.y + 3);
				setFontSize(10);
				drawText(col2Str(lowCol::red) + L"대기증류장치", pivotRect.x + 29, pivotRect.y + 4);
				setFontSize(8);
				drawText(L"#FFFF00TOGGLE#FFFFFF : 360 kcal/act", pivotRect.x + pivotRect.w - 90, pivotRect.y + 11);
			}

			// 액티브 바이오닉 스크롤 그리기
			{
				SDL_Rect activeScrollBox = { whiteRectActive.x + whiteRectActive.w - 5,whiteRectActive.y + 25 + 19 * 1, 2, 270 };
				drawFillRect(activeScrollBox, { 120,120,120 });
			}

			setFontSize(10);
			drawTextCenter(L"#FFFFFF[패시브]", whiteRectPassive.x + whiteRectPassive.w / 2, whiteRectPassive.y + 15);

			
			{
				SDL_Rect pivotRect = { whiteRectPassive.x + 6, whiteRectPassive.y + 25, whiteRectPassive.w - 14, 22 };

				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				if (checkCursor(&pivotRect))
				{
					if (click == false) { btnColor = lowCol::blue; }
					else { btnColor = lowCol::deepBlue; }
				}
				else if (bionicCursor == 1) btnColor = lowCol::blue;

				drawFillRect(pivotRect.x, pivotRect.y, pivotRect.w, pivotRect.h, btnColor);
				drawRect(pivotRect.x + 4, pivotRect.y + 2, 18, 18, col::white); //아이콘 테두리
				drawSprite(spr::mutationIcon, 17, pivotRect.x + 5, pivotRect.y + 3);
				setFontSize(10);
				drawText(col2Str(lowCol::skyBlue) + L"망원렌즈", pivotRect.x + 29, pivotRect.y + 4);
				setFontSize(8);
				drawText(L"#FFFF00TOGGLE#FFFFFF : 360 kcal/act", pivotRect.x + pivotRect.w - 90, pivotRect.y + 11);
			}


			//drawStadium(whiteRectPassive.x + 6, whiteRectPassive.y + 25, whiteRectPassive.w - 12, 16, col::black, 200, 5);
			//setFontSize(10);
			//drawTextCenter(L"#FFFFFF이름", whiteRectPassive.x + whiteRectPassive.w / 2, whiteRectPassive.y + 25 + 1 + 7);

			//drawStadium(whiteRectPassive.x + 6, whiteRectPassive.y + 25 + 19 * 1, whiteRectPassive.w - 12, 16, col::black, 200, 5);
			//setFontSize(10);
			//drawTextCenter(col2Str(lowCol::skyBlue) + L"망원렌즈", whiteRectPassive.x + whiteRectPassive.w / 2, whiteRectPassive.y + 25 + 1 + 7 + 19 * 1);

			setZoom(1.5);
			drawSpriteCenter(spr::bionicGauge, 0, bionicBase.x + 70, bionicBase.y + 36 + 94);
			setZoom(1.0);
			setFontSize(20);
			drawTextCenter(L"#4A823E100%", bionicBase.x + 70, bionicBase.y + 36 + 94 - 12);
			setFontSize(10);
			drawTextCenter(L"#FFFFFF9800.5 KJ", bionicBase.x + 70, bionicBase.y + 36 + 94 + 6);
			
			drawLine(bionicBase.x + 70 - 24, bionicBase.y + 36 + 94 + 13, bionicBase.x + 70 + 24, bionicBase.y + 36 + 94 + 13, col::white);
			
			drawTextCenter(L"#FFFFFF9800.5 KJ", bionicBase.x + 70, bionicBase.y + 36 + 94 + 20);

			// 패시브 바이오닉 스크롤 그리기
			{
				SDL_Rect passiveScrollBox = { whiteRectPassive.x + whiteRectPassive.w - 5,whiteRectPassive.y + 25 + 19 * 1, 2, 270 };
				drawFillRect(passiveScrollBox, { 120,120,120 });
			}

		}
		else
		{
			SDL_Rect vRect = bionicBase;
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