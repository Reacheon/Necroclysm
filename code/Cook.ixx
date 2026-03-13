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
	static constexpr int DD_HEATSRC = -2;   // 열원 드롭다운 타겟
	bool ddOpen = false;
	int ddTarget = -1;          // -1:없음, DD_HEATSRC:열원, 0:cookware, 1~:ingredient 슬롯
	float ddRatio = 0.0f;       // 애니메이션 비율 (0.0~1.0)
	int ddScroll = 0;
	SDL_Rect ddRect = {};

	struct DdItem {
		int itemCode;
		int totalCount;
		ItemData* itemPtr;
		Prop* propPtr = nullptr;  // 열원 전용 (나머지 nullptr)
	};
	std::vector<DdItem> ddItems;

	//레시피 구조체
	struct CookRecipe {
		int resultCode;
		std::wstring resultName;
		std::vector<int> heatSources;
		std::vector<int> cookwareList;
		int minWaterML;
		std::vector<int> requiredIngredients;
	};
	inline static const std::vector<CookRecipe> recipes = {
		{
			itemID::chickPilaff,
			L"Chicken Pilaf",
			{ itemID::campfire, itemID::electricCooktop },
			{ itemID::fryingPan, itemID::cookingPot },
			200,
			{ itemID::rice, itemID::butter, itemID::rawChicken, itemID::egg }
		},
	};

	//선택된 재료
	Prop* heatSrcPropPtr = nullptr;
	ItemData* cookwarePtr = nullptr;
	std::array<int, 6> ingredientCode;
	int ingredientCount = 0;
	int matchedRecipeIdx = -1;
	bool canCook = false;
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
			if (heatSrcPropPtr != nullptr)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::itemset, getItemSprIndex(heatSrcPropPtr->leadItem), heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 5);
				//drawSpriteCenter(spr::itemset, getItemSprIndex(heatSrcPropPtr->leadItem), heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 10);
				setZoom(1.0);
				setFontSize(12);
				drawTextCenter(heatSrcPropPtr->leadItem.name, heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y);
			}
			else
			{
				setZoom(3.0);
				//drawSpriteCenter(spr::itemset, 152, heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 - 5);
				setZoom(1.0);
				setFontSize(16);
				drawTextCenter(L"Heat", heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y);
				//drawTextCenter(L"Heat", heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 30 - 40);
				//drawTextCenter(L"Source", heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 30 - 23);
			}

			if (checkCursor(&recipeBtn)) drawStadium(recipeBtn, click ? lowCol::deepBlue : lowCol::blue, 255, 3);
			else drawStadium(recipeBtn, col::black, 255, 3);
			setZoom(1.0);
			drawSpriteCenter(spr::icon48, 190, recipeBtn.x + recipeBtn.w / 2, recipeBtn.y + recipeBtn.h / 2 + 2);
			setZoom(1.0);
			setFontSize(16);
			drawTextCenter(L"Recipe", recipeBtn.x + recipeBtn.w / 2, recipeBtn.y);

			//확대된 요리 그림
			
			drawSpriteCenter(spr::fryingPan, 1, cookBase.x + cookBase.w / 2, cookBase.y + cookBase.h / 2 - 140);

			//쿡웨어 버튼(프라이팬 혹은 냄비 혹은 뚝배기)
			setFontSize(18);
			drawTextCenter(col2Str(col::lightGray) + L"Cookware", cookwareBtn.x - 52, cookwareBtn.y + cookwareBtn.h/2);
			if (checkCursor(&cookwareBtn)) { drawFillRect(cookwareBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(cookwareBtn, col::lightGray); }
			else { drawFillRect(cookwareBtn, col::black); drawRect(cookwareBtn, col::gray); }

			if (cookwarePtr != nullptr)
			{
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, getItemSprIndex(*cookwarePtr), cookwareBtn.x + 20, cookwareBtn.y + cookwareBtn.h / 2);
				setZoom(1.0);
				setFontSize(16);
				int waterML = getWaterML(cookwarePtr);
				if (waterML > 0)
					drawText(cookwarePtr->name + L" (" + std::to_wstring(waterML) + L"mL)", cookwareBtn.x + 46, cookwareBtn.y + cookwareBtn.h / 2 - 11);
				else
					drawText(cookwarePtr->name, cookwareBtn.x + 46, cookwareBtn.y + cookwareBtn.h / 2 - 11);
			}

			setFontSize(16);
			drawTextCenter(col2Str(col::lightGray) + L"Ingredients", cookBase.x + cookBase.w / 2, cookBase.y + 273);

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
					drawSpriteCenter(spr::itemset, getItemSprIndex(itemDex[ingredientCode[i]]), ingredientBtn[i].x + 20, ingredientBtn[i].y + ingredientBtn[i].h / 2);
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

			if (canCook)
			{
				if (checkCursor(&cookBtn)) { drawFillRect(cookBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(cookBtn, col::lightGray); }
				else { drawFillRect(cookBtn, col::black); drawRect(cookBtn, col::gray); }
				setFontSize(22);
				drawTextCenter(L"Cook", cookBtn.x + cookBtn.w / 2, cookBtn.y + cookBtn.h / 2);
			}
			else
			{
				drawFillRect(cookBtn, col::black, 80);
				drawRect(cookBtn, col::gray, 80);
				setFontSize(22);
				drawTextCenter(col2Str(col::gray) + L"Cook", cookBtn.x + cookBtn.w / 2, cookBtn.y + cookBtn.h / 2);
			}
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
		if (heatSrcPropPtr != nullptr)
		{
			heatSrcPropPtr = nullptr;
		}
		else
		{
			openDropdown(DD_HEATSRC, itemFlag::INGREDIENT);
		}
	}
	void onClickCookBtn()
	{
		if (!canCook || matchedRecipeIdx < 0) return;
		const CookRecipe& recipe = recipes[matchedRecipeIdx];
		updateLog(recipe.resultName + L" has been cooked successfully!");
		//TODO: 실제 아이템 생성 및 재료 소모 로직 추가
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
					SDL_Rect blockRect = { ddRect.x, ddRect.y + DD_BLOCK_H * i, ddRect.w, DD_BLOCK_H - 1 };
					if (checkCursor(&blockRect))
					{
						selectItem(idx);
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
			if (cookwarePtr == nullptr)
			{
				openDropdown(0, itemFlag::COOKWARE);
			}
			else
			{
				cookwarePtr = nullptr;
			}
		}
		else if (checkCursor(&cookBtn))
		{
			if (canCook) onClickCookBtn();
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

	//========== 헬퍼 함수 ==========

	//열원 아이템 코드 확인
	static bool isHeatSource(int code)
	{
		return code == itemID::campfire || code == itemID::electricCooktop;
	}

	//쿡웨어 내부 물의 양(mL) 반환, 물 없으면 0
	static int getWaterML(ItemData* cw)
	{
		if (cw == nullptr || cw->pocketPtr == nullptr) return 0;
		for (auto& item : cw->pocketPtr->itemInfo)
		{
			if (item.itemCode == itemID::water) return (int)item.number;
		}
		return 0;
	}

	//========== 드롭다운 헬퍼 함수 ==========

	//이미 선택된 아이템 수량 계산 (Ingredient 전용, Cookware는 포인터로 별도 비교)
	int countSelected(int itemCode)
	{
		int count = 0;
		for (int i = 0; i < ingredientCount; i++)
		{
			if (ingredientCode[i] == itemCode) count++;
		}
		return count;
	}

	//포켓 하나를 스캔하여 ddItems에 등록
	void scanPocket(ItemPocket* pocket, itemFlag targetFlag, bool individual)
	{
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			ItemData& item = pocket->itemInfo[i];
			if (item.checkFlag(targetFlag))
			{
				if (individual)
				{
					//Cookware: 개별 등록 (내부 상태가 다를 수 있으므로 합산하지 않음)
					ddItems.push_back({ (int)item.itemCode, 1, &item });
				}
				else
				{
					//Ingredient: itemCode 기준 합산
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
						ddItems.push_back({ (int)item.itemCode, (int)item.number, &item });
					}
				}
			}
		}
	}

	//주변 아이템 스캔
	void scanItems(itemFlag targetFlag)
	{
		ddItems.clear();
		bool individual = (targetFlag == itemFlag::COOKWARE);

		//1. 플레이어 장비
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		scanPocket(equipPtr, targetFlag, individual);

		//2. 장비 내부 포켓 1단계 (가방 안 아이템)
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				scanPocket(equipPtr->itemInfo[i].pocketPtr.get(), targetFlag, individual);
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
				scanPocket(stack->getPocket(), targetFlag, individual);
			}

			//프롭 내부 포켓 (냉장고, 상자 등)
			Prop* prop = TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				scanPocket(prop->leadItem.pocketPtr.get(), targetFlag, individual);
			}
		}

		//이미 선택된 아이템 제거
		if (individual)
		{
			//Cookware: 포인터로 비교하여 이미 선택된 아이템 제거
			for (int i = (int)ddItems.size() - 1; i >= 0; i--)
			{
				if (ddItems[i].itemPtr == cookwarePtr)
				{
					ddItems.erase(ddItems.begin() + i);
				}
			}
		}
		else
		{
			//Ingredient: 코드 기준 수량 차감
			for (int i = (int)ddItems.size() - 1; i >= 0; i--)
			{
				ddItems[i].totalCount -= countSelected(ddItems[i].itemCode);
				if (ddItems[i].totalCount <= 0)
				{
					ddItems.erase(ddItems.begin() + i);
				}
			}
		}
	}

	//주변 열원 프롭 스캔 (9타일)
	void scanHeatSources()
	{
		ddItems.clear();
		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);
			Prop* prop = TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (prop != nullptr && isHeatSource(prop->leadItem.itemCode))
			{
				if (prop == heatSrcPropPtr) continue;
				ddItems.push_back({ (int)prop->leadItem.itemCode, 1, &prop->leadItem, prop });
			}
		}
	}

	//드롭다운 열기
	void openDropdown(int target, itemFlag flag)
	{
		if (target == DD_HEATSRC)
			scanHeatSources();
		else
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
		if (target == DD_HEATSRC) parentRect = heatSrcBtn;
		else if (target == 0) parentRect = cookwareBtn;
		else parentRect = ingredientBtn[ingredientCount];

		int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
		int ddW = myMax(parentRect.w, 180);
		ddRect = { parentRect.x, parentRect.y + parentRect.h, ddW, DD_BLOCK_H * visibleCount };
	}

	//드롭다운 닫기
	void closeDropdown()
	{
		ddOpen = false;
		ddRatio = 0.0f;
		ddTarget = -1;
	}

	//아이템 선택
	void selectItem(int ddIndex)
	{
		if (ddTarget == DD_HEATSRC)
		{
			heatSrcPropPtr = ddItems[ddIndex].propPtr;
		}
		else if (ddTarget == 0)
		{
			cookwarePtr = ddItems[ddIndex].itemPtr;
		}
		else if (ddTarget >= 1 && ingredientCount < 6)
		{
			ingredientCode[ingredientCount] = ddItems[ddIndex].itemCode;
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

	//========== 레시피 체크 ==========

	//가장 많이 일치하는 레시피를 찾고, 모든 조건 충족 시 canCook 활성화
	void checkCanCook()
	{
		matchedRecipeIdx = -1;
		canCook = false;

		int bestMatchCount = 0;
		int bestIdx = -1;

		for (int r = 0; r < (int)recipes.size(); r++)
		{
			const CookRecipe& recipe = recipes[r];

			//재료 매칭 수 세기
			int matchCount = 0;
			for (int req : recipe.requiredIngredients)
			{
				for (int s = 0; s < ingredientCount; s++)
				{
					if (ingredientCode[s] == req) { matchCount++; break; }
				}
			}

			if (matchCount > bestMatchCount)
			{
				bestMatchCount = matchCount;
				bestIdx = r;
			}
		}

		if (bestIdx < 0) return;
		const CookRecipe& best = recipes[bestIdx];

		//1. 모든 필수 재료가 있는지 확인
		if (bestMatchCount < (int)best.requiredIngredients.size()) return;

		//2. 열원 확인
		if (heatSrcPropPtr == nullptr) return;
		bool heatOK = false;
		for (int h : best.heatSources)
		{
			if ((int)heatSrcPropPtr->leadItem.itemCode == h) { heatOK = true; break; }
		}
		if (!heatOK) return;

		//3. 쿡웨어 확인
		if (cookwarePtr == nullptr) return;
		bool cwOK = false;
		for (int c : best.cookwareList)
		{
			if ((int)cookwarePtr->itemCode == c) { cwOK = true; break; }
		}
		if (!cwOK) return;

		//4. 물의 양 확인
		if (best.minWaterML > 0)
		{
			int waterML = getWaterML(cookwarePtr);
			if (waterML < best.minWaterML) return;
		}

		//모든 조건 충족
		matchedRecipeIdx = bestIdx;
		canCook = true;
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

			SDL_Rect blockRect = { ddRect.x, ddRect.y + DD_BLOCK_H * i, ddRect.w, DD_BLOCK_H - 1 };

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
			drawSpriteCenter(spr::itemset, getItemSprIndex(*ddItems[idx].itemPtr), blockRect.x + 16, blockRect.y + DD_BLOCK_H / 2);
			setZoom(1.0);

			//이름 (쿡웨어일 경우 mL 표시)
			setFontSize(16);
			if (ddTarget == 0)
			{
				int waterML = getWaterML(ddItems[idx].itemPtr);
				if (waterML > 0)
					drawText(ddItems[idx].itemPtr->name + L" (" + std::to_wstring(waterML) + L"mL)", blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);
				else
					drawText(ddItems[idx].itemPtr->name, blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);
			}
			else
			{
				drawText(ddItems[idx].itemPtr->name, blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);
			}

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

		//레시피 확인
		checkCanCook();
	}
};