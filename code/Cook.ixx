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
	SDL_Rect transferBtn;
	SDL_Rect eatBtn;
	int cookCursor = -1;
	int cookScroll = 0;

	//드롭다운 메뉴 상태
	static constexpr int MAX_DD_VISIBLE = 6;
	static constexpr int DD_BLOCK_H = 36;
	static constexpr int DD_HEATSRC = -2;   // 열원 드롭다운 타겟
	static constexpr int DD_TRANSFER = -3;  // Transfer 드롭다운 타겟
	bool ddOpen = false;
	int ddTarget = -1;          // -1:없음, DD_HEATSRC:열원, DD_TRANSFER:Transfer, 0:cookware, 1~:ingredient 슬롯
	float ddRatio = 0.0f;       // 애니메이션 비율 (0.0~1.0)
	int ddScroll = 0;
	SDL_Rect ddRect = {};

	struct DdItem {
		int itemCode;
		int totalCount;
		ItemData* itemPtr;
		Prop* propPtr = nullptr;    // 열원 전용 (나머지 nullptr)
		std::wstring locationTag;   // Transfer 전용: "Wield", "E Tile" 등 (나머지 빈 문자열)
		ItemPocket* sourcePocket = nullptr; // Transfer 전용: 아이템이 속한 포켓
		int sourceIndex = -1;       // Transfer 전용: 포켓 내 인덱스
	};
	std::vector<DdItem> ddItems;

	bool resultPhase = false;

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
			itemID::eggFriedRice,
			L"Egg Fried Rice",
			{ itemID::campfire, itemID::electricCooktop },
			{ itemID::fryingPan, itemID::cookingPot },
			0,
			{ itemID::rice, itemID::egg, itemID::scallion, itemID::carrot }
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
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
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
		transferBtn = { cookBase.x + 170, cookBase.y + 465, 140, 40 };
		transferBtn.x += transferBtn.w / 2 - 152;
		eatBtn = { cookBase.x + 170, cookBase.y + 465, 140, 40 };
		eatBtn.x += eatBtn.w / 2;
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
			{
				if (cookwarePtr != nullptr)
				{
					Sprite* cwSpr = spr::fryingPan;
					if ((int)cookwarePtr->itemCode == itemID::cookingPot) cwSpr = spr::cookingPot;
					int cwSprIdx = getCookwareLargeSprIndex();
					drawSpriteCenter(cwSpr, cwSprIdx, cookBase.x + cookBase.w / 2, cookBase.y + cookBase.h / 2 - 140);
				}

				//김 애니메이션 (요리 완성 시)
				if (resultPhase)
				{
					Uint32 t = SDL_GetTicks();
					int steamCX = cookBase.x + cookBase.w / 2;
					int steamCY = cookBase.y + cookBase.h / 2 - 140;

					constexpr int steamCount = 6;
					constexpr float offsets[steamCount] = { 0, 370, 780, 1150, 1560, 1900 };
					constexpr float xOffsets[steamCount] = { 0, 2.1f, 4.2f, 1.0f, 3.3f, 5.4f };

					for (int p = 0; p < steamCount; p++)
					{
						float phase = std::fmod((t + (Uint32)offsets[p]) / 2400.0f, 1.0f);
						int py = steamCY - (int)(phase * 50);
						int px = steamCX + (int)(std::sin(phase * 3.14159 * 2.0 + xOffsets[p]) * 8);
						int rx = 7 + (int)(phase * 6);
						int ry = 5 + (int)(phase * 4);
						Uint8 alpha = (Uint8)((1.0f - phase * phase) * 70);
						drawFillEllipse(px, py, rx, ry, { 255, 255, 255 }, alpha);
					}
				}
			}

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

				if (resultPhase && matchedRecipeIdx >= 0)
				{
					//결과 단계: 쿡웨어 이름(흰색 12px) + 요리 이름(노란색 16px) 2줄 표시
					setFontSize(12);
					drawText(cookwarePtr->name, cookwareBtn.x + 46, cookwareBtn.y + 3);
					setFontSize(16);
					drawText(L"#e9c900" + recipes[matchedRecipeIdx].resultName, cookwareBtn.x + 46, cookwareBtn.y + 16);
				}
				else
				{
					setFontSize(16);
					int waterML = getWaterML(cookwarePtr);
					if (waterML > 0)
						drawText(cookwarePtr->name + L" (" + std::to_wstring(waterML) + L"mL)", cookwareBtn.x + 46, cookwareBtn.y + cookwareBtn.h / 2 - 11);
					else
						drawText(cookwarePtr->name, cookwareBtn.x + 46, cookwareBtn.y + cookwareBtn.h / 2 - 11);
				}
			}

			setFontSize(16);
			if (resultPhase)
				drawTextCenter(col2Str(col::lightGray) + L"Description", cookBase.x + cookBase.w / 2, cookBase.y + 273);
			else
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

			if (!resultPhase)
			{
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
			}
			//resultPhase일 때 Description 내용 표시
			if (resultPhase && matchedRecipeIdx >= 0)
			{
				int descX = cookBase.x + 32;
				int descY = cookBase.y + 300;
				setFontSize(14);

				if (recipes[matchedRecipeIdx].resultCode == itemID::eggFriedRice)
				{
					//계란볶음밥 설명
					setFontSize(24);
					drawTextCenter(L"#e9c900★★★", cookBase.x + cookBase.w / 2, cookBase.y + 303);

					setFontSize(16);
					drawTextWidth(L" Rice stir-fried with scrambled egg and carrot sauteed in scallion oil. A quick, satisfying meal.", cookBase.x + 19, cookBase.y + 326, false, 380,20);

					for (int i = 0; i < 4; i++)
					{
						int pivotX = cookBase.x+62 + 192*(i%2);
						int pivotY = cookBase.y+398 + 25 * (i / 2);

						if (i == 0)
						{

							setFontSize(16);
							setFont(fontType::mainFontSemiBold);
							drawTextCenter(L"Hunger", pivotX, pivotY);

							setFont(fontType::mainFontBold);
							drawTextCenter(L"#59cb65-12%", pivotX + 92, pivotY);
							setFont(fontType::mainFont);
						}
						else if (i == 1)
						{
							setFontSize(16);
							setFont(fontType::mainFontSemiBold);
							drawTextCenter(L"Thirsty", pivotX, pivotY);

							setFont(fontType::mainFontBold);
							drawTextCenter(L"#59cb65-16%", pivotX + 92, pivotY);
							setFont(fontType::mainFont);
						}
						else if (i == 2)
						{
							setFontSize(16);
							setFont(fontType::mainFontSemiBold);
							drawTextCenter(L"Mental", pivotX, pivotY);

							setFont(fontType::mainFontBold);
							drawTextCenter(L"#59cb65+25%", pivotX + 92, pivotY);
							setFont(fontType::mainFont);
						}
						else if (i == 3)
						{
							setFontSize(16);
							setFont(fontType::mainFontSemiBold);
							drawTextCenter(L"Atk Speed", pivotX, pivotY);

							setFont(fontType::mainFontBold);
							setFontSize(14);
							std::wstring valText = L"#59cb65-11%#e9c900 (-3%)";
							drawTextCenter(valText, pivotX + 92, pivotY);
							int valWidth = getTextWidthWithoutColor(removeColorCodes(valText));
							drawSprite(spr::icon16, 107, pivotX + 92 + valWidth / 2 - 3, pivotY - 12);//별모양 심볼
							setFont(fontType::mainFont);
						}
					}
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

			if (resultPhase)
			{
				//결과 단계: Transfer / Eat Now 버튼 2개
				if (checkCursor(&transferBtn)) { drawFillRect(transferBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(transferBtn, col::lightGray); }
				else { drawFillRect(transferBtn, col::black); drawRect(transferBtn, col::gray); }
				setFontSize(18);
				drawTextCenter(L"Transfer", transferBtn.x + transferBtn.w / 2, transferBtn.y + transferBtn.h / 2);

				if (checkCursor(&eatBtn)) { drawFillRect(eatBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(eatBtn, col::lightGray); }
				else { drawFillRect(eatBtn, col::black); drawRect(eatBtn, col::gray); }
				setFontSize(18);
				drawTextCenter(L"Eat Now", eatBtn.x + eatBtn.w / 2, eatBtn.y + eatBtn.h / 2);
			}
			else if (canCook)
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

		//쿡웨어 내부 물 삭제
		if (cookwarePtr != nullptr && cookwarePtr->pocketPtr != nullptr)
		{
			auto& pocketInfo = cookwarePtr->pocketPtr->itemInfo;
			for (int i = (int)pocketInfo.size() - 1; i >= 0; i--)
			{
				if (pocketInfo[i].itemCode == itemID::water)
				{
					cookwarePtr->pocketPtr->eraseItemInfo(i);
				}
			}
			//쿡웨어에 완성된 요리 추가
			cookwarePtr->pocketPtr->addItemFromDex(recipe.resultCode, 1);
		}

		resultPhase = true;
	}
	void onClickTransferBtn()
	{
		openDropdown(DD_TRANSFER, itemFlag::PLATE);
	}
	void onClickEatBtn()
	{
		if (cookwarePtr == nullptr || cookwarePtr->pocketPtr == nullptr) return;
		if (matchedRecipeIdx < 0) return;

		const CookRecipe& recipe = recipes[matchedRecipeIdx];

		//쿡웨어 포켓에서 완성된 요리 제거
		ItemPocket* cwPocket = cookwarePtr->pocketPtr.get();
		for (int i = 0; i < (int)cwPocket->itemInfo.size(); i++)
		{
			if (cwPocket->itemInfo[i].itemCode == recipe.resultCode)
			{
				cwPocket->eraseItemInfo(i);
				break;
			}
		}

		updateLog(L"You ate " + recipe.resultName + L".");
		close(aniFlag::winUnfoldClose);
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
		else if (resultPhase)
		{
			//결과 단계: Transfer / Eat Now 버튼만 처리
			if (checkCursor(&transferBtn))
			{
				onClickTransferBtn();
			}
			else if (checkCursor(&eatBtn))
			{
				onClickEatBtn();
			}
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

	//쿡웨어 확대 스프라이트 인덱스 (0:빈, 1:물, 2:요리)
	int getCookwareLargeSprIndex()
	{
		if (cookwarePtr == nullptr || cookwarePtr->pocketPtr == nullptr) return 0;
		for (auto& item : cookwarePtr->pocketPtr->itemInfo)
		{
			if (item.itemCode == itemID::eggFriedRice) return 2;
		}
		for (auto& item : cookwarePtr->pocketPtr->itemInfo)
		{
			if (item.itemCode == itemID::water) return 1;
		}
		return 0;
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

	//방향 인덱스 → 문자열 (Transfer 위치 표시용)
	static std::wstring dirToLabel(int dir)
	{
		switch (dir)
		{
		case 0: return L"E";
		case 1: return L"NE";
		case 2: return L"N";
		case 3: return L"NW";
		case 4: return L"W";
		case 5: return L"SW";
		case 6: return L"S";
		case 7: return L"SE";
		default: return L"Here";
		}
	}

	//Transfer 가능한 용기인지 확인 (PLATE 또는 COOKWARE)
	static bool isTransferTarget(const ItemData& item)
	{
		return item.checkFlag(itemFlag::PLATE) || item.checkFlag(itemFlag::COOKWARE);
	}

	//Transfer 대상 포켓 스캔 (개별 등록 + 위치 태그)
	void scanPocketForTransfer(ItemPocket* pocket, const std::wstring& locTag)
	{
		for (int i = 0; i < (int)pocket->itemInfo.size(); i++)
		{
			ItemData& item = pocket->itemInfo[i];
			if (isTransferTarget(item))
			{
				//현재 요리에 사용 중인 쿡웨어는 제외
				if (&item == cookwarePtr) continue;
				ddItems.push_back({ (int)item.itemCode, 1, &item, nullptr, locTag, pocket, i });
			}
		}
	}

	//Transfer 대상 스캔: 장비, 가방, 주변 타일
	void scanTransferTargets()
	{
		ddItems.clear();

		//1. 플레이어 장비 (Wield/Equip)
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		for (int i = 0; i < (int)equipPtr->itemInfo.size(); i++)
		{
			ItemData& item = equipPtr->itemInfo[i];
			if (isTransferTarget(item))
			{
				if (&item == cookwarePtr) continue;
				std::wstring tag;
				if (item.equipState == equipHandFlag::left) tag = L"L Hand";
				else if (item.equipState == equipHandFlag::right) tag = L"R Hand";
				else if (item.equipState == equipHandFlag::both) tag = L"Both";
				else tag = L"Equip";
				ddItems.push_back({ (int)item.itemCode, 1, &item, nullptr, tag, equipPtr, i });
			}
		}

		//2. 장비 내부 포켓 (가방 안)
		for (int i = 0; i < (int)equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				scanPocketForTransfer(equipPtr->itemInfo[i].pocketPtr.get(), L"Bag");
			}
		}

		//3. 바닥 아이템스택 + 프롭 포켓 (주변 9타일)
		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);
			std::wstring tileTag = (dir == -1) ? L"Floor" : (dirToLabel(dir) + L" Tile");

			ItemStack* stack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (stack != nullptr)
			{
				scanPocketForTransfer(stack->getPocket(), tileTag);
			}

			Prop* prop = TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				scanPocketForTransfer(prop->leadItem.pocketPtr.get(), tileTag);
			}
		}
	}

	//드롭다운 열기
	void openDropdown(int target, itemFlag flag)
	{
		if (target == DD_HEATSRC)
			scanHeatSources();
		else if (target == DD_TRANSFER)
			scanTransferTargets();
		else
			scanItems(flag);

		if (ddItems.empty())
		{
			if (target == DD_TRANSFER)
				updateLog(L"There are no containers nearby to transfer to.");
			else
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
		else if (target == DD_TRANSFER) parentRect = transferBtn;
		else if (target == 0) parentRect = cookwareBtn;
		else parentRect = ingredientBtn[ingredientCount];

		int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
		int ddW = myMax(parentRect.w, 220);
		ddRect = { parentRect.x, parentRect.y - DD_BLOCK_H * visibleCount, ddW, DD_BLOCK_H * visibleCount };

		//Transfer 드롭다운은 버튼 위쪽으로 열림 (화면 하단 근처이므로)
		if (target != DD_TRANSFER)
		{
			ddRect.y = parentRect.y + parentRect.h;
		}
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
		else if (ddTarget == DD_TRANSFER)
		{
			executeTransfer(ddIndex);
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

	//Transfer 실행: 쿡웨어에서 완성된 요리를 선택된 용기로 이동
	void executeTransfer(int ddIndex)
	{
		if (cookwarePtr == nullptr || cookwarePtr->pocketPtr == nullptr) return;
		if (matchedRecipeIdx < 0) return;

		const CookRecipe& recipe = recipes[matchedRecipeIdx];
		DdItem& target = ddItems[ddIndex];

		//쿡웨어 포켓에서 완성된 요리 찾기
		ItemPocket* cwPocket = cookwarePtr->pocketPtr.get();
		int dishIndex = -1;
		for (int i = 0; i < (int)cwPocket->itemInfo.size(); i++)
		{
			if (cwPocket->itemInfo[i].itemCode == recipe.resultCode)
			{
				dishIndex = i;
				break;
			}
		}
		if (dishIndex < 0) return;

		//대상 용기에 포켓이 있으면 포켓으로 이동, 없으면 같은 포켓에 추가
		ItemPocket* destPocket = nullptr;
		if (target.itemPtr->pocketPtr != nullptr)
		{
			destPocket = target.itemPtr->pocketPtr.get();
		}
		else if (target.sourcePocket != nullptr)
		{
			destPocket = target.sourcePocket;
		}

		if (destPocket != nullptr)
		{
			//transferItem이 destPocket->itemInfo를 변경할 수 있으므로
			//같은 벡터에 속한 target.itemPtr이 무효화될 수 있음 → 이름을 미리 복사
			std::wstring targetName = target.itemPtr->name;
			cwPocket->transferItem(destPocket, dishIndex, 1);
			updateLog(L"Transferred " + recipe.resultName + L" to " + targetName + L".");
		}

		//Transfer 후 Cook UI 닫기
		close(aniFlag::winUnfoldClose);
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
		if (resultPhase) return; //결과 단계에서는 재검사하지 않음
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

		//애니메이션 중: 검은 배경만 (Transfer는 아래→위로, 나머지는 위→아래로)
		if (ddTarget == DD_TRANSFER)
		{
			int animY = ddRect.y + ddRect.h - animH;
			drawFillRect(ddRect.x, animY, ddRect.w, animH, col::black);
			drawRect(ddRect.x, animY, ddRect.w, animH, col::gray);
		}
		else
		{
			drawFillRect(ddRect.x, ddRect.y, ddRect.w, animH, col::black);
			drawRect(ddRect.x, ddRect.y, ddRect.w, animH, col::gray);
		}

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

			//이름 표시
			setFontSize(16);
			if (ddTarget == DD_TRANSFER)
			{
				//Transfer: 아이템 이름 + 위치 태그 (회색으로)
				drawText(ddItems[idx].itemPtr->name, blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);
				if (!ddItems[idx].locationTag.empty())
				{
					int nameW = getTextWidthWithoutColor(ddItems[idx].itemPtr->name);
					setFontSize(13);
					drawText(col2Str(col::gray) + L" " + ddItems[idx].locationTag, blockRect.x + 42 + nameW, blockRect.y + DD_BLOCK_H / 2 - 9);
				}
			}
			else if (ddTarget == 0)
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