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
import World;
import Player;
import ItemStack;
import Item;
import Prop;
import Sprite;

//Cook
//주어진 재료 최대 6개와 열원, 조리도구를 써서 요리하는 GUI
//DD라는 약칭은 드롭다운메뉴(재료 누르면 아래로 펼쳐지는거)를 의미함
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

	static constexpr int MAX_DD_VISIBLE = 6;
	static constexpr int DD_BLOCK_H = 36;
	static constexpr int DD_HEATSRC = -2;
	static constexpr int DD_TRANSFER = -3;
	bool ddOpen = false;
	int ddTarget = -1;
	float ddRatio = 0.0f;
	int ddScroll = 0;
	SDL_Rect ddRect = {};

	struct DdItem 
	{
		int itemCode;
		int totalCount;
		ItemData* itemPtr;
		Prop* propPtr = nullptr;
		std::wstring locationTag;
		ItemPocket* sourcePocket = nullptr;
		int sourceIndex = -1;
	};
	std::vector<DdItem> ddItems;

	bool resultPhase = false;

	struct CookRecipe 
	{
		int resultCode;
		std::vector<int> heatSources;
		std::vector<int> cookwareList;
		int minWaterML;
		std::vector<int> requiredIngredients;
	};

	inline static const std::vector<CookRecipe> recipes = 
	{
		{
			itemID::eggFriedRice,
			{ itemID::campfire, itemID::electricCooktop },
			{ itemID::fryingPan, itemID::cookingPot },
			0,
			{ itemID::rice, itemID::egg, itemID::scallion, itemID::carrot }
		},
	};

	Prop* heatSrcPropPtr = nullptr;
	ItemData* cookwarePtr = nullptr;
	std::array<int, 6> ingredientCode;
	int ingredientCount = 0;
	int matchedRecipeIdx = -1;
	bool canCook = false;
