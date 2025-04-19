#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module ContextMenu;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import wrapVar;
import checkCursor;
import drawWindow;
import actFuncSet;
import Player;
import Loot;
import World;
import Vehicle;
import Bionic;
import log;
import Lst;
import Maint;

export class ContextMenu : public GUI
{
private:
	inline static ContextMenu* ptr = nullptr;
	SDL_Rect contextMenuBase;
	SDL_Rect subContextMenuBase;
	int contextMenuCursor = -1;
	int contextMenuScroll = 0;
	std::vector<act> actOptions;
	std::array<SDL_Rect,30> optionRect;

public:
	ContextMenu(int inputMouseX, int inputMouseY, int inputGridX, int inputGridY, std::vector<act> inputOptions) : GUI(false)
	{
		actOptions = inputOptions;
		contextMenuTargetGrid = { inputGridX, inputGridY };
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//�޼��� �ڽ� ������
		changeXY(inputMouseX, inputMouseY, false);
		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);
	}
	~ContextMenu()
	{
		ptr = nullptr;
	}
	static ContextMenu* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		//94
		contextMenuBase = { 0, 0, 110, 6 + 22 * (int)actOptions.size() };
		subContextMenuBase = { 0,0,0,0 };

		

		if (center == false)
		{
			contextMenuBase.x += inputX;
			contextMenuBase.y += inputY;
		}
		else
		{
			contextMenuBase.x += inputX - contextMenuBase.w / 2;
			contextMenuBase.y += inputY - contextMenuBase.h / 2;
		}


		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - contextMenuBase.w / 2;
			y = inputY - contextMenuBase.h / 2;
		}

		for (int i = 0; i < 30; i++) optionRect[i] = { contextMenuBase.x + 4, contextMenuBase.y + 4 + 22 * i, contextMenuBase.w - 8, 20 };

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }





		if (getFoldRatio() == 1.0)
		{

			drawFillRect(contextMenuBase, col::black);
			drawRect(contextMenuBase, col::gray);

			//94 110
			for (int i = 0; i < actOptions.size(); i++)
			{
				//64,65
				std::wstring optionText;
				int iconIndex = 0;
				if (actOptions[i] == act::closeDoor)
				{
					optionText = sysStr[176];//�ݱ�
					iconIndex = 64;
				}
				else if (actOptions[i] == act::inspect)
				{
					optionText = sysStr[177];//����
					iconIndex = 65;
				}
				else if (actOptions[i] == act::unbox)
				{
					optionText = sysStr[178];//����
					iconIndex = 66;
				}
				else if (actOptions[i] == act::pull)
				{
					optionText = sysStr[179];//����
					iconIndex = 67;
				}
				else if (actOptions[i] == act::climb)
				{
					optionText = sysStr[188];//���
					iconIndex = 69;
				}
				else if (actOptions[i] == act::swim)
				{
					optionText = sysStr[186];//����
					iconIndex = 70;
				}
				else if (actOptions[i] == act::ride)
				{
					optionText = sysStr[187];//ž��
					iconIndex = 71;
				}
				else if (actOptions[i] == act::vehicleRepair)
				{
					optionText = sysStr[203];//����
					iconIndex = 28;
				}
				else if (actOptions[i] == act::vehicleDetach)
				{
					optionText = sysStr[204];//Ż��
					iconIndex = 85;
				}
				else optionText = L"???";

				if (checkCursor(&optionRect[i]))
				{
					if (click) drawFillRect(optionRect[i], lowCol::deepBlue);
					else  drawFillRect(optionRect[i], lowCol::blue);
				}
				else drawFillRect(optionRect[i], lowCol::black);
				setFontSize(16);
				drawTextCenter(col2Str(col::white) + optionText, optionRect[i].x + optionRect[i].w / 2 + 16, optionRect[i].y + optionRect[i].h / 2);
				drawSpriteCenter(spr::icon16, iconIndex, optionRect[i].x + 10, optionRect[i].y + 10);
			}
			
			// �������
			drawLine(contextMenuBase.x, contextMenuBase.y, contextMenuBase.x + 12, contextMenuBase.y, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y + 1, contextMenuBase.x + 12, contextMenuBase.y + 1, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y, contextMenuBase.x, contextMenuBase.y + 12, col::lightGray);
			drawLine(contextMenuBase.x + 1, contextMenuBase.y, contextMenuBase.x + 1, contextMenuBase.y + 12, col::lightGray);

			// �������
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y + 1, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + 1, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + 12, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y, contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y + 12, col::lightGray);

			// �����ϴ�
			drawLine(contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + 12, contextMenuBase.y + contextMenuBase.h - 1, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 2, contextMenuBase.x + 12, contextMenuBase.y + contextMenuBase.h - 2, col::lightGray);
			drawLine(contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);
			drawLine(contextMenuBase.x + 1, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + 1, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);

			// �����ϴ�
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 1, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 13, contextMenuBase.y + contextMenuBase.h - 2, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 2, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + contextMenuBase.w - 1, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);
			drawLine(contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y + contextMenuBase.h - 1, contextMenuBase.x + contextMenuBase.w - 2, contextMenuBase.y + contextMenuBase.h - 13, col::lightGray);


			drawFillRect(subContextMenuBase, col::black);

		}
		else
		{
			SDL_Rect vRect = contextMenuBase;
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
		else if(checkCursor(&contextMenuBase))
		{
			for (int i = 0; i < actOptions.size(); i++)
			{
				if (checkCursor(&optionRect[i]))
				{
					CORO(executeContextAct(actOptions[i]));
				}

			}

			close(aniFlag::null);
		}
		else close(aniFlag::winUnfoldClose);
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
		contextMenuBase.h = 6 + 22 * (int)actOptions.size();
		//bool openSub = false;
		//for (int i = 0; i < actOptions.size(); i++)
		//{
		//	if (actOptions[i] == act::inspect)
		//	{
		//		SDL_Rect optionRect = { contextMenuBase.x + 4, contextMenuBase.y + 4 + 22 * i, 90, 20 };
		//		if (checkCursor(&optionRect) || checkCursor(&subContextMenuBase))
		//		{
		//			subContextMenuBase = { contextMenuBase.x + contextMenuBase.w, contextMenuBase.y + 4 + 22 * i - 1, contextMenuBase.w,100 };
		//			openSub = true;
		//		}
		//	}
		//}

		//if(openSub==false) subContextMenuBase = { -1, -1, -1,-1 };
	}

	Corouter executeContextAct(act inputAct)
	{
		if (inputAct == act::closeDoor)
		{
			actFunc::closeDoor(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			tabType = tabFlag::autoAtk;
		}
		else if (inputAct == act::unbox)
		{
			if (World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).VehiclePtr != nullptr)
			{
				Vehicle* vPtr = (Vehicle*)World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).VehiclePtr;
				for (int i = 0; i < vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo.size(); i++)
				{
					if (vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo[i].checkFlag(itemFlag::POCKET))
					{
						new Loot((ItemPocket*)(vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo[i].pocketPtr), &(vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo[i]));
						break;
					}
				}
			}
			tabType = tabFlag::autoAtk;
		}
		else if (inputAct == act::pull)
		{
			if (Player::ins()->pulledCart == nullptr)
			{
				Player::ins()->pulledCart = (Vehicle*)World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).VehiclePtr;
			}
			else
			{
				Player::ins()->pulledCart = nullptr;
			}
			tabType = tabFlag::autoAtk;
		}
		else if (inputAct == act::ride)
		{
			Player::ins()->ridingEntity = TileEntity(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ());
			World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).EntityPtr = nullptr;
			Player::ins()->ridingType = ridingFlag::horse;
			tabType = tabFlag::autoAtk;
		}
		else if (inputAct == act::vehicleRepair)
		{
			Vehicle* vPtr = (Vehicle*)World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).VehiclePtr;
			std::vector<ItemData>& vParts = vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo;
			std::vector<std::wstring> partNames;
			for (int i = 0; i < vParts.size(); i++)
			{
				partNames.push_back(vParts[i].name);
			}

			new Maint(L"����", L"������ ��ǰ�� �������ּ���.", {contextMenuTargetGrid.x,contextMenuTargetGrid.y,PlayerZ()},maintFlag::repair);
			co_await std::suspend_always();
		}
		else if (inputAct == act::vehicleDetach)
		{
			Vehicle* vPtr = (Vehicle*)World::ins()->getTile(contextMenuTargetGrid.x, contextMenuTargetGrid.y, PlayerZ()).VehiclePtr;
			std::vector<ItemData>& vParts = vPtr->partInfo[{contextMenuTargetGrid.x, contextMenuTargetGrid.y}]->itemInfo;
			std::vector<std::wstring> partNames;
			for (int i = 0; i < vParts.size(); i++)
			{
				partNames.push_back(vParts[i].name);
			}

			new Maint(L"Ż��", L"�������� �и��� ��ǰ�� �������ּ���.", { contextMenuTargetGrid.x,contextMenuTargetGrid.y,PlayerZ() },maintFlag::detach);
			co_await std::suspend_always();
		}
	}
};