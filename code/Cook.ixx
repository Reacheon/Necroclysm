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

export class Cook : public GUI
{
private:
	inline static Cook* ptr = nullptr;
	SDL_Rect cookBase;
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

			SDL_Rect heatSrcBtn = { cookBase.x + 15, cookBase.y + 50, 64, 64 };
			drawStadium(heatSrcBtn, col::black, 255, 3);
			setZoom(3.0);
			drawSpriteCenter(spr::itemset, 152, heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 - 5);
			setZoom(1.0);
			setFontSize(16);
			drawTextCenter(L"Heat", heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 30 - 5);

			SDL_Rect recipeBtn = { cookBase.x + cookBase.w - 79, cookBase.y + 50, 64, 64 };
			drawStadium(recipeBtn, col::black, 255, 3);
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
			SDL_Rect containerBtn = { cookBase.x + 132, cookBase.y + 212, 248,40 };
			setFontSize(18);
			drawTextCenter(col2Str(col::lightGray) + L"Cookware", containerBtn.x - 52, containerBtn.y + containerBtn.h/2 - 12);
			drawStadium(containerBtn, col::black, 255, 3);

			drawRect(containerBtn, col::gray);

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



			std::array<SDL_Rect, 6> ingredientBtn;
			for (int i = 0; i < 6; i++)
			{
				int pivotX = cookBase.x + 22;
				int pivotY = cookBase.y + 296;
				ingredientBtn[i] = { pivotX + 190 * (i % 2) ,pivotY + 50 * (i / 2), 170, 40 };
				drawStadium(ingredientBtn[i], col::black, 255, 3);
				drawRect(ingredientBtn[i], col::gray);
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

			SDL_Rect cookBtn = { cookBase.x + 170, cookBase.y + 465, 140, 40 };
			cookBtn.x += cookBtn.w / 2;
			drawStadium(cookBtn, col::black, 255, 3);
			drawRect(cookBtn, col::gray);
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
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else
		{
		}
	}

	void step()
	{
		tabType = tabFlag::back;
	}
};