module;
#include <SDL3/SDL.h>

export module Cook;

import std;
import util;
import GUI;
import textureVar;
import constVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import log;

export class Cook : public GUI
{
private:
	inline static Cook* ptr = nullptr;
	SDL_Rect cookBase;
	SDL_Rect heatSrcBtn;
	SDL_Rect recipeBtn;
	SDL_Rect cookwareBtn;
	std::array<SDL_Rect, 6> ingredientBtn;
	SDL_Rect cookBtn;
	int cookCursor = -1;
	int cookScroll = 0;
public:
	Cook() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		int arrowEndX, arrowEndY, targetX, targetY;
		arrowEndX = cameraW / 2 - 8 * zoomScale + 16 * 0 * zoomScale;
		arrowEndY = cameraH / 2 + 16 * 0 * zoomScale;
		targetX = arrowEndX - 429;
		targetY = arrowEndY - 266;

		changeXY(targetX, targetY, false);

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Cook()
	{
		ptr = nullptr;
	}
	static Cook* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		cookBase = { 0,0,404,520 };


		if (center == false)
		{
			cookBase.x += inputX;
			cookBase.y += inputY;
		}
		else
		{
			cookBase.x += inputX - cookBase.w / 2;
			cookBase.y += inputY - cookBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - cookBase.w / 2;
			y = inputY - cookBase.h / 2;
		}

