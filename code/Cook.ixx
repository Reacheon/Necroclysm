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
import wrapFunc;
import ItemStack;
import ItemPocket;
import ItemData;
import Prop;

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

	//드롭다운 메뉴 상태
	static constexpr int MAX_DD_VISIBLE = 6;
	static constexpr int DD_BLOCK_H = 36;
	bool ddOpen = false;
	int ddTarget = -1;          // -1:없음, 0:cookware, 1~:ingredient 슬롯
	float ddRatio = 0.0f;       // 애니메이션 비율 (0.0~1.0)
	int ddScroll = 0;
	SDL_Rect ddRect = {};

	struct DdItem {
		int itemCode;
		int totalCount;
	};
	std::vector<DdItem> ddItems;

	//선택된 재료 (가상넘버)
	int cookwareCode = -1;
	std::array<int, 6> ingredientCode;
	int ingredientCount = 0;
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
		ingredientCode.fill(-1);

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

			if (cookwareCode != -1)
			{
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, itemDex[cookwareCode].itemSprIndex, cookwareBtn.x + 20, cookwareBtn.y + cookwareBtn.h / 2);
				setZoom(1.0);
				setFontSize(16);
				drawText(itemDex[cookwareCode].name, cookwareBtn.x + 46, cookwareBtn.y + cookwareBtn.h / 2 - 11);
			}

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
				if (i < ingredientCount)//이미 해당 슬롯에 재료가 있을 경우
				{
					if (checkCursor(&ingredientBtn[i])) { drawFillRect(ingredientBtn[i], click ? lowCol::deepBlue : lowCol::blue); drawRect(ingredientBtn[i], col::lightGray); }
					else { drawFillRect(ingredientBtn[i], col::black); drawRect(ingredientBtn[i], col::gray); }

					setZoom(2.0);
					drawSpriteCenter(spr::itemset, itemDex[ingredientCode[i]].itemSprIndex, ingredientBtn[i].x + 20, ingredientBtn[i].y + ingredientBtn[i].h / 2);
					setZoom(1.0);

					setFontSize(16);
					drawText(itemDex[ingredientCode[i]].name, ingredientBtn[i].x + 46, ingredientBtn[i].y + ingredientBtn[i].h / 2 - 11);
				}
				else if (i == ingredientCount && ingredientCount < 6)//재료를 새롭게 추가할 수 있는 (+) 버튼
				{
					if (checkCursor(&ingredientBtn[i])) { drawFillRect(ingredientBtn[i], click ? lowCol::deepBlue : lowCol::blue); drawRect(ingredientBtn[i], col::lightGray); }
					else { drawFillRect(ingredientBtn[i], col::black); drawRect(ingredientBtn[i], col::gray); }

					setFontSize(28);
					drawTextCenter(L"+", ingredientBtn[i].x + ingredientBtn[i].w / 2, ingredientBtn[i].y + ingredientBtn[i].h / 2 - 4);
				}
				else//빈 슬롯이며 눌러도 아무 기능 없음
				{
					drawFillRect(ingredientBtn[i], col::black, 80);
					drawRect(ingredientBtn[i], col::gray, 80);
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

		//드롭다운 메뉴 (모든 UI 위에 그림)
		drawDropdown();
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

		//드롭다운이 열려있을 때: 최우선 처리
		if (ddOpen)
		{
			if (ddRatio >= 1.0f)
			{
				int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
				for (int i = 0; i < visibleCount; i++)
				{
					int idx = i + ddScroll;
					if (idx >= (int)ddItems.size()) break;
					SDL_Rect blockRect = { ddRect.x, ddRect.y + DD_BLOCK_H * i, ddRect.w, DD_BLOCK_H };
					if (checkCursor(&blockRect))
					{
						selectItem(ddItems[idx].itemCode);
						closeDropdown();
						return;
					}
				}
			}
			//드롭다운 외부 클릭 → 닫기
			closeDropdown();
			return;
		}

		//일반 클릭 처리
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
		else if (checkCursor(&cookwareBtn))
		{
			if (cookwareCode == -1)
			{
				openDropdown(0, itemFlag::COOKWARE);
			}
			else
			{
				cookwareCode = -1;
			}
		}
		else if (checkCursor(&cookBtn))
		{
			onClickCookBtn();
		}
		else
		{
			//Ingredient 버튼 클릭
			for (int i = 0; i < 6; i++)
			{
				if (checkCursor(&ingredientBtn[i]))
				{
					if (i < ingredientCount)
					{
						//기존 재료 슬롯 클릭 → 제거 후 앞으로 당기기
						removeIngredient(i);
					}
					else if (i == ingredientCount && ingredientCount < 6)
					{
						//"+" 버튼 클릭 → 드롭다운 열기
						openDropdown(i + 1, itemFlag::INGREDIENT);
					}
					break;
				}
			}
		}
	}

	void mouseWheel()
	{
		if (!ddOpen || ddRatio < 1.0f) return;
		if (!checkCursor(&ddRect)) return;

		int maxScroll = myMax(0, (int)ddItems.size() - MAX_DD_VISIBLE);
		if (event.wheel.y > 0 && ddScroll > 0) ddScroll--;
		else if (event.wheel.y < 0 && ddScroll < maxScroll) ddScroll++;
	}

	//========== 드롭다운 헬퍼 함수 ==========

	//이미 선택된 아이템 수량 계산
	int countSelected(int itemCode)
	{
		int count = 0;
		if (cookwareCode == itemCode) count++;
		for (int i = 0; i < ingredientCount; i++)
		{
			if (ingredientCode[i] == itemCode) count++;
		}
		return count;
	}

	//포켓 하나를 스캔하여 ddItems에 합산
	void scanPocket(ItemPocket* pocket, itemFlag targetFlag)
	{
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			ItemData& item = pocket->itemInfo[i];
			if (item.checkFlag(targetFlag))
			{
				bool found = false;
				for (int j = 0; j < ddItems.size(); j++)
				{
					if (ddItems[j].itemCode == item.itemCode)
					{
						ddItems[j].totalCount += item.number;
						found = true;
						break;
					}
				}
				if (!found)
				{
					ddItems.push_back({ (int)item.itemCode, (int)item.number });
				}
			}
		}
	}

	//주변 아이템 스캔
	void scanItems(itemFlag targetFlag)
	{
		ddItems.clear();

		//1. 플레이어 장비
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		scanPocket(equipPtr, targetFlag);

		//2. 장비 내부 포켓 1단계 (가방 안 아이템)
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				scanPocket(equipPtr->itemInfo[i].pocketPtr.get(), targetFlag);
			}
		}

		//3. 바닥 아이템스택 + 프롭 포켓 (주변 9타일)
		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);

			//바닥 아이템스택
			ItemStack* stack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (stack != nullptr)
			{
				scanPocket(stack->getPocket(), targetFlag);
			}

			//프롭 내부 포켓 (냉장고, 상자 등)
			Prop* prop = TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				scanPocket(prop->leadItem.pocketPtr.get(), targetFlag);
			}
		}

		//이미 선택된 수량을 빼고 남은 것이 없으면 제거
		for (int i = (int)ddItems.size() - 1; i >= 0; i--)
		{
			ddItems[i].totalCount -= countSelected(ddItems[i].itemCode);
			if (ddItems[i].totalCount <= 0)
			{
				ddItems.erase(ddItems.begin() + i);
			}
		}
	}

	//드롭다운 열기
	void openDropdown(int target, itemFlag flag)
	{
		scanItems(flag);
		if (ddItems.empty())
		{
			updateLog(L"There are no items nearby.");
			return;
		}
		ddTarget = target;
		ddOpen = true;
		ddRatio = 0.0f;
		ddScroll = 0;

		//부모 Rect 결정
		SDL_Rect parentRect;
		if (target == 0) parentRect = cookwareBtn;
		else parentRect = ingredientBtn[ingredientCount];

		int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
		ddRect = { parentRect.x, parentRect.y + parentRect.h, parentRect.w, DD_BLOCK_H * visibleCount };
	}

	//드롭다운 닫기
	void closeDropdown()
	{
		ddOpen = false;
		ddRatio = 0.0f;
		ddTarget = -1;
	}

	//아이템 선택
	void selectItem(int itemCode)
	{
		if (ddTarget == 0)
		{
			cookwareCode = itemCode;
		}
		else if (ddTarget >= 1 && ingredientCount < 6)
		{
			ingredientCode[ingredientCount] = itemCode;
			ingredientCount++;
		}
	}

	//재료 제거 (앞으로 당기기)
	void removeIngredient(int slotIndex)
	{
		for (int i = slotIndex; i < ingredientCount - 1; i++)
		{
			ingredientCode[i] = ingredientCode[i + 1];
		}
		ingredientCode[ingredientCount - 1] = -1;
		ingredientCount--;
	}

	//드롭다운 그리기
	void drawDropdown()
	{
		if (!ddOpen) return;

		int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
		int animH = (int)(ddRect.h * ddRatio);

		//애니메이션 중: 검은 배경만
		drawFillRect(ddRect.x, ddRect.y, ddRect.w, animH, col::black);
		drawRect(ddRect.x, ddRect.y, ddRect.w, animH, col::gray);

		if (ddRatio < 1.0f) return;

		//완전 열림: 내용 그리기
		for (int i = 0; i < visibleCount; i++)
		{
			int idx = i + ddScroll;
			if (idx >= (int)ddItems.size()) break;

			SDL_Rect blockRect = { ddRect.x, ddRect.y + DD_BLOCK_H * i, ddRect.w, DD_BLOCK_H };

			//호버/클릭 색상
			if (checkCursor(&blockRect))
			{
				drawFillRect(blockRect, click ? lowCol::deepBlue : lowCol::blue);
			}
			else
			{
				drawFillRect(blockRect, col::black);
			}

			//아이콘
			setZoom(2.0);
			drawSpriteCenter(spr::itemset, itemDex[ddItems[idx].itemCode].itemSprIndex, blockRect.x + 16, blockRect.y + DD_BLOCK_H / 2);
			setZoom(1.0);

			//이름
			setFontSize(16);
			drawText(itemDex[ddItems[idx].itemCode].name, blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);

			//블록 사이 구분선 (마지막 제외)
			if (i < visibleCount - 1)
			{
				int lineY = blockRect.y + DD_BLOCK_H - 1;
				int centerX = ddRect.x + ddRect.w / 2;
				int halfLen = ddRect.w / 2 - 8;
				for (int p = 0; p < halfLen; p++)
				{
					Uint8 alpha = 255 - (255 * p / halfLen);
					drawPoint(centerX - p, lineY, col::gray, alpha);
					drawPoint(centerX + p, lineY, col::gray, alpha);
				}
			}
		}

		//외곽 테두리
		drawRect(ddRect.x, ddRect.y, ddRect.w, ddRect.h, col::gray);

		//스크롤바 (아이템이 MAX_DD_VISIBLE보다 많을 때만)
		if ((int)ddItems.size() > MAX_DD_VISIBLE)
		{
			int sbX = ddRect.x + ddRect.w - 4;
			int sbY = ddRect.y + 2;
			int sbH = ddRect.h - 4;

			//트랙
			SDL_Rect scrollTrack = { sbX, sbY, 2, sbH };
			drawFillRect(scrollTrack, { 120, 120, 120 });

			//썸
			int maxScroll = (int)ddItems.size() - MAX_DD_VISIBLE;
			int thumbH = myMax(5, (int)(sbH * ((double)MAX_DD_VISIBLE / ddItems.size())));
			int thumbY = sbY + (int)((sbH - thumbH) * ((double)ddScroll / maxScroll));

			SDL_Rect scrollThumb = { sbX, thumbY, 2, thumbH };
			drawFillRect(scrollThumb, col::white);
		}
	}

	void step()
	{
		tabType = tabFlag::back;

		//드롭다운 애니메이션
		if (ddOpen && ddRatio < 1.0f)
		{
			ddRatio += 0.1f;
			if (ddRatio >= 1.0f) ddRatio = 1.0f;
		}
	}
};