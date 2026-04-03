module;
#include <SDL3/SDL.h>

export module GodPanel;

import std;
import util;
import globalVar;
import constVar;
import textureVar;
import checkCursor;
import drawSprite;
import drawText;
import Player;
import World;
import GUI;
import drawWindow;
import GodService;
import GodBehavior;
import GodRegistry;
import log;

export class GodPanel : public GUI
{
private:
	inline static GodPanel* ptr = nullptr;

	SDL_Rect panelBase;
	SDL_Rect actionBtn;		// Devote / Renounce
	dir16 arrowDir = dir16::left;
	int arrowOffsetY = 145;

	godFlag targetGod = godFlag::none;	// GUI가 표시하는 신
	bool fromBarAct = false;			// barAct에서 열었는지 여부

public:
	// ── 제단 클릭으로 열기 ──
	GodPanel(godFlag god, Point3 tgtPoint) : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than one GodPanel instance was generated.");
		ptr = this;
		targetGod = god;
		fromBarAct = false;

		int revX = tgtPoint.x - PlayerX();
		int revY = tgtPoint.y - PlayerY();

		int arrowEndX, arrowEndY, targetX, targetY;
		if (revX >= 0)
		{
			arrowDir = dir16::left;
			arrowEndX = cameraW / 2 + 8 * zoomScale + 16 * revX * zoomScale;
			arrowEndY = cameraH / 2 + 16 * revY * zoomScale;
			targetX = arrowEndX + 26;
			targetY = arrowEndY - 263;
		}
		else
		{
			arrowDir = dir16::right;
			arrowEndX = cameraW / 2 - 8 * zoomScale + 16 * revX * zoomScale;
			arrowEndY = cameraH / 2 + 16 * revY * zoomScale;
			targetX = arrowEndX - 429;
			targetY = arrowEndY - 263;
		}
		targetX = std::clamp(targetX, 0, cameraW - 404);
		targetY = std::clamp(targetY, 0, cameraH - 506);
		arrowOffsetY = arrowEndY - 25 - targetY;

		changeXY(targetX, targetY, false);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}

	// ── barAct에서 열기 (플레이어 중심) ──
	GodPanel(godFlag god) : GUI(false)
	{
		errorBox(ptr != nullptr, L"More than one GodPanel instance was generated.");
		ptr = this;
		targetGod = god;
		fromBarAct = true;

		// 플레이어 위치 기준 (revX=0, revY=0 → Loot과 동일 공식)
		arrowDir = dir16::left;
		int arrowEndX = cameraW / 2 + 8 * zoomScale;
		int arrowEndY = cameraH / 2;
		int targetX = arrowEndX + 26;
		int targetY = arrowEndY - 263;
		targetX = std::clamp(targetX, 0, cameraW - 404);
		targetY = std::clamp(targetY, 0, cameraH - 506);
		arrowOffsetY = arrowEndY - 25 - targetY;

		changeXY(targetX, targetY, false);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}

	~GodPanel()
	{
		ptr = nullptr;
	}

	static GodPanel* ins() { return ptr; }

	void changeXY(int inputX, int inputY, bool center)
	{
		panelBase = { 0, 0, 404, 506 };
		if (center == false)
		{
			panelBase.x += inputX;
			panelBase.y += inputY;
		}
		else
		{
			panelBase.x += inputX - panelBase.w / 2;
			panelBase.y += inputY - panelBase.h / 2;
		}

		// 우측 하단 버튼 (Msg 버튼 크기: 134x68)
		actionBtn = { panelBase.x + panelBase.w - 134 - 16, panelBase.y + panelBase.h - 68 - 16, 134, 68 };

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - panelBase.w / 2;
			y = inputY - panelBase.h / 2;
		}
	}

	void drawGUI();

	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		// 탭 버튼 (HUD 우측 상단)
		if (checkCursor(&tab))
		{
			executeTab();
			return;
		}

		// Devote / Renounce 버튼
		if (checkCursor(&actionBtn))
		{
			if (playerGod == targetGod)
			{
				// 배교
				GodService::leaveGod();
				updateLog(L"You have renounced your faith.");
				close(aniFlag::null);
			}
			else if (playerGod == godFlag::none)
			{
				// 입교
				GodService::joinGod(targetGod);
				auto* behavior = GodRegistry::get(targetGod);
				if (behavior)
				{
					updateLog(L"You kneel before the altar. " + behavior->name + L" accepts you as a follower.");
				}
				close(aniFlag::null);
			}
			else
			{
				// 이미 다른 신을 믿고 있음
				updateLog(L"You must abandon your current god first.");
			}
			return;
		}
	}

	void step()
	{
		tabType = tabFlag::back;
	}

	void executeTab()
	{
		close(aniFlag::winUnfoldClose);
	}

	void clickRightGUI()
	{
		executeTab();
	}
};
