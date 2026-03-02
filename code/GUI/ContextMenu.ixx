#include <SDL3/SDL.h>

export module ContextMenu;

import std;
import constVar;
import GUI;

export class ContextMenu : public GUI
{
private:
	inline static ContextMenu* ptr = nullptr;
	SDL_Rect contextMenuBase;
	SDL_Rect subContextMenuBase;
	int contextMenuCursor = -1;
	int contextMenuScroll = 0;
	std::vector<act> actOptions;
	std::array<SDL_Rect, 30> optionRect;

public:
	ContextMenu(int inputMouseX, int inputMouseY, int inputGridX, int inputGridY, std::vector<act> inputOptions);
	~ContextMenu();
	static ContextMenu* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center);
	void drawGUI();
	void clickUpGUI();
	void step();
	void executeContextAct(act inputAct);
};