		heatSrcBtn = { cookBase.x + 15, cookBase.y + 50, 64, 64 };
		recipeBtn = { cookBase.x + cookBase.w - 79, cookBase.y + 50, 64, 64 };
		cookwareBtn = { cookBase.x + 132, cookBase.y + 212, 248, 40 };
		for (int i = 0; i < 6; i++)
		{
			int pivotX = cookBase.x + 22;
			int pivotY = cookBase.y + 296;
			ingredientBtn[i] = { pivotX + 190 * (i % 2), pivotY + 50 * (i / 2), 170, 40 };
		}
		cookBtn = { cookBase.x + 170, cookBase.y + 465, 140, 40 };
		cookBtn.x += cookBtn.w / 2;
	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&cookBase, sysStr[352]/*Cooking*/, 0);
			setFlip(SDL_FLIP_HORIZONTAL);
			drawSprite(spr::newWindowArrow, 0, cookBase.x + cookBase.w - 4, cookBase.y + 234);
			setFlip(SDL_FLIP_NONE);

			if (checkCursor(&heatSrcBtn)) drawStadium(heatSrcBtn, click ? lowCol::deepBlue : lowCol::blue, 255, 3);
			else drawStadium(heatSrcBtn, col::black, 255, 3);
			setZoom(3.0);
			drawSpriteCenter(spr::itemset, 152, heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 - 5);
			setZoom(1.0);
			setFontSize(16);
			drawTextCenter(L"Heat", heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 30 - 5);

			if (checkCursor(&recipeBtn)) drawStadium(recipeBtn, click ? lowCol::deepBlue : lowCol::blue, 255, 3);
			else drawStadium(recipeBtn, col::black, 255, 3);
			setZoom(1.0);
			drawSpriteCenter(spr::icon48, 190, recipeBtn.x + recipeBtn.w / 2, recipeBtn.y + recipeBtn.h / 2 - 5);
			setZoom(1.0);
			setFontSize(16);
			drawTextCenter(L"Recipe", recipeBtn.x + recipeBtn.w / 2, recipeBtn.y + recipeBtn.h / 2 + 30 - 5);

			//확대된 요리 그림
			setZoom(10.0);
			drawSpriteCenter(spr::itemset, 159, cookBase.x + cookBase.w / 2, cookBase.y + cookBase.h / 2 - 120);
			setZoom(1.0);

			//쿡웨어 버튼(프라이팬 혹은 냄비 혹은 뚝배기)
			setFontSize(18);
			drawTextCenter(col2Str(col::lightGray) + L"Cookware", cookwareBtn.x - 52, cookwareBtn.y + cookwareBtn.h/2 - 12);
			if (checkCursor(&cookwareBtn)) { drawFillRect(cookwareBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(cookwareBtn, col::lightGray); }
			else { drawFillRect(cookwareBtn, col::black); drawRect(cookwareBtn, col::gray); }

			setFontSize(16);
			drawTextCenter(col2Str(col::lightGray) + L"Ingredients", cookBase.x + cookBase.w / 2, cookBase.y + 263);

			{
				int lineY = cookBase.y + 263 + 12;
				int startX = cookBase.x + cookBase.w / 2 - 50;
				int lineLen = 130;
				for (int i = 0; i < lineLen; i++)
				{
					Uint8 alpha = 255 - (255 * i / lineLen);
					drawPoint(startX - i, lineY, col::gray, alpha);
				}
			}

			{
				int lineY = cookBase.y + 263 + 12;
				int startX = cookBase.x + cookBase.w / 2 + 50;
				int lineLen = 130;
				for (int i = 0; i < lineLen; i++)
				{
					Uint8 alpha = 255 - (255 * i / lineLen);
					drawPoint(startX + i, lineY, col::gray, alpha);
				}
			}



			for (int i = 0; i < 6; i++)
			{
				
				if (i == 0)//이미 해당 슬롯에 재료가 있을 경우
				{
					drawFillRect(ingredientBtn[i], col::black);
					drawRect(ingredientBtn[i], col::gray);

					setZoom(2.0);
					drawSpriteCenter(spr::itemset, 606, ingredientBtn[i].x + 20, ingredientBtn[i].y + ingredientBtn[i].h / 2);
					setZoom(1.0);
					
					drawText(L"Cabbage", ingredientBtn[i].x + 46, ingredientBtn[i].y + ingredientBtn[i].h / 2 - 11);
				}
				else if (i == 1)//재료를 새롭게 추가할 수 있는 버튼, 재료가 있는 칸 바로 다음에 딱 1칸만 그려지는 버튼
				{
					drawFillRect(ingredientBtn[i], col::black);
					drawRect(ingredientBtn[i], col::gray);

					setFontSize(28);
					drawTextCenter(L"+", ingredientBtn[i].x + ingredientBtn[i].w / 2, ingredientBtn[i].y + ingredientBtn[i].h / 2 - 4);
				}
				else//빈 슬롯이며 눌러도 아무 기능 없음
				{
					drawFillRect(ingredientBtn[i], col::black,80);
					drawRect(ingredientBtn[i], col::gray,80);
				}
			}

			{
				int lineY = cookBase.y + 470 - 18;
				int startX = cookBase.x + 22;
				int lineLen = 360;
				int half = lineLen / 2;
				for (int i = 0; i < lineLen; i++)
				{
					int distFromCenter = std::abs(i - half);
					Uint8 alpha = 255 - (255 * distFromCenter / half);
					drawPoint(startX + i, lineY, col::gray, alpha);
				}
			}

			if (checkCursor(&cookBtn)) { drawFillRect(cookBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(cookBtn, col::lightGray); }
			else { drawFillRect(cookBtn, col::black); drawRect(cookBtn, col::gray); }
			setFontSize(22);
			drawTextCenter(L"Cook", cookBtn.x + cookBtn.w / 2, cookBtn.y + cookBtn.h / 2);
		}
		else
		{
			SDL_Rect vRect = cookBase;
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
	void onClickRecipeBtn()
	{
		updateLog(L"[Cook] Recipe button clicked.");
	}
	void onClickHeatBtn()
	{
		updateLog(L"[Cook] Heat source button clicked.");
	}
	void onClickCookBtn()
	{
		updateLog(L"[Cook] Cook button clicked.");
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else if (checkCursor(&recipeBtn))
		{
			onClickRecipeBtn();
		}
		else if (checkCursor(&heatSrcBtn))
		{
			onClickHeatBtn();
		}
		else if (checkCursor(&cookBtn))
		{
			onClickCookBtn();
		}
	}

	void step()
	{
		tabType = tabFlag::back;
	}
};