public:
	Cook() : GUI(false)
	{
		errorBox(ptr != nullptr, L"중복된 GUI 인스턴스가 생성되었다.");
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
			drawWindow(&cookBase, sysStr[260], 0);
			setFlip(SDL_FLIP_HORIZONTAL);
			drawSprite(spr::newWindowArrow, 0, cookBase.x + cookBase.w - 4, cookBase.y + 234);
			setFlip(SDL_FLIP_NONE);

			if (checkCursor(&heatSrcBtn)) drawStadium(heatSrcBtn, click ? lowCol::deepBlue : lowCol::blue, 255, 3);
			else drawStadium(heatSrcBtn, col::black, 255, 3);
			if (heatSrcPropPtr != nullptr)
			{
				setZoom(3.0);
				drawSpriteCenter(spr::itemset, heatSrcPropPtr->leadItem.getSprIndex(), heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y + heatSrcBtn.h / 2 + 5);
				setZoom(1.0);
				setFontSize(12);
				drawTextCenter(heatSrcPropPtr->leadItem.name, heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y);
			}
			else
			{
				setZoom(3.0);
				setZoom(1.0);
				setFontSize(16);
				drawTextCenter(sysStr[391], heatSrcBtn.x + heatSrcBtn.w / 2, heatSrcBtn.y);
			}

			if (checkCursor(&recipeBtn)) drawStadium(recipeBtn, click ? lowCol::deepBlue : lowCol::blue, 255, 3);
			else drawStadium(recipeBtn, col::black, 255, 3);
			setZoom(1.0);
			drawSpriteCenter(spr::icon48, 190, recipeBtn.x + recipeBtn.w / 2, recipeBtn.y + recipeBtn.h / 2 + 2);
			setZoom(1.0);
			setFontSize(16);
			drawTextCenter(sysStr[392], recipeBtn.x + recipeBtn.w / 2, recipeBtn.y);

			{
				if (cookwarePtr != nullptr)
				{
					Sprite* cwSpr = spr::fryingPan;
					if ((int)cookwarePtr->itemCode == itemID::cookingPot) cwSpr = spr::cookingPot;
					int cwSprIdx = getCookwareLargeSprIndex();
					drawSpriteCenter(cwSpr, cwSprIdx, cookBase.x + cookBase.w / 2, cookBase.y + cookBase.h / 2 - 140);
				}

				
				if (resultPhase)
				{
				}
			}

			setFontSize(18);
			drawTextCenter(col2Str(col::lightGray) + sysStr[393], cookwareBtn.x - 52, cookwareBtn.y + cookwareBtn.h/2);
			if (checkCursor(&cookwareBtn)) { drawFillRect(cookwareBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(cookwareBtn, col::lightGray); }
			else { drawFillRect(cookwareBtn, col::black); drawRect(cookwareBtn, col::gray); }

			if (cookwarePtr != nullptr)
			{
				setZoom(2.0);
				drawSpriteCenter(spr::itemset, (*cookwarePtr).getSprIndex(), cookwareBtn.x + 20, cookwareBtn.y + cookwareBtn.h / 2);
				setZoom(1.0);

				if (resultPhase && matchedRecipeIdx >= 0)
				{
					setFontSize(12);
					drawText(cookwarePtr->name, cookwareBtn.x + 46, cookwareBtn.y + 3);
					setFontSize(16);
					drawText(L"#e9c900" + itemDex[recipes[matchedRecipeIdx].resultCode].name, cookwareBtn.x + 46, cookwareBtn.y + 16);
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
				drawTextCenter(col2Str(col::lightGray) + sysStr[394], cookBase.x + cookBase.w / 2, cookBase.y + 273);
			else
				drawTextCenter(col2Str(col::lightGray) + sysStr[167], cookBase.x + cookBase.w / 2, cookBase.y + 273);

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
					if (i < ingredientCount)
					{
						if (checkCursor(&ingredientBtn[i])) { drawFillRect(ingredientBtn[i], click ? lowCol::deepBlue : lowCol::blue); drawRect(ingredientBtn[i], col::lightGray); }
						else { drawFillRect(ingredientBtn[i], col::black); drawRect(ingredientBtn[i], col::gray); }

						setZoom(2.0);
						drawSpriteCenter(spr::itemset, itemDex[ingredientCode[i]].getSprIndex(), ingredientBtn[i].x + 20, ingredientBtn[i].y + ingredientBtn[i].h / 2);
						setZoom(1.0);

						setFontSize(16);
						drawText(itemDex[ingredientCode[i]].name, ingredientBtn[i].x + 46, ingredientBtn[i].y + ingredientBtn[i].h / 2 - 9);
					}
					else if (i == ingredientCount && ingredientCount < 6)
					{
						if (checkCursor(&ingredientBtn[i])) { drawFillRect(ingredientBtn[i], click ? lowCol::deepBlue : lowCol::blue); drawRect(ingredientBtn[i], col::lightGray); }
						else { drawFillRect(ingredientBtn[i], col::black); drawRect(ingredientBtn[i], col::gray); }

						setFontSize(28);
						drawTextCenter(L"+", ingredientBtn[i].x + ingredientBtn[i].w / 2, ingredientBtn[i].y + ingredientBtn[i].h / 2);
					}
					else
					{
						drawFillRect(ingredientBtn[i], col::black, 80);
						drawRect(ingredientBtn[i], col::gray, 80);
					}
				}
			}
			if (resultPhase && matchedRecipeIdx >= 0)
			{
				int descX = cookBase.x + 32;
				int descY = cookBase.y + 300;
				setFontSize(14);

				if (recipes[matchedRecipeIdx].resultCode == itemID::eggFriedRice)
				{
					setFontSize(24);
					drawTextCenter(L"#e9c900★★★", cookBase.x + cookBase.w / 2, cookBase.y + 303);

					setFontSize(16);
					drawTextWidth(L" 계란과 밥, 당근을 볶아 만든 간단한 식사.", cookBase.x + 19, cookBase.y + 326, 380,20);

					for (int i = 0; i < 4; i++)
					{
						int pivotX = cookBase.x+62 + 192*(i%2);
						int pivotY = cookBase.y+398 + 25 * (i / 2);

						if (i == 0)
						{

							setFontSize(16);
							drawTextCenter(sysStr[332], pivotX, pivotY);

							drawTextCenter(L"#59cb65-12%", pivotX + 92, pivotY);
						}
						else if (i == 1)
						{
							setFontSize(16);
							drawTextCenter(sysStr[333], pivotX, pivotY);

							drawTextCenter(L"#59cb65-16%", pivotX + 92, pivotY);
						}
						else if (i == 2)
						{
							setFontSize(16);
							drawTextCenter(sysStr[397], pivotX, pivotY);

							drawTextCenter(L"#59cb65+25%", pivotX + 92, pivotY);
						}
						else if (i == 3)
						{
							setFontSize(16);
							drawTextCenter(sysStr[398], pivotX, pivotY);

							setFontSize(14);
							std::wstring valText = L"#59cb65-11%#e9c900 (-3%)";
							drawTextCenter(valText, pivotX + 92, pivotY);
							int valWidth = getTextWidthWithoutColor(removeColorCodes(valText));
							drawSprite(spr::icon16, 107, pivotX + 92 + valWidth / 2 - 3, pivotY - 16);
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
				if (checkCursor(&transferBtn)) { drawFillRect(transferBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(transferBtn, col::lightGray); }
				else { drawFillRect(transferBtn, col::black); drawRect(transferBtn, col::gray); }
				setFontSize(18);
				drawTextCenter(sysStr[395], transferBtn.x + transferBtn.w / 2, transferBtn.y + transferBtn.h / 2);

				if (checkCursor(&eatBtn)) { drawFillRect(eatBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(eatBtn, col::lightGray); }
				else { drawFillRect(eatBtn, col::black); drawRect(eatBtn, col::gray); }
				setFontSize(18);
				drawTextCenter(sysStr[20], eatBtn.x + eatBtn.w / 2, eatBtn.y + eatBtn.h / 2);
			}
			else if (canCook)
			{
				if (checkCursor(&cookBtn)) { drawFillRect(cookBtn, click ? lowCol::deepBlue : lowCol::blue); drawRect(cookBtn, col::lightGray); }
				else { drawFillRect(cookBtn, col::black); drawRect(cookBtn, col::gray); }
				setFontSize(20);
				drawTextCenter(sysStr[396], cookBtn.x + cookBtn.w / 2, cookBtn.y + cookBtn.h / 2);
			}
			else
			{
				drawFillRect(cookBtn, col::black, 80);
				drawRect(cookBtn, col::gray, 80);
				setFontSize(20);
				drawTextCenter(col2Str(col::gray) + sysStr[396], cookBtn.x + cookBtn.w / 2, cookBtn.y + cookBtn.h / 2);
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

		drawDropdown();
	}
	void onClickRecipeBtn()
	{
		updateLog(L"[DEBUG] 레시피 버튼을 눌렀다.");
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
		updateLog(replaceStr(sysStr[401], L"(%item)", itemDex[recipe.resultCode].name));

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

		ItemPocket* cwPocket = cookwarePtr->pocketPtr.get();
		for (int i = 0; i < (int)cwPocket->itemInfo.size(); i++)
		{
			if (cwPocket->itemInfo[i].itemCode == recipe.resultCode)
			{
				cwPocket->eraseItemInfo(i);
				break;
			}
		}

		updateLog(replaceStr(sysStr[402], L"(%item)", itemDex[recipe.resultCode].name));
		close(aniFlag::winUnfoldClose);
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

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
			closeDropdown();
			return;
		}

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else if (resultPhase)
		{
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
			for (int i = 0; i < 6; i++)
			{
				if (checkCursor(&ingredientBtn[i]))
				{
					if (i < ingredientCount)
					{
						removeIngredient(i);
					}
					else if (i == ingredientCount && ingredientCount < 6)
					{
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


	static bool isHeatSource(int code)
	{
		return code == itemID::campfire || code == itemID::electricCooktop;
	}

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

	static int getWaterML(ItemData* cw)
	{
		if (cw == nullptr || cw->pocketPtr == nullptr) return 0;
		for (auto& item : cw->pocketPtr->itemInfo)
		{
			if (item.itemCode == itemID::water) return (int)item.number;
		}
		return 0;
	}


	int countSelected(int itemCode)
	{
		int count = 0;
		for (int i = 0; i < ingredientCount; i++)
		{
			if (ingredientCode[i] == itemCode) count++;
		}
		return count;
	}

	void scanPocket(ItemPocket* pocket, itemFlag targetFlag, bool individual)
	{
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			ItemData& item = pocket->itemInfo[i];
			if (item.checkFlag(targetFlag))
			{
				if (individual)
				{
					ddItems.push_back({ (int)item.itemCode, 1, &item });
				}
				else
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
						ddItems.push_back({ (int)item.itemCode, (int)item.number, &item });
					}
				}
			}
		}
	}

	void scanItems(itemFlag targetFlag)
	{
		ddItems.clear();
		bool individual = (targetFlag == itemFlag::COOKWARE);

		ItemPocket* equipPtr = PlayerEquip();
		scanPocket(equipPtr, targetFlag, individual);

		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				scanPocket(equipPtr->itemInfo[i].pocketPtr.get(), targetFlag, individual);
			}
		}

		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);

			ItemStack* stack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (stack != nullptr)
			{
				scanPocket(stack->getPocket(), targetFlag, individual);
			}

			Prop* prop = TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr)
			{
				scanPocket(prop->leadItem.pocketPtr.get(), targetFlag, individual);
			}
		}

		if (individual)
		{
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


	static bool isTransferTarget(const ItemData& item)
	{
		return item.checkFlag(itemFlag::PLATE) || item.checkFlag(itemFlag::COOKWARE);
	}

	void scanPocketForTransfer(ItemPocket* pocket, const std::wstring& locTag)
	{
		for (int i = 0; i < (int)pocket->itemInfo.size(); i++)
		{
			ItemData& item = pocket->itemInfo[i];
			if (isTransferTarget(item))
			{
				if (&item == cookwarePtr) continue;
				ddItems.push_back({ (int)item.itemCode, 1, &item, nullptr, locTag, pocket, i });
			}
		}
	}

	void scanTransferTargets()
	{
		ddItems.clear();

		ItemPocket* equipPtr = PlayerEquip();
		for (int i = 0; i < (int)equipPtr->itemInfo.size(); i++)
		{
			ItemData& item = equipPtr->itemInfo[i];
			if (isTransferTarget(item))
			{
				if (&item == cookwarePtr) continue;
				std::wstring tag;
				if (item.equipState == equipHandFlag::left) tag = sysStr[32];
				else if (item.equipState == equipHandFlag::right) tag = sysStr[33];
				else if (item.equipState == equipHandFlag::both) tag = sysStr[34];
				else tag = sysStr[242];
				ddItems.push_back({ (int)item.itemCode, 1, &item, nullptr, tag, equipPtr, i });
			}
		}

		for (int i = 0; i < (int)equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketPtr != nullptr)
			{
				scanPocketForTransfer(equipPtr->itemInfo[i].pocketPtr.get(), sysStr[399]);
			}
		}

		for (int dir = -1; dir < 8; dir++)
		{
			int dx = 0, dy = 0;
			dir2Coord(dir, dx, dy);


			auto dirToLabel = [](int dir) -> std::wstring
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
					default: return L"?";
					}
				};

			std::wstring tileTag = (dir == -1) ? sysStr[182] : (dirToLabel(dir) + L" " + sysStr[400]);

			ItemStack* stack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (stack != nullptr) scanPocketForTransfer(stack->getPocket(), tileTag);

			Prop* prop = TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ());
			if (prop != nullptr && prop->leadItem.pocketPtr != nullptr) scanPocketForTransfer(prop->leadItem.pocketPtr.get(), tileTag);
		}
	}

	void openDropdown(int target, itemFlag flag)
	{
		if (target == DD_HEATSRC) scanHeatSources();
		else if (target == DD_TRANSFER) scanTransferTargets();
		else scanItems(flag);

		if (ddItems.empty())
		{
			if (target == DD_TRANSFER) updateLog(sysStr[403]);
			else updateLog(sysStr[404]);
			return;
		}

		ddTarget = target;
		ddOpen = true;
		ddRatio = 0.0f;
		ddScroll = 0;

		SDL_Rect parentRect;
		if (target == DD_HEATSRC) parentRect = heatSrcBtn;
		else if (target == DD_TRANSFER) parentRect = transferBtn;
		else if (target == 0) parentRect = cookwareBtn;
		else parentRect = ingredientBtn[ingredientCount];

		int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
		int ddW = myMax(parentRect.w, 220);
		ddRect = { parentRect.x, parentRect.y - DD_BLOCK_H * visibleCount, ddW, DD_BLOCK_H * visibleCount };

		if (target != DD_TRANSFER) ddRect.y = parentRect.y + parentRect.h;
	}

	void closeDropdown()
	{
		ddOpen = false;
		ddRatio = 0.0f;
		ddTarget = -1;
	}

	void selectItem(int ddIndex)
	{
		if (ddTarget == DD_HEATSRC) heatSrcPropPtr = ddItems[ddIndex].propPtr;
		else if (ddTarget == DD_TRANSFER) executeTransfer(ddIndex);
		else if (ddTarget == 0) cookwarePtr = ddItems[ddIndex].itemPtr;
		else if (ddTarget >= 1 && ingredientCount < 6)
		{
			ingredientCode[ingredientCount] = ddItems[ddIndex].itemCode;
			ingredientCount++;
		}
	}

	void executeTransfer(int ddIndex)
	{
		if (cookwarePtr == nullptr || cookwarePtr->pocketPtr == nullptr) return;
		if (matchedRecipeIdx < 0) return;

		const CookRecipe& recipe = recipes[matchedRecipeIdx];
		DdItem& target = ddItems[ddIndex];

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
			std::wstring targetName = target.itemPtr->name;
			cwPocket->transferItem(destPocket, dishIndex, 1);
			updateLog(replaceStr(replaceStr(sysStr[405], L"(%item)", itemDex[recipe.resultCode].name), L"(%target)", targetName));
		}

		close(aniFlag::winUnfoldClose);
	}

	void removeIngredient(int slotIndex)
	{
		for (int i = slotIndex; i < ingredientCount - 1; i++) ingredientCode[i] = ingredientCode[i + 1];
		ingredientCode[ingredientCount - 1] = -1;
		ingredientCount--;
	}


	void checkCanCook()
	{
		if (resultPhase) return;
		matchedRecipeIdx = -1;
		canCook = false;

		int bestMatchCount = 0;
		int bestIdx = -1;

		for (int r = 0; r < (int)recipes.size(); r++)
		{
			const CookRecipe& recipe = recipes[r];

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

		if (bestMatchCount < (int)best.requiredIngredients.size()) return;

		if (heatSrcPropPtr == nullptr) return;
		bool heatOK = false;
		for (int h : best.heatSources)
		{
			if ((int)heatSrcPropPtr->leadItem.itemCode == h) { heatOK = true; break; }
		}
		if (!heatOK) return;

		if (cookwarePtr == nullptr) return;
		bool cwOK = false;
		for (int c : best.cookwareList)
		{
			if ((int)cookwarePtr->itemCode == c) { cwOK = true; break; }
		}
		if (!cwOK) return;

		if (best.minWaterML > 0)
		{
			int waterML = getWaterML(cookwarePtr);
			if (waterML < best.minWaterML) return;
		}

		matchedRecipeIdx = bestIdx;
		canCook = true;
	}

	void drawDropdown()
	{
		if (!ddOpen) return;

		int visibleCount = std::min((int)ddItems.size(), MAX_DD_VISIBLE);
		int animH = (int)(ddRect.h * ddRatio);

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

		for (int i = 0; i < visibleCount; i++)
		{
			int idx = i + ddScroll;
			if (idx >= (int)ddItems.size()) break;

			SDL_Rect blockRect = { ddRect.x, ddRect.y + DD_BLOCK_H * i, ddRect.w, DD_BLOCK_H - 1 };

			if (checkCursor(&blockRect)) drawFillRect(blockRect, click ? lowCol::deepBlue : lowCol::blue);
			else drawFillRect(blockRect, col::black);

			setZoom(2.0);
			drawSpriteCenter(spr::itemset, (*ddItems[idx].itemPtr).getSprIndex(), blockRect.x + 16, blockRect.y + DD_BLOCK_H / 2);
			setZoom(1.0);

			setFontSize(16);
			if (ddTarget == DD_TRANSFER)
			{
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
				if (waterML > 0) drawText(ddItems[idx].itemPtr->name + L" (" + std::to_wstring(waterML) + L"mL)", blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);
				else drawText(ddItems[idx].itemPtr->name, blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);
			}
			else drawText(ddItems[idx].itemPtr->name, blockRect.x + 42, blockRect.y + DD_BLOCK_H / 2 - 10);

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

		drawRect(ddRect.x, ddRect.y, ddRect.w, ddRect.h, col::gray);

		if ((int)ddItems.size() > MAX_DD_VISIBLE)
		{
			int sbX = ddRect.x + ddRect.w - 4;
			int sbY = ddRect.y + 2;
			int sbH = ddRect.h - 4;

			SDL_Rect scrollTrack = { sbX, sbY, 2, sbH };
			drawFillRect(scrollTrack, { 120, 120, 120 });

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

		if (ddOpen && ddRatio < 1.0f)
		{
			ddRatio += 0.1f;
			if (ddRatio >= 1.0f) ddRatio = 1.0f;
		}

		checkCanCook();
	}
};
