#include <SDL.h>

export module Inventory;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import ItemData;
import ItemPocket;
import drawItemList;

export class Inventory : public GUI
{
private:
	ItemData* inventoryItemData;
	ItemPocket* inventoryPocket;

	inline static Inventory* ptr = nullptr;
	SDL_Rect inventoryBase;
	std::array<SDL_Rect, 12> bionicRect;
	int inventoryCursor = -1;
	int inventoryScroll = 0;

	std::wstring titleInventory = sysStr[185];
	int titleItemSprIndex = 60;

	std::array<SDL_Rect, INVENTORY_ITEM_MAX> inventoryItemRect; //마우스를 위한 인벤아이템렉트 판정 박스
	std::array<SDL_Rect, INVENTORY_ITEM_MAX> inventoryItemSelectRect; //마우스를 위한 인벤아이템렉트 셀렉트 판정 박스
	SDL_Rect inventoryLabel;
	SDL_Rect inventoryLabelSelect;
	SDL_Rect inventoryLabelName;
	SDL_Rect inventoryLabelQuantity;
	SDL_Rect inventoryScrollBox;

public:
	Inventory(int inputX, int inputY, ItemData* inputData) : GUI(false)
	{
		inventoryItemData = inputData;
		inventoryPocket = inputData->pocketPtr.get();

		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(inputX, inputY, false);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~Inventory()
	{
		tabType = tabFlag::closeWin;
		ptr = nullptr;
	}
	static Inventory* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		inventoryBase = { 0, 0, 335, 336 };
		inventoryBase.h = 164 + 32 * myMax(0, (myMin(INVENTORY_ITEM_MAX, inventoryPocket->itemInfo.size() - 1)));

		if (center == false)
		{
			inventoryBase.x += inputX;
			inventoryBase.y += inputY;
		}
		else
		{
			inventoryBase.x += inputX - inventoryBase.w / 2;
			inventoryBase.y += inputY - inventoryBase.h / 2;
		}

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - inventoryBase.w / 2;
			y = inputY - inventoryBase.h / 2;
		}

		for (int i = 0; i < INVENTORY_ITEM_MAX; i++)
		{
			inventoryItemRect[i] = { inventoryBase.x + 52, inventoryBase.y + 125 + 32*i, 270, 26 };
			inventoryItemSelectRect[i] = { inventoryBase.x + 10, inventoryBase.y + 125 + 32 * i, 36, 26 };
		}
		inventoryLabel = { inventoryBase.x + 10, inventoryBase.y + 95, inventoryBase.w - 20 , 26 };
		inventoryLabelSelect = { inventoryLabel.x, inventoryLabel.y, 62 , 26 };
		inventoryLabelName = { inventoryLabel.x + inventoryLabelSelect.w, inventoryLabel.y, 182 , 26 };
		inventoryLabelQuantity = { inventoryLabel.x + inventoryLabelName.w + inventoryLabelSelect.w, inventoryLabel.y, 71 , 26 };
	}
	void drawGUI();
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
	void clickMotionGUI(int dx, int dy) { }
	void clickDownGUI() { }
	void clickRightGUI() { }
	void mouseWheel() 
	{
		if (checkCursor(&inventoryBase))
		{
			if (event.wheel.y > 0 && inventoryScroll > 1) inventoryScroll -= 1;
			else if (event.wheel.y < 0 && inventoryScroll + INVENTORY_ITEM_MAX < inventoryPocket->itemInfo.size()) inventoryScroll += 1;
		}
	}
	void clickHoldGUI() { }
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step() { }
};